#include <stdLib.h>
#include <stdio.h>
#include <Windows.h>
#include <Shlwapi.h>

#include <string>

#include "../common/defs.h"
#include "../common/tools.h"

#include "zip.h"
#include "unzip.h"
#include "iowin32.h"

void replaceSlashes (char *path) {
    for (auto chr = path; *chr; ++ chr) {
        if ((*chr) == '/') *chr = '\\';
    }
}

void CreateDirectoryRecursive (char *destPath, char *path) {
    if (strchr (path, '/') || strchr (path, '\\')) {
        char folder [MAX_PATH];

        strcpy (folder, path);
        replaceSlashes (folder);
        PathRemoveFileSpecA (folder);
        CreateDirectoryRecursive (destPath, folder);
    }

    char fullFolderPath [MAX_PATH];    
    PathCombineA (fullFolderPath, destPath, path);
    replaceSlashes (fullFolderPath);
    CreateDirectoryA (fullFolderPath, 0);
}

bool unzipAll (char *path, char *destPath) {
    bool result = false;

    zlib_filefunc_def funcDef;

    fill_fopen_filefunc (& funcDef);

    auto archive = unzOpen2 (path, & funcDef);

    if (archive) {
        unz_global_info globalInfo;
        unz_file_info fileInfo;
        char fileName [MAX_PATH];

        if (unzGetGlobalInfo (archive, & globalInfo) == ZIP_OK) {
            for (auto i = 0; i < globalInfo.number_entry; ++ i) {
                unzGetCurrentFileInfo (archive, & fileInfo, fileName, sizeof (fileName), NULL, 0, NULL, 0);

                if ((fileInfo.external_fa & FILE_ATTRIBUTE_DIRECTORY) == 0) {
                    // If the file contains subpath we make sure that apprpriate folder exists
                    if (strchr (fileName, '/') || strchr (fileName, '\\')) {
                        char folder [MAX_PATH];
                        strcpy (folder, fileName);
                        replaceSlashes (folder);
                        PathRemoveFileSpecA (folder);
                        CreateDirectoryRecursive (destPath, folder);
                    }

                    char *buffer = (char *) malloc (fileInfo.uncompressed_size);

                    if (buffer) {
                        unzOpenCurrentFile (archive);

                        auto actualSize = unzReadCurrentFile (archive, buffer, fileInfo.uncompressed_size);

                        char destFilePath [MAX_PATH];
                        PathCombineA (destFilePath, destPath, fileName);
                        replaceSlashes (destFilePath);

                        FILE *destFile = fopen (destFilePath, "wb");
                        fwrite (buffer, 1, actualSize, destFile);
                        fclose (destFile);

                        free (buffer);

                        unzGoToNextFile (archive);
                    } else {
                        break;
                    }
                } else {
                    char folderPath [MAX_PATH];

                    PathCombineA (folderPath, destPath, fileName);
                    CreateDirectoryA (folderPath, 0);
                }
            }
        }
        result = true;

        unzClose (archive);
    }

    return result;
}

size_t extractAndPopulateField (config& cfg, bunkeringData& data, char *source , size_t index, std::string& result) {
    enum field {
        port = 0,
        bunkeringDate,
        vessel,
        imo,
        beginTime,
        endTime,
        beginDate,
        endDate,
        barge,
        densityLoaded,
        viscosityLoaded,
        sulphurLoaded,
        temperatureLoaded,
        volumeLoaded,
        quantityLoaded,
    };

    static std::vector<char *> templateFields {
        "__PORT__",
        "__BUNK_DATE__",
        "__VESSEL__",
        "__IMO__",
        "__B_TM__",
        "__E_TM__",
        "__B_DT__",
        "__E_DT__",
        "__BARGE__",
        "__DENS_LD__",
        "__VISC_LD__",
        "__SULP_LD__",
        "__TEMP_LD__",
        "__VOL_LD__",
        "__QNTY_LD__",
    };

    auto compare = [] (char *field, char *buffer, size_t offset) {
        for (auto i = 0; field [i]; ++ i) {
            if (field [i] != buffer [offset+i]) return false;
        }
        return true;
    };

    auto ansi2utf8 = [] (const char *source, char *dest, size_t size) {
        wchar_t buffer [200];
        MultiByteToWideChar (CP_ACP, 0, source, -1, buffer, sizeof (buffer) / sizeof (*buffer));
        WideCharToMultiByte (CP_UTF8, 0, buffer, -1, dest, size, 0, 0);
        return dest;
    };

    for (auto i = 0; i < templateFields.size (); ++ i) {
        if (compare (templateFields [i], source, index)) {
            char buffer [100];
            switch (i) {
                case field::port:
                    result += ansi2utf8 (data.port.c_str (), buffer, sizeof (buffer)); break;
                case field::bunkeringDate:
                    result += formatTimestampEx (data.begin, buffer, timiestampFormatFlags::showDate); break;
                case field::vessel:
                    result += cfg.shipInfo.name; break;
                case field::imo:
                    result += itoa (cfg.shipInfo.imo, buffer, 10); break;
                case field::beginTime:
                    result += formatTimestampEx (data.begin, buffer, timiestampFormatFlags::showTime); break;
                case field::endTime:
                    result += formatTimestampEx (data.end, buffer, timiestampFormatFlags::showTime); break;
                case field::beginDate:
                    result += formatTimestampEx (data.begin, buffer, timiestampFormatFlags::showDate); break;
                case field::endDate:
                    result += formatTimestampEx (data.end, buffer, timiestampFormatFlags::showDate); break;
                case field::barge:
                    result += ansi2utf8 (data.barge.c_str (), buffer, sizeof (buffer)); break;
                case field::densityLoaded:
                    result += ftoa (data.loaded.density, buffer, "%.4f"); break;
                case field::viscosityLoaded:
                    result += ftoa (data.loaded.density, buffer, "%.2f"); break;
                case field::sulphurLoaded:
                    result += ftoa (data.loaded.sulphur, buffer, "%.2f"); break;
                case field::temperatureLoaded:
                    result += ftoa (data.loaded.temp, buffer, "%.1f"); break;
                case field::volumeLoaded:
                    result += ftoa (data.loaded.volume, buffer, "%.3f"); break;
                case field::quantityLoaded:
                    result += ftoa (data.loaded.quantity, buffer, "%.3f"); break;
            }

            return index + strlen (templateFields [i]) - 1;
        }
    }

    return index;
}

void populateData (config& cfg, bunkeringData& data, char *docPath) {
    char stringFilePath [MAX_PATH];

    PathCombineA (stringFilePath, docPath, "xl\\sharedStrings.xml");
    replaceSlashes (stringFilePath);

    FILE *file = fopen (stringFilePath, "rb");

    if (file) {
        size_t size;
        char *buffer;
        std::string result;

        fseek (file, 0, SEEK_END);

        size = ftell (file);

        fseek (file, 0, SEEK_SET);

        buffer = (char *) malloc (size + 1);

        if (buffer) {
            fread (buffer, 1, size, file);

            buffer [size] = '\0';

            for (auto i = 0; buffer [i]; ++ i) {
                if (buffer [i] == '_' && (i + 1) < size && buffer [i+1] == '_') {
                    i = extractAndPopulateField (cfg, data, buffer , i, result);
                } else {
                    result += buffer [i];
                }
            }

            free (buffer);
        }

        fclose (file);

        FILE *file = fopen (stringFilePath, "wb");

        if (file) {
            fwrite (result.c_str (), 1, result.length (), file);
            fclose (file);
        }
    }
}

void generateReport (config& cfg, bunkeringData& data) {
    char templPath [MAX_PATH], docPath [MAX_PATH], folder [100];

    sprintf (folder, "bd_%d_%zd", data.id, time (0));

    GetModuleFileNameA (0, templPath, sizeof (templPath));
    PathRemoveFileSpecA (templPath);
    PathAppendA (templPath, "..");
    PathAppendA (templPath, cfg.repCfg.templatePath.c_str ());

    GetModuleFileNameA (0, docPath, sizeof (docPath));
    PathRemoveFileSpecA (docPath);
    PathAppendA (docPath, "../doc");
    PathAppendA (docPath, folder);
    CreateDirectoryA (docPath, 0);

    unzipAll (templPath, docPath);
    populateData (cfg, data, docPath);
}
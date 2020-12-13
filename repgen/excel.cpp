#include <stdLib.h>
#include <stdio.h>
#include <Windows.h>
#include <Shlwapi.h>

#include <string>

#include "../common/defs.h"
#include "../common/tools.h"
#include "../zip/zip_tool.h"

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
                    result += _itoa (cfg.shipInfo.imo, buffer, 10); break;
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
                    result += ftoa (data.loaded.volume.reported, buffer, "%.3f"); break;
                case field::quantityLoaded:
                    result += ftoa (data.loaded.quantity.reported, buffer, "%.3f"); break;
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

void generateReport (config& cfg, bunkeringData& data, HINSTANCE instance, HWND parent) {
    char templPath [MAX_PATH], docPath [MAX_PATH], tempPath [MAX_PATH], folder [100], reportPath [MAX_PATH], fileTitle [200];
    OPENFILENAMEA fileInfo;

    memset (& fileInfo, 0, sizeof (fileInfo));
    memset (reportPath, 0, sizeof (reportPath));
    memset (fileTitle, 0, sizeof (fileTitle));

    fileInfo.hInstance = instance;
    fileInfo.hwndOwner = parent;
    fileInfo.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    fileInfo.lpstrFile = reportPath;
    fileInfo.lpstrFileTitle = fileTitle;
    fileInfo.nMaxFile = sizeof (reportPath);
    fileInfo.nMaxFileTitle = sizeof (fileTitle);
    fileInfo.lpstrFilter = "Excel workbooks (*.xlsx)\0*.xlsx\0All files\0*.*\0\0";
    fileInfo.lpstrTitle = "Select report workbook";
    fileInfo.lpstrDefExt = ".xlsx";
    fileInfo.lStructSize = sizeof (fileInfo);

    if (GetSaveFileNameA (& fileInfo)) {
        sprintf (folder, "bd_%d_%I64d", data.id, time (0));

        GetModuleFileNameA (0, templPath, sizeof (templPath));
        PathRemoveFileSpecA (templPath);
        PathAppendA (templPath, "..");
        PathAppendA (templPath, cfg.repCfg.templatePath.c_str ());

        GetModuleFileNameA (0, docPath, sizeof (docPath));
        PathRemoveFileSpecA (docPath);
        PathAppendA (docPath, "../doc");
        PathAppendA (docPath, folder);
        CreateDirectoryA (docPath, 0);

        GetModuleFileNameA (0, tempPath, sizeof (tempPath));
        PathRemoveFileSpecA (tempPath);
        PathAppendA (tempPath, "../temp");
        PathAppendA (tempPath, folder);
        CreateDirectoryA (tempPath, 0);

        unzipAll (templPath, tempPath);
        populateData (cfg, data, tempPath);
        //PathCombineA (reportPath, docPath, "report.xlsx");
        zipFolder (tempPath, tempPath, reportPath);
    }
}

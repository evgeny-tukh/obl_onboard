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

struct zipInfo {
    void *archive;
    char *basePath, *archivePath;
};

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

void populateZipFileInfo (zip_fileinfo& fileInfo, char *path, WIN32_FIND_DATAA *findData) {
    FILETIME locatFileTime;
    WORD *dosTime = (WORD *) & fileInfo.dosDate;

    memset (& fileInfo, 0, sizeof (fileInfo));

    FileTimeToLocalFileTime (& findData->ftLastWriteTime, & locatFileTime);
    FileTimeToDosDateTime (& locatFileTime, dosTime + 1, dosTime);
}

void zipCb (char *filePath, void *param, WIN32_FIND_DATAA *findData) {
    zipInfo *info = (zipInfo *) param;
    zlib_filefunc_def funcDef;
    zip_fileinfo fileInfo;

    fill_win32_filefunc (& funcDef);

    if (!info->archive) info->archive = zipOpen2 (info->archivePath, APPEND_STATUS_ADDINZIP, 0, & funcDef);
    if (!info->archive) info->archive = zipOpen2 (info->archivePath, APPEND_STATUS_CREATE, 0, & funcDef);
    if (!info->archive) return;

    char *subPath = filePath;

    for (auto i = 0; filePath [i]; ++ i) {
        if (filePath [i] != info->basePath [i]) {
            subPath = filePath + i; break;
        }
    }

    while (*subPath == '/' || *subPath == '\\') ++ subPath;

    populateZipFileInfo (fileInfo, filePath, findData);
    zipOpenNewFileInZip3 (info->archive, subPath, & fileInfo, 0, 0, 0, 0, 0, Z_DEFLATED, 9, 0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, 0, 0);

    FILE *file = fopen (filePath, "rb+");

    if (file) {
        fseek (file, 0, SEEK_END);

        auto size = ftell (file);

        fseek (file, 0, SEEK_SET);

        char *buffer = (char *) malloc (size + 1);

        buffer [size] = '\0';

        fread (buffer, 1, size, file);
        fclose (file);

        zipWriteInFileInZip (info->archive, buffer, size);
        free (buffer);
    }

    zipCloseFileInZip (info->archive);
}

void zipFolder (char *folderPath, char *basePath, char *archivePath) {
    zipInfo info { 0, basePath, archivePath };

    walkThroughFolder (folderPath, zipCb, & info);
    
    if (info.archive) zipClose (info.archive, 0);
}

#include <stdLib.h>
#include <stdio.h>
#include <Windows.h>
#include <Shlwapi.h>

#include "../common/defs.h"

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
        PathRemoveFileSpec (folder);
        CreateDirectoryRecursive (destPath, folder);
    }

    char fullFolderPath [MAX_PATH];    
    PathCombine (fullFolderPath, destPath, path);
    replaceSlashes (fullFolderPath);
    CreateDirectory (fullFolderPath, 0);
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
                        PathRemoveFileSpec (folder);
                        CreateDirectoryRecursive (destPath, folder);
                    }

                    char *buffer = (char *) malloc (fileInfo.uncompressed_size);

                    if (buffer) {
                        unzOpenCurrentFile (archive);

                        auto actualSize = unzReadCurrentFile (archive, buffer, fileInfo.uncompressed_size);

                        char destFilePath [MAX_PATH];
                        PathCombine (destFilePath, destPath, fileName);
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

                    PathCombine (folderPath, destPath, fileName);
                    CreateDirectory (folderPath, 0);
                }
            }
        }
        result = true;

        unzClose (archive);
    }

    return result;
}

void generateReport (bunkeringData& data) {
    char templPath [MAX_PATH], docPath [MAX_PATH], folder [100];

    sprintf (folder, "bd_%d_%zd", data.id, time (0));

    GetModuleFileName (0, templPath, sizeof (templPath));
    PathRemoveFileSpec (templPath);
    PathAppend (templPath, "../templ/bunker1.xlsx");

    GetModuleFileName (0, docPath, sizeof (docPath));
    PathRemoveFileSpec (docPath);
    PathAppend (docPath, "../doc");
    PathAppend (docPath, folder);
    CreateDirectory (docPath, 0);

    unzipAll (templPath, docPath);
}
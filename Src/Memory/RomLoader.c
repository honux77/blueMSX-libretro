/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Memory/RomLoader.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2008-03-30 18:38:42 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "RomLoader.h"
#include "ziphelper.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __linux__
#include <dirent.h>
#include <strings.h>
#endif

// PacketFileSystem.h Need to be included after all other includes
#include "PacketFileSystem.h"

#ifdef __linux__
static FILE* fopenCaseInsensitive(const char* filename, const char* mode)
{
    FILE* file;
    char dir[1024];
    char base[256];
    char* p;
    DIR* dp;
    struct dirent* de;

    file = fopen(filename, mode);
    if (file) return file;

    strncpy(dir, filename, sizeof(dir) - 1);
    dir[sizeof(dir) - 1] = 0;
    p = strrchr(dir, '/');
    if (!p) return NULL;

    strncpy(base, p + 1, sizeof(base) - 1);
    base[sizeof(base) - 1] = 0;
    *p = 0;

    dp = opendir(dir);
    if (!dp) return NULL;

    while ((de = readdir(dp)) != NULL) {
        if (strcasecmp(de->d_name, base) == 0) {
            char fullpath[1280];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, de->d_name);
            closedir(dp);
            return fopen(fullpath, mode);
        }
    }

    closedir(dp);
    return NULL;
}
#endif

UInt8* romLoad(const char *fileName, const char *fileInZipFile, int* size)
{
    UInt8* buf = NULL;
    FILE *file;

    if (!fileName || strlen(fileName) == 0)
        goto error;

    if (fileInZipFile && strlen(fileInZipFile) == 0)
        fileInZipFile = NULL;

    if (fileInZipFile)
    {
        buf = (UInt8*)zipLoadFile(fileName, fileInZipFile, size);
        if (!buf)
           goto error;
        return buf;
    }

#ifdef __linux__
    file = fopenCaseInsensitive(fileName, "rb");
#else
    file = fopen(fileName, "rb");
#endif
    if (!file)
        goto error;

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    if (*size == 0) {
        fclose(file);
        return (UInt8*)malloc(1);
    }

    fseek(file, 0, SEEK_SET);

    buf = (UInt8*)malloc(*size);
    
    *size = fread(buf, 1, *size, file);
    fclose(file);

    return buf;

error:
    if (fileName && fileName[0])
      fflush(stdout);
    return NULL;
}

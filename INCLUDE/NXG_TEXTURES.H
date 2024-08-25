#pragma once
#include "INCLUDE/FILEIO.H"

typedef struct NuTextureSet_s       NuTextureSet;
typedef struct NuTextureHeader_s    NuTextureHeader;
typedef U8 NuChecksum[16];

struct NuTextureHeader_s
{
    NuChecksum checksum;
    char      *Path;
    U32        nuttype;
    U32        Level;
    U32        ObjectID;
};

struct NuTextureSet_s
{
    U32   Version;
    char  FourCC[8];
    char *CONVDATE;
    U32   NumberOfFiles;
    NuTextureHeader *FileHeaders;
};


SIZE READTEXTURESET(NuTextureSet *TXST, const char *Path);
void DESTROYTEXTURESET(NuTextureSet *TXST);

void EXTRACTTEXTURES(PATHINFO *Path);
void COMPILETEXTURES(PATHINFO *Path, const char *HDRPATH);
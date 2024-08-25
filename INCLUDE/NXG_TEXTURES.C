#include <STRING.H>
#include "INCLUDE/DDS.H"

#include "NXG_TEXTURES.H"

typedef void (*ReadNuTHeaderFunc)(NuTextureHeader *hdr, FILE *fh);

static 
void ReadNuTHeader_Gen0(NuTextureHeader *hdr, FILE *fh)
{
    READBUFFERED(hdr->checksum, 16, fh);
    hdr->Path = READSTRING(STRING32, fh);
    hdr->nuttype = READU32(fh);
}

static 
void ReadNuTHeader_Gen1(NuTextureHeader *hdr, FILE *fh)
{
    READBUFFERED(hdr->checksum, 16, fh);
    U16 StringSize = READU16(fh);
    IGNORE(StringSize, fh);
    hdr->Path = READSTRING(STRING16, fh);
    hdr->nuttype = READU8(fh);
}

static 
void ReadNuTHeader_Gen2(NuTextureHeader *hdr, FILE *fh)
{
    ReadNuTHeader_Gen1(hdr, fh);

    (void)READU8(fh);
    (void)READU8(fh);
    (void)READU16(fh);
    U16 strsize = READU16(fh);
    IGNORE(strsize, fh);
    (void)READU8(fh);
}


SIZE READTEXTURESET(NuTextureSet *TXST, const char *Path)
{
    SETBIGENDIAN(1);
    FILE *fh = OPEN_READ(Path, F_BINARY);

    U32 resh = READU32(fh);

    IGNORE(resh, fh);

    IGNORE(4, fh); // Size
    READ4CC("4CC.", fh);
    READ4CC("TXST", fh);

    U32 TXSTCount = READU32(fh);

    READ4CC("TXST", fh);
    TXST->Version = READU32(fh);

    ReadNuTHeaderFunc ReadFunction = ReadNuTHeader_Gen0;
    switch (TXST->Version)
    {
        case 0x01:
            ReadFunction = ReadNuTHeader_Gen0;
            break;
        case 0x0C:
            ReadFunction = ReadNuTHeader_Gen1;
            break;
        case 0x0E:
            ReadFunction = ReadNuTHeader_Gen2;
            break;
        default:
            break;
    }

    if (TXST->Version)
    {
        TXST->CONVDATE = READSTRING(STRING32, fh);
    }

    (void)READU32(fh); // VTOR IS FOR SOME REASON MISSING IN DX11
    TXST->NumberOfFiles = READU32(fh);

    TXST->FileHeaders = calloc(TXST->NumberOfFiles, sizeof(NuTextureHeader));
    ASSERT(TXST->FileHeaders);

    for (int i = 0; i < TXST->NumberOfFiles; i++)
    {
        ReadFunction(&TXST->FileHeaders[i], fh);
    }

    SIZE pos = GET_POSITION(fh);

    CLOSE(fh);

    SETBIGENDIAN(0);

    return pos;
}

void DESTROYTEXTURESET(NuTextureSet *TXST)
{
    for (int i = 0; i < TXST->NumberOfFiles; i++)
    {
        free(TXST->FileHeaders[i].Path);
    }

    free(TXST->FileHeaders);
    free(TXST->CONVDATE);
}
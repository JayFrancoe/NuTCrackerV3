#include "INCLUDE/FILEIO.H"
#include "INCLUDE/NXG_TEXTURES.H"

extern int __cdecl strcmp(char const *_Str1, char const *_Str2);

static
SIZE FILE_COPY(const char *FILE1, const char *FILE2, MODE ATTRIBUTES)
{
    FILE *hF1 = OPEN_READ(FILE1, ATTRIBUTES);
    FILE *hF2 = OPEN_WRITE(FILE2, ATTRIBUTES);

    SIZE sz = GET_SIZE(hF1);

    U8 *DATA = calloc(sz, 1);
    ASSERT(DATA);

    fread(DATA, 1, sz, hF1);
    fwrite(DATA, 1, sz, hF2);

    free(DATA);

    CLOSE(hF1);
    CLOSE(hF2);

    return sz;
}

static
SIZE FILE_APPEND(FILE *OUTFILE, const char *FILENAME, MODE ATTRIBUTES)
{
    FILE *hF1 = OPEN_READ(FILENAME, ATTRIBUTES);

    SIZE sz = GET_SIZE(hF1);

    U8 *DATA = calloc(sz, 1);
    ASSERT(DATA);

    fread(DATA, 1, sz, hF1);
    fwrite(DATA, 1, sz, OUTFILE);

    free(DATA);

    CLOSE(hF1);

    return sz;
}

void COMPILETEXTURES(PATHINFO *Path, const char *HDRPATH)
{
    NuTextureSet TXST = { 0 };
    READTEXTURESET(&TXST, HDRPATH);

    char OUTPATH[260];
    sprintf(OUTPATH, "%s/%s", Path->FileDirectory, Path->FileName);

    char OUTFILE[260];
    sprintf(OUTFILE,
        "%s/%s_new.%s", Path->FileDirectory, Path->FileName, Path->FileExtension);

    FILE *hTXST = OPEN_WRITE(OUTFILE, F_BINARY);

    // HDR
    FILE_APPEND(hTXST, HDRPATH, F_BINARY);

    U32 nlightmaps = 0;

    for (int i = 0; i < TXST.NumberOfFiles; i++)
    {
        NuTextureHeader *cur = &TXST.FileHeaders[i];

        if ('\0' == *cur->Path)
            continue;

        PATHINFO tmp = {0,0,0,0};
        GET_PATHINFO(&tmp, cur->Path);

        char folder[260] = {0};
        GET_FOLDER(tmp.FilePath, folder);

        char FILENAME[260];
        if (cur->nuttype == 0x0400
        || !strcmp(tmp.FilePath, "Lightmap"))
        {
            sprintf(FILENAME, "lightmaps/lightmap%u.dds", nlightmaps++);
        }
        else
        {
            sprintf(FILENAME, "%s/%s.dds", folder, tmp.FileName);
        }

        char FILEPATH[260];
        sprintf(FILEPATH, "%s/%s", OUTPATH, FILENAME);

        SIZE sz = FILE_APPEND(hTXST, FILEPATH, F_BINARY);

        printf("%3u: %08zu %s\n", i, sz, FILENAME);
    }

    DESTROYTEXTURESET(&TXST);
    SIZE sz = GET_POSITION(hTXST);

    CLOSE(hTXST);

    printf("FINISHED COMPILING FILE (%08zx bytes)", sz);
}
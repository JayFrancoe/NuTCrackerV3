#include <string.h>
#include "INCLUDE/FILEIO.H"
#include "INCLUDE/DDS.H"

#include "INCLUDE/NXG_TEXTURES.H"

void EXTRACTTEXTURES(PATHINFO *Path)
{
    // GET TEXTURESET DATA

    char OUTPATH[260];
    sprintf(OUTPATH, "%s/%s", Path->FileDirectory, Path->FileName);

    NuTextureSet TXST = { 0 };

    SIZE offset = READTEXTURESET(&TXST, Path->FilePath);

    SETBIGENDIAN(0);

    FILE *hTXST = OPEN_READ(Path->FilePath, F_BINARY);

    CREATE_DIRECTORY(OUTPATH);

    // HEADER
    {
        U8 *header = calloc(offset, 1);
        ASSERT(NULL != header);

        fread(header, 1, offset, hTXST);

        char HDRFilePath[260];
        sprintf(HDRFilePath, "%s/%s.HDR", OUTPATH, Path->FileName);

        FILE *hHDR = OPEN_WRITE(HDRFilePath, F_BINARY);

        fwrite(header, 1, offset, hHDR);

        free(header);
        CLOSE(hHDR);
    }

    char LIGHTMAPS[260];
    sprintf(LIGHTMAPS, "%s/lightmaps", OUTPATH);

    U32 nLightmaps = 0;

    for (U32 i = 0; i < TXST.NumberOfFiles; i++)
    {
        NuTextureHeader *cur = &TXST.FileHeaders[i];

        if (cur->Path == NULL)
            continue;

        PATHINFO pi = { 0,0,0,0 };
        GET_PATHINFO(&pi, cur->Path);

        char TEXTURENAME[260] = { 0 };
        if (cur->nuttype == 0x0400
            || !strcmp(pi.FilePath, "Lightmap")) // LIGHTMAP
        {
            CREATE_DIRECTORY(LIGHTMAPS);

            sprintf(TEXTURENAME, "lightmaps/lightmap%u.dds", nLightmaps++);
        }
        else
        {
            char FOLDER[50];
            GET_FOLDER(pi.FilePath, FOLDER);

            if (FOLDER[0])
            {
                sprintf(TEXTURENAME, "%s/%s", OUTPATH, FOLDER);
            }

            CREATE_DIRECTORY(TEXTURENAME);

            if (FOLDER[0])
                sprintf(TEXTURENAME, "%s/%s.dds", FOLDER, pi.FileName);
            else
                sprintf(TEXTURENAME, "%s.dds", pi.FileName);
        }

        char TEXTUREPATH[260];
        sprintf(TEXTUREPATH, "%s/%s", OUTPATH, TEXTURENAME);

        READ4CC("DDS ", hTXST);
        //printf("%#08x\n", ftell(hTXST));

        IMAGE_DDS Image = { 0 };
        int ret = ReadDDSFile(hTXST, &Image);
        if (0 == ret)
        {
            WriteDDS(&Image, TEXTUREPATH);
            printf("%3u: %s\n", i, TEXTURENAME);
            continue;
        }

        // If DDS Header could not be read
        // Read byte by byte

        FILE *hDDS = OPEN_WRITE(TEXTUREPATH, F_BINARY);

        fwrite("DDS ", 4, 1, hDDS);

        fwrite(&Image.HEADER, sizeof(DDS_HEADER), 1, hDDS);

        if (Image.HEADER_DXT10.dxgiFormat)
        {
            fwrite(&Image.HEADER_DXT10, 0x14, 1, hDDS);
        }

        while (0 == IS_EOF(hTXST))
        {
            char c = READU8(hTXST);
            if (c == 'D')
            {
                c = READU8(hTXST);
                if (c == 'D')
                {
                    c = READU8(hTXST);
                    if (c == 'S')
                    {
                        int pos = GET_POSITION(hTXST);
                        SET_POSITION(pos - 3, hTXST);
                        break;
                    }
                    WRITEU8(c, hDDS);
                    continue;
                }
                WRITEU8(c, hDDS);
                continue;
            }
            WRITEU8(c, hDDS);
        }

        printf("%3u: %s\n", i, TEXTURENAME);
    }
    DESTROYTEXTURESET(&TXST);
    CLOSE(hTXST);

    puts("Finished parsing nxg_textures file.");
}
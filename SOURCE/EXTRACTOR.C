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

    SIZE filesize = GET_SIZE(hTXST);


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

        if ('\0' == cur->Path[0])
            continue;

        SIZE POSITION = GET_POSITION(hTXST);

        if (POSITION >= filesize)
        {
            printf("[!! END OF FILE REACHED !!]\n");
            break;
        }

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


#ifdef _DEBUG
        printf("%#08x\n", POSITION);
#endif // DEBUG

        READ4CC("DDS ", hTXST);

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

        printf("NON-STANDARD HEADER FOUND (0x%08x) READING BYTE BY BYTE...\n", Image.HEADER.ddspf.dwFourCC);

        FILE *hDDS = OPEN_WRITE(TEXTUREPATH, F_BINARY);

        fwrite("DDS ", 4, 1, hDDS);

        fwrite(&Image.HEADER, sizeof(DDS_HEADER), 1, hDDS);

        if (Image.HEADER_DXT10.dxgiFormat)
        {
            fwrite(&Image.HEADER_DXT10, 0x14, 1, hDDS);
        }

        register 
            char next = 0, next1 = 0, next2 = 0;

        while (0 == IS_EOF(hTXST))
        {
            next = READU8(hTXST);
            if (next == 'D')
            {
                next1 = READU8(hTXST);
                if (next1 == 'D')
                {
                    char next2 = READU8(hTXST);
                    if (next2 == 'S')
                    {
                        int pos = GET_POSITION(hTXST);
                        SET_POSITION(pos - 3, hTXST);
                        break;
                    }
                    WRITEU8(next, hDDS);
                    WRITEU8(next1, hDDS);
                    WRITEU8(next2, hDDS);
                    continue;
                }
                WRITEU8(next, hDDS);
                WRITEU8(next1, hDDS);
                continue;
            }
            WRITEU8(next, hDDS);
        }

        CLOSE(hDDS);

        printf("%3u: %s\n", i, TEXTURENAME);
    }
    DESTROYTEXTURESET(&TXST);
    CLOSE(hTXST);

    puts("Finished parsing nxg_textures file.");
}
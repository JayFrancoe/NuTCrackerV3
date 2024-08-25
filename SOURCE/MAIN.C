#define _CRT_SECURE_NO_WARNINGS
#include <STRING.H>
#include "INCLUDE/DDS.H"
#include "INCLUDE/FILEIO.H"

#include "INCLUDE/NXG_TEXTURES.H"


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf(
            "Not enough arguments\n\n"
            "Usage:\n"
            "Drag and drop .NXG_TEXTURES file onto exe\n"
            "Or use commandline \n"
            "> NuTCracker.exe {Path-to-File} {Option -use nut/dds}\n"
            "\n"
            "Press any key to exit\n");
        EXIT(-1);
    }

    // GET ARG

    if (!FILE_EXISTS(argv[1]))
    {
        printf("Could not find the file %s\n", argv[1]);
        EXIT(-1);
    }

    PATHINFO Path = { 0,0 };

    GET_PATHINFO(&Path, argv[1]);

    puts(Path.FilePath);

    char HDRPATH[260];
    sprintf(HDRPATH, "%s/%s/%s.HDR",
        Path.FileDirectory, Path.FileName, Path.FileName);

    if (FILE_EXISTS(HDRPATH))
    {
        puts(
            "Detected directory.\n"
            "Do you want to compile a new nxg_textures file, or overwrite directory?\n"
            "[1] Compile Directory, [2] Overwrite Directory.");

        while (1)
        {
            char c = _getch();
            if (c & 48)
            {
                if ('1' == c)
                {
                    COMPILETEXTURES(&Path, HDRPATH);
                    break;
                }
                if ('2' == c)
                {
                    EXTRACTTEXTURES(&Path);
                    break;
                }
            }
        }

        EXIT(0);
    }

    EXTRACTTEXTURES(&Path);

    EXIT(0);
}
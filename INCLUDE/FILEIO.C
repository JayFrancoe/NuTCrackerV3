#include <STDARG.H>
#include <STRING.H>
#include <ASSERT.H>
#include <CTYPE.H>
#include <MEMORY.H>
#include "FILEIO.H"

/*----------------------------------------------------------*/
                           /* READ */

static U32   BIGENDIAN = 0;
static FILE *gfh;

DEFINE_READER(U64);
DEFINE_READER(U32);
DEFINE_READER(U16);
DEFINE_READER(U8);

DEFINE_READER(F64);
DEFINE_READER(F32);


#define MAXSTRTOLERANCE 280

void IGNORE(SIZE bytes, FILE *fh)
{
    fseek(fh, bytes, SEEK_CUR);
}

#define MAXREADUNTIL 400

void IGNORELINE(FILE *FH)
{
    int i = 0;
    while (i++ != MAXREADUNTIL)
    {
        char c = fgetc(FH);
        if (c == '\n')
            return;
    }
}

static const char *errorstr =
"\n"
" ___READER__RAN__INTO__AN__ERROR___\n"
"|                                  \n"
#ifdef _DEBUG
"|   DEBUG:                         \n"
"|   SOURCE FILE: %s\n"
"|   LINE NUMBER: %u                \n"
"|                                  \n"
#endif _DEBUG
"|   FILE OFFSET: 0x%08x            \n"
"|                                  \n"
"|   EXPRESSION:                    \n"
"|   \"%s\"                         \n"
"|                                  \n"
"|__________________________________\n"
"> ";

void ERROR(const char *Expression,
           const char *FILENAME, 
           int LINE)
{
SIZE Offset = (gfh) ? ftell(gfh) : 0;

#ifdef _DEBUG
    fprintf(stderr,
        errorstr, FILENAME, LINE, Offset, Expression);
#else
    fprintf(stderr,
        errorstr, Offset, Expression);
    (void)_getch();
#endif
    if (gfh) CLOSE(gfh);
}

void EXIT(int code)
{
    (void)_getch();
    exit(code);
}

void READBUFFERED(void *data, SIZE size, FILE *fh)
{
    SIZE r = fread(data, 1, size, fh);
    ASSERT(r == size);
}

static inline
FILE *_OPEN(const char *FILENAME, MODE ATTRIBUTES)
{
    if (FILENAME == NULL)
    {
        exit(1);
    }

    char FMODE[4] = {0,0,0,0};

    FMODE[0] = (ATTRIBUTES & F_READ)   ? 'r' : 'w';
    FMODE[0] = (ATTRIBUTES & F_WRITE)  ? 'w' : 'r';
    FMODE[0] = (ATTRIBUTES & F_APPEND) ? 'a' : FMODE[0];

    FMODE[1] = (ATTRIBUTES & F_TEXT) ? 0 : 'b';
    FMODE[1] = (ATTRIBUTES & F_UPDATE) ? '+' : FMODE[1];
    
    if (ATTRIBUTES & F_UPDATE
    &&  ATTRIBUTES & F_BINARY)
    {
        FMODE[1] = 'b';
        FMODE[2] = '+';
    }

    FILE *fh = fopen(FILENAME, FMODE);

    if (!fh)
    {
        printf("COULD NOT OPEN FILE %s\n"
            "Errno: %u", FILENAME, errno);
        (void)_getch();
        exit(1);
    }

    gfh = fh;

    return fh;
}

FILE *OPEN_READ(char *FILENAME, MODE ATTRIBUTES)
{
   return _OPEN(FILENAME, ATTRIBUTES | F_READ);
}

void CLOSE(FILE *FILEHANDLE)
{
    gfh = NULL;
    fclose(FILEHANDLE);
}

char *READSTRING(SIZE Size, FILE *fh)
{
    switch (Size)
    {
    case STRING0:
         Size = MAXSTRTOLERANCE;
         break;
    case STRING8:
        Size = READU8(fh);
        break;
    case STRING16:
        Size = READU16(fh);
        break;
    case STRING32:
        Size = READU32(fh);
        break;
    case STRING64:
        Size = READU64(fh);
        break;
    default:
        ASSERT(Size < MAXSTRTOLERANCE);
        break;
    }

    if (Size == 0)
        return NULL;

    char *ret = calloc(Size, 1);
    ASSERT(ret);

    if (Size != MAXSTRTOLERANCE)
    {
        fread(ret, Size, 1, fh);

        for (SIZE i = 0; i < Size; i++)
        {
            ASSERT(isascii(ret[i]));
        }
    }
    else
    {
        for (SIZE i = 0; i < Size; i++)
        {
            char cur = fgetc(fh);
            if (!cur)
                return ret;

            ASSERT(isascii(cur));
            ret[i] = cur;
        }
    }


    return ret;
}

int SCANFORMATTED(FILE *FH, int argCount, const char *Format, ...)
{
    va_list args;
    int scanresult = 0;
    
    va_start(args, Format);
    scanresult = vfscanf(FH, Format, args);
    va_end(args);

    ASSERT(scanresult == argCount);

    return scanresult;
}

/*-----------------------------------------------------------*/
                          /* WRITE */

DEFINE_WRITER(U64);
U32 WRITEU32(U32 data, FILE *fh) {
    U32 w = data;
    if (BIGENDIAN == 1) 
        switch (sizeof(data)) {
        case 2: w = _byteswap_ushort(data); break; 
        case 4: w = _byteswap_ulong(data); break; 
        case 8: w = _byteswap_uint64(data); break;
    }; 
    fwrite(&w, sizeof(U32), 1, fh); 
    return;
};
DEFINE_WRITER(U16);
DEFINE_WRITER(U8);

DEFINE_WRITER(F64);
DEFINE_WRITER(F32);

FILE *OPEN_WRITE(char *FILENAME, MODE ATTRIBUTES)
{
    return _OPEN(FILENAME, ATTRIBUTES |= F_WRITE);
}

void WRITELINE(FILE *FH, const char *STRING, ...)
{
    va_list args;
    va_start(args, STRING);

    vfprintf(FH, STRING, args);

    va_end(args);

    return;
}

void WRITESTRING(const char *STRING, MODE ATTRIBUTES, FILE *FH)
{
    SIZE sz = strlen(STRING) + 1;

    //if (BIGENDIAN) REVERSESTRING(STRING, sz);

    if (ATTRIBUTES == STRING0)
    {
        fwrite(STRING, 1, sz, FH);
        return;
    }

    switch (ATTRIBUTES)
    {
        case STRING8:
            WRITEU8(sz & UINT8_MAX, FH);
            break;
        case STRING16:
            WRITEU16(sz & UINT16_MAX, FH);
            break;
        case STRING32:
            WRITEU32(sz & UINT32_MAX, FH);
            break;
        case STRING64:
            WRITEU64(sz, FH);
        default:
            break;
    }

    fwrite(STRING, 1, sz, FH);
    return;
}

void WRITEFORMATTED(FILE *FH, const char *FORMAT, ...)
{
    va_list args;
    va_start(args, FORMAT);

    vfprintf(FH, FORMAT, args);

    va_end(args);
}

void WRITEFORMATTEDEX(FILE *FH, MODE ATTRIBUTES, const char *FORMAT, ...)
{
    va_list args;
    va_start(args,FORMAT);

    char buffer[1024];
    vsprintf(buffer, FORMAT, args);

    WRITESTRING(buffer, ATTRIBUTES, FH);
}

void WRITEBUFFERED(U8 *Data,
    SIZE size,
    FILE *fh)
{
    fwrite(Data, 1, size, fh);
}



/*-----------------------------------------------------------*/
                          /* DIRECTORY */

// AVOID INCLUDING WINDOWS.H
#ifdef _WIN32

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_IX86)
#define _X86_
#if !defined(_CHPE_X86_ARM64_) && defined(_M_HYBRID)
#define _CHPE_X86_ARM64_
#endif
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_AMD64)
#define _AMD64_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_ARM)
#define _ARM_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_ARM64)
#define _ARM64_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_M68K)
#define _68K_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_IA64_) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_MPPC)
#define _MPPC_
#endif

#if !defined(_68K_) && !defined(_MPPC_) && !defined(_X86_) && !defined(_M_IX86) && !defined(_AMD64_) && !defined(_ARM_) && !defined(_ARM64_) && defined(_M_IA64)
#if !defined(_IA64_)
#define _IA64_
#endif /* !_IA64_ */
#endif

#ifndef _MAC
#if defined(_68K_) || defined(_MPPC_)
#define _MAC
#endif
#endif

#include <fileapi.h>

void CREATE_DIRECTORY(const char *PATHNAME)
{
    CreateDirectoryA(PATHNAME, NULL);
}

U32 QUERY_DIRECTORY(const char *PATHNAME)
{
    U32 dwAttrib = GetFileAttributes(PATHNAME);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES 
        && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


#elif __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd>

U32 CREATE_DIRECTORY(const char *PATHNAME)
{
    return mkdir(PATHNAME);
}

#endif // _WIN32
/*-----------------------------------------------------------*/
                          /* UTIL */

void GET_PATHINFO(PATHINFO *fi, const char *Path)
{
    ASSERT(Path != NULL);

    SIZE i = 0,
         j = 0,
         k = 0;

    for (; Path[i]; i++)
    {
        ASSERT(i < MAX_PATH);
        if (j >= MAX_NAME)
            j = 0;
        if (k >= MAX_EXTENSION)
            k = 0;


        fi->FilePath[i]        = Path[i];
        fi->FileDirectory[i]   = Path[i];
        fi->FileName[j]        = Path[i];
        fi->FileExtension[k]   = Path[i];

        j++;
        k++;

        if (Path[i] == '\\' 
        ||  Path[i] == '/')
        {
            fi->FilePath[i] = '/';
            fi->FileDirectory[i] = '/';
            j = 0;
        }
        if (Path[i] == '.')
        {
            k = 0;
        }
    }

    fi->FileName[j - k - 1] = '\0';
    fi->FileExtension[k] = '\0';
    fi->FilePath[i] = '\0';

    while (i --> 0)
    {
        if (fi->FilePath[i] == '/')
        {
            fi->FileDirectory[i] = '\0';
            break;
        }
    }
}

void GET_FOLDER(const char *file_path, char *directory_name)
{
    // Find the last occurrence of '/'
    const char* last_slash = strrchr(file_path, '/');

    // If no slash is found, there is no directory in the path
    if (last_slash == NULL) {
        strcpy(directory_name, "");
        return;
    }

    // Find the second-to-last occurrence of '/'
    const char* second_last_slash = strrchr(file_path, '/');

    // Walk back from the last slash to find the second-to-last slash
    const char* current = last_slash - 1;
    while (current > file_path) {
        if (*current == '/') {
            second_last_slash = current;
            break;
        }
        current--;
    }

    // If no second-to-last slash is found, use the start of the string
    if (second_last_slash == NULL) {
        second_last_slash = file_path - 1; // Before the start of the string
    }

    if (last_slash == second_last_slash)
    {
        strcpy(directory_name, "");
        return;
    }

    // Extract the directory name between the second-to-last and last slash
    size_t directory_length = last_slash - second_last_slash - 1;
    strncpy(directory_name, second_last_slash + 1, directory_length);
    directory_name[directory_length] = '\0'; // Null-terminate the string
}

SIZE GET_POSITION(FILE *fh)
{
    return ftell(fh);
}

void SET_POSITION(U32 OFFSET, FILE *fh)
{
    fseek(fh, OFFSET, SEEK_SET);
}

SIZE GET_SIZE(FILE *fh)
{
    SIZE offset = GET_POSITION(fh);

    fseek(fh, 0, SEEK_END);

    SIZE ret = GET_POSITION(fh);

    SET_POSITION(offset, fh);

    return ret;
}

U32 IS_EOF(FILE *fh)
{
    return feof(fh);
}

void SETBIGENDIAN(U32 truefalse)
{
    BIGENDIAN = (truefalse != 0);
}

int FILE_EXISTS(const char *NAME)
{
    FILE *tmp = fopen(NAME, "r");
    if (tmp)
    {
        fclose(tmp);
        return 1;
    }
    return 0;
}

void _STRBIN(SIZE NUMBER, SIZE LEN, SIZE TRIM, char *BUFFER)
{
    for (SIZE i = 0; i < TRIM; i++)
    {
        BUFFER[TRIM - i - 1] = '0' + (NUMBER >> i & 1);
    }

    BUFFER[TRIM] = '\0';

    return BUFFER;
}
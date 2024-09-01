#pragma once
#include <CONIO.H>
#include <STDINT.H>
#include <STDIO.H>
#include <STDLIB.H>

#define FILIO_VERSION 2.0f

#ifdef _DEBUG
    #define ASSERT(statement) \
    if (!(statement)) \
    { \
        ERROR(#statement, __FILE__, __LINE__); \
        (void)_getch(); \
        EXIT(0); \
    }
#else
    #define ASSERT(statement) \
    if (!(statement)) \
    { \
    ERROR(#statement, NULL, 0); \
    EXIT(0); \
}

#endif

typedef size_t   SIZE;
typedef SIZE     MODE;
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t  U8;

typedef float  F32;
typedef double F64;

typedef struct PATHINFO_s PATHINFO;

#define ALLOC(T, Name) T *Name = (T*)calloc(sizeof(T), 1); ASSERT(Name);

#define STRBIN(name, num, trim) \
char name[sizeof(num) + 4] = {"0"};\
_STRBIN(num, sizeof(num) + 4, trim, name);


#define F_TEXT         0x00000004
#define F_BINARY       0x00000008
#define F_WRITE        0x00000010
#define F_APPEND       0x00000020
#define F_READ         0x00000040
#define F_UPDATE       0x00000080
#define F_DEFAULT      F_LITTLEENDIAN | F_BINARY

#define STRING0  0x80000000
#define STRING8  0x80000001
#define STRING16 0x80000002
#define STRING32 0x80000004
#define STRING64 0x80000008

#define DECLARE_READER(TYPE) \
TYPE READ##TYPE##(FILE *fh);

#define DECLARE_WRITER(TYPE) \
TYPE WRITE##TYPE##(TYPE data, FILE *fh);


#define DEFINE_BYTESWAP(r) \
switch (sizeof(r)) \
{    \
    case 2:  \
        r = _byteswap_ushort(r); \
        break;\
    case 4:  \
        r = _byteswap_ulong(r);  \
        break;\
    case 8:  \
        r = _byteswap_uint64(r); \
    break;\
case 1: \
    break; \
}

#define DEFINE_READER(TYPE) \
TYPE READ##TYPE##(FILE *fh) \
{ TYPE ret = 0; \
fread_s(&ret, sizeof(TYPE), sizeof(TYPE), 1, fh);\
if (BIGENDIAN == 0) return ret;\
    DEFINE_BYTESWAP(ret); return ret;\
}

#define DEFINE_WRITER(TYPE) \
TYPE WRITE##TYPE##(TYPE data, FILE *fh) \
{ \
if (BIGENDIAN == 1) DEFINE_BYTESWAP(data); \
fwrite(&data, sizeof(TYPE), 1, fh);\
return; \
}

DECLARE_READER(U64);
DECLARE_READER(U32);
DECLARE_READER(U16);
DECLARE_READER(U8);

DECLARE_READER(F32);
DECLARE_READER(F64);

DECLARE_WRITER(U64);
DECLARE_WRITER(U32);
DECLARE_WRITER(U16);
DECLARE_WRITER(U8);

DECLARE_WRITER(F32);
DECLARE_WRITER(F64);

#define READ4CC(c, fh)\
{\
    char FourCC[4] = {0,0,0,0}; \
    *(U32 *)FourCC = READU32(fh); \
    ASSERT(FourCC[0] == c[0]); \
    ASSERT(FourCC[1] == c[1]); \
    ASSERT(FourCC[2] == c[2]); \
    ASSERT(FourCC[3] == c[3]); \
} \


#define MAX_EXTENSION 25
#define MAX_NAME 80
#define MAX_PATH 260

struct PATHINFO_s
{
    char FileExtension[MAX_EXTENSION];
    char FileName[MAX_NAME];
    char FileDirectory[MAX_PATH];
    char FilePath[MAX_PATH];
};


/*----------------------------------------------------------*/
                           /* READ */

FILE *OPEN_READ(char *FILENAME, MODE ATTRIBUTES);
void  CLOSE(FILE *FILEHANDLE);
void  READBUFFERED(void *DATA, SIZE SIZE, FILE *FH);
char *READSTRING(SIZE SIZE, FILE *FH);
int   SCANFORMATTED(FILE *FH, int ARGS, const char *Format, ...);
void  IGNORE(SIZE BYTES, FILE *FH);
void  IGNORELINE(FILE *FH);
void  ERROR(const char *EXPRESSION, 
            const char *FILENAME, 
            int LINE);
void  EXIT(int CODE);
             
/*-----------------------------------------------------------*/
                          /* DIRECTORY */

void  CREATE_DIRECTORY(const char *PATHNAME);
U32   QUERY_DIRECTORY(const char *PATHNAME);

/*-----------------------------------------------------------*/
                          /* WRITE */

FILE *OPEN_WRITE(char *PATH, MODE ATTRIBUTES);
void  WRITELINE(FILE *FH, const char *STRING, ...);
void  WRITESTRING(const char *STRING, MODE ATTRIBUTES, FILE *FH);
void  WRITEFORMATTED(FILE *FH, const char *FORMAT,  ...);
void  WRITEFORMATTEDEX(FILE *FH, MODE ATTRIBUTES, const char *FORMAT, ...);
void  WRITEBUFFERED(U8 *DATA, SIZE SIZE, FILE *FH);

/*-----------------------------------------------------------*/
                          /* UTIL */
void  SETBIGENDIAN(U32 truefalse);
int   FILE_EXISTS(const char *NAME);
void  _STRBIN(SIZE NUMBER, SIZE LEN, SIZE TRIM, char *BUFFER);
void  GET_PATHINFO(PATHINFO *FI, const char *PATH);
void  GET_FOLDER(const char *FILE, char *BUFFER);
SIZE  GET_POSITION(FILE *fh);
void  SET_POSITION(U32 OFFSET, FILE *fh);
SIZE  GET_SIZE(FILE *fh);
U32   IS_EOF(FILE *fh);
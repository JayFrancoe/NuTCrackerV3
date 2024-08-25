#pragma once
#include "INCLUDE/FILEIO.H"

typedef struct  IMAGE_DDS_s			IMAGE_DDS;
typedef struct  PixelBlock_s        PixelBlock;
typedef struct  DDS_PIXELFORMAT_s   DDS_PIXELFORMAT;
typedef struct  DDS_HEADER_s        DDS_HEADER;
typedef struct  DDS_HEADER_DXT10_s  DDS_HEADER_DXT10;
typedef enum    DDSCompression_e    DDSCompression;
typedef enum    DXGI_FORMAT_e		DXGI_FORMAT;

void WriteDDS(const IMAGE_DDS *DDSIMAGE, const char *PATH);

int ReadDDSFile(FILE *fh, IMAGE_DDS *DDS);


enum DDSCompression_e
{
	DXT1 = 1,
	DXT5 = 6,
	DX10 = 31,
};

struct PixelBlock_s {
	U16 colors[2];
	U32 lookup;
};

struct DDS_PIXELFORMAT_s {
	U32  dwSize;
	U32  dwFlags;
	U32  dwFourCC;
	U32  dwRGBBitCount;
	U32  dwRBitMask;
	U32  dwGBitMask;
	U32  dwBBitMask;
	U32  dwABitMask;
};

struct DDS_HEADER_s {
	U32             dwSize;
	U32             dwFlags;
	U32             dwHeight;
	U32             dwWidth;
	U32             dwPitchOrLinearSize;
	U32             dwDepth;
	U32             dwMipMapCount;
	U32             dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	U32             dwCaps;
	U32             dwCaps2;
	U32             dwCaps3;
	U32             dwCaps4;
	U32             dwReserved2;
};


enum DXGI_FORMAT_e {
	DXGI_FORMAT_BC1_UNORM = 71,
	DXGI_FORMAT_BC2_UNORM = 74,
	DXGI_FORMAT_BC3_UNORM = 77,
	DXGI_FORMAT_BC4_UNORM = 80,
	DXGI_FORMAT_BC5_UNORM = 83,
	DXGI_FORMAT_BC6H_UF16 = 95,
	DXGI_FORMAT_BC7_UNORM = 98,
};

struct DDS_HEADER_DXT10_s {
	DXGI_FORMAT             dxgiFormat;
	U32						resourceDimension;
	U32                     miscFlag;
	U32                     arraySize;
	U32                     miscFlags2;
};


struct IMAGE_DDS_s
{
	DDS_HEADER       HEADER;
	DDS_HEADER_DXT10 HEADER_DXT10;
	U8  *IMAGEDATA;
	SIZE SIZE;
};
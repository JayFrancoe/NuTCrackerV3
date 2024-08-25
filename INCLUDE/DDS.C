#include "DDS.H"

void WriteDDS(const IMAGE_DDS *DDSIMAGE, const char *PATH)
{
    FILE *hDDS = OPEN_WRITE(PATH, F_BINARY);

    fwrite("DDS ", 4, 1, hDDS);

    fwrite(&DDSIMAGE->HEADER, sizeof(DDS_HEADER), 1, hDDS);
    
    if (DDSIMAGE->HEADER_DXT10.dxgiFormat)
    {
        fwrite(&DDSIMAGE->HEADER_DXT10, 0x14, 1, hDDS);
    }

    fwrite(DDSIMAGE->IMAGEDATA, DDSIMAGE->SIZE, 1, hDDS);

    CLOSE(hDDS);

    free(DDSIMAGE->IMAGEDATA);

    return;
}

int ReadDDSFile(FILE *fh, IMAGE_DDS *DDS)
{
    DDS_HEADER       *HDR   = &DDS->HEADER;
    DDS_HEADER_DXT10 *HDR10 = &DDS->HEADER_DXT10;

    fread(HDR, 1, 0x7C, fh);
    
    SIZE blockSize = 0;

    U32 fcc = _byteswap_ulong(HDR->ddspf.dwFourCC);
    switch (fcc & 0x000000FF)
    {
        case '0':
            ASSERT('1' != (fcc >> 8)& 0xFF);

            fread(HDR10, 1, 0x14, fh);
            switch (HDR10->dxgiFormat)
            {
                case DXGI_FORMAT_BC1_UNORM:
                    blockSize = 8;
                    break;
                case DXGI_FORMAT_BC2_UNORM:
                case DXGI_FORMAT_BC3_UNORM:
                case DXGI_FORMAT_BC4_UNORM:
                case DXGI_FORMAT_BC5_UNORM:
                case DXGI_FORMAT_BC6H_UF16:
                case DXGI_FORMAT_BC7_UNORM:
                    blockSize = 16;
                    break;
                default:
                    return 1;
            }
            break;
        case 'A':
            HDR->ddspf.dwFourCC = '1TXD'; // TT GAMES
        case '1':
        case '2':
            blockSize = 8;
            break;
        case '3':
        case '5':
            blockSize = 16;
            break;
        default:
            return 1;
    }

    SIZE size = 0,
        width = HDR->dwWidth,
        height = HDR->dwHeight,
        depth = (HDR->dwDepth > 0) ? HDR->dwDepth : 1,
        mipMapCount = (HDR->dwMipMapCount > 0) ? HDR->dwMipMapCount : 1;

    U32 IsCubemap = HDR->dwCaps2 & 0xFE00;

    SIZE faces = (IsCubemap) ? 6 : 1;

    for (SIZE face = 0; face < faces; face++)
    {
        SIZE currentWidth = width,
            currentHeight = height,
            currentDepth = depth;

        for (SIZE mip = 0; mip < mipMapCount; mip++)
        {
            size += ((currentWidth + 3) / 4)
                * ((currentHeight + 3) / 4)
                * blockSize
                * currentDepth;

            currentWidth = (currentWidth > 1) ? currentWidth / 2 : 1;
            currentHeight = (currentHeight > 1) ? currentHeight / 2 : 1;
            currentDepth = (currentDepth > 1) ? currentDepth / 2 : 1;
        }
    }

    DDS->SIZE = size;

    DDS->IMAGEDATA = calloc(size, 1);
    ASSERT(DDS->IMAGEDATA);

    fread(DDS->IMAGEDATA, 1, size, fh);

    return 0;
}
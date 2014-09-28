
#include "cs499rImage.hpp"
#include "../libs/libpng/png.h"


namespace CS499R
{

    size_t
    Image::saveToPngFile(char const * path) const
    {
        CS499R_ASSERT(mTexels != nullptr);

        FILE * file = fopen(path, "wb");

        if (file == nullptr)
        {
            return 1;
        }

        auto pngWrite = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL, NULL, NULL);

        CS499R_ASSERT(pngWrite);

        auto pngInfo = png_create_info_struct(pngWrite);

        CS499R_ASSERT(pngInfo);

        if (setjmp(png_jmpbuf(pngWrite)))
        {
            CS499R_CRASH();
        }

        png_init_io(pngWrite, file);

        switch (mChanels)
        {
        case 4:
            png_set_IHDR(pngWrite, pngInfo, mWidth, mHeight, mDepth, PNG_COLOR_TYPE_RGBA,
                         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            break;

        case 3:
            png_set_IHDR(pngWrite, pngInfo, mWidth, mHeight, mDepth, PNG_COLOR_TYPE_RGB,
                         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            break;

        case 2:
            png_set_IHDR(pngWrite, pngInfo, mWidth, mHeight, mDepth, PNG_COLOR_TYPE_GRAY_ALPHA,
                         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            break;

        case 1:
            png_set_IHDR(pngWrite, pngInfo, mWidth, mHeight, mDepth, PNG_COLOR_TYPE_GRAY,
                         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
            break;

        default:
            CS499R_CRASH();
            return 1;
        }

        auto rowPointers = new png_byte * [mHeight];

        size_t stride = mWidth * mChanels;
        png_byte * tmpTexels = nullptr;

        if (mDepth == 16)
        {
            stride *= 2;

            size_t size = mWidth * mHeight * mChanels;

            tmpTexels = (png_byte *) malloc(size * 2);

            for (size_t i = 0 ; i < size ; i ++)
            {
                tmpTexels[2 * i] = ((png_byte *) mTexels)[2 * i + 1] ;
                tmpTexels[2 * i + 1] = ((png_byte *) mTexels)[2 * i] ;
            }

            for (size_t i = 0 ; i < mHeight ; i++)
            {
                rowPointers[i] = (png_byte *) (tmpTexels + (mHeight - 1 - i) * stride);
            }
        }
        else if (mDepth == 8)
        {
            for (size_t i = 0 ; i < mHeight ; i++)
            {
                rowPointers[i] = ((png_byte *) mTexels) + (mHeight - 1 - i) * stride;
            }
        }
        else
        {
            CS499R_CRASH();
        }

        png_set_rows(pngWrite, pngInfo, rowPointers);
        png_write_png(pngWrite, pngInfo, 0, NULL);
        png_write_end(pngWrite, pngInfo);
        png_destroy_write_struct(&pngWrite, &pngInfo);

        if (tmpTexels)
        {
            free(tmpTexels);
        }

        delete [] rowPointers;

        fclose(file);

        return 0;
    }

}

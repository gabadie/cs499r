
#include <string.h>
#include "cs499rImage.hpp"


namespace CS499R
{

    size_t
    Image::saveToFile(char const * path) const
    {
        return saveToPngFile(path);
    }

    Image::Image(size_t width, size_t height, size_t chanels, size_t depth)
    {
        mTexels = nullptr;

        allocTexels(width, height, chanels, depth);

        memset(mTexels, 0, memorySize());
    }

    Image::~Image()
    {
        if (mTexels != nullptr)
        {
            freeTexels();
        }
    }

    void
    Image::allocTexels(size_t width, size_t height, size_t chanels, size_t depth)
    {
        CS499R_ASSERT(mTexels == nullptr);
        CS499R_ASSERT(chanels >= 1);
        CS499R_ASSERT(chanels <= 4);
        CS499R_ASSERT((depth % 8) == 0);

        mWidth = width;
        mHeight = height;
        mChanels = chanels;
        mDepth = depth;
        mTexels = (void *) this;
        mTexels = malloc(memorySize());

        CS499R_ASSERT(mTexels != nullptr);
    }

    void
    Image::freeTexels()
    {
        CS499R_ASSERT(mTexels != nullptr);

        free(mTexels);

        mTexels = nullptr;
    }

}


#ifndef _H_CS499R_IMAGE
#define _H_CS499R_IMAGE

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * Contains a image on the CPU side
     */
    class Image
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        /*
         * compute the texels' memory size
         */
        size_t
        memorySize() const
        {
            CS499R_ASSERT(mTexels != nullptr);

            return mWidth * mHeight * mChanels * ((mDepth + 7) / 8);
        }

        /*
         * Save texel to a file
         */
        size_t
        saveToFile(char const * path) const;

        /*
         * Getters
         */
        inline
        size_t
        width() const
        {
            return mWidth;
        }

        inline
        size_t
        height() const
        {
            return mHeight;
        }

        inline
        size_t
        chanels() const
        {
            return mChanels;
        }

        inline
        size_t
        depth() const
        {
            return mDepth;
        }

        template <typename T>
        inline
        T *
        texels()
        {
            return (T *) mTexels;
        }

        template <typename T>
        inline
        T const *
        texels() const
        {
            return (T *) mTexels;
        }


        // --------------------------------------------------------------------- IDLE

        Image(size_t width, size_t height, size_t chanels = 4, size_t depth = 8);
        ~Image();

        Image(Image const &) = delete;
        Image & operator = (Image const &) = delete;


    private:
        // --------------------------------------------------------------------- MEMBERS

        size_t mWidth;
        size_t mHeight;
        size_t mChanels;
        size_t mDepth;
        void * mTexels;


        // --------------------------------------------------------------------- PRIVATE METHODS

        /*
         * allocates new texels
         */
        void
        allocTexels(size_t width, size_t height, size_t chanels, size_t depth);

        /*
         * frees previously allocated texels
         */
        void
        freeTexels();

        /*
         * Saves to a PNG file
         */
        size_t
        saveToPngFile(char const * path) const;

    };

}

#endif // _H_CS499R_IMAGE

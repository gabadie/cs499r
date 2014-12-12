
#ifndef _H_CS499R_RENDERABSTRACTTRACKER
#define _H_CS499R_RENDERABSTRACTTRACKER

#include "cs499rRenderShotCtx.hpp"


namespace CS499R
{

    /*
     * The render abstract tracker is an interface to process every single
     * render events
     */
    class RenderAbstractTracker
    {
    protected:
        // --------------------------------------------------------------------- ENTRIES

        /*
         * Events triggered when the shot is starting up
         */
        virtual
        void
        eventShotStart(RenderShotCtx const * ctx) = 0;

        /*
         * Events triggered when the shot is ending up
         */
        virtual
        void
        eventShotEnd() = 0;

        /*
         * Events triggered every at every current++
         */
        virtual
        void
        eventShotProgress(size_t current, size_t total) = 0;


        // --------------------------------------------------------------------- FRIENDSHIPS

        friend class RenderState;

    };


}

#endif // _H_CS499R_RENDERABSTRACTTRACKER

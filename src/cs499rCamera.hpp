
#ifndef _H_CS499R_CAMERA
#define _H_CS499R_CAMERA

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * The camera is the mathematic concept to shot a scene from
     */
    class Camera
    {
    public:
        // --------------------------------------------------------------------- METHODES


        // --------------------------------------------------------------------- IDLE

        Camera();


        // --------------------------------------------------------------------- MEMBERS

        // the camera position
        float3 mPosition;

        // the focus position
        float3 mFocusPosition;

        // the field of view
        float mViewField;

        // the shot diagonal's length
        float mShotDiagonalLength;

        /*
         * Figure 1: Shot plan
         *
         *      \
         *         \
         *            M-----------------------A
         *            |  \                    |
         *            |     \                 |
         *            |        \              |
         *            |           P           |
         *            |              \        |
         *            |                 \     |
         *            |                    \  |
         *            B-----------------------N
         *                                       \
         *                                          \
         *                                     (diagonal plan)
         *
         *
         * Figure 2: Diagonal plan
         *                                           |
         *          |           /
         *          |        /                       |
         *          |     /
         *          |  /                             |
         *          M
         *       /  |                                |
         *    /     |
         * O  -  -  P  -  -  -  -  -  -  -  -  -  -  F
         *    \     |
         *       \  |                                |
         *          N
         *          |  \                             |
         *          |     \
         *          |        \                       |
         *          |           \
         *     (shot plan)                           |
         *                                      (focus plan)
         *
         *
         * Glossary:
         *      Point P is the camera's shot area center (mPosition)
         *      Point F is the camera's focus position (mFocusPosition)
         *      Point O is the matematical frustum cone's apex
         *      Angle MOP is the field of view of the camera (mViewField)
         *      Distance MN is the shot diagonal (mShotDiagonalLength)
         *      Vector P->F is the camera shot direction
         *
         *
         * Acknowledgements:
         *      Angle MOP == PON
         *      MP == PN
         *      MANB is a rectangle
         *      {M, P, N} ar aligned
         *      {O, P, F} ar aligned
         *      diagonal plan is defined by {O, M, F}
         *      shot plan is defined by {M, P, A}
         *      shot plan and focus plan are parrallel
         */


    };

}

#endif // _H_CS499R_SCENE

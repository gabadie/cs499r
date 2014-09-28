
#include "cs499rCamera.hpp"


namespace CS499R
{

    void
    Camera::exportToShotCamera(common_camera_t * outGpuCamera, float aspectRatio) const
    {
        float32x3_t direction = mFocusPosition - mShotPosition;
        float32_t directionLength = length(direction);
        float32x3_t directionNormalized = direction * (1.0f / directionLength);
        float32x3_t u = normalize(cross(directionNormalized, float32x3_t(0.0f, 0.0f, 1.0f)));
        float32x3_t v = cross(u, directionNormalized);

        float32_t alpha = atanf(1.0f / aspectRatio);
        float32_t cosAlpha = cos(alpha) * 0.5f;
        float32_t sinAlpha = sin(alpha) * 0.5f;

        float32_t focusDiagonalLength = mShotDiagonalLength + sin(mViewField) * directionLength;

        outGpuCamera->shotPosition = mShotPosition;
        outGpuCamera->shotBasisU = u * (cosAlpha * mShotDiagonalLength);
        outGpuCamera->shotBasisV = v * (sinAlpha * mShotDiagonalLength);
        outGpuCamera->focusPosition = mFocusPosition;
        outGpuCamera->focusBasisU = u * (cosAlpha * focusDiagonalLength);
        outGpuCamera->focusBasisV = v * (sinAlpha * focusDiagonalLength);
    }

    Camera::Camera()
    {
        mShotPosition = float32x3_t(-1.0f, -1.0f, -1.0f);
        mFocusPosition = float32x3_t(0.0f, 0.0f, 0.0f);
        mViewField = kPI * 0.33f;
        mShotDiagonalLength = 0.02;
    }

}

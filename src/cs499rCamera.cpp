
#include "cs499rCamera.hpp"


namespace CS499R
{

    void
    Camera::exportToShotCamera(common_camera_t * outGpuCamera, float aspectRatio) const
    {
        float3 direction = mFocusPosition - mShotPosition;
        float directionLength = length(direction);
        float3 directionNormalized = direction * (1.0f / directionLength);
        float3 u = normalize(cross(directionNormalized, float3(0.0f, 0.0f, 1.0f)));
        float3 v = cross(u, directionNormalized);

        float alpha = atanf(1.0f / aspectRatio);
        float cosAlpha = cos(alpha);
        float sinAlpha = sin(alpha);

        float focusDiagonalLength = mShotDiagonalLength + sin(mViewField) * directionLength;

        outGpuCamera->shotPosition = mShotPosition;
        outGpuCamera->shotBasisU = u * (cosAlpha * mShotDiagonalLength);
        outGpuCamera->shotBasisV = v * (sinAlpha * mShotDiagonalLength);
        outGpuCamera->focusPosition = mFocusPosition;
        outGpuCamera->focusBasisU = u * (cosAlpha * focusDiagonalLength);
        outGpuCamera->focusBasisV = v * (sinAlpha * focusDiagonalLength);
    }

    Camera::Camera()
    {
        mShotPosition = float3 { -1.0f, -1.0f, -1.0f };
        mFocusPosition = float3 { 0.0f, 0.0f, 0.0f };
        mViewField = kPI * 0.33f;
        mShotDiagonalLength = 0.02;
    }

}

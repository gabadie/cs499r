
#ifndef _H_APP
#define _H_APP

#include "cs499r.hpp"


namespace App
{

    /*
     * Chooses the OpenCL device from its name
     */
    int
    chooseDevice(char const * selectedDeviceName, cl_device_id * deviceId);

    /*
     * Builds the application's scene
     */
    void
    buildSceneMeshes(CS499R::Scene & scene);


}


#endif // _H_APP

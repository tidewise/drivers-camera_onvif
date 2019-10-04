#include "CameraOnvif.hpp"
#include <iostream>

int main()
{
    camera_onvif::CameraOnvif camera = camera_onvif::CameraOnvif();

    std::cout << camera.readBrightness() << std::endl;
    std::cout << camera.readContrast() << std::endl;
    std::cout << camera.readColorSaturation() << std::endl;

    return 0;
}


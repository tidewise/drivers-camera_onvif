#include "CameraOnvif.hpp"
#include <iostream>
#include<unistd.h>

int main()
{
    camera_onvif::CameraOnvif camera = camera_onvif::CameraOnvif();

    // std::cout << camera.readBrightness() << std::endl;
    // std::cout << camera.readContrast() << std::endl;
    // std::cout << camera.readColorSaturation() << std::endl;

    camera.setBrightness(0.1);
    camera.setContrast(0.1);
    camera.setColorSaturation(0.1);

    sleep(3);

    camera.setBrightness(1.0);
    camera.setContrast(1.0);
    camera.setColorSaturation(1.0);

    sleep(3);

    camera.setBrightness(0.5);
    camera.setContrast(0.5);
    camera.setColorSaturation(0.5);

    return 0;
}


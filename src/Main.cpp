#include "CameraOnvif.hpp"
#include <iostream>
#include<unistd.h>

int main()
{
    camera_onvif::CameraOnvif camera = camera_onvif::CameraOnvif(
        "admin", "camera01", "10.20.0.188", 1920, 1080);

    // std::cout << camera.readBrightness() << std::endl;
    // std::cout << camera.readContrast() << std::endl;
    // std::cout << camera.readColorSaturation() << std::endl;

    camera.setImageParam(0.1, 0.1, 0.1);

    sleep(3);

    camera.setImageParam(0.9, 0.9, 0.9);

    sleep(3);

    camera.setImageParam(0.5, 0.5, 0.5);

    return 0;
}


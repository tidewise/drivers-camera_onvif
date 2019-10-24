#include "CameraOnvif.hpp"
#include <iostream>
#include<unistd.h>

int main()
{
    camera_onvif::CameraOnvif camera = camera_onvif::CameraOnvif(
        "admin", "camera01", "10.20.0.188");

    camera.setResolution(1920, 1080);

    auto params = camera_onvif::ImageParam();

    params.contrast = 0.1;
    params.brightness = 0.1;
    params.color_saturation = 0.1;
    camera.setImageParam(params);

    sleep(3);

    params.contrast = 0.9;
    params.brightness = 0.9;
    params.color_saturation = 0.9;
    camera.setImageParam(params);

    sleep(3);

    params.contrast = 0.5;
    params.brightness = 0.5;
    params.color_saturation = 0.5;
    camera.setImageParam(params);

    return 0;
}


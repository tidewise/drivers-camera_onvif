#ifndef CAMERA_ONVIF_CAMERAONVIF_HPP
#define CAMERA_ONVIF_CAMERAONVIF_HPP

#include <string>

namespace camera_onvif {
    /**
     *
     */
    class CameraOnvif {
        struct Private;
        Private* m_private = nullptr;

        std::string user;
        std::string pass;
        std::string uri;
        std::string video_token;

        float brightness[2]; // [Min, Max]
        float color_saturation[2]; // [Min, Max]
        float contrast[2]; // [Min, Max]

        void setCredentials();
        void reportError();

    public:
        CameraOnvif(/* args */);
        ~CameraOnvif();

        void printCameraInfo();

        float readBrightness();
        float readColorSaturation();
        float readContrast();

        void setBrighttness();
        void setColorSaturation();
        void setContrast();
    };
}

#endif
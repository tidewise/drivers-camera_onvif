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
        std::string video_conf_token;

        float brightness[2]; // [Min, Max]
        float color_saturation[2]; // [Min, Max]
        float contrast[2]; // [Min, Max]

        void setCredentials();
        void reportError();
        void init();

    public:
        CameraOnvif(/* args */);
        CameraOnvif(std::string user, std::string pass, std::string ip);
        CameraOnvif(std::string user, std::string pass, std::string ip,
                    int width, int height);
        ~CameraOnvif();

        void printCameraInfo();

        float readBrightness();
        float readColorSaturation();
        float readContrast();

        void setBrightness(float percentage);
        void setColorSaturation(float percentage);
        void setContrast(float percentage);
        void setResolution(int width, int height);
    };
}

#endif
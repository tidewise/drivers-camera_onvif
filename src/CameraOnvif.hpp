#ifndef CAMERA_ONVIF_CAMERAONVIF_HPP
#define CAMERA_ONVIF_CAMERAONVIF_HPP

#include <string>

namespace camera_onvif {
    /**
     *
     */
    class CameraOnvif {
        // The goal of Private is to avoid leaking the whole SOAP generated code
        struct Private;
        Private* m_private = nullptr;

        std::string m_user;
        std::string m_pass;
        std::string m_uri;
        std::string m_video_token;
        std::string m_video_conf_token;

        float m_brightness[2]; // [Min, Max]
        float m_color_saturation[2]; // [Min, Max]
        float m_contrast[2]; // [Min, Max]

        void setCredentials();
        void reportError();
        void init();

    public:
        CameraOnvif();
        CameraOnvif(const std::string& user, const std::string& pass,
                    const std::string& ip);
        CameraOnvif(const std::string& user, const std::string& pass,
                    const std::string& ip, int width, int height);
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
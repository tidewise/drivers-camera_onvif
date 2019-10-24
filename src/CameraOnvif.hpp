#ifndef CAMERA_ONVIF_CAMERAONVIF_HPP
#define CAMERA_ONVIF_CAMERAONVIF_HPP

#include <string>

class _trt__GetVideoEncoderConfigurationResponse;

namespace camera_onvif {
    struct Resolution {
        int width;
        int height;
        Resolution() = default;
        ~Resolution() = default;
    };

    /**
     * Class to handle the Onvif connection.
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

        // [Min, Max]
        float m_brightness[2];
        // [Min, Max]
        float m_color_saturation[2];
        // [Min, Max]
        float m_contrast[2];

        void setCredentials();
        // to report an error
        void reportError();
        void init();
        void getVideoEncoderConfiguration(_trt__GetVideoEncoderConfigurationResponse& resp);

    public:
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

        Resolution getResolution();
    };
}

#endif
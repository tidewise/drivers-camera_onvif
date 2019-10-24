#include "CameraOnvif.hpp"
#include "SOAPException.hpp"

#include <gsoap/plugin/wsseapi.h>
#include <gsoap/plugin/wsddapi.h>
#include "onvif/soapDeviceBindingProxy.h"
#include "onvif/soapImagingBindingProxy.h"
#include "onvif/soapMediaBindingProxy.h"
#include "onvif/soapPTZBindingProxy.h"
#include "onvif/soapPullPointSubscriptionBindingProxy.h"
#include "onvif/soapRemoteDiscoveryBindingProxy.h"
#include "wsdd.nsmap"

#include <iostream>
#include <sstream>

using namespace std;
using namespace camera_onvif;

struct CameraOnvif::Private {
    struct soap *soap = soap_new1(SOAP_XML_STRICT | SOAP_XML_CANONICAL | SOAP_C_UTFSTRING);
    MediaBindingProxy *proxy_media = nullptr;
    ImagingBindingProxy *proxy_image = nullptr;
    _trt__GetVideoEncoderConfigurationOptionsResponse video_options;
    Private() = default;
    ~Private(){
        /*
         * Free all deserialized and managed data, we can still reuse the context
         * and proxies after this.
         */
        soap_destroy(soap);
        soap_end(soap);

        // free the shared context, proxy classes must terminate as well after this
        soap_free(soap);

        delete proxy_image;
        delete proxy_media;
    }
};

CameraOnvif::CameraOnvif(const string& user, const string& pass, const string& ip) :
 m_user(user), m_pass(pass), m_uri("http://" + ip + "/onvif/device_service"), m_timeout(10) {

    this->init();
}

void CameraOnvif::init(){
    m_private = new Private();
    m_private->soap->connect_timeout = m_timeout;
    m_private->soap->recv_timeout = m_timeout;
    m_private->soap->send_timeout = m_timeout;
    soap_register_plugin(m_private->soap, soap_wsse);

    m_private->proxy_media = new MediaBindingProxy(m_private->soap);
    m_private->proxy_media->soap_endpoint = m_uri.c_str();
    m_private->proxy_image = new ImagingBindingProxy(m_private->soap);
    m_private->proxy_image->soap_endpoint = m_uri.c_str();

    _trt__GetVideoSources get_video_src;
    _trt__GetVideoSourcesResponse get_video_src_resp;
    _trt__GetVideoSourceConfigurations get_video_src_conf;
    _trt__GetVideoSourceConfigurationsResponse get_video_src_conf_resp;

    setCredentials();
    bool error1 = m_private->proxy_media->GetVideoSources(&get_video_src, get_video_src_resp);
    setCredentials();
    bool error2 = m_private->proxy_media->GetVideoSourceConfigurations(&get_video_src_conf, get_video_src_conf_resp);
    if (error1 || error2){
        reportError();
    }

    m_video_token = get_video_src_resp.VideoSources[0]->token;
    m_video_conf_token = get_video_src_conf_resp.Configurations[0]->token;

    _trt__GetVideoEncoderConfigurationOptions get_video_enc_conf_opt;
    get_video_enc_conf_opt.ConfigurationToken = &m_video_conf_token;
    _timg__GetOptions get_opt;
    get_opt.VideoSourceToken = m_video_token;
    _timg__GetOptionsResponse get_opt_resp;

    setCredentials();
    bool error3 = m_private->proxy_media->GetVideoEncoderConfigurationOptions(
                            &get_video_enc_conf_opt, m_private->video_options);
    setCredentials();
    bool error4 = m_private->proxy_image->GetOptions(&get_opt, get_opt_resp);
    if (error3 || error4){
        reportError();
    }

    m_brightness[0] = get_opt_resp.ImagingOptions->Brightness->Min;
    m_brightness[1] = get_opt_resp.ImagingOptions->Brightness->Max;
    m_contrast[0] = get_opt_resp.ImagingOptions->Contrast->Min,
    m_contrast[1] = get_opt_resp.ImagingOptions->Contrast->Max;
    m_color_saturation[0] = get_opt_resp.ImagingOptions->ColorSaturation->Min;
    m_color_saturation[1] = get_opt_resp.ImagingOptions->ColorSaturation->Max;
}

ImageParam CameraOnvif::getImageParam(){
    ImageParam response = ImageParam();
    _timg__GetImagingSettings get_imaging;
    get_imaging.VideoSourceToken = m_video_token;
    _timg__GetImagingSettingsResponse get_imaging_resp;
    setCredentials();
    if (m_private->proxy_image->GetImagingSettings(&get_imaging, get_imaging_resp)){
        reportError();
    }
    response.color_saturation = *get_imaging_resp.ImagingSettings->ColorSaturation;
    response.brightness = *get_imaging_resp.ImagingSettings->Brightness;
    response.contrast = *get_imaging_resp.ImagingSettings->Contrast;

    return response;
}

void CameraOnvif::setImageParam(const ImageParam& new_param){
    float new_brightness = (m_brightness[1] - m_brightness[0]) *
                            new_param.brightness + m_brightness[0];
    float new_color_saturation = (m_color_saturation[1] - m_color_saturation[0]) *
                                  new_param.color_saturation + m_color_saturation[0];
    float new_contrast = (m_contrast[1] - m_contrast[0]) *
                          new_param.contrast + m_contrast[0];

    _timg__SetImagingSettings set_imaging;
    set_imaging.VideoSourceToken = m_video_token;
    set_imaging.ImagingSettings = new tt__ImagingSettings20();
    set_imaging.ImagingSettings->Brightness = &new_brightness;
    set_imaging.ImagingSettings->ColorSaturation = &new_color_saturation;
    set_imaging.ImagingSettings->Contrast = &new_contrast;

    _timg__SetImagingSettingsResponse set_imaging_resp;
    setCredentials();
    if (m_private->proxy_image->SetImagingSettings(&set_imaging, set_imaging_resp)) {
        reportError();
    }
}

void CameraOnvif::setResolution(int width, int height){
    bool valid = false;
    for (auto & e : m_private->video_options.Options->H264->ResolutionsAvailable) {
        if (e->Width == width && e->Height == height) {
            valid = true;
        }
    }
    if (!valid) {
        throw SOAPException("You are trying to set a video resolution that is not available!");
    }

    _trt__GetVideoEncoderConfigurationResponse conf_get_response;
    getVideoEncoderConfiguration(conf_get_response);

    _trt__SetVideoEncoderConfiguration conf_set;
    _trt__SetVideoEncoderConfigurationResponse conf_set_response;
    conf_set.Configuration = conf_get_response.Configuration;

    conf_set.Configuration->Resolution->Width = width;
    conf_set.Configuration->Resolution->Height = height;
    setCredentials();
    if (m_private->proxy_media->SetVideoEncoderConfiguration(&conf_set, conf_set_response)){
        reportError();
    }
}

void CameraOnvif::getVideoEncoderConfiguration(
    _trt__GetVideoEncoderConfigurationResponse& resp
) {
    _trt__GetVideoEncoderConfiguration GetVideoEncoderConfiguration;
    GetVideoEncoderConfiguration.ConfigurationToken = m_video_conf_token;

    setCredentials();
    bool error = m_private->proxy_media->GetVideoEncoderConfiguration(
        &GetVideoEncoderConfiguration, resp);
    if (error) {
        reportError();
    }
}

camera_onvif::Resolution CameraOnvif::getResolution(){
    _trt__GetVideoEncoderConfigurationResponse video_conf;
    getVideoEncoderConfiguration(video_conf);
    Resolution response = Resolution();
    response.width = video_conf.Configuration->Resolution->Width;
    response.height = video_conf.Configuration->Resolution->Height;
    return response;
}

int CameraOnvif::getTimeout(){
    return m_timeout;
}

void CameraOnvif::setTimeout(int timeout){
    m_timeout = timeout;
    m_private->soap->connect_timeout = m_timeout;
    m_private->soap->recv_timeout = m_timeout;
    m_private->soap->send_timeout = m_timeout;
}

void CameraOnvif::reportError(){
    std::ostringstream my_stream (std::ostringstream::ate);
    my_stream << "Oops, something went wrong:" << endl;
    soap_stream_fault(m_private->soap, my_stream);
    throw SOAPException(my_stream.str());
}

void CameraOnvif::setCredentials(){
    soap_wsse_delete_Security(m_private->soap);
    bool error1 = soap_wsse_add_Timestamp(m_private->soap, "Time", 10);
    bool error2 = soap_wsse_add_UsernameTokenDigest(
                    m_private->soap, "Auth", m_user.c_str(), m_pass.c_str());
    if ( error1 || error2) {
        reportError();
    }
}

void CameraOnvif::printCameraInfo(){
    // create the proxies to access the ONVIF service API at HOSTNAME
    DeviceBindingProxy proxyDevice(m_private->soap);

    // get device info and print
    proxyDevice.soap_endpoint = m_uri.c_str();
    _tds__GetDeviceInformation GetDeviceInformation;
    _tds__GetDeviceInformationResponse GetDeviceInformationResponse;
    setCredentials();
    if (proxyDevice.GetDeviceInformation(&GetDeviceInformation, GetDeviceInformationResponse)){
        reportError();
    }
    cout << "\n------------- Camera Information -------------" << endl;
    cout << "Manufacturer:    " << GetDeviceInformationResponse.Manufacturer << endl;
    cout << "Model:           " << GetDeviceInformationResponse.Model << endl;
    cout << "FirmwareVersion: " << GetDeviceInformationResponse.FirmwareVersion << endl;
    cout << "SerialNumber:    " << GetDeviceInformationResponse.SerialNumber << endl;
    cout << "HardwareId:      " << GetDeviceInformationResponse.HardwareId << endl;
}

CameraOnvif::~CameraOnvif() {
    delete m_private;
}

/******************************************************************************\
 *
 *	WS-Discovery event handlers must be defined, even when not used
 *
\******************************************************************************/

void wsdd_event_Hello(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int MetadataVersion)
{ }

void wsdd_event_Bye(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, const char *EndpointReference, const char *Types, const char *Scopes, const char *MatchBy, const char *XAddrs, unsigned int *MetadataVersion)
{ }

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *Types, const char *Scopes, const char *MatchBy, struct wsdd__ProbeMatchesType *ProbeMatches)
{
  return SOAP_WSDD_ADHOC;
}

void wsdd_event_ProbeMatches(struct soap *soap, unsigned int InstanceId, const char *SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ProbeMatchesType *ProbeMatches)
{ }

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *MessageID, const char *ReplyTo, const char *EndpointReference, struct wsdd__ResolveMatchType *match)
{
  return SOAP_WSDD_ADHOC;
}

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int InstanceId, const char * SequenceId, unsigned int MessageNumber, const char *MessageID, const char *RelatesTo, struct wsdd__ResolveMatchType *match)
{ }

int SOAP_ENV__Fault(struct soap *soap, char *faultcode, char *faultstring, char *faultactor, struct SOAP_ENV__Detail *detail, struct SOAP_ENV__Code *SOAP_ENV__Code, struct SOAP_ENV__Reason *SOAP_ENV__Reason, char *SOAP_ENV__Node, char *SOAP_ENV__Role, struct SOAP_ENV__Detail *SOAP_ENV__Detail)
{
  // populate the fault struct from the operation arguments to print it
  soap_fault(soap);
  // SOAP 1.1
  soap->fault->faultcode = faultcode;
  soap->fault->faultstring = faultstring;
  soap->fault->faultactor = faultactor;
  soap->fault->detail = detail;
  // SOAP 1.2
  soap->fault->SOAP_ENV__Code = SOAP_ENV__Code;
  soap->fault->SOAP_ENV__Reason = SOAP_ENV__Reason;
  soap->fault->SOAP_ENV__Node = SOAP_ENV__Node;
  soap->fault->SOAP_ENV__Role = SOAP_ENV__Role;
  soap->fault->SOAP_ENV__Detail = SOAP_ENV__Detail;
  // set error
  soap->error = SOAP_FAULT;
  // handle or display the fault here with soap_stream_fault(soap, std::cerr);
  // return HTTP 202 Accepted
  return soap_send_empty_response(soap, SOAP_OK);
}

/******************************************************************************\
 *
 *    OpenSSL
 *
\******************************************************************************/

#ifdef WITH_OPENSSL

struct CRYPTO_dynlock_value
{ MUTEX_TYPE mutex;
};

static MUTEX_TYPE *mutex_buf;

static struct CRYPTO_dynlock_value *dyn_create_function(const char *file, int line)
{ struct CRYPTO_dynlock_value *value;
  value = (struct CRYPTO_dynlock_value*)malloc(sizeof(struct CRYPTO_dynlock_value));
  if (value)
    MUTEX_SETUP(value->mutex);
  return value;
}

static void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(l->mutex);
  else
    MUTEX_UNLOCK(l->mutex);
}

static void dyn_destroy_function(struct CRYPTO_dynlock_value *l, const char *file, int line)
{ MUTEX_CLEANUP(l->mutex);
  free(l);
}

void locking_function(int mode, int n, const char *file, int line)
{ if (mode & CRYPTO_LOCK)
    MUTEX_LOCK(mutex_buf[n]);
  else
    MUTEX_UNLOCK(mutex_buf[n]);
}

unsigned long id_function()
{ return (unsigned long)THREAD_ID;
}

int CRYPTO_thread_setup()
{ int i;
  mutex_buf = (MUTEX_TYPE*)malloc(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
  if (!mutex_buf)
    return SOAP_EOM;
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_SETUP(mutex_buf[i]);
  CRYPTO_set_id_callback(id_function);
  CRYPTO_set_locking_callback(locking_function);
  CRYPTO_set_dynlock_create_callback(dyn_create_function);
  CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
  CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
  return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{ int i;
  if (!mutex_buf)
    return;
  CRYPTO_set_id_callback(NULL);
  CRYPTO_set_locking_callback(NULL);
  CRYPTO_set_dynlock_create_callback(NULL);
  CRYPTO_set_dynlock_lock_callback(NULL);
  CRYPTO_set_dynlock_destroy_callback(NULL);
  for (i = 0; i < CRYPTO_num_locks(); i++)
    MUTEX_CLEANUP(mutex_buf[i]);
  free(mutex_buf);
  mutex_buf = NULL;
}

#else

/* OpenSSL not used */

int CRYPTO_thread_setup()
{
  return SOAP_OK;
}

void CRYPTO_thread_cleanup()
{ }

#endif
#include "CameraOnvif.hpp"

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

using namespace std;
using namespace camera_onvif;

struct CameraOnvif::Private {
    struct soap *soap = soap_new1(SOAP_XML_STRICT | SOAP_XML_CANONICAL | SOAP_C_UTFSTRING);
    MediaBindingProxy *proxy_media;
    ImagingBindingProxy *proxy_image;
}defined;

CameraOnvif::CameraOnvif(/* args */) {
    user = "admin";
    pass = "camera01";
    uri = "http://10.20.0.188/onvif/device_service";

    // m_private->soap = &soap_new1(SOAP_XML_STRICT | SOAP_XML_CANONICAL);
    m_private = &defined;
    m_private->soap->connect_timeout = m_private->soap->recv_timeout = m_private->soap->send_timeout = 10;
    soap_register_plugin(m_private->soap, soap_wsse);





    m_private->proxy_media = new MediaBindingProxy(m_private->soap);
    m_private->proxy_media->soap_endpoint = uri.c_str();

    _trt__GetVideoSources GetVideoSources;
    _trt__GetVideoSourcesResponse GetVideoSourcesResponse;
    setCredentials();
    if (m_private->proxy_media->GetVideoSources(&GetVideoSources, GetVideoSourcesResponse))
        reportError();

    video_token = GetVideoSourcesResponse.VideoSources[0]->token;

    m_private->proxy_image = new ImagingBindingProxy(m_private->soap);
    m_private->proxy_image->soap_endpoint = uri.c_str();

     _timg__GetOptions GetOptions;
    GetOptions.VideoSourceToken = video_token;
    _timg__GetOptionsResponse GetOptionsResponse;
    setCredentials();
    if (m_private->proxy_image->GetOptions(&GetOptions, GetOptionsResponse))
        reportError();

    brightness[0] = GetOptionsResponse.ImagingOptions->Brightness->Min;
    brightness[1] = GetOptionsResponse.ImagingOptions->Brightness->Max;
    contrast[0] = GetOptionsResponse.ImagingOptions->Contrast->Min,
    contrast[0] = GetOptionsResponse.ImagingOptions->Contrast->Max;
    color_saturation[0] = GetOptionsResponse.ImagingOptions->ColorSaturation->Min;
    color_saturation[1] = GetOptionsResponse.ImagingOptions->ColorSaturation->Max;
}

float CameraOnvif::readBrightness(){
    _timg__GetImagingSettings GetImagingSettings;
    GetImagingSettings.VideoSourceToken = video_token;
    _timg__GetImagingSettingsResponse GetImagingSettingsResponse;
    setCredentials();
    if (m_private->proxy_image->GetImagingSettings(&GetImagingSettings, GetImagingSettingsResponse))
        reportError();
    return *GetImagingSettingsResponse.ImagingSettings->Brightness;
}
float CameraOnvif::readColorSaturation(){
    _timg__GetImagingSettings GetImagingSettings;
    GetImagingSettings.VideoSourceToken = video_token;
    _timg__GetImagingSettingsResponse GetImagingSettingsResponse;
    setCredentials();
    if (m_private->proxy_image->GetImagingSettings(&GetImagingSettings, GetImagingSettingsResponse))
        reportError();
    return *GetImagingSettingsResponse.ImagingSettings->ColorSaturation;
}
float CameraOnvif::readContrast(){
    _timg__GetImagingSettings GetImagingSettings;
    GetImagingSettings.VideoSourceToken = video_token;
    _timg__GetImagingSettingsResponse GetImagingSettingsResponse;
    setCredentials();
    if (m_private->proxy_image->GetImagingSettings(&GetImagingSettings, GetImagingSettingsResponse))
        reportError();
    return *GetImagingSettingsResponse.ImagingSettings->Contrast;
}

// to report an error
void CameraOnvif::reportError()
{
  cerr << "Oops, something went wrong:" << endl;
  soap_stream_fault(m_private->soap, cerr);
  exit(EXIT_FAILURE);
}

void CameraOnvif::setCredentials()
{
  soap_wsse_delete_Security(m_private->soap);
  if (soap_wsse_add_Timestamp(m_private->soap, "Time", 10)
   || soap_wsse_add_UsernameTokenDigest(m_private->soap, "Auth", user.c_str(), pass.c_str()))
    reportError();
}

void CameraOnvif::printCameraInfo(){
    // create the proxies to access the ONVIF service API at HOSTNAME
    DeviceBindingProxy proxyDevice(m_private->soap);

    // get device info and print
    proxyDevice.soap_endpoint = uri.c_str();
    _tds__GetDeviceInformation GetDeviceInformation;
    _tds__GetDeviceInformationResponse GetDeviceInformationResponse;
    setCredentials();
    if (proxyDevice.GetDeviceInformation(&GetDeviceInformation, GetDeviceInformationResponse))
        reportError();
    cout << "\n------------- Camera Information -------------" << endl;
    cout << "Manufacturer:    " << GetDeviceInformationResponse.Manufacturer << endl;
    cout << "Model:           " << GetDeviceInformationResponse.Model << endl;
    cout << "FirmwareVersion: " << GetDeviceInformationResponse.FirmwareVersion << endl;
    cout << "SerialNumber:    " << GetDeviceInformationResponse.SerialNumber << endl;
    cout << "HardwareId:      " << GetDeviceInformationResponse.HardwareId << endl;
}

CameraOnvif::~CameraOnvif() {
    // free all deserialized and managed data, we can still reuse the context and proxies after this
    soap_destroy(m_private->soap);
    soap_end(m_private->soap);

    // free the shared context, proxy classes must terminate as well after this
    soap_free(m_private->soap);
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
 *	OpenSSL
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

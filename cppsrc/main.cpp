/* cppsrc/main.cpp */
#include <napi.h>

#include <stdio.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

 
IAudioEndpointVolume * GetEndpointVolume()
{
	HRESULT hr;
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
	IMMDevice *defaultDevice = NULL;
	

	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get audio device enumerator").ThrowAsJavaScriptException();
	}

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);

	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get default audio endpoint device").ThrowAsJavaScriptException();
	}

	deviceEnumerator->Release();
	deviceEnumerator = NULL;
	 
	IAudioEndpointVolume *endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to activate default audio endpoint volume").ThrowAsJavaScriptException();
	}

	defaultDevice->Release();
	defaultDevice = NULL; 
	return endpointVolume;
}

bool SetMasterVolume(float level) {
  HRESULT hr;
  CoInitialize(NULL);
  IAudioEndpointVolume *endpointVolume = GetEndpointVolume();
  hr = endpointVolume->SetMasterVolumeLevelScalar((float)(level), NULL);

  if (FAILED(hr))
  {
	  Napi::TypeError::New(napi_env(), "Failed to set master volume").ThrowAsJavaScriptException();
  }

  endpointVolume->Release();
  CoUninitialize();
  return 0;
}

float GetMasterVolume() {
	HRESULT hr;
	CoInitialize(NULL);
	IAudioEndpointVolume* endpointVolume = GetEndpointVolume();

	float level;
	hr = endpointVolume->GetMasterVolumeLevelScalar(&level);

	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get master volume").ThrowAsJavaScriptException();
	}

	endpointVolume->Release();
	CoUninitialize();
	return level;
}

bool SetMasterMute(bool value) {
	HRESULT hr;
	CoInitialize(NULL);
	IAudioEndpointVolume* endpointVolume = GetEndpointVolume();
	hr = endpointVolume->SetMute((BOOL)(value), NULL);
	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to set mute status").ThrowAsJavaScriptException();
	}
	endpointVolume->Release();
	CoUninitialize();
	return 0;
}

bool GetMasterMute()
{
	HRESULT hr;
	CoInitialize(NULL);
	IAudioEndpointVolume* endpointVolume = GetEndpointVolume();
	BOOL value;
	hr = endpointVolume->GetMute(&value);
	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get mute status").ThrowAsJavaScriptException();
	}
	endpointVolume->Release();
	CoUninitialize();
	return (bool)value;
}

std::pair<float,bool> GetMasterVolumeAndMute()
{
	HRESULT hr;
	CoInitialize(NULL);
	IAudioEndpointVolume* endpointVolume = GetEndpointVolume();

	float level;
	BOOL mute;
	hr = endpointVolume->GetMasterVolumeLevelScalar(&level);
	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get master volume, in pair checking").ThrowAsJavaScriptException();
	}
	hr = endpointVolume->GetMute(&mute);
	if (FAILED(hr))
	{
		Napi::TypeError::New(napi_env(), "Failed to get mute status, in pair checking").ThrowAsJavaScriptException();
	}
	endpointVolume->Release();
	CoUninitialize();
	return std::make_pair(level,(bool)mute);
}

void SetVolume(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	auto volume = info[0].ToNumber().FloatValue();
	SetMasterVolume(volume);
}

void SetMute(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	auto mute = info[0].ToBoolean().Value();
	SetMasterMute(mute);
}

Napi::Boolean GetMute(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	return Napi::Boolean::New(env, GetMasterMute());
}

Napi::Number GetVolume(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();
	return Napi::Number::New(env, GetMasterVolume());
}

Napi::Object GetVolumeAndMute(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	auto volume = info[0].ToNumber().FloatValue();
	auto val = GetMasterVolumeAndMute();

	Napi::Object obj = Napi::Object::New(env);
	obj.Set(Napi::String::New(env, "volume"), val.first);
	obj.Set(Napi::String::New(env, "mute"), val.second);

	return obj;
}

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {

	exports.Set("setVolume", Napi::Function::New(env, SetVolume));
	exports.Set("getVolume", Napi::Function::New(env, GetVolume));
	exports.Set("getMute", Napi::Function::New(env, GetMute));
	exports.Set("setMute", Napi::Function::New(env, SetMute));
	exports.Set("getVolumeAndMute", Napi::Function::New(env, GetVolumeAndMute));

	return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, InitAll)
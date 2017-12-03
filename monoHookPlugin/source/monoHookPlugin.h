#pragma once

#include "Unity/IUnityGraphics.h"

typedef void(UNITY_INTERFACE_API *UnityEngineDebugFnPtr)(const char *);

typedef enum RenderEventFuncID
{
	Default_OnRenderEvent = 0,
	RenderThreadDetachEvent = 1,
	ManagedCodeRenderEvent = 2,
};

#ifdef __cplusplus
extern "C" {
#endif
	UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc(RenderEventFuncID funcID);
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API LinkDebug(UnityEngineDebugFnPtr fn);
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetRenderEventCallback(UnityRenderingEvent fn);
	void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API Shutdown();
#ifdef __cplusplus
}
#endif
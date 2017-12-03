#include "pch.h"

#include "monoHookPlugin.h"
#include "mono/Mono.h"
#include "mono/MonoWrapper.h"

// --------------------------------------------------------------------------
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

static IUnityInterfaces* s_UnityInterfaces = NULL;
static IUnityGraphics* s_Graphics = NULL;

extern "C" void	UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

// --------------------------------------------------------------------------
// GraphicsDeviceEvent

static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		s_DeviceType = s_Graphics->GetRenderer();
	}

	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		s_DeviceType = kUnityGfxRendererNull;
	}
}

// --------------------------------------------------------------------------
// Helper utilities

static UnityEngineDebugFnPtr s_debugLog = nullptr;
static UnityRenderingEvent s_renderingCallback = nullptr;
void UNITY_INTERFACE_API LinkDebug(UnityEngineDebugFnPtr fn) { s_debugLog = fn; }
void UNITY_INTERFACE_API SetRenderEventCallback(UnityRenderingEvent fn) { s_renderingCallback = fn;}

inline void DebugLog(const char *str) {
	if (s_debugLog != nullptr) {
		s_debugLog(str);
	}
}

void UNITY_INTERFACE_API Shutdown()
{
	s_debugLog = nullptr;
	s_renderingCallback = nullptr;
}

// --------------------------------------------------------------------------
// OnRenderEvent
// This will be called for GL.IssuePluginEvent script calls; eventID will
// be the integer passed to IssuePluginEvent. In this example, we just ignore
// that value.

static void UNITY_INTERFACE_API OnRenderEvent(int /*eventID*/)
{
	//Native RenderEvent;
}

static void UNITY_INTERFACE_API ManagedCodeRenderEventCallback(int eventID)
{
	if (s_renderingCallback != nullptr)
	{
		DebugLog("This is ManagedCodeRenderEventCallback");
		s_renderingCallback(eventID);
	}
}

static void UNITY_INTERFACE_API RenderThreadDetach(int /*eventID*/)
{
	if (g_mono_dll)
	{
		_mono_thread_detach(_mono_thread_current());
	}
}


// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.
UnityRenderingEvent UNITY_INTERFACE_API GetRenderEventFunc(RenderEventFuncID FuncID)
{
	DebugLog("GetRenderEventFunc!");
	switch (FuncID)
	{
	case RenderThreadDetachEvent:
		return RenderThreadDetach;
	case ManagedCodeRenderEvent:
		return ManagedCodeRenderEventCallback;
	default:
	case Default_OnRenderEvent:
		return OnRenderEvent;
	}
}

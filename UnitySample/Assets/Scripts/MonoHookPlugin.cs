using System;
using System.Runtime.InteropServices;

public enum RenderEventFuncID
{
    Default_OnRenderEvent = 0,
    RenderThreadDetachEvent = 1,
    ManagedCodeRenderEvent = 2,
};

public class MonoHookNativeInterop
{

    const string NATIVE_PLUGIN_NAME = "monoHookPlugin";

    [DllImportAttribute(NATIVE_PLUGIN_NAME)]
    internal static extern IntPtr GetRenderEventFunc(RenderEventFuncID funcID);
    [DllImportAttribute(NATIVE_PLUGIN_NAME)]
    internal static extern void LinkDebug([MarshalAs(UnmanagedType.FunctionPtr)]IntPtr debugCal);
    [DllImportAttribute(NATIVE_PLUGIN_NAME)]
    internal static extern void SetRenderEventCallback([MarshalAs(UnmanagedType.FunctionPtr)]IntPtr renderCal);
    [DllImportAttribute(NATIVE_PLUGIN_NAME)]
    internal static extern void Shutdown();
}

public class MonoHookNativePlugin
{
    [UnmanagedFunctionPointer(CallingConvention.Winapi)]
    private delegate void DebugLog(string log);
    private static readonly DebugLog debugLog = DebugWrapper;
    private static readonly IntPtr functionPointer = Marshal.GetFunctionPointerForDelegate(debugLog);
    private static void DebugWrapper(string log) { UnityEngine.Debug.Log(log); }

    public static void Init()
    {
        MonoHookNativeInterop.LinkDebug(functionPointer);
    }

    public static void Shutdown()
    {
        MonoHookNativeInterop.Shutdown();
    }
}

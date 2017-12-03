using System;
using UnityEngine;
using UnityEngine.Rendering;
using AOT;
using System.Runtime.InteropServices;

[RequireComponent(typeof(Camera))]
public class ManagedCodeRendering : MonoBehaviour {

	// Use this for initialization
	void Start () {
        MonoHookNativePlugin.Init();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    private void OnPostRender()
    {
        //This will register a managed code callback to the rendering thread.
        GL.IssuePluginEvent(RegisterRenderCallback(__submit), 121);
    }

    private void OnDisable()
    {
        //Use mono hook to detach rendering thread from mono.
        CommandBuffer cb = new CommandBuffer();
        cb.IssuePluginEvent(MonoHookNativeInterop.GetRenderEventFunc(RenderEventFuncID.RenderThreadDetachEvent), 0);
        Graphics.ExecuteCommandBuffer(cb);

        MonoHookNativePlugin.Shutdown();
    }

    protected IntPtr RegisterRenderCallback(RenderEventDelegate callback)
    {
        RenderEventFuncPtr = Marshal.GetFunctionPointerForDelegate(callback);
        MonoHookNativeInterop.SetRenderEventCallback(RenderEventFuncPtr);
        return MonoHookNativeInterop.GetRenderEventFunc(RenderEventFuncID.ManagedCodeRenderEvent);
    }

    [UnmanagedFunctionPointer(CallingConvention.StdCall)]
    protected delegate void RenderEventDelegate(int id);

    [MarshalAs(UnmanagedType.FunctionPtr)]
    private IntPtr RenderEventFuncPtr;

    [MonoPInvokeCallbackAttribute(typeof(RenderEventDelegate))]
    public void __submit(int id)
    {
        Debug.Log("_submitDelegate " + id);
        Debug.Log("This is managed code callback on Rendering thread!");
    }
}

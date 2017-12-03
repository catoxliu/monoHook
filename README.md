# monoHook

When I want to use mananged code as Unity3D rendering thread callback, I faced a problem like [ousttrue ](https://ousttrue.github.io/post/2017/04/mono_domain_unload/) do. The problem here is that when rendering thread (which is unmanaged code) calls managed code (which is managed by mono), it will "attach" to mono. Then at the time the application is going to quit, the mono main thread will wait for all attached threads stoping first. Here, the application will hang forever on exit process.

Normally, we do not need to care about mono when using Unity3D. However, the project [USDForUnity from Unity3D-JP](https://github.com/unity3d-jp/USDForUnity) inspire me that I could hook mono to use mono's api to fix the bug decribed above. Then, here it is.

Of course it could be used for any other purpose, while it's on your risk~.


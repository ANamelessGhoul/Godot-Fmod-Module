def can_build(env, platform):
    return platform == "x11" or platform == "windows" or platform == "osx" or platform == "android" or platform == "iphone" or platform == "javascript"


def configure(env):
    if env["platform"] == "windows":
        env.Append(LIBS=["fmod_vc", "fmodstudio_vc"])
        if env["bits"] == "32":
            env.Append(LIBPATH=["#modules/fmod/api/windows/core/lib/x86/",
                                "#modules/fmod/api/windows/studio/lib/x86/"])
        else:
            env.Append(LIBPATH=["#modules/fmod/api/windows/core/lib/x64/",
                                "#modules/fmod/api/windows/studio/lib/x64/"])

    elif env["platform"] == "x11":
        env.Append(LIBS=["fmod", "fmodstudio"])
        if env["bits"] == "32":
            env.Append(
                LIBPATH=["#modules/fmod/api/x11/core/lib/x86/",
                         "#modules/fmod/api/x11/studio/lib/x86/"])
        else:
            env.Append(
                LIBPATH=["#modules/fmod/api/x11/core/lib/x86_64/",
                         "#modules/fmod/api/x11/studio/lib/x86_64/"])

    elif env["platform"] == "osx":
        env.Append(LIBS=["fmod", "fmodstudio"])
        env.Append(
            LIBPATH=["#modules/fmod/api/osx/core/lib/", "#modules/fmod/api/osx/studio/lib/"])

    elif env["platform"] == "android":
        if env["android_arch"] == "arm64v8":
            env.Append(LIBPATH=["#modules/fmod/api/android/core/lib/arm64-v8a",
                                "#modules/fmod/api/android/studio/lib/arm64-v8a"])
        else:
            env.Append(LIBPATH=["#modules/fmod/api/android/core/lib/armeabi-v7a",
                                "#modules/fmod/api/android/studio/lib/armeabi-v7a"])
        env.Append(LIBS=["fmod", "fmodstudio"])

    elif env["platform"] == "iphone":
        env.Append(LIBPATH=["#modules/fmod/api/iphone/core/lib/",
                            "#modules/fmod/api/iphone/studio/lib/"])
        env.Append(LIBS=["libfmod_iphoneos.a", "libfmodstudio_iphoneos.a"])
    
    elif env["platform"] == "javascript":
        env.Append(LIBS=["fmod_wasm.a", "fmodstudio_wasm.a"])
        env.Append(LIBPATH=["#modules/fmod/api/javascript/core/lib/upstream/w32/",
                                "#modules/fmod/api/javascript/studio/lib/upstream/w32/"])

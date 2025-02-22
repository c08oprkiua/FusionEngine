import os
import sys


def is_active():
    return True


def get_name():
    return "gc_wii"


def can_build():

    if os.name != "posix":
        return False

    return True  # enabled


def get_opts():
    return [
         ('gamecube','Compile gc_wii for GameCube instead of Wii','no')
        ]


def get_flags():
    return [
        ("builtin_zlib", "no"),
        ("tools", "no"),
        ("theora", "no"),
        ("module_pspmath_enabled", "no"),
    ]


def configure(env):
    env.disabled_modules = ["openssl", "webp"]

    env["bits"] = "32"
    devkitpro_path = os.environ["DEVKITPRO"]
    devkitppc_path = devkitpro_path + "/devkitPPC"
    ogc_path = devkitpro_path + "/libogc2"

    env.Append(CPPPATH=["#platform/gc_wii"])
    env["CC"] = devkitppc_path + "/bin/powerpc-eabi-gcc"
    env["CXX"] = devkitppc_path + "/bin/powerpc-eabi-g++"
    env["LD"] = devkitppc_path + "/bin/powerpc-eabi-g++"
    env["AR"] = devkitppc_path + "/bin/powerpc-eabi-ar"
    env["RANLIB"] = devkitppc_path + "/bin/powerpc-eabi-ranlib"
    env["AS"] = devkitppc_path + "/bin/powerpc-eabi-as"

    #not sure where MD and P come from...
    arch = ["-MMD", "-MP", "-DGEKKO", "-mcpu=750", "-meabi", "-mhard-float"]

    portlib_include_path = ""
    ogc_lib_path = ""
    portlib_lib_path = ""

    console_flags = []

    if env["gamecube"] == 'yes':
        portlib_include_path = "/portlibs/gamecube/include/"
        ogc_lib_path = "/lib/cube/"
        portlib_lib_path = "/portlibs/gamecube/lib/"

        arch += ["-mogc"]

        console_flags = [
            "-DNINTENDO_GAMECUBE",
            "-DOGC_MACHINE='ogc'",
            "__GAMECUBE__",
            "-DHW_DOL"
        ]
    else:
        portlib_include_path = "/portlibs/wii/include/"
        ogc_lib_path = "/lib/wii/"
        portlib_lib_path = "/portlibs/wii/lib/"

        arch += ["-mrvl"]

        console_flags = [
            "-DNINTENDO_WII",
            "-DOGC_MACHINE='rvl'" #This could be formatted wrong for SCons (and the one for gamecube)
            "-DWII_ENABLED",
            "-D__WII__",
            "-DHW_RVL",

            "-DPOSIX_IP_ENABLED", #may or may not work on GC without issues, but for now...
        ]

        #Wii only libs
        env.Append(LIBS=["wiiuse", "wiikeyboard", "bte"])

    #Fusion flags
    env.Append(
        CCFLAGS=[
            "-DGEKKO",
            #"-DGAMECUBE_WII",
            "-DBIG_ENDIAN_ENABLED",

            "-DNEED_LONG_INT",
            "-DLIBC_FILEIO_ENABLED",
            "-DNO_SAFE_CAST",
            "-DPTHREAD_NO_RENAME",
            "-DNO_THREADS",
            "-DNO_SAFE_CAST",
            "-DNO_FCNTL",

            "-DGLEW_ENABLED",
            "-DGLES_OVER_GL", #will this be needed when we use GX?
            "-fno-exceptions",
            "-fno-rtti",

        ]
        + console_flags
        + arch
    )

    env.Append(
        CPPPATH=[
            ogc_path + "/include",
            devkitpro_path + portlib_include_path,
            devkitpro_path + "/portlibs/ppc/include/",
        ]
    )
    env.Append(
        LIBPATH=[
            ogc_path + ogc_lib_path,
            devkitpro_path + portlib_lib_path,
            devkitpro_path + "/portlibs/ppc/lib/",
        ]
    )

    env.Append(ENV={"DEVKITPRO": devkitpro_path})

    env.Append(
        LIBS=[
            #external ported libs
            "opengx",
            "SDLmain", #TODO: Remove SDL
            "SDL", #TODO: Remove SDL

            #libogc2
            "ogc",
            "aesnd",
            "fat",
            "m",
        ]
    )
    env.Append(LIBS=["png", "z"])
    env.Append(LINKFLAGS=arch)

    if env["target"] == "release":
        env.Append(CCFLAGS=["-O2", "-ffast-math"])
    elif env["target"] == "release_debug":
        env.Append(CCFLAGS=["-g2", "-O2", "-ffast-math", "-DDEBUG_ENABLED"])
    elif env["target"] == "debug":
        env.Append(
            CCFLAGS=["-O0", "-g2", "-Wall", "-DDEBUG_ENABLED", "-DDEBUG_MEMORY_ENABLED"]
        )



import os

import sys	


def is_active():
	return True
        
def get_name():
	return "Win32"

def can_build():
	mingw = "i586-mingw32msvc-"
	mingw64 = "i686-w64-mingw32-"
	if (os.getenv("MINGW32_PREFIX")):
		mingw=os.getenv("MINGW32_PREFIX")
	if (os.getenv("MINGW64_PREFIX")):
		mingw64=os.getenv("MINGW64_PREFIX")

	if os.system(mingw+"gcc --version >/dev/null") == 0 or os.system(mingw64+"gcc --version >/dev/null") ==0 or os.system("gcc.exe --version >/dev/null") ==0:
		return True
			
	return False
		
def get_opts():

	mingw=""
	mingw64=""
	if (os.name!="nt"):
		mingw = "i586-mingw32msvc-"
		mingw64 = "i686-w64-mingw32-"
		if (os.getenv("MINGW32_PREFIX")):
			mingw=os.getenv("MINGW32_PREFIX")
		if (os.getenv("MINGW64_PREFIX")):
			mingw64=os.getenv("MINGW64_PREFIX")
	tdm_available = os.system("gcc.exe --version >/dev/null") ==0

	return [
		('mingw_prefix','Mingw Prefix',mingw),
		('mingw_prefix_64','Mingw Prefix 64 bits',mingw64),
		('mingw64_for_32','Use Mingw 64 for 32 Bits Build',"no"),
		('use_tdm','Use TDM-GCC instead of Mingw','yes' if tdm_available else 'no'),
	]
  
def get_flags():

	return [
		('freetype','builtin'), #use builtin freetype
		('openssl','no'), #use builtin openssl
		('theora','no'),
	]
			


def configure(env):

	env.Append(CPPPATH=['#platform/windows'])
	env.Append(CPPPATH=['#platform/win9x'])
	#build using mingw
	if (os.name=="nt"):
		env['ENV']['TMP'] = os.environ['TMP'] #way to go scons, you can be so stupid sometimes
	else:
		env["PROGSUFFIX"]=env["PROGSUFFIX"]+".exe"

	mingw_prefix=""

	if (env["bits"]=="default"):
		env["bits"]="32"

	use64=False
	if (env["bits"]=="32"):

		if (env["mingw64_for_32"]=="yes"):
			env.Append(CCFLAGS=['-m32'])
			env.Append(LINKFLAGS=['-m32'])
			env.Append(LINKFLAGS=['-static-libgcc'])
			env.Append(LINKFLAGS=['-static-libstdc++'])
			mingw_prefix=env["mingw_prefix_64"];
		else:
			mingw_prefix=env["mingw_prefix"];


	else:
		mingw_prefix=env["mingw_prefix_64"];
		env.Append(LINKFLAGS=['-static'])

	nulstr=""

	if (os.name=="posix"):
		nulstr=">/dev/null"
	else:
		nulstr=">nul"



	if os.system(mingw_prefix+"gcc --version"+nulstr)!=0:
		#not really super consistent but..
		print("Can't find Windows compiler: "+mingw_prefix)
		sys.exit(255)

	if (env["target"]=="release"):
		
		env.Append(CCFLAGS=['-Os','-ffast-math','-fomit-frame-pointer'])
		env.Append(LINKFLAGS=['-Wl,--subsystem,windows'])

	elif (env["target"]=="release_debug"):

		env.Append(CCFLAGS=['-Os','-DDEBUG_ENABLED'])

	elif (env["target"]=="debug"):
				
		env.Append(CCFLAGS=['-g', '-Wall','-DDEBUG_ENABLED','-DDEBUG_MEMORY_ENABLED'])

	if (env["freetype"]!="no"):
		env.Append(CCFLAGS=['-DFREETYPE_ENABLED'])
		env.Append(CPPPATH=['#tools/freetype'])
		env.Append(CPPPATH=['#tools/freetype/freetype/include'])

	if env['use_tdm']!='no':
		env.use_windows_spawn_fix()
		env["CC"]="gcc.exe"
		env['AS']="as.exe"
		env['CXX'] = "g++.exe"
		env['AR'] = "ar.exe"
		env['RANLIB'] = "ranlib.exe"
		env['LD'] = "g++.exe"
	else:
		env["CC"]=mingw_prefix+"gcc"
		env['AS']=mingw_prefix+"as"
		env['CXX'] = mingw_prefix+"g++"
		env['AR'] = mingw_prefix+"ar"
		env['RANLIB'] = mingw_prefix+"ranlib"
		env['LD'] = mingw_prefix+"g++"

	env.Append(CCFLAGS=['-DWINDOWS_ENABLED','-DWIN98_ENABLED','-mwindows','-D__MSVCRT_VERSION__=0x400'])
	#env.Append(CPPFLAGS=['-DRTAUDIO_ENABLED'])
	env.Append(CCFLAGS=['-DOPENGL_ENABLED','-DGLES1_ENABLED','-DGLES_OVER_GL','-DGLEW_ENABLED','-DNO_SAFE_CAST','-fno-rtti'])
	env.Append(LIBS=['mingw32','opengl32', 'dsound', 'ole32','winmm','gdi32','iphlpapi','kernel32'])

	env.Append(CPPFLAGS=['-march=pentium'])
	env.Append(CCFLAGS=['-march=pentium'])
	env.Append(LINKFLAGS=['-march=pentium'])

	env.Append(LINKFLAGS=['-Wl,--stack,'+str(16*1024*1024)])

	#'d3dx9d'
	env.Append(CPPFLAGS=['-DMINGW_ENABLED'])
	env.Append(LINKFLAGS=['-g'])

	#import methods
	#env.Append( BUILDERS = { 'HLSL9' : env.Builder(action = methods.build_hlsl_dx9_headers, suffix = 'hlsl.h',src_suffix = '.hlsl') } )

	


/*************************************************************************/
/*  OS_VITA.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifdef GLES2_ENABLED
#define PSP2_GLES2
#endif

#include "servers/visual/visual_server_raster.h"
#include "servers/visual/rasterizer_dummy.h"
#ifdef PSP2_GLES2

extern "C" {
	// extern void glInitAngle(void* a, void* b, void* c);
}

#include "drivers/gles2/rasterizer_gles2.h"
#else
#include "rasterizer_vita.h"
#endif
#include "os_vita.h"
#include <stdio.h>
#include <stdlib.h>
#include "print_string.h"
#include "servers/physics/physics_server_sw.h"
#include "drivers/unix/memory_pool_static_malloc.h"
#include "drivers/unix/dir_access_unix.h"
#include "drivers/unix/file_access_unix.h"
#include "os/memory_pool_dynamic_static.h"
#include "core/os/thread_dummy.h"
#include "drivers/unix/thread_posix.h"
#include "drivers/unix/semaphore_posix.h"
#include "drivers/unix/mutex_posix.h"
#include "main/main.h"
#include <sys/time.h>
#include <unistd.h>

#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/processmgr.h>

#include <EGL/eglplatform.h>
#include <EGL/egl.h>
extern "C" {
#include <gpu_es4/psp2_pvr_hint.h>
}
int OS_VITA::get_video_driver_count() const {

	return 1;
}
const char * OS_VITA::get_video_driver_name(int p_driver) const {

	return "pvrpsp2-gles1";
}
OS::VideoMode OS_VITA::get_default_video_mode() const {

	return OS::VideoMode(960,544,true);
}
static EGLDisplay Display;
static EGLConfig Config;
static EGLSurface Surface;
static EGLContext Context;
static MemoryPoolStaticMalloc *mempool_static=NULL;
static MemoryPoolDynamicStatic *mempool_dynamic=NULL;
	
	
void OS_VITA::initialize_core() {

	ThreadPosix::make_default();
	SemaphoreDummy::make_default();
	MutexPosix::make_default();

	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);
	//FileAccessBufferedFA<FileAccessUnix>::make_default();
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);


	mempool_static = new MemoryPoolStaticMalloc;
	mempool_dynamic = memnew( MemoryPoolDynamicStatic );
	
	ticks_start = 0;
	ticks_start = get_ticks_usec();

	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessUnix>(FileAccess::ACCESS_FILESYSTEM);
	//FileAccessBufferedFA<FileAccessUnix>::make_default();
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessUnix>(DirAccess::ACCESS_FILESYSTEM);
	
	sceKernelLoadStartModule("vs0:sys/external/libfios2.suprx", 0, NULL, 0, NULL, NULL);
	sceKernelLoadStartModule("vs0:sys/external/libc.suprx", 0, NULL, 0, NULL, NULL);
	sceKernelLoadStartModule("app0:module/libgpu_es4_ext.suprx", 0, NULL, 0, NULL, NULL);
  	sceKernelLoadStartModule("app0:module/libIMGEGL.suprx", 0, NULL, 0, NULL, NULL);
	
	PVRSRV_PSP2_APPHINT hint;
  	PVRSRVInitializeAppHint(&hint);
	snprintf(hint.szGLES1, 256, "%s/%s", "app0:module", "libGLESv1_CM.suprx");
    snprintf(hint.szWindowSystem, 256, "%s/%s", "app0:module", "libpvrPSP2_WSEGL.suprx");
    hint.ui32SwTexOpCleanupDelay = 16000;
	// hint.ui32UNCTexHeapSize = 60 * 1024 * 1024;
    // hint.ui32CDRAMTexHeapSize = 96 * 1024 * 1024;
	// hint.bDisableHWTQBufferBlit = 1;
	// hint.bDisableAsyncTextureOp = 1;
  	PVRSRVCreateVirtualAppHint(&hint);
	
	printf("created\n");
	EGLint NumConfigs;
	EGLint ConfigAttr[] = {
        EGL_BUFFER_SIZE, EGL_DONT_CARE,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
#ifdef PSP2_GLES2
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
#endif
        EGL_NONE
    };
	EGLint ContextAttributeList[] = 
	{
#ifdef PSP2_GLES2
		EGL_CONTEXT_CLIENT_VERSION, 2,
#endif
		EGL_NONE
	};
	EGLBoolean Res;
	Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(!Display)
	{
		perror("EGL display get failed.");
		return;
	}

	Res = eglInitialize(Display, NULL, NULL);
	if (Res == EGL_FALSE)
	{
		perror("EGL initialize failed. ");
		return;
	}

	Res = eglChooseConfig(Display, ConfigAttr, &Config, 1, &NumConfigs);
	if (Res == EGL_FALSE)
	{
		perror("EGL config initialize failed. ");
		return;
	}
	Psp2NativeWindow window;
	window.type = PSP2_DRAWABLE_TYPE_WINDOW;
    window.numFlipBuffers = 2;
    window.flipChainThrdAffinity = 0x20000;
    window.windowSize = PSP2_WINDOW_960X544;
	Surface = eglCreateWindowSurface(Display, Config, &window, nullptr);
	if(!Surface)
	{
		perror("EGL surface create failed.");
		return;
	}

	Context = eglCreateContext(Display, Config, EGL_NO_CONTEXT, ContextAttributeList);
	if(!Context)
	{
		perror("EGL content create failed.");
		return;
	}

	eglMakeCurrent(Display, Surface, Surface, Context);
	printf("done egl\n");
#ifdef PSP2_GLES2
	// glInitAngle(0, 0, 0);
#endif
}

void OS_VITA::finalize_core() {
	if (mempool_dynamic)
		memdelete( mempool_dynamic );
	delete mempool_static;
}

void OS_VITA::initialize(const VideoMode& p_desired,int p_video_driver,int p_audio_driver) {

	args=OS::get_singleton()->get_cmdline_args();
	current_videomode=p_desired;
	main_loop=NULL;
	
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	
	samples_in = memnew_arr(int32_t, 2048);
	samples_out = memnew_arr(int16_t, 2048);
#ifndef PSP2_GLES2
	rasterizer = memnew( RasterizerGLES1 );
#else
	RasterizerGLES2 *rasterizer_gles2 = memnew( RasterizerGLES2(false,false,false,false) );
	rasterizer_gles2->set_use_framebuffers(false);
	rasterizer = rasterizer_gles2;
#endif
	visual_server = memnew( VisualServerRaster(rasterizer) );

	AudioDriverManagerSW::get_driver(p_audio_driver)->set_singleton();

	if (AudioDriverManagerSW::get_driver(p_audio_driver)->init()!=OK) {

		ERR_PRINT("Initializing audio failed.");
	}

	sample_manager = memnew( SampleManagerMallocSW );
	audio_server = memnew( AudioServerSW(sample_manager) );
	audio_server->init();
	spatial_sound_server = memnew( SpatialSoundServerSW );
	spatial_sound_server->init();
	spatial_sound_2d_server = memnew( SpatialSound2DServerSW );
	spatial_sound_2d_server->init();

	
	ERR_FAIL_COND(!visual_server);

	visual_server->init();
	//
	physics_server = memnew( PhysicsServerSW );
	physics_server->init();
	physics_2d_server = memnew( Physics2DServerSW );
	physics_2d_server->init();

	input = memnew( InputDefault );

	
}
void OS_VITA::finalize() {

	if(main_loop)
		memdelete(main_loop);
	main_loop=NULL;

	spatial_sound_server->finish();
	memdelete(spatial_sound_server);
	spatial_sound_2d_server->finish();
	memdelete(spatial_sound_2d_server);

	//if (debugger_connection_console) {
//		memdelete(debugger_connection_console);
//}

	audio_server->finish();
	memdelete(audio_server);
	memdelete(sample_manager);

	visual_server->finish();
	memdelete(visual_server);
	memdelete(rasterizer);
	
	physics_server->finish();
	memdelete(physics_server);

	physics_2d_server->finish();
	memdelete(physics_2d_server);

	memdelete(input);

	args.clear();
	
	memdelete_arr(samples_in);
	memdelete_arr(samples_out);
}

void OS_VITA::set_mouse_show(bool p_show) {


}

SceCtrlButtons buttons[] = {
	SCE_CTRL_CROSS, SCE_CTRL_CIRCLE, SCE_CTRL_SQUARE, SCE_CTRL_TRIANGLE,
	SCE_CTRL_L2, SCE_CTRL_R2, SCE_CTRL_L1, SCE_CTRL_R1,
	SCE_CTRL_L3, SCE_CTRL_R3, SCE_CTRL_SELECT, SCE_CTRL_START,
	SCE_CTRL_UP, SCE_CTRL_DOWN, SCE_CTRL_LEFT, SCE_CTRL_RIGHT
};
#define MAX_JOY_AXIS 32768
void OS_VITA::process_keys() {
	sceCtrlReadBufferPositive(0, &pad, 1);

	last++;

	for(int i = 0; i < 16; i++) {
		if (pad.buttons & buttons[i]) {
			InputEvent event;
			event.type = InputEvent::JOYSTICK_BUTTON;
			event.device = 0;
			event.joy_button.button_index = i;
			event.joy_button.pressed = true;
			event.ID = last;
			input->parse_input_event(event);
		} else {
			InputEvent event;
			event.type = InputEvent::JOYSTICK_BUTTON;
			event.device = 0;
			event.joy_button.button_index = i;
			event.joy_button.pressed = false;
			event.ID = last;
			input->parse_input_event(event);
		}
	}
	/*
	uint8_t lx = ((pad.lx) / 255.0f) * 2.0 - 1.0;
	uint8_t ly = ((pad.ly) / 255.0f) * 2.0 - 1.0;
	uint8_t rx = ((pad.rx) / 255.0f) * 2.0 - 1.0;
	uint8_t ry = ((pad.ry) / 255.0f) * 2.0 - 1.0;
	
	input->set_joy_axis(0, 0, lx);
	input->set_joy_axis(0, 1, ly);
	input->set_joy_axis(0, 2, rx);
	input->set_joy_axis(0, 3, ry);
	*/
	uint8_t values[4] = {pad.lx, pad.ly, pad.rx, pad.ry};
	for(int i = 0; i < 4; i++) {
		InputEvent ievent;
		
		ievent.type = InputEvent::JOYSTICK_MOTION;
		ievent.ID = ++last;
		ievent.joy_motion.axis = i;
		if(i == 2)
			ievent.joy_motion.axis = 3;
		if(i == 3)
			ievent.joy_motion.axis = 4;
		
		ievent.joy_motion.axis_value = (float)(((values[i] - 127.5) / 127.5));

		input->parse_input_event( ievent );
	}
}

void OS_VITA::set_mouse_grab(bool p_grab) {

	grab=p_grab;
}
bool OS_VITA::is_mouse_grab_enabled() const {

	return grab;
}

int OS_VITA::get_mouse_button_state() const {

	return 0;
}

Point2 OS_VITA::get_mouse_pos() const {

	return Point2();
}

void OS_VITA::set_window_title(const String& p_title) {


}

void OS_VITA::set_video_mode(const VideoMode& p_video_mode,int p_screen) {


}

OS::Date OS_VITA::get_date() const {
	Date ret;
	return ret;
}

OS::Time OS_VITA::get_time() const {
	Time ret;
	return ret;
}


void OS_VITA::delay_usec(uint32_t p_usec) const{
	usleep(p_usec);
}

uint64_t OS_VITA::get_ticks_usec() const{
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);

	uint64_t longtime = (uint64_t)tv_now.tv_usec + (uint64_t)tv_now.tv_sec * 1000000L;
	longtime -= ticks_start;

	return longtime;
}

OS::VideoMode OS_VITA::get_video_mode(int p_screen) const {

	//return current_videomode;
	return get_default_video_mode();
}
void OS_VITA::get_fullscreen_mode_list(List<VideoMode> *p_list,int p_screen) const {


}


MainLoop *OS_VITA::get_main_loop() const {

	return main_loop;
}

void OS_VITA::delete_main_loop() {

	if (main_loop)
		memdelete(main_loop);
	main_loop=NULL;
}

void OS_VITA::set_main_loop( MainLoop * p_main_loop ) {

	main_loop=p_main_loop;
	input->set_main_loop(p_main_loop);
}

bool OS_VITA::can_draw() const {

	return true;
};


String OS_VITA::get_name() {

	return "Vita";
}



void OS_VITA::move_window_to_foreground() {

}

void OS_VITA::set_cursor_shape(CursorShape p_shape) {


}

void OS_VITA::process_audio() {
	audio_server->driver_process(1024, samples_in);
	for(int i = 0; i < 2048; ++i) {
		samples_out[i] = samples_in[i] >> 16;
	}
	
	// printf("%d\n", samples_out[1]);
	
	// sceAudioOutput2OutputBlocking(0x8000, samples_out);
	// sceAudioOutput
	// sceAudioOutOutput(port, samples_out);
}

void OS_VITA::run() {

	force_quit = false;
	
	if (!main_loop)
		return;
		
	main_loop->init();
		
	while (!force_quit) {
		// process_audio();
		process_keys();
		
		if (Main::iteration()==true)
			break;
		
		
	};
	
	main_loop->finish();
}

void OS_VITA::swap_buffers() {
	// glutSwapBuffers();
	eglSwapBuffers(Display,Surface);
}

OS_VITA::OS_VITA() {

	AudioDriverManagerSW::add_driver(&driver_dummy);
	//adriver here
	grab=false;
	_verbose_stdout=true;

};

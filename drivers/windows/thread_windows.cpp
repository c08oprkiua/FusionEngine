/*************************************************************************/
/*  thread_windows.cpp                                                   */
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
#include "thread_windows.h"

#if defined(WINDOWS_ENABLED) && !defined(WINRT_ENABLED)

#include "os/memory.h"

Thread::ID ThreadWindows::get_ID() const {

	return id;	
}

Thread* ThreadWindows::create_thread_windows() {

	return memnew( ThreadWindows );
}

DWORD ThreadWindows::thread_callback( LPVOID userdata ) {

	ERR_FAIL_NULL_V(userdata, 0);
	ThreadWindows *t=reinterpret_cast<ThreadWindows*>(userdata);
	ERR_FAIL_NULL_V(t->callback, 0);

	t->callback(t->user);
	t->id=(ID)GetCurrentThreadId(); // must implement
	return 0;
}

Thread* ThreadWindows::create_func_windows(ThreadCreateCallback p_callback,void *p_user,const Settings&) {

	ThreadWindows *tr= memnew(ThreadWindows);
	tr->callback=p_callback;
	tr->user=p_user;	
	tr->handle=CreateThread(
            NULL,                   // default security attributes
            0,                      // use default stack size  
            thread_callback,       // thread function name
            tr,          // argument to thread function
            0,                      // use default creation flags 
            NULL);   // returns the thread identifier 
	
	return tr;
}
Thread::ID ThreadWindows::get_thread_ID_func_windows() {

	return (ID)GetCurrentThreadId(); //must implement
}
void ThreadWindows::wait_to_finish_func_windows(Thread* p_thread) {

	
	ThreadWindows *tp=static_cast<ThreadWindows*>(p_thread);
	ERR_FAIL_COND(!tp);
	WaitForSingleObject( tp->handle, INFINITE );
	CloseHandle(tp->handle);
        //`memdelete(tp);
}


void ThreadWindows::make_default() {

	create_func=create_func_windows;
	get_thread_ID_func=get_thread_ID_func_windows;
	wait_to_finish_func=wait_to_finish_func_windows;
	
}

ThreadWindows::ThreadWindows() {

	handle=NULL;
}


ThreadWindows::~ThreadWindows() {

}


#endif

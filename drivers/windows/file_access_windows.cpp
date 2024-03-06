/*************************************************************************/
/*  file_access_windows.cpp                                              */
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
#ifdef WINDOWS_ENABLED

#include "file_access_windows.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <wchar.h>
#include <tchar.h>
#include "print_string.h"

#ifdef _MSC_VER
 #define S_ISREG(m) ((m)&_S_IFREG)
#endif

void FileAccessWindows::check_errors() const {

	ERR_FAIL_COND(!f);

	if (feof(f)) {

		last_error=ERR_FILE_EOF;;
	}

}

Error FileAccessWindows::_open(const String& p_filename, int p_mode_flags) {

	printf("open1 %ls\n", p_filename.c_str());
	String filename=fix_path(p_filename);
	printf("open2 %ls\n", filename.c_str());
	String file_c = filename.replace("/", "\\");
	if (f)
		close();


#ifdef WIN98_ENABLED
	const char* mode_string;

	if (p_mode_flags==READ)
		mode_string="rb";
	else if (p_mode_flags==WRITE)
		mode_string="wb";
	else if (p_mode_flags==READ_WRITE)
		mode_string="wb+";
	else
		return ERR_INVALID_PARAMETER;
#else
	const wchar_t* mode_string;

	if (p_mode_flags==READ)
		mode_string=L"rb";
	else if (p_mode_flags==WRITE)
		mode_string=L"wb";
	else if (p_mode_flags==READ_WRITE)
		mode_string=L"wb+";
	else
		return ERR_INVALID_PARAMETER;
#endif

	/* pretty much every implementation that uses fopen as primary
	   backend supports utf8 encoding */

#ifdef WIN98_ENABLED
	struct stat st;
	if (stat(filename.replace("/", "\\").utf8().get_data(), &st) == 0) {
#else
	struct _stat st;
	if (_wstat(filename.replace("/", "\\").c_str(), &st) == 0) {
#endif

		if (!S_ISREG(st.st_mode))
			return ERR_FILE_CANT_OPEN;

	};

	if (is_backup_save_enabled() && p_mode_flags&WRITE && !(p_mode_flags&READ)) {
		save_path=filename;
		filename=filename+".tmp";
		//print_line("saving instead to "+path);
	}

#ifdef WIN98_ENABLED
	f=fopen(file_c.utf8().get_data(), mode_string);
#else
	f=_wfopen(file_c.c_str(), mode_string);
#endif
	printf("result for %ls = %p\n", file_c.c_str(), (void *)f);


	if (f==NULL) {
		last_error=ERR_FILE_CANT_OPEN;
		return ERR_FILE_CANT_OPEN;
	} else {
		last_error=OK;
		flags=p_mode_flags;
		return OK;
	}

}
void FileAccessWindows::close() {

	if (!f)
		return;

	fclose(f);
	f = NULL;

	if (save_path!="") {

		//unlink(save_path.utf8().get_data());
		//print_line("renaming..");
#ifdef WIN98_ENABLED
		unlink(save_path.utf8().get_data()); //unlink if exists
		int rename_error = rename((save_path+".tmp").utf8().get_data(),save_path.utf8().get_data());
#else
		_wunlink(save_path.c_str()); //unlink if exists
		int rename_error = _wrename((save_path+".tmp").c_str(),save_path.c_str());
#endif
		save_path="";
		ERR_FAIL_COND( rename_error != 0);
	}


}
bool FileAccessWindows::is_open() const {

	return (f!=NULL);
}
void FileAccessWindows::seek(size_t p_position) {

	ERR_FAIL_COND(!f);
	last_error=OK;
	if ( fseek(f,p_position,SEEK_SET) )
		check_errors();
}
void FileAccessWindows::seek_end(int64_t p_position) {

	ERR_FAIL_COND(!f);
	if ( fseek(f,p_position,SEEK_END) )
		check_errors();
}
size_t FileAccessWindows::get_pos() const {


	size_t aux_position=0;
	if ( !(aux_position = ftell(f)) ) {
		check_errors();
	};
	return aux_position;
}
size_t FileAccessWindows::get_len() const {


	ERR_FAIL_COND_V(!f,0);

	size_t pos = get_pos();
	fseek(f,0,SEEK_END);
	int size = get_pos();
	fseek(f, pos, SEEK_SET);

	return size;
}

bool FileAccessWindows::eof_reached() const {

	check_errors();
	return last_error==ERR_FILE_EOF;
}

uint8_t FileAccessWindows::get_8() const {

	ERR_FAIL_COND_V(!f,0);
	uint8_t b;
	if (fread(&b,1,1,f) == 0) {
		check_errors();
	};

	return b;
}

int FileAccessWindows::get_buffer(uint8_t *p_dst, int p_length) const {

	ERR_FAIL_COND_V(!f,-1);
	int read = fread(p_dst, 1,p_length, f);
	check_errors();
	return read;
};


Error FileAccessWindows::get_error() const {

	return last_error;
}

void FileAccessWindows::store_8(uint8_t p_dest) {

	ERR_FAIL_COND(!f);
	fwrite(&p_dest,1,1,f);

}


bool FileAccessWindows::file_exists(const String& p_name) {

	FILE *g;
	String filename=fix_path(p_name);
	String cfname = filename.replace("/", "\\");
#ifdef WIN98_ENABLED
	g=fopen(cfname.utf8().get_data(),"rb");
#else
	g=_wfopen(cfname.c_str(),L"rb");
#endif
	printf("result for %ls = %p\n", cfname.c_str(), g);
	if (g==NULL) {

		return false;
	} else {

		fclose(g);
		return true;
	}
}

uint64_t FileAccessWindows::_get_modified_time(const String& p_file) {

	String file=fix_path(p_file);
	if (file.ends_with("/") && file!="/")
		file=file.substr(0,file.length()-1);

#ifdef WIN98_ENABLED
	struct stat st;
	int rv = stat(file.replace("/", "\\").utf8().get_data(), &st);
#else
	struct _stat st;
	int rv = _wstat(file.replace("/", "\\").c_str(), &st);
#endif

	if (rv == 0) {

		return st.st_mtime;
	} else {
		print_line("no access to "+file );
	}


	ERR_FAIL_V(0);
};


FileAccessWindows::FileAccessWindows() {

	f=NULL;
	flags=0;
	last_error=OK;

}
FileAccessWindows::~FileAccessWindows() {

	close();

}

#endif

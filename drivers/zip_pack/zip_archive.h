/*************************************************************************/
/*  file_access_zip.h                                                    */
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
#ifdef MINIZIP_ENABLED

#ifndef FILE_ACCESS_Zip_H
#define FILE_ACCESS_Zip_H

#include "core/io/file_access_pack.h"
#include "io/zip_io.h"
#include "core/map.h"

class ZipArchive : public PackSource {
public:
	struct File {
		int package;
		unz_file_pos file_pos;
		File() {
			package = -1;
		};
	};
private:
	struct Package {
		String filename;
		unzFile zfile;
	};
	Vector<Package> packages;

	Map<String,File> files;

	static ZipArchive* instance;

	FileAccess::CreateFunc fa_create_func;
public:
	void close_handle(unzFile p_file) const;
	unzFile get_file_handle(String p_file) const;

	Error add_package(String p_name);

	virtual bool try_open_pack(const String& p_path, bool p_replace_files = true);
	virtual FileAccess* get_file(const String& p_path) const;
	virtual FileStatus has_file(const String& p_path) const;

	virtual String get_pack_extension() const;
#ifdef TOOLS_ENABLED
	virtual String get_pack_name() const;
	virtual Error export_pack(FileAccess *p_destination, Vector<FileExportData> p_files, PackingProgressCallback p_progress);
#endif

	static ZipArchive* get_singleton();

	ZipArchive();
	~ZipArchive();
};


class FileAccessZip : public FileAccess {
	unzFile zfile;
	unz_file_info64	file_info;

	mutable bool at_eof;

	ZipArchive* archive;
public:
	virtual Error _open(const String& p_path, int p_mode_flags); ///< open a file
	virtual void close(); ///< close a file
	virtual bool is_open() const; ///< true when file is open

	virtual void seek(size_t p_position); ///< seek to a given position
	virtual void seek_end(int64_t p_position=0); ///< seek from the end of file
	virtual size_t get_pos() const; ///< get position in the file
	virtual size_t get_len() const; ///< get size of the file

	virtual bool eof_reached() const; ///< reading passed EOF

	virtual uint8_t get_8() const; ///< get a byte
	virtual int get_buffer(uint8_t *p_dst,int p_length) const;

	virtual Error get_error() const; ///< get last error

	virtual void store_8(uint8_t p_dest); ///< store a byte
	virtual bool file_exists(const String& p_name); ///< return true if a file exists

	virtual uint64_t _get_modified_time(const String& p_file) { return 0; } // todo

	FileAccessZip(const String& p_path);
	~FileAccessZip();
};

#endif // FILE_ACCESS_ZIP_H

#endif

/**************************************************************************/
/*  resource_format_text.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef RESOURCE_FORMAT_TEXT_H
#define RESOURCE_FORMAT_TEXT_H

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/file_access.h"

#include "variant_parser.h"
#include "scene/resources/packed_scene.h"

class ResourceInteractiveLoaderText : public ResourceInteractiveLoader {

	String local_path;
	String res_path;
	String error_text;

	FileAccess *f;

	VariantParser::StreamFile stream;

	struct ExtResource {
		String path;
		String type;
	};

	bool is_scene;
	String res_type;

	bool ignore_resource_parsing;

	//	Map<String,String> remaps;

	Map<int, ExtResource> ext_resources;

	int resources_total;
	int resource_current;
	String resource_type;

	VariantParser::Tag next_tag;

	mutable int lines;

	Map<String, String> remaps;
	//void _printerr();

	static Error _parse_sub_resources(void *p_self, VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceInteractiveLoaderText *>(p_self)->_parse_sub_resource(p_stream, r_value_res, line, r_err_str); }
	static Error _parse_ext_resources(void *p_self, VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceInteractiveLoaderText *>(p_self)->_parse_ext_resource(p_stream, r_value_res, line, r_err_str); }

	Error _parse_sub_resource(VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str);
	Error _parse_ext_resource(VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str);

	VariantParser::ResourceParser rp;

	static Error _parse_sub_resources_export(void *p_self, VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceInteractiveLoaderText *>(p_self)->_parse_sub_resource_export(p_stream, r_value_res, line, r_err_str); }
	static Error _parse_ext_resources_export(void *p_self, VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str) { return reinterpret_cast<ResourceInteractiveLoaderText *>(p_self)->_parse_ext_resource_export(p_stream, r_value_res, line, r_err_str); }

	Error _parse_sub_resource_export(VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str);
	Error _parse_ext_resource_export(VariantParser::Stream *p_stream, Variant &r_value_res, int &line, String &r_err_str);

	VariantParser::ResourceParser rp_export;

	Ref<PackedScene> packed_scene;

	friend class ResourceFormatLoaderText;

	List<RES> resource_cache;
	Error error;

	RES resource;

public:
	virtual void set_local_path(const String& p_local_path);
	virtual Ref<Resource> get_resource();
	virtual Error poll();
	virtual int get_stage() const;
	virtual int get_stage_count() const;
	virtual Error wait();

	~ResourceInteractiveLoaderText();
};

class ResourceFormatLoaderText : public ResourceFormatLoader {
public:

	virtual Ref<ResourceInteractiveLoader> load_interactive(const String &p_path);
	virtual RES load(const String &p_path,const String& p_original_path="");
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual void get_recognized_extensions_for_type(const String& p_type,List<String> *p_extensions) const;
	bool recognize(const String& p_extension) const;
	virtual bool handles_type(const String& p_type) const;
	virtual String get_resource_type(const String &p_path) const;
	virtual void get_dependencies(const String& p_path,List<String> *p_dependencies);
	virtual Error load_import_metadata(const String &p_path, Ref<ResourceImportMetadata>& r_var) const;

	static ResourceFormatLoaderText *singleton;
	ResourceFormatLoaderText() { singleton = this; }
};

class ResourceFormatSaverTextInstance {

	String local_path;

	Ref<PackedScene> packed_scene;

	bool takeover_paths;
	bool relative_paths;
	bool bundle_resources;
	bool skip_editor;
	FileAccess *f;
	Set<RES> resource_set;
	List<RES> saved_resources;
	Map<RES, int> external_resources;
	Map<RES, int> internal_resources;

	void _find_resources(const Variant &p_variant, bool p_main = false);

	static String _write_resources(void *ud, const RES &p_resource);
	String _write_resource(const RES &res);

public:
	Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
};

class ResourceFormatSaverText : public ResourceFormatSaver {
public:
	static ResourceFormatSaverText *singleton;
	virtual Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
	virtual bool recognize(const RES &p_resource) const;
	virtual void get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const;

	ResourceFormatSaverText();
};

#endif // RESOURCE_FORMAT_TEXT_H

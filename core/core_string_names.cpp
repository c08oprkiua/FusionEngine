/*************************************************************************/
/*  core_string_names.cpp                                                */
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
#include "core_string_names.h"

namespace CoreStringNames {

StringName _free;
StringName changed;
StringName _meta;
StringName _script;
StringName script_changed;
StringName ___pdcdata;
StringName __getvar;
StringName _iter_init;
StringName _iter_next;
StringName _iter_get;

void create() {
	_free=StaticCString::create("free");
	changed=StaticCString::create("changed");
	_meta=StaticCString::create("__meta__");
	_script=StaticCString::create("script/script");
	script_changed=StaticCString::create("script_changed");
	___pdcdata=StaticCString::create("___pdcdata");
	__getvar=StaticCString::create("__getvar");
	_iter_init=StaticCString::create("_iter_init");
	_iter_next=StaticCString::create("_iter_next");
	_iter_get=StaticCString::create("_iter_get");
}

}

/*************************************************************************/
/*  types/ustring.h                                                            */
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
#ifndef RSTRING_H
#define RSTRING_H

#include "typedefs.h"
#include "vector.h"

#ifdef WINDOWS_ENABLED
#include "tchar.h"
#endif

#include <cstring>

/**
	@author red <red@killy>
*/

#define COMPILE_TIME_HASH_STRINGS 1

//Gotta use recursion to make this C++11 compliant, unfortunately.
//IMPORTANT: Do NOT supply the last two args.
//Well, ig you could change the last one cause it's just a seed, but
constexpr uint32_t constexpr_hash(const char *p_cstr, int p_countup = 0, uint32_t p_ret_hash = 5381){
	return p_cstr[p_countup] ?
	constexpr_hash(p_cstr, p_countup + 1,
				   (((p_ret_hash << 5) + p_ret_hash) + p_cstr[p_countup])
				   )
	:
	p_ret_hash
	;
}

class CharString : public Vector<char> {
public:	
	int length() const { return size() ? size()-1 : 0; }
	const char *get_data() const;
	operator const char*() {return get_data();};
};

#ifndef CHARTYPE_16BITS
typedef wchar_t CharType;
#else
typedef wchar_t uint16_t;
#endif

struct StrRange {

	const CharType *c_str;
	int len;

	StrRange(const CharType *p_c_str=NULL,int p_len=0) { c_str=p_c_str; len=p_len; }

};

class String : public Vector<CharType> {

	void copy_from(const char *p_cstr);
	void copy_from(const CharType* p_cstr, int p_clip_to=-1);
	void copy_from(const CharType& p_char);


public:

	enum {

		npos=-1 ///<for "some" compatibility with std::string (npos is a huge value in std::string)
	};


	bool operator==(const String& p_str) const;
	bool operator!=(const String& p_str) const;
	String operator+(const String& p_str) const;
	//String operator+(CharType p_char) const;

	String& operator+=(const String &);
	String& operator+=(CharType p_str);
	String& operator+=(const char * p_str);
	String& operator+=(const CharType * p_str);

	/* Compatibility Operators */


	void operator=(const char *p_str);
	void operator=(const CharType *p_str);
	bool operator==(const char *p_str) const;
	bool operator==(const CharType *p_str) const;
	bool operator==(const StrRange &p_str_range) const;
	bool operator!=(const char *p_str) const;
	bool operator!=(const CharType *p_str) const;
	bool operator<(const CharType *p_str) const;
	bool operator<(const char *p_str) const;
	bool operator<(String p_str) const;
	bool operator<=(String p_str) const;

	signed char casecmp_to(const String& p_str) const;
	signed char nocasecmp_to(const String& p_str) const;

	const CharType * c_str() const;
	/* standard size stuff */

	int length() const;

	/* complex helpers */
	String substr(int p_from,int p_chars) const;
	int find(String p_str,int p_from=0) const; ///< return <0 if failed
	int find_last(String p_str) const; ///< return <0 if failed
	int findn(String p_str,int p_from=0) const; ///< return <0 if failed, case insensitive
	int rfind(String p_str,int p_from=-1) const; ///< return <0 if failed
	int rfindn(String p_str,int p_from=-1) const; ///< return <0 if failed, case insensitive
	int findmk(const Vector<String>& p_keys,int p_from=0,int *r_key=NULL) const; ///< return <0 if failed
	bool match(const String& p_wildcard) const;
	bool matchn(const String& p_wildcard) const;
	bool begins_with(const String& p_string) const;
	bool begins_with(const char* p_string) const;	
	bool ends_with(const String& p_string) const;
	String replace_first(String p_key,String p_with) const;
	String replace(String p_key,String p_with) const;
	String replacen(String p_key,String p_with) const;
	String insert(int p_at_pos,String p_string) const;
	String pad_decimals(int p_digits) const;
	String pad_zeros(int p_digits) const;
	static String num(double p_num,int p_decimals=-1);
	static String num_scientific(double p_num);
	static String num_real(double p_num);
	static String num_int64(int64_t p_num);
	static String chr(CharType p_char);
	static String md5(const uint8_t *p_md5);
	bool is_numeric() const;
	double to_double() const;
	float to_float() const;
	int hex_to_int() const;
	int to_int() const;

	int64_t to_int64() const;
	static int to_int(const char* p_str);
	static double to_double(const char* p_str);
	static double to_double(const CharType* p_str, int p_len=-1, const CharType **r_end=NULL);
	static int64_t to_int(const CharType* p_str,int p_len=-1);
	String capitalize() const;

	int get_slice_count(String p_splitter) const;
	String get_slice(String p_splitter,int p_slice) const;

	Vector<String> split(const String &p_splitter,bool p_allow_empty=true) const;
	Vector<String> split_spaces() const;
	Vector<float> split_floats(const String &p_splitter,bool p_allow_empty=true) const;
	Vector<float> split_floats_mk(const Vector<String> &p_splitters,bool p_allow_empty=true) const;
	Vector<int> split_ints(const String &p_splitter,bool p_allow_empty=true) const;
	Vector<int> split_ints_mk(const Vector<String> &p_splitters,bool p_allow_empty=true) const;

	static CharType char_uppercase(CharType p_char);
	static CharType char_lowercase(CharType p_char);
	String to_upper() const;
	String to_lower() const;

	String left(int p_pos) const;
	String right(int p_pos) const;
	String strip_edges() const;
	String strip_escapes() const;
	String extension() const;
	String basename() const;
	String plus_file(const String& p_file) const;
	CharType ord_at(int p_idx) const;

	void erase(int p_pos, int p_chars);

	CharString ascii(bool p_allow_extended=false) const;
	CharString utf8() const;
	bool parse_utf8(const char* p_utf8,int p_len=-1); //return true on error
	static String utf8(const char* p_utf8,int p_len=-1);

#ifdef WINDOWS_ENABLED
//#ifndef _UNICODE
//	mutable CharString ascii_;
//	mutable bool has_ascii_;
//#endif
	_FORCE_INLINE_ const TCHAR *t_str() const {
#ifdef _UNICODE
		return c_str();
#else
#error "not implemented yet"
		CharString stage = ascii();
		if (!has_ascii_ || strcmp(ascii_.get_data(), stage.get_data()) != 0) {
			ascii_ = stage;
			has_ascii_ = true;
		}
		const char *r = ascii_.get_data();
		ERR_FAIL_NULL_V(r, NULL);
		return r;
#endif
	}
#endif
	
	static uint32_t hash(const CharType* p_str,int p_len); /* hash the string */
	static uint32_t hash(const CharType* p_str); /* hash the string */
	static uint32_t hash(const char* p_cstr,int p_len); /* hash the string */

	constexpr static uint32_t hash(const char* p_cstr){
#ifdef COMPILE_TIME_HASH_STRINGS
	return constexpr_hash(p_cstr);
#else
	uint32_t hashv = 5381;
	uint32_t c = 0;

	while ((c = *p_cstr++))
		hashv = ((hashv << 5) + hashv) + c; /* hash * 33 + c */

	return hashv;
#endif
	} /* hash the string */
	uint32_t hash() const; /* hash the string */
	uint64_t hash64() const; /* hash the string */
	String md5_text() const;
	Vector<uint8_t> md5_buffer() const;

	inline bool empty() const { return length() == 0; }	

	// path functions
	bool is_abs_path() const;
	bool is_rel_path() const;
	bool is_resource_file() const;
	String path_to(const String& p_path) const;
	String path_to_file(const String& p_path) const;
	String get_base_dir() const;
	String get_file() const;
	static String humanize_size(size_t p_size);
	String simplify_path() const;

	String xml_escape(bool p_escape_quotes=false) const;
	String xml_unescape() const;
	String c_escape() const;
	String c_unescape() const;

	String percent_encode() const;
	String percent_decode() const;

	bool is_valid_identifier() const;
	bool is_valid_integer() const;
	bool is_valid_float() const;
	bool is_valid_html_color() const;
	bool is_valid_ip_address() const;

	/**
	 * The constructors must not depend on other overloads
	 */
/*	String(CharType p_char);*/
	inline String() {
#ifdef WINDOWS_ENABLED
#ifndef _UNICODE
		has_ascii_ = false;
#endif
#endif
	}
	String(const char *p_str);
	String(const CharType *p_str,int p_clip_to_len=-1);
	String(const StrRange& p_range);

};

bool operator==(const char *p_chr, const String &p_str);

String operator+(const char *p_chr, const String &p_str);
String operator+(CharType p_chr, const String &p_str);

String itos(int64_t p_val);
String rtos(double p_val);
String rtoss(double p_val); //scientific version


struct NoCaseComparator {
		
	bool operator()(const String &p_a, const String &p_b) const {
		
		return p_a.nocasecmp_to(p_b)<0;
	}
};

/* end of namespace */


#endif

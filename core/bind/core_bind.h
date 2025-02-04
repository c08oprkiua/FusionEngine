#ifndef CORE_BIND_H
#define CORE_BIND_H

#include "io/resource_loader.h"
#include "io/resource_saver.h"
#include "os/file_access.h"
#include "os/dir_access.h"
#include "os/thread.h"
#include "os/semaphore.h"
#include "scene/resources/texture.h"

class _ResourceLoader : public Object  {
	OBJ_TYPE(_ResourceLoader,Object);

protected:

	static void _bind_methods();
	static _ResourceLoader *singleton;
public:


	static _ResourceLoader *get_singleton() { return singleton; }
	Ref<ResourceInteractiveLoader> load_interactive(const String& p_path,const String& p_type_hint="");
	RES load(const String &p_path,const String& p_type_hint="");
	DVector<String> get_recognized_extensions_for_type(const String& p_type);
	void set_abort_on_missing_resources(bool p_abort);
	PackedStringArray get_dependencies(const String& p_path);
	bool has(const String& p_path);

	_ResourceLoader();
};

class _ResourceSaver : public Object  {
	OBJ_TYPE(_ResourceSaver,Object);

protected:

	static void _bind_methods();
	static _ResourceSaver *singleton;
public:

	enum SaverFlags {

		FLAG_RELATIVE_PATHS=1,
		FLAG_BUNDLE_RESOURCES=2,
		FLAG_CHANGE_PATH=4,
		FLAG_OMIT_EDITOR_PROPERTIES=8,
		FLAG_SAVE_BIG_ENDIAN=16,
		FLAG_COMPRESS=32,
	};

	static _ResourceSaver *get_singleton() { return singleton; }

	Error save(const String &p_path,const RES& p_resource, uint32_t p_flags);
	DVector<String> get_recognized_extensions(const RES& p_resource);


	_ResourceSaver();
};

class MainLoop;

class _OS : public Object  {
	OBJ_TYPE(_OS,Object);

protected:

	static void _bind_methods();
	static _OS *singleton;
public:

	enum Weekday {
		DAY_SUNDAY,
		DAY_MONDAY,
		DAY_TUESDAY,
		DAY_WEDNESDAY,
		DAY_THURSDAY,
		DAY_FRIDAY,
		DAY_SATURDAY
	};

	enum Month {
		MONTH_JANUARY,
		MONTH_FEBRUARY,
		MONTH_MARCH,
		MONTH_APRIL,
		MONTH_MAY,
		MONTH_JUNE,
		MONTH_JULY,
		MONTH_AUGUST,
		MONTH_SEPTEMBER,
		MONTH_OCTOBER,
		MONTH_NOVEMBER,
		MONTH_DECEMBER
	};

	Point2 get_mouse_pos() const;
	void set_window_title(const String& p_title);
	int get_mouse_button_state() const;


	void set_clipboard(const String& p_text);
	String get_clipboard() const;

	void set_video_mode(const Size2& p_size, bool p_fullscreen,bool p_resizeable,int p_screen=0);
	Size2 get_video_mode(int p_screen=0) const;
	bool is_video_mode_fullscreen(int p_screen=0) const;
	bool is_video_mode_resizable(int p_screen=0) const;
	Array get_fullscreen_mode_list(int p_screen=0) const;

	Error native_video_play(String p_path, float p_volume, String p_audio_track, String p_subtitle_track);
	bool native_video_is_playing();
	void native_video_pause();
	void native_video_stop();

	void set_iterations_per_second(int p_ips);
	int get_iterations_per_second() const;

	void set_target_fps(int p_fps);
	float get_target_fps() const;

	void set_low_processor_usage_mode(bool p_enabled);
	bool is_in_low_processor_usage_mode() const;

	String get_executable_path() const;
	int execute(const String& p_path, const Vector<String> & p_arguments,bool p_blocking,Array p_output=Array());

	Error kill(int p_pid);
	Error shell_open(String p_uri);

	int get_process_ID() const;

	bool has_environment(const String& p_var) const;
	String get_environment(const String& p_var) const;

	String get_name() const;
	Vector<String> get_cmdline_args();

	String get_locale() const;
	String get_model_name() const;
	MainLoop *get_main_loop() const;

	String get_custom_level() const;

	float get_frames_per_second() const;

	void dump_memory_to_file(const String& p_file);
	void dump_resources_to_file(const String& p_file);

	void print_resources_in_use(bool p_short=false);
	void print_all_resources(const String& p_to_file);
	void print_all_textures_by_size();
	void print_resources_by_type(const Vector<String>& p_types);

	bool has_touchscreen_ui_hint() const;

	bool is_debug_build() const;

	String get_unique_ID() const;

	/*
	struct Date {

		int year;
		Month month;
		int day;
		Weekday weekday;
		bool dst;
	};

	struct Time {

		int hour;
		int min;
		int sec;
	};
*/

	void set_use_file_access_save_and_swap(bool p_enable);

	void set_icon(const Image& p_icon);
	Dictionary get_date() const;
	Dictionary get_time() const;
	uint64_t get_unix_time() const;
	
	bool get_camera_enabled();
	Ref<ImageTexture> get_camera_image();
	void set_camera_enabled(bool p_enabled);

	int get_static_memory_usage() const;
	int get_static_memory_peak_usage() const;
	int get_dynamic_memory_usage() const;

	void delay_usec(uint32_t p_usec) const;
	void delay_msec(uint32_t p_msec) const;
	uint32_t get_ticks_msec() const;

	bool can_use_threads() const;

	bool can_draw() const;

	int  get_frames_drawn();

	bool is_stdout_verbose() const;

	int get_processor_count() const;

	enum SystemDir {
		SYSTEM_DIR_DESKTOP,
		SYSTEM_DIR_DCIM,
		SYSTEM_DIR_DOCUMENTS,
		SYSTEM_DIR_DOWNLOADS,
		SYSTEM_DIR_MOVIES,
		SYSTEM_DIR_MUSIC,
		SYSTEM_DIR_PICTURES,
		SYSTEM_DIR_RINGTONES,
	};

	String get_system_dir(SystemDir p_dir) const;


	String get_data_dir() const;

	void set_time_scale(float p_scale);
	float get_time_scale();

	static _OS *get_singleton() { return singleton; }

	_OS();
};

VARIANT_ENUM_CAST(_OS::SystemDir);


class _Geometry : public Object {

	OBJ_TYPE(_Geometry, Object);

	static _Geometry *singleton;
protected:

	static void _bind_methods();
public:

	static _Geometry *get_singleton();
	DVector<Plane> build_box_planes(const Vector3& p_extents);
	DVector<Plane> build_cylinder_planes(float p_radius, float p_height, int p_sides, Vector3::Axis p_axis=Vector3::AXIS_Z);
	DVector<Plane> build_capsule_planes(float p_radius, float p_height, int p_sides, int p_lats, Vector3::Axis p_axis=Vector3::AXIS_Z);
	Variant segment_intersects_segment_2d(const Vector2& p_from_a,const Vector2& p_to_a,const Vector2& p_from_b,const Vector2& p_to_b);
	DVector<Vector2> get_closest_points_between_segments_2d( const Vector2& p1,const Vector2& q1, const Vector2& p2,const Vector2& q2);
	DVector<Vector3> get_closest_points_between_segments(const Vector3& p1,const Vector3& p2,const Vector3& q1,const Vector3& q2);
	Vector3 get_closest_point_to_segment(const Vector3& p_point, const Vector3& p_a,const Vector3& p_b);
	Variant ray_intersects_triangle( const Vector3& p_from, const Vector3& p_dir, const Vector3& p_v0,const Vector3& p_v1,const Vector3& p_v2);
	Variant segment_intersects_triangle( const Vector3& p_from, const Vector3& p_to, const Vector3& p_v0,const Vector3& p_v1,const Vector3& p_v2);
	DVector<Vector3> segment_intersects_sphere( const Vector3& p_from, const Vector3& p_to, const Vector3& p_sphere_pos,real_t p_sphere_radius);
	DVector<Vector3> segment_intersects_cylinder( const Vector3& p_from, const Vector3& p_to, float p_height,float p_radius);
	DVector<Vector3> segment_intersects_convex(const Vector3& p_from, const Vector3& p_to,const Vector<Plane>& p_planes);
	real_t segment_intersects_circle(const Vector2& p_from, const Vector2& p_to, const Vector2& p_circle_pos, real_t p_circle_radius);
	int get_uv84_normal_bit(const Vector3& p_vector);

	Vector<int> triangulate_polygon(const Vector<Vector2>& p_polygon);

	Dictionary make_atlas(const Vector<Size2>& p_rects);

	_Geometry();
};




class _File : public RefCounted {

	OBJ_TYPE(_File,RefCounted);
	FileAccess *f;
	bool eswap;
protected:

	static void _bind_methods();
public:

	enum ModeFlags  {

		READ=1,
		WRITE=2,
		READ_WRITE=3,
	};

	Error open_encrypted(const String& p_path, int p_mode_flags,const Vector<uint8_t>& p_key);
	Error open_encrypted_pass(const String& p_path, int p_mode_flags,const String& p_pass);


	Error open(const String& p_path, int p_mode_flags); ///< open a file
	void close(); ///< close a file
	bool is_open() const; ///< true when file is open

	void seek(int64_t p_position); ///< seek to a given position
	void seek_end(int64_t p_position=0); ///< seek from the end of file
	int64_t get_pos() const; ///< get position in the file
	int64_t get_len() const; ///< get size of the file

	bool eof_reached() const; ///< reading passed EOF

	uint8_t get_8() const; ///< get a byte
	uint16_t get_16() const; ///< get 16 bits uint
	uint32_t get_32() const; ///< get 32 bits uint
	uint64_t get_64() const; ///< get 64 bits uint

	float get_float() const;
	double get_double() const;
	real_t get_real() const;

	Variant get_var() const;

	DVector<uint8_t> get_buffer(int p_length) const; ///< get an array of bytes
	String get_line() const;
	String get_as_text() const;

	/**< use this for files WRITTEN in _big_ endian machines (ie, amiga/mac)
	 * It's not about the current CPU type but file formats.
	 * this flags get reset to false (little endian) on each open
	 */

	void set_endian_swap(bool p_swap);
	bool get_endian_swap();

	Error get_error() const; ///< get last error

	void store_8(uint8_t p_dest); ///< store a byte
	void store_16(uint16_t p_dest); ///< store 16 bits uint
	void store_32(uint32_t p_dest); ///< store 32 bits uint
	void store_64(uint64_t p_dest); ///< store 64 bits uint

	void store_float(float p_dest);
	void store_double(double p_dest);
	void store_real(real_t p_real);

	void store_string(const String& p_string);
	void store_line(const String& p_string);

	virtual void store_pascal_string(const String& p_string);
	virtual String get_pascal_string();

	Vector<String> get_csv_line() const;


	void store_buffer(const DVector<uint8_t>& p_buffer); ///< store an array of bytes

	void store_var(const Variant& p_var);

	bool file_exists(const String& p_name) const; ///< return true if a file exists

	_File();
	virtual ~_File();

};

class _Directory : public RefCounted {

	OBJ_TYPE(_Directory,RefCounted);
	DirAccess *d;
protected:

	static void _bind_methods();
public:

	Error open(const String& p_path);

	bool list_dir_begin(); ///< This starts dir listing
	String get_next();
	bool current_is_dir() const;

	void list_dir_end(); ///<

	int get_drive_count();
	String get_drive(int p_drive);

	Error change_dir(String p_dir); ///< can be relative or absolute, return false on success
	String get_current_dir(); ///< return current dir location

	Error make_dir(String p_dir);
	Error make_dir_recursive(String p_dir);

	bool file_exists(String p_file);
	bool dir_exists(String p_dir);

	int get_space_left();

	Error copy(String p_from,String p_to);
	Error rename(String p_from, String p_to);
	Error remove(String p_name);


	_Directory();
	virtual ~_Directory();

};

class _Marshalls : public RefCounted {

	OBJ_TYPE(_Marshalls,RefCounted);

protected:

	static void _bind_methods();


public:

	String variant_to_base64(const Variant& p_var);
	Variant base64_to_variant(const String& p_str);

	_Marshalls() {};
};


class _Mutex : public RefCounted {

	OBJ_TYPE(_Mutex,RefCounted);
	Mutex *mutex;

	static void _bind_methods();
public:

	void lock();
	Error try_lock();
	void unlock();

	_Mutex();
	~_Mutex();
};

class _Semaphore : public RefCounted {

	OBJ_TYPE(_Semaphore,RefCounted);
	Semaphore *semaphore;

	static void _bind_methods();
public:

	Error wait();
	Error post();

	_Semaphore();
	~_Semaphore();
};

class _Thread : public RefCounted {

	OBJ_TYPE(_Thread,RefCounted);

protected:

	Variant ret;
	Variant userdata;
	volatile bool active;
	Object *target_instance;
	StringName target_method;
	Thread *thread;
	static void _bind_methods();
	static void _start_func(void *ud);
public:

	enum Priority {

		PRIORITY_LOW,
		PRIORITY_NORMAL,
		PRIORITY_HIGH
	};

	Error start(Object *p_instance,const StringName& p_method,const Variant& p_userdata=Variant(),int p_priority=PRIORITY_NORMAL);
	String get_id() const;
	bool is_active() const;
	Variant wait_to_finish();

	_Thread();
	~_Thread();
};

#endif // CORE_BIND_H

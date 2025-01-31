#include "export.h"
#include "platform/psp/logo.h"
#include "tools/editor/editor_import_export.h"
#include "tools/editor/editor_settings.h"
#include "scene/resources/texture.h"
#include "core/os/os.h"
#include "core/os/file_access.h"
#include "core/os/dir_access.h"
#include "core/globals.h"


static const char *sdk_path_var = "psp/base_sdk_path";
static const char *pack_pbp_path = "/bin/pack-pbp";
static const char *mksfo_path = "/bin/mksfo";

class EditorExportPlatformPSP : public EditorExportPlatform {
    OBJ_TYPE(EditorExportPlatformPSP, EditorExportPlatform);
private:

public:

	void _get_property_list( List<PropertyInfo> *p_list) const;
	bool _set(const StringName& p_name, const Variant& p_value);
	bool _get(const StringName& p_name,Variant &r_ret) const;

	virtual String get_name() const;
	virtual ImageCompression get_image_compression() const;
	virtual Ref<Texture> get_logo() const;

	virtual bool poll_devices() { return false; }
	virtual int get_device_count() const { return 0; }
	virtual String get_device_name(int p_device) const { return ""; }
	virtual String get_device_info(int p_device) const { return ""; }
	virtual Error run(int p_device,bool p_dumb=false) { return OK; }

	virtual bool can_export(String *r_error=NULL) const;

	virtual bool requires_password(bool p_debug) const { return false; }
	virtual String get_binary_extension() const;
	virtual Error export_project(const String& p_path,bool p_debug,bool p_dumb=false);
};

void EditorExportPlatformPSP::_get_property_list( List<PropertyInfo> *p_list) const{
	p_list->push_back(PropertyInfo(Variant::BOOL, "data/embed_pck"));
	p_list->push_back(PropertyInfo(Variant::STRING, "launcher/background", PROPERTY_HINT_FILE, "png"));
	p_list->push_back(PropertyInfo(Variant::STRING, "launcher/icon", PROPERTY_HINT_FILE, "png"));
	p_list->push_back(PropertyInfo(Variant::STRING, "launcher/sound", PROPERTY_HINT_FILE, "at3"));
}

bool EditorExportPlatformPSP::_set(const StringName& p_name, const Variant& p_value){

}

bool EditorExportPlatformPSP::_get(const StringName& p_name,Variant &r_ret) const{

}


String EditorExportPlatformPSP::get_name() const {
    return "Playstation Portable";
}

EditorExportPlatform::ImageCompression EditorExportPlatformPSP::get_image_compression() const{
    return ImageCompression::IMAGE_COMPRESSION_NONE;
}

Ref<Texture> EditorExportPlatformPSP::get_logo() const {
	Image img(_psp_logo);
	Ref<ImageTexture> logo = memnew(ImageTexture);
	logo->create_from_image(img);
    return logo;
}

bool EditorExportPlatformPSP::can_export(String *r_error) const {
	String err = "";

	bool valid = true;

	String sdk_path = EditorSettings::get_singleton()->get(sdk_path_var);

	Error dir_err;
	DirAccess::open(sdk_path, &dir_err);
	if (dir_err != OK){
		valid = false;
		err += "Can't open the PSPSDK path, please set this in Editor Settings.\n";
	} else {
		if (not FileAccess::exists(sdk_path + mksfo_path)){
			valid = false;
			err += "Can't find mksfo in the PSPSDK.\n";
		}
		if (not FileAccess::exists(sdk_path + pack_pbp_path)){
			valid = false;
			err += "Can't find pack-pbp in the PSPSDK.\n";
		}
	}

	String exe_path = EditorSettings::get_singleton()->get_settings_path()+"/templates/";

	bool debug_found = FileAccess::exists(exe_path + "psp_debug.32");
	bool release_found = FileAccess::exists(exe_path + "psp_release.32");

	if (not debug_found or not release_found) {
		valid=false;
		err +="No export templates found.\nDownload and install export templates.\n";
	} else if (not debug_found){
		err += "No debug export template found.\n";
	} else if(not release_found){
		err += "No release export template found.\n";
	}

	//TODO: Validity checks on the icons and etc.

	if (r_error){
		*r_error=err;
	}

    return valid;
};

String EditorExportPlatformPSP::get_binary_extension() const {
    return "PBP";
};

Error EditorExportPlatformPSP::export_project(const String& p_path,bool p_debug,bool p_dumb){
	//This is not a complete implementation, just a start
	List<String> args;
	int retcode;
	Error run_err;

	String sdk_path = EditorSettings::get_singleton()->get(sdk_path_var);

	//Make PARAM.SFO

	String application_name = Globals::get_singleton()->get("application/name");

	if (application_name.empty()){
		application_name = "Fusion Engine";
	}

	args.push_back(application_name);

	String sfo_path = EditorSettings::get_singleton()->get_settings_path() + "/tmp/PARAM.SFO";

	args.push_back(sfo_path);

	run_err = OS::get_singleton()->execute(sdk_path + mksfo_path, args, true, NULL, NULL, &retcode);

	if (run_err or retcode != 0){
		print_line("Error in running mksfo: " + String::num(retcode));
		return ERR_CANT_CREATE;
	}

	retcode = 0;

	args.clear();

	//Make EBOOT.PBP

	args.push_back(p_path); //output.pbp
	args.push_back(sfo_path); //param.sfo

	args.push_back("NULL"); //icon0.png
	args.push_back("NULL"); //icon1.pmf
	args.push_back("NULL"); //pic0.png
	args.push_back("NULL"); //pic1.png
	args.push_back("NULL"); //snd0.at3

	String exe_path = EditorSettings::get_singleton()->get_settings_path()+"/templates/";

	if (p_debug){ //debug template
		args.push_back(exe_path + "psp_debug.32");
	} else { //release template
		args.push_back(exe_path + "psp_release.32");
	}
	args.push_back("NULL"); //data.psar

	run_err = OS::get_singleton()->execute(sdk_path + pack_pbp_path, args, true, NULL, NULL, &retcode);

	if (run_err or retcode != 0){
		print_line("Error in running pack-pbp: " + String::num(retcode));
		return ERR_CANT_CREATE;
	}

	return OK;
};

void register_psp_exporter(){
	EDITOR_DEF(sdk_path_var, "");
    Ref<EditorExportPlatformPSP> exporter = Ref<EditorExportPlatformPSP>(memnew(EditorExportPlatformPSP));
	EditorImportExport::get_singleton()->add_export_platform(exporter);
}

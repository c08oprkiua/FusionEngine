#include "export.h"
#include "platform/psp/logo.h"
#include "tools/editor/editor_import_export.h"
#include "tools/editor/editor_settings.h"
#include "scene/resources/texture.h"
#include "core/os/os.h"
#include "core/globals.h"

class EditorExportPlatformPSP : public EditorExportPlatform {
    OBJ_TYPE(EditorExportPlatformPSP, EditorExportPlatform);
private:

public:

	void _get_property_list( List<PropertyInfo> *p_list) const;

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
	p_list->push_back(PropertyInfo(Variant::STRING, "launcher/sound", PROPERTY_HINT_FILE, "mp3"));
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
	String err("PSP exporting is not set up yet!");
	r_error = &err;
    return false;
};

String EditorExportPlatformPSP::get_binary_extension() const {
    return "PBP";
};

Error EditorExportPlatformPSP::export_project(const String& p_path,bool p_debug,bool p_dumb){
	//This is not a complete implementation, just a start
	List<String> args;

	//Make PARAM.SFO
	args.push_back(Globals::get_singleton()->get("application/name"));

	String sfo_path = EditorSettings::get_singleton()->get_settings_path() + "/tmp/PARAM.SFO";

	args.push_back(sfo_path);

	OS::get_singleton()->execute("mksfo", args, true);

	args.clear();

	//Make EBOOT.PBP

	args.push_back(p_path + "EBOOT.PBP"); //output.pbp
	args.push_back(sfo_path); //param.sfo

	args.push_back("NULL"); //icon0.png
	args.push_back("NULL"); //icon1.pmf
	args.push_back("NULL"); //pic0.png
	args.push_back("NULL"); //pic1.png
	args.push_back("NULL"); //snd0.at3

	if (p_debug){ //debug template
		args.push_back("NULL");
	} else { //release template
		args.push_back("NULL");
	}
	args.push_back("NULL"); //data.psar

	OS::get_singleton()->execute("pack-pbp", args, true);
};

void register_psp_exporter(){
    Ref<EditorExportPlatformPSP> exporter = Ref<EditorExportPlatformPSP>(memnew(EditorExportPlatformPSP));
	EditorImportExport::get_singleton()->add_export_platform(exporter);
}

[CCode (cprefix = "UtilWrapper", lower_case_cprefix = "util_wrapper_")]
namespace UtilWrapper {
	[CCode (cheader_filename = "util-wrapper.h")]
	public bool is_empty_ssid (GLib.ByteArray ssid);
}

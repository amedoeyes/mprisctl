#ifndef MPRIS_COMMON_HPP
#define MPRIS_COMMON_HPP

#include <dbus/dbus.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

constexpr const char* dbus_interface_properties =
	"org.freedesktop.DBus.Properties";
constexpr const char* dbus_properties_method_get_all = "GetAll";
constexpr const char* dbus_properties_method_set = "Set";
constexpr const char* dbus_arg_volume = "Volume";
constexpr const char* mpris_name = "org.mpris.MediaPlayer2";
constexpr const char* mpris_path = "/org/mpris/MediaPlayer2";
constexpr const char* mpris_interface = "org.mpris.MediaPlayer2";
constexpr const char* mpris_interface_player = "org.mpris.MediaPlayer2.Player";
constexpr const char* mpris_interface_track_list =
	"org.mpris.MediaPlayer2.TrackList";
constexpr const char* mpris_method_raise = "Raise";
constexpr const char* mpris_method_quit = "Quit";
constexpr const char* mpris_player_method_next = "Next";
constexpr const char* mpris_player_method_previous = "Previous";
constexpr const char* mpris_player_method_pause = "Pause";
constexpr const char* mpris_player_method_play = "Play";
constexpr const char* mpris_player_method_play_pause = "PlayPause";
constexpr const char* mpris_player_method_stop = "Stop";
constexpr const char* mpris_player_method_seek = "Seek";
constexpr const char* mpris_player_method_set_position = "SetPosition";
constexpr const char* mpris_player_method_open_uri = "OpenUri";
constexpr const char* mpris_track_list_method_add_track = "AddTrack";
constexpr const char* mpris_track_list_method_remove_track = "RemoveTrack";
constexpr const char* mpris_track_list_method_go_to = "GoTo";
constexpr const char* mpris_track_list_method_get_tracks_metadata =
	"GetTracksMetadata";
constexpr const char* mpris_track_list_no_track =
	"/org/mpris/MediaPlayer2/TrackList/NoTrack";

struct Metadata {
	std::string mpris_track_id = "";
	int64_t mpris_length = 0;
	std::string mpris_art_url = "";

	std::string xesam_title = "";
	std::string xesam_album = "";
	std::vector<std::string> xesam_artist = {};
	std::vector<std::string> xesam_album_artist = {};
	int xesam_disc_number = 0;
	int xesam_track_number = 0;

	std::string xesam_url = "";
	std::vector<std::string> xesam_genre = {};
	std::vector<std::string> xesam_composer = {};
	std::vector<std::string> xesam_lyricist = {};
	std::vector<std::string> xesam_comment = {};
	std::string xesam_as_text = "";

	std::string xesam_content_created = "";
	std::string xesam_first_used = "";
	std::string xesam_last_used = "";
	int xesam_user_count = 0;

	float xesam_auto_rating = 0.0f;
	float xesam_user_rating = 0.0f;

	int xesam_audio_bpm = 0;
};

template <typename T>
void extract(DBusMessageIter& iter, T& value) {
	dbus_message_iter_get_basic(&iter, &value);
}

template <>
inline void extract(DBusMessageIter& iter, bool& value) {
	dbus_bool_t dbus_value;
	dbus_message_iter_get_basic(&iter, &dbus_value);
	value = dbus_value == TRUE;
}

template <>
inline void extract(DBusMessageIter& iter, std::string& value) {
	char* str;
	dbus_message_iter_get_basic(&iter, &str);
	value = str;
}

template<>
inline void extract(DBusMessageIter& iter, std::vector<std::string>& value) {
	int type = dbus_message_iter_get_arg_type(&iter);
	if (type == DBUS_TYPE_STRING) {
		const char* str = nullptr;
		dbus_message_iter_get_basic(&iter, &str);
		value.emplace_back(str ? str : "");
	} else if (type == DBUS_TYPE_ARRAY) {
		DBusMessageIter array_iter;
		dbus_message_iter_recurse(&iter, &array_iter);
		while (dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID) {
			const char* str = nullptr;
			dbus_message_iter_get_basic(&array_iter, &str);
			value.emplace_back(str ? str : "");
			dbus_message_iter_next(&array_iter);
		}
	}
}

inline void extract_metadata(DBusMessageIter& iter, Metadata& metadata) {
	DBusMessageIter array_iter;
	dbus_message_iter_recurse(&iter, &array_iter);

	while (dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID) {
		DBusMessageIter dict_iter;
		dbus_message_iter_recurse(&array_iter, &dict_iter);

		DBusBasicValue value;
		dbus_message_iter_get_basic(&dict_iter, &value);
		dbus_message_iter_next(&dict_iter);

		std::string_view key = value.str;

		DBusMessageIter variant_iter;
		dbus_message_iter_recurse(&dict_iter, &variant_iter);

		if (key == "mpris:trackid") {
			extract(variant_iter, metadata.mpris_track_id);
		} else if (key == "mpris:length") {
			extract(variant_iter, metadata.mpris_length);
		} else if (key == "mpris:artUrl") {
			extract(variant_iter, metadata.mpris_art_url);
		} else if (key == "xesam:title") {
			extract(variant_iter, metadata.xesam_title);
		} else if (key == "xesam:album") {
			extract(variant_iter, metadata.xesam_album);
		} else if (key == "xesam:artist") {
			extract(variant_iter, metadata.xesam_artist);
		} else if (key == "xesam:albumArtist") {
			extract(variant_iter, metadata.xesam_album_artist);
		} else if (key == "xesam:discNumber") {
			extract(variant_iter, metadata.xesam_disc_number);
		} else if (key == "xesam:trackNumber") {
			extract(variant_iter, metadata.xesam_track_number);
		} else if (key == "xesam:url") {
			extract(variant_iter, metadata.xesam_url);
		} else if (key == "xesam:genre") {
			extract(variant_iter, metadata.xesam_genre);
		} else if (key == "xesam:composer") {
			extract(variant_iter, metadata.xesam_composer);
		} else if (key == "xesam:lyricist") {
			extract(variant_iter, metadata.xesam_lyricist);
		} else if (key == "xesam:comment") {
			extract(variant_iter, metadata.xesam_comment);
		} else if (key == "xesam:asText") {
			extract(variant_iter, metadata.xesam_as_text);
		} else if (key == "xesam:contentCreated") {
			extract(variant_iter, metadata.xesam_content_created);
		} else if (key == "xesam:firstUsed") {
			extract(variant_iter, metadata.xesam_first_used);
		} else if (key == "xesam:lastUsed") {
			extract(variant_iter, metadata.xesam_last_used);
		} else if (key == "xesam:userCount") {
			extract(variant_iter, metadata.xesam_user_count);
		} else if (key == "xesam:autoRating") {
			extract(variant_iter, metadata.xesam_auto_rating);
		} else if (key == "xesam:userRating") {
			extract(variant_iter, metadata.xesam_user_rating);
		} else if (key == "xesam:audioBPM") {
			extract(variant_iter, metadata.xesam_audio_bpm);
		}

		dbus_message_iter_next(&array_iter);
	}
}

template <typename T>
inline void print_field(const T value) {
	std::cout << value << '\n';
}

template <typename T>
void print_field(const std::vector<T> value) {
	if (value.empty()) return;
	for (auto& item : value) {
		std::cout << item << '\n';
	}
}

template <typename T>
inline void print_field(const std::string_view field, const T value) {
	std::cout << field << ": " << value << '\n';
}

template <typename T>
void print_field(const std::string_view field, const std::vector<T> value) {
	if (value.empty()) return;
	std::cout << field << ":";
	for (size_t i = 0; i < value.size(); i++) {
		std::cout << " " << value[i];
		if (i + 1 < value.size()) std::cout << ',';
	}
	std::cout << '\n';
}

inline void print_metadata(
	const Metadata& metadata, const std::string_view field = ""
) {
	if (!field.empty()) {
		if (field == "mpris:trackid") print_field(metadata.mpris_track_id);
		else if (field == "mpris:length") print_field(metadata.mpris_length);
		else if (field == "mpris:artUrl") print_field(metadata.mpris_art_url);
		else if (field == "xesam:title") print_field(metadata.xesam_title);
		else if (field == "xesam:album") print_field(metadata.xesam_album);
		else if (field == "xesam:artist") print_field(metadata.xesam_artist);
		else if (field == "xesam:albumArtist")
			print_field(metadata.xesam_album_artist);
		else if (field == "xesam:discNumber")
			print_field(metadata.xesam_disc_number);
		else if (field == "xesam:trackNumber")
			print_field(metadata.xesam_track_number);
		else if (field == "xesam:url") print_field(metadata.xesam_url);
		else if (field == "xesam:genre") print_field(metadata.xesam_genre);
		else if (field == "xesam:composer")
			print_field(metadata.xesam_composer);
		else if (field == "xesam:lyricist")
			print_field(metadata.xesam_lyricist);
		else if (field == "xesam:comment") print_field(metadata.xesam_comment);
		else if (field == "xesam:asText") print_field(metadata.xesam_as_text);
		else if (field == "xesam:contentCreated")
			print_field(metadata.xesam_content_created);
		else if (field == "xesam:firstUsed")
			print_field(metadata.xesam_first_used);
		else if (field == "xesam:lastUsed")
			print_field(metadata.xesam_last_used);
		else if (field == "xesam:userCount")
			print_field(metadata.xesam_user_count);
		else if (field == "xesam:autoRating")
			print_field(metadata.xesam_auto_rating);
		else if (field == "xesam:userRating")
			print_field(metadata.xesam_user_rating);
		else if (field == "xesam:audioBPM")
			print_field(metadata.xesam_audio_bpm);
		return;
	}
	print_field("mpris:trackid", metadata.mpris_track_id);
	print_field("mpris:length", metadata.mpris_length);
	print_field("mpris:artUrl", metadata.mpris_art_url);
	print_field("xesam:title", metadata.xesam_title);
	print_field("xesam:album", metadata.xesam_album);
	print_field("xesam:artist", metadata.xesam_artist);
	print_field("xesam:albumArtist", metadata.xesam_album_artist);
	print_field("xesam:discNumber", metadata.xesam_disc_number);
	print_field("xesam:trackNumber", metadata.xesam_track_number);
	print_field("xesam:url", metadata.xesam_url);
	print_field("xesam:genre", metadata.xesam_genre);
	print_field("xesam:composer", metadata.xesam_composer);
	print_field("xesam:lyricist", metadata.xesam_lyricist);
	print_field("xesam:comment", metadata.xesam_comment);
	print_field("xesam:asText", metadata.xesam_as_text);
	print_field("xesam:contentCreated", metadata.xesam_content_created);
	print_field("xesam:firstUsed", metadata.xesam_first_used);
	print_field("xesam:lastUsed", metadata.xesam_last_used);
	print_field("xesam:userCount", metadata.xesam_user_count);
	print_field("xesam:autoRating", metadata.xesam_auto_rating);
	print_field("xesam:userRating", metadata.xesam_user_rating);
	print_field("xesam:audioBPM", metadata.xesam_audio_bpm);
}

#endif	// !MPRIS_COMMON_HPP

#include "mpris/track_list.hpp"

#include <algorithm>
#include <cstring>

#include "dbus/dbus-protocol.h"
#include "dbus/dbus.h"
#include "mpris/common.hpp"

TrackList::TrackList(DBusConnection* connection, const std::string_view name)
	: _connection(connection), _name(name) {
	init_properties();
}

void TrackList::add_track(
	const std::string_view uri, const std::string_view after_track,
	bool set_as_current
) {
	auto it = std::find(_tracks.begin(), _tracks.end(), after_track);
	if (!_can_edit_tracks ||
		(it == _tracks.end() && after_track != mpris_track_list_no_track))
		return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_track_list,
		mpris_track_list_method_add_track
	);

	const char* dbus_uri = uri.data();
	const char* dbus_after_track = after_track.data();
	dbus_bool_t dbus_set_as_current = set_as_current;

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dbus_uri);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_OBJECT_PATH, &dbus_after_track
	);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_BOOLEAN, &dbus_set_as_current
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);

	reset_properties();
	init_properties();
}

void TrackList::remove_track(const std::string_view track_id) {
	auto it = std::find(_tracks.begin(), _tracks.end(), track_id);
	if (!_can_edit_tracks || it == _tracks.end()) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_track_list,
		mpris_track_list_method_remove_track
	);

	const char* dbus_track_id = track_id.data();

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_OBJECT_PATH, &dbus_track_id
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);

	_tracks.erase(it);
}

void TrackList::go_to(const std::string_view track_id) const {
	auto it = std::find(_tracks.begin(), _tracks.end(), track_id);
	if (!_can_edit_tracks || it == _tracks.end()) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_track_list,
		mpris_track_list_method_go_to
	);

	const char* dbus_track_id = track_id.data();

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_OBJECT_PATH, &dbus_track_id
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

std::vector<Metadata> TrackList::get_metadata(
	const std::vector<std::string_view> track_ids
) const {
	std::vector<Metadata> tracks_metadata;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_track_list,
		mpris_track_list_method_get_tracks_metadata
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	DBusMessageIter array_iter;
	dbus_message_iter_open_container(
		&args, DBUS_TYPE_ARRAY, DBUS_TYPE_OBJECT_PATH_AS_STRING, &array_iter
	);
	for (const auto& track_id : track_ids) {
		auto it = std::find(_tracks.begin(), _tracks.end(), track_id);
		if (it == _tracks.end()) continue;
		const char* dbus_track_id = track_id.data();
		dbus_message_iter_append_basic(
			&array_iter, DBUS_TYPE_OBJECT_PATH, &dbus_track_id
		);
	}
	dbus_message_iter_close_container(&args, &array_iter);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_iter_init(reply, &args);
	dbus_message_iter_recurse(&args, &array_iter);

	while (dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID) {
		Metadata track_metadata;
		extract_metadata(array_iter, track_metadata);
		tracks_metadata.push_back(track_metadata);
		dbus_message_iter_next(&array_iter);
	}

	dbus_message_unref(msg);
	dbus_message_unref(reply);

	return tracks_metadata;
}

void TrackList::set_name(const std::string_view name) {
	_name = name;
	reset_properties();
	init_properties();
}

void TrackList::print_properties(const std::string_view field) const {
	if (!field.empty()) {
		if (field == "Tracks") print_field(_tracks);
		else if (field == "CanEditTracks") print_field(_can_edit_tracks);
		return;
	}
	print_field("Tracks", _tracks);
	print_field("CanEditTracks", _can_edit_tracks);
}

void TrackList::init_properties() {
	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, dbus_interface_properties,
		dbus_properties_method_get_all
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_STRING, &mpris_interface_track_list
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	DBusMessageIter array_iter;
	dbus_message_iter_init(reply, &array_iter);
	dbus_message_iter_recurse(&array_iter, &array_iter);

	while (dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID) {
		DBusMessageIter dict_iter;
		dbus_message_iter_recurse(&array_iter, &dict_iter);

		DBusBasicValue value;
		dbus_message_iter_get_basic(&dict_iter, &value);
		dbus_message_iter_next(&dict_iter);

		std::string_view key = value.str;

		DBusMessageIter variant_iter;
		dbus_message_iter_recurse(&dict_iter, &variant_iter);

		if (key == "Tracks") extract(variant_iter, _tracks);
		else if (key == "CanEditTracks")
			extract(variant_iter, _can_edit_tracks);

		dbus_message_iter_next(&array_iter);
	}

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void TrackList::reset_properties() {
	_tracks.clear();
	_can_edit_tracks = false;
}

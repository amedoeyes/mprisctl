#include "mpris/mpris.hpp"

#include <algorithm>

#include "mpris/common.hpp"

MPRIS::MPRIS() {
	DBusError err;
	dbus_error_init(&err);
	_connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);
	init_players();
	if (_players.empty()) throw std::runtime_error("No MPRIS players found");
	_player = new Player(_connection, _players[0]);
	init_properties();
	if (_has_track_list) _track_list = new TrackList(_connection, _players[0]);
}

MPRIS::MPRIS(const std::string_view player) {
	DBusError err;
	dbus_error_init(&err);
	_connection = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);
	init_players();
	if (_players.empty()) throw std::runtime_error("No MPRIS players found");
	auto it = std::find(_players.begin(), _players.end(), player);
	if (it == _players.end()) {
		_player = new Player(_connection, _players[0]);
	} else {
		_current_player_index = it - _players.begin();
		_player = new Player(_connection, player);
	}
	init_properties();
	if (_has_track_list) _track_list = new TrackList(_connection, player);
}

MPRIS::~MPRIS() {
	dbus_connection_unref(_connection);
	delete _player;
	if (_has_track_list) delete _track_list;
}

void MPRIS::raise() const {
	if (!_can_raise) return;

	DBusMessage *msg = dbus_message_new_method_call(
		_player->get_name().c_str(), mpris_path, mpris_interface,
		mpris_method_raise
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage *reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void MPRIS::quit() const {
	if (!_can_quit) return;

	DBusMessage *msg = dbus_message_new_method_call(
		_player->get_name().c_str(), mpris_path, mpris_interface,
		mpris_method_quit
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage *reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void MPRIS::next() {
	if (_current_player_index + 1 >= _players.size()) _current_player_index = 0;
	else _current_player_index++;
	reload();
}

void MPRIS::previous() {
	if (_current_player_index == 0) _current_player_index = _players.size() - 1;
	else _current_player_index--;
	reload();
}

void MPRIS::set_player(const std::string_view name) {
	auto it = std::find(_players.begin(), _players.end(), name);
	if (it == _players.end()) return;
	_current_player_index = it - _players.begin();
	reload();
}

Player *MPRIS::get_player() const {
	return _player;
}

TrackList *MPRIS::get_track_list() const {
	return _track_list;
}

void MPRIS::print_properties(const std::string_view field) const {
	if (!field.empty()) {
		if (field == "Identity") print_field(_identity);
		else if (field == "DesktopEntry") print_field(_desktop_entry);
		else if (field == "Fullscreen") print_field(_fullscreen);
		else if (field == "HasTrackList") print_field(_has_track_list);
		else if (field == "SupportedUriSchemes")
			print_field(_supported_uri_schemes);
		else if (field == "SupportedMimeTypes")
			print_field(_supported_mime_types);
		else if (field == "CanQuit") print_field(_can_quit);
		else if (field == "CanRaise") print_field(_can_raise);
		else if (field == "Player") print_field(_player->get_name());
		else if (field == "Players") print_field(_players);
		return;
	}

	print_field("Identity", _identity);
	print_field("DesktopEntry", _desktop_entry);
	print_field("Fullscreen", _fullscreen);
	print_field("HasTrackList", _has_track_list);
	print_field("SupportedUriSchemes", _supported_uri_schemes);
	print_field("SupportedMimeTypes", _supported_mime_types);
	print_field("CanQuit", _can_quit);
	print_field("CanRaise", _can_raise);
	print_field("Player", _player->get_name());
	print_field("Players", _players);
}

void MPRIS::init_players() {
	DBusMessage *msg = dbus_message_new_method_call(
		"org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus",
		"ListNames"
	);

	DBusError err;
	dbus_error_init(&err);

	DBusMessage *reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) {
		throw std::runtime_error(err.message);
		dbus_error_free(&err);
	}

	DBusMessageIter reply_iter;
	dbus_message_iter_init(reply, &reply_iter);

	DBusMessageIter array_iter;
	dbus_message_iter_recurse(&reply_iter, &array_iter);

	while (dbus_message_iter_get_arg_type(&array_iter) != DBUS_TYPE_INVALID) {
		char *name;
		dbus_message_iter_get_basic(&array_iter, &name);
		dbus_message_iter_next(&array_iter);
		if (strstr(name, "org.mpris.MediaPlayer2.") != NULL) {
			_players.push_back(name);
		}
	}

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void MPRIS::init_properties() {
	DBusMessage *msg = dbus_message_new_method_call(
		_player->get_name().c_str(), mpris_path, dbus_interface_properties,
		dbus_properties_method_get_all
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &mpris_interface);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage *reply =
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

		if (key == "Identity") extract(variant_iter, _identity);
		else if (key == "DesktopEntry") extract(variant_iter, _desktop_entry);
		else if (key == "Fullscreen") extract(variant_iter, _fullscreen);
		else if (key == "HasTrackList") extract(variant_iter, _has_track_list);
		else if (key == "SupportedUriSchemes")
			extract(variant_iter, _supported_uri_schemes);
		else if (key == "SupportedMimeTypes")
			extract(variant_iter, _supported_mime_types);
		else if (key == "CanQuit") extract(variant_iter, _can_quit);
		else if (key == "CanRaise") extract(variant_iter, _can_raise);

		dbus_message_iter_next(&array_iter);
	}

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void MPRIS::reset_properties() {
	_identity.clear();
	_desktop_entry.clear();
	_fullscreen = false;
	_has_track_list = false;
	_supported_uri_schemes.clear();
	_supported_mime_types.clear();
	_can_quit = false;
	_can_raise = false;
}

void MPRIS::reload() {
	reset_properties();
	_player->set_name(_players[_current_player_index]);
	init_properties();
	if (_has_track_list && _track_list != nullptr)
		_track_list->set_name(_players[_current_player_index]);
	else if (_has_track_list)
		_track_list =
			new TrackList(_connection, _players[_current_player_index]);
}

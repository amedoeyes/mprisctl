#include "mpris/player.hpp"

#include <dbus/dbus.h>

#include "mpris/common.hpp"

Player::Player(DBusConnection* connection, const std::string_view name)
	: _connection(connection), _name(name) {
	init_properties();
};

void Player::set_name(const std::string_view name) {
	_name = name;
	reset_properties();
	init_properties();
}

std::string Player::get_name() const {
	return _name;
}

void Player::next() const {
	if (!_can_go_next) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_next
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::previous() const {
	if (!_can_go_previous) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_previous
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::pause() const {
	if (!_can_pause) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_pause
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::play() const {
	if (!_can_play) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_play
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::play_pause() const {
	if (!_can_pause) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_play_pause
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::stop() const {
	if (!_can_control) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_stop
	);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::seek(int64_t offset) const {
	if (!_can_seek) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_seek
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &offset);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::set_position(int64_t position) const {
	if (!_can_seek) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_set_position
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_OBJECT_PATH, &_metadata.mpris_track_id
	);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_INT64, &position);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::open_uri(const std::string_view uri) const {
	if (!_can_control) return;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, mpris_interface_player,
		mpris_player_method_open_uri
	);

	const char* dbus_uri = uri.data();

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dbus_uri);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::set_volume(double volume) {
	if (!_can_control) return;

	if (volume < 0) volume = 0;
	else if (volume > 1) volume = 1;

	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, dbus_interface_properties,
		dbus_properties_method_set
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_STRING, &mpris_interface_player
	);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &dbus_arg_volume);
	DBusMessageIter variant;
	dbus_message_iter_open_container(
		&args, DBUS_TYPE_VARIANT, DBUS_TYPE_DOUBLE_AS_STRING, &variant
	);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_DOUBLE, &volume);
	dbus_message_iter_close_container(&args, &variant);

	DBusError err;
	dbus_error_init(&err);
	DBusMessage* reply =
		dbus_connection_send_with_reply_and_block(_connection, msg, -1, &err);
	if (dbus_error_is_set(&err)) throw std::runtime_error(err.message);

	dbus_message_unref(msg);
	dbus_message_unref(reply);

	_volume = volume;
}

double Player::get_volume() const {
	return _volume;
}

Metadata Player::get_metadata() const {
	return _metadata;
}

void Player::print_properties(const std::string_view field) const {
	if (!field.empty()) {
		if (field == "PlaybackStatus") print_field(_playback_status);
		else if (field == "LoopStatus") print_field(_loop_status);
		else if (field == "Volume") print_field(_volume);
		else if (field == "Position") print_field(_position);
		else if (field == "Shuffle") print_field(_shuffle);
		else if (field == "Rate") print_field(_rate);
		else if (field == "MinimumRate") print_field(_minimum_rate);
		else if (field == "MaximumRate") print_field(_maximum_rate);
		else if (field == "CanGoNext") print_field(_can_go_next);
		else if (field == "CanGoPrevious") print_field(_can_go_previous);
		else if (field == "CanPlay") print_field(_can_play);
		else if (field == "CanPause") print_field(_can_pause);
		else if (field == "CanSeek") print_field(_can_seek);
		else if (field == "CanControl") print_field(_can_control);
		return;
	}
	print_field("PlaybackStatus", _playback_status);
	print_field("LoopStatus", _loop_status);
	print_field("Volume", _volume);
	print_field("Position", _position);
	print_field("Shuffle", _shuffle);
	print_field("Rate", _rate);
	print_field("MinimumRate", _minimum_rate);
	print_field("MaximumRate", _maximum_rate);
	print_field("CanGoNext", _can_go_next);
	print_field("CanGoPrevious", _can_go_previous);
	print_field("CanPlay", _can_play);
	print_field("CanPause", _can_pause);
	print_field("CanSeek", _can_seek);
	print_field("CanControl", _can_control);
}

void Player::init_properties() {
	DBusMessage* msg = dbus_message_new_method_call(
		_name.c_str(), mpris_path, dbus_interface_properties,
		dbus_properties_method_get_all
	);

	DBusMessageIter args;
	dbus_message_iter_init_append(msg, &args);
	dbus_message_iter_append_basic(
		&args, DBUS_TYPE_STRING, &mpris_interface_player
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

		if (key == "PlaybackStatus") extract(variant_iter, _playback_status);
		else if (key == "LoopStatus") extract(variant_iter, _loop_status);
		else if (key == "Volume") extract(variant_iter, _volume);
		else if (key == "Position") extract(variant_iter, _position);
		else if (key == "Shuffle") extract(variant_iter, _shuffle);
		else if (key == "Rate") extract(variant_iter, _rate);
		else if (key == "MinimumRate") extract(variant_iter, _minimum_rate);
		else if (key == "MaximumRate") extract(variant_iter, _maximum_rate);
		else if (key == "CanGoNext") extract(variant_iter, _can_go_next);
		else if (key == "CanGoPrevious")
			extract(variant_iter, _can_go_previous);
		else if (key == "CanPlay") extract(variant_iter, _can_play);
		else if (key == "CanPause") extract(variant_iter, _can_pause);
		else if (key == "CanSeek") extract(variant_iter, _can_seek);
		else if (key == "CanControl") extract(variant_iter, _can_control);
		else if (key == "Metadata") extract_metadata(variant_iter, _metadata);

		dbus_message_iter_next(&array_iter);
	}

	dbus_message_unref(msg);
	dbus_message_unref(reply);
}

void Player::reset_properties() {
	_playback_status = "";
	_loop_status = "";
	_volume = 0;
	_position = 0;
	_shuffle = false;
	_rate = 0;
	_minimum_rate = 0;
	_maximum_rate = 0;

	_can_go_next = false;
	_can_go_previous = false;
	_can_play = false;
	_can_pause = false;
	_can_seek = false;
	_can_control = false;

	_metadata.mpris_track_id = "";
	_metadata.mpris_length = 0;
	_metadata.mpris_art_url = "";
	_metadata.xesam_title = "";
	_metadata.xesam_album = "";
	_metadata.xesam_artist.clear();
	_metadata.xesam_album_artist.clear();
	_metadata.xesam_disc_number = 0;
	_metadata.xesam_track_number = 0;
	_metadata.xesam_url = "";
	_metadata.xesam_genre.clear();
	_metadata.xesam_composer.clear();
	_metadata.xesam_lyricist.clear();
	_metadata.xesam_comment.clear();
	_metadata.xesam_as_text = "";
	_metadata.xesam_content_created = "";
	_metadata.xesam_first_used = "";
	_metadata.xesam_last_used = "";
	_metadata.xesam_user_count = 0;
	_metadata.xesam_auto_rating = 0;
	_metadata.xesam_user_rating = 0;
	_metadata.xesam_audio_bpm = 0;
}

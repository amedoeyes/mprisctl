#ifndef MPRIS_MPRIS_HPP
#define MPRIS_MPRIS_HPP

#include <dbus/dbus.h>

#include <string>
#include <vector>

#include "mpris/player.hpp"
#include "mpris/track_list.hpp"

class MPRIS {
	public:
	MPRIS();
	MPRIS(const std::string_view player);
	~MPRIS();

	public:
	void raise() const;
	void quit() const;
	void next();
	void previous();

	void set_player(const std::string_view name);
	Player* get_player() const;
	TrackList* get_track_list() const;

	void print_properties(const std::string_view field = "") const;

	public:
	DBusConnection* _connection = nullptr;

	std::vector<std::string> _players = {};
	size_t _current_player_index = 0;
	Player* _player = nullptr;
	TrackList* _track_list = nullptr;

	std::string _identity = "";
	std::string _desktop_entry = "";
	bool _fullscreen = false;
	bool _has_track_list = false;
	std::vector<std::string> _supported_uri_schemes = {};
	std::vector<std::string> _supported_mime_types = {};

	bool _can_quit = false;
	bool _can_raise = false;

	private:
	void init_players();
	void init_properties();
	void reset_properties();
	void reload();
};

#endif	// MPRIS_MPRI_HPP

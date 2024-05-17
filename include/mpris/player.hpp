#ifndef MPRIS_PLAYER_HPP
#define MPRIS_PLAYER_HPP

#include "common.hpp"

class Player {
	public:
	Player(DBusConnection* connection, const std::string_view name);

	public:
	void next() const;
	void previous() const;
	void pause() const;
	void play() const;
	void play_pause() const;
	void stop() const;
	void seek(int64_t offset) const;
	void set_position(int64_t position) const;
	void open_uri(const std::string_view uri) const;

	void set_volume(double volume);
	double get_volume() const;

	Metadata get_metadata() const;

	void set_name(const std::string_view name);
	std::string get_name() const;

	void print_properties(const std::string_view field) const;

	private:
	DBusConnection* _connection;
	std::string _name;

	std::string _playback_status;
	std::string _loop_status;
	double _volume;
	int64_t _position;
	bool _shuffle;
	double _rate;
	double _minimum_rate;
	double _maximum_rate;

	Metadata _metadata;

	bool _can_go_next;
	bool _can_go_previous;
	bool _can_play;
	bool _can_pause;
	bool _can_seek;
	bool _can_control;

	private:
	void init_properties();
	void reset_properties();
};

#endif	// !MPRIS_PLAYER_HPP

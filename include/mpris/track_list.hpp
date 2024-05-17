#ifndef MPRIS_TRACK_LIST_HPP
#define MPRIS_TRACK_LIST_HPP

#include <dbus/dbus.h>

#include "mpris/common.hpp"

class TrackList {
	public:
	TrackList(DBusConnection *connection, const std::string_view name);

	public:
	void add_track(
		const std::string_view uri, const std::string_view after_track,
		bool set_as_current
	);
	void remove_track(const std::string_view track_id);
	void go_to(const std::string_view track_id) const;
	std::vector<Metadata> get_metadata(
		const std::vector<std::string_view> track_ids
	) const;

	void set_name(const std::string_view name);

	void print_properties(std::string_view field) const;

	private:
	DBusConnection *_connection;
	std::string _name;

	std::vector<std::string> _tracks;
	bool _can_edit_tracks;

	private:
	void init_properties();
	void reset_properties();
};

#endif	// !MPRIS_TRACK_LIST_HPP

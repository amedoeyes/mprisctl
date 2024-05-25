#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>
#include <fstream>
#include <iostream>

#include "mpris/common.hpp"
#include "mpris/mpris.hpp"

namespace po = boost::program_options;

constexpr const char *tmp_file = "/tmp/mprisctl_current_player";

static void save_current_player(std::string player) {
	std::ofstream file(tmp_file);
	file << player;
	file.close();
}

static std::string load_current_player() {
	std::string player;
	std::ifstream file(tmp_file);
	file >> player;
	file.close();
	return player;
}

int main(int argc, char *argv[]) {
	po::options_description desc("mprisctl options");
	desc.add_options()
		("next-player,N", "Switch to the next available MPRIS player.")
		("previous-player,P", "Switch to the previous available MPRIS player.")
		("set-player", po::value<std::string>()->value_name("str"), "Specify the player to control by name.")
		("raise,r", "Bring the specified player to the foreground.")
		("quit,q", "Quit the specified player.")
		("display-mpris-properties", po::value<std::string>()->implicit_value("")->value_name("str"), "Display MPRIS properties of the current player. Optionally, specify a field to display only that particular property's value.")

		("next,n", "Skip to the next track.")
		("previous,p", "Skip to the previous track.")
		("pause", "Pause playback.")
		("play", "Start playback.")
		("play-pause,t", "Toggle between play and pause.")
		("stop", "Stop playback.")
		("seek,s", po::value<int64_t>()->value_name("int"), "Seek forward or backward by the specified number of microseconds.")
		("set-position,S", po::value<int64_t>()->value_name("int"), "Set playback position to the specified microsecond.")
		("open-uri,u", po::value<std::string>()->value_name("str"), "Open and play the media from the specified URI.")
		("set-volume,v", po::value<double>()->value_name("double"), "Set the playback volume (range: 0.0 to 1.0).")
		("increment-volume,i", po::value<double>()->value_name("double"), "Increment the playback volume by the specified amount.")
		("decrement-volume,d", po::value<double>()->value_name("double"), "Decrement the playback volume by the specified amount.")
		("display-metadata", po::value<std::string>()->implicit_value("")->value_name("str"),"Display metadata for the current track. Optionally, specify a field to display only that particular property's value.")
		("display-player-properties", po::value<std::string>()->implicit_value("")->value_name("str"), "Display properties of the current player. Optionally, specify a field to display only that particular property's value.")

		("add-track", po::value<std::string>()->value_name("str"), "Add a track to the track list by URI.")
		("add-track-current" ,"Mark the added track as the current track")
		("add-track-after", po::value<std::string>()->value_name("str"), "Add the track after the specified track id.")
		("remove-track", po::value<std::string>()->value_name("str"), "Remove a track from the track list by track id.")
		("go-to-track", po::value<std::string>()->value_name("str"), "Jump to the specified track in the track list by track id.")
		("display-track-list-properties", po::value<std::string>()->implicit_value(""), "Display track list properties of the current player. Optionally, specify a field to display only that particular property's value.")

		("version", "Display version information.")
		("help,h", "Display this help message.");

	if (argc == 1) {
		std::cout << desc << '\n';
		return 0;
	}

	try {
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << '\n';
			return 0;
		}
		if (vm.count("version")) {
			std::cout << "mprisctl version " << VERSION << '\n';
			return 0;
		}

		std::string current_player = load_current_player();
		MPRIS mpris(current_player);

		if (vm.count("next-player")) mpris.next();
		if (vm.count("previous-player")) mpris.previous();
		if (vm.count("set-player"))
			mpris.set_player(vm["set-player"].as<std::string>());
		if (vm.count("raise")) mpris.raise();
		if (vm.count("quit")) mpris.quit();
		if (vm.count("display-mpris-properties"))
			mpris.print_properties(
				vm["display-mpris-properties"].as<std::string>()
			);

		Player *player = mpris.get_player();

		if (vm.count("next")) player->next();
		if (vm.count("previous")) player->previous();
		if (vm.count("pause")) player->pause();
		if (vm.count("play")) player->play();
		if (vm.count("play-pause")) player->play_pause();
		if (vm.count("stop")) player->stop();
		if (vm.count("seek")) player->seek(vm["seek"].as<int64_t>());
		if (vm.count("set-position"))
			player->set_position(vm["set-position"].as<int64_t>());
		if (vm.count("open-uri"))
			player->open_uri(vm["open-uri"].as<std::string>());
		if (vm.count("set-volume"))
			player->set_volume(vm["set-volume"].as<double>());
		if (vm.count("increment-volume")) {
			double current_volume = player->get_volume() * 100;
			double new_volume = vm["increment-volume"].as<double>() * 100;
			player->set_volume((current_volume + new_volume) / 100);
		}
		if (vm.count("decrement-volume")) {
			double current_volume = player->get_volume() * 100;
			double new_volume = vm["decrement-volume"].as<double>() * 100;
			player->set_volume((current_volume - new_volume) / 100);
		}
		if (vm.count("display-metadata"))
			print_metadata(
				player->get_metadata(), vm["display-metadata"].as<std::string>()
			);
		if (vm.count("display-player-properties"))
			player->print_properties(
				vm["display-player-properties"].as<std::string>()
			);

		TrackList *track_list = mpris.get_track_list();

		if (track_list != nullptr) {
			if (vm.count("add-track")) {
				bool add_track_current = false;
				std::string after_track = mpris_track_list_no_track;
				if (vm.count("add-track-current")) add_track_current = true;
				std::cout << add_track_current << '\n';
				if (vm.count("add-track-after"))
					after_track = vm["add-track-after"].as<std::string>();
				track_list->add_track(
					vm["add-track"].as<std::string>().c_str(),
					after_track.c_str(), add_track_current
				);
			}
			if (vm.count("remove-track"))
				track_list->remove_track(vm["remove-track"].as<std::string>());
			if (vm.count("go-to-track"))
				track_list->go_to(vm["go-to-track"].as<std::string>());
			if (vm.count("display-track-list-properties"))
				track_list->print_properties(
					vm["display-track-list-properties"].as<std::string>()
				);
		}

		if (current_player != player->get_name())
			save_current_player(player->get_name());

	} catch (const std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}

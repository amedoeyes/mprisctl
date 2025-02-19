#include "mpris/common.hpp"
#include "mpris/mpris.hpp"

#include <cassert>

import std;

static constexpr auto tmp_file = "/tmp/mprisctl_current_player";

using value_type = std::variant<std::monostate, std::int64_t, double, std::string>;

enum class option_category : std::uint64_t { general, player, tracklist, misc, size };

template<typename T>
struct option_value {
	std::string name;
	bool optional = false;
};

struct option {
	std::pair<std::string, std::string> flags;
	std::string description;
	option_category category;
	std::optional<std::variant<option_value<std::int64_t>, option_value<double>, option_value<std::string>>>
		value = std::nullopt;
};

static const auto options = std::array<std::pair<std::string, option>, 27>{{
	{
		"next_player",
		{
			{"-N", "--next-player"},
			"Switch to next player.",
			option_category::general,
		},
	},
	{
		"previous_player",
		{
			{"-P", "--previous-player"},
			"Switch to previous player.",
			option_category::general,
		},
	},
	{
		"set_player",
		{
			{"", "--set-player"},
			"Set player to control.",
			option_category::general,
			option_value<std::string>{.name = "Name"},
		},
	},
	{
		"raise",
		{
			{"-r", "--raise"},
			"Raise current player.",
			option_category::general,
		},
	},
	{
		"quit",
		{
			{"-q", "--quit"},
			"Quit current player.",
			option_category::general,
		},
	},
	{
		"properties",
		{
			{"", "--properties"},
			"Display MPRIS properties.",
			option_category::general,
			option_value<std::string>{.name = "Field", .optional = true},
		},
	},

	{
		"next",
		{
			{"-n", "--next"},
			"Skip to next track.",
			option_category::player,
		},
	},
	{
		"previous",
		{
			{"-p", "--previous"},
			"Skip to previous track.",
			option_category::player,
		},
	},
	{
		"pause",
		{
			{"", "--pause"},
			"Pause playback.",
			option_category::player,
		},
	},
	{
		"play",
		{
			{"", "--play"},
			"Start playback.",
			option_category::player,
		},
	},
	{
		"play_pause",
		{
			{"-t", "--play-pause"},
			"Toggle between play and pause.",
			option_category::player,
		},
	},
	{
		"stop",
		{
			{"", "--stop"},
			"Stop playback.",
			option_category::player,
		},
	},
	{
		"seek",
		{
			{"-s", "--seek"},
			"Seek forward or backward by microseconds.",
			option_category::player,
			option_value<std::int64_t>{.name = "Microseconds"},
		},
	},
	{
		"set_position",
		{
			{"-S", "--set-position"},
			"Set playback position.",
			option_category::player,
			option_value<std::int64_t>{.name = "Microseconds"},
		},
	},
	{
		"open",
		{
			{"-o", "--open"},
			"Open and play media.",
			option_category::player,
			option_value<std::string>{.name = "URI"},
		},
	},
	{
		"set_volume",
		{
			{"-v", "--set-volume"},
			"Set playback volume.",
			option_category::player,
			option_value<double>{.name = "Volume"},
		},
	},
	{
		"increment_volume",
		{
			{"-i", "--increment-volume"},
			"Increment playback volume.",
			option_category::player,
			option_value<double>{.name = "Volume"},
		},
	},
	{
		"decrement_volume",
		{
			{"-d", "--decrement-volume"},
			"Decrement playback volume.",
			option_category::player,
			option_value<double>{.name = "volume"},
		},
	},
	{
		"metadata",
		{
			{"", "--metadata"},
			"Display metadata of current track.",
			option_category::player,
			option_value<std::string>{.name = "Field", .optional = true},
		},
	},
	{
		"player_properties",
		{
			{"", "--player-properties"},
			"Display properties of current player.",
			option_category::player,
			option_value<std::string>{.name = "Field", .optional = true},
		},
	},

	{
		"add_track",
		{
			{"", "--add-track"},
			"Add track to tracklist.",
			option_category::tracklist,
			option_value<std::string>{.name = "URI"},
		},
	},
	{
		"add_track_after",
		{
			{"", "--add-track-after"},
			"Add track after the specified track.",
			option_category::tracklist,
			option_value<std::string>{.name = "ID"},
		},
	},
	{
		"remove_track",
		{
			{"", "--remove-track"},
			"Remove track from tracklist.",
			option_category::tracklist,
			option_value<std::string>{.name = "ID"},
		},
	},
	{
		"go_to_track",
		{
			{"", "--go-to-track"},
			"Go to track in tracklist.",
			option_category::tracklist,
			option_value<std::string>{.name = "ID"},
		},
	},
	{
		"tracklist_properties",
		{
			{"", "--tracklist-properties"},
			"Display tracklist properties of current player.",
			option_category::tracklist,
			option_value<std::string>{.name = "Field", .optional = true},
		},
	},

	{
		"version",
		{
			{"", "--version"},
			"Display version information.",
			option_category::misc,
		},
	},
	{
		"help",
		{
			{"-h", "--help"},
			"Display this help message.",
			option_category::misc,
		},
	},
}};

template<class... Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

auto parse_args(std::span<const std::string_view> args) {
	auto result = std::unordered_map<std::string, value_type>{};

	for (auto i = 0ul; i < args.size();) {
		auto arg = args[i++];
		auto assigned_value = std::string_view{};
		if (std::ranges::contains(arg, '=')) {
			auto parts = arg | std::views::split('=') | std::views::transform([](auto&& p) { return std::string_view(p); });
			auto it = parts.begin();
			arg = *it;
			assigned_value = *(++it);
		}

		const auto it = std::ranges::find_if(options, [&](auto&& o) {
			return o.second.flags.first == arg || o.second.flags.second == arg;
		});
		if (it == options.end()) throw std::runtime_error(std::format("'{}' is not a valid option", arg));
		const auto& [option_name, option] = *it;

		if (!option.value && !assigned_value.empty()) {
			throw std::runtime_error(std::format("option '{}' does not expect a value", arg));
		}

		auto value = value_type{std::monostate{}};
		if (option.value) {
			auto value_str = std::string_view{};
			if (!assigned_value.empty()) {
				value_str = assigned_value;
			} else if (i < args.size() && !args[i].starts_with("-")) {
				value_str = args[i++];
			} else if (!std::visit([](auto&& v) { return v.optional; }, *option.value)) {
				throw std::runtime_error(std::format("option '{}' expects value '{}'",
				                                     arg,
				                                     std::visit([](auto&& v) { return v.name; }, *option.value)));
			}
			if (!value_str.empty()) {
				value = std::visit(overloaded{
														 [&](const option_value<std::int64_t>&) -> value_type {
															 auto value = 0l;
															 auto [_, ec] = std::from_chars(value_str.data(),
					                                                    value_str.data() + value_str.size(),
					                                                    value);
															 if (ec != std::errc{}) {
																 throw std::runtime_error(std::format("'{}' is not a valid value for {}",
						                                                          value_str,
						                                                          arg));
															 }
															 return value;
														 },
														 [&](const option_value<double>&) -> value_type {
															 auto value = 0.0;
															 try {
																 value = std::stod(value_str.data());
															 } catch (...) {
																 throw std::runtime_error(std::format("'{}' is not a valid value for {}",
						                                                          value_str,
						                                                          arg));
															 }
															 // not supported by libc++ yet :/
					                     // auto [_, ec] = std::from_chars(arg_val.data(), arg_val.data() + arg_val.size(),
					                     // value); if (ec != std::errc{}) { 	throw std::runtime_error(std::format("'{}' is
					                     // not a valid value for {}", arg_val, arg));
					                     // }
															 return value;
														 },
														 [&](const option_value<std::string>&) -> value_type { return std::string(value_str); },
													 },
				                   *option.value);
			}
		}

		result.emplace(option_name, value);
	}

	return result;
}

auto display_help() -> void {
	auto get_option_width = [&](const auto& o) {
		auto width = 0;
		const auto& flags = o.flags;
		width += 2 + flags.second.size();
		const auto& value = o.value;
		if (value) {
			width += std::visit([](auto&& v) { return v.name.size(); }, *value);
			width += std::visit([](auto&& v) { return v.optional; }, *value) ? 2 : 0;
		}
		return width;
	};

	const auto max_len = std::ranges::max(options | std::views::values | std::views::transform(get_option_width));

	auto categorized_options = std::array<std::vector<option>, std::to_underlying(option_category::size)>{};
	for (const auto& o : options | std::views::values) {
		categorized_options[std::to_underlying(o.category)].emplace_back(o);
	}

	static constexpr auto get_category_str = [](const auto& category) {
		switch (category) {
			using enum option_category;
		case general: return "General";
		case player: return "Player";
		case tracklist: return "Tracklist";
		case misc: return "Miscellaneous";
		default: std::unreachable();
		}
	};

	std::println("Usage:");
	std::println("  mprisctl options");
	std::println("");
	for (const auto i : std::views::iota(0ul, categorized_options.size())) {
		std::println("{} Options:", get_category_str(static_cast<option_category>(i)));
		for (const auto& o : categorized_options[i]) {
			const auto padding = max_len - get_option_width(o);
			std::print("  ");
			std::print("{:2}", o.flags.first);
			if (o.flags.first.size() > 0) std::print(", ");
			else std::print("  ");
			std::print("{}", o.flags.second);
			std::print(" ");
			if (o.value) {
				const auto name = std::visit([](auto&& v) { return v.name; }, *o.value);
				const auto optional = std::visit([](auto&& v) { return v.optional; }, *o.value);
				if (optional) std::print("[");
				std::print("{}", name);
				if (optional) std::print("]");
			}
			for (const auto _ : std::views::iota(0, padding)) std::print(" ");
			std::print("  ");
			std::print("{}", o.description);
			std::println("");
		}
		std::println("");
	}
}

auto main(int argc, char** argv) -> int {
	try {
		const auto args = std::span(std::next(argv), argc - 1)  //
		                | std::views::transform([](const auto& a) { return std::string_view(a); })  //
		                | std::ranges::to<std::vector>();

		if (args.empty()) {
			display_help();
			std::quick_exit(0);
		}

		const auto parsed_args = parse_args(args);

		static const auto handle_arg = [&](auto name, auto&&... funcs) {
			if (parsed_args.contains(name)) {
				std::visit(overloaded{funcs..., [](auto&&) { assert(false); }}, parsed_args.at(name));
			}
		};

		handle_arg("help", [&](const std::monostate&) {
			display_help();
			std::quick_exit(0);
		});

		handle_arg("version", [&](const std::monostate&) {
			std::println("mprisctl " VERSION);
			std::quick_exit(0);
		});

		auto current_player = std::string{};
		std::ifstream{tmp_file} >> current_player;
		auto mpris = MPRIS{current_player};

		handle_arg("next_player", [&](const std::monostate&) { mpris.next(); });
		handle_arg("previous_player", [&](const std::monostate&) { mpris.previous(); });
		handle_arg("set_player", [&](const std::string& s) { mpris.set_player(s); });
		handle_arg("raise", [&](const std::monostate&) { mpris.raise(); });
		handle_arg("quit", [&](const std::monostate&) { mpris.quit(); });
		handle_arg(
			"properties",
			[&](const std::monostate&) { mpris.print_properties(); },
			[&](const std::string& s) { mpris.print_properties(s); });

		Player* player = mpris.get_player();

		handle_arg("next", [&](const std::monostate&) { player->next(); });
		handle_arg("previous", [&](const std::monostate&) { player->previous(); });
		handle_arg("pause", [&](const std::monostate&) { player->pause(); });
		handle_arg("play", [&](const std::monostate&) { player->play(); });
		handle_arg("play_pause", [&](const std::monostate&) { player->play_pause(); });
		handle_arg("stop", [&](const std::monostate&) { player->stop(); });
		handle_arg("seek", [&](const std::int64_t& micro) { player->seek(micro); });
		handle_arg("set_position", [&](const std::int64_t& micro) { player->set_position(micro); });
		handle_arg("open", [&](const std::string& uri) { player->open_uri(uri); });
		handle_arg("set_volume", [&](const double& volume) { player->set_volume(volume); });
		handle_arg("increment_volume", [&](const double& volume) {
			const auto current_volume = player->get_volume() * 100;
			const auto new_volume = volume * 100;
			player->set_volume((current_volume + new_volume) / 100);
		});
		handle_arg("decrement_volume", [&](const double& volume) {
			const auto current_volume = player->get_volume() * 100;
			const auto new_volume = volume * 100;
			player->set_volume((current_volume - new_volume) / 100);
		});
		handle_arg(
			"metadata",
			[&](const std::monostate&) { print_metadata(player->get_metadata()); },
			[&](const std::string& field) { print_metadata(player->get_metadata(), field); });
		handle_arg(
			"player_properties",
			[&](const std::monostate&) { player->print_properties(""); },
			[&](const std::string& field) { player->print_properties(field); });

		TrackList* track_list = mpris.get_track_list();

		if (track_list != nullptr) {
			handle_arg("add_track", [&](const std::string& uri) {
				auto add_track_current = false;
				handle_arg("add_track_current", [&](const std::monostate&) { add_track_current = true; });
				auto after_track = std::string{mpris_track_list_no_track};
				handle_arg("add_track_current", [&](const std::string& uri) { after_track = uri; });
				track_list->add_track(uri, after_track, add_track_current);
			});
			handle_arg("remove_track", [&](const std::string& uri) { track_list->remove_track(uri); });
			handle_arg("go_to_track", [&](const std::string& uri) { track_list->go_to(uri); });
			handle_arg(
				"tracklist_properties",
				[&](const std::monostate&) { track_list->print_properties(""); },
				[&](const std::string& field) { track_list->print_properties(field); });
		}

		if (current_player != player->get_name()) std::ofstream{tmp_file} << player->get_name();
	} catch (const std::exception& e) {
		std::println(std::cerr, "mprisctl: {}", e.what());
		std::quick_exit(1);
	}
}

use std::{
    fs::{File, create_dir_all, write},
    io,
    path::PathBuf,
    str::FromStr,
};

use clap::{Arg, Command, ValueEnum, crate_name, crate_version};
use directories::ProjectDirs;
use mpris::{
    player::{LoopStatus, PlayerMetadata, PlayerProperties},
    root::{Root, RootProperties},
};
use tokio::fs::read_to_string;

#[derive(Debug, thiserror::Error)]
enum Error {
    #[error(transparent)]
    Mpris(#[from] mpris::Error),

    #[error(transparent)]
    Clap(#[from] clap::Error),

    #[error(transparent)]
    Io(#[from] io::Error),
}

type Result<T> = std::result::Result<T, Error>;

#[derive(Debug, Clone, ValueEnum)]
enum LoopStatusArg {
    None,
    Track,
    Playlist,
}

#[derive(Debug, Clone)]
enum VolumeCommand {
    Set(f64),
    Increment(f64),
    Decrement(f64),
}

impl FromStr for VolumeCommand {
    type Err = String;

    fn from_str(s: &str) -> std::result::Result<Self, Self::Err> {
        match s.chars().last() {
            Some('+') => Ok(Self::Increment(
                s[..s.len() - 1].parse().map_err(|_| "Invalid number")?,
            )),
            Some('-') => Ok(Self::Decrement(
                s[..s.len() - 1].parse().map_err(|_| "Invalid number")?,
            )),
            _ => {
                let val = s.parse().map_err(|_| "Invalid number")?;
                if !(0.0..=1.0).contains(&val) {
                    return Err("Value must be 0-1".into());
                }
                Ok(Self::Set(val))
            }
        }
    }
}

fn get_metadata_field(metadata: &PlayerMetadata, name: &str) -> Option<String> {
    match name {
        "mpris:artUrl" => metadata.mpris_art_url.as_ref().map(|s| s.to_string()),
        "mpris:length" => metadata.mpris_length.as_ref().map(|n| n.to_string()),
        "mpris:trackid" => metadata.mpris_trackid.as_ref().map(|s| s.to_string()),
        "xesam:album" => metadata.xesam_album.as_ref().map(|s| s.to_string()),
        "xesam:albumArtist" => metadata.xesam_album_artist.as_ref().map(|v| v.join(", ")),
        "xesam:artist" => metadata.xesam_artist.as_ref().map(|v| v.join(", ")),
        "xesam:asText" => metadata.xesam_as_text.as_ref().map(|s| s.to_string()),
        "xesam:audioBPM" => metadata.xesam_audio_bpm.as_ref().map(|n| n.to_string()),
        "xesam:autoRating" => metadata.xesam_auto_rating.as_ref().map(|n| n.to_string()),
        "xesam:comment" => metadata.xesam_comment.as_ref().map(|v| v.join(", ")),
        "xesam:composer" => metadata.xesam_composer.as_ref().map(|v| v.join(", ")),
        "xesam:contentCreated" => metadata
            .xesam_content_created
            .as_ref()
            .map(|s| s.to_string()),
        "xesam:discNumber" => metadata.xesam_disc_number.as_ref().map(|n| n.to_string()),
        "xesam:firstUsed" => metadata.xesam_first_used.as_ref().map(|s| s.to_string()),
        "xesam:genre" => metadata.xesam_genre.as_ref().map(|v| v.join(", ")),
        "xesam:lastUsed" => metadata.xesam_last_used.as_ref().map(|s| s.to_string()),
        "xesam:lyricist" => metadata.xesam_lyricist.as_ref().map(|v| v.join(", ")),
        "xesam:title" => metadata.xesam_title.as_ref().map(|s| s.to_string()),
        "xesam:trackNumber" => metadata.xesam_track_number.as_ref().map(|n| n.to_string()),
        "xesam:url" => metadata.xesam_url.as_ref().map(|s| s.to_string()),
        "xesam:userCount" => metadata.xesam_user_count.as_ref().map(|n| n.to_string()),
        "xesam:userRating" => metadata.xesam_user_rating.as_ref().map(|n| n.to_string()),
        _ => None,
    }
}

fn get_root_properties_field(props: &RootProperties, name: &str) -> Option<String> {
    match name {
        "Identity" => props.identity.as_ref().map(|s| s.to_string()),
        "DesktopEntry" => props.desktop_entry.as_ref().map(|n| n.to_string()),
        "Fullscreen" => props.fullscreen.as_ref().map(|s| s.to_string()),
        "HasTrackList" => props.has_track_list.as_ref().map(|s| s.to_string()),
        "SupportedMimeTypes" => props.supported_mime_types.as_ref().map(|v| v.join(", ")),
        "SupportedUriSchemes" => props.supported_uri_schemes.as_ref().map(|v| v.join(", ")),
        "CanSetFullscreen" => props.can_set_fullscreen.as_ref().map(|s| s.to_string()),
        "CanQuit" => props.can_quit.as_ref().map(|s| s.to_string()),
        "CanRaise" => props.can_raise.as_ref().map(|s| s.to_string()),
        _ => None,
    }
}

fn get_player_properties_field(props: &PlayerProperties, name: &str) -> Option<String> {
    match name {
        "PlaybackStatus" => props.playback_status.as_ref().map(|s| s.to_string()),
        "LoopStatus" => props.loop_status.as_ref().map(|s| s.to_string()),
        "Shuffle" => props.shuffle.as_ref().map(|s| s.to_string()),
        "Volume" => props.volume.as_ref().map(|s| s.to_string()),
        "Position" => props.position.as_ref().map(|s| s.to_string()),
        "Rate" => props.rate.as_ref().map(|s| s.to_string()),
        "MinimumRate" => props.minimum_rate.as_ref().map(|s| s.to_string()),
        "MaximumRate" => props.maximum_rate.as_ref().map(|s| s.to_string()),
        "CanControl" => props.can_control.as_ref().map(|s| s.to_string()),
        "CanPlay" => props.can_play.as_ref().map(|s| s.to_string()),
        "CanPause" => props.can_pause.as_ref().map(|s| s.to_string()),
        "CanSeek" => props.can_seek.as_ref().map(|s| s.to_string()),
        "CanGoNext" => props.can_go_next.as_ref().map(|s| s.to_string()),
        "CanGoPrevious" => props.can_go_previous.as_ref().map(|s| s.to_string()),
        _ => None,
    }
}

async fn run() -> Result<()> {
    let cmd = Command::new(crate_name!())
        .version(crate_version!())
        .disable_colored_help(true)
        .disable_help_subcommand(true)
        .arg_required_else_help(true)
        .subcommand_required(true)

        .subcommand(Command::new("next-player").about("Switch to next player"))
        .subcommand(Command::new("previous-player").about("Switch to previous player"))
        .subcommand(
            Command::new("set-player")
                .about("Set active player")
                .arg(Arg::new("player").required(true).help("Name of player")),
        )
        .subcommand(Command::new("raise").about("Raise active player"))
        .subcommand(Command::new("quit").about("Quit active player"))

        .subcommand(Command::new("next").about("Skip to next track"))
        .subcommand(Command::new("previous").about("Skip to previous track"))
        .subcommand(Command::new("play").about("Play current track"))
        .subcommand(Command::new("pause").about("Pause current track"))
        .subcommand(Command::new("play-pause").about("Play/Pause current track"))
        .subcommand(Command::new("stop").about("Stop current track"))
        .subcommand(
            Command::new("seek").about("Seek in current track").arg(
                Arg::new("offset")
                    .required(true)
                    .value_parser(clap::value_parser!(i64))
                    .help("Offset of current position in microseconds"),
            ),
        )
        .subcommand(
            Command::new("set-position")
                .about("Set the position of current track")
                .arg(
                    Arg::new("position")
                        .required(true)
                        .value_parser(clap::value_parser!(i64))
                        .help("Position in microseconds"),
                ),
        )
        .subcommand(
            Command::new("set-volume")
                .about("Set the volume of active player")
                .arg(
                    Arg::new("volume")
                        .required(true)
                        .value_parser(clap::value_parser!(VolumeCommand))
                        .help("Floating value between 0 and 1 with optional suffix '+' or '-' to increment or decrement"),
                ),
        )
        .subcommand(
            Command::new("set-rate")
                .about("Set the rate of active player")
                .arg(
                    Arg::new("rate")
                        .required(true)
                        .value_parser(clap::value_parser!(f64))
                        .help("Floating value between minimum_rate and maximum_rate"),
                ),
        )
        .subcommand(
            Command::new("set-shuffle")
                .about("Set shuffle of active player")
                .arg(
                    Arg::new("enabled")
                        .required(true)
                        .value_parser(["true", "false"]),
                ),
        )
        .subcommand(
            Command::new("set-loop")
                .about("Set the loop status of active player")
                .arg(
                    Arg::new("status")
                        .required(true)
                        .value_parser(clap::value_parser!(LoopStatusArg)),
                ),
        )
        .subcommand(
            Command::new("open")
                .about("Open URI to play")
                .arg(Arg::new("uri").required(true).help("URI to open")),
        )

        .subcommand(Command::new("player").about("Get active player"))
        .subcommand(Command::new("players").about("Get all players"))
        .subcommand(
            Command::new("metadata")
                .about("Get metadata of current track")
                .arg(Arg::new("field").required(false)),
        )
        .subcommand(
            Command::new("properties")
                .about("Get root properties")
                .arg(Arg::new("field").required(false)),
        )
        .subcommand(
            Command::new("player-properties")
                .about("Get active player properties")
                .arg(Arg::new("field").required(false)),
        );

    let matches = cmd.try_get_matches()?;
    let mut root = Root::new().await?;

    if let Some(dirs) = ProjectDirs::from("", "", "mprisctl") {
        if let Some(state_dir) = dirs.state_dir() {
            if state_dir.exists() {
                let mut file = PathBuf::from(state_dir);
                file.push("active_player");
                if file.exists() {
                    let _ = root.set_player(&read_to_string(file).await?);
                }
            }
        }
    }

    match matches.subcommand() {
        Some(("next-player", _)) => root.next_player()?,
        Some(("previous-player", _)) => root.previous_player()?,
        Some(("set-player", sub_matches)) => {
            root.set_player(sub_matches.get_one::<String>("player").unwrap())?
        }
        Some(("raise", _)) => root.raise().await?,
        Some(("quit", _)) => root.quit().await?,

        Some(("next", _)) => root.get_player()?.next().await?,
        Some(("previous", _)) => root.get_player()?.previous().await?,
        Some(("play", _)) => root.get_player()?.play().await?,
        Some(("pause", _)) => root.get_player()?.pause().await?,
        Some(("play-pause", _)) => root.get_player()?.play_pause().await?,
        Some(("stop", _)) => root.get_player()?.stop().await?,
        Some(("seek", sub_matches)) => {
            root.get_player()?
                .seek(*sub_matches.get_one::<i64>("offset").unwrap())
                .await?
        }
        Some(("set-position", sub_matches)) => {
            root.get_player()?
                .set_position(*sub_matches.get_one::<i64>("position").unwrap())
                .await?
        }
        Some(("set-volume", sub_matches)) => {
            let player = root.get_player()?;
            let adjust_volume = |c: f64, d: f64| {
                ((c * 100.0).round() as i64 + (d * 100.0).round() as i64).clamp(0, 100) as f64
                    / 100.0
            };
            match sub_matches.get_one::<VolumeCommand>("volume").unwrap() {
                VolumeCommand::Set(v) => player.set_volume(*v).await?,
                VolumeCommand::Increment(v) => {
                    player
                        .set_volume(adjust_volume(player.get_volume().await?, *v))
                        .await?
                }
                VolumeCommand::Decrement(v) => {
                    player
                        .set_volume(adjust_volume(player.get_volume().await?, -*v))
                        .await?
                }
            }
        }
        Some(("set-rate", sub_matches)) => {
            root.get_player()?
                .set_rate(*sub_matches.get_one::<f64>("rate").unwrap())
                .await?
        }
        Some(("set-shuffle", sub_matches)) => {
            let enabled = sub_matches.get_one::<String>("enabled").unwrap() == "true";
            root.get_player()?.set_shuffle(enabled).await?
        }
        Some(("set-loop", sub_matches)) => {
            let loop_status = match sub_matches.get_one::<LoopStatusArg>("status").unwrap() {
                LoopStatusArg::None => LoopStatus::None,
                LoopStatusArg::Track => LoopStatus::Track,
                LoopStatusArg::Playlist => LoopStatus::Playlist,
            };
            root.get_player()?.set_loop_status(loop_status).await?
        }
        Some(("open", sub_matches)) => {
            root.get_player()?
                .open_uri(sub_matches.get_one::<String>("uri").unwrap())
                .await?
        }

        Some(("player", _)) => println!("{}", root.get_player()?.name),
        Some(("players", _)) => println!("{}", root.get_players().join("\n")),
        Some(("metadata", sub_matches)) => {
            let metadata = root.get_player()?.get_metadata().await?;
            if let Some(field) = sub_matches.get_one::<String>("field") {
                if let Some(value) = get_metadata_field(&metadata, field) {
                    println!("{}", value);
                }
            } else {
                [
                    "mpris:artUrl",
                    "mpris:length",
                    "mpris:trackid",
                    "xesam:album",
                    "xesam:albumArtist",
                    "xesam:artist",
                    "xesam:asText",
                    "xesam:audioBPM",
                    "xesam:autoRating",
                    "xesam:comment",
                    "xesam:composer",
                    "xesam:contentCreated",
                    "xesam:discNumber",
                    "xesam:firstUsed",
                    "xesam:genre",
                    "xesam:lastUsed",
                    "xesam:lyricist",
                    "xesam:title",
                    "xesam:trackNumber",
                    "xesam:url",
                    "xesam:userCount",
                    "xesam:userRating",
                ]
                .iter()
                .for_each(|f| {
                    if let Some(value) = get_metadata_field(&metadata, f) {
                        println!("{}: {}", f, value);
                    }
                });
            }
        }
        Some(("properties", sub_matches)) => {
            let props = root.get_properties().await?;
            if let Some(field) = sub_matches.get_one::<String>("field") {
                if let Some(value) = get_root_properties_field(&props, field) {
                    println!("{}", value);
                }
            } else {
                [
                    "Identity",
                    "DesktopEntry",
                    "Fullscreen",
                    "HasTrackList",
                    "SupportedMimeTypes",
                    "SupportedUriSchemes",
                    "CanSetFullscreen",
                    "CanQuit",
                    "CanRaise",
                ]
                .iter()
                .for_each(|f| {
                    if let Some(value) = get_root_properties_field(&props, f) {
                        println!("{}: {}", f, value);
                    }
                });
            }
        }
        Some(("player-properties", sub_matches)) => {
            let props = root.get_player()?.get_properties().await?;
            if let Some(field) = sub_matches.get_one::<String>("field") {
                if let Some(value) = get_player_properties_field(&props, field) {
                    println!("{}", value);
                }
            } else {
                [
                    "PlaybackStatus",
                    "LoopStatus",
                    "Shuffle",
                    "Volume",
                    "Position",
                    "Rate",
                    "MinimumRate",
                    "MaximumRate",
                    "CanControl",
                    "CanPlay",
                    "CanPause",
                    "CanSeek",
                    "CanGoNext",
                    "CanGoPrevious",
                ]
                .iter()
                .for_each(|f| {
                    if let Some(value) = get_player_properties_field(&props, f) {
                        println!("{}: {}", f, value);
                    }
                });
            }
        }

        _ => unreachable!(),
    };

    if let Some(dirs) = ProjectDirs::from("", "", "mprisctl") {
        if let Some(state_dir) = dirs.state_dir() {
            if !state_dir.exists() {
                create_dir_all(state_dir)?;
            }

            let mut file = PathBuf::from(state_dir);
            file.push("active_player");
            if !file.exists() {
                File::create(&file)?;
            }

            if let Ok(player) = root.get_player() {
                if player.name != read_to_string(&file).await? {
                    write(&file, &player.name)?;
                }
            }
        }
    }

    Ok(())
}

#[tokio::main]
async fn main() {
    if let Err(error) = run().await {
        match error {
            Error::Clap(error) => match error.kind() {
                clap::error::ErrorKind::DisplayHelp => {
                    eprintln!("{}", error);
                    std::process::exit(0);
                }
                clap::error::ErrorKind::DisplayHelpOnMissingArgumentOrSubcommand => {
                    eprintln!("{}", error);
                    std::process::exit(0);
                }
                clap::error::ErrorKind::DisplayVersion => {
                    eprint!("{}", error);
                    std::process::exit(0);
                }
                _ => {
                    eprintln!(
                        "mprisctl: {}",
                        error
                            .to_string()
                            .strip_prefix("error: ")
                            .unwrap_or_default()
                    );
                    std::process::exit(1);
                }
            },
            _ => {
                eprintln!("mprisctl: {}", error);
                std::process::exit(1);
            }
        }
    }
}

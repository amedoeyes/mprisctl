# mprisctl

`mprisctl` is a command-line utility that allows you to control MPRIS-compatible media players.

## Features

- Switch between MPRIS-compatible media players
- Control playback
- Adjust volume
- Manage track list
- Retrieve and display properties and metadata

## Build

Build dependencies:

- Boost
- D-Bus

To build the project, you need to have Meson and Ninja installed. Then, run the following commands:

```bash
meson setup build --buildtype=release
ninja -C build
```

## Usage

```bash
mprisctl [options]
```

### General options

- `-h, --help`: Display help message.
- ` --version`: Display version information.

### Player Management

- `-N, --next-player`: Switch to the next available MPRIS player.
- `-P, --previous-player`: Switch to the previous available MPRIS player.
- `--set-player <str>`: Specify the player to control by name.
- `-r, --raise`: Bring the specified player to the foreground.
- `-q, --quit`: Quit the specified player.

### Playback Control

- `-n, --next`: Skip to the next track.
- `-p, --previous`: Skip to the previous track.
- `--pause`: Pause playback.
- `--play`: Start playback.
- `-t, --play-pause`: Toggle between play and pause.
- `--stop`: Stop playback.
- `-s, --seek <int>`: Seek forward or backward by the specified number of microseconds.
- `-S, --set-position <int>`: Set playback position to the specified microsecond.
- `-u, --open-uri <str>`: Open and play the media from the specified URI.

### Volume Control

- `-v, --set-volume <double>`: Set the playback volume (range: 0.0 to 1.0).
- `-i, --increment-volume <double>`: Increment the playback volume by the specified amount.
- `-d, --decrement-volume <double>`: Decrement the playback volume by the specified amount.

### Track List Management

- `--add-track <str>`: Add a track to the track list by URI
- `--add-track-current`: Mark the added track as the current track
- `--add-track-after <str>`: Add the track after the specified track id
- `--remove-track <str>`: Remove a track from the track list by track id
- `--go-to-track <str>`: Jump to the specified track in the track list by track id

### Metadata and Properties

- `--display-metadata <str>`: Display metadata for the current track. Optionally, specify a field to display only that particular property's value.
- `--display-player-properties <str>`: Display properties of the current player. Optionally, specify a field to display only that particular property's value.
- `--display-mpris-properties <str>`: Display MPRIS properties of the current player. Optionally, specify a field to display only that particular property's value.
- `--display-track-list-properties <str>`: Display track list properties of the current player. Optionally, specify a field to display only that particular property's value.

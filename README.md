# MPRIS Control

Command-line tool to interact with MPRIS compatible media players.

## Installation

```sh
cargo install --locked --git https://github.com/amedoeyes/mprisctl mprisctl
```

## Usage

```
Usage: mprisctl <COMMAND>

Commands:
  next-player        Switch to next player
  previous-player    Switch to previous player
  set-player         Set active player
  raise              Raise active player
  quit               Quit active player
  next               Skip to next track
  previous           Skip to previous track
  play               Play current track
  pause              Pause current track
  play-pause         Play/Pause current track
  stop               Stop current track
  seek               Seek in current track
  set-position       Set the position of current track
  set-volume         Set the volume of active player
  set-rate           Set the rate of active player
  set-shuffle        Set shuffle of active player
  set-loop           Set the loop status of active player
  open               Open URI to play
  player             Get active player
  players            Get all players
  metadata           Get metadata of current track
  properties         Get root properties
  player-properties  Get active player properties
  completion         Generate shell completions

Options:
  -h, --help     Print help
  -V, --version  Print version
```

## Contributing

Contributions are welcome! If you notice a bug or want to add a feature, feel free to open an issue or submit a pull request.

use std::collections::HashMap;

use crate::player;
use zbus::{
    proxy,
    zvariant::{ObjectPath, OwnedValue},
};

#[proxy(
    default_path = "/org/mpris/MediaPlayer2",
    interface = "org.mpris.MediaPlayer2"
)]
pub trait Root {
    fn quit(&self) -> zbus::Result<()>;
    fn raise(&self) -> zbus::Result<()>;

    #[zbus(property)]
    fn identity(&self) -> zbus::Result<String>;

    #[zbus(property)]
    fn fullscreen(&self) -> zbus::Result<bool>;
    #[zbus(property)]
    fn set_fullscreen(&self, enabled: bool) -> zbus::Result<()>;

    #[zbus(property)]
    fn supported_uri_schemes(&self) -> zbus::Result<Vec<String>>;

    #[zbus(property)]
    fn supported_mime_types(&self) -> zbus::Result<Vec<String>>;

    #[zbus(property)]
    fn has_track_list(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_quit(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_raise(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_set_fullscreen(&self) -> zbus::Result<bool>;
}

#[proxy(
    default_path = "/org/mpris/MediaPlayer2",
    interface = "org.mpris.MediaPlayer2.Player"
)]
pub trait Player {
    fn next(&self) -> zbus::Result<()>;
    fn previous(&self) -> zbus::Result<()>;
    fn pause(&self) -> zbus::Result<()>;
    fn play_pause(&self) -> zbus::Result<()>;
    fn stop(&self) -> zbus::Result<()>;
    fn play(&self) -> zbus::Result<()>;
    fn seek(&self, offset: i64) -> zbus::Result<()>;
    fn set_position<'a>(&self, track_id: &ObjectPath<'a>, position: i64) -> zbus::Result<()>;
    fn open_uri(&self, uri: &str) -> zbus::Result<()>;

    #[zbus(property)]
    fn playback_status(&self) -> zbus::Result<player::PlaybackStatus>;

    #[zbus(property)]
    fn loop_status(&self) -> zbus::Result<player::LoopStatus>;
    #[zbus(property)]
    fn set_loop_status(&self, status: player::LoopStatus) -> zbus::Result<()>;

    #[zbus(property)]
    fn rate(&self) -> zbus::Result<f64>;
    #[zbus(property)]
    fn set_rate(&self, rate: f64) -> zbus::Result<()>;

    #[zbus(property)]
    fn shuffle(&self) -> zbus::Result<bool>;
    #[zbus(property)]
    fn set_shuffle(&self, enabled: bool) -> zbus::Result<()>;

    #[zbus(property)]
    fn metadata(&self) -> zbus::Result<HashMap<String, OwnedValue>>;

    #[zbus(property)]
    fn volume(&self) -> zbus::Result<f64>;
    #[zbus(property)]
    fn set_volume(&self, volume: f64) -> zbus::Result<()>;

    #[zbus(property)]
    fn position(&self) -> zbus::Result<i64>;

    #[zbus(property)]
    fn minimum_rate(&self) -> zbus::Result<f64>;

    #[zbus(property)]
    fn maximum_rate(&self) -> zbus::Result<f64>;

    #[zbus(property)]
    fn cat_go_next(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_go_previous(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_play(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_pause(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_seek(&self) -> zbus::Result<bool>;

    #[zbus(property)]
    fn can_control(&self) -> zbus::Result<bool>;
}

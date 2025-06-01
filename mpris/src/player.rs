use zbus::{
    Connection,
    fdo::PropertiesProxy,
    zvariant::{self, ObjectPath, OwnedValue, Type, Value},
};

use crate::{Result, proxy::PlayerProxy, util::extract_value};

#[derive(Debug)]
pub struct Player {
    pub connection: Connection,
    pub name: String,
}

#[derive(Debug)]
pub struct PlayerProperties {
    pub playback_status: Option<String>,
    pub loop_status: Option<String>,
    pub shuffle: Option<bool>,
    pub volume: Option<f64>,
    pub position: Option<i64>,
    pub rate: Option<f64>,
    pub minimum_rate: Option<f64>,
    pub maximum_rate: Option<f64>,

    pub can_control: Option<bool>,
    pub can_play: Option<bool>,
    pub can_pause: Option<bool>,
    pub can_seek: Option<bool>,
    pub can_go_next: Option<bool>,
    pub can_go_previous: Option<bool>,
}

impl Player {
    async fn proxy(&self) -> Result<PlayerProxy> {
        Ok(PlayerProxy::builder(&self.connection)
            .destination(self.name.as_str())?
            .build()
            .await?)
    }

    pub async fn get_properties(&self) -> Result<PlayerProperties> {
        let props = PropertiesProxy::builder(&self.connection)
            .destination(self.name.as_str())?
            .path("/org/mpris/MediaPlayer2")?
            .build()
            .await?
            .get_all(zbus::names::InterfaceName::try_from("org.mpris.MediaPlayer2.Player").unwrap())
            .await?;

        Ok(PlayerProperties {
            playback_status: extract_value(&props, "PlaybackStatus"),
            loop_status: extract_value(&props, "LoopStatus"),
            volume: extract_value(&props, "Volume"),
            position: extract_value(&props, "Position"),
            shuffle: extract_value(&props, "Shuffle"),
            rate: extract_value(&props, "Rate"),
            minimum_rate: extract_value(&props, "MinimumRate"),
            maximum_rate: extract_value(&props, "MaximumRate"),

            can_control: extract_value(&props, "CanControl"),
            can_play: extract_value(&props, "CanPlay"),
            can_pause: extract_value(&props, "CanPause"),
            can_seek: extract_value(&props, "CanSeek"),
            can_go_next: extract_value(&props, "CanGoNext"),
            can_go_previous: extract_value(&props, "CanGoPrevious"),
        })
    }

    pub async fn get_metadata(&self) -> Result<PlayerMetadata> {
        let metadata = self.proxy().await?.metadata().await?;
        Ok(PlayerMetadata {
            mpris_art_url: extract_value(&metadata, "mpris:artUrl"),
            mpris_length: extract_value(&metadata, "mpris:length"),
            mpris_trackid: extract_value::<ObjectPath>(&metadata, "mpris:trackid")
                .map(|x| x.to_string()),
            xesam_album: extract_value(&metadata, "xesam:album"),
            xesam_album_artist: extract_value(&metadata, "xesam:albumArtist"),
            xesam_artist: extract_value(&metadata, "xesam:artist"),
            xesam_as_text: extract_value(&metadata, "xesam:asText"),
            xesam_audio_bpm: extract_value(&metadata, "xesam:audioBPM"),
            xesam_auto_rating: extract_value(&metadata, "xesam:autoRating"),
            xesam_comment: extract_value(&metadata, "xesam:comment"),
            xesam_composer: extract_value(&metadata, "xesam:composer"),
            xesam_content_created: extract_value(&metadata, "xesam:contentCreated"),
            xesam_disc_number: extract_value(&metadata, "xesam:discNumber"),
            xesam_first_used: extract_value(&metadata, "xesam:firstUsed"),
            xesam_genre: extract_value(&metadata, "xesam:genre"),
            xesam_last_used: extract_value(&metadata, "xesam:lastUsed"),
            xesam_lyricist: extract_value(&metadata, "xesam:lyricist"),
            xesam_title: extract_value(&metadata, "xesam:title"),
            xesam_track_number: extract_value(&metadata, "xesam:trackNumber"),
            xesam_url: extract_value(&metadata, "xesam:url"),
            xesam_user_count: extract_value(&metadata, "xesam:useCount"),
            xesam_user_rating: extract_value(&metadata, "xesam:userRating"),
        })
    }

    pub async fn next(&self) -> Result<()> {
        self.proxy().await?.next().await?;
        Ok(())
    }

    pub async fn previous(&self) -> Result<()> {
        self.proxy().await?.previous().await?;
        Ok(())
    }

    pub async fn play(&self) -> Result<()> {
        self.proxy().await?.play().await?;
        Ok(())
    }

    pub async fn pause(&self) -> Result<()> {
        self.proxy().await?.pause().await?;
        Ok(())
    }

    pub async fn play_pause(&self) -> Result<()> {
        self.proxy().await?.play_pause().await?;
        Ok(())
    }

    pub async fn stop(&self) -> Result<()> {
        self.proxy().await?.stop().await?;
        Ok(())
    }

    pub async fn seek(&self, offset: i64) -> Result<()> {
        self.proxy().await?.seek(offset).await?;
        Ok(())
    }

    pub async fn set_position(&self, position: i64) -> Result<()> {
        if let Some(trackid) = self.get_metadata().await?.mpris_trackid {
            self.proxy()
                .await?
                .set_position(&ObjectPath::try_from(trackid.clone())?, position)
                .await?;
        }

        Ok(())
    }

    pub async fn get_volume(&self) -> Result<f64> {
        Ok(self.proxy().await?.volume().await?)
    }

    pub async fn set_volume(&self, volume: f64) -> Result<()> {
        self.proxy().await?.set_volume(volume).await?;
        Ok(())
    }

    pub async fn set_shuffle(&self, enabled: bool) -> Result<()> {
        self.proxy().await?.set_shuffle(enabled).await?;
        Ok(())
    }

    pub async fn set_rate(&self, rate: f64) -> Result<()> {
        self.proxy().await?.set_rate(rate).await?;
        Ok(())
    }

    pub async fn set_loop_status(&self, status: LoopStatus) -> Result<()> {
        self.proxy().await?.set_loop_status(status).await?;
        Ok(())
    }

    pub async fn open_uri(&self, uri: &str) -> Result<()> {
        self.proxy().await?.open_uri(uri).await?;
        Ok(())
    }
}

#[derive(Debug)]
pub struct PlayerMetadata {
    pub mpris_art_url: Option<String>,
    pub mpris_length: Option<i64>,
    pub mpris_trackid: Option<String>,
    pub xesam_album: Option<String>,
    pub xesam_album_artist: Option<Vec<String>>,
    pub xesam_artist: Option<Vec<String>>,
    pub xesam_as_text: Option<String>,
    pub xesam_audio_bpm: Option<i64>,
    pub xesam_auto_rating: Option<f64>,
    pub xesam_comment: Option<Vec<String>>,
    pub xesam_composer: Option<Vec<String>>,
    pub xesam_content_created: Option<String>,
    pub xesam_disc_number: Option<i64>,
    pub xesam_first_used: Option<String>,
    pub xesam_genre: Option<Vec<String>>,
    pub xesam_last_used: Option<String>,
    pub xesam_lyricist: Option<Vec<String>>,
    pub xesam_title: Option<String>,
    pub xesam_track_number: Option<i64>,
    pub xesam_url: Option<String>,
    pub xesam_user_count: Option<i64>,
    pub xesam_user_rating: Option<f64>,
}

#[derive(Debug, Type)]
pub enum PlaybackStatus {
    Playing,
    Paused,
    Stopped,
}

impl From<PlaybackStatus> for Value<'static> {
    fn from(status: PlaybackStatus) -> Self {
        match status {
            PlaybackStatus::Playing => "Playing".into(),
            PlaybackStatus::Paused => "Paused".into(),
            PlaybackStatus::Stopped => "Stopped".into(),
        }
    }
}

impl TryFrom<OwnedValue> for PlaybackStatus {
    type Error = zvariant::Error;

    fn try_from(value: OwnedValue) -> std::result::Result<Self, Self::Error> {
        match Value::from(value) {
            Value::Str(s) => match s.as_str() {
                "Playing" => Ok(PlaybackStatus::Playing),
                "Paused" => Ok(PlaybackStatus::Paused),
                "Stopped" => Ok(PlaybackStatus::Stopped),
                _ => Err(zvariant::Error::IncorrectType),
            },
            _ => Err(zvariant::Error::IncorrectType),
        }
    }
}

#[derive(Debug, Type)]
pub enum LoopStatus {
    None,
    Track,
    Playlist,
}

impl From<LoopStatus> for zvariant::Value<'static> {
    fn from(status: LoopStatus) -> Self {
        match status {
            LoopStatus::None => "None".into(),
            LoopStatus::Track => "Track".into(),
            LoopStatus::Playlist => "Playlist".into(),
        }
    }
}

impl TryFrom<zvariant::OwnedValue> for LoopStatus {
    type Error = zvariant::Error;

    fn try_from(value: zvariant::OwnedValue) -> std::result::Result<Self, Self::Error> {
        match zvariant::Value::from(value) {
            zvariant::Value::Str(s) => match s.as_str() {
                "None" => Ok(LoopStatus::None),
                "Track" => Ok(LoopStatus::Track),
                "Playlist" => Ok(LoopStatus::Playlist),
                _ => Err(zvariant::Error::IncorrectType),
            },
            _ => Err(zvariant::Error::IncorrectType),
        }
    }
}

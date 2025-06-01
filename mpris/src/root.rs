use zbus::{
    Connection,
    fdo::{DBusProxy, PropertiesProxy},
    names::InterfaceName,
};

use crate::{Error, Result, player::Player, proxy::RootProxy, util::extract_value};

#[derive(Debug)]
pub struct Root {
    connection: Connection,
    players: Vec<String>,
    player_index: Option<usize>,
    player: Option<Player>,
}

impl Root {
    pub async fn new() -> Result<Self> {
        let connection = Connection::session().await?;

        let mut players: Vec<String> = DBusProxy::new(&connection)
            .await?
            .list_names()
            .await?
            .into_iter()
            .map(|x| x.to_string())
            .filter(|s| s.starts_with("org.mpris.MediaPlayer2."))
            .collect();
        players.sort();

        let mut mpris = Self {
            connection,
            players,
            player_index: None,
            player: None,
        };

        if let Some(first) = mpris.players.first().cloned() {
            mpris.set_player(&first)?;
        }

        Ok(mpris)
    }

    async fn proxy(&self) -> Result<RootProxy> {
        if let Some(player) = &self.player {
            Ok(RootProxy::builder(&self.connection)
                .destination(player.name.as_str())?
                .build()
                .await?)
        } else {
            Err(Error::NoPlayerFound)
        }
    }

    pub async fn get_properties(&self) -> Result<RootProperties> {
        if let Some(player) = self.player.as_ref() {
            let props = PropertiesProxy::builder(&self.connection)
                .destination(player.name.as_str())?
                .path("/org/mpris/MediaPlayer2")?
                .build()
                .await?
                .get_all(InterfaceName::try_from("org.mpris.MediaPlayer2").unwrap())
                .await?;

            Ok(RootProperties {
                identity: extract_value(&props, "Identity"),
                desktop_entry: extract_value(&props, "DesktopEntry"),
                fullscreen: extract_value(&props, "Fullscreen"),
                has_track_list: extract_value(&props, "HasTrackList"),
                supported_uri_schemes: extract_value(&props, "SupportedUriSchemes"),
                supported_mime_types: extract_value(&props, "SupportedMimeTypes"),

                can_set_fullscreen: extract_value(&props, "CanSetFullscreen"),
                can_quit: extract_value(&props, "CanQuit"),
                can_raise: extract_value(&props, "CanRaise"),
            })
        } else {
            Err(Error::NoPlayerFound)
        }
    }

    pub async fn quit(&self) -> Result<()> {
        self.proxy().await?.quit().await?;
        Ok(())
    }

    pub async fn raise(&self) -> Result<()> {
        self.proxy().await?.raise().await?;
        Ok(())
    }

    pub fn get_players(&mut self) -> &Vec<String> {
        &self.players
    }

    pub fn get_player(&mut self) -> Result<&Player> {
        self.player.as_ref().ok_or(Error::NoPlayerFound)
    }

    pub fn set_player(&mut self, name: &str) -> Result<()> {
        if let Some(index) = self.players.iter().position(|x| x.eq(name)) {
            self.player_index = Some(index);
            self.player = Some(Player {
                connection: self.connection.clone(),
                name: name.to_string(),
            });

            Ok(())
        } else {
            Err(Error::PlayerNotFound(name.to_string()))
        }
    }

    pub fn next_player(&mut self) -> Result<()> {
        if let Some(index) = self.player_index {
            self.set_player(&self.players[(index + 1) % self.players.len()].clone())?
        }

        Ok(())
    }

    pub fn previous_player(&mut self) -> Result<()> {
        if let Some(index) = self.player_index {
            self.set_player(&self.players[(index - 1) % self.players.len()].clone())?
        }

        Ok(())
    }
}

#[derive(Debug)]
pub struct RootProperties {
    pub identity: Option<String>,
    pub desktop_entry: Option<String>,
    pub fullscreen: Option<bool>,
    pub has_track_list: Option<bool>,
    pub supported_mime_types: Option<Vec<String>>,
    pub supported_uri_schemes: Option<Vec<String>>,

    pub can_set_fullscreen: Option<bool>,
    pub can_quit: Option<bool>,
    pub can_raise: Option<bool>,
}

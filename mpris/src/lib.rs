pub mod player;
mod proxy;
pub mod root;
mod util;

#[derive(Debug, thiserror::Error)]
pub enum Error {
    #[error("No player found")]
    NoPlayerFound,

    #[error("Player '{0}' not found")]
    PlayerNotFound(String),

    #[error("DBus error: {0}")]
    DBus(#[from] zbus::Error),

    #[error("DBus FDO error: {0}")]
    DBusFdo(#[from] zbus::fdo::Error),

    #[error("ZVariant error: {0}")]
    ZVariant(#[from] zbus::zvariant::Error),
}

pub type Result<T> = std::result::Result<T, Error>;

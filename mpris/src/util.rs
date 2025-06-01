use std::collections::HashMap;

use zbus::zvariant;

pub fn extract_value<T: TryFrom<zvariant::OwnedValue>>(
    map: &HashMap<String, zvariant::OwnedValue>,
    key: &str,
) -> Option<T> {
    map.get(key)
        .and_then(|value| T::try_from(value.clone()).ok())
}

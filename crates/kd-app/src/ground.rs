//! The ground the app hands the renderer (A11.1, A11.5): until α02b makes areas from cells, the demo area (A5.3),
//! its materials shown as the catalogue's surfaces.
//!
//! Implements PRE-20 and WLD-12 in part, see A11.5 and A5.3: each square metre of ground is drawn as its surface.

use kd_data::Catalogue;
use kd_view::AreaMeshes;
use kd_world::area::{Ground, Material};

/// The surface each of the demo's materials shows, by id, until α02a's rocks and soils name their own: soil grows
/// grass, and both kinds of bed show bare rock (T01b.3).
const DEMO_SURFACES: [(Material, &str); 5] = [
    (Material::Soil, "grass"),
    (Material::Dirt, "dirt"),
    (Material::Scree, "scree"),
    (Material::HardRock, "rock"),
    (Material::SoftRock, "rock"),
];

/// Each material's surface number, indexed by the material's byte, or the id the catalogue lacks.
pub fn surface_numbers(cat: &Catalogue) -> Result<[u8; Material::ALL.len()], String> {
    let mut out = [0u8; Material::ALL.len()];
    for (m, id) in DEMO_SURFACES {
        let s = cat
            .surface(id)
            .ok_or_else(|| format!("the catalogue has no surface {id:?}"))?;
        out[m as usize] = u8::try_from(s.number).map_err(|_| format!("surface {id:?} is numbered over 255"))?;
    }
    Ok(out)
}

/// An area's ground as the renderer takes it (A11.1): heights in metres above its lowest point, and each square's
/// surface number from its material.
pub fn area_meshes(g: &Ground, surfaces: &[u8; Material::ALL.len()]) -> AreaMeshes {
    AreaMeshes {
        id: g.id,
        base_m: g.base_dm as f32 / 10.0,
        heights: g.heights.iter().map(|&h| f32::from(h) / 10.0).collect(),
        surfaces: g.material.iter().map(|&m| surfaces[usize::from(m)]).collect(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-20 WLD-12
    #[test]
    fn demo_materials_show_surfaces() {
        let cat = Catalogue::load(crate::CATALOGUE).expect("the embedded catalogue");
        let numbers = surface_numbers(&cat).expect("every material has its surface");
        let id = |m: Material| {
            let n = numbers[m as usize];
            cat.surfaces
                .iter()
                .find(|s| s.number == u16::from(n))
                .map(|s| s.id.as_str())
        };
        assert_eq!(id(Material::Soil), Some("grass"));
        assert_eq!(id(Material::Dirt), Some("dirt"));
        assert_eq!(id(Material::Scree), Some("scree"));
        assert_eq!(
            (id(Material::HardRock), id(Material::SoftRock)),
            (Some("rock"), Some("rock"))
        );
        // Every material is mapped once, and rock draws as rock.
        assert!(
            Material::ALL
                .iter()
                .all(|m| DEMO_SURFACES.iter().filter(|d| d.0 == *m).count() == 1)
        );
        assert!(cat.surface("rock").is_some_and(|r| r.rock));
    }
}

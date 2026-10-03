//! Plants on the cells (A5.5): how dense each plant group stands in a cell, the share area contents test their
//! spots against and coarse ground draws its cover by, so coarse woods match the areas they dissolve into. Until
//! α02d brings species and their seasons, a group's density is the cell's cover share of it.
//!
//! The plan put `density` in `kd-life`, which sits beside `kd-world` and so cannot read its cells (A2.3); A5.1 lists
//! `plants` among `kd-world`'s modules, and here it is.
//!
//! Implements WLD-12 and PRE-03 in part, see A5.5: one density for the picture and the world alike.

use kd_core::geo::CellIx;
use kd_core::time::GameTime;

use crate::cells::CellState;

/// The plant groups of a cell's cover (A5.2), by their place in its shares; the fifth share is bare ground.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum PlantGroup {
    Trees = 0,
    Bushes = 1,
    Herbs = 2,
    Reeds = 3,
}

impl PlantGroup {
    pub const ALL: [PlantGroup; 4] = [
        PlantGroup::Trees,
        PlantGroup::Bushes,
        PlantGroup::Herbs,
        PlantGroup::Reeds,
    ];
}

/// How dense a plant group stands in cell `c` on `date`, from 0 to 1: its share of the cell's cover, the same in
/// every season until species come (α02d).
pub fn density(st: &CellState, c: CellIx, group: PlantGroup, _date: GameTime) -> f32 {
    f32::from(st.cover[c.0 as usize][group as usize]) / 255.0
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: WLD-12 PRE-03
    #[test]
    fn density_is_the_cover_share() {
        let mut st = CellState::new();
        let c = CellIx::at(1000, 244);
        st.cover[c.0 as usize] = [140, 38, 64, 5, 8];
        let at = |g| density(&st, c, g, GameTime(0));
        assert_eq!(at(PlantGroup::Trees), 140.0 / 255.0);
        assert_eq!(at(PlantGroup::Bushes), 38.0 / 255.0);
        assert_eq!(at(PlantGroup::Herbs), 64.0 / 255.0);
        assert_eq!(at(PlantGroup::Reeds), 5.0 / 255.0);
        // The groups and the bare ground share the whole cell, and a bare cell grows nothing.
        let sum: f32 = PlantGroup::ALL.iter().map(|&g| at(g)).sum();
        assert!((sum + 8.0 / 255.0 - 1.0).abs() < 1e-6);
        let bare = CellIx::at(1001, 244);
        assert!(
            PlantGroup::ALL
                .iter()
                .all(|&g| density(&st, bare, g, GameTime(0)) == 0.0)
        );
    }
}

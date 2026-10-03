//! Uids (A3.4): every entity's permanent identity, 64 bits never reused and made without a counter two threads
//! share. The top two bits pick the space: play (the window of game time, the lane and an ordinal), area (the area,
//! its record's epoch and an ordinal), place (a kind of place and its index, as A3.7's grids number them) and
//! set-up (the stage and an ordinal). Handles and stores join with the first beings.
//!
//! Implements WLD-01 and TIM-16, see A3.4 and A3.7: places named by their grid indices, the same on every run.

/// A permanent identity (A3.4).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Uid(pub u64);

/// The four spaces of uids, the top two bits (A3.4).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Space {
    /// Beings and batch systems during play: window (32 bits) · lane (12) · ordinal (18).
    Play = 0,
    /// Area processes and catch-ups: area index (25) · epoch (12) · ordinal (25).
    Area = 1,
    /// Places: kind (6) · index (56).
    Place = 2,
    /// World settling, the bands' life before history and scene set-up: stage (14) · ordinal (48).
    SetUp = 3,
}

/// The kinds of place a place uid names (A3.4, A5.3): grid cells by their A3.7 index, the rest by their own tables'.
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(u8)]
pub enum PlaceKind {
    Cell = 0,
    Area = 1,
    Weather = 2,
    Region = 3,
    River = 4,
    Cave = 5,
    Feature = 6,
    Spot = 7,
    Patch = 8,
}

impl PlaceKind {
    pub const ALL: [PlaceKind; 9] = [
        PlaceKind::Cell,
        PlaceKind::Area,
        PlaceKind::Weather,
        PlaceKind::Region,
        PlaceKind::River,
        PlaceKind::Cave,
        PlaceKind::Feature,
        PlaceKind::Spot,
        PlaceKind::Patch,
    ];
}

/// The widths of each space's fields, from the top, after the two bits of the space.
const PLAY: [u32; 3] = [32, 12, 18];
const AREA: [u32; 3] = [25, 12, 25];
const PLACE: [u32; 2] = [6, 56];
const SET_UP: [u32; 2] = [14, 48];

/// `fields` packed below the space's two bits, each `widths[i]` bits, the first highest; a field too wide for its
/// bits is a bug, caught in tests and debug builds, and masked otherwise.
fn pack(space: Space, fields: &[u64], widths: &[u32]) -> Uid {
    let mut v = space as u64;
    for (&f, &w) in fields.iter().zip(widths) {
        debug_assert!(f < 1 << w, "a uid field of {f} does not fit {w} bits");
        v = v << w | (f & ((1 << w) - 1));
    }
    Uid(v)
}

/// The fields of `uid` below its space's two bits, by their widths.
fn unpack<const N: usize>(uid: Uid, widths: [u32; N]) -> [u64; N] {
    let mut out = [0; N];
    let mut shift = 62;
    for (o, w) in out.iter_mut().zip(widths) {
        shift -= w;
        *o = uid.0 >> shift & ((1 << w) - 1);
    }
    out
}

impl Uid {
    /// A uid made during play: the window of game time (game time ÷ `WINDOW`), the lane (clusters 0–4,031, barrier
    /// lanes 4,032–4,095) and the window's ordinal in that lane (A3.4).
    pub fn play(window: u32, lane: u16, ordinal: u32) -> Uid {
        pack(
            Space::Play,
            &[u64::from(window), u64::from(lane), u64::from(ordinal)],
            &PLAY,
        )
    }

    /// A uid made by an area's processes: its area, its record's epoch and its ordinal there (A3.4).
    pub fn area(area: u32, epoch: u16, ordinal: u32) -> Uid {
        pack(
            Space::Area,
            &[u64::from(area), u64::from(epoch), u64::from(ordinal)],
            &AREA,
        )
    }

    /// A place's uid: its kind and its index (A3.4); a cell's, an area's, a weather cell's and a region's index is
    /// its A3.7 grid index.
    pub fn place(kind: PlaceKind, index: u64) -> Uid {
        pack(Space::Place, &[kind as u64, index], &PLACE)
    }

    /// A uid made while the world is set up: the stage and its ordinal (A3.4).
    pub fn set_up(stage: u16, ordinal: u64) -> Uid {
        pack(Space::SetUp, &[u64::from(stage), ordinal], &SET_UP)
    }

    /// Its space.
    pub fn space(self) -> Space {
        match self.0 >> 62 {
            0 => Space::Play,
            1 => Space::Area,
            2 => Space::Place,
            _ => Space::SetUp,
        }
    }

    /// A place uid's kind and index; none for another space or a kind no place has.
    pub fn as_place(self) -> Option<(PlaceKind, u64)> {
        if self.space() != Space::Place {
            return None;
        }
        let [kind, index] = unpack(self, PLACE);
        let kind = *PlaceKind::ALL.get(kind as usize)?;
        Some((kind, index))
    }

    /// A play uid's window, lane and ordinal.
    pub fn as_play(self) -> Option<(u32, u16, u32)> {
        let [w, l, o] = unpack(self, PLAY);
        (self.space() == Space::Play).then_some((w as u32, l as u16, o as u32))
    }

    /// An area uid's area, epoch and ordinal.
    pub fn as_area(self) -> Option<(u32, u16, u32)> {
        let [a, e, o] = unpack(self, AREA);
        (self.space() == Space::Area).then_some((a as u32, e as u16, o as u32))
    }

    /// A set-up uid's stage and ordinal.
    pub fn as_set_up(self) -> Option<(u16, u64)> {
        let [s, o] = unpack(self, SET_UP);
        (self.space() == Space::SetUp).then_some((s as u16, o))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::geo::{AreaId, CellIx, RegionIx, WeatherIx};

    // checks: WLD-01 TIM-16
    #[test]
    fn place_uid_fields() {
        // A place uid is tag 10, its kind in the next 6 bits and its index in the low 56, and reads back as made.
        let c = CellIx::at(1000, 244);
        let u = Uid::place(PlaceKind::Cell, u64::from(c.0));
        assert_eq!(u.0 >> 62, 0b10);
        assert_eq!(u.0 >> 56 & 63, PlaceKind::Cell as u64);
        assert_eq!(u.0 & ((1 << 56) - 1), u64::from(c.0));
        assert_eq!(u.as_place(), Some((PlaceKind::Cell, u64::from(c.0))));
        for (k, kind) in PlaceKind::ALL.iter().enumerate() {
            assert_eq!(*kind as usize, k);
            for index in [0, 1, 487_000, (1 << 56) - 1] {
                let u = Uid::place(*kind, index);
                assert_eq!((u.space(), u.as_place()), (Space::Place, Some((*kind, index))));
            }
        }
        // The grids' places are distinct uids, kind by kind, however their indices compare.
        let area = AreaId::at(4001, 977);
        let ids = [
            Uid::place(PlaceKind::Cell, u64::from(area.cell().0)),
            Uid::place(PlaceKind::Area, u64::from(area.0)),
            Uid::place(PlaceKind::Weather, u64::from(WeatherIx::at(100, 24).0)),
            Uid::place(PlaceKind::Region, u64::from(RegionIx::at(10, 2).0)),
        ];
        for (i, a) in ids.iter().enumerate() {
            assert!(ids[i + 1..].iter().all(|b| b != a));
        }
        // The other spaces pack their fields in A3.4's widths and read back as made, at their limits too.
        let play = Uid::play(u32::MAX, 4_095, (1 << 18) - 1);
        assert_eq!(
            (play.space(), play.as_play()),
            (Space::Play, Some((u32::MAX, 4_095, (1 << 18) - 1)))
        );
        assert_eq!(Uid::play(7, 4_032, 3).as_play(), Some((7, 4_032, 3)));
        let area_uid = Uid::area(area.0, 4_095, (1 << 25) - 1);
        assert_eq!(area_uid.space(), Space::Area);
        assert_eq!(area_uid.as_area(), Some((area.0, 4_095, (1 << 25) - 1)));
        let set_up = Uid::set_up(16_383, 5);
        assert_eq!((set_up.space(), set_up.as_set_up()), (Space::SetUp, Some((16_383, 5))));
        // Each reads as its own space only; a place uid of a kind no place has reads as no place.
        assert_eq!((play.as_place(), play.as_area(), play.as_set_up()), (None, None, None));
        assert_eq!(Uid::place(PlaceKind::Patch, 9).as_play(), None);
        assert_eq!(Uid(0b10 << 62 | 63 << 56).as_place(), None);
        // Area uids sort by area, then epoch, then ordinal, as their records keep them.
        assert!(Uid::area(5, 0, 9) < Uid::area(5, 1, 0) && Uid::area(5, 9, 9) < Uid::area(6, 0, 0));
    }
}

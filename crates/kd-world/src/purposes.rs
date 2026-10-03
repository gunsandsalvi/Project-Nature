//! kd-world's purposes (A3.3): what each keyed draw of the world's making is about, by its permanent number in
//! system 1, `world_gen`.
//!
//! Implements TIM-16, see A3.3: every draw keyed on its place and moment, the same on every run.

kd_core::purposes! {
    system WORLD_GEN;
    1 LAND_SHELF "the seed of a land preset's coast noise" subject Place fortune None;
    2 LAND_RELIEF "the seed of a land preset's hills" subject Place fortune None;
    3 VALLEY_SHIFT "how far to one side a valley's point lies" subject Place fortune None;
    4 ROCK_BEDS "how thick a cell's top two rock beds are" subject Place fortune None;
    5 CAVE_PLACE "where along an escarpment a cave or shelter opens" subject Place fortune None;
    6 CAVE_SIZE "how deep a cave or shelter runs" subject Place fortune None;
    7 CAVE_DRY "whether a cave is dry" subject Place fortune None;
    retired [];
}

#[cfg(test)]
mod tests {
    // checks: TIM-16
    #[test]
    fn purposes_registered() {
        assert_eq!(kd_core::chance::registry::check(&[super::LIST]), Ok(()));
    }
}

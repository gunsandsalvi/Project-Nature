use super::*;

// checks: TIM-14 TIM-18
#[test]
fn date_round_trips() {
    let k = 10;
    let start = GameTime(k * YEAR);
    assert_eq!(start.date(start).to_string(), "Year 1, spring, day 1");
    let seasons = [Season::Spring, Season::Summer, Season::Autumn, Season::Winter];
    for y in 0..3u64 {
        for (s, &season) in seasons.iter().enumerate() {
            let first = start.0 + y * YEAR + s as u64 * SEASON;
            let last = first + SEASON - 1;
            let d = GameTime(first).date(start);
            assert_eq!((d.year, d.season, d.day, d.second), (y as i32 + 1, season, 1, 0));
            let d = GameTime(last).date(start);
            assert_eq!(
                (d.year, d.season, d.day, d.second),
                (y as i32 + 1, season, 15, (DAY - 1) as u32)
            );
        }
    }
    // Settling: years zero and below.
    assert_eq!(GameTime(start.0 - 1).date(start).year, 0);
    assert_eq!(GameTime(start.0 - 1).date(start).season, Season::Winter);
    assert_eq!(GameTime(0).date(start).year, 1 - k as i32);
    let d = Date {
        year: 112,
        season: Season::Autumn,
        day: 6,
        second: 0,
    };
    assert_eq!(d.to_string(), "Year 112, autumn, day 6");
}

// checks: TIM-14
#[test]
fn barrier_arithmetic() {
    assert_eq!(GameTime(0).next_barrier(), GameTime(300));
    assert_eq!(GameTime(299).next_barrier(), GameTime(300));
    assert_eq!(GameTime(300).next_barrier(), GameTime(600));
    let t = (1u64 << 40) - 1;
    let b = GameTime(t).next_barrier();
    assert!(b.0 > t && b.0.is_multiple_of(WINDOW) && b.0 - t <= WINDOW);
    assert_eq!(GameTime(t).window(), (t / 300) as u32);
    assert_eq!(GameTime(YEAR).window(), 17_280);
}

// checks: TIM-18
#[test]
fn game_length_rows() {
    assert_eq!(game_length(14 * DAY, None), Ok(14 * DAY));
    assert_eq!(game_length(14 * DAY, Some(14 * DAY)), Ok(14 * DAY));
    assert_eq!(game_length(15 * DAY, None), Err(LengthError::MissingGameLength));
    assert_eq!(game_length(84 * DAY, Some(10 * DAY)), Ok(10 * DAY));
    assert_eq!(game_length(85 * DAY, None), Ok(1_207_233));
    assert_eq!(game_length(365 * DAY, None), Ok(5_184_000));
    assert_eq!(
        game_length(365 * DAY, Some(YEAR + 1)),
        Err(LengthError::GameLengthMismatch { expected_s: YEAR })
    );
    assert_eq!(
        game_length(3 * DAY, Some(2 * DAY)),
        Err(LengthError::GameLengthMismatch { expected_s: 3 * DAY })
    );
    assert_eq!(
        game_length(40 * DAY, Some(6 * DAY)),
        Err(LengthError::GameLengthOutOfRange)
    );
    assert_eq!(
        game_length(40 * DAY, Some(41 * DAY)),
        Err(LengthError::GameLengthOutOfRange)
    );
}

// checks: TIM-18 TIM-14
#[test]
fn fourteen_years_is_fourteen_game_years() {
    let start = GameTime(10 * YEAR);
    let born = start;
    let adult = game_length(14 * 365 * DAY, None).unwrap();
    assert_eq!(adult, 14 * YEAR);
    assert_eq!(born.date(start).year, 1);
    assert_eq!(GameTime(born.0 + adult).date(start).year, 15);
}

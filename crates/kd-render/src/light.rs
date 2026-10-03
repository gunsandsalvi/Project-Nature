//! The air model (A11.4): the sun's light and the sky's light from the sun's height through the air, per channel at
//! the air's three wavelengths; twilight; night with the moon and starlight; exposure that follows the light in
//! part; rod vision's loss of colour; Narkowicz's filmic curve; the vivid grade; sRGB; and the haze's colour by the
//! angle to the sun. Every function goes through `kd_core::m`, so the palette has the same bits on the phone, in the
//! browser and in the cloud. Light is linear RGB with the sun's white above the air as 1.
//!
//! Implements PRE-30, see A11.4: the sun's and the sky's colours come from the sun's height through the air.

use kd_core::m;
use kd_data::Air;
use kd_view::SkyView;

pub type Rgb = [f32; 3];

/// Luminance weights of linear sRGB.
pub const LUM: Rgb = [0.2126, 0.7152, 0.0722];
const DEG: f32 = std::f32::consts::PI / 180.0;

pub fn dot(a: Rgb, b: Rgb) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

fn add(a: Rgb, b: Rgb) -> Rgb {
    [a[0] + b[0], a[1] + b[1], a[2] + b[2]]
}

fn mul(a: Rgb, b: Rgb) -> Rgb {
    [a[0] * b[0], a[1] * b[1], a[2] * b[2]]
}

fn scale(a: Rgb, k: f32) -> Rgb {
    [a[0] * k, a[1] * k, a[2] * k]
}

/// Kasten and Young's relative air mass at a sun height in degrees: 1 overhead, 37.9 at the horizon.
pub fn air_mass(h_deg: f32) -> f32 {
    let h = h_deg.max(0.0);
    1.0 / (m::sin(h * DEG) + 0.50572 * m::powf(h + 6.07995, -1.6364))
}

/// The air's optical depths straight up, per channel.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Depths {
    pub rayleigh: Rgb,
    /// The aerosol's: turbidity × wavelength in µm to the minus Ångström's exponent.
    pub aerosol: Rgb,
    pub ozone: Rgb,
}

pub fn depths(air: &Air, turbidity: f32) -> Depths {
    Depths {
        rayleigh: air.rayleigh,
        aerosol: air
            .wavelengths_nm
            .map(|nm| turbidity * m::powf(nm / 1000.0, -air.aerosol_exponent)),
        ozone: air.ozone,
    }
}

/// By day: the sun's light facing it, and the shares of the beam Rayleigh scattering and the aerosol take, at a sun
/// height of `h_deg` ≥ 0.
fn beam(d: &Depths, h_deg: f32) -> (Rgb, Rgb, Rgb) {
    let mass = air_mass(h_deg);
    let total = add(add(d.rayleigh, d.aerosol), d.ozone);
    let sun = total.map(|t| m::exp(-mass * t));
    let took_r = d.rayleigh.map(|t| 1.0 - m::exp(-mass * t));
    let took_a = d.aerosol.map(|t| 1.0 - m::exp(-mass * t));
    (sun, took_r, took_a)
}

/// How much sky light reaches level ground at a sun height, relative to the sky's scattering: `sin h` by day, as
/// A11.4 has it, plus `horizon_sky` fading over `horizon_fade_deg` above the horizon, so the sky still lights the
/// ground as the sun sets and twilight starts where the day ends; below the horizon it falls by e every
/// `twilight_fall_deg`.
pub fn sky_level(air: &Air, h_deg: f32) -> f32 {
    if h_deg >= 0.0 {
        m::sin(h_deg * DEG) + air.horizon_sky * m::exp(-h_deg / air.horizon_fade_deg)
    } else {
        air.horizon_sky * m::exp(h_deg / air.twilight_fall_deg)
    }
}

/// The sky's light on level ground at a sun height: by day the scattered shares of the beam; in twilight the
/// horizon's sky, dimming and turning blue as its light crosses more ozone, `twilight_ozone_per_deg` air masses for
/// each degree below the horizon.
pub fn sky_light(air: &Air, d: &Depths, h_deg: f32) -> Rgb {
    let (_, took_r, took_a) = beam(d, h_deg.max(0.0));
    let scattered = add(
        scale(took_r, air.sky_rayleigh_share),
        scale(took_a, air.sky_aerosol_share),
    );
    let level = sky_level(air, h_deg);
    if h_deg >= 0.0 {
        scale(scattered, level)
    } else {
        let ozone_path = -h_deg * air.twilight_ozone_per_deg;
        let blue = d.ozone.map(|t| m::exp(-ozone_path * t));
        scale(mul(scattered, blue), level)
    }
}

/// One frame's light (A11.4).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Light {
    /// The light's direction: the sun's while it is up, else the moon's (east, north, up).
    pub dir: [f32; 3],
    /// The sun's (or the moon's) light facing it.
    pub sun: Rgb,
    /// The sky's light on level ground, with the moon's sky and starlight at night.
    pub sky: Rgb,
    /// The light on level ground in luminance: `LUM · sky + LUM · sun × max(dir.up, 0)`.
    pub global: f32,
    /// The scale exposure applies: `exposure_scale / global ^ exposure_power`.
    pub exposure: f32,
    /// How far rod vision has taken colour, 0 to 1.
    pub rods: f32,
    /// The sun's height in degrees, and the shares of its beam the air scatters (for the haze's colour).
    pub sun_height_deg: f32,
    pub took_r: Rgb,
    pub took_a: Rgb,
}

/// The light on level ground with the sun overhead, in luminance: noon's light, which rod vision is measured from.
pub fn noon_global(air: &Air) -> f32 {
    let d = depths(air, air.turbidity);
    let (sun, _, _) = beam(&d, 90.0);
    dot(LUM, sky_light(air, &d, 90.0)) + dot(LUM, sun)
}

/// The light for a sky (A11.4). Until the moon comes (`MIL-04`), night has a half moon standing opposite the sun.
pub fn light(air: &Air, sky: &SkyView) -> Light {
    let turbidity = if sky.turbidity > 0.0 {
        sky.turbidity
    } else {
        air.turbidity
    };
    let d = depths(air, turbidity);
    let h_sun = m::asin(sky.sun_dir[2].clamp(-1.0, 1.0)) / DEG;
    let (beam_sun, took_r, took_a) = beam(&d, h_sun.max(0.0));
    let mut sky_rgb = sky_light(air, &d, h_sun);
    let (mut dir, mut sun) = if h_sun >= 0.0 {
        (sky.sun_dir, beam_sun)
    } else {
        (sky.sun_dir, [0.0; 3])
    };
    // The moon: a second sun of its own colour, weakened; its sky; starlight always.
    let moon_dir = [-sky.sun_dir[0], -sky.sun_dir[1], -sky.sun_dir[2]];
    let h_moon = m::asin(moon_dir[2].clamp(-1.0, 1.0)) / DEG;
    if h_moon > 0.0 {
        let k = 0.5 / air.moon_weakness;
        let (moon_beam, _, _) = beam(&d, h_moon);
        sky_rgb = add(sky_rgb, scale(mul(sky_light(air, &d, h_moon), air.moon_tint), k));
        if h_sun < 0.0 {
            dir = moon_dir;
            sun = scale(mul(moon_beam, air.moon_tint), k);
        }
    }
    sky_rgb = add(sky_rgb, scale(air.moon_tint, air.starlight));
    let global = dot(LUM, sky_rgb) + dot(LUM, sun) * dir[2].max(0.0);
    let exposure = air.exposure_scale / m::powf(global, air.exposure_power);
    let noon = noon_global(air);
    let rods = (m::ln(air.rods_start * noon / global) / m::ln(air.rods_start / air.rods_full)).clamp(0.0, 1.0);
    Light {
        dir,
        sun,
        sky: sky_rgb,
        global,
        exposure,
        rods,
        sun_height_deg: h_sun,
        took_r,
        took_a,
    }
}

/// The sky for the app's hour at a place, from the sky's sun (A3.7, A11.4): the moon's own place waits for
/// `MIL-04`, the turbidity for the weather.
pub fn sky_view(state: &kd_core::sky::SkyState) -> SkyView {
    SkyView {
        sun_dir: state.sun_dir,
        moon_dir: state.moon_dir,
        moon_phase: state.moon_phase,
        turbidity: 0.0,
    }
}

/// Narkowicz's fit of the ACES filmic curve, per channel.
pub fn aces(x: f32) -> f32 {
    ((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14)).clamp(0.0, 1.0)
}

/// Linear sRGB to OKLab.
pub fn to_oklab(c: Rgb) -> [f32; 3] {
    let l = m::cbrt(0.412_221_46 * c[0] + 0.536_332_55 * c[1] + 0.051_445_995 * c[2]);
    let mm = m::cbrt(0.211_903_5 * c[0] + 0.680_699_5 * c[1] + 0.107_396_96 * c[2]);
    let s = m::cbrt(0.088_302_46 * c[0] + 0.281_718_85 * c[1] + 0.629_978_7 * c[2]);
    [
        0.210_454_26 * l + 0.793_617_8 * mm - 0.004_072_047 * s,
        1.977_998_5 * l - 2.428_592_2 * mm + 0.450_593_7 * s,
        0.025_904_037 * l + 0.782_771_77 * mm - 0.808_675_77 * s,
    ]
}

/// OKLab to linear sRGB.
pub fn from_oklab(lab: [f32; 3]) -> Rgb {
    let [ll, a, b] = lab;
    let l = ll + 0.396_337_78 * a + 0.215_803_76 * b;
    let mm = ll - 0.105_561_346 * a - 0.063_854_17 * b;
    let s = ll - 0.089_484_18 * a - 1.291_485_5 * b;
    let (l, mm, s) = (l * l * l, mm * mm * mm, s * s * s);
    [
        4.076_741_7 * l - 3.307_711_6 * mm + 0.230_969_94 * s,
        -1.268_438 * l + 2.609_757_4 * mm - 0.341_319_38 * s,
        -0.004_196_086_3 * l - 0.703_418_6 * mm + 1.707_614_7 * s,
    ]
}

fn in_gamut(c: Rgb) -> bool {
    c.iter().all(|&v| (-1e-5..=1.0 + 1e-5).contains(&v))
}

/// The vivid grade (A11.4): OKLab chroma times `factor` at unchanged lightness; a colour pushed outside sRGB keeps
/// its hue and lightness and loses chroma, found by halving, until it fits.
pub fn vivid(c: Rgb, factor: f32) -> Rgb {
    let [l, a, b] = to_oklab(c);
    let at = |k: f32| from_oklab([l, a * k, b * k]);
    if in_gamut(at(factor)) {
        return at(factor).map(|v| v.clamp(0.0, 1.0));
    }
    let (mut lo, mut hi) = (0.0f32, factor);
    for _ in 0..24 {
        let mid = (lo + hi) / 2.0;
        if in_gamut(at(mid)) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    at(lo).map(|v| v.clamp(0.0, 1.0))
}

/// Rod vision: colour fades toward a blue-grey of the same lightness by `rods`, 0 to 1.
pub fn rod_vision(c: Rgb, rods: f32) -> Rgb {
    if rods <= 0.0 {
        return c;
    }
    let [l, a, b] = to_oklab(c);
    let keep = 1.0 - rods;
    from_oklab([l, a * keep - 0.004 * rods * l, b * keep - 0.02 * rods * l]).map(|v| v.clamp(0.0, 1.0))
}

/// Linear light to an 8-bit sRGB colour.
pub fn srgb8(c: Rgb) -> [u8; 3] {
    c.map(|v| {
        let v = v.clamp(0.0, 1.0);
        let s = if v <= 0.003_130_8 {
            12.92 * v
        } else {
            1.055 * m::powf(v, 1.0 / 2.4) - 0.055
        };
        (s * 255.0 + 0.5) as u8
    })
}

/// A surface's displayed colour: its albedo under some light, exposed, tone-mapped, faded by rod vision, graded and
/// put in 8-bit sRGB (A11.3, A11.4).
pub fn shade(air: &Air, light: &Light, albedo: Rgb, received: Rgb) -> [u8; 3] {
    let exposed = scale(mul(albedo, received), light.exposure);
    let toned = exposed.map(aces);
    srgb8(vivid(rod_vision(toned, light.rods), air.grade_chroma))
}

/// The haze's colour seen along `view` (east, north, up, from the eye into the scene): the sky's light, plus the
/// sun's scattered toward the eye by Rayleigh's phase and, for the aerosol, Henyey and Greenstein's (A11.4).
pub fn haze_colour(air: &Air, light: &Light, view: [f32; 3]) -> Rgb {
    let c = view[0] * light.dir[0] + view[1] * light.dir[1] + view[2] * light.dir[2];
    let rayleigh = 0.75 * (1.0 + c * c);
    let g = air.haze_g;
    let hg = (1.0 - g * g) / m::powf(1.0 + g * g - 2.0 * g * c, 1.5);
    let scattered = add(
        scale(light.took_r, air.sky_rayleigh_share * rayleigh),
        scale(light.took_a, air.sky_aerosol_share * hg),
    );
    add(
        light.sky,
        mul(light.sun, scale(scattered, light.dir[2].max(0.0).max(0.05))),
    )
}

#[cfg(test)]
pub(crate) mod tests {
    use super::*;

    pub(crate) fn air() -> Air {
        crate::tests::catalogue().air
    }

    /// The sky at 21° N on the spring equinox at an hour, as the app sets it.
    pub(crate) fn sky_at(hour: f32) -> SkyView {
        let t = kd_core::time::GameTime((hour * 3600.0) as u64);
        sky_view(&kd_core::sky::sun_moon(t, 21.0, 0.0, &kd_core::sky::Sky::FIRST))
    }

    fn sky_at_height(h_deg: f32) -> SkyView {
        let h = h_deg * DEG;
        SkyView {
            sun_dir: [0.0, -m::cos(h), m::sin(h)],
            moon_dir: [0.0; 3],
            moon_phase: 0.25,
            turbidity: 0.0,
        }
    }

    // checks: PRE-30
    #[test]
    fn sun_colour_follows_the_air() {
        let air = air();
        let d = depths(&air, air.turbidity);
        // The formula itself at 60°, 15° and 3°.
        for h in [60.0, 15.0, 3.0] {
            let l = light(&air, &sky_at_height(h));
            let mass = air_mass(h);
            for c in 0..3 {
                let want = m::exp(-mass * (d.rayleigh[c] + d.aerosol[c] + d.ozone[c]));
                assert!((l.sun[c] - want).abs() < 1e-6, "{h}° channel {c}");
            }
        }
        // Near white high up, golden at 15°, red near the horizon.
        let ratio = |h: f32| {
            let s = light(&air, &sky_at_height(h)).sun;
            s[2] / s[0]
        };
        assert!(ratio(60.0) > 0.7, "{}", ratio(60.0));
        assert!((0.3..0.7).contains(&ratio(15.0)), "{}", ratio(15.0));
        assert!(ratio(3.0) < 0.2, "{}", ratio(3.0));
        assert!((air_mass(90.0) - 1.0).abs() < 1e-3 && (air_mass(0.0) - 37.9).abs() < 0.1);
    }

    // checks: PRE-30
    #[test]
    fn sky_is_blue_by_day() {
        let air = air();
        let noon = light(&air, &sky_at(12.0));
        assert!(noon.sky[2] > noon.sky[1] && noon.sky[1] > noon.sky[0], "{:?}", noon.sky);
        // The sky is a fill: dimmer than the sun facing it at noon, but not by a hundred.
        let (y_sky, y_sun) = (dot(LUM, noon.sky), dot(LUM, noon.sun));
        assert!(y_sky < y_sun && y_sky > y_sun / 20.0, "sky {y_sky}, sun {y_sun}");
    }

    // checks: PRE-30
    #[test]
    fn twilight_has_no_jumps() {
        // From 3° above the horizon to 14° below, past both of twilight's borders, no exposed light moves by more
        // than 0.05 between neighbouring hundredths of a degree.
        let air = air();
        let mut last: Option<[f32; 6]> = None;
        let mut h = 3.0f32;
        while h > -14.0 {
            let l = light(&air, &sky_at_height(h));
            let now = [0, 1, 2, 3, 4, 5].map(|i| {
                if i < 3 {
                    l.sky[i] * l.exposure
                } else {
                    l.sun[i - 3] * l.exposure * l.dir[2].max(0.0)
                }
            });
            if let Some(prev) = last {
                for i in 0..6 {
                    assert!((now[i] - prev[i]).abs() < 0.05, "a jump at {h}°: {prev:?} to {now:?}");
                }
            }
            last = Some(now);
            h -= 0.01;
        }
        // Twilight turns blue.
        let t = light(&air, &sky_at_height(-6.0));
        assert!(t.sky[2] > t.sky[1] && t.sky[2] > t.sky[0], "{:?}", t.sky);
    }

    // checks: PRE-30
    #[test]
    fn night_darker_than_dusk_darker_than_day() {
        // A mid-grey under the sky and the sun on level ground, as displayed: day above dusk above night.
        let air = air();
        let shown = |hour: f32| {
            let l = light(&air, &sky_at(hour));
            let received = add(l.sky, scale(l.sun, l.dir[2].max(0.0)));
            let c = shade(&air, &l, [0.2, 0.2, 0.2], received);
            u32::from(c[0]) + u32::from(c[1]) + u32::from(c[2])
        };
        let (day, dusk, night) = (shown(12.0), shown(17.75), shown(23.0));
        assert!(
            day > dusk && dusk > night && night > 0,
            "day {day}, dusk {dusk}, night {night}"
        );
        // Night is dim and loses colour to rod vision; noon keeps all of it.
        assert!(light(&air, &sky_at(23.0)).rods > 0.3 && light(&air, &sky_at(12.0)).rods == 0.0);
    }

    // checks: PRE-30
    #[test]
    fn haze_warmer_toward_the_sun() {
        let air = air();
        let l = light(&air, &sky_at_height(8.0));
        let toward = haze_colour(&air, &l, [0.0, -0.99, 0.14]);
        let away = haze_colour(&air, &l, [0.0, 0.99, 0.14]);
        assert!(
            toward[0] / toward[2] > away[0] / away[2],
            "toward {toward:?}, away {away:?}"
        );
        assert!(dot(LUM, toward) > dot(LUM, away));
    }

    // checks: PRE-30
    #[test]
    fn grade_keeps_lightness() {
        for c in [
            [0.2, 0.4, 0.1],
            [0.8, 0.3, 0.05],
            [0.05, 0.06, 0.3],
            [0.5, 0.5, 0.5],
            [0.95, 0.9, 0.02],
        ] {
            let before = to_oklab(c)[0];
            let after = to_oklab(vivid(c, 1.2));
            assert!((after[0] - before).abs() < 2e-3, "{c:?}: {before} to {}", after[0]);
            let chroma = |lab: [f32; 3]| (lab[1] * lab[1] + lab[2] * lab[2]).sqrt();
            assert!(chroma(after) >= chroma(to_oklab(c)) - 1e-4, "{c:?} lost chroma");
        }
        // A grey stays grey, and OKLab round trips.
        let g = vivid([0.3, 0.3, 0.3], 1.2);
        assert!((g[0] - g[2]).abs() < 1e-3);
        let back = from_oklab(to_oklab([0.2, 0.5, 0.7]));
        assert!((back[0] - 0.2).abs() < 1e-4 && (back[2] - 0.7).abs() < 1e-4);
    }
}

//! The compiled catalogue's entries (A3.6): what the game reads, in each kind's number order (`data/ids.lock`).
//! The entries as written in `data/` (with `stage` and `checks`) are the compiler's, behind feature `compile`.

use serde::{Deserialize, Serialize};

/// `data/VERSION.toml`: the rules version (major, minor) and the generator version (`PLT-09`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Serialize, Deserialize)]
pub struct Versions {
    pub major: u16,
    pub minor: u16,
    pub generator: u16,
}

/// A fixed colour of the palette (A11.3, `data/palette/colours.md`): `void`, `ink` and the UI's colours, in number
/// order after `void`, which is number 0 and palette index 0.
#[derive(Clone, Debug, PartialEq, Eq, Serialize, Deserialize)]
pub struct Colour {
    pub id: String,
    pub name: String,
    /// Its permanent number in `data/ids.lock`.
    pub number: u16,
    pub rgb: [u8; 3],
}

/// A material's look (A11.3, `PRE-20`, `data/palette/looks.md`): its colour under white light, how many steps its
/// ladder has, and where it has them a sheen and a glow.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Look {
    pub id: String,
    pub name: String,
    /// Its permanent number in `data/ids.lock`; the palette packs looks in number order (A11.3).
    pub number: u16,
    /// The material's colour under white light, as sRGB; `albedo` takes it to linear.
    pub rgb: [u8; 3],
    /// 4 to 7 (`PRE-20`).
    pub steps: u8,
    /// 0 for none, up to 1 for wet or polished surfaces.
    pub sheen: f32,
    /// The colour of its glow, for flames.
    pub glow: Option<[u8; 3]>,
}

impl Look {
    /// The look's colour as linear albedo (A11.3).
    pub fn albedo(&self) -> [f32; 3] {
        self.rgb.map(|c| srgb_to_linear(f32::from(c) / 255.0))
    }
}

/// The sRGB transfer curve, inverted: an sRGB value in 0–1 to linear light.
pub fn srgb_to_linear(v: f32) -> f32 {
    if v <= 0.04045 {
        v / 12.92
    } else {
        kd_core::m::powf((v + 0.055) / 1.055, 2.4)
    }
}

/// The air and the light model's numbers (A11.4, `data/palette/light.md`, the one entry `air`), each tuned there.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Air {
    pub id: String,
    pub name: String,
    /// The three channels' wavelengths, red, green and blue.
    pub wavelengths_nm: [f32; 3],
    /// Zenith optical depths of Rayleigh scattering per channel.
    pub rayleigh: [f32; 3],
    /// The aerosol's depth at 1 µm (Ångström's β): 0.04 clear, 0.12 hazy.
    pub turbidity: f32,
    /// Ångström's exponent: the aerosol's depth falls as wavelength to its minus.
    pub aerosol_exponent: f32,
    /// Zenith optical depths of ozone's Chappuis band per channel.
    pub ozone: [f32; 3],
    /// The shares of what Rayleigh scattering and the aerosol take from the beam that light the sky.
    pub sky_rayleigh_share: f32,
    pub sky_aerosol_share: f32,
    /// The sky's light on level ground at sunset, as a share of the sky's scattering, fading over
    /// `horizon_fade_deg` above the horizon, so twilight starts where the day ends.
    pub horizon_sky: f32,
    pub horizon_fade_deg: f32,
    /// Twilight runs from the horizon to this sun height, its sky light falling by e every `twilight_fall_deg` and
    /// turning blue through `twilight_ozone_per_deg` more air masses of ozone for each degree.
    pub twilight_end_deg: f32,
    pub twilight_fall_deg: f32,
    pub twilight_ozone_per_deg: f32,
    /// How many times weaker the moon's light is than the sun's, and its tint.
    pub moon_weakness: f32,
    pub moon_tint: [f32; 3],
    /// Starlight, as a share of noon's light on level ground.
    pub starlight: f32,
    /// Exposure scales the light by `exposure_scale` over the global light to the power `exposure_power`.
    pub exposure_scale: f32,
    pub exposure_power: f32,
    /// Below the first share of noon's light, colour fades toward rod vision's blue-grey, fully by the second.
    pub rods_start: f32,
    pub rods_full: f32,
    /// The vivid grade: OKLab chroma times this, at unchanged lightness.
    pub grade_chroma: f32,
    /// Haze `1 − exp(−depth)` thresholds of levels 1–3, and how far each level mixes toward the haze colour.
    pub haze_levels: [f32; 3],
    pub haze_mixes: [f32; 3],
    /// Henyey and Greenstein's asymmetry for the aerosol's scattering toward the sun.
    pub haze_g: f32,
    /// Scale heights of the aerosol and of the air.
    pub aerosol_height_m: f32,
    pub rayleigh_height_m: f32,
}

/// A ground surface (A11.5, `PRE-20`, `data/models/surfaces.md`): the looks it is drawn in, how a world-fixed noise
/// splits them, and whether it draws as rock.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Surface {
    pub id: String,
    pub name: String,
    /// Its permanent number in `data/ids.lock`; areas name their surfaces by it.
    pub number: u16,
    /// One to three looks, by number.
    pub looks: Vec<u16>,
    /// With more than one look, the octaves of the noise that splits them: their periods in metres, largest first,
    /// each half the last's height; empty with one look.
    pub split_m: Vec<f32>,
    /// Where each next look takes over, as values of that noise (scaled to ±1), rising.
    pub split_at: Vec<f32>,
    /// Drawn in the rock category, whose outlines are finer than the ground's (A11.2).
    pub rock: bool,
}

/// The whole compiled catalogue, as the blob holds it.
#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Catalogue {
    pub versions: Versions,
    pub colours: Vec<Colour>,
    pub looks: Vec<Look>,
    pub air: Air,
    pub surfaces: Vec<Surface>,
}

impl Catalogue {
    /// A fixed colour by id.
    pub fn colour(&self, id: &str) -> Option<&Colour> {
        self.colours.iter().find(|c| c.id == id)
    }

    /// A surface by id.
    pub fn surface(&self, id: &str) -> Option<&Surface> {
        self.surfaces.iter().find(|s| s.id == id)
    }
}

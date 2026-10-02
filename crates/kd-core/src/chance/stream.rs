//! A purpose's stream and its helpers (A3.3). Implements `TIM-16` in part.

use super::hash::{draw, stream_seed};
use super::registry::Purpose;
use crate::time::GameTime;

/// A draw's moment: `(game_second << 16) | slot`; the slot separates several draws by one subject for one purpose
/// in one second (A3.3).
#[inline]
pub fn moment(t: GameTime, slot: u16) -> u64 {
    (t.0 << 16) | slot as u64
}

/// The fortune retry's purpose number: bit 31 flipped (A3.3, `GOD-04` from α35b).
#[inline]
pub fn retry_number(number: u32) -> u32 {
    number ^ 0x8000_0000
}

/// One purpose's cached seed (A3.3).
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Stream {
    seed: u64,
}

impl Stream {
    /// The stream of purpose `p` in world `world`.
    pub fn new(world: u64, p: Purpose) -> Stream {
        Stream {
            seed: stream_seed(world, p.system, p.number),
        }
    }

    /// The fortune retry's stream: the same key with bit 31 of the purpose flipped (A3.3).
    pub fn retry(world: u64, p: Purpose) -> Stream {
        Stream {
            seed: stream_seed(world, p.system, retry_number(p.number)),
        }
    }

    /// The raw 64-bit draw for this subject at this moment.
    #[inline]
    pub fn draw(&self, subject: u64, moment: u64) -> u64 {
        draw(self.seed, subject, moment)
    }

    /// A 24-bit float in [0, 1), exact (A3.2).
    #[inline]
    pub fn unit(&self, subject: u64, moment: u64) -> f32 {
        (self.draw(subject, moment) >> 40) as f32 * (1.0 / 16_777_216.0)
    }

    /// True with probability `p`.
    #[inline]
    pub fn chance(&self, subject: u64, moment: u64, p: f32) -> bool {
        self.unit(subject, moment) < p
    }

    /// A whole number in `0..n` (`n` at least 1).
    #[inline]
    pub fn below(&self, subject: u64, moment: u64, n: u32) -> u32 {
        (((self.draw(subject, moment) >> 32) * n as u64) >> 32) as u32
    }

    /// A float in [lo, hi).
    #[inline]
    pub fn range(&self, subject: u64, moment: u64, lo: f32, hi: f32) -> f32 {
        lo + (hi - lo) * self.unit(subject, moment)
    }

    /// An index picked with the given weights, cumulative in index order; all zero gives 0.
    pub fn pick_weighted(&self, subject: u64, moment: u64, weights: &[f32]) -> usize {
        let mut total = 0.0f32;
        for &w in weights {
            total += w;
        }
        if total <= 0.0 {
            return 0;
        }
        let target = self.unit(subject, moment) * total;
        let mut cum = 0.0f32;
        let mut last = 0;
        for (i, &w) in weights.iter().enumerate() {
            if w > 0.0 {
                cum += w;
                last = i;
                if target < cum {
                    return i;
                }
            }
        }
        last
    }

    /// About normal, mean 0 and spread 1, within ±3.4641: the four 16-bit quarters of one draw as fractions of
    /// 65,536, summed, minus 2, times 1.732 (A3.3).
    pub fn normal(&self, subject: u64, moment: u64) -> f32 {
        let d = self.draw(subject, moment);
        let mut sum = 0.0f32;
        for q in 0..4 {
            sum += ((d >> (16 * q)) & 0xffff) as f32 * (1.0 / 65_536.0);
        }
        (sum - 2.0) * 1.732
    }
}

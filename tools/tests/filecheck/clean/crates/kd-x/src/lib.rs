//! kd-x: the self-test's clean crate (PRC-12).

/// Implements `ABC-01`, see A1.1.
pub fn one() -> u8 {
    1
}

#[cfg(test)]
mod tests {
    // checks: ABC-01
    #[test]
    fn one_is_one() {
        assert_eq!(super::one(), 1);
    }
}

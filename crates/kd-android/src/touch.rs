//! Android's touches as raw input events (A12.2): `MotionEvent`'s action names one pointer for downs and ups,
//! and every pointer for moves and cancels.

use kd_view::{InputEvent, InputKind};

/// `MotionEvent.ACTION_*`, as `Native.touch` passes them.
pub const DOWN: i32 = 0;
pub const UP: i32 = 1;
pub const MOVE: i32 = 2;
pub const CANCEL: i32 = 3;
pub const POINTER_DOWN: i32 = 5;
pub const POINTER_UP: i32 = 6;

/// The input events one `MotionEvent` makes: positions in screen pixels from the top-left corner.
pub fn events(action: i32, index: i32, ids: &[i32], xs: &[f32], ys: &[f32], t_ns: u64) -> Vec<InputEvent> {
    let n = ids.len().min(xs.len()).min(ys.len());
    let one = |kind: InputKind, i: usize| InputEvent {
        kind,
        pointer: ids[i],
        x: xs[i],
        y: ys[i],
        t_ns,
    };
    let at = usize::try_from(index).ok().filter(|&i| i < n);
    match action {
        DOWN | POINTER_DOWN => at.map(|i| vec![one(InputKind::Down, i)]).unwrap_or_default(),
        UP | POINTER_UP => at.map(|i| vec![one(InputKind::Up, i)]).unwrap_or_default(),
        MOVE => (0..n).map(|i| one(InputKind::Move, i)).collect(),
        CANCEL => (0..n).map(|i| one(InputKind::Cancel, i)).collect(),
        _ => Vec::new(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // checks: PRE-33
    #[test]
    fn two_fingers_in_order() {
        // One finger down, a second down, both move, the second lifts, the first lifts: each pointer keeps its id.
        let d = events(DOWN, 0, &[7], &[10.0], &[20.0], 1);
        assert_eq!(
            (d.len(), d[0].kind, d[0].pointer, d[0].x, d[0].y),
            (1, InputKind::Down, 7, 10.0, 20.0)
        );
        let pd = events(POINTER_DOWN, 1, &[7, 9], &[10.0, 50.0], &[20.0, 60.0], 2);
        assert_eq!((pd.len(), pd[0].kind, pd[0].pointer), (1, InputKind::Down, 9));
        let m = events(MOVE, 0, &[7, 9], &[11.0, 51.0], &[21.0, 61.0], 3);
        assert_eq!(
            m.iter().map(|e| (e.kind, e.pointer)).collect::<Vec<_>>(),
            vec![(InputKind::Move, 7), (InputKind::Move, 9)]
        );
        let pu = events(POINTER_UP, 1, &[7, 9], &[11.0, 51.0], &[21.0, 61.0], 4);
        assert_eq!((pu.len(), pu[0].kind, pu[0].pointer), (1, InputKind::Up, 9));
        let u = events(UP, 0, &[7], &[11.0], &[21.0], 5);
        assert_eq!((u[0].kind, u[0].pointer, u[0].t_ns), (InputKind::Up, 7, 5));
        // A cancel ends every pointer; an index outside the arrays or an unknown action makes nothing.
        assert_eq!(events(CANCEL, 0, &[7, 9], &[0.0, 0.0], &[0.0, 0.0], 6).len(), 2);
        assert!(events(DOWN, 3, &[7], &[0.0], &[0.0], 7).is_empty());
        assert!(events(9, 0, &[7], &[0.0], &[0.0], 8).is_empty());
    }
}

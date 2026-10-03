//! Each type and method the root `clippy.toml` bans, used once (A2.3 rule 6, A3.2); `tools/check-banned.sh` fails
//! unless clippy flags every one.

#![allow(dead_code, unused_must_use, clippy::let_underscore_future)]

use std::cell::{Cell, LazyCell, OnceCell, RefCell};
use std::collections::{HashMap, HashSet};
use std::fs::File;
use std::net::{TcpStream, UdpSocket};
use std::sync::atomic::{AtomicBool, AtomicI32, AtomicI64, AtomicU32, AtomicU64, AtomicUsize};
use std::sync::{LazyLock, Mutex, OnceLock, RwLock};
use std::time::{Instant, SystemTime};

pub struct Types {
    a: HashMap<u8, u8>,
    b: HashSet<u8>,
    c: Cell<u8>,
    d: RefCell<u8>,
    e: OnceCell<u8>,
    f: LazyCell<u8>,
    g: Mutex<u8>,
    h: RwLock<u8>,
    i: OnceLock<u8>,
    j: LazyLock<u8>,
    k: AtomicBool,
    l: AtomicU32,
    m: AtomicU64,
    n: AtomicUsize,
    o: AtomicI32,
    p: AtomicI64,
    q: Instant,
    r: SystemTime,
    s: File,
    t: TcpStream,
    u: UdpSocket,
}

pub fn methods(x: f32) {
    let _ = (x.sin(), x.cos(), x.tan(), x.sin_cos(), x.asin(), x.acos(), x.atan(), x.atan2(x));
    let _ = (x.sinh(), x.cosh(), x.tanh(), x.asinh(), x.acosh(), x.atanh());
    let _ = (x.exp(), x.exp2(), x.exp_m1(), x.ln(), x.ln_1p(), x.log(x), x.log2(), x.log10());
    let _ = (x.powf(x), x.powi(2), x.hypot(x), x.cbrt(), x.mul_add(x, x), x.min(x), x.max(x));
    let _ = std::thread::spawn(|| ());
    let _ = (std::env::var("A"), std::env::vars());
    let _ = (std::fs::read("a"), std::fs::read_to_string("a"), std::fs::write("a", "b"));
}

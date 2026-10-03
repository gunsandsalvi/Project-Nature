//! Each type and method the root `clippy.toml` bans, used once (A2.3 rule 6, A3.2); `tools/check-banned.sh` fails
//! unless clippy flags every one.

#![allow(dead_code, unused_must_use, clippy::let_underscore_future)]

use std::cell::{Cell, LazyCell, OnceCell, RefCell};
use std::collections::{HashMap, HashSet};
use std::fs::{File, OpenOptions};
use std::net::{TcpListener, TcpStream, UdpSocket};
use std::process::Command;
use std::sync::atomic::{
    AtomicBool, AtomicI8, AtomicI16, AtomicI32, AtomicI64, AtomicIsize, AtomicPtr, AtomicU8, AtomicU16, AtomicU32,
    AtomicU64, AtomicUsize,
};
use std::sync::mpsc::{Receiver, Sender, SyncSender};
use std::sync::{Barrier, Condvar, LazyLock, Mutex, OnceLock, RwLock};
use std::thread::Builder;
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
    v: (AtomicI8, AtomicI16, AtomicIsize, AtomicU8, AtomicU16, AtomicPtr<u8>),
    w: (Condvar, Barrier, Sender<u8>, SyncSender<u8>, Receiver<u8>, Builder),
    x: (TcpListener, OpenOptions, Command),
}

pub fn methods(x: f32) {
    let _ = (x.sin(), x.cos(), x.tan(), x.sin_cos(), x.asin(), x.acos(), x.atan(), x.atan2(x));
    let _ = (x.sinh(), x.cosh(), x.tanh(), x.asinh(), x.acosh(), x.atanh());
    let _ = (x.exp(), x.exp2(), x.exp_m1(), x.ln(), x.ln_1p(), x.log(x), x.log2(), x.log10());
    let _ = (x.powf(x), x.powi(2), x.hypot(x), x.cbrt(), x.mul_add(x, x), x.min(x), x.max(x));
    let _ = std::thread::spawn(|| ());
    std::thread::scope(|_| ());
    std::thread::sleep(std::time::Duration::ZERO);
    let _ = (std::sync::mpsc::channel::<u8>(), std::sync::mpsc::sync_channel::<u8>(1));
    let _ = (std::env::var("A"), std::env::vars(), std::env::var_os("A"), std::env::vars_os());
    let _ = (std::env::args(), std::env::args_os(), std::env::current_dir(), std::env::temp_dir());
    let _ = std::io::stdin();
    let _ = (std::fs::read("a"), std::fs::read_to_string("a"), std::fs::write("a", "b"));
    let _ = (std::fs::read_dir("a"), std::fs::remove_file("a"), std::fs::remove_dir("a"));
    let _ = (std::fs::remove_dir_all("a"), std::fs::create_dir("a"), std::fs::create_dir_all("a"));
    let _ = (std::fs::rename("a", "b"), std::fs::copy("a", "b"), std::fs::metadata("a"));
    std::process::exit(0);
}

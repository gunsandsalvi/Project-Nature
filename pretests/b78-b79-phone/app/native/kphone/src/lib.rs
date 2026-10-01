//! B79: phone helpers for the round-1 app (class dev.kindling.pretests.Native).
//! - Memory steps for the PLT-01 budget: map and touch native memory in steps, keep it, free it.
//! - The CPU the calling thread runs on, to confirm pinning.
use std::sync::Mutex;

struct Block(*mut libc::c_void, usize);
unsafe impl Send for Block {}
static BLOCKS: Mutex<Vec<Block>> = Mutex::new(Vec::new());

/// Maps `mib` MiB of private anonymous memory and writes to every 4 KiB, so the pages
/// are really resident. Returns false if the mapping fails. The memory is kept until `free_all`.
pub fn alloc_touch(mib: usize) -> bool {
    let len = mib << 20;
    // SAFETY: plain anonymous mapping; we only write inside [ptr, ptr+len).
    unsafe {
        let p = libc::mmap(std::ptr::null_mut(), len, libc::PROT_READ | libc::PROT_WRITE,
                           libc::MAP_PRIVATE | libc::MAP_ANONYMOUS, -1, 0);
        if p == libc::MAP_FAILED {
            return false;
        }
        let bytes = p as *mut u8;
        let mut off = 0;
        while off < len {
            std::ptr::write_volatile(bytes.add(off), (off >> 12) as u8 | 1);
            off += 4096;
        }
        BLOCKS.lock().unwrap_or_else(|e| e.into_inner()).push(Block(p, len));
    }
    true
}

/// Frees everything `alloc_touch` kept; returns the MiB freed.
pub fn free_all() -> usize {
    let mut blocks = BLOCKS.lock().unwrap_or_else(|e| e.into_inner());
    let mut total = 0;
    for b in blocks.drain(..) {
        // SAFETY: each block came from mmap with this length.
        unsafe { libc::munmap(b.0, b.1) };
        total += b.1 >> 20;
    }
    total
}

pub fn current_cpu() -> i32 {
    // SAFETY: no arguments, no memory.
    unsafe { libc::sched_getcpu() }
}

#[cfg(feature = "android")]
mod android {
    use jni::objects::JClass;
    use jni::sys::{jboolean, jint, JNI_FALSE, JNI_TRUE};
    use jni::JNIEnv;

    #[no_mangle]
    pub extern "system" fn Java_dev_kindling_pretests_Native_memAllocMiB(_e: JNIEnv, _c: JClass, mib: jint) -> jboolean {
        if mib > 0 && super::alloc_touch(mib as usize) { JNI_TRUE } else { JNI_FALSE }
    }
    #[no_mangle]
    pub extern "system" fn Java_dev_kindling_pretests_Native_memFreeAll(_e: JNIEnv, _c: JClass) -> jint {
        super::free_all() as jint
    }
    #[no_mangle]
    pub extern "system" fn Java_dev_kindling_pretests_Native_currentCpu(_e: JNIEnv, _c: JClass) -> jint {
        super::current_cpu()
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn alloc_touch_and_free() {
        assert!(super::alloc_touch(8));
        assert!(super::alloc_touch(4));
        assert_eq!(super::free_all(), 12);
        assert_eq!(super::free_all(), 0);
        assert!(super::current_cpu() >= 0);
    }
}

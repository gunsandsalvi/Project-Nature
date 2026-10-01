// B78 shell 3: pure native app. One screen (filled with a colour made from the
// core's answer) and one native call. No Kotlin; showing text would need a UI library.
use android_activity::{AndroidApp, MainEvent, PollEvent};
use ndk::hardware_buffer_format::HardwareBufferFormat;
use ndk::native_window::NativeWindow;

#[no_mangle]
fn android_main(app: AndroidApp) {
    let answer = kcore::mix(42, 1_000_000);
    let colour = 0xFF00_0000u32 | (answer as u32 & 0x00FF_FFFF);
    let mut quit = false;
    while !quit {
        app.poll_events(None, |event| match event {
            PollEvent::Main(MainEvent::InitWindow { .. }) | PollEvent::Main(MainEvent::RedrawNeeded { .. }) => {
                if let Some(window) = app.native_window() {
                    fill(&window, colour);
                }
            }
            PollEvent::Main(MainEvent::Destroy) => quit = true,
            _ => {}
        });
    }
}

fn fill(window: &NativeWindow, colour: u32) {
    let _ = window.set_buffers_geometry(0, 0, Some(HardwareBufferFormat::R8G8B8A8_UNORM));
    if let Ok(mut buffer) = window.lock(None) {
        let (w, h, stride) = (buffer.width(), buffer.height(), buffer.stride());
        let pixels = buffer.bits() as *mut u32;
        for y in 0..h {
            for x in 0..w {
                // SAFETY: the locked buffer holds `stride * h` 32-bit pixels.
                unsafe { *pixels.add(y * stride + x) = colour };
            }
        }
    } // unlocked and shown on drop
}

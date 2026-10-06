## What the phone says of itself through Android, for the benchmark (A3.9, PLT-04): its battery's
## charge, whether it is plugged in, and the current it draws. Godot reaches Android's own classes
## through its AndroidRuntime singleton; off Android, and wherever a reading fails, nothing is
## known. Implements PLT-04.
class_name Phone
extends RefCounted

## BatteryManager's properties (android.os.BatteryManager): the current drawn now, in microamperes,
## and the charge, in percent.
const CURRENT_NOW := 2
const CAPACITY := 4


## The battery: {"percent", "current_ma" (drawn, so positive while discharging), "plugged"}, or
## nothing where Android cannot be asked.
static func battery() -> Dictionary:
	var manager: Object = _service("batterymanager")
	if manager == null:
		return {}
	var out := {}
	var percent: Variant = manager.call("getIntProperty", CAPACITY)
	if percent is int and percent >= 0 and percent <= 100:
		out["percent"] = percent
	var current: Variant = manager.call("getIntProperty", CURRENT_NOW)
	if current is int and current != 0 and absi(current) < 20_000_000:
		# phones differ in the sign they give; the current the benchmark wants is what is drawn
		out["current_ma"] = absf(current / 1000.0)
	var charging: Variant = manager.call("isCharging")
	if charging is bool:
		out["plugged"] = charging
	return out


## The Android version, such as 16, or 0 off Android.
static func android_version() -> int:
	return OS.get_version().get_slice(".", 0).to_int() if OS.get_name() == "Android" else 0


static func _service(name: String) -> Object:
	if OS.get_name() != "Android" or not Engine.has_singleton("AndroidRuntime"):
		return null
	var activity: Object = Engine.get_singleton("AndroidRuntime").getActivity()
	if activity == null:
		return null
	return activity.call("getSystemService", name)

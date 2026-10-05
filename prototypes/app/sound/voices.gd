## P14's voice manager (IMPLEMENTATION α0.7c, SND-01, SND-07, A16): keeps the 32 sounds the phone
## plays at once in SND-01's shares: up to 6 for place and weather, 4 for single voices, 8 for
## music, 2 kept free for a sudden sound, and the rest, at least 12, for work, fire, footsteps and
## animals. When a share is full its quietest sound joins its blend, a hum as loud as all it holds,
## until each would have ended; a hum is one of its share's sounds. Talk and work have hums; in the
## other shares the quietest is let go. Pre-production code (research 00).
extends RefCounted

enum Share { PLACE, VOICES, MUSIC, FREE, WORK }

const CAP := 32
## Each share's most at once; work has what the others and the free ones leave.
const MOST := {Share.PLACE: 6, Share.VOICES: 4, Share.MUSIC: 8, Share.FREE: 2}
## The shares whose quietest sounds join a hum.
const BLENDED: Array[int] = [Share.VOICES, Share.WORK]
const NAMES := ["place", "voices", "music", "free", "work"]

## The sounds playing: id → [share, loudness where you are, when it ends (0: when stopped)].
var playing := {}
## Each blended share's hum: what it holds, as [loudness, when it ends].
var blends := {}
var _next := 1


func _init() -> void:
	for share: int in BLENDED:
		blends[share] = []


## Whether a share's hum sounds: it holds something.
func humming(share: int) -> bool:
	return share in blends and not (blends[share] as Array).is_empty()


## How loud a share's hum is: as loud as all it holds together.
func hum(share: int) -> float:
	var power := 0.0
	for entry: Array in blends.get(share, []):
		power += entry[0] * entry[0]
	return sqrt(power)


## The sounds a share plays now, its hum counted as one.
func used(share: int) -> int:
	var n := 1 if humming(share) else 0
	for id: int in playing:
		if playing[id][0] == share:
			n += 1
	return n


## All the sounds playing now, hums included.
func total() -> int:
	var n := 0
	for share: int in Share.values():
		n += used(share)
	return n


## The most a share may play now: its own most, or for work all the others and the free ones leave.
func most(share: int) -> int:
	if share != Share.WORK:
		return MOST[share]
	return CAP - MOST[Share.FREE] - used(Share.PLACE) - used(Share.VOICES) - used(Share.MUSIC)


## Asks to play a sound in a share, as loud as `loudness` where you are, for `seconds` (0: until it
## is stopped, as a loop, which never joins a blend). Gives the id to play it under, or 0 where it
## went straight to its blend, and the ids of the sounds to stop now, gone to their blend.
func ask(share: int, loudness: float, seconds: float, now: float) -> Dictionary:
	expire(now)
	var stop: Array[int] = []
	var ends := now + seconds if seconds > 0.0 else 0.0
	if not _make_room(share, loudness, ends, now, stop):
		return {"id": 0, "stop": stop}
	var id := _next
	_next += 1
	playing[id] = [share, loudness, ends]
	# work yields what the other shares now take within their own most; where work holds only
	# loops, which never yield, the new sound gives way instead
	if share != Share.WORK and used(Share.WORK) > most(Share.WORK):
		_make_room(Share.WORK, -1.0, 0.0, now, stop)
		if used(Share.WORK) > most(Share.WORK):
			playing.erase(id)
			return {"id": 0, "stop": stop}
	return {"id": id, "stop": stop}


## A sound that ended or was stopped.
func done(id: int) -> void:
	playing.erase(id)


## Lets go of what has ended: sounds past their end, and what each hum held past its end.
func expire(now: float) -> void:
	for id: int in playing.keys():
		var ends: float = playing[id][2]
		if ends > 0.0 and ends <= now:
			playing.erase(id)
	for share: int in blends:
		blends[share] = (blends[share] as Array).filter(func(e: Array) -> bool: return e[1] > now)


## Makes room in a share for a new sound as loud as `loudness` (below 0: no new sound, only bring
## the share within its most): its quietest sounds, the new one among them, join its hum, which
## takes a place of its own when it starts, or, in a share with no hum, are let go. Loops stay.
## Whether the new sound plays.
func _make_room(share: int, loudness: float, ends: float, now: float, stop: Array[int]) -> bool:
	var plays := loudness >= 0.0
	var free := most(share) - used(share)
	if free >= (1 if plays else 0):
		return plays
	var hum_place := 1 if share in BLENDED and not humming(share) else 0
	var quiet: Array = []
	for id: int in playing:
		if playing[id][0] == share and playing[id][2] > 0.0:
			quiet.append([playing[id][1], id, playing[id][2]])
	if plays:
		quiet.append([loudness, 0, ends])
	quiet.sort_custom(func(a: Array, b: Array) -> bool: return a[0] < b[0])
	var gone: Array = []
	for entry: Array in quiet:
		if hum_place + (1 if plays else 0) <= free:
			break
		if entry[1] == 0:
			plays = false
		else:
			playing.erase(entry[1])
			stop.append(entry[1])
			free += 1
		gone.append(entry)
	# a hum starts only where it has a place
	if share in BLENDED and hum_place <= free:
		for entry: Array in gone:
			(blends[share] as Array).append([entry[0], maxf(entry[2], now + 0.5)])
	return plays

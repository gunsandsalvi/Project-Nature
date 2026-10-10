## Finite waits consume display snapshots, rather than retaining days of unseen ways.
extends RefCounted


static func to(world: KdWorld, moment: int) -> void:
	while world.frontier() < moment:
		world.run_until(mini(moment, world.frontier() + 900))
		world.prepare_dream()

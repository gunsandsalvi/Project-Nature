## Plain cards from recorded decisions, with ordinary dates rather than simulation seconds.
extends RefCounted


static func when(second: int, now: int) -> String:
	var clock := "%02d:%02d" % [posmod(second, 86400) / 3600, posmod(second, 3600) / 60]
	if second / 86400 == now / 86400:
		return clock
	return "day %d at %s" % [second / 86400 + 1, clock]


static func person(p: Dictionary, details: bool, now: int) -> String:
	var goals := ["food", "water", "rest", "nearby supplies"]
	var choice := int(p.choice)
	var action := str(p.activity)
	if int(p.action_code) == 1:
		action += " to " + goals[choice]
	if int(p.get("dream_pull", 0)) > 0 and choice == 3:
		action = (
			("Watching " if int(p.action_code) == 0 else "Visiting ")
			+ ["food plants", "water", "shelter"][int(p.dream_decision_subject)]
		)
	var words := (
		"%s · %s · %d%% done\nNeeds met: food %d · water %d · rest %d\n"
		% [p.name, action, int(p.progress_ppm) / 10000, p.food_need, p.water_need, p.rest_need]
	)
	if choice < 3:
		words += (
			"Why: %s was %d/100. Expected +%d.\nAbout %d min for travel and work."
			% [
				goals[choice],
				p.decision_needs[choice],
				p.benefits[choice],
				maxi(1, ceili(float(p.cost_seconds[choice]) / 60))
			]
		)
	elif int(p.get("dream_pull", 0)) > 0:
		words += "Why: a remembered dream draws a visit.\nTheir needs can wait for this short walk."
	else:
		words += "Why: their known supplies weren't worth a trip yet.\nLooking around, then watching."
	if not details:
		return words
	words += "\n\nChose this at %s." % when(int(p.decision_at), now)
	if int(p.get("dream_pull", 0)) > 0:
		words += "\nThe dream made this feel a little more worthwhile."
	var rejected := [0, 1, 2, 3]
	rejected.erase(choice)
	rejected.sort_custom(func(a: int, b: int) -> bool: return int(p.scores[a]) > int(p.scores[b]))
	var actions := ["gather", "drink", "rest", "look around"]
	var chosen: String = [
		"Gathering and eating",
		"Drinking",
		"Resting",
		"Visiting the place" if int(p.get("dream_pull", 0)) > 0 else "Looking around"
	][choice]
	var exclusions := [
		"",
		"they haven't noticed a place for it",
		"they last saw it empty",
		"the path is blocked",
		"they haven't learned gathering",
		"they're waiting to try the path again"
	]
	for option: int in rejected.slice(0, 2):
		var reason := str(chosen) + " felt more worthwhile"
		if option < 3:
			reason = exclusions[int(p.unavailable[option])]
			if reason.is_empty():
				reason = (
					"%s was %d/100. Expected +%d; about %d min.\n%s felt more worthwhile"
					% [
						goals[option],
						p.decision_needs[option],
						p.benefits[option],
						maxi(1, ceili(float(p.cost_seconds[option]) / 60)),
						chosen
					]
				)
		words += "\n\nDidn't %s: %s." % [actions[option], reason]
	words += "\n\nRemembered supplies\nThey may have changed since then."
	for i in 3:
		if int(p.sources[i]) == 0:
			words += "\n\n%s: not yet noticed." % goals[i].capitalize()
		else:
			var amount := "shelter"
			if i == 0:
				amount = "%.2f kg" % (float(p.known_amounts[i]) / 1000000)
			elif i == 1:
				amount = "%.2f L" % (float(p.known_amounts[i]) / 1000)
			var source := "Seen" if int(p.sources[i]) == 1 else "Known from the start"
			if int(p.sources[i]) == 3:
				source = "Found while using it"
			words += (
				"\n\n%s: %s.\n%s at %s."
				% [goals[i].capitalize(), amount, source, when(int(p.seen_at[i]), now)]
			)
	if int(p.get("dream_at", -1)) >= 0:
		words += (
			"\n\nDreamt of %s at %s."
			% [
				["food plants", "water", "shelter"][int(p.dream_subject)],
				when(int(p.dream_at), now)
			]
		)
		words += (
			"\nThe pull has faded."
			if int(p.dream_until) <= now
			else "\nThe place still draws a visit."
		)
	words += (
		"\n\nAge at start: %d. Gathering skill: %d.\nCarrying %.2f kg berries."
		% [p.age_at_start, p.gathering_skill, float(p.carried_food_mg) / 1000000]
	)
	if int(p.memory_at) >= 0:
		var acts := {2: "Rested", 5: "Ate berries", 6: "Drank water"}
		words += (
			"\n\nLast remembered: %s at %s."
			% [acts.get(int(p.memory_kind), "Acted"), when(int(p.memory_at), now)]
		)
	return words

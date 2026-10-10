## Plain cards from recorded decisions, with ordinary dates rather than simulation seconds.
extends RefCounted


static func when(second: int, now: int) -> String:
	var clock := "%02d:%02d" % [posmod(second, 86400) / 3600, posmod(second, 3600) / 60]
	if second / 86400 == now / 86400:
		return clock
	return "day %d at %s" % [second / 86400 + 1, clock]


static func person(p: Dictionary, details: bool, now: int) -> String:
	if not details:
		return _summary(p)
	var goals := ["food", "water", "rest", "nearby supplies"]
	var choice := int(p.choice)
	var action := str(p.activity)
	if int(p.action_code) == 1:
		action += " to " + goals[choice]
	if int(p.get("dream_pull", 0)) > 0 and choice == 3 and int(p.get("dream_kind", 0)) == 0:
		action = (
			("Watching " if int(p.action_code) == 0 else "Visiting ")
			+ ["food plants", "water", "shelter"][int(p.dream_decision_subject)]
		)
	var words := (
		"%s · %s · %d%% done\nNeeds met: food %d · water %d · rest %d\n"
		% [p.name, action, int(p.progress_ppm) / 10000, p.food_need, p.water_need, p.rest_need]
	)
	if p.has("felt_milli_c"):
		words += (
			"Feels %.1f °C · warmth %d/100.\n" % [float(p.felt_milli_c) / 1000, int(p.warmth_need)]
		)
	if not p.get("reasons", []).is_empty():
		return words + kept_reasons(p.reasons)
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
		words += (
			"Why: an idea makes a familiar try feel worthwhile."
			if int(p.get("dream_kind", 0)) == 1
			else "Why: a remembered dream draws a visit.\nTheir needs can wait for this short walk."
		)
	else:
		words += "Why: their known supplies weren't worth a trip yet.\nLooking around, then watching."
	if int(p.get("work_state", 0)) != 0:
		var work := recipe(str(p.work_recipe)) if p.work_known else "Trying familiar materials"
		if p.get("work_taught", false):
			work = "Shared practice: " + recipe(str(p.work_recipe))
		words = (
			"%s · %s\nNeeds met: food %d · water %d · rest %d\n"
			% [p.name, work, p.food_need, p.water_need, p.rest_need]
		)
		words += (
			(
				"%d completed tries · next try at %s."
				% [int(p.work_tries), when(int(p.work_next_try), now)]
			)
			if int(p.work_state) == 2
			else str(p.activity)
		)
		if int(p.work_state) == 4:
			words += "\n%d seconds of gradual work kept." % int(p.work_progress)
		if details:
			words += "\n\nChose this at %s." % when(int(p.decision_at), now)
			words += (
				"\nWork began at %s. Reserved inputs: %d."
				% [when(int(p.work_start), now), p.work_inputs.size()]
			)
		return words
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
		if int(p.get("dream_kind", 0)) == 1:
			words += (
				"\n\nWoke with an idea about familiar materials at %s." % when(int(p.dream_at), now)
			)
		else:
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
			else "\nThe thought still gives a mild pull."
		)
	words += (
		"\n\nAge at start: %d. Gathering skill: %d.\nCarrying %.2f kg food."
		% [p.age_at_start, p.gathering_skill, float(p.carried_food_mg) / 1000000]
	)
	if int(p.memory_at) >= 0:
		var acts := {2: "Rested", 5: "Ate food", 6: "Drank water"}
		words += (
			"\n\nLast remembered: %s at %s."
			% [acts.get(int(p.memory_kind), "Acted"), when(int(p.memory_at), now)]
		)
	return words


static func kept_reasons(reasons: Array) -> String:
	var words := ""
	if reasons.size() >= 2 and reasons[0].has("score_differences"):
		var differences: PackedInt64Array = reasons[0].score_differences
		words += (
			"\nCompared with %s: need %+d, inclination/plan %+d, effort %+d."
			% [reasons[1].name, differences[0], differences[1], differences[2]]
		)
	for n in reasons.size():
		var r: Dictionary = reasons[n]
		words += (
			"\n%s %s · priority %d." % ["Chose" if n == 0 else "Rejected", r.name, int(r.score)]
		)
		if int(r.get("kind", 0)) == 3:
			words += (
				" Saw heat %d and %.2f kg fuel; estimate %d%% certain."
				% [int(r.observed_heat), float(r.observed_fuel_mg) / 1000000, int(r.confidence)]
			)
		else:
			words += (
				" Need met %d/100; expected +%d; about %d min."
				% [
					int(r.get("need_met", 100)),
					int(r.benefit),
					maxi(1, ceili(float(r.seconds) / 60))
				]
			)
		if int(r.get("unavailable", 0)) != 0:
			words += (
				" Unavailable: "
				+ [
					"",
					"no known source",
					"known source empty",
					"route blocked",
					"skill missing",
					"waiting after failed route"
				][int(r.unavailable)]
				+ "."
			)
	return words


static func _summary(p: Dictionary) -> String:
	var goals := ["food", "water", "rest", "nearby supplies"]
	var choice := int(p.choice)
	var action := str(p.activity)
	var reason := "Their known supplies weren't worth a trip yet."
	if not p.get("reasons", []).is_empty():
		var winner: Dictionary = p.reasons[0]
		if int(p.get("work_state", 0)) != 0:
			action = (
				"Making " + recipe(str(p.work_recipe))
				if p.work_known
				else "Trying familiar materials"
			)
		else:
			action = str(winner.name)
		return (
			"%s · %s\nWhy: need met %d/100, priority %d; estimate %d%% certain."
			% [p.name, action, int(winner.need_met), int(winner.score), int(winner.confidence)]
		)
	if int(p.action_code) == 1:
		action += " to " + goals[choice]
	if choice < 3:
		reason = "%s was %d/100." % [goals[choice].capitalize(), p.decision_needs[choice]]
	if int(p.get("dream_pull", 0)) > 0 and choice == 3:
		reason = "A remembered idea makes this feel worthwhile."
	return "%s · %s\nWhy: %s" % [p.name, action, reason]


static func name_of(id: int, people: Array) -> String:
	for person: Dictionary in people:
		if int(person.id) == id:
			return str(person.name)
	return "the scene" if id == 0 else "person %d" % id


static func recipe(key: String) -> String:
	return key.get_slice(":", 1).replace("_", " ").capitalize()


static func item(p: Dictionary, people: Array, details: bool, now: int) -> String:
	var holder := (
		"shared stock" if int(p.owner) == 0 else "held by " + name_of(int(p.owner), people)
	)
	var words := (
		"%s · %s\n%.3f kg · %d mm · %s"
		% [p.name, p.material, float(p.mass_mg) / 1000000, int(p.length_mm), holder]
	)
	if int(p.mass_mg) == 0:
		words += "\nSpent input · no usable material remains."
	if p.has("fire_heat"):
		words = "%s · %.2f kg fuel" % [p.name, float(p.fuel_mg) / 1000000]
		if int(p.fire_heat) >= 2:
			words += " · about %d minutes left" % ceili(float(p.fuel_seconds) / 60)
		elif int(p.fire_heat) == 1:
			words += " · glowing embers"
		else:
			words += " · already out"
		if details:
			words += "\n%.2f kg ash · heat %d." % [float(p.ash_mg) / 1000000, int(p.fire_heat)]
	if p.has("heat_exposure_seconds"):
		if int(p.state) == 1:
			words += "\nMore filling than raw."
		elif int(p.state) == 2:
			words += "\nBurnt · no food left."
		else:
			words += (
				"\n%d minutes of heat kept%s."
				% [int(p.heat_exposure_seconds) / 60, " · paused" if p.cooking_paused else ""]
			)
	var facts: Dictionary = p.get("facts", {})
	if facts.is_empty():
		words += "\nSelect a person to inspect their familiar properties."
	else:
		var labels := [
			"Hardness",
			"Edge",
			"Toughness",
			"Flaking",
			"Flexibility",
			"Weight",
			"Burn",
			"Fuel",
			"Food",
			"Water",
			"Poison",
			"Medicine",
			"Warmth",
			"Fibre",
			"Stickiness",
			"Plasticity",
			"Waterproof",
			"Pigment"
		]
		var sources := [
			"No source recorded",
			"Seen",
			"Starting knowledge",
			"Handled",
			"Experienced result",
			"Watched",
			"Told"
		]
		words += "\nKnown by %s" % name_of(int(p.observer), people)
		var shown: Array[int] = []
		if details:
			for i in 18:
				shown.append(i)
		else:
			for i in [1, 8, 12]:
				if (int(facts.mask) & (1 << i)) != 0 and int(facts.values[i]) > 0:
					shown.append(i)
			if shown.is_empty():
				shown.append(0)
		for i in shown:
			if (int(facts.mask) & (1 << i)) == 0:
				continue
			words += "\n%s %d" % [labels[i], int(facts.values[i])]
			if details:
				words += (
					" · %s at %s"
					% [sources[int(facts.sources[i])], when(int(facts.learned_at[i]), now)]
				)
	if int(p.made_at) >= 0:
		words += "\nMade by %s at %s." % [name_of(int(p.maker), people), when(int(p.made_at), now)]
	if details:
		words += "\nWear %.2f / 5 · quality %d / 5." % [float(p.wear) / 1000000, int(p.quality)]
	return words


static func knowledge(k: Dictionary, people: Array, now: int) -> String:
	if k.is_empty():
		return ""
	var actions := [
		"gather",
		"dig",
		"strike",
		"press",
		"cut",
		"scrape",
		"grind",
		"twist",
		"bind",
		"weave",
		"shape",
		"drill",
		"heat",
		"soak",
		"dry",
		"mix",
		"stack",
		"plant",
		"throw",
		"feed",
		"apply"
	]
	var words := "\n\nPersonal knowledge"
	var routes := [
		"Known from the start", "Own accident", "Own experiment", "Own hunch", "Watched", "Taught"
	]
	for skill: Dictionary in k.skills:
		words += (
			"\n%s · skill %.1f · %s"
			% [skill.name, float(skill.level) / 1000, routes[int(skill.route)]]
		)
		if int(skill.source) != 0:
			words += " · " + name_of(int(skill.source), people)
	for observation: Dictionary in k.get("observations", []):
		words += "\nObserved use · %.2f / 5 credits · still learning." % float(observation.credits)
	for hunch: Dictionary in k.hunches:
		words += (
			"\nHunch: try %s with %s · hint at %s."
			% [actions[int(hunch.action)], ", ".join(hunch.inputs), when(int(hunch.last_use), now)]
		)
		if int(hunch.get("origin", 0)) == 2:
			words += " From a dream."
		if int(hunch.source) != 0:
			words += " From " + name_of(int(hunch.source), people) + "."
	for choice: Dictionary in k.get("choices", []):
		words += "\n\nKept choice at " + when(int(choice.at), now) + kept_reasons(choice.reasons)
	return words


static func history(event: Dictionary, people: Array, now: int) -> String:
	var actor := name_of(int(event.actor), people)
	var words := ""
	match int(event.kind):
		1:
			words = "%s noticed %s" % [actor, str(event.name).to_lower()]
		2:
			words = "%s learned %s" % [actor, str(event.name).to_lower()]
			if int(event.source) != 0:
				words += " from " + name_of(int(event.source), people)
		3:
			words = "%s: no holder of %s remains" % [actor, str(event.name).to_lower()]
		4:
			words = "%s rediscovered %s" % [actor, str(event.name).to_lower()]
	if int(event.source) == 0:
		words += " · no source recorded"
	words += " at %s." % when(int(event.at), now)
	var routes := [
		"starting knowledge", "an accident", "an experiment", "a hunch", "watching", "teaching"
	]
	words += (
		"\nRoute: %s. Place: %.1f m east, %.1f m north."
		% [routes[int(event.route)], float(event.east_cm) / 100, float(event.north_cm) / 100]
	)
	if not str(event.word).is_empty():
		words += "\nWord: " + str(event.word) + "."
	words += "\nEvent %d · %d inputs." % [int(event.id), event.inputs.size()]
	for source in event.get("heat_sources", []):
		words += (
			"\nHeat: fire %d, ember %d, %d seconds."
			% [int(source.fire), int(source.origin), int(source.seconds)]
		)
		if int(source.tended_at) >= 0:
			words += " Tended at %s." % when(int(source.tended_at), now)
	words += kept_reasons(event.get("reasons", []))
	return words

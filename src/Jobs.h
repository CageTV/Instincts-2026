/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Jobs
{
	// Pet jobs, built on Harbinger (APMF). Gather: a gatherer or scavenger pet walks to up to `maxItems` unharvested plants within `range`, picks them
	// (they go into its own inventory), stays away until `gameHours` game hours have passed, then walks back to the player. Combat, death or a lost
	// destination ends the trip. For now it starts with the jobs key while the crosshair is on the pet; the dialogue entry comes later.
	void Init();

	// Sends one gatherer / scavenger pet out to gather (the menu entry). Returns true if it set out. Game thread only.
	bool Gather(RE::Actor* a_pet);
}

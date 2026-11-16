/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace BoxSkip
{
	// Skips the "What would you like to do? [Attempt to Tame] [Activate]" message box that Instincts' Taming perk raises on a wild animal:
	// the box is hidden and the first choice ("Attempt to Tame") is run for the player, so pressing E goes straight to the feeding menu.
	void Init();

	// Shows "Attempt to Tame" under the crosshair (the E prompt) on wild animals that can be tamed right now.
	void InitLabel();
}

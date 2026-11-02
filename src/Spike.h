/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Spike
{
	// Harbinger (APMF) test: with `spike.enabled` on, pressing the spike key sends the animal under the crosshair walking to a point `distance` units
	// ahead of the player through Harbinger's travel facet, logs the leg once a second, then releases it. Shows whether Harbinger can drive an animal
	// the way the pet jobs need. Off by default; nothing happens unless the key is pressed.
	void Init();
}

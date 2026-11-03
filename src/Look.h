/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Look
{
	// Lets the skin variants of Dynamic Animal Variants Plus show on Instincts pets: adds each pet race to the armor addons of that mod
	// that name the matching wild race, and registers the Papyrus function BeastHarmony_Look.CopyLook(wild, pet).
	void Init();
	// While the plugin loads
	void RegisterPapyrus();
}

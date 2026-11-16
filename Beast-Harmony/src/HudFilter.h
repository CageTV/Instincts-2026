/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace HudFilter
{
	// Hides the on-screen line "<animal> is too powerful for Subdued". It comes from the Calm part of the knock-out spell and means nothing to the player.
	// Only that one message is dropped; every other HUD message (and the same wording for any other spell) is left alone.
	void Init();
}

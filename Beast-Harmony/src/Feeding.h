/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#pragma once

namespace Feeding
{
	// How much a pet gets out of food, decided here and not in a script: the Papyrus function BeastHarmony_Feeding.Multiplier(akPlayer) returns
	// 1.0 + the feeding bonus of the player's race (Bosmer 15 % by default, see config.json "feeding"). The patched Instincts feeding script multiplies
	// the hunger it restores with it. Register while the plugin loads.
	bool Bind(RE::BSScript::IVirtualMachine* a_vm);
}

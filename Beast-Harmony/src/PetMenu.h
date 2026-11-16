/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#pragma once

namespace PetMenu
{
	// The "Do something for me" menu: a list of what the pet can do (Gather ...). It is opened by the Papyrus function
	// BeastHarmony_Menu.Open(akPet), which the patched dialogue fragment of that topic calls. Register while the plugin loads.
	bool Bind(RE::BSScript::IVirtualMachine* a_vm);
}

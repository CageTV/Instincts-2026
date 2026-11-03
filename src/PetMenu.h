/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace PetMenu
{
	// The "Do something for me" menu: a list of what the pet can do (Gather ...). It is opened by the Papyrus function
	// BeastHarmony_Menu.Open(akPet), which the patched dialogue fragment of that topic calls. Register while the plugin loads.
	void RegisterPapyrus();
}

/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#pragma once

namespace Widget
{
	void Init();               // registers the HUD element with SKSE Menu Framework (nothing happens without the framework)
	void SetEditing(bool);     // menu: show a preview and let the mouse move it
	bool Editing();
}

/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Widget
{
	void Init();               // registers the HUD element with SKSE Menu Framework (nothing happens without the framework)
	void SetEditing(bool);     // menu: show a preview and let the mouse move it
	bool Editing();
}

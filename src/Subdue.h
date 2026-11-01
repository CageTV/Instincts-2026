/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Subdue
{
	// Looks up BeastHarmony.esp's records and Instincts' lists, then listens for hits and for the Taming Mode key.
	void Init();

	// Does this wild animal currently offer "Attempt to Tame" (a species Instincts can tame, alive, not already a companion, and either a docile
	// species or hurt below the taming limit / at 10 HP or less)? Same rules as Instincts' Taming perk.
	bool QualifiesForTaming(RE::Actor* a_actor);
}

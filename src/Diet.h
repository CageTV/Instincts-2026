/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Diet
{
	// Trims Instincts' food lists at game load: the carnivore (meat) lists lose honey, sweets, bee products, wings and jellies; the herbivore
	// (plant) lists lose poisons, Nirnroot, Spriggan sap and sweets. Which species uses which list is left to Instincts.
	void Init();
}

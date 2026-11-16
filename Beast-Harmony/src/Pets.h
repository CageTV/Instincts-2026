/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#pragma once

namespace Pets
{
	// Sorts every tamed Instincts animal into a class (fighter / scavenger / gatherer / producer) and sets its combat behaviour from the class.
	void Init();

	// The class name of a tamed Instincts animal ("fighter", "scavenger", "gatherer", "producer"), or "" if the actor is not one. Game thread only.
	std::string ClassOf(RE::Actor* a_actor);

	// The species name of a tamed Instincts animal ("fox", "wolf", "hare" ...), or "" if unknown. Game thread only.
	std::string SpeciesNameOf(RE::Actor* a_actor);
}

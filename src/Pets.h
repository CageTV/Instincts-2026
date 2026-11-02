/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#pragma once

namespace Pets
{
	// Sorts every tamed Instincts animal into a class (fighter / scavenger / gatherer / producer) and sets its combat behaviour from the class.
	void Init();

	// The class name of a tamed Instincts animal ("fighter", "scavenger", "gatherer", "producer"), or "" if the actor is not one. Game thread only.
	std::string ClassOf(RE::Actor* a_actor);
}

/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Licensed under the GNU General Public License, version 3 or later; see LICENSE.
 */
#include "PCH.h"
#include "HudFilter.h"

namespace HudFilter
{
	namespace
	{
		using ProcessMessage_t = RE::UI_MESSAGE_RESULTS (*)(RE::HUDMenu*, RE::UIMessage&);
		std::uintptr_t g_original = 0;

		RE::UI_MESSAGE_RESULTS Thunk(RE::HUDMenu* a_menu, RE::UIMessage& a_message)
		{
			try {
				if (a_message.data) {
					if (const auto* hud = skyrim_cast<RE::HUDData*>(a_message.data)) {
						const char* text = hud->text.c_str();
						if (text && std::string_view(text).find("too powerful for Subdued") != std::string_view::npos) {
							static int logged = 0;
							if (logged < 3) {
								++logged;
								SKSE::log::info("hid HUD message: {}", text);
							}
							return RE::UI_MESSAGE_RESULTS::kHandled;
						}
					}
				}
			} catch (...) {
			}
			return reinterpret_cast<ProcessMessage_t>(g_original)(a_menu, a_message);
		}
	}

	void Init()
	{
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_HUDMenu[0] };
		g_original = vtbl.write_vfunc(0x4, reinterpret_cast<std::uintptr_t>(&Thunk));
		SKSE::log::info("hud filter: hooked HUDMenu::ProcessMessage (original {:X})", g_original);
	}
}

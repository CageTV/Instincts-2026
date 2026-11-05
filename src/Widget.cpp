/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Widget.h"
#include "Config.h"
#include "Subdue.h"
#include "KeyPicker.h"

// The knock-out indicator: visible for as long as the knock-out (Taming Mode) is active, i.e. until the toggle key is pressed again.
// The position is saved as fractions of the screen, so it survives resolution changes. While "move with the mouse" is on in the menu page
// (and the SKSE Menu Framework window is open) a preview is drawn even when the mode is off and can be dragged anywhere.
namespace Widget
{
	namespace
	{
		bool editing = false;
		bool dragging = false;

		void Render()
		{
			using namespace ImGuiMCP;
			auto& w = Config::GetWidget();
			if (!w.enabled) {
				return;
			}
			const bool preview = editing && SKSEMenuFramework::IsAnyBlockingWindowOpened();
			const bool active = Subdue::KnockoutActive();
			if (!active && !preview) {
				return;
			}
			if (!preview) {
				// not over a full screen menu / the loading screen
				if (auto* ui = RE::UI::GetSingleton(); ui && (ui->GameIsPaused() || !ui->IsShowingMenus())) {
					return;
				}
			}

			const auto* io = GetIO();
			const float W = io->DisplaySize.x;
			const float H = io->DisplaySize.y;
			auto*       dl = GetForegroundDrawList();

			const float k = w.scale;
			const float bw = 64.0f * k;
			const float bh = 64.0f * k;

			float x = std::clamp(w.x * W, 0.0f, std::max(0.0f, W - bw));
			float y = std::clamp(w.y * H, 0.0f, std::max(0.0f, H - bh));

			if (preview) {
				const ImVec2 m = io->MousePos;
				if (!dragging && IsMouseClicked(0) && !io->WantCaptureMouse && m.x >= x && m.x <= x + bw && m.y >= y && m.y <= y + bh) {
					dragging = true;
				}
				if (dragging) {
					if (IsMouseDown(0)) {
						x = std::clamp(x + io->MouseDelta.x, 0.0f, std::max(0.0f, W - bw));
						y = std::clamp(y + io->MouseDelta.y, 0.0f, std::max(0.0f, H - bh));
						w.x = x / W;
						w.y = y / H;
					} else {
						dragging = false;
						Config::Save();
					}
				}
			} else {
				dragging = false;
			}

			// just the icon (a paw print), white with a soft dark shadow; the preview is dimmer and gets a frame while it is dragged
			static ImTextureID tex = SKSEMenuFramework::LoadTexture("Data\Interface\BeastHarmony\knockout.png");
			const bool dim = preview && !active;
			if (tex) {
				const float sh = 2.0f * k;
				ImDrawListManager::AddImage(dl, tex, ImVec2(x + sh, y + sh), ImVec2(x + bw + sh, y + bh + sh), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(0, 0, 0, dim ? 90 : 170));
				ImDrawListManager::AddImage(dl, tex, ImVec2(x, y), ImVec2(x + bw, y + bh), ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 205, 90, dim ? 120 : 245));
			} else {
				ImDrawListManager::AddRectFilled(dl, ImVec2(x, y), ImVec2(x + bw, y + bh), IM_COL32(230, 160, 40, 200), 8.0f * k, 0);
			}
			if (preview) {
				ImDrawListManager::AddRect(dl, ImVec2(x, y), ImVec2(x + bw, y + bh), dragging ? IM_COL32(255, 235, 140, 255) : IM_COL32(230, 160, 40, 160), 6.0f * k, 0, 1.5f);
			}
		}
	}

	void SetEditing(bool a_on)
	{
		editing = a_on;
		if (!a_on) {
			dragging = false;
		}
	}

	bool Editing() { return editing; }

	void Init()
	{
		if (!SKSEMenuFramework::IsInstalled()) {
			SKSE::log::info("widget: SKSE Menu Framework not installed, no knock-out indicator");
			return;
		}
		SKSEMenuFramework::AddHudElement(Render);
		SKSE::log::info("widget: knock-out indicator registered");
	}
}

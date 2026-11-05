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

			const std::string line1 = preview && !active ? "KNOCK-OUT MODE (preview)" : "KNOCK-OUT MODE ON";
			const std::string line2 = std::format("press {} to turn it off", KeyPicker::KeyName(Config::Get().toggleKey));
			ImVec2            s1{}, s2{};
			CalcTextSize(&s1, line1.c_str(), nullptr, false, -1.0f);
			CalcTextSize(&s2, line2.c_str(), nullptr, false, -1.0f);
			const float k = w.scale;
			const float pad = 8.0f * k;
			const float iconW = 84.0f * k;
			const float iconH = 58.0f * k;
			const float textW = std::max(s1.x, s2.x * 0.8f) * k;
			const float textH = (s1.y + s2.y * 0.8f) * k;
			const float bw = pad * 2.0f + iconW + 6.0f * k + textW;
			const float bh = std::max(iconH, textH) + pad * 2.0f;

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

			const auto col = [](int r, int g, int b, int a) { return IM_COL32(r, g, b, a); };
			ImDrawListManager::AddRectFilled(dl, ImVec2(x, y), ImVec2(x + bw, y + bh), col(12, 14, 18, 190), 6.0f * k, 0);
			ImDrawListManager::AddRect(dl, ImVec2(x, y), ImVec2(x + bw, y + bh), dragging ? col(255, 235, 140, 255) : col(230, 160, 40, 235), 6.0f * k, 0, 2.0f);

			// a dog silhouette (side view, facing right) with a cartoon hit marker on the head
			const float ox = x + pad;
			const float oy = y + (bh - iconH) * 0.5f;
			const auto  P = [&](float px, float py) { return ImVec2(ox + px * k, oy + py * k); };
			const ImU32 sil = col(225, 225, 230, 240);
			ImDrawListManager::AddRectFilled(dl, P(14, 22), P(54, 38), sil, 6.0f * k, 0);                 // body
			ImDrawListManager::AddTriangleFilled(dl, P(15, 24), P(15, 31), P(3, 15), sil);                // tail
			ImDrawListManager::AddRectFilled(dl, P(16, 36), P(22, 54), sil, 2.0f * k, 0);                 // legs
			ImDrawListManager::AddRectFilled(dl, P(27, 36), P(33, 54), sil, 2.0f * k, 0);
			ImDrawListManager::AddRectFilled(dl, P(40, 36), P(46, 54), sil, 2.0f * k, 0);
			ImDrawListManager::AddRectFilled(dl, P(49, 36), P(55, 54), sil, 2.0f * k, 0);
			ImDrawListManager::AddRectFilled(dl, P(48, 18), P(60, 34), sil, 4.0f * k, 0);                 // neck
			ImDrawListManager::AddCircleFilled(dl, P(60, 20), 9.0f * k, sil, 24);                         // head
			ImDrawListManager::AddRectFilled(dl, P(64, 20), P(80, 28), sil, 3.0f * k, 0);                 // snout
			ImDrawListManager::AddTriangleFilled(dl, P(54, 14), P(58, 1), P(63, 13), sil);                // ear
			ImDrawListManager::AddCircleFilled(dl, P(65, 18), 1.6f * k, col(20, 20, 24, 255), 8);         // eye
			// hit marker: a burst of ticks around the head, as in a cartoon
			const ImU32 hit = col(255, 70, 40, 255);
			const ImVec2 c = P(60, 20);
			const float  ticks[8][2] = { { 1, 0 }, { 0.7071f, 0.7071f }, { 0, 1 }, { -0.7071f, 0.7071f }, { -1, 0 }, { -0.7071f, -0.7071f }, { 0, -1 }, { 0.7071f, -0.7071f } };
			for (const auto& t : ticks) {
				ImDrawListManager::AddLine(dl, ImVec2(c.x + t[0] * 12.0f * k, c.y + t[1] * 12.0f * k), ImVec2(c.x + t[0] * 19.0f * k, c.y + t[1] * 19.0f * k), hit, 3.0f * k);
			}
			ImDrawListManager::AddCircle(dl, c, 11.0f * k, col(255, 190, 60, 255), 24, 2.0f * k);

			const float tx = ox + iconW + 6.0f * k;
			const float ty = y + (bh - textH) * 0.5f;
			ImDrawListManager::AddText(dl, GetFont(), GetFontSize() * k, ImVec2(tx, ty), col(255, 200, 80, 255), line1.c_str());
			ImDrawListManager::AddText(dl, GetFont(), GetFontSize() * k * 0.8f, ImVec2(tx, ty + s1.y * k), col(210, 210, 210, 230), line2.c_str());
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

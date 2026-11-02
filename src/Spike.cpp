/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Spike.h"
#include "APMF_API.h"
#include "Config.h"

#include <chrono>
#include <cmath>
#include <format>
#include <thread>
#include <Windows.h>

namespace Spike
{
	namespace
	{
		const APMF_API::APMF_API_v12* g_api = nullptr;
		bool                          g_busy = false;

		const char* StateName(std::uint32_t a_state)
		{
			switch (a_state) {
			case APMF_API::kLeg_None: return "None";
			case APMF_API::kLeg_Pending: return "Pending";
			case APMF_API::kLeg_Walking: return "Walking";
			case APMF_API::kLeg_Arrived: return "Arrived";
			case APMF_API::kLeg_Blocked: return "Blocked";
			case APMF_API::kLeg_CombatCancelled: return "CombatCancelled";
			case APMF_API::kLeg_DestGone: return "DestGone";
			case APMF_API::kLeg_StuckTimeout: return "StuckTimeout";
			case APMF_API::kLeg_ActorGone: return "ActorGone";
			case APMF_API::kLeg_Failed: return "Failed";
			case APMF_API::kLeg_Released: return "Released";
			default: return "?";
			}
		}

		bool Connect()
		{
			if (g_api) {
				return true;
			}
			const auto mod = GetModuleHandleA("APMF.dll");
			if (!mod) {
				SKSE::log::warn("spike: APMF.dll (Harbinger) is not loaded");
				return false;
			}
			const auto fn = reinterpret_cast<APMF_API::GetInterface_t>(GetProcAddress(mod, APMF_API::kGetInterfaceExport));
			if (!fn) {
				SKSE::log::warn("spike: APMF.dll has no {}", APMF_API::kGetInterfaceExport);
				return false;
			}
			const auto* base = fn(12);
			if (!base) {
				SKSE::log::warn("spike: Harbinger refused ABI 12");
				return false;
			}
			SKSE::log::info("spike: Harbinger ABI {} (built against {})", base->abiVersion, APMF_API::kABIVersion);
			if (base->abiVersion < 12) {
				return false;
			}
			g_api = reinterpret_cast<const APMF_API::APMF_API_v12*>(base);
			return true;
		}

		void Start()
		{
			if (g_busy) {
				SKSE::log::info("spike: a test is already running");
				return;
			}
			auto* player = RE::PlayerCharacter::GetSingleton();
			const auto* pick = RE::CrosshairPickData::GetSingleton();
			const auto  target = pick ? pick->GetActiveTarget().get() : nullptr;
			auto*       actor = target ? target->As<RE::Actor>() : nullptr;
			if (!player || !actor || actor == player) {
				SKSE::log::info("spike: aim the crosshair at an animal first");
				return;
			}
			if (!Connect()) {
				return;
			}
			const float dist = Config::GetSpike().distance;
			const float ang = player->GetAngleZ();
			const auto  p = player->GetPosition();
			APMF_API::APMF_Param param{};
			param.ival = static_cast<std::int32_t>(APMF_API::kTravel_ToPosition | APMF_API::kTravel_SpeedSet | APMF_API::kTravel_SpeedJog);
			param.posX = p.x + std::sin(ang) * dist;
			param.posY = p.y + std::cos(ang) * dist;
			param.posZ = p.z;
			const RE::FormID id = actor->GetFormID();
			const auto       handle = g_api->RequestEx(id, APMF_API::kIntent_Travel, 50.0f, &param);
			SKSE::log::info("spike: {} [{:08X}] travel to ({:.0f},{:.0f},{:.0f}), {:.0f} units away -> handle {}", actor->GetName() ? actor->GetName() : "?", id, param.posX, param.posY, param.posZ, dist, handle);
			if (handle == APMF_API::kInvalidHandle) {
				return;
			}
			g_busy = true;
			const float dx = param.posX, dy = param.posY;
			std::thread([id, handle, dx, dy]() {
				bool ended = false;
				for (int sec = 1; sec <= 50 && !ended; ++sec) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					APMF_API::APMF_TravelLegInfo info{};
					info.size = sizeof(info);
					const auto state = g_api->GetTravelLegState(id, &info);
					SKSE::GetTaskInterface()->AddTask([id, sec, state, dx, dy]() {
						auto* a = RE::TESForm::LookupByID<RE::Actor>(id);
						if (a) {
							const auto pos = a->GetPosition();
							SKSE::log::info("spike +{}s: state {} , {:.0f} units from the point, in combat {}", sec, StateName(state), std::hypot(pos.x - dx, pos.y - dy), a->IsInCombat());
						} else {
							SKSE::log::info("spike +{}s: state {} , actor not found", sec, StateName(state));
						}
					});
					ended = state == APMF_API::kLeg_Arrived || state == APMF_API::kLeg_CombatCancelled || state == APMF_API::kLeg_Failed || state == APMF_API::kLeg_DestGone ||
					        state == APMF_API::kLeg_ActorGone || state == APMF_API::kLeg_StuckTimeout || state == APMF_API::kLeg_Blocked;
				}
				g_api->Release(handle);
				SKSE::log::info("spike: claim released (handle {})", handle);
				for (const int sec : { 2, 6 }) {
					std::this_thread::sleep_for(std::chrono::seconds(sec == 2 ? 2 : 4));
					SKSE::GetTaskInterface()->AddTask([id, sec, dx, dy]() {
						if (auto* a = RE::TESForm::LookupByID<RE::Actor>(id)) {
							const auto pos = a->GetPosition();
							SKSE::log::info("spike: {}s after release: {:.0f} units from the point, in combat {}", sec, std::hypot(pos.x - dx, pos.y - dy), a->IsInCombat());
						}
					});
				}
				g_busy = false;
			}).detach();
		}

		struct KeySink : RE::BSTEventSink<RE::InputEvent*>
		{
			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_events, RE::BSTEventSource<RE::InputEvent*>*) override
			{
				if (!a_events) {
					return RE::BSEventNotifyControl::kContinue;
				}
				for (auto* e = *a_events; e; e = e->next) {
					const auto* b = e->AsButtonEvent();
					if (b && b->IsDown() && b->GetDevice() == RE::INPUT_DEVICE::kKeyboard && static_cast<int>(b->GetIDCode()) == Config::GetSpike().key) {
						auto* ui = RE::UI::GetSingleton();
						if (ui && !ui->GameIsPaused()) {
							SKSE::GetTaskInterface()->AddTask([]() { Start(); });
						}
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};
		KeySink keySink;
	}

	void Init()
	{
		if (!Config::GetSpike().enabled) {
			return;
		}
		if (auto* input = RE::BSInputDeviceManager::GetSingleton()) {
			input->AddEventSink(&keySink);
			SKSE::log::info("spike: on, key {} sends the animal under the crosshair {:.0f} units ahead via Harbinger", Config::GetSpike().key, Config::GetSpike().distance);
		}
	}
}

/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Jobs.h"
#include "APMF_API.h"
#include "Config.h"
#include "Keys.h"
#include "Pets.h"

#include <algorithm>
#include <atomic>
#include <map>
#include <random>
#include <chrono>
#include <cmath>
#include <format>
#include <thread>
#include <vector>
#include <Windows.h>

namespace Jobs
{
	namespace
	{
		const APMF_API::APMF_API_v12* g_api = nullptr;
		constexpr float               kBasis = 50.0f;

		enum class Phase { Walking, Dwelling, Returning };

		struct Job
		{
			RE::FormID               pet = 0;
			std::string              name;
			std::vector<RE::FormID>  plants;
			std::size_t              next = 0;       // index of the plant being walked to
			Phase                    phase = Phase::Walking;
			APMF_API::Handle         travel = 0;
			APMF_API::Handle         hold = 0;
			float                    startDays = 0.0f;
			std::chrono::steady_clock::time_point legStart;
			int                      picked = 0;
		};

		std::vector<Job>  g_jobs;            // game thread only
		std::atomic<bool> g_ticking{ false };

		void Note(const std::string& a_text) { RE::SendHUDMessage::ShowHUDMessage(a_text.c_str()); }

		bool Connect()
		{
			if (g_api) {
				return true;
			}
			const auto mod = GetModuleHandleA("APMF.dll");
			const auto fn = mod ? reinterpret_cast<APMF_API::GetInterface_t>(GetProcAddress(mod, APMF_API::kGetInterfaceExport)) : nullptr;
			const auto* base = fn ? fn(12) : nullptr;
			if (!base || base->abiVersion < 12) {
				SKSE::log::warn("jobs: Harbinger (APMF.dll) is not available");
				return false;
			}
			g_api = reinterpret_cast<const APMF_API::APMF_API_v12*>(base);
			return true;
		}

		float Dist2D(const RE::NiPoint3& a, const RE::NiPoint3& b) { return std::hypot(a.x - b.x, a.y - b.y); }

		// walk the pet to a reference (a plant, or the player)
		APMF_API::Handle WalkTo(RE::FormID a_pet, RE::FormID a_dest)
		{
			APMF_API::APMF_Param p{};
			p.form = a_dest;
			p.fval = Config::GetJobs().arriveRadius;
			p.ival = static_cast<std::int32_t>(APMF_API::kTravel_SpeedSet | APMF_API::kTravel_SpeedJog);
			return g_api->RequestEx(a_pet, APMF_API::kIntent_Travel, kBasis, &p);
		}

		void ReleaseAll(Job& j)
		{
			if (g_api) {
				if (j.travel) {
					g_api->Release(j.travel);
				}
				if (j.hold) {
					g_api->Release(j.hold);
				}
			}
			j.travel = 0;
			j.hold = 0;
		}

		// plants that give produce: flora (herbs, mushrooms) and trees/bushes (snowberries, juniper, the mod added berry bushes are TREE records)
		const RE::TESProduceForm* ProduceOf(RE::TESObjectREFR* a_ref)
		{
			auto* base = a_ref ? a_ref->GetBaseObject() : nullptr;
			if (!base) {
				return nullptr;
			}
			if (const auto* flora = base->As<RE::TESFlora>()) {
				return flora;
			}
			if (const auto* tree = base->As<RE::TESObjectTREE>()) {
				return tree;
			}
			return nullptr;
		}

		bool IsGatherable(RE::TESObjectREFR* a_ref)
		{
			if (!a_ref || a_ref->IsDisabled() || a_ref->IsDeleted() || !a_ref->Is3DLoaded()) {
				return false;
			}
			if (a_ref->GetFormFlags() & RE::TESObjectREFR::RecordFlags::kHarvested) {
				return false;
			}
			const auto* flora = ProduceOf(a_ref);
			return flora && flora->produceItem;
		}

		std::vector<RE::FormID> FindPlants(RE::Actor* a_pet, float a_range, int a_max)
		{
			std::vector<std::pair<float, RE::FormID>> found;
			int                                       withProduce = 0;  // refs that could give produce, whatever their state (for the log when nothing is found)
			if (auto* tes = RE::TES::GetSingleton()) {
				tes->ForEachReferenceInRange(a_pet, a_range, [&](RE::TESObjectREFR* a_ref) {
					if (const auto* p = ProduceOf(a_ref); p && p->produceItem) {
						++withProduce;
					}
					if (IsGatherable(a_ref)) {
						found.emplace_back(a_ref->GetPosition().GetDistance(a_pet->GetPosition()), a_ref->GetFormID());
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});
			}
			if (found.empty()) {
				SKSE::log::info("jobs: no gatherable plant in range; {} references with produce were seen (harvested, disabled or not loaded ones do not count)", withProduce);
			}
			std::ranges::sort(found);
			std::vector<RE::FormID> out;
			for (const auto& f : found) {
				if (static_cast<int>(out.size()) >= a_max) {
					break;
				}
				out.push_back(f.second);
			}
			return out;
		}

		// the pet picks a plant: the engine's own activation, so the plant looks harvested and the produce lands in the pet's inventory
		bool Pick(RE::Actor* a_pet, RE::TESObjectREFR* a_plant)
		{
			const auto* flora = ProduceOf(a_plant);
			if (!flora || !flora->produceItem) {
				return false;
			}
			const auto before = a_pet->GetInventoryCounts();
			const auto count = [&](const std::map<RE::TESBoundObject*, std::int32_t>& m) {
				const auto it = m.find(flora->produceItem);
				return it != m.end() ? it->second : 0;
			};
			const int  had = count(before);
			a_plant->ActivateRef(a_pet, 0, nullptr, 1, false);
			const int  now = count(a_pet->GetInventoryCounts());
			const bool harvested = (a_plant->GetFormFlags() & RE::TESObjectREFR::RecordFlags::kHarvested) != 0;
			SKSE::log::info("jobs: {} picked {:08X}: {} in inventory {} -> {}, plant harvested {}", a_pet->GetName() ? a_pet->GetName() : "?", a_plant->GetFormID(),
				flora->produceItem->GetName() ? flora->produceItem->GetName() : "?", had, now, harvested);
			return now > had || harvested;
		}

		// the animal's own "task done" sound (generated with ElevenLabs, records BH_PetDone_* in BeastHarmony.esp, files in Data/Sound/FX/BeastHarmony)
		void PlayDone(RE::Actor* a_pet)
		{
			if (!Config::GetJobs().sound || !a_pet) {
				return;
			}
			static const std::map<std::string, std::vector<std::uint32_t>> kSounds = {
				{ "fox", { 0x804 } }, { "snow fox", { 0x804 } },
				{ "wolf", { 0x805, 0x806, 0x807 } }, { "ice wolf", { 0x805, 0x806, 0x807 } },  // the dog family has several
				{ "hare", { 0x808 } }, { "deer", { 0x809 } }, { "glow deer", { 0x809 } }, { "elk", { 0x809 } }, { "goat", { 0x80A } }, { "chicken", { 0x80B } },
			};
			const auto species = Pets::SpeciesNameOf(a_pet);
			const auto it = kSounds.find(species);
			if (it == kSounds.end()) {
				return;
			}
			static std::mt19937 rng{ std::random_device{}() };
			const auto          id = it->second[std::uniform_int_distribution<std::size_t>(0, it->second.size() - 1)(rng)];
			auto*               dh = RE::TESDataHandler::GetSingleton();
			auto*               form = dh ? dh->LookupForm<RE::BGSSoundDescriptorForm>(id, "BeastHarmony.esp"sv) : nullptr;
			auto*               am = RE::BSAudioManager::GetSingleton();
			if (!form || !form->soundDescriptor || !am) {
				SKSE::log::warn("jobs: sound record {:X} not found", id);
				return;
			}
			RE::BSSoundHandle handle;
			if (!am->GetSoundHandle(handle, form->soundDescriptor)) {
				SKSE::log::warn("jobs: could not build the sound {:X}", id);
				return;
			}
			handle.SetPosition(a_pet->GetPosition());
			if (auto* node = a_pet->Get3D()) {
				handle.SetObjectToFollow(node);
			}
			handle.SetVolume(Config::GetJobs().soundVolume);
			handle.Play();
			SKSE::log::info("jobs: {} makes its {} sound ({:X})", a_pet->GetName() ? a_pet->GetName() : "?", species, id);
		}

		void Finish(Job& j, const char* a_why)
		{
			ReleaseAll(j);
			PlayDone(RE::TESForm::LookupByID<RE::Actor>(j.pet));
			SKSE::log::info("jobs: {} [{:08X}] trip over ({}), {} picked", j.name, j.pet, a_why, j.picked);
			Note(std::format("{} is back{}.", j.name, j.picked ? std::format(" with {} item(s)", j.picked) : std::string(" empty-handed")));
		}

		// returns true when the job is finished and should be removed
		bool Step(Job& j)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(j.pet);
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!pet || !player || pet->IsDead() || !pet->Is3DLoaded()) {
				ReleaseAll(j);
				SKSE::log::info("jobs: {} [{:08X}] gone, trip dropped", j.name, j.pet);
				return true;
			}
			if (pet->IsInCombat()) {
				Finish(j, "combat");
				return true;
			}
			const auto secs = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - j.legStart).count();
			APMF_API::APMF_TravelLegInfo info{};
			info.size = sizeof(info);
			const auto state = g_api->GetTravelLegState(j.pet, &info);

			if (j.phase == Phase::Walking) {
				auto* plant = j.next < j.plants.size() ? RE::TESForm::LookupByID<RE::TESObjectREFR>(j.plants[j.next]) : nullptr;
				const bool ended = state == APMF_API::kLeg_Blocked || state == APMF_API::kLeg_StuckTimeout || state == APMF_API::kLeg_Failed || state == APMF_API::kLeg_DestGone;
				const bool arrived = state == APMF_API::kLeg_Arrived || (plant && pet->GetPosition().GetDistance(plant->GetPosition()) < Config::GetJobs().arriveRadius + 50.0f);
				if (plant && arrived) {
					if (Pick(pet, plant)) {
						++j.picked;
					}
				} else if (plant && !ended && secs < 60) {
					return false;  // still walking
				} else {
					SKSE::log::info("jobs: {} gave up on plant {} (state {}, {}s)", j.name, j.next, state, secs);
				}
				++j.next;
				if (j.travel) {
					g_api->Release(j.travel);
					j.travel = 0;
				}
				if (j.next < j.plants.size()) {
					j.travel = WalkTo(j.pet, j.plants[j.next]);
					j.legStart = std::chrono::steady_clock::now();
					return j.travel == 0;
				}
				// all plants done: stand still where it is until the time is up
				j.hold = g_api->Request(j.pet, APMF_API::kIntent_MovementBlock, kBasis);
				j.phase = Phase::Dwelling;
				SKSE::log::info("jobs: {} waits for {:.2f} game hours", j.name, Config::GetJobs().gameHours);
				return false;
			}

			if (j.phase == Phase::Dwelling) {
				const auto* cal = RE::Calendar::GetSingleton();
				const float hours = cal ? (cal->GetCurrentGameTime() - j.startDays) * 24.0f : 99.0f;
				if (hours < Config::GetJobs().gameHours) {
					return false;
				}
				if (j.hold) {
					g_api->Release(j.hold);
					j.hold = 0;
				}
				j.travel = WalkTo(j.pet, 0x14);
				j.phase = Phase::Returning;
				j.legStart = std::chrono::steady_clock::now();
				SKSE::log::info("jobs: {} heads back (handle {})", j.name, j.travel);
				if (!j.travel) {
					Finish(j, "could not walk back");
					return true;
				}
				return false;
			}

			// Returning
			const bool done = state == APMF_API::kLeg_Arrived || pet->GetPosition().GetDistance(player->GetPosition()) < Config::GetJobs().arriveRadius + 150.0f;
			if (done || secs > 120 || state == APMF_API::kLeg_Blocked || state == APMF_API::kLeg_StuckTimeout || state == APMF_API::kLeg_Failed) {
				Finish(j, done ? "arrived" : "gave up on the way back");
				return true;
			}
			return false;
		}

		void Tick()
		{
			try {
				for (auto it = g_jobs.begin(); it != g_jobs.end();) {
					it = Step(*it) ? g_jobs.erase(it) : it + 1;
				}
			} catch (const std::exception& e) {
				SKSE::log::error("jobs tick: {}", e.what());
				for (auto& j : g_jobs) {
					ReleaseAll(j);
				}
				g_jobs.clear();
			}
			if (g_jobs.empty()) {
				g_ticking = false;
			}
		}

		void StartTicker()
		{
			if (g_ticking.exchange(true)) {
				return;
			}
			std::thread([]() {
				while (g_ticking) {
					std::this_thread::sleep_for(std::chrono::seconds(1));
					SKSE::GetTaskInterface()->AddTask([]() { Tick(); });
				}
			}).detach();
		}

		// sends one pet out; returns true if it set out
		bool StartFor(RE::Actor* pet, const std::string& cls)
		{
			const std::string name = pet->GetName() ? pet->GetName() : "Your pet";
			if (std::ranges::any_of(g_jobs, [&](const Job& j) { return j.pet == pet->GetFormID(); })) {
				return false;
			}
			if (pet->IsInCombat() || pet->IsDead()) {
				return false;
			}
			const auto& cfg = Config::GetJobs();
			Job         j;
			j.pet = pet->GetFormID();
			j.name = name;
			j.plants = FindPlants(pet, cfg.range, cfg.maxItems);
			if (j.plants.empty()) {
				Note(name + " finds nothing to gather nearby.");
				SKSE::log::info("jobs: {} [{:08X}] found no plants within {:.0f} units", name, j.pet, cfg.range);
				return false;
			}
			j.startDays = RE::Calendar::GetSingleton() ? RE::Calendar::GetSingleton()->GetCurrentGameTime() : 0.0f;
			j.legStart = std::chrono::steady_clock::now();
			j.travel = WalkTo(j.pet, j.plants[0]);
			if (!j.travel) {
				SKSE::log::warn("jobs: Harbinger refused the first leg for {}", name);
				return false;
			}
			SKSE::log::info("jobs: {} [{:08X}] ({}) sets out for {} plant(s), staying away {:.2f} game hours", name, j.pet, cls, j.plants.size(), cfg.gameHours);
			Note(std::format("{} sets out to gather.", name));
			g_jobs.push_back(std::move(j));
			StartTicker();
			return true;
		}

		// one key: every tamed gatherer / scavenger near the player goes out for what its species can gather
		void Start()
		{
			auto* player = RE::PlayerCharacter::GetSingleton();
			auto* lists = RE::ProcessLists::GetSingleton();
			if (!player || !lists || !Connect()) {
				return;
			}
			int pets = 0;
			int out = 0;
			for (auto& handle : lists->highActorHandles) {
				const auto actor = handle.get();
				if (!actor || actor.get() == player) {
					continue;
				}
				const std::string cls = Pets::ClassOf(actor.get());
				if (cls != "gatherer" && cls != "scavenger") {
					continue;
				}
				++pets;
				if (StartFor(actor.get(), cls)) {
					++out;
				}
			}
			if (pets == 0) {
				Note("None of your pets nearby can gather.");
				SKSE::log::info("jobs: no tamed gatherer or scavenger is loaded near the player");
			} else {
				SKSE::log::info("jobs: {} of {} pet(s) set out", out, pets);
			}
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
					if (b && b->IsDown() && b->GetDevice() == RE::INPUT_DEVICE::kKeyboard && static_cast<int>(b->GetIDCode()) == Config::GetJobs().key && Keys::ModifierHeld(Config::GetJobs().modifier)) {
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
		if (!Config::GetJobs().enabled) {
			return;
		}
		if (auto* input = RE::BSInputDeviceManager::GetSingleton()) {
			input->AddEventSink(&keySink);
			SKSE::log::info("jobs: on, key {} sends every nearby gatherer / scavenger pet out to gather", Config::GetJobs().key);
		}
	}
}

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

		enum class Phase { Walking, Dwelling, Returning, Hunting };
		enum class Kind { Gather, Hunt };

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
			// Hunt
			Kind                     kind = Kind::Gather;
			RE::FormID               prey = 0;            // the animal it is after now
			std::vector<RE::FormID>  tried;               // animals already chased on this trip
			int                      kills = 0;
			std::chrono::steady_clock::time_point preyStart;   // when it started after this prey
			std::chrono::steady_clock::time_point lastEngage;  // last time it was told to attack
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

		// fish are flora in Skyrim (and in CC Fishing / Improved Fish): recognised by the name of the fish or of what it gives
		bool NameHasFish(const char* a_name)
		{
			if (!a_name || !*a_name) {
				return false;
			}
			std::string n = a_name;
			std::ranges::transform(n, n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			if (n.find("egg") != std::string::npos || n.find("roe") != std::string::npos || n.find("nest") != std::string::npos) {
				return false;
			}
			for (const char* k : { "salmon", "fish", "bass", "trout", "pike", "carp", "eel", "perch", "cod", "herring", "tuna", "pickerel", "sturgeon" }) {
				if (n.find(k) != std::string::npos) {
					return true;
				}
			}
			return false;
		}

		bool IsSalmon(const char* a_name)
		{
			if (!a_name) {
				return false;
			}
			std::string n = a_name;
			std::ranges::transform(n, n.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return n.find("salmon") != std::string::npos;
		}

		bool IsFishRef(RE::TESObjectREFR* a_ref)
		{
			const auto* base = a_ref ? a_ref->GetBaseObject() : nullptr;
			const auto* produce = ProduceOf(a_ref);
			return (base && NameHasFish(base->GetName())) || (produce && produce->produceItem && NameHasFish(produce->produceItem->GetName()));
		}

		bool IsBear(const std::string& a_species) { return a_species.find("bear") != std::string::npos; }

		// a_fish: only fish, salmon first (the bear's job); otherwise only things that are not fish
		std::vector<RE::FormID> FindPlants(RE::Actor* a_pet, float a_range, int a_max, bool a_fish)
		{
			std::vector<std::pair<float, RE::FormID>> found;
			int                                       withProduce = 0;  // refs that could give produce, whatever their state (for the log when nothing is found)
			if (auto* tes = RE::TES::GetSingleton()) {
				tes->ForEachReferenceInRange(a_pet, a_range, [&](RE::TESObjectREFR* a_ref) {
					if (const auto* p = ProduceOf(a_ref); p && p->produceItem) {
						++withProduce;
					}
					if (IsGatherable(a_ref) && IsFishRef(a_ref) == a_fish) {
						float d = a_ref->GetPosition().GetDistance(a_pet->GetPosition());
						if (a_fish && IsSalmon(a_ref->GetBaseObject() ? a_ref->GetBaseObject()->GetName() : nullptr)) {
							d -= 100000.0f;  // salmon first, whatever the distance
						}
						found.emplace_back(d, a_ref->GetFormID());
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});
			}
			if (found.empty()) {
				SKSE::log::info("jobs: nothing to {} in range; {} references with produce were seen (harvested, disabled or not loaded ones do not count)", a_fish ? "fish" : "gather", withProduce);
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
			// the produce can be a leveled list or arrive a moment later: log what really changed in the whole inventory, now and 1.5 s on
			{
				std::map<RE::FormID, std::int32_t> snap;
				for (const auto& [obj, n] : before) {
					snap[obj->GetFormID()] = n;
				}
				const auto petId = a_pet->GetFormID();
				const auto plantId = a_plant->GetFormID();
				const auto produceId = flora->produceItem->GetFormID();
				const auto produceType = flora->produceItem->GetFormType();
				std::thread([snap, petId, plantId, produceId, produceType]() {
					std::this_thread::sleep_for(std::chrono::milliseconds(1500));
					SKSE::GetTaskInterface()->AddTask([snap, petId, plantId, produceId, produceType]() {
						auto* pet = RE::TESForm::LookupByID<RE::Actor>(petId);
						if (!pet) {
							return;
						}
						std::string gained;
						for (const auto& [obj, n] : pet->GetInventoryCounts()) {
							const auto it = snap.find(obj->GetFormID());
							const int  was = it != snap.end() ? it->second : 0;
							if (n > was) {
								gained += std::format(" {}x {} [{:08X}]", n - was, obj->GetName() ? obj->GetName() : "?", obj->GetFormID());
							}
						}
						SKSE::log::info("jobs: 1.5 s after picking {:08X} (produce form {:08X}, type {}) the pet gained:{}", plantId, produceId, static_cast<int>(produceType), gained.empty() ? " nothing" : gained);
					});
				}).detach();
			}
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
			Note(std::format("{} is back{}{}.", j.name, j.kind == Kind::Hunt ? " from the hunt" : "", j.picked ? std::format(" with {} item(s)", j.picked) : std::string(" empty-handed")));
		}

		// ------------------------------------------------------------------ Hunt

			std::string Lower(std::string s)
			{
				std::ranges::transform(s, s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
				return s;
			}

			// the largest prey (health) this species hunts: wolves small game, big cats small to medium, bears any (up to the cap); 0 = does not hunt
			float HuntHealthCap(const std::string& a_species)
			{
				const auto& c = Config::GetJobs();
				if (a_species.find("wolf") != std::string::npos) {
					return c.huntHealthWolf;
				}
				if (a_species.find("sabre cat") != std::string::npos) {
					return c.huntHealthCat;
				}
				if (IsBear(a_species)) {
					return c.huntHealthBear;
				}
				return 0.0f;
			}

			// Why an animal is NOT prey for this pet: "" = it is prey, "-" = not a living animal at all (not worth a log line).
			// Prey = an unaggressive wild animal nobody owns (no crime faction, no owner), not unique / essential / tamed, within the size cap.
			std::string PreyVerdict(RE::Actor* a_actor, RE::Actor* a_pet, float a_cap, float* a_health)
			{
				if (!a_actor || a_actor == a_pet || a_actor->IsDead() || a_actor->IsDisabled() || a_actor->IsDeleted() || !a_actor->Is3DLoaded()) {
					return "-";
				}
				const auto* race = a_actor->GetRace();
				if (!race || !race->HasKeywordString("ActorTypeAnimal"sv)) {
					return "-";
				}
				if (a_actor->IsPlayerTeammate()) {
					return "a follower or pet";
				}
				if (a_actor->IsEssential() || a_actor->IsProtected()) {
					return "essential / protected";
				}
				if (a_actor->IsGhost()) {
					return "a ghost";
				}
				if (a_actor->GetCrimeFaction()) {
					return "has a crime faction (livestock / village animal)";
				}
				if (a_actor->GetOwner()) {
					return "has an owner";
				}
				const auto* base = a_actor->GetActorBase();
				if (!base || base->IsUnique() || !Pets::ClassOf(a_actor).empty()) {
					return "unique or a tamed pet";
				}
				auto*       av = a_actor->AsActorValueOwner();
				const float aggression = av->GetActorValue(RE::ActorValue::kAggression);
				if (aggression >= 1.0f) {
					return std::format("aggressive ({:.0f}), a predator", aggression);
				}
				const auto rn = Lower(race->GetName() ? race->GetName() : "");
				for (const char* k : { "horse", "dog", "mammoth", "giant" }) {
					if (rn.find(k) != std::string::npos) {
						return std::format("race '{}' is never hunted", race->GetName());
					}
				}
				const float hp = av->GetPermanentActorValue(RE::ActorValue::kHealth);
				if (a_health) {
					*a_health = hp;
				}
				if (hp < 1.0f || hp > a_cap) {
					return std::format("health {:.0f} is over the size cap {:.0f}", hp, a_cap);
				}
				return {};
			}

			// prey within reach, nearest first
			std::vector<RE::FormID> FindPrey(RE::Actor* a_pet, float a_cap, const std::vector<RE::FormID>& a_tried)
			{
				std::vector<std::pair<float, RE::FormID>> found;
				int                                       animals = 0;
				if (auto* tes = RE::TES::GetSingleton()) {
					tes->ForEachReferenceInRange(a_pet, Config::GetJobs().huntRange, [&](RE::TESObjectREFR* a_ref) {
						auto* actor = a_ref ? a_ref->As<RE::Actor>() : nullptr;
						if (actor && actor != a_pet && !actor->IsDead()) {
							++animals;
						}
						float       hp = 0.0f;
						const auto  why = PreyVerdict(actor, a_pet, a_cap, &hp);
						if (!why.empty() && why != "-") {
							SKSE::log::info("hunt: skip {} [{:08X}] ({}): {}", actor->GetName() ? actor->GetName() : "?", actor->GetFormID(), actor->GetRace() && actor->GetRace()->GetName() ? actor->GetRace()->GetName() : "?", why);
						}
						if (why.empty() && std::ranges::find(a_tried, actor->GetFormID()) == a_tried.end()) {
							const float d = actor->GetPosition().GetDistance(a_pet->GetPosition());
							found.emplace_back(d, actor->GetFormID());
							SKSE::log::info("hunt: candidate {} [{:08X}] health {:.0f} (cap {:.0f}), {:.0f} units away", actor->GetName() ? actor->GetName() : "?", actor->GetFormID(), hp, a_cap, d);
						}
						return RE::BSContainer::ForEachResult::kContinue;
					});
				}
				if (found.empty()) {
					SKSE::log::info("hunt: no prey within {:.0f} units ({} living actors seen, size cap {:.0f})", Config::GetJobs().huntRange, animals, a_cap);
				}
				std::ranges::sort(found);
				std::vector<RE::FormID> out;
				for (const auto& f : found) {
					out.push_back(f.second);
				}
				return out;
			}

			// the pet takes what the carcass holds (not worn skins, weapons or quest items), up to a share of its carry weight; returns how many items moved
			int TakeCarcass(RE::Actor* a_pet, RE::Actor* a_prey, std::string& a_what)
			{
				float room = std::max(a_pet->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCarryWeight), 100.0f) * Config::GetJobs().huntLootFraction;
				int   moved = 0;
				auto  inv = a_prey->GetInventory([](RE::TESBoundObject& o) { return !o.IsArmor() && !o.IsWeapon(); });
				for (auto& [obj, entry] : inv) {
					const auto  count = entry.first;
					const auto* data = entry.second.get();
					if (!obj || count <= 0 || (data && (data->IsWorn() || data->IsQuestObject()))) {
						continue;
					}
					int         n = count;
					const float w = obj->GetWeight();
					if (w > 0.0f) {
						n = std::min(n, static_cast<int>(room / w));
					}
					if (n <= 0) {
						continue;
					}
					a_prey->RemoveItem(obj, n, RE::ITEM_REMOVE_REASON::kRemove, nullptr, a_pet);
					room -= w * static_cast<float>(n);
					moved += n;
					a_what += std::format(" {}x {}", n, obj->GetName() ? obj->GetName() : "?");
				}
				return moved;
			}

			// picks the nearest untried prey and sets the pet on it; false when there is none
			bool NextPrey(Job& j, RE::Actor* a_pet)
			{
				const auto prey = FindPrey(a_pet, HuntHealthCap(Pets::SpeciesNameOf(a_pet)), j.tried);
				if (prey.empty()) {
					return false;
				}
				j.prey = prey.front();
				j.tried.push_back(j.prey);
				j.preyStart = j.lastEngage = std::chrono::steady_clock::now();
				auto*      target = RE::TESForm::LookupByID<RE::Actor>(j.prey);
				const bool ok = target && a_pet->StartCombat(target);
				SKSE::log::info("hunt: {} [{:08X}] goes after {} [{:08X}] (StartCombat {})", j.name, j.pet, target && target->GetName() ? target->GetName() : "?", j.prey, ok);
				return true;
			}

			// stops the fight and walks the pet back to the player; true when the trip is over for good
			bool GoHome(Job& j, RE::Actor* a_pet, const char* a_why)
			{
				a_pet->StopCombat();
				j.prey = 0;
				if (j.travel) {
					g_api->Release(j.travel);
					j.travel = 0;
				}
				j.travel = WalkTo(j.pet, 0x14);
				j.phase = Phase::Returning;
				j.legStart = std::chrono::steady_clock::now();
				SKSE::log::info("hunt: {} [{:08X}] heads back ({}), {} kill(s), {} item(s) taken", j.name, j.pet, a_why, j.kills, j.picked);
				if (!j.travel) {
					Finish(j, "could not walk back");
					return true;
				}
				return false;
			}

			bool HuntStep(Job& j, RE::Actor* a_pet)
			{
				const auto& cfg = Config::GetJobs();
				const auto  now = std::chrono::steady_clock::now();
				auto*       prey = j.prey ? RE::TESForm::LookupByID<RE::Actor>(j.prey) : nullptr;

				if (prey && prey->IsDead()) {
					std::string what;
					const int   n = TakeCarcass(a_pet, prey, what);
					j.picked += n;
					++j.kills;
					SKSE::log::info("hunt: {} killed {} [{:08X}], took {} item(s):{}", j.name, prey->GetName() ? prey->GetName() : "?", j.prey, n, what.empty() ? " nothing" : what);
					Note(std::format("{} brings down {}.", j.name, prey->GetName() ? prey->GetName() : "an animal"));
					a_pet->StopCombat();
					j.prey = 0;
					if (j.kills < cfg.huntKills && NextPrey(j, a_pet)) {
						return false;
					}
					return GoHome(j, a_pet, "hunt done");
				}
				const auto secs = std::chrono::duration_cast<std::chrono::seconds>(now - j.preyStart).count();
				if (!j.prey || !prey || secs > cfg.huntSecondsPerPrey) {
					if (j.prey) {
						SKSE::log::info("hunt: {} gave up on [{:08X}] ({}, {}s)", j.name, j.prey, prey ? "too slow" : "no longer loaded", secs);
					}
					a_pet->StopCombat();
					j.prey = 0;
					if (NextPrey(j, a_pet)) {
						return false;
					}
					return GoHome(j, a_pet, j.kills ? "no more prey" : "nothing caught");
				}
				// the engine drops the fight when the animal runs out of sight: tell the pet again
				if (!a_pet->IsInCombat() && std::chrono::duration_cast<std::chrono::seconds>(now - j.lastEngage).count() >= 3) {
					j.lastEngage = now;
					a_pet->StartCombat(prey);
				}
				return false;
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
			if (j.kind == Kind::Hunt && j.phase == Phase::Hunting) {
					return HuntStep(j, pet);
				}
				// a hunter has only just left the fight on the way home: give the engine a few seconds to drop it
				if (pet->IsInCombat() && !(j.kind == Kind::Hunt && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - j.legStart).count() < 8)) {
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
					SKSE::log::info("jobs: {} gave up on plant {} [{:08X}] (state {}, {}s, {:.0f} units away, blocker kind {}, plant {})", j.name, j.next, j.next < j.plants.size() ? j.plants[j.next] : 0u, state, secs,
						plant ? pet->GetPosition().GetDistance(plant->GetPosition()) : -1.0f, info.blockerKind, plant ? (plant->IsDisabled() ? "disabled" : "enabled") : "not loaded");
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
			const bool fishing = IsBear(Pets::SpeciesNameOf(pet));
			j.plants = FindPlants(pet, cfg.range, cfg.maxItems, fishing);
			if (j.plants.empty()) {
				Note(name + (fishing ? " finds no fish nearby." : " finds nothing to gather nearby."));
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
			Note(std::format("{} sets out to {}.", name, fishing ? "fish" : "gather"));
			g_jobs.push_back(std::move(j));
			StartTicker();
			return true;
		}

		// sends one fighter pet out to hunt; returns true if it set out
			bool StartHunt(RE::Actor* pet)
			{
				const std::string name = pet->GetName() ? pet->GetName() : "Your pet";
				if (std::ranges::any_of(g_jobs, [&](const Job& j) { return j.pet == pet->GetFormID(); })) {
					return false;
				}
				if (pet->IsInCombat() || pet->IsDead()) {
					return false;
				}
				Job j;
				j.pet = pet->GetFormID();
				j.name = name;
				j.kind = Kind::Hunt;
				j.phase = Phase::Hunting;
				if (!NextPrey(j, pet)) {
					Note(name + " finds no game nearby.");
					return false;
				}
				SKSE::log::info("hunt: {} [{:08X}] ({}) sets out, up to {} kill(s)", name, j.pet, Pets::SpeciesNameOf(pet), Config::GetJobs().huntKills);
				Note(std::format("{} sets out to hunt.", name));
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
				if (!CanGather(actor.get())) {
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

	bool Gather(RE::Actor* a_pet)
	{
		if (!a_pet || !Connect()) {
			return false;
		}
		const std::string cls = Pets::ClassOf(a_pet);
		if (!CanGather(a_pet)) {
			Note(std::string(a_pet->GetName() ? a_pet->GetName() : "Your pet") + " does not gather.");
			return false;
		}
		return StartFor(a_pet, cls);
	}

	bool CanGather(RE::Actor* a_pet)
	{
		const std::string cls = Pets::ClassOf(a_pet);
		return cls == "gatherer" || cls == "scavenger" || IsBear(Pets::SpeciesNameOf(a_pet));
	}

	const char* GatherLabel(RE::Actor* a_pet) { return IsBear(Pets::SpeciesNameOf(a_pet)) ? "Go fishing" : "Gather what you can find"; }

	bool CanHunt(RE::Actor* a_pet) { return a_pet && Pets::ClassOf(a_pet) == "fighter" && HuntHealthCap(Pets::SpeciesNameOf(a_pet)) > 0.0f; }

	bool Hunt(RE::Actor* a_pet)
	{
		if (!a_pet || !Connect()) {
			return false;
		}
		if (!CanHunt(a_pet)) {
			Note(std::string(a_pet->GetName() ? a_pet->GetName() : "Your pet") + " does not hunt.");
			return false;
		}
		return StartHunt(a_pet);
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

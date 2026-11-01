/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Subdue.h"
#include "Config.h"

#include <random>

namespace Subdue
{
	namespace
	{
		constexpr auto kInstincts = "Instincts - Animal Taming in Skyrim.esp"sv;
		constexpr auto kOwn = "BeastHarmony.esp"sv;

		struct Forms
		{
			RE::SpellItem*      spell = nullptr;  // BH_Subdued
			RE::EffectSetting*  paralysis = nullptr;
			RE::TESFaction*     rolled = nullptr;  // BH_SubdueRolled: this animal was already decided
			RE::BGSListForm*    originalRaces = nullptr;
			RE::BGSListForm*    powers = nullptr;
			RE::TESGlobal*      petCount = nullptr;
			bool                ok = false;
		} F;

		bool tamingMode = false;

		enum class Weapon { None, Fists, Blunt, Bladed };

		const char* WeaponName(Weapon w)
		{
			switch (w) {
			case Weapon::Fists: return "fists";
			case Weapon::Blunt: return "blunt weapon";
			case Weapon::Bladed: return "bladed weapon or arrow";
			default: return "no weapon";
			}
		}

		Weapon Classify(const RE::TESHitEvent* e)
		{
			auto* src = RE::TESForm::LookupByID(e->source);
			auto* w = src ? src->As<RE::TESObjectWEAP>() : nullptr;
			if (!w) {
				return Weapon::None;  // a spell, a trap, an explosion: no knock-out
			}
			if (e->flags.any(RE::TESHitEvent::Flag::kBashAttack)) {
				return Weapon::Blunt;
			}
			if (w->IsHandToHandMelee()) {
				return Weapon::Fists;
			}
			if (w->IsStaff() || w->GetWeaponType() == RE::WEAPON_TYPE::kOneHandMace || w->HasKeywordString("WeapTypeWarhammer"sv)) {
				return Weapon::Blunt;
			}
			return Weapon::Bladed;
		}

		bool ModeOn()
		{
			const auto& c = Config::Get();
			if (!c.enabled || c.mode == "never") {
				return false;
			}
			return c.mode == "always" || tamingMode;
		}

		// Instincts' power of an animal (index of its race in the original race list), or -1 when it is not one of the animals Instincts can tame
		int PowerOf(const RE::Actor* a_actor)
		{
			if (!F.originalRaces || !F.powers) {
				return -1;
			}
			const auto* race = a_actor->GetRace();
			if (!race) {
				return -1;
			}
			const auto& forms = F.originalRaces->forms;
			for (std::uint32_t i = 0; i < forms.size(); ++i) {
				if (forms[i] == race) {
					if (i < F.powers->forms.size()) {
						if (const auto* g = F.powers->forms[i] ? F.powers->forms[i]->As<RE::TESGlobal>() : nullptr) {
							return static_cast<int>(g->value);
						}
					}
					return -1;
				}
			}
			return -1;
		}

		int LevelNeeded(int a_power)
		{
			const auto& c = Config::Get();
			return a_power >= 10 ? c.levelHuge : a_power >= 8 ? c.levelGreat : a_power >= 6 ? c.levelBig : 0;
		}

		int SlotCost(int a_power) { return a_power >= 10 ? 3 : a_power >= 6 ? 2 : 1; }

		std::mt19937& Rng()
		{
			static std::mt19937 rng{ std::random_device{}() };
			return rng;
		}

		void Note(const std::string& a_text) { RE::SendHUDMessage::ShowHUDMessage(a_text.c_str()); }

		float MaxHealth(RE::Actor* a_actor) { return a_actor->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kHealth); }

		// runs on the game thread, after the hit has been applied
		void Decide(RE::FormID a_id, Weapon a_weapon)
		{
			auto* t = RE::TESForm::LookupByID<RE::Actor>(a_id);
			auto* player = RE::PlayerCharacter::GetSingleton();
			if (!t || !player || t->IsDead() || t->IsPlayerTeammate()) {
				return;
			}
			const float hp = t->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
			const float maxHp = MaxHealth(t);
			if (maxHp <= 0.0f) {
				return;
			}
			const auto& c = Config::Get();
			const bool  subdued = t->HasMagicEffect(F.paralysis);
			if (subdued) {
				// a knocked-out animal cannot be finished off by stray hits: keep it at the knock-out health
				const float floor = maxHp * c.threshold;
				if (hp < floor && hp > 0.0f) {
					t->AsActorValueOwner()->RestoreActorValue(RE::ActorValue::kHealth, floor - hp);
				}
				return;
			}
			if (hp <= 0.0f || hp / maxHp > c.threshold || t->IsInFaction(F.rolled)) {
				return;  // not low enough yet, or already decided
			}
			const int power = PowerOf(t);
			if (power < 0) {
				return;  // not an animal Instincts can tame
			}
			t->AddToFaction(F.rolled, 0);  // decided once, whatever the outcome
			const std::string name = t->GetName() ? t->GetName() : "animal";
			const int         need = LevelNeeded(power);
			if (static_cast<int>(player->GetLevel()) < need) {
				SKSE::log::info("{}: no knock-out, player level {} < {} needed (power {})", name, player->GetLevel(), need, power);
				Note("You are not experienced enough to tame a " + name + " (level " + std::to_string(need) + ").");
				return;
			}
			const int used = F.petCount ? static_cast<int>(F.petCount->value) : 0;
			if (used + SlotCost(power) > c.slotLimit) {
				SKSE::log::info("{}: no knock-out, pet slots {} + {} > {}", name, used, SlotCost(power), c.slotLimit);
				Note("You have no room for a " + name + " (" + std::to_string(SlotCost(power)) + " pet slot(s) needed).");
				return;
			}
			const float chance = a_weapon == Weapon::Fists ? c.chanceFists : a_weapon == Weapon::Blunt ? c.chanceBlunt : c.chanceBladed;
			const float roll = std::uniform_real_distribution<float>(0.0f, 1.0f)(Rng());
			const bool  knockedOut = roll < chance;
			SKSE::log::info("{}: {:.0f}% health after a hit with {} -> chance {:.0f}%, rolled {:.2f}: {}", name, 100.0f * hp / maxHp, WeaponName(a_weapon), 100.0f * chance, roll,
				knockedOut ? "knocked out" : "killed");
			if (knockedOut) {
				t->StopCombat();
				if (auto* caster = t->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)) {
					caster->CastSpellImmediate(F.spell, false, t, 1.0f, false, 0.0f, nullptr);
				}
				Note("The " + name + " is knocked out.");
			} else {
				t->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, hp + 1.0f);
				Note("The " + name + " dies of the blow.");
			}
		}

		struct HitSink : RE::BSTEventSink<RE::TESHitEvent>
		{
			RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event, RE::BSTEventSource<RE::TESHitEvent>*) override
			{
				if (!a_event || !a_event->target || !a_event->cause || !F.ok) {
					return RE::BSEventNotifyControl::kContinue;
				}
				auto* target = a_event->target->As<RE::Actor>();
				if (!target || a_event->cause.get() != RE::PlayerCharacter::GetSingleton()) {
					// anything may hit a knocked-out animal: the floor applies to every hit
					if (target && F.paralysis && target->HasMagicEffect(F.paralysis)) {
						const auto id = target->GetFormID();
						SKSE::GetTaskInterface()->AddTask([id]() { Decide(id, Weapon::Bladed); });
					}
					return RE::BSEventNotifyControl::kContinue;
				}
				const bool subdued = F.paralysis && target->HasMagicEffect(F.paralysis);
				if (!subdued && !ModeOn()) {
					return RE::BSEventNotifyControl::kContinue;
				}
				const auto w = Classify(a_event);
				if (w == Weapon::None && !subdued) {
					return RE::BSEventNotifyControl::kContinue;
				}
				const auto id = target->GetFormID();
				SKSE::GetTaskInterface()->AddTask([id, w]() { Decide(id, w); });
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		struct KeySink : RE::BSTEventSink<RE::InputEvent*>
		{
			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_events, RE::BSTEventSource<RE::InputEvent*>*) override
			{
				const auto& c = Config::Get();
				if (!a_events || c.mode != "hotkey" || !c.enabled || c.toggleKey <= 0) {
					return RE::BSEventNotifyControl::kContinue;
				}
				for (auto* e = *a_events; e; e = e->next) {
					const auto* b = e->AsButtonEvent();
					if (b && b->IsDown() && b->GetDevice() == RE::INPUT_DEVICE::kKeyboard && static_cast<int>(b->GetIDCode()) == c.toggleKey) {
						auto* ui = RE::UI::GetSingleton();
						if (ui && !ui->GameIsPaused()) {
							tamingMode = !tamingMode;
							Note(tamingMode ? "Taming mode: ON. Knock animals out instead of killing them." : "Taming mode: OFF.");
							SKSE::log::info("Taming mode {}", tamingMode ? "on" : "off");
						}
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};

		HitSink hitSink;
		KeySink keySink;
	}

	void Init()
	{
		auto* dh = RE::TESDataHandler::GetSingleton();
		if (!dh) {
			return;
		}
		F.spell = dh->LookupForm<RE::SpellItem>(0x803, kOwn);
		F.paralysis = dh->LookupForm<RE::EffectSetting>(0x801, kOwn);
		F.rolled = dh->LookupForm<RE::TESFaction>(0x800, kOwn);
		F.originalRaces = dh->LookupForm<RE::BGSListForm>(0x023FD8, kInstincts);
		F.powers = dh->LookupForm<RE::BGSListForm>(0x0519C2, kInstincts);
		F.petCount = dh->LookupForm<RE::TESGlobal>(0x00FB4F, kInstincts);
		F.ok = F.spell && F.paralysis && F.rolled && F.originalRaces && F.powers;
		SKSE::log::info("Beast Harmony: BeastHarmony.esp {}, Instincts lists {}", (F.spell && F.paralysis && F.rolled) ? "found" : "NOT FOUND", (F.originalRaces && F.powers) ? "found" : "NOT FOUND");
		if (!F.ok) {
			SKSE::log::warn("the knock-out rule is off (needs BeastHarmony.esp and Instincts - Animal Taming in Skyrim.esp)");
			return;
		}
		RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESHitEvent>(&hitSink);
		if (auto* input = RE::BSInputDeviceManager::GetSingleton()) {
			input->AddEventSink(&keySink);
		}
		const auto& c = Config::Get();
		SKSE::log::info("knock-out rule on: mode {}, key {}, threshold {:.0f}%, chances fists/blunt/bladed {:.0f}/{:.0f}/{:.0f}%", c.mode, c.toggleKey, c.threshold * 100.0f, c.chanceFists * 100.0f,
			c.chanceBlunt * 100.0f, c.chanceBladed * 100.0f);
	}
}

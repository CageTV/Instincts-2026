/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "PetMenu.h"
#include "Jobs.h"
#include "Pets.h"
#include "Config.h"

#include "SkyPrompt/API.hpp"

#include <format>

namespace PetMenu
{
	namespace
	{
		enum class Action { Gather, Favor, Nothing };

		// the game's own "do a favor" mode of the Instincts pets (what the old "Do something for me" entry switched on)
		void StartFavor(RE::Actor* a_pet)
		{
			auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			if (!vm || !a_pet) {
				return;
			}
			auto* policy = vm->GetObjectHandlePolicy();
			if (!policy) {
				return;
			}
			const auto                                          handle = policy->GetHandleForObject(static_cast<std::uint32_t>(a_pet->GetFormType()), a_pet);
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> none;
			auto* args = RE::MakeFunctionArguments(true);
			vm->DispatchMethodCall(handle, "Actor", "SetDoingFavor", args, none);
		}

		// carries out a chosen command on the game thread
		void Perform(Action a_act, RE::FormID a_id)
		{
			SKSE::GetTaskInterface()->AddTask([a_act, a_id]() {
				auto* actor = RE::TESForm::LookupByID<RE::Actor>(a_id);
				if (!actor) {
					return;
				}
				switch (a_act) {
				case Action::Gather:
					Jobs::Gather(actor);
					break;
				case Action::Favor:
					StartFavor(actor);
					break;
				default:
					break;
				}
			});
		}

		std::vector<Action> ActionsFor(RE::Actor* a_pet)
		{
			std::vector<Action> out;
			const std::string   cls = Pets::ClassOf(a_pet);
			if (cls == "gatherer" || cls == "scavenger") {
				out.push_back(Action::Gather);
			}
			out.push_back(Action::Favor);
			out.push_back(Action::Nothing);
			return out;
		}

		const char* Label(Action a_act)
		{
			switch (a_act) {
			case Action::Gather:
				return "Gather what you can find";
			case Action::Favor:
				return "Something else";
			default:
				return "Never mind";
			}
		}

		// ---- SkyPrompt ----
		struct PromptSink : SkyPromptAPI::PromptSink
		{
			SkyPromptAPI::ClientID                                                     client = 0;
			RE::FormID                                                                 pet = 0;
			std::uint32_t                                                              serial = 0;  // which showing this is (timeouts of an older one do nothing)
			std::vector<Action>                                                        actions;
			std::vector<std::string>                                                   texts;
			std::vector<std::vector<std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>>> keys;
			std::vector<SkyPromptAPI::Prompt>                                          prompts;

			void ProcessEvent(SkyPromptAPI::PromptEvent a_event) const override
			{
				if (a_event.type != SkyPromptAPI::PromptEventType::kAccepted) {
					return;
				}
				const auto idx = a_event.prompt.actionID;
				const auto act = idx < actions.size() ? actions[idx] : Action::Nothing;
				const auto id = pet;
				SKSE::log::info("menu: prompt {} accepted for pet {:08X}", idx, id);
				SKSE::GetTaskInterface()->AddTask([act, id]() {
					if (Get().prompts.empty()) {
						return;  // the key sink below already handled this press
					}
					Hide();
					Perform(act, id);
				});
			}

			std::span<const SkyPromptAPI::Prompt> GetPrompts() const override { return prompts; }

			static PromptSink& Get()
			{
				static PromptSink s;
				return s;
			}

			static void Hide()
			{
				auto& me = Get();
				if (me.client) {
					SkyPromptAPI::RemovePrompt(&me, me.client);
				}
				me.prompts.clear();
				++me.serial;
			}
		};

		// Fallback: SkyPrompt did not report presses of some keys (F2-F4 in the 2026-11-05 test), so the shown prompts are also read straight from the keyboard.
		struct PromptKeySink : RE::BSTEventSink<RE::InputEvent*>
		{
			RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_events, RE::BSTEventSource<RE::InputEvent*>*) override
			{
				auto& me = PromptSink::Get();
				if (!a_events || me.prompts.empty()) {
					return RE::BSEventNotifyControl::kContinue;
				}
				for (auto* e = *a_events; e; e = e->next) {
					const auto* b = e->AsButtonEvent();
					if (!b || !b->IsDown() || b->GetDevice() != RE::INPUT_DEVICE::kKeyboard) {
						continue;
					}
					const auto code = b->GetIDCode();
					for (std::size_t i = 0; i < me.keys.size() && i < me.actions.size(); ++i) {
						if (!me.keys[i].empty() && me.keys[i][0].second == code) {
							const auto act = me.actions[i];
							const auto id  = me.pet;
							SKSE::log::info("menu: key {:#x} pressed, action {} for pet {:08X} (read from the keyboard)", code, i, id);
							SKSE::GetTaskInterface()->AddTask([act, id]() {
								if (PromptSink::Get().prompts.empty()) {
									return;
								}
								PromptSink::Hide();
								Perform(act, id);
							});
							return RE::BSEventNotifyControl::kContinue;
						}
					}
				}
				return RE::BSEventNotifyControl::kContinue;
			}
		};
		PromptKeySink promptKeySink;

		bool ShowPrompts(RE::FormID a_pet)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			if (!pet) {
				return false;
			}
			auto& me = PromptSink::Get();
			if (!me.client) {
				me.client = SkyPromptAPI::RequestClientID();
				if (!me.client) {
					SKSE::log::warn("menu: SkyPrompt gave no client id (not installed or an incompatible version), using a message box");
					return false;
				}
				SKSE::log::info("menu: SkyPrompt client id {}", me.client);
			}
			PromptSink::Hide();
			static bool keySinkAdded = false;
			if (!keySinkAdded) {
				if (auto* idm = RE::BSInputDeviceManager::GetSingleton()) {
					idm->AddEventSink(&promptKeySink);
					keySinkAdded = true;
				}
			}
			const auto& cfg = Config::GetMenu();
			me.pet = a_pet;
			me.actions = ActionsFor(pet);
			me.texts.clear();
			me.keys.clear();
			me.prompts.clear();
			const int scan[3] = { cfg.keyGather, cfg.keyOther, cfg.keyCancel };
			for (std::size_t i = 0; i < me.actions.size(); ++i) {
				// the last entry is always "Never mind", the first Gather when it exists
				const int code = me.actions[i] == Action::Gather ? scan[0] : (me.actions[i] == Action::Favor ? scan[1] : scan[2]);
				me.texts.push_back(Label(me.actions[i]));
				me.keys.push_back({ { RE::INPUT_DEVICE::kKeyboard, static_cast<SkyPromptAPI::ButtonID>(code) } });
			}
			for (std::size_t i = 0; i < me.actions.size(); ++i) {
				me.prompts.emplace_back(me.texts[i], static_cast<SkyPromptAPI::EventID>(1), static_cast<SkyPromptAPI::ActionID>(i), SkyPromptAPI::PromptType::kSinglePress, a_pet,
					std::span<const std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>>(me.keys[i]));
			}
			if (!SkyPromptAPI::SendPrompt(&me, me.client)) {
				SKSE::log::warn("menu: SkyPrompt refused the prompts, using a message box");
				me.prompts.clear();
				return false;
			}
			const auto ser = ++me.serial;
			const auto secs = cfg.seconds;
			std::thread([ser, secs]() {
				std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(secs * 1000.0f)));
				SKSE::GetTaskInterface()->AddTask([ser]() {
					if (PromptSink::Get().serial == ser && !PromptSink::Get().prompts.empty()) {
						SKSE::log::info("menu: prompts timed out");
						PromptSink::Hide();
					}
				});
			}).detach();
			SKSE::log::info("menu: {} SkyPrompt prompts shown for {} [{:08X}] (keys {:#x} {:#x} {:#x}, {:.0f} s)", me.actions.size(), pet->GetName() ? pet->GetName() : "pet", a_pet,
				cfg.keyGather, cfg.keyOther, cfg.keyCancel, secs);
			return true;
		}

		struct Choice : RE::IMessageBoxCallback
		{
			RE::FormID          pet = 0;
			std::vector<Action> actions;

			void Run(std::uint8_t a_button) override
			{
				if (a_button >= actions.size()) {
					return;
				}
				Perform(actions[a_button], pet);
			}
		};

		void ShowBox(RE::FormID a_pet)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			auto* queue = RE::UIMessageQueue::GetSingleton();
			if (!pet || !queue) {
				SKSE::log::warn("menu: cannot show the list (pet {:08X} found {}, message queue found {})", a_pet, pet != nullptr, queue != nullptr);
				return;
			}
			const std::string cls = Pets::ClassOf(pet);
			const std::string name = pet->GetName() ? pet->GetName() : "Your pet";

			auto* data = RE::UIMessageDataFactory::Create<RE::MessageBoxData>();
			if (!data) {
				SKSE::log::warn("menu: the game gave no message box data");
				return;
			}
			auto* cb = new Choice();
			cb->pet = a_pet;
			const auto add = [&](const char* a_label, Action a_action) {
				data->buttonText.push_back(a_label);
				cb->actions.push_back(a_action);
			};
			if (cls == "gatherer" || cls == "scavenger") {
				add("Gather what you can find", Action::Gather);
			}
			add("Something else", Action::Favor);
			add("Never mind", Action::Nothing);

			data->bodyText = std::format("What should {} do?", name).c_str();
			data->callback = RE::BSTSmartPointer<RE::IMessageBoxCallback>(cb);
			data->cancelButtonIndex = static_cast<std::int32_t>(cb->actions.size()) - 1;
			data->isCancellable = true;
			queue->AddMessage(RE::MessageBoxMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, data);
			SKSE::log::info("menu: list queued for {} [{:08X}] (class '{}'), {} buttons", name, a_pet, cls, cb->actions.size());
		}

		// the dialogue that called Open is still on screen for a moment; the prompts / box are shown once it has closed
		void Show(RE::FormID a_pet, int a_tries)
		{
			auto* ui = RE::UI::GetSingleton();
			if (ui && ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) && a_tries < 40) {
				std::thread([a_pet, a_tries]() {
					std::this_thread::sleep_for(std::chrono::milliseconds(150));
					SKSE::GetTaskInterface()->AddTask([a_pet, a_tries]() { Show(a_pet, a_tries + 1); });
				}).detach();
				return;
			}
			if (a_tries > 0) {
				SKSE::log::info("menu: dialogue closed after {} checks", a_tries);
			}
			if (Config::GetMenu().useSkyPrompt && ShowPrompts(a_pet)) {
				return;
			}
			ShowBox(a_pet);
		}

		void Open(RE::StaticFunctionTag*, RE::Actor* a_pet)
		{
			SKSE::log::info("menu: Open called from Papyrus (pet {:08X})", a_pet ? a_pet->GetFormID() : 0);
			if (!a_pet) {
				return;
			}
			const auto id = a_pet->GetFormID();
			SKSE::GetTaskInterface()->AddTask([id]() { Show(id, 0); });
		}

	}

	bool Bind(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->RegisterFunction("Open", "BeastHarmony_Menu", Open);
		return true;
	}
}

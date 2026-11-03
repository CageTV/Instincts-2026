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

		struct Choice : RE::IMessageBoxCallback
		{
			RE::FormID          pet = 0;
			std::vector<Action> actions;

			void Run(std::uint8_t a_button) override
			{
				if (a_button >= actions.size()) {
					return;
				}
				const auto act = actions[a_button];
				const auto id = pet;
				SKSE::GetTaskInterface()->AddTask([act, id]() {
					auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
					if (!actor) {
						return;
					}
					switch (act) {
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
		};

		void Show(RE::FormID a_pet)
		{
			auto* pet = RE::TESForm::LookupByID<RE::Actor>(a_pet);
			auto* queue = RE::UIMessageQueue::GetSingleton();
			if (!pet || !queue) {
				return;
			}
			const std::string cls = Pets::ClassOf(pet);
			const std::string name = pet->GetName() ? pet->GetName() : "Your pet";

			auto* data = RE::UIMessageDataFactory::Create<RE::MessageBoxData>();
			if (!data) {
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
		}

		void Open(RE::StaticFunctionTag*, RE::Actor* a_pet)
		{
			if (!a_pet) {
				return;
			}
			const auto id = a_pet->GetFormID();
			SKSE::GetTaskInterface()->AddTask([id]() { Show(id); });
		}

		bool Register(RE::BSScript::IVirtualMachine* a_vm)
		{
			a_vm->RegisterFunction("Open", "BeastHarmony_Menu", Open);
			return true;
		}
	}

	void RegisterPapyrus()
	{
		SKSE::GetPapyrusInterface()->Register(Register);
	}
}

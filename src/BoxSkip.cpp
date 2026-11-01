/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "BoxSkip.h"
#include "Config.h"

namespace BoxSkip
{
	namespace
	{
		constexpr auto kLabel = "Attempt to Tame"sv;

		using ProcessMessage_t = RE::UI_MESSAGE_RESULTS (*)(RE::MessageBoxMenu*, RE::UIMessage&);
		std::uintptr_t g_original = 0;

		// run `a_fn` on the game thread after `a_frames` more frames
		void AfterFrames(int a_frames, std::function<void()> a_fn)
		{
			SKSE::GetTaskInterface()->AddTask([a_frames, fn = std::move(a_fn)]() mutable {
				if (a_frames <= 1) {
					fn();
				} else {
					AfterFrames(a_frames - 1, std::move(fn));
				}
			});
		}

		bool IsActivateChoiceCallback(RE::IMessageBoxCallback* a_cb)
		{
			if (!a_cb) {
				return false;
			}
			static const REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE___ActivateChoiceMenuCallbackFunctor[0] };
			return *reinterpret_cast<const std::uintptr_t*>(a_cb) == vtbl.address();
		}

		RE::UI_MESSAGE_RESULTS Thunk(RE::MessageBoxMenu* a_menu, RE::UIMessage& a_message)
		{
			const auto result = reinterpret_cast<ProcessMessage_t>(g_original)(a_menu, a_message);
			try {
				const auto& cfg = Config::GetBox();
				{
					// diagnostics: every message the box menu receives (a handful per box), so a box that is not caught can be explained
					static int logged = 0;
					if (logged < 60) {
						++logged;
						std::string buttons;
						bool        isChoice = false;
						if (a_message.data && a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
							auto* d = static_cast<RE::MessageBoxData*>(a_message.data);
							isChoice = IsActivateChoiceCallback(d->callback.get());
							for (auto& b : d->buttonText) {
								buttons += std::format("[{}]", b.c_str() ? b.c_str() : "?");
							}
						}
						SKSE::log::info("MessageBoxMenu message type {} data {} activateChoiceCallback {} buttons {}", static_cast<std::uint32_t>(a_message.type.get()), a_message.data ? "yes" : "none", isChoice, buttons);
					}
				}
				if (!cfg.skip || a_message.type != RE::UI_MESSAGE_TYPE::kShow || !a_message.data) {
					return result;
				}
				auto* data = static_cast<RE::MessageBoxData*>(a_message.data);
				if (!IsActivateChoiceCallback(data->callback.get()) || data->buttonText.empty()) {
					return result;
				}
				const char* first = data->buttonText[0].c_str();
				if (!first || std::string_view(first) != kLabel) {
					return result;  // some other activation choice box (another mod's perk): leave it alone
				}
				RE::BSTSmartPointer<RE::IMessageBoxCallback> cb = data->callback;
				SKSE::log::info("activation box '{}' ({} buttons): hiding it and running choice 0 in {} frames", first, data->buttonText.size(), cfg.delayFrames);
				if (auto* queue = RE::UIMessageQueue::GetSingleton()) {
					queue->AddMessage(RE::InterfaceStrings::GetSingleton()->messageBoxMenu, RE::UI_MESSAGE_TYPE::kHide, nullptr);
				}
				AfterFrames(std::max(1, cfg.delayFrames), [cb]() {
					SKSE::log::info("activation box: running choice 0 (Attempt to Tame)");
					cb->Run(0);
				});
			} catch (const std::exception& e) {
				SKSE::log::error("activation box hook: {}", e.what());
			}
			return result;
		}
	}

	void Init()
	{
		if (!Config::GetBox().skip) {
			SKSE::log::info("activation box skip: off in config");
			return;
		}
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_MessageBoxMenu[0] };
		g_original = vtbl.write_vfunc(0x4, reinterpret_cast<std::uintptr_t>(&Thunk));
		SKSE::log::info("activation box skip: hooked MessageBoxMenu::ProcessMessage (original {:X})", g_original);
	}
}

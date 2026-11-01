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

		// the pending activation-choice box, read from the engine's message-box queue (the show message itself carries no data)
		RE::BSTSmartPointer<RE::IMessageBoxCallback> FindPendingChoiceBox(std::string& a_firstLabel, std::size_t& a_queueSize)
		{
			RE::BSTSmartPointer<RE::IMessageBoxCallback> found;
			RE::BSSpinLockGuard                            lock(RE::MessageBoxMenu::GetQueueLock());
			auto&                                        queue = RE::MessageBoxMenu::GetQueue();
			a_queueSize = queue.size();
			for (auto* d : queue) {
				if (!d || d->buttonText.empty() || !IsActivateChoiceCallback(d->callback.get())) {
					continue;
				}
				const char* first = d->buttonText[0].c_str();
				a_firstLabel = first ? first : "";
				if (a_firstLabel == kLabel) {
					found = d->callback;
					break;
				}
			}
			return found;
		}

		RE::UI_MESSAGE_RESULTS Thunk(RE::MessageBoxMenu* a_menu, RE::UIMessage& a_message)
		{
			RE::BSTSmartPointer<RE::IMessageBoxCallback> cb;
			const auto&                                  cfg = Config::GetBox();
			try {
				if (cfg.skip && a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
					std::string  label;
					std::size_t  queued = 0;
					cb = FindPendingChoiceBox(label, queued);
					static int logged = 0;
					if (logged < 40) {
						++logged;
						SKSE::log::info("message box shown: {} box(es) queued, activation choice '{}' {}", queued, label, cb ? "FOUND" : "not found");
					}
				}
			} catch (const std::exception& e) {
				SKSE::log::error("activation box hook (find): {}", e.what());
			}
			const auto result = reinterpret_cast<ProcessMessage_t>(g_original)(a_menu, a_message);
			if (cb) {
				try {
					SKSE::log::info("activation box: running choice 0 while the box is still open, then hiding it (in {} frames)", cfg.delayFrames);
					// ORDER MATTERS: the choice list the callback reads exists only while the box is open (hiding the box clears it - running the
					// choice after the hide crashed in the engine, 2026-11-01). So run choice 0 first, then close the box.
					AfterFrames(std::max(1, cfg.delayFrames), [cb]() {
						SKSE::log::info("activation box: running choice 0 (Attempt to Tame)");
						cb->Run(0);
						SKSE::log::info("activation box: choice 0 returned, hiding the box");
						if (auto* queue = RE::UIMessageQueue::GetSingleton()) {
							queue->AddMessage(RE::InterfaceStrings::GetSingleton()->messageBoxMenu, RE::UI_MESSAGE_TYPE::kHide, nullptr);
						}
					});
				} catch (const std::exception& e) {
					SKSE::log::error("activation box hook (run): {}", e.what());
				}
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

/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "BoxSkip.h"
#include "Config.h"
#include "Subdue.h"

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

		// A callback that does nothing. One permanent reference is held here, so the count never reaches zero and the engine never frees
		// (with its own allocator) an object this DLL allocated.
		struct NoopCallback : RE::IMessageBoxCallback
		{
			void Run(std::uint8_t) override {}
		};

		RE::BSTSmartPointer<RE::IMessageBoxCallback>& Noop()
		{
			static RE::BSTSmartPointer<RE::IMessageBoxCallback> keep{ new NoopCallback() };
			return keep;
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
					// The engine ALSO runs the box's callback when the box is closed, and the choice list is gone after our own run: that second run
					// crashed (CTD 15:14 and 15:30, 2026-11-01). Give the box a do-nothing callback so only our run counts.
					d->callback = Noop();
					break;
				}
			}
			return found;
		}

		// The box's second button is the plain "Activate" choice, and the hover-label hook gives it the same text as the first ("Attempt to Tame").
		// Rename it "Cancel": pressing it runs the normal activation, which does nothing to a wild animal, so it works as a way out.
		void RenameSecondButton()
		{
			RE::BSSpinLockGuard lock(RE::MessageBoxMenu::GetQueueLock());
			for (auto* d : RE::MessageBoxMenu::GetQueue()) {
				if (!d || d->buttonText.size() < 2 || !IsActivateChoiceCallback(d->callback.get())) {
					continue;
				}
				const char* first = d->buttonText[0].c_str();
				const char* second = d->buttonText[1].c_str();
				if (first && second && std::string_view(first) == kLabel && std::string_view(second) == kLabel) {
					d->buttonText[1] = "Cancel";
				}
			}
		}

		RE::UI_MESSAGE_RESULTS Thunk(RE::MessageBoxMenu* a_menu, RE::UIMessage& a_message)
		{
			RE::BSTSmartPointer<RE::IMessageBoxCallback> cb;
			const auto&                                  cfg = Config::GetBox();
			try {
				if (!cfg.skip && a_message.type == RE::UI_MESSAGE_TYPE::kShow) {
					RenameSecondButton();
				}
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
		// the hook is always installed: with skip off it only renames the second button to "Cancel"
		if (!Config::GetBox().skip) {
			SKSE::log::info("activation box skip: off in config (box stays open, second button renamed Cancel)");
		}
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_MessageBoxMenu[0] };
		g_original = vtbl.write_vfunc(0x4, reinterpret_cast<std::uintptr_t>(&Thunk));
		SKSE::log::info("activation box skip: hooked MessageBoxMenu::ProcessMessage (original {:X})", g_original);
	}

	namespace
	{
		using GetActivateText_t = bool (*)(RE::TESNPC*, RE::TESObjectREFR*, RE::BSString&);
		std::uintptr_t g_origText = 0;

		bool TextThunk(RE::TESNPC* a_base, RE::TESObjectREFR* a_activator, RE::BSString& a_dst)
		{
			const bool had = reinterpret_cast<GetActivateText_t>(g_origText)(a_base, a_activator, a_dst);
			try {
				// (the first argument is NOT the player pointer - logged 16:53 - so it is not tested; the crosshair target + base identify the animal)
				if (had || !Config::GetBox().label) {
					return had;
				}
				const auto* pick = RE::CrosshairPickData::GetSingleton();
				const auto  target = pick ? pick->GetActiveTarget().get() : nullptr;
				auto*       actor = target ? target->As<RE::Actor>() : nullptr;
				if (!actor || actor->GetActorBase() != a_base || !Subdue::QualifiesForTaming(actor)) {
					return had;
				}
				a_dst = "Attempt to Tame";
				static int logged = 0;
				if (logged < 5) {
					++logged;
					SKSE::log::info("hover label: 'Attempt to Tame' shown for {}", actor->GetName() ? actor->GetName() : "?");
				}
				return true;
			} catch (const std::exception& e) {
				SKSE::log::error("hover label hook: {}", e.what());
				return had;
			}
		}
	}

	void InitLabel()
	{
		if (!Config::GetBox().label) {
			SKSE::log::info("hover label: off in config");
			return;
		}
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_TESNPC[0] };
		g_origText = vtbl.write_vfunc(0x4C, reinterpret_cast<std::uintptr_t>(&TextThunk));
		SKSE::log::info("hover label: hooked TESNPC::GetActivateText (original {:X})", g_origText);
	}
}

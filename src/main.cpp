/*
 * Beast Harmony
 * Copyright (c) 2026 CageTV
 *
 * Released under the MIT License; see LICENSE.txt.
 */
#include "PCH.h"
#include "Config.h"
#include "Subdue.h"
#include "Pets.h"
#include "BoxSkip.h"
#include "Diet.h"
#include "HudFilter.h"

namespace
{
	void SetupLog()
	{
		const auto dir = SKSE::log::log_directory();
		if (!dir) {
			return;
		}
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>((*dir / "BeastHarmony.log").string(), true);
		auto log = std::make_shared<spdlog::logger>("BeastHarmony", std::move(sink));
		log->set_level(spdlog::level::info);
		log->flush_on(spdlog::level::info);
		log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
		spdlog::set_default_logger(std::move(log));
	}

	void OnMessage(SKSE::MessagingInterface::Message* a_msg)
	{
		if (a_msg->type == SKSE::MessagingInterface::kDataLoaded) {
			try {
				Config::Load();
				Subdue::Init();
				Pets::Init();
				BoxSkip::Init();
				BoxSkip::InitLabel();
				Diet::Init();
				HudFilter::Init();
			} catch (const std::exception& e) {
				SKSE::log::error("failed: {}", e.what());
			}
		}
	}
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse);
	SetupLog();
	SKSE::log::info("BeastHarmony 0.1.0, game {}", REL::Module::get().version().string());
	SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
	return true;
}

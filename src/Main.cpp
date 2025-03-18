#include <SKSE/SKSE.h>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <thread>
#include <spdlog/sinks/msvc_sink.h>

#include "DSXController.hpp"
#include "EventHandler.h"

std::vector<DSX::TriggerSetting> userTriggers;
std::vector<DSX::Packet> triggerPackets;
DSX::NetworkManager networkManager;
DSX::Packet lastLeftPacket;
DSX::Packet lastRightPacket;
std::string actionLeft;
std::string actionRight;

namespace
{
	void InitializeDSX();
	void BackgroundThread(std::chrono::milliseconds interval);
	bool LoadTriggerSettings();
	void GeneratePackets();

	void MessageHandler(SKSE::MessagingInterface::Message* a_message)
	{
		switch (a_message->type) {
		case SKSE::MessagingInterface::kDataLoaded:
			logger::info("DataLoaded: Initializing DSX");
			InitializeDSX();
			DSX::RegisterEventHandlers();
			break;
		case SKSE::MessagingInterface::kPostLoadGame:
			logger::info("PostLoadGame: Registering handlers and checking weapons");
			DSX::CheckWeaponOnGameLoad();
			break;
		}
	}

	void InitializeDSX()
	{
		logger::info("Initializing DSXSkyrim");
		if (!networkManager.Initialize()) {
			logger::error("Failed to initialize network connection");
			return;
		}
		if (!LoadTriggerSettings()) {
			logger::error("Failed to load trigger settings");
			return;
		}
		GeneratePackets();

		auto interval = std::chrono::milliseconds(10000);
		std::thread backgroundThread(BackgroundThread, interval);
		backgroundThread.detach();

		logger::info("DSXSkyrim initialized successfully");
	}

	bool LoadTriggerSettings()
	{
		try {
			std::ifstream stream(".\\Data\\SKSE\\Plugins\\DSXSkyrim\\DSXSkyrimConfig.json");
			if (!stream.is_open()) {
				logger::error("Could not open config file");
				return false;
			}
			nlohmann::json j;
			stream >> j;
			userTriggers.clear();

			for (const auto& item : j) {
				DSX::TriggerSetting setting;
				item.at("Name").get_to(setting.name);
				item.at("formID").get_to(setting.formID);  // Updated to match new JSON
				item.at("Category").get_to(setting.category);
				item.at("TriggerSide").get_to(setting.triggerSide);
				item.at("TriggerType").get_to(setting.triggerType);
				item.at("customTriggerMode").get_to(setting.customTriggerMode);
				item.at("playerLEDNewRev").get_to(setting.playerLEDNewRev);
				item.at("MicLEDMode").get_to(setting.micLEDMode);
				item.at("TriggerThreshold").get_to(setting.triggerThresh);
				item.at("ControllerIndex").get_to(setting.controllerIndex);
				item.at("TriggerParams").get_to(setting.triggerParams);
				item.at("RGBUpdate").get_to(setting.rgbUpdate);
				userTriggers.push_back(setting);
			}
			logger::info("Loaded {} trigger settings", userTriggers.size());
			return true;
		} catch (const std::exception& e) {
			logger::error("Exception in LoadTriggerSettings: {}", e.what());
			return false;
		}
	}

	void GeneratePackets()
	{
		triggerPackets.clear();
		for (const auto& trigger : userTriggers) {
			DSX::Packet packet;
			if (trigger.triggerType == static_cast<int>(DSX::TriggerMode::CustomTriggerValue)) {
				packet.AddCustomAdaptiveTrigger(
					trigger.controllerIndex,
					static_cast<DSX::Trigger>(trigger.triggerSide),
					static_cast<DSX::TriggerMode>(trigger.triggerType),
					static_cast<DSX::CustomTriggerValueMode>(trigger.customTriggerMode),
					{ trigger.triggerParams[0], trigger.triggerParams[1], trigger.triggerParams[2], trigger.triggerParams[3] });
			} else {
				packet.AddAdaptiveTrigger(
					trigger.controllerIndex,
					static_cast<DSX::Trigger>(trigger.triggerSide),
					static_cast<DSX::TriggerMode>(trigger.triggerType),
					{ trigger.triggerParams[0], trigger.triggerParams[1], trigger.triggerParams[2], trigger.triggerParams[3] });
			}
			packet.AddTriggerThreshold(trigger.controllerIndex, static_cast<DSX::Trigger>(trigger.triggerSide), trigger.triggerThresh);
			if (!trigger.rgbUpdate.empty()) {
				packet.AddRGB(trigger.controllerIndex, trigger.rgbUpdate[0], trigger.rgbUpdate[1], trigger.rgbUpdate[2]);
			}
			packet.AddPlayerLED(trigger.controllerIndex, static_cast<DSX::PlayerLEDNewRevision>(trigger.playerLEDNewRev));
			packet.AddMicLED(trigger.controllerIndex, static_cast<DSX::MicLEDMode>(trigger.micLEDMode));
			triggerPackets.push_back(packet);
		}
		logger::info("Generated {} trigger packets", triggerPackets.size());
	}

	void BackgroundThread(std::chrono::milliseconds interval)
	{
		while (true) {
			if (!lastLeftPacket.instructions.empty()) {
				networkManager.SendPacket(lastLeftPacket);
			}
			if (!lastRightPacket.instructions.empty()) {
				networkManager.SendPacket(lastRightPacket);
			}
			std::this_thread::sleep_for(interval);
		}
	}
}



extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() noexcept {
	SKSE::PluginVersionData v;
	v.PluginName(Plugin::NAME.data());
	v.PluginVersion(Plugin::VERSION);
	v.UsesAddressLibrary(true);
	v.HasNoStructUse();
	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory");
	}

	*path /= Plugin::NAME;
	*path += ".log";

	// File sink
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	// Debug output sink (for Visual Studio Output window)
	auto debugSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();

	// Combine sinks
	std::vector<spdlog::sink_ptr> sinks{ fileSink, debugSink };
	auto log = std::make_shared<spdlog::logger>("global log", sinks.begin(), sinks.end());

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v");

	logger::info(FMT_STRING("{} v{}"), Plugin::NAME, Plugin::VERSION);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	// Debugger check after logging is set up
#ifndef NDEBUG
	while (!IsDebuggerPresent()) {
		Sleep(100);
	}
	logger::info("Debugger attached, proceeding...");
#endif

	Init(a_skse);

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
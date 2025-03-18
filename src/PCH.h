#pragma once

#define WIN32_LEAN_AND_MEAN

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>

namespace logger = SKSE::log;

namespace stl
{
	using namespace SKSE::stl;
}

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
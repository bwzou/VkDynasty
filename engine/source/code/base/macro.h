#pragma once

#include "../log/LogSystem.h"

#include <chrono>
#include <thread>

#define LOG_HELPER(LOG_LEVEL, ...) \
    mLoggerSystem->log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " + __VA_ARGS__);

#define LOG_DEBUG(...) LOG_HELPER(LogSystem::LogLevel::debug, __VA_ARGS__);

#define LOG_INFO(...) LOG_HELPER(LogSystem::LogLevel::info, __VA_ARGS__);

#define LOG_WARN(...) LOG_HELPER(LogSystem::LogLevel::warn, __VA_ARGS__);

#define LOG_ERROR(...) LOG_HELPER(LogSystem::LogLevel::error, __VA_ARGS__);

#define LOG_FATAL(...) LOG_HELPER(LogSystem::LogLevel::fatal, __VA_ARGS__);

#define DynastySleep(_ms) std::this_thread::sleep_for(std::chrono::milliseconds(_ms));

#define DynastyNameOf(name) #name

#ifdef NDEBUG
#define ASSERT(statement)
#else
#define ASSERT(statement) assert(statement)
#endif

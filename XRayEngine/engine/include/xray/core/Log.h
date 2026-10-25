#pragma once

// Logging: the engine's first window into its own behavior.
//
// Every engine system reports what it does through this interface:
//
//     xray::log::Info("Platform", "window created ({}x{})", width, height);
//
// A message has a level, a category (which subsystem spoke) and text. Messages
// are delivered to "sinks". The default sink prints to the console. Later, a
// debug overlay or an event-trace recorder can register another sink with
// AddSink() and receive the very same messages, without any engine code changing.
//
// This is the one deliberate piece of global state in the engine: a logger is
// needed everywhere, and passing it into every function would add noise. All the
// state is hidden inside Log.cpp and protected by a mutex.

#include <format>
#include <functional>
#include <string_view>
#include <utility>

namespace xray::log {

enum class Level { Trace, Debug, Info, Warning, Error };

// What a sink receives. The string_views are only valid during the sink call;
// copy them if you want to keep them.
struct Message {
    Level level;
    std::string_view category;
    std::string_view text;
    double timeSeconds;  // seconds since the first log call (roughly: process start)
};

// A sink must not call back into xray::log (the logger is locked while sinks run).
using Sink = std::function<void(const Message&)>;

// Prints "[   1.234] [INFO ] [Category] text". Warnings and errors go to stderr.
void ConsoleSink(const Message& message);

void AddSink(Sink sink);
void ClearSinks();   // removes every sink, including the console sink
void ResetSinks();   // back to the default: only the console sink

void SetMinimumLevel(Level level);  // messages below this level are dropped
bool IsEnabled(Level level);

const char* LevelName(Level level);

// Low-level entry point. Prefer the formatted helpers below.
void Write(Level level, std::string_view category, std::string_view text);

// Formatted helpers. The format string is checked at compile time, and the
// text is only formatted if the level is enabled.
template <typename... Args>
void Log(Level level, std::string_view category, std::format_string<Args...> format, Args&&... args) {
    if (!IsEnabled(level)) {
        return;
    }
    Write(level, category, std::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
void Trace(std::string_view category, std::format_string<Args...> format, Args&&... args) {
    Log(Level::Trace, category, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Debug(std::string_view category, std::format_string<Args...> format, Args&&... args) {
    Log(Level::Debug, category, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Info(std::string_view category, std::format_string<Args...> format, Args&&... args) {
    Log(Level::Info, category, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Warning(std::string_view category, std::format_string<Args...> format, Args&&... args) {
    Log(Level::Warning, category, format, std::forward<Args>(args)...);
}

template <typename... Args>
void Error(std::string_view category, std::format_string<Args...> format, Args&&... args) {
    Log(Level::Error, category, format, std::forward<Args>(args)...);
}

}  // namespace xray::log

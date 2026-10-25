#include "xray/core/Log.h"

#include <chrono>
#include <cstdio>
#include <mutex>
#include <vector>

namespace xray::log {
namespace {

// All logger state in one place. It is created on first use (function-local
// static) so logging works even from code that runs before main().
struct LoggerState {
    std::mutex mutex;
    std::vector<Sink> sinks;
    Level minimumLevel = Level::Debug;
    std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

    LoggerState() { sinks.push_back(ConsoleSink); }
};

LoggerState& State() {
    static LoggerState state;
    return state;
}

}  // namespace

const char* LevelName(Level level) {
    switch (level) {
        case Level::Trace:   return "TRACE";
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
    }
    return "?";
}

void ConsoleSink(const Message& message) {
    std::FILE* stream = (message.level >= Level::Warning) ? stderr : stdout;
    std::fprintf(stream, "[%9.3f] [%-5s] [%.*s] %.*s\n",
                 message.timeSeconds,
                 LevelName(message.level),
                 static_cast<int>(message.category.size()), message.category.data(),
                 static_cast<int>(message.text.size()), message.text.data());
    // Flush every line so that the last messages before a crash are not lost
    // in a buffer. Logging volume is low, so the cost does not matter yet.
    std::fflush(stream);
}

void AddSink(Sink sink) {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    state.sinks.push_back(std::move(sink));
}

void ClearSinks() {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    state.sinks.clear();
}

void ResetSinks() {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    state.sinks.clear();
    state.sinks.push_back(ConsoleSink);
}

void SetMinimumLevel(Level level) {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    state.minimumLevel = level;
}

bool IsEnabled(Level level) {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    return level >= state.minimumLevel;
}

void Write(Level level, std::string_view category, std::string_view text) {
    LoggerState& state = State();
    std::lock_guard lock(state.mutex);
    if (level < state.minimumLevel) {
        return;
    }

    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - state.startTime;
    const Message message{level, category, text, elapsed.count()};
    for (const Sink& sink : state.sinks) {
        sink(message);
    }
}

}  // namespace xray::log

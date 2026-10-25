#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "xray/core/Log.h"

namespace {

// What a test sink remembers about each message (copies, because the
// string_views inside a Message die when the sink call returns).
struct CapturedMessage {
    xray::log::Level level;
    std::string category;
    std::string text;
};

// Replaces all sinks with one that records messages, and restores the default
// console sink and level when the test ends. The logger is global state, so
// tests must always leave it as they found it.
class LogCapture {
public:
    LogCapture() {
        xray::log::ClearSinks();
        xray::log::SetMinimumLevel(xray::log::Level::Trace);
        xray::log::AddSink([this](const xray::log::Message& message) {
            messages.push_back({message.level, std::string(message.category), std::string(message.text)});
        });
    }

    ~LogCapture() {
        xray::log::ResetSinks();
        xray::log::SetMinimumLevel(xray::log::Level::Debug);
    }

    std::vector<CapturedMessage> messages;
};

}  // namespace

TEST_CASE("Log: a message reaches the sink with its level, category and text") {
    LogCapture capture;

    xray::log::Info("Test", "hello");

    REQUIRE(capture.messages.size() == 1);
    CHECK(capture.messages[0].level == xray::log::Level::Info);
    CHECK(capture.messages[0].category == "Test");
    CHECK(capture.messages[0].text == "hello");
}

TEST_CASE("Log: arguments are formatted into the text") {
    LogCapture capture;

    xray::log::Warning("Test", "{}x{} at {:.1f}", 1280, 720, 59.96);

    REQUIRE(capture.messages.size() == 1);
    CHECK(capture.messages[0].level == xray::log::Level::Warning);
    CHECK(capture.messages[0].text == "1280x720 at 60.0");
}

TEST_CASE("Log: messages below the minimum level are dropped") {
    LogCapture capture;
    xray::log::SetMinimumLevel(xray::log::Level::Warning);

    xray::log::Info("Test", "ignored");
    xray::log::Warning("Test", "kept");
    xray::log::Error("Test", "also kept");

    REQUIRE(capture.messages.size() == 2);
    CHECK(capture.messages[0].text == "kept");
    CHECK(capture.messages[1].level == xray::log::Level::Error);
    CHECK(xray::log::IsEnabled(xray::log::Level::Info) == false);
    CHECK(xray::log::IsEnabled(xray::log::Level::Error) == true);
}

TEST_CASE("Log: every registered sink receives each message") {
    LogCapture capture;
    int secondSinkCalls = 0;
    xray::log::AddSink([&secondSinkCalls](const xray::log::Message&) { ++secondSinkCalls; });

    xray::log::Debug("Test", "one");
    xray::log::Debug("Test", "two");

    CHECK(capture.messages.size() == 2);
    CHECK(secondSinkCalls == 2);
}

TEST_CASE("Log: level names") {
    CHECK(std::string(xray::log::LevelName(xray::log::Level::Trace)) == "TRACE");
    CHECK(std::string(xray::log::LevelName(xray::log::Level::Warning)) == "WARN");
    CHECK(std::string(xray::log::LevelName(xray::log::Level::Error)) == "ERROR");
}

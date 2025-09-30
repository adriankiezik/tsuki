#include "command_dispatcher.hpp"
#include "commands/help_command.hpp"
#include "commands/new_command.hpp"
#include "commands/package_command.hpp"
#include "commands/fuse_command.hpp"
#include "commands/generate_definitions_command.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <memory>

using namespace tsuki::cli;

void initializeLogging() {
    // Create color console logger
    auto console = spdlog::stdout_color_mt("tsuki");
    spdlog::set_default_logger(console);

    // Set pattern: [time] [level] message
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");

    // Set log level (info by default, can be changed via env var)
    const char* log_level = std::getenv("TSUKI_LOG_LEVEL");
    if (log_level) {
        if (std::string(log_level) == "trace") spdlog::set_level(spdlog::level::trace);
        else if (std::string(log_level) == "debug") spdlog::set_level(spdlog::level::debug);
        else if (std::string(log_level) == "info") spdlog::set_level(spdlog::level::info);
        else if (std::string(log_level) == "warn") spdlog::set_level(spdlog::level::warn);
        else if (std::string(log_level) == "error") spdlog::set_level(spdlog::level::err);
    } else {
        spdlog::set_level(spdlog::level::info);
    }
}

void registerAllCommands(CommandDispatcher& dispatcher, const char* program_name) {
    dispatcher.registerCommand("--help", std::make_unique<HelpCommand>(program_name));
    dispatcher.registerCommand("-h", std::make_unique<HelpCommand>(program_name));
    dispatcher.registerCommand("--version", std::make_unique<VersionCommand>());
    dispatcher.registerCommand("-v", std::make_unique<VersionCommand>());
    dispatcher.registerCommand("new", std::make_unique<NewCommand>());
    dispatcher.registerCommand("generate-definitions", std::make_unique<GenerateDefinitionsCommand>());
    dispatcher.registerCommand("--package", std::make_unique<PackageCommand>());
    dispatcher.registerCommand("--fuse", std::make_unique<FuseCommand>());
    dispatcher.registerCommand("--fuse-all", std::make_unique<FuseAllCommand>());
}

int main(int argc, char* argv[]) {
    try {
        // Initialize logging system
        initializeLogging();

        CommandDispatcher dispatcher;
        registerAllCommands(dispatcher, argv[0]);
        return dispatcher.dispatch(argc, argv);
    } catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        spdlog::critical("Unknown fatal error occurred");
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 1;
    }
}
#include <tsuki/tsuki.hpp>
#include <tsuki/packaging.hpp>
#include <tsuki/version.hpp>
#include <tsuki/platform.hpp>
#include <zip.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <vector>
#ifdef _WIN32
    #include <process.h>
    #define getpid _getpid
#else
    #include <unistd.h>
#endif

void printUsage(const char* program_name) {
    std::cout << tsuki::PROJECT_NAME << " Game Engine v" << tsuki::VERSION << "\n";
    std::cout << "Usage:\n\n";

    std::cout << "  Creating projects:\n";
    std::cout << "    " << program_name << " new <name>                              Create new game project\n";
    std::cout << "    " << program_name << " new <name> --template minimal           Create minimal project\n";
    std::cout << "    " << program_name << " new <name> --no-intellisense            Skip IntelliSense setup\n\n";

    std::cout << "  Running games:\n";
    std::cout << "    " << program_name << " <game_directory>     Run a game from directory\n";
    std::cout << "    " << program_name << " <game.tsuki>        Run a .tsuki game file\n";
    std::cout << "    " << program_name << "                     Run if executable contains embedded game\n\n";

    std::cout << "  Packaging:\n";
    std::cout << "    " << program_name << " --package <dir> <output.tsuki>          Create .tsuki file from directory\n";
    std::cout << "    " << program_name << " --fuse <game.tsuki> <output>            Create standalone executable\n";
    std::cout << "    " << program_name << " --fuse <game.tsuki> <output> --target windows  Create Windows executable from Linux\n";
    std::cout << "    " << program_name << " --fuse-all <game.tsuki> <prefix>        Create executables for all platforms\n\n";

    std::cout << "  Other:\n";
    std::cout << "    " << program_name << " --help              Show this help\n";
    std::cout << "    " << program_name << " --version           Show version\n\n";

    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " new mygame                           # Create new project with starter template\n";
    std::cout << "  " << program_name << " new mygame --template minimal        # Create minimal project\n";
    std::cout << "  " << program_name << " mygame/                              # Run game from directory\n";
    std::cout << "  " << program_name << " mygame.tsuki                         # Run packaged game\n";
    std::cout << "  " << program_name << " mygame                               # Auto-detect mygame.tsuki\n";
    std::cout << "  " << program_name << " --package mygame/ mygame             # Create mygame.tsuki (auto-append extension)\n";
    std::cout << "  " << program_name << " --fuse mygame.tsuki mygame_linux     # Create Linux standalone game\n";
    std::cout << "  " << program_name << " --fuse mygame.tsuki mygame.exe --target windows  # Create Windows .exe from Linux\n";
    std::cout << "  " << program_name << " --fuse-all mygame.tsuki mygame       # Create mygame-linux, mygame-windows.exe, mygame-macos\n";
}

void printVersion() {
    std::cout << tsuki::PROJECT_NAME << " Game Engine v" << tsuki::VERSION << "\n";
    std::cout << "Built with C++23, SDL3, and Lua 5.4\n";
    std::cout << "Packaging support with libzip\n";
}

// Helper function to create minimal main.lua template
std::string getMinimalTemplate(const std::string& project_name) {
    return R"(function tsuki.load()
    -- Called once when the game starts
    window.setTitle(")" + project_name + R"(")
end

function tsuki.update(dt)
    -- Called every frame for game logic
end

function tsuki.draw()
    -- Called every frame for rendering
    graphics.clear(0.1, 0.1, 0.2, 1.0)

    graphics.setColor(1.0, 1.0, 1.0, 1.0)
    graphics.print("Welcome to Tsuki!", window.getWidth()/2, window.getHeight()/2 - 20, "center")
    graphics.print("Edit main.lua to start building your game", window.getWidth()/2, window.getHeight()/2 + 20, "center")
end
)";
}

// Helper function to copy file
bool copyFile(const std::string& src, const std::string& dest) {
    try {
        std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying file: " << e.what() << std::endl;
        return false;
    }
}

// Helper function to copy directory recursively
bool copyDirectory(const std::string& src, const std::string& dest) {
    try {
        std::filesystem::copy(src, dest,
            std::filesystem::copy_options::recursive |
            std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error copying directory: " << e.what() << std::endl;
        return false;
    }
}

// Helper function to create .luarc.json configuration
bool createLuarcConfig(const std::string& project_dir) {
    std::string config_content = R"({
    "Lua.runtime.version": "Lua 5.4",
    "Lua.diagnostics.globals": ["tsuki"],
    "Lua.workspace.checkThirdParty": false,
    "Lua.workspace.library": ["tsuki-definitions.lua"]
})";

    std::string config_path = project_dir + "/.luarc.json";
    std::ofstream config_file(config_path);
    if (!config_file) {
        return false;
    }

    config_file << config_content;
    config_file.close();

    // Verify the file was created successfully
    return std::filesystem::exists(config_path);
}

// Helper function to download and extract IntelliSense from GitHub releases
bool downloadIntelliSenseFromGitHub(const std::string& project_dir) {
    // Construct GitHub release URL
    std::string version = tsuki::VERSION;
    std::string github_url = "https://github.com/adriankiezik/tsuki/releases/download/v" +
                            version + "/tsuki-intellisense-v" + version + ".zip";

    // Create temporary directory for download
    std::string temp_dir = std::filesystem::temp_directory_path().string();
    std::string zip_path = temp_dir + "/tsuki-intellisense-" + version + ".zip";

    // Download the zip file using curl (silent)
    std::string curl_cmd = "curl -s -L -o \"" + zip_path + "\" \"" + github_url + "\" 2>/dev/null";
    int result = system(curl_cmd.c_str());

    if (result != 0) {
        return false;
    }

    // Check if file was downloaded successfully
    if (!std::filesystem::exists(zip_path)) {
        return false;
    }

    // Extract tsuki-definitions.lua from the zip file
    // Create temporary extraction directory
    std::string extract_dir = temp_dir + "/tsuki-intellisense-extract-" + std::to_string(getpid());

    try {
        std::filesystem::create_directories(extract_dir);

        // Use libzip to extract (same library used in packaging.cpp)
        zip_t* archive = zip_open(zip_path.c_str(), ZIP_RDONLY, nullptr);
        if (!archive) {
            std::filesystem::remove_all(extract_dir);
            std::filesystem::remove(zip_path);
            return false;
        }

        // Look for tsuki-definitions.lua in the zip
        zip_int64_t num_entries = zip_get_num_entries(archive, 0);
        bool found_definitions = false;

        for (zip_int64_t i = 0; i < num_entries; i++) {
            const char* name = zip_get_name(archive, i, 0);
            if (name && std::string(name).find("tsuki-definitions.lua") != std::string::npos) {
                // Extract this file
                zip_file_t* file = zip_fopen_index(archive, i, 0);
                if (file) {
                    std::string output_path = project_dir + "/tsuki-definitions.lua";
                    std::ofstream output(output_path, std::ios::binary);

                    if (output) {
                        char buffer[8192];
                        zip_int64_t bytes_read;
                        while ((bytes_read = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                            output.write(buffer, bytes_read);
                        }
                        output.close();
                        found_definitions = true;
                    }

                    zip_fclose(file);
                }
                break;
            }
        }

        zip_close(archive);

        // Cleanup
        std::filesystem::remove_all(extract_dir);
        std::filesystem::remove(zip_path);

        return found_definitions;

    } catch (const std::exception& e) {
        std::filesystem::remove_all(extract_dir);
        std::filesystem::remove(zip_path);
        return false;
    }
}

// Helper function to get executable directory
std::string getExecutableDir() {
    try {
        return std::filesystem::canonical("/proc/self/exe").parent_path().string();
    } catch (...) {
        return std::filesystem::current_path().string();
    }
}

// Helper function to set up intellisense files
bool setupIntelliSense(const std::string& project_dir, bool skip_intellisense) {
    if (skip_intellisense) {
        return true;
    }

    // Get executable directory to look for definitions relative to the executable
    std::string exe_dir = getExecutableDir();

    // Look for tsuki-definitions.lua relative to the executable location
    std::vector<std::string> possible_paths = {
        exe_dir + "/dist/tsuki-definitions.lua",
        exe_dir + "/../dist/tsuki-definitions.lua",
        exe_dir + "/../../dist/tsuki-definitions.lua",
        exe_dir + "/tsuki-definitions.lua",
        "/usr/local/share/tsuki/tsuki-definitions.lua",
        "/usr/share/tsuki/tsuki-definitions.lua"
    };

    std::string definitions_path;
    for (const auto& path : possible_paths) {
        if (std::filesystem::exists(path)) {
            definitions_path = path;
            break;
        }
    }

    bool definitions_available = false;

    if (!definitions_path.empty()) {
        // Copy local definitions file
        if (copyFile(definitions_path, project_dir + "/tsuki-definitions.lua")) {
            definitions_available = true;
        }
    }

    // If no local definitions found, try downloading from GitHub
    if (!definitions_available) {
        definitions_available = downloadIntelliSenseFromGitHub(project_dir);
    }

    if (!definitions_available) {
        std::cout << "Warning: Could not find tsuki-definitions.lua" << std::endl;
        std::cout << "IntelliSense may not work properly. You can:" << std::endl;
        std::cout << "  1. Copy tsuki-definitions.lua to your project manually" << std::endl;
        std::cout << "  2. Download from GitHub releases" << std::endl;
        std::cout << "\nCurrent link address is" << std::endl;
        std::cout << "https://github.com/adriankiezik/tsuki/releases/download/v" << tsuki::VERSION << "/tsuki-intellisense-v" << tsuki::VERSION << ".zip" << std::endl;
        std::cout << "for this version." << std::endl;
        return true; // Don't fail the entire project creation
    }

    // Create .luarc.json - always try to create it if we have definitions
    std::string config_content = R"({
    "Lua.runtime.version": "Lua 5.4",
    "Lua.diagnostics.globals": ["tsuki"],
    "Lua.workspace.checkThirdParty": false,
    "Lua.workspace.library": ["tsuki-definitions.lua"]
})";

    std::string config_path = project_dir + "/.luarc.json";
    std::ofstream config_file(config_path);
    if (!config_file) {
        std::cout << "Warning: Failed to create .luarc.json" << std::endl;
        return true; // Don't fail the entire project creation
    }

    config_file << config_content;
    config_file.close();

    // Verify the file was created successfully
    if (!std::filesystem::exists(config_path)) {
        std::cout << "Warning: .luarc.json file was not created" << std::endl;
    }

    return true;
}

// Main function to create a new project
int createNewProject(const std::string& project_name, const std::string& template_type, bool skip_intellisense) {
    std::cout << "Creating new Tsuki project: " << project_name << std::endl;

    // Validate project name
    if (project_name.empty()) {
        std::cerr << "Error: Project name cannot be empty" << std::endl;
        return 1;
    }

    // Check if directory already exists
    if (std::filesystem::exists(project_name)) {
        std::cerr << "Error: Directory '" << project_name << "' already exists!" << std::endl;
        return 1;
    }

    // Create project directory
    try {
        std::filesystem::create_directory(project_name);
        std::cout << "✓ Created project directory: " << project_name << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: Cannot create directory '" << project_name << "': " << e.what() << std::endl;
        return 1;
    }

    // Create main.lua based on template
    std::string main_lua_path = project_name + "/main.lua";

    if (template_type == "starter") {
        // Copy from examples/starter
        std::string starter_path = "examples/starter/main.lua";
        if (std::filesystem::exists(starter_path)) {
            if (!copyFile(starter_path, main_lua_path)) {
                std::cerr << "Error: Failed to copy starter template" << std::endl;
                std::filesystem::remove_all(project_name);
                return 1;
            }
            std::cout << "✓ Created main.lua from starter template" << std::endl;
        } else {
            // Fallback to minimal if starter not found
            std::cout << "Warning: Starter template not found, using minimal template" << std::endl;
            std::ofstream main_file(main_lua_path);
            if (!main_file) {
                std::cerr << "Error: Cannot create main.lua" << std::endl;
                std::filesystem::remove_all(project_name);
                return 1;
            }
            main_file << getMinimalTemplate(project_name);
            std::cout << "✓ Created main.lua from minimal template" << std::endl;
        }
    } else {
        // Create minimal template
        std::ofstream main_file(main_lua_path);
        if (!main_file) {
            std::cerr << "Error: Cannot create main.lua" << std::endl;
            std::filesystem::remove_all(project_name);
            return 1;
        }
        main_file << getMinimalTemplate(project_name);
        std::cout << "✓ Created main.lua from minimal template" << std::endl;
    }

    // Set up IntelliSense
    if (!setupIntelliSense(project_name, skip_intellisense)) {
        // IntelliSense setup failed, but continue with project creation
        std::cout << "Warning: IntelliSense setup incomplete" << std::endl;
    }

    std::cout << "\n✅ Project '" << project_name << "' created successfully!" << std::endl;
    std::cout << "\nNext steps:" << std::endl;
    std::cout << "  cd " << project_name << std::endl;
    std::cout << "  tsuki ." << std::endl;
    std::cout << "\nFor VSCode IntelliSense:" << std::endl;
    std::cout << "  1. Install the 'Lua' extension by sumneko" << std::endl;
    std::cout << "  2. Open the project folder in VSCode" << std::endl;
    std::cout << "  3. IntelliSense should work automatically!" << std::endl;

    return 0;
}

int runGame(const std::string& game_path) {
    // Initialize the engine
    auto& engine = tsuki::Engine::getInstance();
    if (!engine.init()) {
        std::cerr << "Failed to initialize Tsuki!" << std::endl;
        return 1;
    }

    // Run the Lua game
    engine.runLuaGame(game_path);

    return 0;
}

int main(int argc, char* argv[]) {
    // No arguments - check if this is a fused executable
    if (argc == 1) {
        if (tsuki::Packaging::isFusedExecutable(argv[0])) {
            std::cout << "Detected embedded game in executable" << std::endl;

            // Extract to temporary directory
            std::string temp_dir = (std::filesystem::temp_directory_path() / ("tsuki_fused_" + std::to_string(getpid()))).string();
            if (!tsuki::Packaging::extractFromFusedExecutable(argv[0], temp_dir)) {
                std::cerr << "Failed to extract embedded game!" << std::endl;
                return 1;
            }

            int result = runGame(temp_dir);

            // Cleanup
            std::filesystem::remove_all(temp_dir);
            return result;
        } else {
            printUsage(argv[0]);
            return 1;
        }
    }

    std::string arg = argv[1];

    // Handle command line options
    if (arg == "--help" || arg == "-h") {
        printUsage(argv[0]);
        return 0;
    }

    if (arg == "--version" || arg == "-v") {
        printVersion();
        return 0;
    }

    // Handle new project command
    if (arg == "new") {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " new <project_name> [--template starter|minimal] [--no-intellisense]" << std::endl;
            return 1;
        }

        std::string project_name = argv[2];
        std::string template_type = "starter"; // default template
        bool skip_intellisense = false;

        // Parse additional arguments
        for (int i = 3; i < argc; i++) {
            std::string current_arg = argv[i];
            if (current_arg == "--template" && i + 1 < argc) {
                template_type = argv[++i];
                if (template_type != "starter" && template_type != "minimal") {
                    std::cerr << "Error: Invalid template '" << template_type << "'. Valid options: starter, minimal" << std::endl;
                    return 1;
                }
            } else if (current_arg == "--no-intellisense") {
                skip_intellisense = true;
            } else {
                std::cerr << "Error: Unknown option '" << current_arg << "'" << std::endl;
                return 1;
            }
        }

        return createNewProject(project_name, template_type, skip_intellisense);
    }

    // Handle packaging command
    if (arg == "--package") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " --package <source_directory> <output.tsuki>" << std::endl;
            return 1;
        }

        std::string source_dir = argv[2];
        std::string output_file = argv[3];

        // Auto-append .tsuki extension if not present
        if (!output_file.ends_with(".tsuki")) {
            output_file += ".tsuki";
        }

        if (tsuki::Packaging::createTsukiFile(source_dir, output_file)) {
            std::cout << "Successfully created " << output_file << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to create .tsuki file" << std::endl;
            return 1;
        }
    }

    // Handle fusion command
    if (arg == "--fuse") {
        if (argc < 4) {
            std::cerr << "Usage: " << argv[0] << " --fuse <game.tsuki> <output_executable> [--target windows|linux] [--arch x64|x86]" << std::endl;
            return 1;
        }

        std::string tsuki_file = argv[2];
        std::string output_exe = argv[3];
        // Default to current platform
        std::string target_platform = tsuki::Platform::getCurrentPlatform();
        std::string target_arch = "x64";        // default architecture

        // Parse additional arguments
        for (int i = 4; i < argc; i++) {
            std::string current_arg = argv[i];
            if (current_arg == "--target" && i + 1 < argc) {
                target_platform = argv[++i];
            } else if (current_arg == "--arch" && i + 1 < argc) {
                target_arch = argv[++i];
            }
        }

        // Validate target platform
        if (target_platform != "linux" && target_platform != "windows" && target_platform != "macos") {
            std::cerr << "Error: Unsupported target platform '" << target_platform << "'. Supported: linux, windows, macos" << std::endl;
            return 1;
        }

        // Validate architecture
        if (target_arch != "x64" && target_arch != "x86") {
            std::cerr << "Error: Unsupported architecture '" << target_arch << "'. Supported: x64, x86" << std::endl;
            return 1;
        }

        std::cout << "Creating standalone executable for " << target_platform << " (" << target_arch << ")" << std::endl;

        // Resolve the full path to the current executable
        std::string engine_path = argv[0];

        // Check if argv[0] contains a path separator (cross-platform)
        bool has_path = tsuki::Platform::hasPathSeparator(engine_path);

        if (!has_path) {
            // argv[0] is just the command name, need to find it in PATH
            std::string which_result = tsuki::Platform::findExecutableInPath(engine_path);
            if (!which_result.empty()) {
                engine_path = which_result;
            }
        }

        if (tsuki::Packaging::createStandaloneExecutable(engine_path, tsuki_file, output_exe, target_platform, target_arch)) {
            std::cout << "Successfully created standalone executable: " << output_exe << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to create standalone executable" << std::endl;
            return 1;
        }
    }

    // Handle fusion-all command (create executables for all platforms)
    if (arg == "--fuse-all") {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " --fuse-all <game.tsuki> <output_prefix>" << std::endl;
            return 1;
        }

        std::string tsuki_file = argv[2];
        std::string output_prefix = argv[3];

        std::cout << "Creating standalone executables for all platforms..." << std::endl;

        // Resolve the full path to the current executable
        std::string engine_path = argv[0];

        // Check if argv[0] contains a path separator (cross-platform)
        bool has_path = tsuki::Platform::hasPathSeparator(engine_path);

        if (!has_path) {
            // argv[0] is just the command name, need to find it in PATH
            std::string which_result = tsuki::Platform::findExecutableInPath(engine_path);
            if (!which_result.empty()) {
                engine_path = which_result;
            }
        }

        // Define all platforms and their output names
        std::vector<std::pair<std::string, std::string>> platforms = {
            {"linux", output_prefix + "-linux"},
            {"windows", output_prefix + "-windows.exe"},
            {"macos", output_prefix + "-macos"}
        };

        bool all_successful = true;
        int successful_count = 0;

        for (const auto& [platform, output_name] : platforms) {
            std::cout << "\n=== Creating " << platform << " executable ===" << std::endl;

            if (tsuki::Packaging::createStandaloneExecutable(engine_path, tsuki_file, output_name, platform, "x64")) {
                std::cout << "✓ Successfully created: " << output_name << std::endl;
                successful_count++;
            } else {
                std::cerr << "✗ Failed to create " << platform << " executable" << std::endl;
                all_successful = false;
            }
        }

        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "Successfully created " << successful_count << " out of " << platforms.size() << " executables." << std::endl;

        if (all_successful) {
            std::cout << "All platform executables created successfully!" << std::endl;
            return 0;
        } else {
            std::cerr << "Some executables failed to create. See errors above." << std::endl;
            return 1;
        }
    }

    // Regular game running
    std::string game_path = arg;

    // Auto-detect .tsuki files: if path doesn't end with .tsuki, doesn't exist as directory, but path.tsuki exists, use that
    if (!game_path.ends_with(".tsuki") &&
        !std::filesystem::exists(game_path) &&
        std::filesystem::exists(game_path + ".tsuki")) {
        game_path += ".tsuki";
        std::cout << "Auto-detected .tsuki file: " << game_path << std::endl;
    }

    // Check if it's a .tsuki file
    if (game_path.ends_with(".tsuki")) {
        std::cout << "Loading .tsuki file: " << game_path << std::endl;

        // Create temporary directory for extraction
        std::string temp_dir = (std::filesystem::temp_directory_path() / ("tsuki_" + std::to_string(getpid()))).string();
        if (!tsuki::Packaging::extractTsukiFile(game_path, temp_dir)) {
            std::cerr << "Failed to extract .tsuki file" << std::endl;
            return 1;
        }

        int result = runGame(temp_dir);

        // Cleanup temporary directory
        std::filesystem::remove_all(temp_dir);
        return result;
    }

    // Check if the path exists
    if (!std::filesystem::exists(game_path)) {
        std::cerr << "Error: Game path '" << game_path << "' does not exist!" << std::endl;
        return 1;
    }

    // Check if it's a directory
    if (!std::filesystem::is_directory(game_path)) {
        std::cerr << "Error: '" << game_path << "' is not a directory!" << std::endl;
        return 1;
    }

    // Check if main.lua exists
    std::string main_lua = game_path + "/main.lua";
    if (!std::filesystem::exists(main_lua)) {
        std::cerr << "Error: No main.lua found in '" << game_path << "'!" << std::endl;
        std::cerr << "Tsuki games must have a main.lua file." << std::endl;
        return 1;
    }

    return runGame(game_path);
}
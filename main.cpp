#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include "library.h"

static int run(int argc, char *argv[]) {

    std::string input_file;
    std::string output_file;
    TrajectoryType trajectory_type = TrajectoryType::Hover;

    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];

        if (arg == "--input-file") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                std::cerr << "Помилка. Missing value for --input-file\n";
                return 1;
            }
            input_file = argv[++i];

        } else if (arg == "--output-file") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                std::cerr << "Помилка. Missing value for --output-file\n";
                return 1;
            }
            output_file = argv[++i];

        } else if (arg == "--trajectory") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                std::cerr << "Помилка. Missing value for --trajectory\n";
                return 1;
            }

            const std::string trajectory_name = argv[++i];
            if (trajectory_name == "hover") {
                trajectory_type = TrajectoryType::Hover;
            } else if (trajectory_name == "forward") {
                trajectory_type = TrajectoryType::Forward;
            } else {
                std::cerr << "Помилка: trajectory must be hover or forward\n";
                return 1;
            }

        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Помилка. Usage: ./drop_calc --input-file <path> " << "--output-file <path>\n";
            return 1;
        }
    }

    if (input_file.empty() || output_file.empty()) {
        std::cerr << "Помилка. Usage: ./drop_calc --input-file <path> " << "--output-file <path>\n";
        return 1;
    }

    std::ifstream in(input_file);
    if (!in.is_open()) {
        std::cerr << "Помилка. Could not open input file: " << input_file << "\n";
        return 1;
    }

    Scenario scenario{};
    const ScenarioParseError parse_error = parseScenario(in, scenario);
    switch (parse_error) {
        case ScenarioParseError::None:
            break;
        case ScenarioParseError::InvalidData:std::cerr << "Помилка. Invalid data\n";
            return 1;
        case ScenarioParseError::EmptyScenario:std::cerr << "Помилка. Empty scenario\n";
            return 1;
        case ScenarioParseError::InvalidAltitudeOrCount:std::cerr << "Помилка. Invalid altitude or count\n";
            return 1;
        case ScenarioParseError::InvalidTargetData:std::cerr << "Помилка. Invalid target data\n";
            return 1;
        case ScenarioParseError::HpTooLow:std::cerr << "Помилка. Hp must be at least 1\n";
            return 1;
        case ScenarioParseError::InvalidTargetType:std::cerr << "Помилка. Invalid target type\n";
            return 1;
    }

    const auto start = std::chrono::steady_clock::now();
    const CalculationResult result = calculate(scenario, trajectory_type);
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "elapsed_us " << elapsed_us << '\n';

    std::ofstream out(output_file);

    if (!out.is_open()) {
        std::cerr << "Помилка. Could not open output file: " << output_file << "\n";
        return 1;
    }

    if (!writeResult(out, result)) {
        std::cerr << "Помилка. Error writing output file\n";
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    
    try {
        return run(argc, argv);
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}

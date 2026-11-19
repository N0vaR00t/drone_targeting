#include "library.h"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
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
            std::cerr << "Помилка. Usage: ./drop_calc --input-file <path> "<< "--output-file <path>\n";
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

    Scenario scenario;
    double wind_x;
    double wind_y;
    int count;

    if (!(in >> scenario.altitude_m >> wind_x >> wind_y >> count)) {
        std::cerr << "Помилка. Invalid data\n";
        return 1;
    }

    if (count == 0) {
        std::cerr << "Помилка. Empty scenario\n";
        return 1;
    }

    if (scenario.altitude_m <= 0 || count < 0) {
        std::cerr << "Помилка. Invalid altitude or count\n";
        return 1;
    }

    scenario.wind = Vector2D(wind_x, wind_y);

    for (int i = 0; i < count; i++) {
        double x;
        double y;
        double hp;
        std::string type_name;

        if (!(in >> x >> y >> hp >> type_name)) {
            std::cerr << "Помилка. Invalid target data\n";
            return 1;
        }

        if (hp < 1) {
            std::cerr << "Помилка. Hp must be at least 1\n";
            return 1;
        }

        TargetType type;
        if (!parseTargetType(type_name, type)) {
            std::cerr << "Помилка. Invalid target type\n";
            return 1;
        }

        scenario.targets.push_back({type, Vector2D(x, y), hp});
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

    out << std::fixed << std::setprecision(2);
    out << result.impact.getX() << " " << result.impact.getY() << "\n";
    out << result.release.getX() << " " << result.release.getY() << "\n";
    out << result.weapon_name << "\n";
    out << result.destroyed << " " << result.value_sum << " " << result.weapon_price << " " << result.net << "\n";

    if (!out) {
        std::cerr << "Помилка. Error writing output file\n";
        return 1;
    }

    return 0;
}

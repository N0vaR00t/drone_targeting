#include "library.h"
#include "provided_coverage.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <istream>
#include <memory>
#include <ostream>
#include <vector>

namespace {

constexpr double G = 9.81;
enum class WeaponType { Frag, HE, HEAT };

class Weapon {

   private:
    WeaponType type;
    double radius;
    double damage;
    int price;

   public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- intentional weapon parameter order.
    Weapon(WeaponType weapon_type, double weapon_radius, double weapon_damage, int weapon_price) : type(weapon_type), radius(weapon_radius), damage(weapon_damage), price(weapon_price) {}

    WeaponType getType() const {
        return type;
    }

    double getRadius() const {
        return radius;
    }

    double getDamage() const {
        return damage;
    }

    int getPrice() const {
        return price;
    }
};

double targetValue(TargetType type) {

    switch (type) {
        case TargetType::PERSONNEL:
            return 10;
        case TargetType::LOGISTICS:
            return 20;
        case TargetType::APC:
            return 40;
        case TargetType::TANK:
            return 60;
        case TargetType::RADAR:
            return 90;
    }
    return 0;
}

std::string weaponName(WeaponType type) {

    switch (type) {
        case WeaponType::Frag:
            return "Frag";
        case WeaponType::HE:
            return "HE";
        case WeaponType::HEAT:
            return "HEAT";
    }
    return {};
}

struct SelectionResult {

    int net;
    int value_sum;
    int destroyed;
    int best_target;
    int best_weapon;
};

class TargetWeaponSelector {

   public:
    SelectionResult select(const std::vector<Target> &targets, const std::vector<Weapon> &weapons) const {

        std::vector<double> radii;
        radii.reserve(weapons.size());
        for (const Weapon &weapon : weapons) {
            radii.push_back(weapon.getRadius());
        }
        const CoverageChecker coverage_checker(radii.data(), radii.size());

        struct CompareResults {
            const std::vector<Weapon> &weapons;

            bool operator()(const SelectionResult &a, const SelectionResult &b) const {
                if (a.net != b.net) {
                    return a.net < b.net;
                }

                if (weapons[a.best_weapon].getPrice() != weapons[b.best_weapon].getPrice()) {
                    return weapons[a.best_weapon].getPrice() > weapons[b.best_weapon].getPrice();
                }
                return false;
            }
        };

        std::vector<SelectionResult> results;
        int target_index = 0;

        for (const Target &target : targets) {
            int weapon_index = 0;

            for (const Weapon &weapon : weapons) {
                int value_sum = 0;
                int destroyed = 0;

                for (const Target &other_target : targets) {

                    const double dx = target.position.getX() - other_target.position.getX();
                    const double dy = target.position.getY() - other_target.position.getY();
                    if (coverage_checker.covers(static_cast<std::size_t>(weapon_index), dx, dy) &&
                        weapon.getDamage() >= other_target.hp) {
                        value_sum += static_cast<int>(targetValue(other_target.type));
                        destroyed++;
                    }
                }

                results.push_back({value_sum - weapon.getPrice(), value_sum, destroyed, target_index, weapon_index});
                weapon_index++;
            }
            target_index++;
        }

        const auto best = std::max_element(results.begin(), results.end(), CompareResults{weapons});
        return *best;
    }
};

class ITrajectorySource {

   public:
    virtual ~ITrajectorySource() = default;

    virtual Vector2D calculateReleasePoint(const Vector2D &impact, double altitude, const Vector2D &wind) const = 0;
};

class HoverTrajectory : public ITrajectorySource {

   public:
    Vector2D calculateReleasePoint(const Vector2D &impact, double altitude, const Vector2D &wind) const override {
        const double time = std::sqrt(2.0 * altitude / G);
        return Vector2D(impact.getX() - wind.getX() * time, impact.getY() - wind.getY() * time);
    }
};

class ForwardThrowTrajectory : public ITrajectorySource {

   public:
    Vector2D calculateReleasePoint(const Vector2D &impact, double altitude, const Vector2D &wind) const override {
        const double time = std::sqrt(2.0 * altitude / G);
        const double drone_velocity_x = 12.0;
        return Vector2D(impact.getX() - (wind.getX() + drone_velocity_x) * time, impact.getY() - wind.getY() * time);
    }
};

} 

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- coordinates are intentionally x, y.
Vector2D::Vector2D(double x, double y) : x_(x), y_(y) {}

double Vector2D::getX() const {
    return x_;
}

double Vector2D::getY() const {
    return y_;
}

static bool parseTargetType(const std::string &name, TargetType &type) {

    if (name == "PERSONNEL") {
        type = TargetType::PERSONNEL;
    } else if (name == "LOGISTICS") {
        type = TargetType::LOGISTICS;
    } else if (name == "APC") {
        type = TargetType::APC;
    } else if (name == "TANK") {
        type = TargetType::TANK;
    } else if (name == "RADAR") {
        type = TargetType::RADAR;
    } else {
        return false;
    }
    return true;
}

ScenarioParseError parseScenario(std::istream &input, Scenario &scenario) {

    double altitude_m;
    double wind_x;
    double wind_y;
    int count;

    if (!(input >> altitude_m >> wind_x >> wind_y >> count)) {
        return ScenarioParseError::InvalidData;
    }

    if (count == 0) {
        return ScenarioParseError::EmptyScenario;
    }

    if (altitude_m <= 0 || count < 0) {
        return ScenarioParseError::InvalidAltitudeOrCount;
    }

    scenario.altitude_m = altitude_m;
    scenario.wind = Vector2D(wind_x, wind_y);
    scenario.targets.clear();
    scenario.targets.reserve(static_cast<std::size_t>(count));

    for (int i = 0; i < count; i++) {

        double x;
        double y;
        double hp;
        std::string type_name;

        if (!(input >> x >> y >> hp >> type_name)) {
            return ScenarioParseError::InvalidTargetData;
        }

        if (hp < 1) {
            return ScenarioParseError::HpTooLow;
        }

        TargetType type;
        if (!parseTargetType(type_name, type)) {
            return ScenarioParseError::InvalidTargetType;
        }

        scenario.targets.push_back({type, Vector2D(x, y), hp});
    }

    return ScenarioParseError::None;
}

CalculationResult calculate(const Scenario &scenario, TrajectoryType trajectory_type) {
    
    const std::vector<Weapon> weapons = {{WeaponType::Frag, 60.0, 25.0, 35}, {WeaponType::HE, 35.0, 55.0, 20}, {WeaponType::HEAT, 15.0, 130.0, 45}};
    const TargetWeaponSelector selector;
    const SelectionResult selection = selector.select(scenario.targets, weapons);
    const Vector2D impact = scenario.targets[selection.best_target].position;

    std::unique_ptr<ITrajectorySource> trajectory;
    if (trajectory_type == TrajectoryType::Hover) {
        trajectory = std::make_unique<HoverTrajectory>();
    } else {
        trajectory = std::make_unique<ForwardThrowTrajectory>();
    }

    const Vector2D release = trajectory->calculateReleasePoint(impact, scenario.altitude_m, scenario.wind);
    const Weapon &selected_weapon = weapons[selection.best_weapon];

    return {impact, release, weaponName(selected_weapon.getType()), selection.destroyed, selection.value_sum, selected_weapon.getPrice(), selection.net};
}

bool writeResult(std::ostream &output, const CalculationResult &result) {

    output << std::fixed << std::setprecision(2);
    output << result.impact.getX() << " " << result.impact.getY() << "\n";
    output << result.release.getX() << " " << result.release.getY() << "\n";
    output << result.weapon_name << "\n";
    output << result.destroyed << " " << result.value_sum << " " << result.weapon_price << " " << result.net << "\n";
    return static_cast<bool>(output);
}

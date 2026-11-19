#ifndef DROP_CALC_LIBRARY_H
#define DROP_CALC_LIBRARY_H

#include <string>
#include <vector>

enum class TargetType {
    PERSONNEL,
    LOGISTICS,
    APC,
    TANK,
    RADAR
};

enum class TrajectoryType {
    Hover,
    Forward
};

class Vector2D {
public:
    Vector2D() = default;
    Vector2D(double x, double y);

    double getX() const;
    double getY() const;

private:
    double x_ = 0.0;
    double y_ = 0.0;
};

struct Target {
    TargetType type;
    Vector2D position;
    double hp;
};

struct Scenario {
    double altitude_m;
    Vector2D wind;
    std::vector<Target> targets;
};

struct CalculationResult {
    Vector2D impact;
    Vector2D release;
    std::string weapon_name;
    int destroyed;
    int value_sum;
    int weapon_price;
    int net;
};

bool parseTargetType(const std::string& name, TargetType& type);
CalculationResult calculate(const Scenario& scenario, TrajectoryType trajectory_type);

#endif

#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <memory>

inline void forward_throw_release(
    double impact_x, double impact_y,
    double altitude_m,
    double wind_x, double wind_y,
    double drone_vx, double drone_vy,
    double& out_release_x, double& out_release_y
) {
    const double g = 9.81;
    const double t = std::sqrt(2.0 * altitude_m / g);

    out_release_x = impact_x - (wind_x + drone_vx) * t;
    out_release_y = impact_y - (wind_y + drone_vy) * t;
}

const double G = 9.81;

enum class TargetType {
    PERSONNEL,
    LOGISTICS,
    APC,
    TANK,
    RADAR
};

enum class WeaponType {
    Frag,
    HE,
    HEAT
};

class Vector2D {
private:
    double x;
    double y;

public:
    Vector2D(double x_value = 0.0, double y_value = 0.0) : x(x_value), y(y_value) {}

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    Vector2D operator+(const Vector2D& other) const {
        return Vector2D(x + other.x, y + other.y);
    }

    Vector2D operator-(const Vector2D& other) const {
        return Vector2D(x - other.x, y - other.y);
    }

    Vector2D operator*(double scalar) const {
        return Vector2D(x * scalar, y * scalar);
    }

    double distanceTo(const Vector2D& other) const {
        double dx = x - other.x;
        double dy = y - other.y;

        return std::sqrt(dx * dx + dy * dy);
    }
};

class ITrajectorySource {
public:
    virtual Vector2D calculateReleasePoint(const Vector2D& impact, double altitude, const Vector2D& wind) const = 0;
    virtual ~ITrajectorySource() = default;
};

class Target {
private:
    TargetType type;
    Vector2D position;
    double hp;
    double value;

public:
    Target(TargetType target_type, const Vector2D& target_position, double target_hp, double target_value) : type(target_type), position(target_position), hp(target_hp), value(target_value) {}

    TargetType getType() const {
        return type;
    }

    Vector2D getPosition() const {
        return position;
    }

    double getHp() const {
        return hp;
    }

    double getValue() const {
        return value;
    }
};

class Weapon {
private:
    WeaponType type;
    double radius;
    double damage;
    int price;

public:
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

struct SelectionResult {
    int net;
    int value_sum;
    int destroyed;
    int best_target;
    int best_weapon;
};

class TargetWeaponSelector {
public:
    SelectionResult select(const std::vector<Target> & targets, const std::vector<Weapon>& weapons) const {
        SelectionResult result = {
            -100000, 0, 0, 0, 0
        };

        int t = 0;

        for (const Target& target : targets) {
            int w = 0;

            for (const Weapon& weapon : weapons) {
                int sum = 0;
                int temp_destroyed = 0;

                for (const Target& other_target : targets) {
                    double d = target.getPosition().distanceTo(
                        other_target.getPosition()
                    );

                    if (d <= weapon.getRadius() && weapon.getDamage() >= other_target.getHp()) {
                        sum += static_cast<int>(other_target.getValue());
                        temp_destroyed++;
                    }
                }
                
                int temp_net = sum - weapon.getPrice();

                if (temp_net > result.net || (temp_net == result.net && weapon.getPrice() < weapons[result.best_weapon].getPrice())) {

                    result.net = temp_net;
                    result.value_sum = sum;
                    result.destroyed = temp_destroyed;
                    result.best_target = t;
                    result.best_weapon = w;
                }

                w++;
            }

            t++;
        }

        return result;
    }
};

class HoverTrajectory : public ITrajectorySource {
private:
    double fallTime(double altitude_m) const {
        return std::sqrt(2.0 * altitude_m / G);
    }

public:
    Vector2D calculateReleasePoint(const Vector2D& impact, double altitude, const Vector2D& wind) const override {
        double time = fallTime(altitude);
        return impact - wind * time;
    }
};

class ForwardThrowTrajectory : public ITrajectorySource {
private:
    Vector2D drone_velocity;

    double fallTime(double altitude_m) const {
        return std::sqrt(2.0 * altitude_m / G);
    }

public:
    explicit ForwardThrowTrajectory(const Vector2D& velocity = Vector2D(12.0, 0.0)) : drone_velocity(velocity) {}

    Vector2D calculateReleasePoint(const Vector2D& impact, double altitude, const Vector2D& wind) const override
    {
        double release_x;
        double release_y;

        forward_throw_release(impact.getX(), impact.getY(), altitude, wind.getX(), wind.getY(), drone_velocity.getX(), drone_velocity.getY(), release_x, release_y);

        return Vector2D(release_x, release_y);
    }
};

bool getTargetType(const std::string& name, TargetType& type, double& value) {
    
    if (name == "PERSONNEL") {
        type = TargetType::PERSONNEL;
        value = 10;
    }
    else if (name == "LOGISTICS") {
        type = TargetType::LOGISTICS;
        value = 20;
    }
    else if (name == "APC") {
        type = TargetType::APC;
        value = 40;
    }
    else if (name == "TANK") {
        type = TargetType::TANK;
        value = 60;
    }
    else if (name == "RADAR") {
        type = TargetType::RADAR;
        value = 90;
    }
    else {
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    std::string trajectory_name = "hover";

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--input-file") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                std::cerr << "Помилка. Missing value for --input-file\n";
                return 1;
            }

            input_file = argv[++i];
        }
        
        else if (arg == "--output-file") {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                std::cerr << "Помилка. Missing value for --output-file\n";
                return 1;
            }

            output_file = argv[++i];
        }
        
        else if (arg == "--trajectory") {
            trajectory_name = argv[++i];
        }
        
        else {
            std::cerr << "Unknown argument: " << arg << "\n";
            std::cerr << "Помилка. Usage: ./drop_calc --input-file <path> " << "--output-file <path>\n";
            return 1;
        }
    }
    
    if (trajectory_name != "hover" && trajectory_name != "forward") {
        std::cerr << "Помилка: trajectory must be hover or forward\n";
        return 1;
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
    
    std::unique_ptr<ITrajectorySource> trajectory_source;

    if (trajectory_name == "hover") {
        trajectory_source = std::make_unique<HoverTrajectory>();
    }
    else {
        trajectory_source = std::make_unique<ForwardThrowTrajectory>();
    }
    
    double altitude_m;
    double wind_x;
    double wind_y;
    int count;

    if (!(in >> altitude_m >> wind_x >> wind_y >> count)) {
        std::cerr << "Помилка. Invalid data\n";
        return 1;
    }

    if (altitude_m <= 0 || count < 1 || count > 64) {
        std::cerr << "Помилка. altitude_m must be > 0, " << "count must be between 1 and 64\n";
        return 1;
    }

    std::vector<Target> targets;

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
        double value;

        if (!getTargetType(type_name, type, value)) {
            std::cerr << "Помилка. Invalid target type\n";
            return 1;
        }

        targets.emplace_back(
            type,
            Vector2D(x, y),
            hp,
            value
        );
    }

    std::vector<Weapon> weapons = {
        {WeaponType::Frag, 60.0, 25.0, 35},
        {WeaponType::HE,   35.0, 55.0, 20},
        {WeaponType::HEAT, 15.0, 130.0, 45}
    };

    TargetWeaponSelector selector;

    SelectionResult selection = selector.select(targets, weapons);

    int net = selection.net;
    int value_sum = selection.value_sum;
    int destroyed = selection.destroyed;
    int best_target = selection.best_target;
    int best_weapon = selection.best_weapon;

    double impact_x = targets[best_target].getPosition().getX();
    double impact_y = targets[best_target].getPosition().getY();

    Vector2D impact(impact_x, impact_y);
    Vector2D wind(wind_x, wind_y);

    Vector2D release = trajectory_source->calculateReleasePoint(impact, altitude_m, wind);

    double release_x = release.getX();
    double release_y = release.getY();

    std::string final_weapon;

    if (weapons[best_weapon].getType() == WeaponType::Frag) {
        final_weapon = "Frag";
    }
    else if (weapons[best_weapon].getType() == WeaponType::HE) {
        final_weapon = "HE";
    }
    else {
        final_weapon = "HEAT";
    }

    std::ofstream out(output_file);

    if (!out.is_open()) {
        std::cerr << "Помилка. Could not open output file: " << output_file << "\n";
        return 1;
    }

    out << std::fixed << std::setprecision(2);

    out << impact_x << " " << impact_y << "\n";
    out << release_x << " " << release_y << "\n";
    out << final_weapon << "\n";

    out << destroyed << " "
        << value_sum << " "
        << weapons[best_weapon].getPrice() << " "
        << net << "\n";

    if (!out) {
        std::cerr << "Помилка. Error writing output file\n";
        return 1;
    }

    return 0;
}

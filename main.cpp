#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <vector>

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

struct Target {
    TargetType type;
    double x;
    double y;
    double hp;
    double value;
};

struct Weapon {
    WeaponType type;
    double radius;
    double damage;
    int price;
};

double fallTime(double altitude_m) {
    return std::sqrt(2.0 * altitude_m / G);
}

double findDistance(double remains_x, double remains_y){
    return std::sqrt(remains_x * remains_x + remains_y * remains_y);
}

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
    
int main() {
    
    std::ifstream in("scenario.txt");
    
    if (!in.is_open()) {
        std::cerr << "Could not open scenario.txt\n" << std::endl;
        return 1;
    }
    
    double altitude_m;
    double wind_x, wind_y;
    int count;
    
    if (!(in >> altitude_m >> wind_x >> wind_y >> count)) {
        std::cerr << "Invalid data\n";
        return 1;
    }
    
    if (altitude_m <= 0 || count < 1 || count > 64) {
        std::cerr << "altitude_m must be > 0, count must be between 1 and 64\n";
        return 1;
    }
    
    std::vector<Target> targets;

    for (int i = 0; i < count; i++) {
        Target target;
        std::string type;

        if (!(in >> target.x >> target.y >> target.hp >> type)) {
            std::cerr << "Invalid target data\n";
            return 1;
        }

        if (!getTargetType(type, target.type, target.value)) {
            std::cerr << "Invalid target type\n";
            return 1;
        }
        
        if (target.hp < 1) {
            std::cerr << "Hp must be at least 1\n";
            return 1;
        }

        targets.push_back(target);
    }
    
    std::vector<Weapon> weapons = {
        {WeaponType::Frag, 60.0, 25.0, 35},
        {WeaponType::HE,   35.0, 55.0, 20},
        {WeaponType::HEAT, 15.0, 130.0, 45}
    };
       
    int net = -100000;
    int value_sum = 0;
    int destroyed = 0;
    int best_target = 0;
    int best_weapon = 0;
    int t = 0;

    for (const Target& target : targets) {
        int w = 0;

        for (const Weapon& weapon : weapons) {
            int sum = 0;
            int temp_destroyed = 0;

            for (const Target& other_targets : targets) {
                double d = findDistance(
                    other_targets.x - target.x,
                    other_targets.y - target.y
                );

                if (d <= weapon.radius && weapon.damage >= other_targets.hp) {
                    sum += other_targets.value;
                    temp_destroyed++;
                }
            }

            int temp_net = sum - weapon.price;

            if (temp_net > net || (temp_net == net && weapon.price < weapons[best_weapon].price)) {
                
                net = temp_net;
                value_sum = sum;
                destroyed = temp_destroyed;
                best_target = t;
                best_weapon = w;
            }

            w++;
        }

        t++;
    }

    double impact_x = targets[best_target].x;
    double impact_y = targets[best_target].y;
    double fall_time = fallTime(altitude_m);
    double release_x = impact_x - wind_x * fall_time;
    double release_y = impact_y - wind_y * fall_time;

    std::string final_weapon;

    if (weapons[best_weapon].type == WeaponType::Frag) {
        final_weapon = "Frag";
    } else if (weapons[best_weapon].type == WeaponType::HE) {
        final_weapon = "HE";
    } else {
        final_weapon = "HEAT";
    }

    std::ofstream out("result.txt");
    out << std::fixed << std::setprecision(2);
    out << impact_x << " " << impact_y << "\n";
    out << release_x << " " << release_y << "\n";
    out << final_weapon << "\n";
    out << destroyed << " " << value_sum << " " << weapons[best_weapon].price << " " << net << "\n";
}

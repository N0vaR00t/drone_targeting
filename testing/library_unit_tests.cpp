#include <gtest/gtest.h>

#include <sstream>

#include "library.h"

namespace {

constexpr char kMissionA[] = "120.0\n" "5.0 0.0\n" "4\n" "300.0 400.0 20 LOGISTICS\n" "300.0 450.0 15 PERSONNEL\n" "260.0 380.0 15 PERSONNEL\n" "340.0 380.0 15 PERSONNEL\n";

Scenario trajectoryScenario() {
    return {19.62, Vector2D(3.0, -2.0), {{TargetType::TANK, Vector2D(100.0, 200.0), 100.0}}};
}

}  

TEST(ScenarioParser, ParsesValidScenarioFromStream) {

    std::istringstream input(kMissionA);
    Scenario scenario{};

    ASSERT_EQ(parseScenario(input, scenario), ScenarioParseError::None);
    EXPECT_DOUBLE_EQ(scenario.altitude_m, 120.0);
    EXPECT_DOUBLE_EQ(scenario.wind.getX(), 5.0);
    EXPECT_DOUBLE_EQ(scenario.wind.getY(), 0.0);
    ASSERT_EQ(scenario.targets.size(), 4U);
    EXPECT_EQ(scenario.targets[0].type, TargetType::LOGISTICS);
}

TEST(ScenarioParser, IdentifiesScenarioWithoutTargets) {

    std::istringstream input("150.0\n4.0 -3.0\n0\n");
    Scenario scenario{};

    EXPECT_EQ(parseScenario(input, scenario), ScenarioParseError::EmptyScenario);
}

TEST(ScenarioParser, RejectsTargetWithUnknownType) {

    std::istringstream input("150.0\n4.0 -3.0\n1\n10.0 20.0 15 UNKNOWN\n");
    Scenario scenario{};

    EXPECT_EQ(parseScenario(input, scenario), ScenarioParseError::InvalidTargetType);
}

TEST(TrajectoryModels, HoverReleasePointMatchesKnownAltitudeAndWind) {

    const CalculationResult result = calculate(trajectoryScenario(), TrajectoryType::Hover);

    EXPECT_NEAR(result.release.getX(), 94.0, 1e-9);
    EXPECT_NEAR(result.release.getY(), 204.0, 1e-9);
}

TEST(TrajectoryModels, ForwardReleasePointMatchesKnownAltitudeAndWind) {

    const CalculationResult result = calculate(trajectoryScenario(), TrajectoryType::Forward);

    EXPECT_NEAR(result.release.getX(), 70.0, 1e-9);
    EXPECT_NEAR(result.release.getY(), 204.0, 1e-9);
}

TEST(TrajectoryModels, ForwardModelAddsDroneVelocityToHorizontalOffset) {

    const Scenario scenario = trajectoryScenario();
    const CalculationResult hover = calculate(scenario, TrajectoryType::Hover);
    const CalculationResult forward = calculate(scenario, TrajectoryType::Forward);

    EXPECT_NEAR(hover.release.getX() - forward.release.getX(), 24.0, 1e-9);
    EXPECT_NEAR(hover.release.getY(), forward.release.getY(), 1e-9);
}

TEST(DecisionSelection, ChoosesWeaponWithHighestNetValue) {

    const Scenario scenario{100.0, Vector2D(), {{TargetType::TANK, Vector2D(10.0, 20.0), 20.0}}};

    const CalculationResult result = calculate(scenario, TrajectoryType::Hover);

    EXPECT_EQ(result.weapon_name, "HE");
    EXPECT_EQ(result.value_sum, 60);
    EXPECT_EQ(result.weapon_price, 20);
    EXPECT_EQ(result.net, 40);
}

TEST(DecisionSelection, BreaksEqualNetTieWithLowerPricedWeapon) {

    const Scenario scenario{100.0, Vector2D(), {{TargetType::TANK, Vector2D(0.0, 0.0), 100.0}, {TargetType::LOGISTICS, Vector2D(45.0, 0.0), 20.0}, {TargetType::LOGISTICS, Vector2D(-22.5, 38.97), 20.0}, {TargetType::PERSONNEL, Vector2D(-22.5, -38.97), 20.0}}};
    const CalculationResult result = calculate(scenario, TrajectoryType::Hover);

    EXPECT_EQ(result.weapon_name, "Frag");
    EXPECT_EQ(result.destroyed, 3);
    EXPECT_EQ(result.value_sum, 50);
    EXPECT_EQ(result.weapon_price, 35);
    EXPECT_EQ(result.net, 15);
    EXPECT_DOUBLE_EQ(result.impact.getX(), 0.0);
    EXPECT_DOUBLE_EQ(result.impact.getY(), 0.0);
}

TEST(ResultFormatting, WritesFieldsInOrderWithTwoDecimalPrecision) {

    const CalculationResult result{Vector2D(12.345, -6.789), Vector2D(10.0, -2.5), "HEAT", 3, 170, 45, 125};
    std::ostringstream output;

    ASSERT_TRUE(writeResult(output, result));
    EXPECT_EQ(output.str(),
              "12.35 -6.79\n"
              "10.00 -2.50\n"
              "HEAT\n"
              "3 170 45 125\n");
}

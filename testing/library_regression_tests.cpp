#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include "library.h"

namespace {

std::string fixtureDirectory(const std::string &directory) {
    const std::string source_file = __FILE__;
    const std::size_t source_directory_end = source_file.find_last_of("/\\");

    if (source_directory_end == std::string::npos) {
        return directory;
    }

    return source_file.substr(0, source_directory_end) + "/" + directory;
}

struct MissionRegressionCase {
    std::string test_name;
    std::string input_file;
    TrajectoryType trajectory;
    std::string expected_file;
};

class MissionRegression : public ::testing::TestWithParam<MissionRegressionCase> {};

}  // namespace

TEST_P(MissionRegression, MatchesExpectedOutput) {
    const MissionRegressionCase &test_case = GetParam();
    SCOPED_TRACE(test_case.test_name);

    std::ifstream input(fixtureDirectory("input") + "/" + test_case.input_file);
    ASSERT_TRUE(input.is_open()) << "Could not open fixture " << test_case.input_file;

    Scenario scenario{};
    ASSERT_EQ(parseScenario(input, scenario), ScenarioParseError::None);

    const CalculationResult result = calculate(scenario, test_case.trajectory);
    std::ostringstream output;
    ASSERT_TRUE(writeResult(output, result));

    std::ifstream expected(fixtureDirectory("expected") + "/" + test_case.expected_file);
    ASSERT_TRUE(expected.is_open()) << "Could not open expected output " << test_case.expected_file;

    std::ostringstream expected_output;
    expected_output << expected.rdbuf();
    ASSERT_FALSE(expected.bad()) << "Could not read expected output " << test_case.expected_file;
    EXPECT_EQ(output.str(), expected_output.str());
}

INSTANTIATE_TEST_SUITE_P(
    ExpectedResults, MissionRegression,
    ::testing::Values(MissionRegressionCase{"MissionA_Hover", "mission-a.txt", TrajectoryType::Hover, "mission-a-hover.txt"},
                      MissionRegressionCase{"MissionA_Forward", "mission-a.txt", TrajectoryType::Forward, "mission-a-forward.txt"},
                      MissionRegressionCase{"MissionB_Hover", "mission-b.txt", TrajectoryType::Hover, "mission-b-hover.txt"},
                      MissionRegressionCase{"MissionB_Forward", "mission-b.txt", TrajectoryType::Forward, "mission-b-forward.txt"},
                      MissionRegressionCase{"MissionC_Hover", "mission-c.txt", TrajectoryType::Hover, "mission-c-hover.txt"},
                      MissionRegressionCase{"MissionC_Forward", "mission-c.txt",TrajectoryType::Forward, "mission-c-forward.txt"},
                      MissionRegressionCase{"Large5000_Hover", "large-5000.txt", TrajectoryType::Hover, "large-5000-hover.txt"},
                      MissionRegressionCase{"Large5000_Forward", "large-5000.txt", TrajectoryType::Forward, "large-5000-forward.txt"}),
    [](const ::testing::TestParamInfo<MissionRegressionCase> &info) {
        return info.param.test_name;
    });

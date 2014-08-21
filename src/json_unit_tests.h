#include <string>

struct JSONUnitTestFailureException {};

void testJSONFiles(std::string inputDirectory, std::string outputDirectory, std::string standardDirectory);
void json_UnitTests();
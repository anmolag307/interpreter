#ifndef CODECRAFTERS_RUNNER_H
#define CODECRAFTERS_RUNNER_H

#include <string>

class Runner {
public:
    Runner() = default;
    int runFromString(const std::string& source);
};

#endif // CODECRAFTERS_RUNNER_H
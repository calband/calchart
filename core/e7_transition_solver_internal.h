#pragma once

#include <stdexcept>

namespace CalChart::TransitionSolverInternal {

class InvalidStepCount : public std::range_error {
public:
    using std::range_error::range_error;
};

auto CheckedStepCount(float steps) -> unsigned;

}

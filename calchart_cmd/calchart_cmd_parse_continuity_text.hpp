#pragma once
//
//  calchart_cmd_parse_continuity_text.hpp
//  calchart_cmd
//
//  Created by Richard Powell on 4/28/2024.
//
//

#include "CalChartShow.h"
#include "print_ps.h"
#include <fstream>

namespace {

auto ParseContinuityText(std::string const& text, std::ostream& os)
{
    try {
        auto&& continuity = CalChart::Continuity(text);
        for (auto& proc : continuity.GetParsedContinuity()) {
            os << *proc << "\n";
        }
    } catch (std::runtime_error const& error) {
        os << "Errors during compile: " << error.what() << "\n";
    }
}

}
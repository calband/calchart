#pragma once
//
//  calchart_cmd_parse.hpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "CalChartAnimationErrors.h"
#include "CalChartShow.h"
#include "print_ps.h"
#include <fstream>
#include <ranges>

namespace {

auto OpenShow(std::string_view showPath) -> std::unique_ptr<CalChart::Show const>
{
    auto input = std::ifstream(std::string(showPath));
    if (!input.is_open()) {
        throw std::runtime_error("could not open file");
    }
    return CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode(), input);
};

auto DumpAnimationErrors(CalChart::Animation const& animation, std::ostream& os)
{
    for (auto&& errors : animation.GetAnimationErrors()) {
        for (auto&& [key, value] : errors.GetErrors()) {
            os << "error " << key << "\n";
        }
    }
}

auto AnimateShow(CalChart::Show const& show, std::ostream& os)
{
    auto animation = CalChart::Animation{ show };
    DumpAnimationErrors(animation, os);
}

auto DumpContinuity(CalChart::Show const& show, std::ostream& os)
{
    auto sheet_num = 0;
    for (auto i = show.GetSheetBegin(); i != show.GetSheetEnd(); ++i, ++sheet_num) {
        for (auto symbol : {
                 CalChart::SYMBOL_PLAIN, CalChart::SYMBOL_SOL, CalChart::SYMBOL_BKSL, CalChart::SYMBOL_SL,
                 CalChart::SYMBOL_X, CalChart::SYMBOL_SOLBKSL, CalChart::SYMBOL_SOLSL, CalChart::SYMBOL_SOLX }) {
            if (!i->ContinuityInUse(symbol)) {
                continue;
            }

            auto&& cont = i->GetContinuityBySymbol(symbol);
            os << "<--StartText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";
            os << cont.GetText() << "\n";
            os << "<--EndText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";

            CalChart::AnimationErrors e;
            auto&& continuity = cont.GetParsedContinuity();
            os << "<--Errors during compile-->\n";
            if (e.AnyErrors()) {
                for (auto&& i : e.GetErrors()) {
                    os << "Error at [" << i.second.line << "," << i.second.col << "] of type " << i.first << "\n";
                }
            }
            os << "<--End errors-->\n";
            os << "<--StartParsed-->\n";
            for (auto&& proc : continuity) {
                os << *proc << "\n";
            }
            os << "<--EndParsed-->\n";
        }
    }
}

auto DumpFileCheck(std::ostream& os)
{
    os << "ContinuityCountDifferentThanSymbol ? 0\n";
}

auto PrintShow(CalChart::Show const& show, std::ostream& os)
{
    auto animation = CalChart::Animation{ show };
    DumpAnimationErrors(animation, os);
    auto currentInfo = animation.GetCurrentInfo(0);
    os << currentInfo.first << "\n";
    std::ranges::copy(currentInfo.second, std::ostream_iterator<std::string>(os, "\n"));
    auto oldInfo = currentInfo;
    for (auto beat : std::views::iota(0UL, animation.GetTotalNumberBeats())) {
        auto currentInfo = animation.GetCurrentInfo(beat);
        if (currentInfo.first != oldInfo.first) {
            os << currentInfo.first << "\n";
        }
        for (auto i : std::views::iota(0ul, currentInfo.second.size())) {
            if (i < oldInfo.second.size() && oldInfo.second.at(i) == currentInfo.second.at(i)) {
                continue;
            }
            os << currentInfo.second.at(i) << "\n";
        }
        oldInfo = currentInfo;
    }
}

auto DumpJSON(CalChart::Show const& show, std::ostream& os)
{
    auto animation = CalChart::Animation{ show };
    auto json = show.toOnlineViewerJSON(animation);
    os << std::setw(4) << json << "\n";
}

}

namespace CalChartCmd {

constexpr auto Parse = [](auto args, auto& os) {
    auto list_of_files = args["<shows>"].asStringList();

    for (auto&& file : list_of_files) {
        auto show = OpenShow(file);

        if (args["--print_show"].asBool()) {
            PrintShow(*show, os);
        }
        if (args["--animate_show"].asBool()) {
            AnimateShow(*show, os);
        }
        if (args["--dump_continuity"].asBool()) {
            DumpContinuity(*show, os);
        }
        if (args["--check_flag"].asBool()) {
            DumpFileCheck(os);
        }
        if (args["--json"].asBool()) {
            DumpJSON(*show, os);
        }
    }
};
}
#pragma once
//
//  calchart_cmd_parse.hpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "CalChartAnimationErrors.h"
#include "CalChartPrintShowToPS.hpp"
#include "CalChartShow.h"
#include <fstream>
#include <print>
#include <ranges>

namespace {

void PrintToPS(
    CalChart::Show const& show, bool landscape, bool cont, bool contsheet, bool overview, std::string_view outfile)
{
    constexpr auto head_font_str = "Palatino-Bold";
    constexpr auto main_font_str = "Helvetica";
    constexpr auto number_font_str = "Helvetica-Bold";
    constexpr auto cont_font_str = "Courier";
    constexpr auto bold_font_str = "Courier-Bold";
    constexpr auto ital_font_str = "Courier-Italic";
    constexpr auto bold_ital_font_str = "Courier-BoldItalic";

    constexpr auto PageWidth = 7.5;
    constexpr auto PageHeight = 10.0;
    constexpr auto PageOffsetX = 0.5;
    constexpr auto PageOffsetY = 0.5;
    constexpr auto PaperLength = 11.0;

    constexpr auto HeaderSize = 3.0;
    constexpr auto YardsSize = 1.5;
    constexpr auto TextSize = 10.0;
    constexpr auto DotRatio = 0.9;
    constexpr auto NumRatio = 1.35;
    constexpr auto PLineRatio = 1.2;
    constexpr auto SLineRatio = 1.2;
    constexpr auto ContRatio = 0.2;
    auto mode = CalChart::ShowMode::GetDefaultShowMode();

    auto printShowToPS = CalChart::PrintShowToPS(show, landscape, cont, contsheet, overview, 50, mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str, bold_font_str, ital_font_str,
            bold_ital_font_str } },
        { PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength }, { HeaderSize, YardsSize, TextSize },
        { DotRatio, NumRatio, PLineRatio, SLineRatio, ContRatio }, CalChart::kDefaultYardLines);

    auto picked = std::set<size_t>{};
    for (auto i = 0UL; i < show.GetNumSheets(); ++i) {
        picked.insert(i);
    }

    auto output = std::ofstream(std::string(outfile));

    output << std::get<0>(printShowToPS(picked, "show"));
}

auto OpenShow(std::string_view showPath) -> std::unique_ptr<CalChart::Show const>
{
    auto input = std::ifstream(std::string(showPath));
    if (!input.is_open()) {
        throw std::runtime_error(std::format("could not open file {}", showPath));
    }
    return CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode(), input);
};

auto DumpAnimationErrors(CalChart::Animation const& animation, std::ostream& os)
{
    for (auto&& errors : animation.GetErrors()) {
        auto sortedErrors = std::accumulate(
            errors.begin(), errors.end(), std::set<CalChart::Animate::Error>{}, [](auto&& acc, auto&& item) {
                acc.insert(item.first);
                return acc;
            });

        for (auto&& key : sortedErrors) {
            os << "error " << key << "\n";
        }
    }
    for (auto&& [sheet, who] : animation.GetCollisions()) {
        os << std::format("collision on sheet {}: ", sheet);
        std::copy(who.begin(), who.end(), std::ostream_iterator<int>(os, ", "));
        os << "\n";
    }
}

auto AnimateShow(CalChart::Show const& show, std::ostream& os)
{
    auto animation = CalChart::Animation{ show };
    DumpAnimationErrors(animation, os);
}

auto DumpContinuity(CalChart::Show const& show, std::ostream& os)
{
    auto sheet_count = show.GetNumSheets();
    for (auto sheet_num = 0UL; sheet_num < sheet_count; ++sheet_num) {
        auto sheet = show.CopySheet(static_cast<unsigned>(sheet_num));
        for (auto symbol : { CalChart::SYMBOL_PLAIN, CalChart::SYMBOL_SOL, CalChart::SYMBOL_BKSL, CalChart::SYMBOL_SL,
                 CalChart::SYMBOL_X, CalChart::SYMBOL_SOLBKSL, CalChart::SYMBOL_SOLSL, CalChart::SYMBOL_SOLX }) {
            if (!sheet.ContinuityInUse(symbol)) {
                continue;
            }

            auto&& cont = sheet.GetContinuityBySymbol(symbol);
            os << "<--StartText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";
            os << cont.GetText() << "\n";
            os << "<--EndText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";

            CalChart::Animate::Errors e;
            auto&& continuity = cont.GetParsedContinuity();
            os << "<--Errors during compile-->\n";
            if (AnyErrors(e)) {
                for (auto&& i : e) {
                    os << "Error of type " << i.first << "\n";
                }
            }
            os << "<--End errors-->\n";
            os << "<--StartParsed-->\n";
            for (auto&& proc : continuity) {
                os << proc->ToString() << "\n";
            }
            os << "<--EndParsed-->\n";
        }
    }
}

auto DumpFileCheck(std::ostream& os) { os << "ContinuityCountDifferentThanSymbol ? 0\n"; }

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

auto DumpJSON(CalChart::Show const& show)
{
    auto animation = CalChart::Animation{ show };
    return show.toOnlineViewerJSON(animation);
}

auto DumpShowJSON(CalChart::Show const& show, std::optional<nlohmann::json> schema)
{
    auto json = show.toJSON();
    if (schema) {
        auto result = CalChart::ValidateShowJson(json, *schema);
        if (!result.IsValid()) {
            throw std::runtime_error(result.GetMessage());
        }
        if (result.HasWarnings()) {
            std::println("JSON validation warnings:\n{}", result.GetMessage());
        }
    }
    return json;
}

auto DumpPrintContinuity(CalChart::Show const& show, std::ostream& os)
{
    auto print_continuities = show.GetAllRawPrintContinuity();
    for (auto&& [index, pc] : CalChart::Ranges::enumerate_view(print_continuities)) {
        if (pc.empty()) {
            continue;
        }
        os << "Sheet: " << index << "\n";
        os << pc << "\n";
    }
}

auto DumpBeats(CalChart::Show const& show, std::ostream& os)
{
    auto downbeatTimes = show.GetDownbeatTimes();
    for (auto&& time : downbeatTimes) {
        os << time.count() << "\n";
    }
}

auto loadJson(std::string_view path) -> nlohmann::json
{
    std::ifstream file(path.data());
    if (!file.is_open()) {
        throw std::runtime_error(std::format("could not open file {}", path));
    }
    nlohmann::json json;
    file >> json;
    return json;
}
}

namespace CalChartCmd {

constexpr auto Parse = [](auto args, auto& os) {
    auto list_of_files = args["<shows>"].asStringList();

    for (auto&& file : list_of_files) {
        auto show = OpenShow(file);

        std::vector<std::jthread> threads;
        if (args["--print_show"].asBool()) {
            threads.emplace_back([&] { PrintShow(*show, os); });
        }
        if (args["--print_show_out"]) {
            threads.emplace_back([&] {
                auto where = args["--print_show_out"].asString();
                auto output = std::ofstream(where);
                PrintShow(*show, output);
            });
        }
        if (args["--animate_show"].asBool()) {
            threads.emplace_back([&] { AnimateShow(*show, os); });
        }
        if (args["--animate_show_out"]) {
            threads.emplace_back([&] {
                auto where = args["--animate_show_out"].asString();
                auto output = std::ofstream(where);
                AnimateShow(*show, output);
            });
        }
        if (args["--dump_continuity"].asBool()) {
            threads.emplace_back([&] { DumpContinuity(*show, os); });
        }
        if (args["--dump_cont_out"]) {
            threads.emplace_back([&] {
                auto where = args["--dump_cont_out"].asString();
                auto output = std::ofstream(where);
                DumpContinuity(*show, output);
            });
        }
        if (args["--check_flag"].asBool()) {
            threads.emplace_back([&] { DumpFileCheck(os); });
        }
        if (args["--json"].asBool()) {
            threads.emplace_back([&] {
                auto jsonwidth = args["--jsonwidth"].asLong();
                os << std::setw(jsonwidth) << DumpJSON(*show) << "\n";
            });
        }
        if (args["--json_out"]) {
            threads.emplace_back([&] {
                auto where = args["--json_out"].asString();
                auto output = std::ofstream(where);
                auto jsonwidth = args["--jsonwidth"].asLong();
                output << std::setw(jsonwidth) << DumpJSON(*show) << "\n";
            });
        }
        if (args["--showjson"].asBool()) {
            threads.emplace_back([&] {
                auto jsonwidth = args["--jsonwidth"].asLong();
                auto schema = std::optional<nlohmann::json>{};
                if (!args["--showschema"].asString().empty()) {
                    auto schemaPath = args["--showschema"].asString();
                    auto schemaJson = loadJson(schemaPath);
                    schema = schemaJson;
                }
                os << std::setw(jsonwidth) << DumpShowJSON(*show, schema) << "\n";
            });
        }
        if (args["--showjson_out"]) {
            threads.emplace_back([&] {
                auto where = args["--showjson_out"].asString();
                auto output = std::ofstream(where);
                auto jsonwidth = args["--jsonwidth"].asLong();
                auto schema = std::optional<nlohmann::json>{};
                if (!args["--showschema"].asString().empty()) {
                    auto schemaPath = args["--showschema"].asString();
                    auto schemaJson = loadJson(schemaPath);
                    schema = schemaJson;
                }
                output << std::setw(jsonwidth) << DumpShowJSON(*show, schema) << "\n";
            });
        }
        if (args["--dump_print_continuity"].asBool()) {
            threads.emplace_back([&] { DumpPrintContinuity(*show, os); });
        }
        if (args["--dump_print_cont_out"]) {
            threads.emplace_back([&] {
                auto where = args["--dump_print_cont_out"].asString();
                auto output = std::ofstream(where);
                DumpPrintContinuity(*show, output);
            });
        }
        if (args["--dump_beats"].asBool()) {
            threads.emplace_back([&] { DumpBeats(*show, os); });
        }
        if (args["--dump_beats_out"]) {
            threads.emplace_back([&] {
                auto where = args["--dump_beats_out"].asString();
                auto output = std::ofstream(where);
                DumpBeats(*show, output);
            });
        }
        if (args["--landscape_out"]) {
            threads.emplace_back([&] {
                auto where = args["--landscape_out"].asString();
                PrintToPS(*show, true, false, false, false, where);
            });
        }
        if (args["--cont_out"]) {
            threads.emplace_back([&] {
                auto where = args["--cont_out"].asString();
                PrintToPS(*show, false, true, false, false, where);
            });
        }
        if (args["--contsheet_out"]) {
            threads.emplace_back([&] {
                auto where = args["--contsheet_out"].asString();
                PrintToPS(*show, false, false, true, false, where);
            });
        }
        if (args["--overview_out"]) {
            threads.emplace_back([&] {
                auto where = args["--overview_out"].asString();
                PrintToPS(*show, false, false, false, true, where);
            });
        }
    }
};
}

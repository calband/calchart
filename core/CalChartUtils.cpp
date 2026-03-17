/*
 * CalChartUtils.cpp
 * General Utilities
 */

/*
   Copyright (C) 1995-2026  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CalChartUtils.h"
#include "CalChartFileFormat.h"
#include "CalChartTypes.h"
#include <charconv>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace CalChart {

// Fermatas in string form are of the form "beat=seconds,beat=seconds,..."
// This is a helper function to make sure transiton is consistent.
// auto ToFermatas(std::string_view input) -> Fermatas;
// auto ToString(Fermatas const& input) -> std::string;
auto ToFermatas(std::string_view input) -> Fermatas
{
    std::map<CalChart::Beats, CalChart::Seconds> result;

    while (!input.empty()) {
        // Strip leading whitespace
        auto nonspace = input.find_first_not_of(" \t");
        if (nonspace == std::string_view::npos) {
            break;
        }
        input.remove_prefix(nonspace);

        // Parse int key
        int key{};
        auto [keyEnd, keyEc] = std::from_chars(input.data(), input.data() + input.size(), key);
        if (keyEc != std::errc{}) {
            return {};
        }
        input.remove_prefix(keyEnd - input.data());
        if (key <= 0) { // must be greater than 0
            return {};
        }

        // Expect '='
        auto eq = input.find_first_not_of(" \t");
        if (eq == std::string_view::npos || input[eq] != '=') {
            return {};
        }
        input.remove_prefix(eq + 1);

        // Strip whitespace before float
        auto beforeFloat = input.find_first_not_of(" \t");
        if (beforeFloat == std::string_view::npos) {
            return {};
        }
        input.remove_prefix(beforeFloat);

        // Parse float via strtof (Apple Clang lacks from_chars for floats)
        char* floatEnd{};
        // strtof needs a null-terminated string — make a temporary
        std::string tmp(input);
        float value = std::strtof(tmp.c_str(), &floatEnd);

        if (floatEnd == tmp.c_str()) { // no digits consumed
            return {};
        }
        if (value == HUGE_VALF) { // overflow
            return {};
        }
        if (value < 0.0f) { // must be positive and greater than 0
            return {};
        }

        result[key - 1] = CalChart::Seconds{ value };
        input.remove_prefix(floatEnd - tmp.c_str());

        // Strip whitespace, then expect ',' or end
        auto afterVal = input.find_first_not_of(" \t");
        if (afterVal == std::string_view::npos) {
            break;
        }
        input.remove_prefix(afterVal);

        if (input.front() == ',') {
            input.remove_prefix(1);
        } else {
            return {};
        }
    }

    return result;
}

auto ToString(Fermatas const& input) -> std::string
{
    std::string result;
    for (auto&& [beat, duration] : input) {
        result += std::format("{}={}, ", beat + 1, duration.count());
    }
    return result;
}

auto ToFileData(const std::filesystem::path& path) -> std::optional<FileData>
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return std::nullopt;
    }

    const auto size = file.tellg();
    if (size < 0) {
        return std::nullopt;
    }

    std::vector<std::byte> buffer(static_cast<size_t>(size));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    if (!file) {
        return std::nullopt;
    }

    return FileData{ buffer, path.filename().string() };
}

auto ToFileData(CalChart::Reader reader) -> FileData
{
    auto table = reader.ParseOutLabels();
    auto dataIter = std::find_if(table.begin(), table.end(), [](auto&& entry) { return std::get<0>(entry) == INGL_DATA; });
    auto nameIter = std::find_if(table.begin(), table.end(), [](auto&& entry) { return std::get<0>(entry) == INGL_NAME; });
    if (dataIter == table.end() || nameIter == table.end()) {
        throw CC_FileException("missing required data or name chunk");
    }
    auto data = std::get<1>(*dataIter).GetVector<std::byte>();
    auto name = std::get<1>(*nameIter).Get<std::string>();

    return FileData{ data, name };
}

auto SerializeFileData(CalChart::FileData const& fileData) -> std::vector<std::byte>
{
    // Write Name
    std::vector<std::byte> tdata;
    Parser::Append(tdata, static_cast<uint32_t>(std::get<0>(fileData).size()));
    Parser::Append(tdata, std::get<0>(fileData));
    std::vector<std::byte> result = Parser::Construct_block(INGL_DATA, tdata);

    // Write Name
    std::vector<std::byte> tstring;
    Parser::AppendAndNullTerminate(tstring, std::get<1>(fileData));
    Parser::Append(result, Parser::Construct_block(INGL_NAME, tstring));

    return result;
}

}

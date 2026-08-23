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
#include <cppcodec/base64_rfc4648.hpp>
#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace {

auto IsValidUTF8(std::string const& s) -> bool
{
    auto i = 0UL;
    auto len = s.size();
    while (i < len) {
        auto c = static_cast<unsigned char>(s[i]);
        auto extra = 0;

        if (c <= 0x7F) {
            extra = 0; // 0xxxxxxx
        } else if ((c & 0xE0) == 0xC0) {
            extra = 1; // 110xxxxx
        } else if ((c & 0xF0) == 0xE0) {
            extra = 2; // 1110xxxx
        } else if ((c & 0xF8) == 0xF0) {
            extra = 3; // 11110xxx
        } else {
            return false; // invalid leading byte
        }

        if (i + extra >= len) {
            return false;
        }

        for (int j = 1; j <= extra; ++j) {
            auto cc = static_cast<unsigned char>(s[i + j]);
            if ((cc & 0xC0) != 0x80) {
                return false; // must be 10xxxxxx
            }
        }
        i += extra + 1;
    }
    return true;
}

// CP1252 bytes 0x80-0x9F map to these Unicode codepoints
// (0xA0-0xFF in CP1252 match Latin-1/Unicode directly, so no table needed there)
static constexpr uint32_t cp1252_0x80_0x9F[32] = { 0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
    0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F, 0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022,
    0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178 };

void AppendUTF8(std::string& out, uint32_t cp)
{
    if (cp <= 0x7F) {
        out += static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

std::string CP1252ToUTF8(std::string const& s)
{
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        if (c < 0x80) {
            out += static_cast<char>(c);
        } else if (c >= 0xA0) {
            AppendUTF8(out, c); // CP1252 0xA0-0xFF == Unicode 0xA0-0xFF (Latin-1)
        } else {
            AppendUTF8(out, cp1252_0x80_0x9F[c - 0x80]);
        }
    }
    return out;
}

}

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
    auto dataIter
        = std::find_if(table.begin(), table.end(), [](auto&& entry) { return std::get<0>(entry) == INGL_DATA; });
    auto nameIter
        = std::find_if(table.begin(), table.end(), [](auto&& entry) { return std::get<0>(entry) == INGL_NAME; });
    if (dataIter == table.end() || nameIter == table.end()) {
        throw CC_FileException("missing required data or name chunk");
    }
    auto data = std::get<1>(*dataIter).GetVector<std::byte>();
    auto name = std::get<1>(*nameIter).Get<std::string>();

    return FileData{ data, name };
}

auto ToFileData(nlohmann::json const& json) -> FileData
{
    if (!json.is_object()) {
        throw std::invalid_argument("FileData JSON must be an object");
    }

    if (!json.contains("name") || !json.contains("data")) {
        throw std::invalid_argument("FileData JSON missing required fields");
    }

    auto name = json.at("name").get<std::string>();
    auto data = DecodeBase64(json.at("data").get<std::string>());

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

auto FileDataToJSON(FileData const& fileData) -> nlohmann::json
{
    return nlohmann::json{ { "name", std::get<1>(fileData) }, { "data", EncodeBase64(std::get<0>(fileData)) } };
}

auto EncodeBase64(std::vector<std::byte> const& data) -> std::string
{
    if (data.empty()) {
        return "";
    }
    auto* ptr = reinterpret_cast<unsigned char const*>(data.data());

    return cppcodec::base64_rfc4648::encode(ptr, data.size());
}

auto DecodeBase64(std::string const& encoded) -> std::vector<std::byte>
{
    if (encoded.empty()) {
        return {};
    }

    auto decoded = cppcodec::base64_rfc4648::decode(encoded);
    std::vector<std::byte> result;
    result.reserve(decoded.size());
    std::transform(decoded.begin(), decoded.end(), std::back_inserter(result),
        [](unsigned char c) { return static_cast<std::byte>(c); });
    return result;
}

auto SanitizeToUTF8(std::string const& raw) -> std::string
{
    if (IsValidUTF8(raw)) {
        return raw; // already good, no conversion needed
    }
    return CP1252ToUTF8(raw); // assume CP1252, convert
}
}

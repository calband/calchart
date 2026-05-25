/*
 * CalChartDebugExport.cpp
 * Export debug information for troubleshooting
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

#include "CalChartDebugExport.hpp"
#include "CalChartAnimation.h"
#include "CalChartShow.h"
#include "ccvers.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <zlib.h>

namespace CalChart {

auto DisplayInfo::toJSON() const -> nlohmann::json
{
    nlohmann::json j;
    j["dpi_scale"] = dpi_scale;
    j["screen_width"] = screen_width;
    j["screen_height"] = screen_height;
    j["os_name"] = os_name;
    j["os_version"] = os_version;
    return j;
}

auto DebugExportData::toJSON() const -> nlohmann::json
{
    nlohmann::json j;
    j["format_version"] = FORMAT_VERSION;
    j["timestamp"] = timestamp;
    j["calchart_version"] = calchart_version;
    j["build_type"] = build_type;
    j["compiler_info"] = compiler_info;
    j["display_info"] = display_info.toJSON();

    j["show_summary"] = nlohmann::json{
        { "num_sheets", show_summary.num_sheets },
        { "num_marchers", show_summary.num_marchers },
        { "show_mode", show_summary.show_mode }
    };

    j["animation_data"] = animation_data;

    return j;
}

auto DebugExportData::toString() const -> std::string
{
    return toJSON().dump(2); // Pretty print with 2-space indent
}

auto DebugExportData::toCompressedBytes() const -> std::vector<unsigned char>
{
    // Get the JSON string
    auto jsonStr = toString();

    // Initialize zlib stream for gzip compression
    z_stream stream{};
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;

    // Initialize deflate with gzip encoding (windowBits + 16 for gzip format)
    constexpr int GZIP_ENCODING = 16;
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return {}; // Return empty vector on error
    }

    // Set input
    stream.avail_in = static_cast<uInt>(jsonStr.size());
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(jsonStr.data()));

    // Prepare output buffer (compressed data is typically smaller)
    std::vector<unsigned char> compressed;
    compressed.reserve(jsonStr.size() / 2); // Start with half the size

    // Compress in chunks
    constexpr size_t CHUNK_SIZE = 16384;
    std::vector<unsigned char> tempBuffer(CHUNK_SIZE);

    int ret;
    do {
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = tempBuffer.data();

        ret = deflate(&stream, Z_FINISH);

        if (ret != Z_STREAM_ERROR) {
            size_t have = CHUNK_SIZE - stream.avail_out;
            compressed.insert(compressed.end(), tempBuffer.data(), tempBuffer.data() + have);
        }
    } while (ret != Z_STREAM_END);

    deflateEnd(&stream);

    return compressed;
}

auto DebugExportData::Create(Show const& show, Animation const& animation, DisplayInfo const& displayInfo) -> DebugExportData
{
    DebugExportData data;

    // Timestamp (ISO 8601 format)
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%dT%H:%M:%SZ");
    data.timestamp = oss.str();

    // Version info
    data.calchart_version = CC_VERSION;
#ifdef NDEBUG
    data.build_type = "Release";
#else
    data.build_type = "Debug";
#endif

    // Compiler info
#if defined(__clang__)
    data.compiler_info = "Clang " + std::string(__clang_version__);
#elif defined(__GNUC__)
    data.compiler_info = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    data.compiler_info = "MSVC " + std::to_string(_MSC_VER);
#else
    data.compiler_info = "Unknown";
#endif

    // Display info
    data.display_info = displayInfo;

    // Show summary
    data.show_summary.num_sheets = show.GetNumSheets();
    data.show_summary.num_marchers = show.GetNumPoints();

    // Show mode - provide field dimensions
    auto showMode = show.GetShowMode();
    auto fieldSize = showMode.FieldSize();
    data.show_summary.show_mode = std::to_string(fieldSize.x) + "x" + std::to_string(fieldSize.y);

    // Animation data - export all cached marcher info
    nlohmann::json animData;
    animData["sheets"] = nlohmann::json::array();

    for (size_t sheetIdx = 0; sheetIdx < show.GetNumSheets(); ++sheetIdx) {
        nlohmann::json sheetData;
        sheetData["sheet_index"] = sheetIdx;
        sheetData["sheet_name"] = show.GetSheetName(sheetIdx);

        auto numBeats = show.GetSheetBeats(sheetIdx);
        auto beatOffset = animation.GetBeatForShowSheet(sheetIdx);

        sheetData["beats"] = numBeats;
        sheetData["beat_offset"] = beatOffset;
        sheetData["marchers"] = nlohmann::json::array();

        // For each marcher
        for (MarcherIndex marcherIdx = 0; marcherIdx < show.GetNumPoints(); ++marcherIdx) {
            nlohmann::json marcherData;
            marcherData["marcher_index"] = marcherIdx;
            marcherData["marcher_label"] = show.GetPointLabel(marcherIdx);
            marcherData["marcher_instrument"] = show.GetPointInstrument(marcherIdx);
            marcherData["beats_info"] = nlohmann::json::array();

            // For each beat in this sheet
            for (Beats beat = 0; beat <= numBeats; ++beat) {
                auto globalBeat = beatOffset + beat;
                auto info = animation.GetAnimateInfo(marcherIdx, globalBeat);

                nlohmann::json beatInfo;
                beatInfo["beat"] = beat;
                beatInfo["global_beat"] = globalBeat;
                beatInfo["position"] = {
                    { "x", info.mMarcherInfo.mPosition.x },
                    { "y", info.mMarcherInfo.mPosition.y }
                };
                beatInfo["facing_direction"] = info.mMarcherInfo.mFacingDirection.getValue();
                beatInfo["step_style"] = static_cast<int>(info.mMarcherInfo.mStepStyle);
                beatInfo["collision"] = static_cast<int>(info.mCollision);

                marcherData["beats_info"].push_back(beatInfo);
            }

            sheetData["marchers"].push_back(marcherData);
        }

        animData["sheets"].push_back(sheetData);
    }

    data.animation_data = animData;

    return data;
}

} // namespace CalChart

#pragma once
/*
 * CalChartSheet.h
 * Definitions for the calchart sheet classes
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

/**
 * CalChart Sheet
 *
 *  The CalChart sheet object is a collection of CC_point locations, the number of beats and the different marcher's continuity.
 *
 */

#include "CalChartAnimation.h"
#include "CalChartContinuity.h"
#include "CalChartFileFormat.h"
#include "CalChartImage.h"
#include "CalChartPoint.h"
#include "CalChartText.h"
#include "CalChartTypes.h"

#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

namespace CalChart {

class Continuity;
class Reader;
struct ParseErrorHandlers;

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
class Sheet {
public:
    Sheet(size_t numPoints);
    Sheet(size_t numPoints, std::string const& newname);
    // intentionally a reference to Reader.
    Sheet(Version_3_3_and_earlier, size_t numPoints, Reader&, ParseErrorHandlers const* correction = nullptr);
    Sheet(size_t numPoints, Reader, ParseErrorHandlers const* correction = nullptr);
    ~Sheet();

private:
    auto SerializeAllPoints() const -> std::vector<std::byte>;
    auto SerializeContinuityData() const -> std::vector<std::byte>;
    auto SerializePrintContinuityData() const -> std::vector<std::byte>;
    auto SerializeBackgroundImageInfo() const -> std::vector<std::byte>;
    auto SerializeSheetData() const -> std::vector<std::byte>;

public:
    auto SerializeSheet() const -> std::vector<std::byte>;

    // continuity Functions
    [[nodiscard]] auto GetContinuityBySymbol(SYMBOL_TYPE i) const
    {
        return mAnimationContinuity.at(i);
    }
    [[nodiscard]] auto GetContinuities() const
    {
        return k_symbols | std::views::transform([this](auto symbol) { return GetContinuityBySymbol(symbol); });
    }
    [[nodiscard]] auto ContinuityInUse(SYMBOL_TYPE idx) const -> bool;
    [[nodiscard]] auto ContinuitiesInUse() const
    {
        return k_symbols | std::views::transform([this](auto symbol) { return ContinuityInUse(symbol); });
    }
    void SetContinuity(SYMBOL_TYPE sym, Continuity const& new_cont);

    // print continuity
    void SetPrintableContinuity(std::string const& name, std::string const& lines);
    Textline_list GetPrintableContinuity() const;
    std::string GetRawPrintContinuity() const;
    auto GetPrintContinuity() const { return mPrintableContinuity; }

    // beats
    unsigned short GetBeats() const;
    void SetBeats(unsigned short b);
    bool IsInAnimation() const { return (GetBeats() != 0); }

    // points
    Point const& GetPoint(unsigned i) const;
    Point& GetPoint(unsigned i);

    auto GetAllPoints() const { return mPoints; }

    SYMBOL_TYPE GetSymbol(unsigned i) const;
    void SetSymbol(unsigned i, SYMBOL_TYPE sym);
    std::vector<Point> GetPoints() const;
    auto GetNumberPoints() const { return mPoints.size(); }
    std::vector<SYMBOL_TYPE> GetSymbols() const;
    void SetPoints(std::vector<Point> const& points);
    [[nodiscard]] auto FindMarcher(Coord where, Coord::units searchBound, unsigned ref = 0) const -> std::optional<int>;
    std::vector<Point> RemapPoints(std::vector<size_t> const& table) const;
    Coord GetPosition(unsigned i, unsigned ref = 0) const;
    void SetAllPositions(Coord val, unsigned i);
    void SetPosition(Coord val, unsigned i, unsigned ref = 0);
    void SetAllPoints(std::vector<Point> const& newpts);
    SelectionList MakeSelectPointsBySymbol(SYMBOL_TYPE i) const;
    std::vector<Point> NewNumPointsPositions(int num, int columns, Coord new_march_position) const;
    void DeletePoints(SelectionList const& sl);

    // titles
    std::string GetName() const;
    void SetName(std::string const& newname);
    std::string GetNumber() const;
    void SetNumber(std::string const& newnumber);

    // image
    std::vector<ImageInfo> const& GetBackgroundImages() const;
    void AddBackgroundImage(ImageInfo const& image, size_t where);
    void RemoveBackgroundImage(size_t which);
    void MoveBackgroundImage(size_t which, int left, int top, int scaled_width, int scaled_height);

    [[nodiscard]] auto ShouldPrintLandscape() const -> bool;

    /*!
     * @brief Generates a JSON that could represent this
     * sheet in an Online Viewer '.viewer' file.
     * @param sheetNum The index of this sheet (NOT zero-indexed;
     * the first sheet in the show should have a sheetNum of 1).
     * @param dotLabels A vector which maps each dot index to its
     * label (e.g. a valid label for dot 1 would be A1).
     * @param compiledSheet An up-to-date animation of this sheet.
     * @return A JSON which could represent this sheet in
     * a '.viewer' file.
     */
    nlohmann::json toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, std::map<std::string, std::vector<nlohmann::json>> const& movements) const;

private:
    std::vector<Continuity> mAnimationContinuity;
    PrintContinuity mPrintableContinuity;
    unsigned short mBeats;
    std::vector<Point> mPoints;
    std::string mName;
    std::vector<ImageInfo> mBackgroundImages;

    // unit tests
    friend void Sheet_UnitTests();
    static void sheet_round_trip_test();
};

void Sheet_UnitTests();
}

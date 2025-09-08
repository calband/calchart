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

#include "CalChartConstants.h"
#include "CalChartContinuity.h"
#include "CalChartCoord.h"
#include "CalChartFileFormat.h"
#include "CalChartImage.h"
#include "CalChartPoint.h"
#include "CalChartText.h"
#include "CalChartTypes.h"

#include <cstddef>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace CalChart {

class Continuity;
class Reader;
class Curve;
struct ParseErrorHandlers;

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
class Sheet {
public:
    explicit Sheet(size_t numPoints);
    Sheet(size_t numPoints, std::string name);
    // intentionally a reference to Reader.
    Sheet(Version_3_3_and_earlier, size_t numPoints, Reader&, ParseErrorHandlers const* correction = nullptr);
    Sheet(size_t numPoints, Reader, ParseErrorHandlers const* correction = nullptr);

private:
    [[nodiscard]] auto SerializeAllPoints() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializeContinuityData() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializePrintContinuityData() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializeBackgroundImageInfo() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializeCurves() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializeCurveAssigments() const -> std::vector<std::byte>;
    [[nodiscard]] auto SerializeSheetData() const -> std::vector<std::byte>;

public:
    [[nodiscard]] auto SerializeSheet() const -> std::vector<std::byte>;

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
    [[nodiscard]] auto GetPrintableContinuity() const -> Textline_list;
    [[nodiscard]] auto GetPrintNumber() const -> std::string;
    [[nodiscard]] auto GetRawPrintContinuity() const -> std::string;
    [[nodiscard]] auto GetPrintContinuity() const { return mPrintableContinuity; }

    // beats
    [[nodiscard]] auto GetBeats() const { return mBeats; }
    void SetBeats(Beats b) { mBeats = b; }
    [[nodiscard]] auto IsInAnimation() const { return (GetBeats() != 0); }

    // Marchers
    [[nodiscard]] auto GetMarcher(MarcherIndex i) const -> Point;
    [[nodiscard]] auto GetAllMarchers() const { return mPoints; }
    [[nodiscard]] auto GetSymbol(MarcherIndex i) const { return mPoints[i].GetSymbol(); }
    void SetSymbol(MarcherIndex i, SYMBOL_TYPE sym);
    [[nodiscard]] auto GetNumberPoints() const { return mPoints.size(); }
    [[nodiscard]] auto GetSymbols() const -> std::vector<SYMBOL_TYPE>;
    void SetPoints(std::vector<Point> const& points);
    [[nodiscard]] auto FindMarcher(Coord where, Coord::units searchBound, unsigned ref = 0) const -> std::optional<MarcherIndex>;
    [[nodiscard]] auto RemapPoints(std::vector<MarcherIndex> const& table) const -> std::vector<Point>;
    [[nodiscard]] auto GetMarcherPosition(MarcherIndex i, unsigned ref = 0) const -> Coord;
    [[nodiscard]] auto GetAllMarcherPositions(unsigned ref = 0) const -> std::vector<Coord>;
    void SetMarchers(std::vector<Point> const& points);
    void SetAllPositions(Coord val, unsigned i);
    void SetPosition(Coord val, MarcherIndex i, unsigned ref = 0);
    void SetMarcherFlip(MarcherIndex i, bool val);
    void SetMarcherLabelVisibility(MarcherIndex i, bool isVisible);
    [[nodiscard]] auto MakeSelectPointsBySymbol(SYMBOL_TYPE i) const -> SelectionList;
    [[nodiscard]] auto NewNumPointsPositions(int num, int columns, Coord new_march_position) const -> std::vector<Point>;
    void DeletePoints(SelectionList const& sl);

    // Curves
    void AddCurve(Curve const& curve, size_t index);
    void RemoveCurve(size_t index);
    void ReplaceCurve(Curve const& curve, size_t index);
    [[nodiscard]] auto GetCurve(size_t index) const -> Curve;
    [[nodiscard]] auto GetNumberCurves() const -> size_t;
    [[nodiscard]] auto FindCurveControlPoint(Coord where, Coord::units searchBound) const -> std::optional<std::tuple<size_t, size_t>>;
    [[nodiscard]] auto FindCurve(Coord where, Coord::units searchBound) const -> std::optional<std::tuple<size_t, size_t, double>>;
    // Curve assignemnts: the idea is some marchers are assigned to curves.  The sheet will automatically determine
    // where they should be.  You can get and set all the curve assignements.
    // But if you want to assign a group of marchers to a curve, you ask first what would be the new assignments
    // if you were to make that change (because some marchers will be moved from 1 curve to another if you do so)
    // and then you would set the sheet with that result.
    [[nodiscard]] auto GetCurveAssignments() const -> std::vector<std::vector<MarcherIndex>>;
    void SetCurveAssignment(std::vector<std::vector<MarcherIndex>> curveAssignments);
    [[nodiscard]] auto GetCurveAssignmentsWithNewAssignments(size_t whichCurve, std::vector<MarcherIndex> whichMarchers) const -> std::vector<std::vector<MarcherIndex>>;

    // titles
    [[nodiscard]] auto GetName() const -> std::string;
    void SetName(std::string const& newname);

    // image
    [[nodiscard]] auto GetBackgroundImages() const -> std::vector<ImageInfo>
    {
        return mBackgroundImages;
    }
    [[nodiscard]] auto GetBackgroundImage(size_t which) const -> ImageInfo
    {
        return mBackgroundImages.at(which);
    }
    [[nodiscard]] auto GetNumberBackgroundImages() const { return mBackgroundImages.size(); }
    [[nodiscard]] auto GetBackgroundImageInfo(size_t which) const -> std::array<int, 4>
    {
        return { mBackgroundImages.at(which).left,
            mBackgroundImages.at(which).top,
            mBackgroundImages.at(which).scaledWidth,
            mBackgroundImages.at(which).scaledHeight };
    }
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
    [[nodiscard]] auto toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, std::map<std::string, std::vector<nlohmann::json>> const& movements) const -> nlohmann::json;

    // Draw Commands
    // the sheet can generate all the elements related to sheet specific draw aspects
    [[nodiscard]] auto GenerateGhostElements(CalChart::Configuration const& config, SelectionList const& selected, std::vector<std::string> const& marcherLabels) const -> std::vector<CalChart::Draw::DrawCommand>;
    [[nodiscard]] auto GenerateSheetElements(CalChart::Configuration const& config, SelectionList const& selected, std::vector<std::string> const& marcherLabels, int referencePoint) const -> std::vector<CalChart::Draw::DrawCommand>;

private:
    std::vector<Continuity> mAnimationContinuity;
    PrintContinuity mPrintableContinuity;
    Beats mBeats;
    std::vector<Point> mPoints;
    std::string mName;
    std::vector<ImageInfo> mBackgroundImages;
    std::vector<std::pair<Curve, std::vector<MarcherIndex>>> mCurves; // curves and the points assigned to them.

    void RepositionCurveMarchers();
    void SetPositionHelper(Coord val, MarcherIndex i, unsigned ref = 0);
    void UnassignMarchersFromAnyCurve(std::vector<MarcherIndex> marchers);
};

}

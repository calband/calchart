#pragma once
/*
 * cc_sheet.h
 * Definitions for the calchart sheet classes
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "animate.h"
#include "cc_continuity.h"
#include "cc_fileformat.h"
#include "cc_image.h"
#include "cc_point.h"
#include "cc_text.h"
#include "cc_types.h"

#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

// CalChart Sheet
// The CalChart sheet object is a collection of CC_point locations, the number
// of
// beats and the different marcher's continuity.

namespace CalChart {

class Continuity;
struct ParseErrorHandlers;

// error occurred on parsing.  First arg is what went wrong, second is the values that need to be fixed.
class Sheet {
public:
    Sheet(size_t numPoints);
    Sheet(size_t numPoints, const std::string& newname);
    Sheet(size_t numPoints, std::istream& stream, ParseErrorHandlers const* correction = nullptr);
    Sheet(size_t numPoints, const uint8_t* ptr, size_t size, ParseErrorHandlers const* correction = nullptr);
    ~Sheet();

private:
    std::vector<uint8_t> SerializeAllPoints() const;
    std::vector<uint8_t> SerializeContinuityData() const;
    std::vector<uint8_t> SerializePrintContinuityData() const;
    std::vector<uint8_t> SerializeBackgroundImageData() const;
    std::vector<uint8_t> SerializeSheetData() const;

public:
    std::vector<uint8_t> SerializeSheet() const;

    // continuity Functions
    const Continuity& GetContinuityBySymbol(SYMBOL_TYPE i) const;
    bool ContinuityInUse(SYMBOL_TYPE idx) const;
    void SetContinuity(SYMBOL_TYPE sym, Continuity const& new_cont);

    // print continuity
    void SetPrintableContinuity(const std::string& name, const std::string& lines);
    Textline_list GetPrintableContinuity() const;
    std::string GetRawPrintContinuity() const;

    // beats
    unsigned short GetBeats() const;
    void SetBeats(unsigned short b);
    bool IsInAnimation() const { return (GetBeats() != 0); }

    // points
    const Point& GetPoint(unsigned i) const;
    Point& GetPoint(unsigned i);
    SYMBOL_TYPE GetSymbol(unsigned i) const;
    void SetSymbol(unsigned i, SYMBOL_TYPE sym);
    std::vector<Point> GetPoints() const;
    std::vector<SYMBOL_TYPE> GetSymbols() const;
    void SetPoints(const std::vector<Point>& points);
    int FindPoint(Coord where, Coord::units searchBound, unsigned ref = 0) const;
    std::vector<Point> RemapPoints(const std::vector<size_t>& table) const;
    Coord GetPosition(unsigned i, unsigned ref = 0) const;
    void SetAllPositions(const Coord& val, unsigned i);
    void SetPosition(const Coord& val, unsigned i, unsigned ref = 0);
    void SetAllPoints(std::vector<Point> const& newpts);
    SelectionList MakeSelectPointsBySymbol(SYMBOL_TYPE i) const;
    std::vector<Point> NewNumPointsPositions(int num, int columns, const Coord& new_march_position) const;
    void DeletePoints(SelectionList const& sl);

    // titles
    std::string GetName() const;
    void SetName(const std::string& newname);
    std::string GetNumber() const;
    void SetNumber(const std::string& newnumber);

    // image
    std::vector<ImageData> const& GetBackgroundImages() const;
    void AddBackgroundImage(ImageData const& image, size_t where);
    void RemoveBackgroundImage(size_t which);
    void MoveBackgroundImage(size_t which, int left, int top, int scaled_width, int scaled_height);

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
    nlohmann::json toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, const AnimateSheet& compiledSheet) const;

private:
    std::vector<Continuity> mAnimationContinuity;
    Print_continuity mPrintableContinuity;
    unsigned short mBeats;
    std::vector<Point> mPoints;
    std::string mName;
    std::vector<ImageData> mBackgroundImages;

    // unit tests
    friend void Sheet_UnitTests();
    static void sheet_round_trip_test();
};

void Sheet_UnitTests();
}

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

#pragma once

#include "cc_types.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "cc_text.h"
#include "cc_fileformat.h"
#include "json.h"
#include "animate.h"
#include "cc_image.h"

#include <vector>
#include <set>
#include <string>

class CC_show;
class CC_coord;
class CC_continuity;
class CC_point;

// CalChart Sheet
// The CalChart sheet object is a collection of CC_point locations, the number
// of
// beats and the different marcher's continuity.

class CC_sheet {
public:
    CC_sheet(size_t numPoints);
    CC_sheet(size_t numPoints, const std::string& newname);
    CC_sheet(size_t numPoints, std::istream& stream, Version_3_3_and_earlier);
    CC_sheet(size_t numPoints, const uint8_t* ptr, size_t size,
        Current_version_and_later);
    ~CC_sheet();

private:
    std::vector<uint8_t> SerializeAllPoints() const;
    std::vector<uint8_t> SerializeContinuityData() const;
    std::vector<uint8_t> SerializePrintContinuityData() const;
    std::vector<uint8_t> SerializeBackgroundImageData() const;
    std::vector<uint8_t> SerializeSheetData() const;

public:
    std::vector<uint8_t> SerializeSheet() const;

    // continuity Functions
    const CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i) const;
    CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i);
    bool ContinuityInUse(SYMBOL_TYPE idx) const;
    void SetContinuityText(SYMBOL_TYPE sym, const std::string& text);

    // print continuity
    void SetPrintableContinuity(const std::string& name, const std::string& lines);
    CC_textline_list GetPrintableContinuity() const;
    std::string GetRawPrintContinuity() const;

    // beats
    unsigned short GetBeats() const;
    void SetBeats(unsigned short b);
    bool IsInAnimation() const { return (GetBeats() != 0); }

    // points
    const CC_point& GetPoint(unsigned i) const;
    CC_point& GetPoint(unsigned i);
    std::vector<CC_point> GetPoints() const;
    void SetPoints(const std::vector<CC_point>& points);
    int FindPoint(Coord x, Coord y, Coord searchBound, unsigned ref = 0) const;
    std::vector<CC_point> RemapPoints(const std::vector<size_t>& table) const;
    CC_coord GetPosition(unsigned i, unsigned ref = 0) const;
    void SetAllPositions(const CC_coord& val, unsigned i);
    void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);
    void SetAllPoints(std::vector<CC_point> const& newpts);
    SelectionList MakeSelectPointsBySymbol(SYMBOL_TYPE i) const;
    std::vector<CC_point> NewNumPointsPositions(unsigned num, unsigned columns, const CC_coord& new_march_position) const;

    // titles
    std::string GetName() const;
    void SetName(const std::string& newname);
    std::string GetNumber() const;
    void SetNumber(const std::string& newnumber);

    // image
    std::vector<calchart_core::ImageData> const& GetBackgroundImages() const;
    void AddBackgroundImage(calchart_core::ImageData const& image, size_t where);
    void RemoveBackgroundImage(size_t which);
    void MoveBackgroundImage(size_t which, int left, int top, int scaled_width, int scaled_height);

    /*!
     * @brief Generates a JSONElement that could represent this
     * sheet in an Online Viewer '.viewer' file.
     * @param sheetNum The index of this sheet (NOT zero-indexed;
     * the first sheet in the show should have a sheetNum of 1).
     * @param dotLabels A vector which maps each dot index to its
     * label (e.g. a valid label for dot 1 would be A1).
     * @param compiledSheet An up-to-date animation of this sheet.
     * @return A JSONElement which could represent this sheet in
     * a '.viewer' file.
     */
    JSONElement toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, const AnimateSheet& compiledSheet) const;

    /*!
     * @brief Manipulates dest so that it contains a JSONElement that
     * could represent this sheet in an Online Viewer '.viewer' file.
     * @param dest A reference to the JSONElement which will be transformed
     * into a JSON representation of this sheet.
     * @param sheetNum The index of this sheet (NOT zero-indexed;
     * the first sheet in the show should have a sheetNum of 1).
     * @param dotLabels A vector which maps each dot index to its
     * label (e.g. a valid label for dot 1 would be A1).
     * @param compiledSheet An up-to-date animation of this sheet.
     */
    void toOnlineViewerJSON(JSONElement& dest, unsigned sheetNum, std::vector<std::string> dotLabels, const AnimateSheet& compiledSheet) const;

private:
    std::vector<CC_continuity> mAnimationContinuity;
    CC_print_continuity mPrintableContinuity;
    unsigned short mBeats;
    std::vector<CC_point> mPoints;
    std::string mName;
    std::vector<calchart_core::ImageData> mBackgroundImages;

    // unit tests
    friend void CC_sheet_UnitTests();
    static void CC_sheet_round_trip_test();
};

void CC_sheet_UnitTests();

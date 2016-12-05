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

    // Observer functions
    const CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i) const;
    std::set<unsigned> SelectPointsBySymbol(SYMBOL_TYPE i) const;
    bool ContinuityInUse(SYMBOL_TYPE idx) const;

    // setting values on the stunt sheet
    // * needs to be through command only *
    void SetNumPoints(unsigned num, unsigned columns,
        const CC_coord& new_march_position);

    // continuity:
    // * needs to be through command only *
    void SetContinuityText(SYMBOL_TYPE sym, const std::string& text);

    // points:
    int FindPoint(Coord x, Coord y, Coord searchBound, unsigned ref = 0) const;
    void RelabelSheet(const std::vector<size_t>& table);

    std::string GetName() const;
    void SetName(const std::string& newname);
    std::string GetNumber() const;
    void SetNumber(const std::string& newnumber);

    // beats
    unsigned short GetBeats() const;
    void SetBeats(unsigned short b);
    bool IsInAnimation() const { return (GetBeats() != 0); }

    const CC_point& GetPoint(unsigned i) const;
    CC_point& GetPoint(unsigned i);
    std::vector<CC_point> GetPoints() const;
    void SetPoints(const std::vector<CC_point>& points);

    CC_coord GetPosition(unsigned i, unsigned ref = 0) const;
    void SetAllPositions(const CC_coord& val, unsigned i);
    void SetPosition(const CC_coord& val, unsigned i, unsigned ref = 0);

    // continuity that gets printed
    void SetPrintableContinuity(const std::string& name,
        const std::string& lines);
    CC_textline_list GetPrintableContinuity() const;
    std::string GetRawPrintContinuity() const;

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

	struct ImageData {
		int left, top;
		int scaled_width, scaled_height;
		int image_width, image_height;
		std::vector<unsigned char> data;
		std::vector<unsigned char> alpha;
		ImageData(int left, int top, int scaled_width, int scaled_height, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha);
		ImageData(uint8_t const* &d);
		std::vector<uint8_t> Serialize() const;
	};

	std::vector<ImageData> GetBackgroundImages() const;
	void AddBackgroundImages(ImageData const& image);
	void RemoveBackgroundImage(int which);
	void MoveBackgroundImage(int which, int left, int top, int scaled_width, int scaled_height);

private:
    CC_continuity& GetContinuityBySymbol(SYMBOL_TYPE i);

    typedef std::vector<CC_continuity> ContContainer;
    ContContainer mAnimationContinuity;

    CC_print_continuity mPrintableContinuity;
    unsigned short beats;
    std::vector<CC_point> pts;
    std::string mName;

	std::vector<ImageData> mBackgroundImages;

    // unit tests
    friend void CC_sheet_UnitTests();
    static void CC_sheet_round_trip_test();
};

void CC_sheet_UnitTests();
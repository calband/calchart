//
//  viewer_translator.h
//  CalChart
//
//  Created by Kevin Durand on 1/5/16.
//
//

#pragma once

#include "cc_coord.h"

/*!
 * @brief A collection of methods used for translating values
 * to their CalChart Online Viewer representations.
 */
class ToOnlineViewer {
public:
    static float xPosition(Coord coord);
    static float yPosition(Coord coord);
    static float angle(float angle);
    static std::string symbolName(SYMBOL_TYPE symbol);
};

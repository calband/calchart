/*!
 * @file viewer_translate.h
 * @brief Contains utilities for translating values to
 * their representations in the CalChart Online Viewer.
 */
//

#pragma once

#include "cc_coord.h"

/*!
 * @brief A collection of methods used for translating values
 * to their CalChart Online Viewer representations.
 */
class ToOnlineViewer {
public:
    /*!
     * @brief Translates a CalChart field x-coordinate
     * to a CalChart Online Viewer field x-coordinate.
     * @details The origin for CalChart coordinates is
     * found at the southwest corner of the field;
     * x-coordinates increase in the northern direction
     * at a rate of one unit per step. CalChart Online
     * Viewer coordinates originate at the center of
     * the field, with x-coordinates increasing in the
     * northern direction at a rate of one unit per step.
     * @param coord A CalChart field x-coordinate.
     * @return An Online Viewer field x-coordinate.
     */
    static float xPosition(Coord coord);
    
    /*!
     * @brief Translates a CalChart field y-coordinate
     * to a CalChart Online Viewer field y-coordinate.
     * @details The origin for CalChart coordinates is
     * found at the southwest corner of the field;
     * y-coordinates increase in the eastward direction
     * at a rate of one unit per step. CalChart
     * Online Viewer coordinates originate at
     * the center of the field, with y-coordinates
     * increasing in the eastward direction at a rate
     * of one unit per step.
     * @param coord A CalChart field y-coordinate.
     * @return An Online Viewer field y-coordinate.
     */
    static float yPosition(Coord coord);
    
    /*!
     * @brief Translates a CalChart angle to a
     * CalChart Online Viewer angle.
     * @details Angles in CalChart are measured
     * in degrees counterclockwise from the northern
     * direction. CalChart Online Viewer angles are
     * measured in degrees clockwise from the
     * eastward direction.
     * @param angle A CalChart angle.
     * @return An Online Viewer angle.
     */
    static float angle(float angle);
    
    /*!
     * @brief Translates a CalChart enumerated
     * symbol type to its equivalent string
     * representation in the CalChart Online Viewer.
     * @param symbol A CalChart enumerated symbol
     * type.
     * @return The Online Viewer string representation
     * of the CalChart symbol type.
     */
    static std::string symbolName(SYMBOL_TYPE symbol);
};

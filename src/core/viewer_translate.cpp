/*!
 * @file viewer_translate.cpp
 * @brief Implements the utilities found in
 * viewer_translate.h.
 */

#include "viewer_translate.h"

float ToOnlineViewer::xPosition(Coord coord)
{
    return Coord2Float(coord) + (80); // TODO; the Online Viewer is only prepared for normal-sized fields, so we'll assume those dimensions for now
}

float ToOnlineViewer::yPosition(Coord coord)
{
    return Coord2Float(coord) + 42; // TODO; the Online Viewer is only prepared for normal-sized fields, so we'll assume those dimensions for now
}

float ToOnlineViewer::angle(float angle)
{
    angle = -(angle - 270);
    while (angle >= 360) {
        angle -= 360;
    }
    while (angle < 0) {
        angle += 360;
    }
    return angle;
}

std::string ToOnlineViewer::symbolName(SYMBOL_TYPE symbol)
{
    switch (symbol) {
    case SYMBOL_PLAIN:
        return "open";
    case SYMBOL_SOL:
        return "solid";
    case SYMBOL_BKSL:
        return "open-backslash";
    case SYMBOL_SL:
        return "open-forwardslash";
    case SYMBOL_X:
        return "open-x";
    case SYMBOL_SOLBKSL:
        return "solid-backslash";
    case SYMBOL_SOLSL:
        return "solid-forwardslash";
    case SYMBOL_SOLX:
        return "solid-x";
    default:
        return "ERR";
    }
}
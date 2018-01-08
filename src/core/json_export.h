#pragma once
/*!
 * @file json_export.h
 * @brief Defines utilities for exporting JSON files.
 */

#include "json.h"

namespace CalChart {

/*!
 * @brief A class containing a collection of methods that can be 
 * used for exporting JSON files.
 */
class JSONExporter {
public:
    static bool exportJSON(const std::string& filename, const JSONElement& json);
};
}

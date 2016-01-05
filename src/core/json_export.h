//
//  json_export.h
//  CalChart
//
//  Created by Kevin Durand on 1/4/15.
//
//

#pragma once

#include "json.h"

/*!
 * @brief A class containing a collection of methods that can be 
 * used for exporting JSON files.
 */
class JSONExporter {
public:
    static bool exportJSON(const std::string& filename, const JSONElement& json);
};
//
//  json_export.h
//  CalChart
//
//  Created by Kevin Durand on 1/4/15.
//
//

#pragma once

#include <iterator>
#include "json.h"

/*!
 @class JSONExporter
 @abstract A collection of methods used for exporting
 JSON files.
 */
class JSONExporter {
public:
    static bool exportJSON(std::ostream_iterator<char>& outIter, const JSONElement& json);
};
//
//  json_export.cpp
//  CalChart
//
//  Created by Kevin Durand on 1/4/16.
//
//

#include "json_export.h"
#include "json_grammar.h"

template <typename T>
static bool exportJSONGeneric(T& outIter, const JSONElement& json) {
    JSONExport<T> exportGenerator;
    
    return boost::spirit::karma::generate(outIter, exportGenerator, json);
}


bool JSONExporter::exportJSON(std::ostream_iterator<char>& outIter, const JSONElement& json) {
    return exportJSONGeneric(outIter, json);
}



//
//  json_export.cpp
//  CalChart
//
//  Created by Kevin Durand on 1/4/16.
//
//

#include "json_export.h"
#include "json_grammar.h"


bool JSONExporter::exportJSON(std::ostream_iterator<char>& outIter, const JSONElement& json) {
    
    JSONExport<std::ostream_iterator<char>> exportGenerator;

    return boost::spirit::karma::generate(outIter, exportGenerator, json);
}


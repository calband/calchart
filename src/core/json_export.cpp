/*!
 * @file json_export.cpp
 * @brief Implements the utilities defined in json_export.h
 * for exporting JSON files.
 */

#include <fstream>
#include <iterator>

#include "json_export.h"
#include "json_grammar.h"

namespace CalChart {

/*!
 * @brief Exports a JSON file to an arbitrary output iterator.
 * @tparam T The output iterator type.
 * @param outIter The output iterator to which the JSON file will
 * be written.
 * @param json The JSON file content, provided as a JSONElement.
 * Note that the outermost element in a JSON file must be an
 * object.
 * @return True if the JSON file was successully written to the
 * output iterator; false otherwise.
 */
template <typename T>
static bool exportJSONGeneric(T& outIter, const JSONElement& json)
{
    JSONExportGrammar<T> exportGenerator;

    return boost::spirit::karma::generate(outIter, exportGenerator, json);
}

bool JSONExporter::exportJSON(const std::string& filename, const JSONElement& json)
{
    std::ofstream outfile(filename);
    std::ostream_iterator<char> outIter = outfile;
    bool success = outfile.good() && exportJSONGeneric(outIter, json);
    outfile.close();
    return success;
}
}

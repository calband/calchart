//
//  json_grammar.h
//  CalChart
//
//  Created by Kevin Durand on 11/17/15.
//
//

#pragma once

#include <boost/spirit/include/karma_string.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "json.h"

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

template <typename OutputIterator>
struct JSONExport
: karma::grammar<OutputIterator, const JSONElement&()>
{
    karma::rule<OutputIterator, void(const JSONElement&)> checkType(JSONData::JSONDataType dataType) {
        return &karma::int_(dataType)[karma::_1 = phoenix::bind(&JSONElement::type, karma::_r1)];
    }
    
    JSONExport()
    : JSONExport::base_type(mainObject)
    {
        std::string indentation_string = "    ";
        auto indentationLevel = karma::_r1;
        auto increaseIndent = karma::eps[++indentationLevel];
        auto decreaseIndent = karma::eps[--indentationLevel];
        auto indent = karma::repeat(indentationLevel)[karma::lit(indentation_string)];
        auto begin_newline = karma::eol << indent;
        
        // Rules that will succeed only if the JSONElement is of a particular type
        isNumber = checkType(JSONData::JSONDataTypeNumber);
        isString = checkType(JSONData::JSONDataTypeString);
        isBoolean = checkType(JSONData::JSONDataTypeBoolean);
        isNull = checkType(JSONData::JSONDataTypeNull);
        isObject = checkType(JSONData::JSONDataTypeObject);
        isArray = checkType(JSONData::JSONDataTypeArray);
        
        // The outermost JSON object
        mainObject = isObject(karma::_val) << element(0);
        
        // An element which will start a new, indented line before printing
        newline_element = begin_newline << element(indentationLevel);
        
        // An arbitrary element in a JSON hierarchy
        element = inline_element | block_element(indentationLevel);
        
        // Elements that don't contain other elements
        inline_element = isNumber(karma::_val) << number
        | isString(karma::_val) << string
        | isBoolean(karma::_val) << boolean
        | isNull(karma::_val) << null;
        
        // Elements which contain other elements
        block_element = isObject(karma::_val) << object(indentationLevel)
        | isArray(karma::_val) << array(indentationLevel);
        
        // A JSON number
        number = karma::eps[karma::_a = phoenix::bind(&JSONDataNumberConstAccessor::operator->, karma::_val)] << karma::double_[karma::_1 = phoenix::bind(&JSONDataNumber::value, karma::_a)];
        
        // A JSON string
        string = karma::eps[karma::_a = phoenix::bind(&JSONDataStringConstAccessor::operator->, karma::_val)] << '"' << karma::string[karma::_1 = phoenix::bind(&JSONDataString::value, karma::_a)] << '"';
        
        // A JSON boolean
        boolean = karma::eps[karma::_a = phoenix::bind(&JSONDataBooleanConstAccessor::operator->, karma::_val)] << karma::bool_[karma::_1 = phoenix::bind(&JSONDataBoolean::value, karma::_a)];
        
        // A JSON null
        null = karma::lit("null");
        
        // A JSON object
        object = karma::eps[karma::_a = phoenix::bind(&JSONDataObjectConstAccessor::operator->, karma::_val)]
        << '{' << increaseIndent
        << objectContent(indentationLevel)[karma::_1 = phoenix::bind(&JSONDataObject::items, karma::_a)]
        << decreaseIndent << begin_newline << '}';
        
        // The collection of 'key:value' mappings within a JSON object
        objectContent = (keyValPair(indentationLevel) % ',');
        
        // A single 'key:value' mapping within a JSON object
        keyValPair = begin_newline << '"' << karma::string << '"' << karma::lit(": ") << element(indentationLevel);
        
        // A JSON array
        array = karma::eps[karma::_a = phoenix::bind(&JSONDataArrayConstAccessor::operator->, karma::_val)]
        << '[' << increaseIndent
        << arrayContent(indentationLevel)[karma::_1 = phoenix::bind(&JSONDataArray::values, karma::_a)]
        << decreaseIndent << begin_newline << ']';
        
        // The list of values within a JSON array
        arrayContent = (newline_element(indentationLevel) % ',');
        
        
    }
    karma::rule<OutputIterator, void(const JSONElement&)> isNumber;
    karma::rule<OutputIterator, void(const JSONElement&)> isString;
    karma::rule<OutputIterator, void(const JSONElement&)> isBoolean;
    karma::rule<OutputIterator, void(const JSONElement&)> isNull;
    karma::rule<OutputIterator, void(const JSONElement&)> isObject;
    karma::rule<OutputIterator, void(const JSONElement&)> isArray;
    
    
    
    karma::rule<OutputIterator, const JSONElement&()> mainObject;
    karma::rule<OutputIterator, const JSONElement&(unsigned)> newline_element;
    karma::rule<OutputIterator, const JSONElement&(unsigned)> element;
    karma::rule<OutputIterator, const JSONElement&()> inline_element;
    karma::rule<OutputIterator, karma::locals<const JSONDataNumber*>, JSONDataNumberConstAccessor()> number;
    karma::rule<OutputIterator, karma::locals<const JSONDataString*>,JSONDataStringConstAccessor()> string;
    karma::rule<OutputIterator, karma::locals<const JSONDataBoolean*>,JSONDataBooleanConstAccessor()> boolean;
    karma::rule<OutputIterator> null;
    karma::rule<OutputIterator, const JSONElement&(unsigned)> block_element;
    karma::rule<OutputIterator, karma::locals<const JSONDataObject*>,JSONDataObjectConstAccessor(unsigned)> object;
    karma::rule<OutputIterator, std::vector<std::pair<const std::string&, const JSONElement&>>&(unsigned)> objectContent;
    karma::rule<OutputIterator, std::pair<const std::string&, const JSONElement&>&(unsigned)> keyValPair;
    karma::rule<OutputIterator, karma::locals<const JSONDataArray*>,JSONDataArrayConstAccessor(unsigned)> array;
    karma::rule<OutputIterator, std::vector<std::reference_wrapper<const JSONElement>>&(unsigned)> arrayContent;
};
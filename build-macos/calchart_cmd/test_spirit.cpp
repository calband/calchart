/*=============================================================================
 Copyright (c) 2001-2010 Joel de Guzman
 
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 =============================================================================*/
///////////////////////////////////////////////////////////////////////////////
//
//  A mini XML-like parser
//
//  [ JDG March 25, 2007 ]   spirit2
//
///////////////////////////////////////////////////////////////////////////////

#include "new_cont_grammar.h"

#include "cc_sheet.h"
#include "cc_show.h"
#include "math_utils.h"
#include "animatecommand.h"
#include "animatecompile.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

//using pos_iterator_t = boost::spirit::line_pos_iterator<std::string::const_iterator>;
//
//namespace qi = boost::spirit::qi;
//namespace ascii = boost::spirit::ascii;


///////////////////////////////////////////////////////////////////////////
//  Our point grammar definition
///////////////////////////////////////////////////////////////////////////




namespace calchart
{
///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<CC_show> make_a_new_show()
{
	auto test_show = CC_show::Create_CC_show();
	test_show->SetupNewShow();
	CC_coord field_offset = { 0, 0 };
	test_show->SetNumPoints(1, 1, field_offset);
	CC_show::CC_sheet_iterator_t sheet = test_show->GetCurrentSheet();
	sheet->GetPoint(0).SetSymbol(SYMBOL_X);
	
	sheet->SetAllPositions(field_offset + CC_coord(Int2Coord(2), Int2Coord(2)), 0);
	sheet->SetPosition(field_offset + CC_coord(Int2Coord(4), Int2Coord(4)), 0, 2);
	sheet->SetBeats(8);
	
	test_show->InsertSheet(*sheet, 1);
	test_show->SetCurrentSheet(0);
	sheet = test_show->GetCurrentSheet();
	++sheet;
	sheet->SetAllPositions(field_offset + CC_coord(Int2Coord(6), Int2Coord(6)), 0);
	test_show->SetCurrentSheet(0);
	
#if 0
	auto point_start = offset + field_offset + CC_coord(Int2Coord(4), Int2Coord(2));
	mPathEnd = point_start + CC_coord(Int2Coord(0), Int2Coord(2));
	mPath.push_back(CC_DrawCommand(point_start, mPathEnd));
	point_start = mPathEnd;
	mPathEnd += CC_coord(Int2Coord(18), Int2Coord(0));
	mPath.push_back(CC_DrawCommand(point_start, mPathEnd));
	
	auto shape_start = field_offset + CC_coord(Int2Coord(18), Int2Coord(-2));
	auto shape_end = shape_start + CC_coord(Int2Coord(4), Int2Coord(4));
	CC_shape_rect rect(shape_start, shape_end);
	mShape = rect.GetCC_DrawCommand(offset.x, offset.y);
	
	test_show->SetNumPoints(1, 1)
	// warning, the labels might not match up
	void CC_show::SetNumPoints(unsigned num, unsigned columns, const CC_coord& new_march_position)
	{
		for (CC_sheet_iterator_t sht = GetSheetBegin(); sht != GetSheetEnd(); ++sht)
		{
			sht->SetNumPoints(num, columns, new_march_position);
		}
		numpoints = num;
		pt_labels.resize(numpoints);
	}
#endif
	return test_show;
}

	template<typename T>
	bool is_equal(const T& a, const T& b) { return a == b; }
	bool is_equal(double a, double b) { static const double kEpsilon = 0.000001; return std::abs(a - b) < kEpsilon; }

template<typename T>
	bool test_result(const T& a, const T& expected)
	{
		if (is_equal(a, expected))
		{
			return true;
		}
		std::cerr<<"Error, expected "<<expected<<", got "<<a<<std::endl;
		return false;
	}



std::pair<int, int> test_points()
{
	std::pair<int, int> result = { 0, 0 };

	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, CC_coord> test_vectors[] = {
		{ "SP", { Int2Coord(2), Int2Coord(2) } }
		, { "NP", { Int2Coord(6), Int2Coord(6) } }
		, { "P", { Int2Coord(2), Int2Coord(2) } }
		, { "R2", { Int2Coord(4), Int2Coord(4) } }
	};
//	const CC_coord test_points[] = {  };
	for (auto& test_vector : test_vectors)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing:\n";
		std::cout << test_vector.first << "\n";
		++result.first;
		AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
		auto parse_result = Parse_point_string(test_vector.first, a);
		
		if (parse_result.first)
		{
			std::cout << "Parsing succeeded\n";
			std::cout << parse_result.second << std::endl;
			std::cout << "-------------------------\n";
			result.second += test_result(parse_result.second, test_vector.second);
		}
		else
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << test_vector.first << std::endl;
			std::cout << "-------------------------\n";
		}
	}
	return result;
}


std::pair<int, int> test_value()
{
	std::pair<int, int> result = { 0, 0 };
	
	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, double> test_vectors[] = {
		{ "S + 50", 230 }
		, { "3.14", 3.14 }
		, { ".5", 0.5 }
		, { "0", 0 }
		, { "S * DM", 180 * std::sqrt(2) }
		, { "DM", std::sqrt(2) }
		, { "1 + 2 * 3", 7 }
		, { "1 * 2 + 3", 5 }
		, { "(1+ 2)*3", 9 }
		, { "-N", -0 }
		, { "E + W * -DM", 270 + 90 * -std::sqrt(2) }
		, { "REM", 8 }
		, { "A", 0 }
		, { "A + E", 270 }
		, { "W - A", 90 }
		, { "W - B", 90 }
		, { "W - OPP(W)", 90-270 }
		, { "DIR(NP)", -45 }
		, { "DIRFROM(SP NP)", -45 }
		, { "DIRFROM(NP SP)", 180-45 }
		, { "DIST(SP)", 0 }
		, { "DISTFROM(SP NP)", 4*std::sqrt(2) }
		, { "EITHER(N  S NP)", 0 }
		, { "OPP(W)", 270 }
		, { "STEP(8 2 NP)", 16 }
		, { "STEP(2 8 NP)", 1 }
	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		std::cout << "-------------------------\n";
		std::cout << "Parsing:\n";
		std::cout << test_vector.first << "\n";
		AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
		auto parse_result = Parse_value_string(test_vector.first, a);
		
		if (parse_result.first)
		{
			std::cout << "Parsing succeeded\n";
			std::cout << parse_result.second << std::endl;
			std::cout << "-------------------------\n";
			result.second += test_result(parse_result.second, test_vector.second);
		}
		else
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << test_vector.first << std::endl;
			std::cout << "-------------------------\n";
		}
	}
	return result;
}


std::pair<int, int> test_procedure()
{
	std::pair<int, int> result = { 0, 0 };
	
	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, double> test_vectors[] = {
		{ "BLAM", 230 }
		, { "COUNTERMARCH SP NP 2 4 6 8", 3.14 }
		, { "mt 16 e", 0 }
		, { "ewns np", 0 }
		, { "mtrm e", 0 }
		, { "mt 16 e\newns np", 0 }
		, { "mt 16 e\newns np\nmtrm e", 0 }
		, { "march .5 48 e", 0 }
		, { "ewns np", 0 }
		, { "mtrm e", 0 }
		, { "march .5 48 e\newns np", 0 }
		, { "march .5 48 e\nmtrm e", 0 }
		, { "ewns np\nmtrm e", 0 }
		, { "march .5 48 e\newns np\nmtrm e", 0 }
		, { "ewns np\nmarch .5 48 e\nmtrm e", 0 }
		, { "a= 4", 0 }
		, { " b =-5", 0 }
		, { " x = a + b", 0 }

	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		std::cout << "-------------------------\n";
		std::cout << "Parsing:\n";
		std::cout << test_vector.first << "\n";
		AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
		auto parse_result = Parse_procedure_string(test_vector.first);
		
		if (parse_result.first)
		{
			std::cout << "Parsing succeeded\n";
			std::cout << "-------------------------\n";
			for (auto&& i : parse_result.second)
			{
				std::cout << i<<"\n";
			}
			result.second += 1;//test_result(parse_result.second, test_vector.second);
		}
		else
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << test_vector.first << std::endl;
			std::cout << "-------------------------\n";
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	auto result = test_points();
//	std::cout<<"test_function\n";
//	auto next_result = test_function();
//	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"test_value\n";
	auto next_result = test_value();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"test_procedure\n";
	next_result = test_procedure();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"ran "<<result.first<<" tests, passed: "<<result.second<<"\n";
	return result.first == result.second;
}
}

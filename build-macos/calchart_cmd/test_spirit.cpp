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

#include "new_cont.h"
//#include "cc_show.h"
#include "cc_sheet.h"
//#include "animate.h"
//#include "animatecompile.h"

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>
#include <boost/phoenix/object/construct.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#if 0
namespace calchart
{
	///////////////////////////////////////////////////////////////////////////
	//  Our employee struct
	///////////////////////////////////////////////////////////////////////////
	//[tutorial_employee_struct
	struct pCOUNTERMARCH
	{
		Point point1;
		Point point2;
	};
	
}
#endif

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


///////////////////////////////////////////////////////////////////////////
//  Our point grammar definition
///////////////////////////////////////////////////////////////////////////

template <typename Iterator>
struct value_grammar;

#if 0
point
: rwP
{ $$ = new ContPoint(); }
| rwSP
{ $$ = new ContStartPoint(); }
| rwNP
{ $$ = new ContNextPoint(); }
| rwR FLOATCONST
{ $$ = new ContRefPoint((unsigned)$2 - 1); }
;
#endif

template <typename Iterator>
struct point_grammar : qi::grammar<Iterator, calchart::continuity::Point(), ascii::space_type>
{
	point_grammar() : point_grammar::base_type(point)
	{
		static const struct PointT_ : qi::symbols<char, calchart::continuity::Point>
		{
			PointT_()
			{
				add
				("P"     , calchart::continuity::CurrentPoint())
				("SP"    , calchart::continuity::StartPoint())
				("NP"    , calchart::continuity::NextPoint())
				;
			}
		} PointT;
		
		refpoint %= qi::lit("R") >> qi::int_ ;
		point %= (PointT | refpoint);
	}
	qi::rule<Iterator, calchart::continuity::Point(), ascii::space_type> point;
	qi::rule<Iterator, calchart::continuity::RefPoint(), ascii::space_type> refpoint;
};

#if 0
function
: fDIR '(' point ')'
{ $$ = new ContFuncDir($3); }
| fDIRFROM '(' point point ')'
{ $$ = new ContFuncDirFrom($3, $4); }
| fDIST '(' point ')'
{ $$ = new ContFuncDist($3); }
| fDISTFROM '(' point point ')'
{ $$ = new ContFuncDistFrom($3, $4); }
| fEITHER '(' value value point ')'
{ $$ = new ContFuncEither($3, $4, $5); }
| fOPP '(' value ')'
{ $$ = new ContFuncOpp($3); }
| fSTEP '(' value value point ')'
{ $$= new ContFuncStep($3, $4, $5); }
;
#endif

template <typename Iterator>
struct function_grammar : qi::grammar<Iterator, calchart::continuity::Function(), ascii::space_type>
{
	function_grammar() : function_grammar::base_type(function)
	{
		using qi::lit;
		FuncDir %= lit("DIR") >> '(' >> point >> ')' ;
		FuncDirFrom %= lit("DIRFROM") >> '(' >> point >> point >> ')' ;
		FuncDist %= lit("DIST") >> '(' >> point >> ')' ;
		FuncDistFrom %= lit("DISTFROM") >> '(' >> point >> point >> ')' ;
		FuncEither %= lit("EITHER") >> '(' >> value >> value >> point >> ')' ;
		FuncOpp %= lit("OPP") >> '(' >> value >> ')' ;
		FuncStep %= lit("STEP") >> '(' >> value >> value >> point >> ')' ;
		function %= ( FuncDir | FuncDirFrom | FuncDist | FuncDistFrom | FuncEither | FuncOpp | FuncStep );
	}
	qi::rule<Iterator, calchart::continuity::Function(), ascii::space_type> function;
	qi::rule<Iterator, calchart::continuity::FunctionDir(), ascii::space_type> FuncDir;
	qi::rule<Iterator, calchart::continuity::FunctionDirFrom(), ascii::space_type> FuncDirFrom;
	qi::rule<Iterator, calchart::continuity::FunctionDist(), ascii::space_type> FuncDist;
	qi::rule<Iterator, calchart::continuity::FunctionDistFrom(), ascii::space_type> FuncDistFrom;
	qi::rule<Iterator, calchart::continuity::FunctionEither(), ascii::space_type> FuncEither;
	qi::rule<Iterator, calchart::continuity::FunctionOpposite(), ascii::space_type> FuncOpp;
	qi::rule<Iterator, calchart::continuity::FunctionStep(), ascii::space_type> FuncStep;
	point_grammar<Iterator> point;
	value_grammar<Iterator> value;
};



template <typename Iterator>
struct value_grammar : qi::grammar<Iterator, calchart::continuity::Value(), ascii::space_type>
{
	value_grammar() : value_grammar::base_type(expression)
	{
		using qi::lit;
		using qi::double_;
		using boost::phoenix::construct;
		using namespace qi::labels;
		static const struct ValueTable_ : qi::symbols<char, double>
		{
			ValueTable_()
			{
				add
				("N"     , 0.0)
				("NW"    , 45.0)
				("W"     , 90.0)
				("SW"    , 135.0)
				("S"     , 180.0)
				("SE"    , 225.0)
				("E"     , 270.0)
				("NE"    , 315.0)
				("HS"    , 1.0)
				("MM"    , 1.0)
				("SH"    , 0.5)
				("JS"    , 0.5)
				("GV"    , 1.0)
				("M"     , 4.0/3.0)
				("DM"    , std::sqrt(2.0))
				;
			}
		} ValueTable;

		expression = term [ _val = _1 ]
					 >> *( ( '+' >> term [ _val = construct<calchart::continuity::ValueAdd>(_val, _1) ] )
						 | ( '-' >> term [ _val = construct<calchart::continuity::ValueSub>(_val, _1) ] )
						 );
		term = factor [ _val = _1 ]
					>> *( ( '*' >> term [ _val = construct<calchart::continuity::ValueMult>(_val, _1) ] )
						 | ( '/' >> term [ _val = construct<calchart::continuity::ValueDiv>(_val, _1) ] )
						 );
		factor %= double_ | ValueTable | '(' >> expression >> ')' | ('-' >> factor [_val = construct<calchart::continuity::ValueNeg>(_1) ]);
	}
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> factor;
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> term;
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> expression;
};



#if 0

//[tutorial_employee_adapt_struct
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::pCOUNTERMARCH,
						  (Point, point1)
						  (Point, point2)
						  )

#endif
//]
namespace calchart
{
	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;
	//]
#if 0
	
	struct cont_procedure_printer
	{
		cont_procedure_printer()
		{
		}
		
		void operator()(pCOUNTERMARCH const& proc) const;
	};
	
	void cont_procedure_printer::operator()(pCOUNTERMARCH const& proc) const
	{
		std::cout << "got: " << boost::fusion::as_vector(proc) << std::endl;
	}
	
	struct Func_printer : boost::static_visitor<>
	{
		Func_printer(std::ostream& os) : os(os) {}
		
		void operator()(FunctionDir const& f) const
		{
			os<<"Dir to "<<f.point;
		}
		std::ostream& os;
	};
	
	std::ostream& operator<<(std::ostream& os, Function const& p)
	{
		boost::apply_visitor(Func_printer(os), p);
		return os;
	}
	
	struct function_printer
	{
		void operator()(Function const& proc) const;
	};
	
	void function_printer::operator()(Function const& f) const
	{
		std::cout << "got: "<<f<<std::endl;
	}
	
	///////////////////////////////////////////////////////////////////////////
	//  Our mini XML grammar definition
	///////////////////////////////////////////////////////////////////////////
	//[tutorial_xml1_grammar
	template <typename Iterator>
	struct cont_point_grammar : qi::grammar<Iterator, Point(), ascii::space_type>
	{
		cont_point_grammar() : cont_point_grammar::base_type(start)
		{
			static const struct PointT_ : qi::symbols<char, Point>
			{
				PointT_()
				{
					add
					("P"     , CurrPoint())
					("NP"    , NextPoint())
					("SP"    , StartPoint())
					;
				}
			} PointT;
			
			refpoint %= qi::lit("R") ;
			start %= (PointT | refpoint);
		}
		qi::rule<Iterator, Point(), ascii::space_type> start;
		qi::rule<Iterator, RefPoint(), ascii::space_type> refpoint;
	};
	
	template <typename Iterator>
	struct cont_procedure_grammar : qi::grammar<Iterator, pCOUNTERMARCH(), ascii::space_type>
	{
		cont_procedure_grammar() : cont_procedure_grammar::base_type(start)
		{
			using qi::int_;
			using qi::lit;
			using qi::double_;
			using qi::string;
			using qi::lexeme;
			using ascii::char_;
			
			start %=
			lit("COUNTERMARCH")
			>> '{'
			>>  point
			>>  point
			>>  '}'
			;
		}
		qi::rule<Iterator, pCOUNTERMARCH(), ascii::space_type> start;
		cont_point_grammar<Iterator> point;
	};
	
	//]
#endif
}

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
	int test_result(const T& a, const T& expected)
	{
		if (a == expected)
		{
			return 1;
		}
		std::cerr<<"Error, expected "<<expected<<", got "<<a<<std::endl;
		return 0;
	}


std::pair<int, int> test_points()
{
	std::pair<int, int> result = { 0, 0 };
	typedef point_grammar<std::string::const_iterator> grammar;
	grammar gram; // Our grammar
	calchart::continuity::Point ast; // Our tree

	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, CC_coord> test_vectors[] = {
		{ "SP", { Int2Coord(2), Int2Coord(2) } }
		, { "NP", { Int2Coord(6), Int2Coord(6) } }
		, { "P", { Int2Coord(2), Int2Coord(2) } }
		, { "R1", { Int2Coord(4), Int2Coord(4) } }
	};
//	const CC_coord test_points[] = {  };
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		using boost::spirit::ascii::space;
		std::string::const_iterator iter = test_vector.first.begin();
		std::string::const_iterator end = test_vector.first.end();
		bool r = phrase_parse(iter, end, gram, space, ast);
		
		if (r && iter == end)
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing succeeded\n";
			std::cout << "-------------------------\n";
			std::cout << (ast)<<" :\n";
	//		printer(ast);
			AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
			CC_coord value = Get(a, ast);
			std::cout<<value<<std::endl;
			result.second += test_result(value, test_vector.second);
		}
		else
		{
			std::string::const_iterator some = iter+30;
			std::string context(iter, (some>end)?end:some);
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "stopped at: \": " << context << "...\"\n";
			std::cout << "-------------------------\n";
		}
	}
	return result;
}


	
std::pair<int, int> test_function()
{
	std::pair<int, int> result = { 0, 0 };
	typedef function_grammar<std::string::const_iterator> grammar;
	grammar gram; // Our grammar
	calchart::continuity::Function ast; // Our tree

	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, double> test_vectors[] = {
		{ "DIR(NP)", -45 }
		, { "DIRFROM(SP NP)", -45 }
		, { "DIRFROM(NP SP)", 180-45 }
		, { "DIST(SP)", 0 }
		, { "DISTFROM(SP NP)", 4*std::sqrt(2) }
		, { "EITHER(N  S NP)", 0 }
		, { "OPP(W)", 270 }
		, { "STEP(2 8 NP)", 270 }
	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		using calchart::continuity::Get;
		using boost::spirit::ascii::space;
		std::string::const_iterator iter = test_vector.first.begin();
		std::string::const_iterator end = test_vector.first.end();
		bool r = phrase_parse(iter, end, gram, space, ast);
		
		if (r && iter == end)
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing succeeded\n";
			std::cout << "-------------------------\n";
			std::cout << test_vector.first << std::endl;
			std::cout << (ast)<<" :\n";
			//		printer(ast);
			AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
			double value = Get(a, ast);
			std::cout<<value<<std::endl;
			result.second += test_result(value, test_vector.second);
		}
		else
		{
			std::string::const_iterator some = iter+30;
			std::string context(iter, (some>end)?end:some);
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "stopped at: \": " << context << "...\"\n";
			std::cout << "-------------------------\n";
		}
	}
	return result;
}

std::pair<int, int> test_value()
{
	std::pair<int, int> result = { 0, 0 };
	typedef value_grammar<std::string::const_iterator> grammar;
	grammar gram; // Our grammar
	calchart::continuity::Value ast; // Our tree
	
	AnimationVariables variablesStates;
	std::map<AnimateError, ErrorMarker> error_markers;
	auto show = make_a_new_show();
	
	const std::pair<std::string, double> test_vectors[] = {
		{ "S + 50", 230 }
		, { "3.14", 3.14 }
		, { "0", 0 }
		, { "S * DM", 180 * std::sqrt(2) }
		, { "DM", std::sqrt(2) }
		, { "1 + 2 * 3", 7 }
		, { "1 * 2 + 3", 5 }
		, { "(1+ 2)*3", 9 }
		, { "-N", -0 }
		, { "E + W * -DM", 270 + 90 * -std::sqrt(2) }
	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		using calchart::continuity::Get;
		using boost::spirit::ascii::space;
		std::string::const_iterator iter = test_vector.first.begin();
		std::string::const_iterator end = test_vector.first.end();
		bool r = phrase_parse(iter, end, gram, space, ast);
		
		if (r && iter == end)
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing succeeded\n";
			std::cout << "-------------------------\n";
			std::cout << test_vector.first << std::endl;
			std::cout << (ast)<<" :\n";
			//		printer(ast);
			AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
			auto value = Get(a, ast);
			std::cout<<value<<std::endl;
			result.second += test_result(value, test_vector.second);
		}
		else
		{
			std::string::const_iterator some = iter+30;
			std::string context(iter, (some>end)?end:some);
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "stopped at: \": " << context << "...\"\n";
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
	auto next_result = test_function();
	result.first += next_result.first; result.second += next_result.second;
	next_result = test_value();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"ran "<<result.first<<" tests, passed: "<<result.second<<"\n";
	return result.first == result.second;
}
}

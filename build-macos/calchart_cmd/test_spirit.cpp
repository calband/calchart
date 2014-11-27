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
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>

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
struct point_grammar : qi::grammar<Iterator, calchart::continuity::Point(), ascii::space_type>
{
	point_grammar() : point_grammar::base_type(start)
	{
		static const struct PointT_ : qi::symbols<char, calchart::continuity::Point>
		{
			PointT_()
			{
				add
				("P"     , calchart::continuity::CurrentPoint())
				("NP"    , calchart::continuity::StartPoint())
				("SP"    , calchart::continuity::NextPoint())
				;
			}
		} PointT;
		
		refpoint %= qi::lit("R") >> qi::int_ ;
		start %= (PointT | refpoint);
	}
	qi::rule<Iterator, calchart::continuity::Point(), ascii::space_type> start;
	qi::rule<Iterator, calchart::continuity::RefPoint(), ascii::space_type> refpoint;
};


template <typename Iterator>
struct function_grammar : qi::grammar<Iterator, calchart::continuity::FunctionDir(), ascii::space_type>
{
	function_grammar() : function_grammar::base_type(start)
	{
		using qi::lit;
		start %= lit("DIR") >> point ;
	}
	qi::rule<Iterator, calchart::continuity::FunctionDir(), ascii::space_type> start;
	point_grammar<Iterator> point;
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
	test_show->SetNumPoints(4, 4, field_offset);
	CC_show::CC_sheet_iterator_t sheet = test_show->GetCurrentSheet();
	sheet->GetPoint(0).SetSymbol(SYMBOL_X);
	sheet->GetPoint(1).SetSymbol(SYMBOL_SOLX);
	sheet->GetPoint(2).SetSymbol(SYMBOL_X);
	sheet->GetPoint(3).SetSymbol(SYMBOL_SOLX);
	
	for (auto i = 0; i < 4; ++i)
	{
		sheet->SetAllPositions(field_offset + CC_coord(Int2Coord(i*4), Int2Coord(2)), i);
		sheet->SetPosition(field_offset + CC_coord(Int2Coord(i*4), Int2Coord(6)), i, 1);
	}
	
	test_show->InsertSheet(*sheet, 1);
	test_show->SetCurrentSheet(0);
	sheet = test_show->GetCurrentSheet();
	++sheet;
	for (auto i = 0; i < 4; ++i)
	{
		sheet->SetAllPositions(field_offset + CC_coord(Int2Coord(18+i*4), Int2Coord(2+2)), i);
		sheet->SetPosition(field_offset + CC_coord(Int2Coord(18+i*4), Int2Coord(2+6)), i, 1);
	}
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

int test_points()
{
	typedef point_grammar<std::string::const_iterator> grammar;
	grammar gram; // Our grammar
	calchart::continuity::Point ast; // Our tree

	AnimationVariables variablesStates;
	AnimateCompile a(*make_a_new_show(), variablesStates);
	

	
	std::string storages[] = { "SP", "NP", "P", "R1" };
	for (auto& storage : storages)
	{
	using boost::spirit::ascii::space;
	std::string::const_iterator iter = storage.begin();
	std::string::const_iterator end = storage.end();
	bool r = phrase_parse(iter, end, gram, space, ast);
	
	if (r && iter == end)
	{
		std::cout << "-------------------------\n";
		std::cout << "Parsing succeeded\n";
		std::cout << "-------------------------\n";
		std::cout<<(ast);
//		printer(ast);
		CC_coord the_point = Get(a, ast);
		std::cout<<the_point<<std::endl;
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
	return 0;
}


int test_function()
{
	typedef function_grammar<std::string::const_iterator> grammar;
	grammar gram; // Our grammar
	calchart::continuity::Function ast; // Our tree

	AnimationVariables variablesStates;
	AnimateCompile a(*make_a_new_show(), variablesStates);
	
	std::string storages[] = { "DIR SP", "DIR NP", "DIR P", "DIR R1" };
	for (auto& storage : storages)
	{
		using boost::spirit::ascii::space;
		std::string::const_iterator iter = storage.begin();
		std::string::const_iterator end = storage.end();
		bool r = phrase_parse(iter, end, gram, space, ast);
		
		if (r && iter == end)
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing succeeded\n";
			std::cout << "-------------------------\n";
			std::cout<<(ast);
			//		printer(ast);
			auto value = Get(a, ast);
			std::cout<<value<<std::endl;
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
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	auto result = test_points();
	result += test_function();
	return result;
}
}

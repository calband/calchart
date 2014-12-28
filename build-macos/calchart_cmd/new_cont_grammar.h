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

#ifndef BOOST_MPL_LIMIT_LIST_SIZE
#define  BOOST_MPL_LIMIT_LIST_SIZE 30
#endif

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
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using pos_iterator_t = boost::spirit::line_pos_iterator<std::string::const_iterator>;

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


///////////////////////////////////////////////////////////////////////////
//  Our point grammar definition
///////////////////////////////////////////////////////////////////////////

template <typename Iterator>
struct value_grammar;
template <typename Iterator>
struct function_grammar;

#if 0
varvalue
: VARIABLE
{ unsigned i;
	switch ($1) {
		case 'A':
		case 'a':
			i = 0;
			break;
		case 'B':
		case 'b':
			i = 1;
			break;
		case 'C':
		case 'c':
			i = 2;
			break;
		case 'D':
		case 'd':
			i = 3;
			break;
		case 'X':
		case 'x':
			i = 4;
			break;
		case 'Y':
		case 'y':
			i = 5;
			break;
		case 'Z':
		case 'z':
		default:
			i = 6;
			break;
	}
	$$ = new ContValueVar(i);
}
;
#endif
template <typename Iterator>
struct variable_grammar : qi::grammar<Iterator, calchart::continuity::Variable(), ascii::space_type>
{
	variable_grammar(Iterator first) : variable_grammar::base_type(variable), annotate(first)
	{
		static const struct variableT_ : qi::symbols<char, calchart::continuity::Variable>
		{
			variableT_()
			{
				add
				("A"     , calchart::continuity::Variable(CONTVAR_A))
				("a"     , calchart::continuity::Variable(CONTVAR_A))
				("B"     , calchart::continuity::Variable(CONTVAR_B))
				("b"     , calchart::continuity::Variable(CONTVAR_B))
				("C"     , calchart::continuity::Variable(CONTVAR_C))
				("c"     , calchart::continuity::Variable(CONTVAR_C))
				("D"     , calchart::continuity::Variable(CONTVAR_D))
				("d"     , calchart::continuity::Variable(CONTVAR_D))
				("X"     , calchart::continuity::Variable(CONTVAR_X))
				("x"     , calchart::continuity::Variable(CONTVAR_X))
				("Y"     , calchart::continuity::Variable(CONTVAR_Y))
				("y"     , calchart::continuity::Variable(CONTVAR_Y))
				("Z"     , calchart::continuity::Variable(CONTVAR_Z))
				("z"     , calchart::continuity::Variable(CONTVAR_Z))
				("DOF"   , calchart::continuity::Variable(CONTVAR_DOF))
				("DOH"   , calchart::continuity::Variable(CONTVAR_DOH))
				;
			}
		} variableTable;
		
		variable %= variableTable;

		// annotate on success
		auto set_location_info = annotate(qi::_val, qi::labels::_1, qi::labels::_3);
		qi::on_success(variable, set_location_info);
	}
	qi::rule<Iterator, calchart::continuity::Variable(), ascii::space_type> variable;
	boost::phoenix::function<calchart::continuity::annotation_f<Iterator>> annotate;
};

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
	point_grammar(Iterator first) : point_grammar::base_type(point), annotate(first)
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

		// annotate on success
		auto set_location_info = annotate(qi::_val, qi::labels::_1, qi::labels::_3);
		qi::on_success(point,    set_location_info);
		qi::on_success(refpoint, set_location_info);
	}
	boost::phoenix::function<calchart::continuity::annotation_f<Iterator>> annotate;
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
	function_grammar(Iterator first) : function_grammar::base_type(function), point(first), value(first), annotate(first)
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

		// annotate on success
		auto set_location_info = annotate(qi::_val, qi::labels::_1, qi::labels::_3);
		qi::on_success(function,     set_location_info);
		qi::on_success(FuncDir,      set_location_info);
		qi::on_success(FuncDirFrom,  set_location_info);
		qi::on_success(FuncDist,     set_location_info);
		qi::on_success(FuncDistFrom, set_location_info);
		qi::on_success(FuncEither,   set_location_info);
		qi::on_success(FuncOpp,      set_location_info);
		qi::on_success(FuncStep,     set_location_info);
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
	boost::phoenix::function<calchart::continuity::annotation_f<Iterator>> annotate;
};


#if 0
value
: FLOATCONST
{ $$ = new ContValueFloat($1); }
| DEFINECONST
{ $$ = new ContValueDefined($1); }
| value '+' value
{ $$ = new ContValueAdd($1, $3); }
| value '-' value
{ $$ = new ContValueSub($1, $3); }
| value '*' value
{ $$ = new ContValueMult($1, $3); }
| value '/' value
{ $$ = new ContValueDiv($1, $3); }
| '-' value %prec UNARY
{ $$ = new ContValueNeg($2); }
| '(' value ')'
{ $$ = $2; }
| rwREM
{ $$ = new ContValueREM(); }
| rwDOF
{ $$ = new ContValueVar(CONTVAR_DOF); }
| rwDOH
{ $$ = new ContValueVar(CONTVAR_DOH); }
| varvalue
{ $$ = $1; }
| function
{ $$ = $1; }
;
#endif

template <typename Iterator>
struct value_grammar : qi::grammar<Iterator, calchart::continuity::Value(), ascii::space_type>
{
	value_grammar(Iterator first) : value_grammar::base_type(expression), variable(first), point(first), annotate(first)
	{
		using qi::lit;
		using qi::double_;
		using boost::phoenix::construct;
		using namespace qi::labels;
		using boost::phoenix::val;
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
		static const struct ValueREMT_ : qi::symbols<char, calchart::continuity::Value>
		{
			ValueREMT_()
			{
				add
				("REM"   , calchart::continuity::ValueREM())
				;
			}
		} ValueREMTable;

		FuncDir %= lit("DIR") >> '(' >> point >> ')' ;
		FuncDirFrom %= lit("DIRFROM") >> '(' >> point >> point >> ')' ;
		FuncDist %= lit("DIST") >> '(' >> point >> ')' ;
		FuncDistFrom %= lit("DISTFROM") >> '(' >> point >> point >> ')' ;
		FuncEither %= lit("EITHER") >> '(' >> expression >> expression >> point >> ')' ;
		FuncOpp %= lit("OPP") >> '(' >> expression >> ')' ;
		FuncStep %= lit("STEP") >> '(' >> expression >> expression >> point >> ')' ;
		function %= ( FuncDir | FuncDirFrom | FuncDist | FuncDistFrom | FuncEither | FuncOpp | FuncStep );

		expression = term [ _val = _1 ]
					 >> *( ( '+' >> term [ _val = construct<calchart::continuity::ValueAdd>(_val, _1) ] )
						 | ( '-' >> term [ _val = construct<calchart::continuity::ValueSub>(_val, _1) ] )
						 );
		term = factor [ _val = _1 ]
					>> *( ( '*' >> term [ _val = construct<calchart::continuity::ValueMult>(_val, _1) ] )
						 | ( '/' >> term [ _val = construct<calchart::continuity::ValueDiv>(_val, _1) ] )
						 );
		factor %= double_ | ValueTable | '(' >> expression >> ')' | ('-' >> factor [_val = construct<calchart::continuity::ValueNeg>(_1) ]) | ValueREMTable | variable | function ;

		expression.name("expression");
		term.name("term");
		factor.name("factor");

		qi::on_error<qi::fail>
		(
		 expression
		 , std::cout
		 << val("Error! Expecting ")
		 << _4                               // what failed?
		 << val(" here: \"")
		 << construct<std::string>(_3, _2)   // iterators to error-pos, end
		 << val("\"")
		 << std::endl
		 );
		
//		qi::debug(expression);
//		qi::debug(term);
//		qi::debug(factor);
	
		// annotate on success
		auto set_location_info = annotate(qi::_val, qi::labels::_1, qi::labels::_3);
		qi::on_success(factor,       set_location_info);
		qi::on_success(term,         set_location_info);
		qi::on_success(expression,   set_location_info);
		qi::on_success(function,     set_location_info);
		qi::on_success(FuncDir,      set_location_info);
		qi::on_success(FuncDirFrom,  set_location_info);
		qi::on_success(FuncDist,     set_location_info);
		qi::on_success(FuncDistFrom, set_location_info);
		qi::on_success(FuncEither,   set_location_info);
		qi::on_success(FuncOpp,      set_location_info);
		qi::on_success(FuncStep,     set_location_info);
	}
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> factor;
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> term;
	qi::rule<Iterator, calchart::continuity::Value(), ascii::space_type> expression;
	qi::rule<Iterator, calchart::continuity::Function(), ascii::space_type> function;
	qi::rule<Iterator, calchart::continuity::FunctionDir(), ascii::space_type> FuncDir;
	qi::rule<Iterator, calchart::continuity::FunctionDirFrom(), ascii::space_type> FuncDirFrom;
	qi::rule<Iterator, calchart::continuity::FunctionDist(), ascii::space_type> FuncDist;
	qi::rule<Iterator, calchart::continuity::FunctionDistFrom(), ascii::space_type> FuncDistFrom;
	qi::rule<Iterator, calchart::continuity::FunctionEither(), ascii::space_type> FuncEither;
	qi::rule<Iterator, calchart::continuity::FunctionOpposite(), ascii::space_type> FuncOpp;
	qi::rule<Iterator, calchart::continuity::FunctionStep(), ascii::space_type> FuncStep;
	variable_grammar<Iterator> variable;
	point_grammar<Iterator> point;
	boost::phoenix::function<calchart::continuity::annotation_f<Iterator>> annotate;
};



#if 0
procedure
;
#endif

template <typename Iterator>
struct procedure_grammar : qi::grammar<Iterator, calchart::continuity::Procedure(), ascii::space_type>
{
	procedure_grammar(Iterator first) : procedure_grammar::base_type(procedure), variable(first), point(first), value(first), annotate(first)
	{
		using qi::lit;
		using qi::double_;
		using boost::phoenix::construct;
		using namespace qi::labels;
		using boost::phoenix::val;
			using ascii::char_;

		static const struct ProcBlamTable_ : qi::symbols<char, calchart::continuity::ProcedureBlam>
		{
			ProcBlamTable_()
			{
				add
				("BLAM"     , calchart::continuity::ProcedureBlam())
				;
			}
		} ProcBlamTable;
		ProcSet       %= variable >> char_('=') >> value;
		ProcCM        %= lit("COUNTERMARCH") >> point >> point >> value >> value >> value >> value ;
		ProcDMCM      %= lit("DMCM") >> point >> point >> value ;
		ProcDMHS      %= lit("DMHS") >> point ;
		ProcEven      %= lit("EVEN") >> value >> point ;
		ProcEWNS      %= lit("EWNS") >> point ;
		ProcFountain2 %= lit("FOUNTAIN") >> value >> value >> point ;
		ProcFountain1 %= lit("FOUNTAIN") >> value >> value >> value >> value >> point ;
		ProcFM        %= lit("FM") >> value >> value ;
		ProcFMTO      %= lit("FMTO") >> point ;
		ProcGrid      %= lit("GRID") >> value ;
		ProcHSCM      %= lit("HSCM") >> point >> point >> value ;
		ProcHSDM      %= lit("HSDM") >> point ;
		ProcMagic     %= lit("MAGIC") >> point ;
		ProcMarch2    %= lit("MARCH") >> value >> value >> value ;
		ProcMarch1    %= lit("MARCH") >> value >> value >> value >> value ;
		ProcMT        %= ( lit("CLOSE") | lit("MT") ) >> value >> value ;
		ProcMTRM      %= lit("MTRM") >> value ;
		ProcNSEW      %= lit("NSEW") >> point ;
		ProcRotate    %= lit("ROTATE") >> value >> value >> point ;
		procedure     %= ( ProcSet | ProcBlamTable | ProcCM | ProcDMCM | ProcDMHS | ProcEven | ProcEWNS | ProcFountain1 | ProcFountain2 | ProcFM | ProcFMTO | ProcGrid | ProcHSCM | ProcHSDM | ProcMagic | ProcMarch1 | ProcMarch2 | ProcMT | ProcMTRM | ProcNSEW | ProcRotate );
		
		// annotate on success
		auto set_location_info = annotate(qi::_val, qi::labels::_1, qi::labels::_3);
		qi::on_success(procedure,    set_location_info);
	}
	qi::rule<Iterator, calchart::continuity::Procedure(), ascii::space_type> procedure;
	qi::rule<Iterator, calchart::continuity::ProcedureSet(),      ascii::space_type> ProcSet;
	qi::rule<Iterator, calchart::continuity::ProcedureCM(),       ascii::space_type> ProcCM;
	qi::rule<Iterator, calchart::continuity::ProcedureDMCM(),     ascii::space_type> ProcDMCM;
	qi::rule<Iterator, calchart::continuity::ProcedureDMHS(),     ascii::space_type> ProcDMHS;
	qi::rule<Iterator, calchart::continuity::ProcedureEven(),     ascii::space_type> ProcEven;
	qi::rule<Iterator, calchart::continuity::ProcedureEWNS(),     ascii::space_type> ProcEWNS;
	qi::rule<Iterator, calchart::continuity::ProcedureFountain1(), ascii::space_type> ProcFountain1;
	qi::rule<Iterator, calchart::continuity::ProcedureFountain2(), ascii::space_type> ProcFountain2;
	qi::rule<Iterator, calchart::continuity::ProcedureFM(),       ascii::space_type> ProcFM;
	qi::rule<Iterator, calchart::continuity::ProcedureFMTO(),     ascii::space_type> ProcFMTO;
	qi::rule<Iterator, calchart::continuity::ProcedureGrid(),     ascii::space_type> ProcGrid;
	qi::rule<Iterator, calchart::continuity::ProcedureHSCM(),     ascii::space_type> ProcHSCM;
	qi::rule<Iterator, calchart::continuity::ProcedureHSDM(),     ascii::space_type> ProcHSDM;
	qi::rule<Iterator, calchart::continuity::ProcedureMagic(),    ascii::space_type> ProcMagic;
	qi::rule<Iterator, calchart::continuity::ProcedureMarch1(),    ascii::space_type> ProcMarch1;
	qi::rule<Iterator, calchart::continuity::ProcedureMarch2(),    ascii::space_type> ProcMarch2;
	qi::rule<Iterator, calchart::continuity::ProcedureMT(),       ascii::space_type> ProcMT;
	qi::rule<Iterator, calchart::continuity::ProcedureMTRM(),     ascii::space_type> ProcMTRM;
	qi::rule<Iterator, calchart::continuity::ProcedureNSEW(),     ascii::space_type> ProcNSEW;
	qi::rule<Iterator, calchart::continuity::ProcedureRotate(),   ascii::space_type> ProcRotate;
	variable_grammar<Iterator> variable;
	point_grammar<Iterator> point;
	value_grammar<Iterator> value;
	boost::phoenix::function<calchart::continuity::annotation_f<Iterator>> annotate;
};



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
	int test_result(const T& a, const T& expected)
	{
		if (is_equal(a, expected))
		{
			return 1;
		}
		std::cerr<<"Error, expected "<<expected<<", got "<<a<<std::endl;
		return 0;
	}


std::pair<int, int> test_points()
{
	std::pair<int, int> result = { 0, 0 };
	typedef point_grammar<pos_iterator_t> grammar;
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
		pos_iterator_t first(test_vector.first.begin()), iter = first, end(test_vector.first.end());
		grammar gram(first); // Our grammar
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
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "-------------------------\n";
		}
	}
	return result;
}


	
std::pair<int, int> test_function()
{
	std::pair<int, int> result = { 0, 0 };
	typedef function_grammar<pos_iterator_t> grammar;
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
		, { "STEP(8 2 NP)", 16 }
		, { "STEP(2 8 NP)", 1 }
	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		using calchart::continuity::Get;
		using boost::spirit::ascii::space;
		pos_iterator_t first(test_vector.first.begin()), iter = first, end(test_vector.first.end());
		grammar gram(first); // Our grammar
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
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "-------------------------\n";
		}
	}
	return result;
}

std::pair<int, int> test_value()
{
	std::pair<int, int> result = { 0, 0 };
	typedef value_grammar<pos_iterator_t> grammar;
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
		, { "REM", 8 }
		, { "A", 0 }
		, { "A + E", 270 }
		, { "W - A", 90 }
		, { "W - B", 90 }
		, { "W - OPP(NP)", 90-16 }
	};
	for (auto& test_vector : test_vectors)
	{
		++result.first;
		using calchart::continuity::Get;
		using boost::spirit::ascii::space;
		pos_iterator_t first(test_vector.first.begin()), iter = first, end(test_vector.first.end());
		grammar gram(first); // Our grammar
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
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "-------------------------\n";
		}
	}
	return result;
}

	std::pair<int, int> test_procedure()
	{
		std::pair<int, int> result = { 0, 0 };
		typedef procedure_grammar<pos_iterator_t> grammar;
		calchart::continuity::Procedure ast; // Our tree
		
		AnimationVariables variablesStates;
		std::map<AnimateError, ErrorMarker> error_markers;
		auto show = make_a_new_show();
		
		const std::pair<std::string, double> test_vectors[] = {
			{ "BLAM", 230 }
			, { "COUNTERMARCH SP NP 2 4 6 8", 3.14 }
		};
		for (auto& test_vector : test_vectors)
		{
			++result.first;
			using calchart::continuity::Get;
			using boost::spirit::ascii::space;
			pos_iterator_t first(test_vector.first.begin()), iter = first, end(test_vector.first.end());
			grammar gram(first); // Our grammar
			bool r = phrase_parse(iter, end, gram, space, ast);
			
			if (r && iter == end)
			{
				std::cout << "-------------------------\n";
				std::cout << "Parsing succeeded\n";
				std::cout << "-------------------------\n";
				std::cout << test_vector.first << std::endl;
				std::cout << (ast)<<" :\n";
				//		printer(ast);
//				AnimateCompile a(*show, show->GetSheetBegin(), 0, SYMBOL_X, variablesStates, error_markers);
//				auto value = Get(a, ast);
//				std::cout<<value<<std::endl;
//				result.second += test_result(value, test_vector.second);
			}
			else
			{
				std::cout << "-------------------------\n";
				std::cout << "Parsing failed\n";
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
	std::cout<<"test_function\n";
	auto next_result = test_function();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"test_value\n";
	next_result = test_value();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"test_procedure\n";
	next_result = test_procedure();
	result.first += next_result.first; result.second += next_result.second;
	std::cout<<"ran "<<result.first<<" tests, passed: "<<result.second<<"\n";
	return result.first == result.second;
}
}

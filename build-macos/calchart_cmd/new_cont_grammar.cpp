/*
 * new_cont_grammar.h
 * Grammar for continuity
 */

/*
   Copyright (C) 2014  Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NEW_CONT_GRAMMAR_H_
#define _NEW_CONT_GRAMMAR_H_

#include "new_cont.h"

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

namespace calchart {
namespace continuity {

using pos_iterator_t = boost::spirit::line_pos_iterator<std::string::const_iterator>;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

///////////////////////////////////////////////////////////////////////////
//  Our point grammar definition
///////////////////////////////////////////////////////////////////////////

template <typename Iterator>
struct variable_grammar : qi::grammar<Iterator, Variable(), ascii::space_type>
{
	variable_grammar(Iterator first) : variable_grammar::base_type(variable), annotate(first)
	{

		static const struct variableT_ : qi::symbols<char, Variable>
		{
			variableT_()
			{
				add
				("a"   , Variable(CONTVAR_A))
				("b"   , Variable(CONTVAR_B))
				("c"   , Variable(CONTVAR_C))
				("d"   , Variable(CONTVAR_D))
				("x"   , Variable(CONTVAR_X))
				("y"   , Variable(CONTVAR_Y))
				("z"   , Variable(CONTVAR_Z))
				("dof" , Variable(CONTVAR_DOF))
				("doh" , Variable(CONTVAR_DOH))
				;
			}
		} variableTable;

		// grammar definition:
		variable %= qi::no_case [ variableTable ] ;

		// annotate on success
		qi::on_success(variable, annotate(qi::_val, qi::labels::_1, qi::labels::_3));

		BOOST_SPIRIT_DEBUG_NODE(variable);
	}
	qi::rule<Iterator, Variable(), ascii::space_type> variable;
	boost::phoenix::function<annotation_f<Iterator>> annotate;
};

template <typename Iterator>
struct point_grammar : qi::grammar<Iterator, Point(), ascii::space_type>
{
	point_grammar(Iterator first) : point_grammar::base_type(point), annotate(first)
	{
		// name rules to help with debugging
		point.name("point");
		refpoint.name("refpoint");

		static const struct PointT_ : qi::symbols<char, Point>
		{
			PointT_()
			{
				add
				("p"     , CurrentPoint())
				("sp"    , StartPoint())
				("np"    , NextPoint())
				;
			}
		} PointT;

		// grammar definition:
		refpoint %= qi::no_case[ qi::lit("r") >> qi::int_ ] ;
		point    %= qi::no_case[ (PointT | refpoint) ] ;

		// annotate on success
		qi::on_success(point, annotate(qi::_val, qi::labels::_1, qi::labels::_3));

		// debug
//		qi::debug(point);
//		qi::debug(refpoint);
	}
	qi::rule<Iterator, Point(), ascii::space_type> point;
	qi::rule<Iterator, RefPoint(), ascii::space_type> refpoint;
	boost::phoenix::function<annotation_f<Iterator>> annotate;
};


template <typename Iterator>
struct value_grammar : qi::grammar<Iterator, Value(), ascii::space_type>
{
	value_grammar(Iterator first) : value_grammar::base_type(expression), variable(first), point(first), annotate(first)
	{
		using boost::phoenix::construct;
		using namespace qi::labels;
		using boost::phoenix::val;

		static const struct ValueTable_ : qi::symbols<char, double>
		{
			ValueTable_()
			{
				add
				("n"     , 0.0)
				("nw"    , 45.0)
				("w"     , 90.0)
				("sw"    , 135.0)
				("s"     , 180.0)
				("se"    , 225.0)
				("e"     , 270.0)
				("ne"    , 315.0)
				("hs"    , 1.0)
				("mm"    , 1.0)
				("sh"    , 0.5)
				("js"    , 0.5)
				("gv"    , 1.0)
				("m"     , 4.0/3.0)
				("dm"    , std::sqrt(2.0))
				;
			}
		} ValueTable;
		static const struct ValueREMT_ : qi::symbols<char, Value>
		{
			ValueREMT_()
			{
				add
				("rem"   , ValueREM())
				;
			}
		} ValueREMTable;

		FuncDir      %= qi::no_case [ qi::lit("dir") >> '(' >> point >> ')' ] ;
		FuncDirFrom  %= qi::no_case [ qi::lit("dirfrom") >> '(' >> point >> point >> ')' ] ;
		FuncDist     %= qi::no_case [ qi::lit("dist") >> '(' >> point >> ')' ] ;
		FuncDistFrom %= qi::no_case [ qi::lit("distfrom") >> '(' >> point >> point >> ')' ] ;
		FuncEither   %= qi::no_case [ qi::lit("either") > '(' >> expression >> expression >> point >> ')' ] ;
		FuncOpp      %= qi::no_case [ qi::lit("opp") >> '(' >> expression >> ')' ] ;
		FuncStep     %= qi::no_case [ qi::lit("step") >> '(' >> expression >> expression >> point >> ')' ] ;
		function     %= qi::no_case [ ( FuncDir | FuncDirFrom | FuncDist | FuncDistFrom | FuncEither | FuncOpp | FuncStep ) ];

		expression   = qi::no_case [ term [ _val = _1 ]
					                 >> *( ( '+' >> term [ _val = boost::phoenix::construct<ValueAdd>(_val, _1) ] )
						                   | ( '-' >> term [ _val = boost::phoenix::construct<ValueSub>(_val, _1) ] )
										 ) ];
		term         = qi::no_case [ factor [ _val = _1 ]
					   >> *( ( '*' >> term [ _val = boost::phoenix::construct<ValueMult>(_val, _1) ] )
						     | ( '/' >> term [ _val = boost::phoenix::construct<ValueDiv>(_val, _1) ] )
						   ) ];
		factor       %= qi::no_case [ ( qi::double_ | function | ( '(' >> expression >> ')' ) | ('-' >> factor [_val = boost::phoenix::construct<ValueNeg>(_1) ]) | ValueREMTable | ValueTable | variable ) ];


		qi::on_error<qi::fail>
		(
		 expression
		 , std::cout
		 << val("Error! Expecting ")
		 << _4                               // what failed?
		 << val(" here: \"")
		 << boost::phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
		 << val("\"")
		 << std::endl
		 );
		
		BOOST_SPIRIT_DEBUG_NODE(expression);
		BOOST_SPIRIT_DEBUG_NODE(term);
		BOOST_SPIRIT_DEBUG_NODE(factor);
		BOOST_SPIRIT_DEBUG_NODE(function);
		BOOST_SPIRIT_DEBUG_NODE(FuncDir);
		BOOST_SPIRIT_DEBUG_NODE(FuncDirFrom);
		BOOST_SPIRIT_DEBUG_NODE(FuncDist);
		BOOST_SPIRIT_DEBUG_NODE(FuncDistFrom);
		BOOST_SPIRIT_DEBUG_NODE(FuncEither);
		BOOST_SPIRIT_DEBUG_NODE(FuncOpp);
		BOOST_SPIRIT_DEBUG_NODE(FuncStep);
	
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
	qi::rule<Iterator, Value(), ascii::space_type> factor;
	qi::rule<Iterator, Value(), ascii::space_type> term;
	qi::rule<Iterator, Value(), ascii::space_type> expression;
	qi::rule<Iterator, Value(), ascii::space_type> ValueE;
	qi::rule<Iterator, Function(), ascii::space_type> function;
	qi::rule<Iterator, FunctionDir(), ascii::space_type> FuncDir;
	qi::rule<Iterator, FunctionDirFrom(), ascii::space_type> FuncDirFrom;
	qi::rule<Iterator, FunctionDist(), ascii::space_type> FuncDist;
	qi::rule<Iterator, FunctionDistFrom(), ascii::space_type> FuncDistFrom;
	qi::rule<Iterator, FunctionEither(), ascii::space_type> FuncEither;
	qi::rule<Iterator, FunctionOpposite(), ascii::space_type> FuncOpp;
	qi::rule<Iterator, FunctionStep(), ascii::space_type> FuncStep;
	variable_grammar<Iterator> variable;
	point_grammar<Iterator> point;
	boost::phoenix::function<annotation_f<Iterator>> annotate;
};



template <typename Iterator>
struct procedure_grammar : qi::grammar<Iterator, std::vector<Procedure>(), ascii::space_type>
{
	procedure_grammar(Iterator first) : procedure_grammar::base_type(procedure), variable(first), point(first), value(first), annotate(first)
	{
		using namespace qi::labels;
		using boost::phoenix::val;
			using ascii::char_;

		static const struct ProcBlamTable_ : qi::symbols<char, ProcedureBlam>
		{
			ProcBlamTable_()
			{
				add
				("blam"     , ProcedureBlam())
				;
			}
		} ProcBlamTable;
		ProcSet       %= variable > char_('=') > value;
		ProcCM        %= qi::no_case [ qi::lit("countermarch") >> point >> point >> value >> value >> value >> value ] ;
		ProcDMCM      %= qi::no_case [ qi::lit("dmcm") >> point >> point >> value ] ;
		ProcDMHS      %= qi::no_case [ qi::lit("dmhs") >> point ] ;
		ProcEven      %= qi::no_case [ qi::lit("even") >> value >> point ] ;
		ProcEWNS      %= qi::no_case [ qi::lit("ewns") >> point ] ;
		ProcFountain1 %= qi::no_case [ qi::lit("fountain") >> value >> value >> value >> value >> point ] ;
		ProcFountain2 %= qi::no_case [ qi::lit("fountain") >> value >> value >> point ] ;
		ProcFM        %= qi::no_case [ qi::lit("fm") >> value >> value ] ;
		ProcFMTO      %= qi::no_case [ qi::lit("fmto") >> point ] ;
		ProcGrid      %= qi::no_case [ qi::lit("grid") >> value ] ;
		ProcHSCM      %= qi::no_case [ qi::lit("hscm") >> point >> point >> value ] ;
		ProcHSDM      %= qi::no_case [ qi::lit("hsdm") >> point ] ;
		ProcMagic     %= qi::no_case [ qi::lit("magic") >> point ] ;
		ProcMarch1    %= qi::no_case [ qi::lit("march") >> value >> value >> value >> value ] ;
		ProcMarch2    %= qi::no_case [ qi::lit("march") >> value >> value >> value ] ;
		ProcMT        %= qi::no_case [ ( qi::lit("close") | qi::lit("mt") ) >> value >> value ] ;
		ProcMTRM      %= qi::no_case [ qi::lit("mtrm") >> value ] ;
		ProcNSEW      %= qi::no_case [ qi::lit("nsew") >> point ] ;
		ProcRotate    %= qi::no_case [ qi::lit("rotate") >> value >> value >> point ] ;
		tprocedure    %= qi::no_case [ ( ProcBlamTable | ProcCM | ProcDMCM | ProcDMHS | ProcEven | ProcEWNS | ProcFountain1 | ProcFountain2 | ProcFM | ProcFMTO | ProcGrid | ProcHSCM | ProcHSDM | ProcMagic | ProcMarch1 | ProcMarch2 | ProcMT | ProcMTRM | ProcNSEW | ProcRotate | ProcSet ) ];
		procedure     %= *( tprocedure );

		// annotate on success
		qi::on_success(tprocedure, annotate(qi::_val, qi::labels::_1, qi::labels::_3));

		// name rules to help with debugging
		BOOST_SPIRIT_DEBUG_NODE(procedure);
		BOOST_SPIRIT_DEBUG_NODE(tprocedure);
	}
	qi::rule<Iterator, std::vector<Procedure>(), ascii::space_type> procedure;
	qi::rule<Iterator, Procedure(), ascii::space_type> tprocedure;
	qi::rule<Iterator, ProcedureSet(),      ascii::space_type> ProcSet;
	qi::rule<Iterator, ProcedureCM(),       ascii::space_type> ProcCM;
	qi::rule<Iterator, ProcedureDMCM(),     ascii::space_type> ProcDMCM;
	qi::rule<Iterator, ProcedureDMHS(),     ascii::space_type> ProcDMHS;
	qi::rule<Iterator, ProcedureEven(),     ascii::space_type> ProcEven;
	qi::rule<Iterator, ProcedureEWNS(),     ascii::space_type> ProcEWNS;
	qi::rule<Iterator, ProcedureFountain1(), ascii::space_type> ProcFountain1;
	qi::rule<Iterator, ProcedureFountain2(), ascii::space_type> ProcFountain2;
	qi::rule<Iterator, ProcedureFM(),       ascii::space_type> ProcFM;
	qi::rule<Iterator, ProcedureFMTO(),     ascii::space_type> ProcFMTO;
	qi::rule<Iterator, ProcedureGrid(),     ascii::space_type> ProcGrid;
	qi::rule<Iterator, ProcedureHSCM(),     ascii::space_type> ProcHSCM;
	qi::rule<Iterator, ProcedureHSDM(),     ascii::space_type> ProcHSDM;
	qi::rule<Iterator, ProcedureMagic(),    ascii::space_type> ProcMagic;
	qi::rule<Iterator, ProcedureMarch1(),    ascii::space_type> ProcMarch1;
	qi::rule<Iterator, ProcedureMarch2(),    ascii::space_type> ProcMarch2;
	qi::rule<Iterator, ProcedureMT(),       ascii::space_type> ProcMT;
	qi::rule<Iterator, ProcedureMTRM(),     ascii::space_type> ProcMTRM;
	qi::rule<Iterator, ProcedureNSEW(),     ascii::space_type> ProcNSEW;
	qi::rule<Iterator, ProcedureRotate(),   ascii::space_type> ProcRotate;
	variable_grammar<Iterator> variable;
	point_grammar<Iterator> point;
	value_grammar<Iterator> value;
	boost::phoenix::function<annotation_f<Iterator>> annotate;
};


} // namespace continuity
} // namespace calchart

#endif // _NEW_CONT_GRAMMAR_H_

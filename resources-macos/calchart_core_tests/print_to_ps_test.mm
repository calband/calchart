//
//  cc_show_tests.m
//  cc_show_tests
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "print_ps.h"
#import "cc_show.h"
#import "modes.h"
#import "zllrbach.h"
#import <regex>

@interface print_ps_tests : XCTestCase

@end

@implementation print_ps_tests

std::string head_font_str = "Palatino-Bold";
std::string main_font_str = "Helvetica";
std::string number_font_str = "Helvetica-Bold";
std::string cont_font_str = "Courier";
std::string bold_font_str = "Courier-Bold";
std::string ital_font_str = "Courier-Italic";
std::string bold_ital_font_str = "Courier-BoldItalic";

double PageWidth = 7.5;
double PageHeight = 10.0;
double PageOffsetX = 0.5;
double PageOffsetY = 0.5;
double PaperLength = 11.0;

double HeaderSize = 3.0;
double YardsSize = 1.5;
double TextSize = 10.0;
double DotRatio = 0.9;
double NumRatio = 1.35;
double PLineRatio = 1.2;
double SLineRatio = 1.2;
double ContRatio = 0.2;

static const CalChart::ShowMode Standard_mode = CalChart::ShowMode::GetDefaultShowMode();

const std::string standard_mode_ps_gold = R"(%!PS-Adobe-3.0
%%BoundingBox: 36 37 444 756
%%CreationDate: Sun Jul  8 13:33:17 2018
%%Title: show
%%Creator: CalChart
%%Pages: (atend)
%%PageOrder: Ascend
%%EndComments
%%BeginProlog
/whash 32 def
/ehash 52 def
/fieldw 653.64 def
/fieldh 342.80 def
/smoveto { transform round exch round exch itransform moveto } def
/slineto { transform round exch round exch itransform lineto } def
/dotbox {
   /y exch def
   /x exch def
   newpath
   x w sub y w sub smoveto
   x w add y w sub slineto
   x w add y w add slineto
   x w sub y w add slineto
   closepath
   fill
} bind def
/drawfield {
   newpath 0 0 smoveto
   0 fieldh slineto
   fieldw fieldh slineto
   fieldw 0 slineto
   closepath stroke
   0 84 ehash sub fieldh mul 84 div smoveto
   .2 fieldw mul 20 div 84 ehash sub fieldh mul 84 div slineto stroke
   19.8 fieldw mul 20 div 84 ehash sub fieldh mul 84 div smoveto
   fieldw 84 ehash sub fieldh mul 84 div slineto stroke
   0 84 whash sub fieldh mul 84 div smoveto
   .2 fieldw mul 20 div 84 whash sub fieldh mul 84 div slineto stroke
   19.8 fieldw mul 20 div 84 whash sub fieldh mul 84 div smoveto
   fieldw 84 whash sub fieldh mul 84 div slineto stroke
   1 1 19 {
      dup fieldw mul 20 div dup 0 smoveto fieldh slineto stroke
      dup .2 sub fieldw mul 20 div 84 ehash sub fieldh mul 84 div smoveto
      dup .2 add fieldw mul 20 div 84 ehash sub fieldh mul 84 div slineto stroke
      dup .2 sub fieldw mul 20 div 84 whash sub fieldh mul 84 div smoveto
      .2 add fieldw mul 20 div 84 whash sub fieldh mul 84 div slineto stroke
   } for
} bind def
%%EndProlog
%%BeginSetup
%% No setup needed
%%EndSetup
%%Page: 1
0 setgray
0.25 setlinewidth
36.00 37.00 translate
408.09 0 translate 90 rotate
32.68 32.65 translate
drawfield
/w 2.72 def
showpage
%%Trailer
%%Pages: 1
%%EOF
)";


void PrintToPS(CalChart::Show const& show, CalChart::ShowMode const& mode, bool landscape, bool cont, bool contsheet, bool overview, std::ostream& output)
{
    CalChart::PrintShowToPS printShowToPS(
        show, landscape, cont, contsheet, overview, 50, mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str,
            bold_font_str, ital_font_str, bold_ital_font_str } },
        PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize,
        YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio,
        ContRatio, CalChart::kDefaultYardLines);

    std::set<size_t> picked;
    for (auto i = 0; i < show.GetNumSheets(); ++i)
        picked.insert(i);

    printShowToPS(output, 0, picked, "show");
}

- (void)test_default_equal {
    auto empty_show = CalChart::Show::Create_CC_show(CalChart::ShowMode::GetDefaultShowMode());
    XCTAssert(empty_show);

    std::regex find_good_part("(.*)CreationDate(.*)Title(.*)", std::regex::extended);

    {
        std::stringstream output;
        PrintToPS(*empty_show, Standard_mode, true, true, true, true, output);
        auto output_str = output.str();
        std::smatch part_match;
        std::regex_search(output_str, part_match, find_good_part);

        std::smatch gold_part_match;
        std::regex_search(standard_mode_ps_gold, gold_part_match, find_good_part);

        XCTAssertEqual(part_match[1], gold_part_match[1]);
        XCTAssertEqual(part_match[3], gold_part_match[3]);
    }
}

- (void)test_builtin {
    CalChart::Show_UnitTests();
}


@end

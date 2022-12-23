#include "CalChartShow.h"
#include "CalChartShowMode.h"
#include "print_ps.h"
#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <regex>

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

const std::string standard_mode_ps_no_overview_gold = R"(%!PS-Adobe-3.0
%%BoundingBox: 36 36 576 756
%%CreationDate: Fri Dec 23 10:47:41 2022
%%Title: show
%%Creator: CalChart
%%Pages: (atend)
%%PageOrder: Ascend
%%DocumentNeededResources: font Palatino-Bold Helvetica Helvetica-Bold Courier Courier-Bold Courier-Italic Courier-BoldItalic
%%DocumentSuppliedResources: font CalChart
%%BeginDefaults
%%PageResources: font Palatino-Bold Helvetica Helvetica-Bold Courier Courier-Bold Courier-Italic Courier-BoldItalic CalChart
%%EndDefaults
%%EndComments
%%BeginProlog
/fieldw 640.00 def
/fieldh 336.00 def
/fieldy 140.00 def
/stepw 160 def
/whash 32 def
/ehash 52 def
/headsize 3.00 def
/yardsize 1.50 def
/smoveto { transform round exch round exch itransform moveto } def
/slineto { transform round exch round exch itransform lineto } def
/dotplain {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   stroke
} bind def
/dotsolid {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   fill
} bind def
/dotbs {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   stroke
   x plinew sub y plinew add smoveto
   x plinew add y plinew sub slineto
   stroke
} bind def
/dotsl {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   stroke
   x plinew add y plinew add smoveto
   x plinew sub y plinew sub slineto
   stroke
} bind def
/dotx {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   stroke
   x plinew sub y plinew add smoveto
   x plinew add y plinew sub slineto
   stroke
   x plinew add y plinew add smoveto
   x plinew sub y plinew sub slineto
   stroke
} bind def
/dotsolbs {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   fill
   x slinew sub y slinew add smoveto
   x slinew add y slinew sub slineto
   stroke
} bind def
/dotsolsl {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   fill
   x slinew add y slinew add smoveto
   x slinew sub y slinew sub slineto
   stroke
} bind def
/dotsolx {
   /y exch def
   /x exch def
   newpath
   x y w 0 360 arc
   closepath
   fill
   x slinew sub y slinew add smoveto
   x slinew add y slinew sub slineto
   stroke
   x slinew add y slinew add smoveto
   x slinew sub y slinew sub slineto
   stroke
} bind def
/leftText {
   x y moveto dup stringwidth pop x add /x exch def show
} bind def
/rightText {
   dup stringwidth pop
   rmargin exch sub
   y smoveto show
} bind def
/centerText {
   dup stringwidth pop 2 div
   rmargin lmargin sub 2 div
   exch sub
   lmargin add
   y smoveto show
} bind def
/drawTextfield {
   lmargin y smoveto rmargin y slineto stroke
   /y y h 1.1 mul sub def centerText
} bind def
/space_over {
   /x x ( ) stringwidth pop add def
} bind def
/do_tab {
   /tabnum exch def
   x tabnum lt {/x tabnum def} {space_over} ifelse
} bind def
/donumber {
   w 1.25 mul add /y exch def /rmargin exch def rightText
} bind def
/donumber2 {
   w 1.25 mul add /y exch def /x exch def leftText
} bind def
/drawwhash {
   x y smoveto x w 10 div add y slineto stroke
   x w .9 mul add y smoveto x w add y slineto stroke
   x w .2 mul add dup y smoveto y w .2 mul sub slineto stroke
   x w .4 mul add dup y smoveto y w .2 mul sub slineto stroke
   x w .6 mul add dup y smoveto y w .2 mul sub slineto stroke
   x w .8 mul add dup y smoveto y w .2 mul sub slineto stroke
} bind def
/drawehash {
   x y smoveto x w 10 div add y slineto stroke
   x w .9 mul add y smoveto x w add y slineto stroke
   x w .2 mul add dup y smoveto y w .2 mul add slineto stroke
   x w .4 mul add dup y smoveto y w .2 mul add slineto stroke
   x w .6 mul add dup y smoveto y w .2 mul add slineto stroke
   x w .8 mul add dup y smoveto y w .2 mul add slineto stroke
} bind def
/doarrows {
   /mainfont findfont fieldw stepw div 1.5 mul scalefont setfont
   /x fieldw stepw div def
   /lmargin 0 def /rmargin fieldw stepw div 8 mul def
   /y fieldw neg stepw div 4 mul def
   newpath rmargin y smoveto 0 y slineto stroke
   x y x sub smoveto 0 y slineto x y x add slineto stroke
   /y y x sub fieldw stepw div 1.5 mul sub def (south) centerText
   /y fieldw stepw div 4 mul fieldh add def
   newpath rmargin y smoveto 0 y slineto
   x y x sub smoveto 0 y slineto x y x add slineto stroke
   /y y x add def (south) centerText
   /lmargin fieldw dup stepw div 8 mul sub def /rmargin fieldw def
   /y fieldw neg stepw div 4 mul def
   newpath lmargin y smoveto rmargin y slineto
   rmargin x sub y x sub smoveto
   rmargin y slineto
   rmargin x sub y x add slineto stroke
   /y y x sub fieldw stepw div 1.5 mul sub def (north) centerText
   /y fieldw stepw div 4 mul fieldh add def
   newpath lmargin y smoveto rmargin y slineto
   rmargin x sub y x sub smoveto
   rmargin y slineto
   rmargin x sub y x add slineto stroke
   /y y x add def (north) centerText
   /lmargin fieldw .9 mul def /rmargin fieldw def
   newpath lmargin fieldy neg smoveto
   rmargin fieldy neg slineto
   rmargin fieldw stepw div 4 mul fieldy sub slineto
   lmargin fieldw stepw div 4 mul fieldy sub slineto closepath stroke
   /mainfont findfont fieldw stepw div 3 mul scalefont setfont
   /y fieldw stepw div 2 div fieldy sub def pagenumtext centerText
} bind def
/drawfield {
   newpath 0 0 smoveto
   0 fieldh slineto
   fieldw fieldh slineto
   fieldw 0 slineto
   closepath stroke
   /w fieldw stepw div 8 mul def
   /x w def stepw 8 idiv 1 sub {
      x 0 smoveto x fieldh slineto stroke /x x w add def
   } repeat
   [ fieldw stepw div ] 0 setdash
   /x w 2 div def stepw 8 idiv {
      x fieldh smoveto x 0 slineto stroke /x x w add def
   } repeat
   4 4 80 {
       dup dup 84 whash sub ne exch 84 ehash sub ne and {
          dup fieldh 84 div mul 0 exch smoveto
          fieldh 84 div mul fieldw exch slineto stroke
       } if
   } for
   [] 0 setdash
   /y fieldh 84 whash sub mul 84 div def
   /x 0 def stepw 8 idiv {drawwhash /x x w add def} repeat
   /y fieldh 84 ehash sub mul 84 div def
   /x 0 def stepw 8 idiv {drawehash /x x w add def} repeat
   doarrows
   /headfont findfont fieldw stepw div headsize mul scalefont setfont
   /lmargin 0 def /rmargin fieldw def
   /y fieldw stepw div 16 headsize sub mul fieldh add def
   (UNIVERSITY OF CALIFORNIA MARCHING BAND) centerText
   /h fieldw stepw div 1.5 mul def
   /mainfont findfont h scalefont setfont
   /lmargin fieldw 4 div def /rmargin fieldw .75 mul def
   /y fieldw stepw div 8 mul fieldh add def
   (Music) drawTextfield
   /y fieldw stepw div 4 mul fieldh add def
   (Formation) drawTextfield
   /lmargin 0 def /rmargin fieldw 8 div def
   /y fieldw stepw div 10 mul fieldh add def
   (game) drawTextfield
   /lmargin fieldw .875 mul def /rmargin fieldw def
   /y fieldw stepw div 10 mul fieldh add def
   (page) drawTextfield
   /mainfont findfont fieldw stepw div 3 mul scalefont setfont
   /y fieldw stepw div 10.5 mul fieldh add def pagenumtext centerText
   /mainfont findfont fieldw stepw div yardsize mul scalefont setfont
   /lmargin 0 def /rmargin fieldw def
   /y fieldw neg stepw div 8 mul def
   (CAL SIDE) centerText
} bind def
/ReencodeFont {
   findfont
   dup length 1 add dict begin
      {1 index /FID ne {def} {pop pop} ifelse} forall
      /Encoding exch def
      currentdict
   end
   definefont pop
} bind def
%%EndProlog
%%BeginSetup
%%IncludeResources: font Palatino-Bold Helvetica Helvetica-Bold Courier Courier-Bold Courier-Italic Courier-BoldItalic
/headfont0 /Palatino-Bold def
/mainfont0 /Helvetica def
/numberfont0 /Helvetica-Bold def
/contfont0 /Courier def
/boldfont0 /Courier-Bold def
/italfont0 /Courier-Italic def
/bolditalfont0 /Courier-BoldItalic def
% run vmstatus.ps to get these values if changes made
%%BeginResource: font CalChart 5007 4717
8 dict begin
/FontType 3 def
/FontMatrix [.001 0 0 .001 0 0] def
/FontBBox [0 0 1000 1000] def
/Encoding 256 array def
0 1 255 {Encoding exch /.notdef put} for
Encoding 65 /plainman put
Encoding 66 /solman put
Encoding 67 /bsman put
Encoding 68 /slman put
Encoding 69 /xman put
Encoding 70 /solbsman put
Encoding 71 /solslman put
Encoding 72 /solxman put
/CharProcs 9 dict def
CharProcs begin
/.notdef {} def
/plainman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   stroke
   0.25 setlinewidth
} bind def
/solman {
   newpath
   300 300 250 0 360 arc
   closepath
   fill
} bind def
/bsman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   stroke
   0 600 moveto
   600 0 lineto
   stroke
   0.25 setlinewidth
} bind def
/slman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   stroke
   600 600 moveto
   0 0 lineto
   stroke
   0.25 setlinewidth
} bind def
/xman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   stroke
   0 600 moveto
   600 0 lineto
   stroke
   600 600 moveto
   0 0 lineto
   stroke
   0.25 setlinewidth
} bind def
/solbsman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   fill
   0 600 moveto
   600 0 lineto
   stroke
   0.25 setlinewidth
} bind def
/solslman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   fill
   600 600 moveto
   0 0 lineto
   stroke
   0.25 setlinewidth
} bind def
/solxman {
   30 setlinewidth
   newpath
   300 300 250 0 360 arc
   closepath
   fill
   0 600 moveto
   600 0 lineto
   stroke
   600 600 moveto
   0 0 lineto
   stroke
   0.25 setlinewidth
} bind def
end
/BuildGlyph {
   700 0 0 0 600 600 setcachedevice
   exch /CharProcs get exch
   2 copy known not {pop /.notdef} if
   get exec
} bind def
/BuildChar {
   1 index /Encoding get exch get
   1 index /BuildGlyph get exec
} bind def
currentdict
end
/CalChart exch definefont pop
%%EndResource
/Enc-iso8859 [ /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /space /exclam /quotedbl /numbersign /dollar /percent /ampersand
     /quoteright /parenleft /parenright /asterisk /plus /comma /hyphen /period
     /slash /zero /one /two /three /four /five /six /seven /eight /nine /colon
     /semicolon /less /equal /greater /question /at /A /B /C /D /E /F /G /H /I
     /J /K /L /M /N /O /P /Q /R /S /T /U /V /W /X /Y /Z /bracketleft /backslash
     /bracketright /asciicircum /underscore /quoteleft /a /b /c /d /e /f /g /h
     /i /j /k /l /m /n /o /p /q /r /s /t /u /v /w /x /y /z /braceleft /bar
     /braceright /asciitilde /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef /.notdef
     /.notdef /.notdef /.notdef /.notdef /dotlessi /grave /acute /circumflex
     /tilde /macron /breve /dotaccent /dieresis /.notdef /ring /cedilla
     /.notdef /hungarumlaut /ogonek /caron /space /exclamdown /cent /sterling
     /currency /yen /brokenbar /section /dieresis /copyright /ordfeminine
     /guillemotleft /logicalnot /hyphen /registered /macron /degree /plusminus
     /twosuperior /threesuperior /acute /mu /paragraph /periodcentered /cedilla
     /onesuperior /ordmasculine /guillemotright /onequarter /onehalf
     /threequarters /questiondown /Agrave /Aacute /Acircumflex /Atilde
     /Adieresis /Aring /AE /Ccedilla /Egrave /Eacute /Ecircumflex /Edieresis
     /Igrave /Iacute /Icircumflex /Idieresis /Eth /Ntilde /Ograve /Oacute
     /Ocircumflex /Otilde /Odieresis /multiply /Oslash /Ugrave /Uacute
     /Ucircumflex /Udieresis /Yacute /Thorn /germandbls /agrave /aacute
     /acircumflex /atilde /adieresis /aring /ae /ccedilla /egrave /eacute
     /ecircumflex /edieresis /igrave /iacute /icircumflex /idieresis /eth
     /ntilde /ograve /oacute /ocircumflex /otilde /odieresis /divide /oslash
     /ugrave /uacute /ucircumflex /udieresis /yacute /thorn /ydieresis] def
/headfont Enc-iso8859 headfont0 ReencodeFont
/mainfont Enc-iso8859 mainfont0 ReencodeFont
/numberfont Enc-iso8859 numberfont0 ReencodeFont
/contfont Enc-iso8859 contfont0 ReencodeFont
/boldfont Enc-iso8859 boldfont0 ReencodeFont
/italfont Enc-iso8859 italfont0 ReencodeFont
/bolditalfont Enc-iso8859 bolditalfont0 ReencodeFont
%%EndSetup
%%Page: 1
/pagenumtext () def
0 setgray
0.25 setlinewidth
36.00 36.00 translate
540.00 0 translate 90 rotate
40.00 140.00 translate
drawfield
/mainfont findfont 6.00 scalefont setfont
/lmargin 0.00 def /rmargin 0.00 def
/y 338.00 def
(0) dup centerText
/y -8.00 def
centerText
/lmargin 32.00 def /rmargin 32.00 def
/y 338.00 def
(5) dup centerText
/y -8.00 def
centerText
/lmargin 64.00 def /rmargin 64.00 def
/y 338.00 def
(10) dup centerText
/y -8.00 def
centerText
/lmargin 96.00 def /rmargin 96.00 def
/y 338.00 def
(15) dup centerText
/y -8.00 def
centerText
/lmargin 128.00 def /rmargin 128.00 def
/y 338.00 def
(20) dup centerText
/y -8.00 def
centerText
/lmargin 160.00 def /rmargin 160.00 def
/y 338.00 def
(25) dup centerText
/y -8.00 def
centerText
/lmargin 192.00 def /rmargin 192.00 def
/y 338.00 def
(30) dup centerText
/y -8.00 def
centerText
/lmargin 224.00 def /rmargin 224.00 def
/y 338.00 def
(35) dup centerText
/y -8.00 def
centerText
/lmargin 256.00 def /rmargin 256.00 def
/y 338.00 def
(40) dup centerText
/y -8.00 def
centerText
/lmargin 288.00 def /rmargin 288.00 def
/y 338.00 def
(45) dup centerText
/y -8.00 def
centerText
/lmargin 320.00 def /rmargin 320.00 def
/y 338.00 def
(50) dup centerText
/y -8.00 def
centerText
/lmargin 352.00 def /rmargin 352.00 def
/y 338.00 def
(45) dup centerText
/y -8.00 def
centerText
/lmargin 384.00 def /rmargin 384.00 def
/y 338.00 def
(40) dup centerText
/y -8.00 def
centerText
/lmargin 416.00 def /rmargin 416.00 def
/y 338.00 def
(35) dup centerText
/y -8.00 def
centerText
/lmargin 448.00 def /rmargin 448.00 def
/y 338.00 def
(30) dup centerText
/y -8.00 def
centerText
/lmargin 480.00 def /rmargin 480.00 def
/y 338.00 def
(25) dup centerText
/y -8.00 def
centerText
/lmargin 512.00 def /rmargin 512.00 def
/y 338.00 def
(20) dup centerText
/y -8.00 def
centerText
/lmargin 544.00 def /rmargin 544.00 def
/y 338.00 def
(15) dup centerText
/y -8.00 def
centerText
/lmargin 576.00 def /rmargin 576.00 def
/y 338.00 def
(10) dup centerText
/y -8.00 def
centerText
/lmargin 608.00 def /rmargin 608.00 def
/y 338.00 def
(5) dup centerText
/y -8.00 def
centerText
/lmargin 640.00 def /rmargin 640.00 def
/y 338.00 def
(0) dup centerText
/y -8.00 def
centerText
/w 1.8000 def
/plinew 2.1600 def
/slinew 2.1600 def
/numberfont findfont 4.86 scalefont setfont
-40.00 -140.00 translate
showpage
%%Trailer
%%Pages: 1
%%EOF
)";

void PrintToPS(CalChart::Show const& show, CalChart::ShowMode const& mode, bool landscape, bool cont, bool contsheet, bool overview, std::ostream& output)
{
    auto printShowToPS = CalChart::PrintShowToPS(
        show, landscape, cont, contsheet, overview, 50, mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str,
            bold_font_str, ital_font_str, bold_ital_font_str } },
        PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize,
        YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio,
        ContRatio, CalChart::ShowMode::GetDefaultYardLines());

    std::set<size_t> picked;
    for (auto i = 0; i < show.GetNumSheets(); ++i)
        picked.insert(i);

    printShowToPS(output, picked, "show");
}

TEST_CASE("CalChartTestPSPrintDefault")
{
    auto empty_show = CalChart::Show::Create(CalChart::ShowMode::GetDefaultShowMode());
    REQUIRE(empty_show);

    std::regex find_good_part("(.*)CreationDate(.*)Title(.*)", std::regex::extended);

    {
        std::stringstream output;
        PrintToPS(*empty_show, Standard_mode, true, true, true, true, output);
        auto output_str = output.str();
        std::smatch part_match;
        std::regex_search(output_str, part_match, find_good_part);

        std::smatch gold_part_match;
        std::regex_search(standard_mode_ps_gold, gold_part_match, find_good_part);

        REQUIRE(part_match[1] == gold_part_match[1]);
        REQUIRE(part_match[3] == gold_part_match[3]);
    }
    {
        std::stringstream output;
        PrintToPS(*empty_show, Standard_mode, true, true, true, false, output);
        auto output_str = output.str();
        std::smatch part_match;
        std::regex_search(output_str, part_match, find_good_part);

        std::smatch gold_part_match;
        std::regex_search(standard_mode_ps_no_overview_gold, gold_part_match, find_good_part);

        REQUIRE(part_match[1] == gold_part_match[1]);
        REQUIRE(part_match[3] == gold_part_match[3]);
    }
}

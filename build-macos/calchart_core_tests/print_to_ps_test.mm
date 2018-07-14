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

auto Get_yard_text = [](size_t offset) {
    static const std::string yard_text[] = {
        "N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D",
        "C", "B", "A", "-10", "-5", "0", "5", "10", "15", "20", "25",
        "30", "35", "40", "45", "50", "45", "40", "35", "30", "25", "20",
        "15", "10", "5", "0", "-5", "-10", "A", "B", "C", "D", "E",
        "F", "G", "H", "I", "J", "K", "L", "M", "N"
    };
    return yard_text[offset];
};
auto Get_spr_line_text = Get_yard_text;

std::unique_ptr<CalChart::ShowMode> Standard_mode = CalChart::ShowModeStandard::CreateShowMode(
    "Standard", { { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } });

std::unique_ptr<CalChart::ShowMode> SpringShow_mode = CalChart::ShowModeSprShow::CreateSpringShowMode(
    "Zellerbach", { { 0xD, 8, 8, 8, 8, -16, -30, 32, 28, 0, 0, 571, 400, 163, 38, 265, 232,
            153, 438, 270, 12 } });

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


const std::string springshow_mode_ps_gold = R"(%!PS-Adobe-3.0
%%BoundingBox: 36 36 576 756
%%CreationDate: Sun Jul  8 14:25:37 2018
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
/fieldw 564.30 def
/fieldh 395.31 def
/fieldy 32.69 def
/sfieldw 261.89 def
/sfieldh 229.28 def
/sfieldx 161.09 def
/sfieldy 37.55 def
/stepsize 8.184105 def
/sprstepsize 7.000000 def
/nfieldw 32 def
/nfieldh 28 def
/headsize 3.00 def
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
/doarrows {
   /mainfont findfont sprstepsize 1.5 mul scalefont setfont
   /x sprstepsize def
   /lmargin 0 def /rmargin sprstepsize 8 mul def
   /y sprstepsize neg 4 mul def
   newpath rmargin y smoveto 0 y slineto stroke
   x y x sub smoveto 0 y slineto x y x add slineto stroke
   /y y x sub sprstepsize 1.5 mul sub def (south) centerText
   /y sprstepsize 4 mul fieldh add def
   newpath rmargin y smoveto 0 y slineto
   x y x sub smoveto 0 y slineto x y x add slineto stroke
   /y y x add def (south) centerText
   /lmargin fieldw sprstepsize 8 mul sub def /rmargin fieldw def
   /y sprstepsize neg 4 mul def
   newpath lmargin y smoveto rmargin y slineto
   rmargin x sub y x sub smoveto
   rmargin y slineto
   rmargin x sub y x add slineto stroke
   /y y x sub sprstepsize 1.5 mul sub def (north) centerText
   /y sprstepsize 4 mul fieldh add def
   newpath lmargin y smoveto rmargin y slineto
   rmargin x sub y x sub smoveto
   rmargin y slineto
   rmargin x sub y x add slineto stroke
   /y y x add def (north) centerText
   /lmargin fieldw .9 mul def /rmargin fieldw def
   newpath lmargin fieldy neg smoveto
   rmargin fieldy neg slineto
   rmargin sprstepsize 4 mul fieldy sub slineto
   lmargin sprstepsize 4 mul fieldy sub slineto closepath stroke
   /mainfont findfont sprstepsize 3 mul scalefont setfont
   /y sprstepsize 2 div fieldy sub def pagenumtext centerText
} bind def
/drawfield {
   sfieldx sfieldy translate
   /w stepsize 8 mul def
   /x 0 def nfieldw 8 idiv 1 add {
      x 0 smoveto x sfieldh slineto stroke /x x w add def
   } repeat
   [ stepsize ] 0 setdash
   /x w 2 div def nfieldw 8 idiv {
      x sfieldh smoveto x 0 slineto stroke /x x w add def
   } repeat
   [] 0 setdash
   /h sfieldh nfieldh div 8 mul def
   /y sfieldh def nfieldh 3 add 8 idiv 1 add {
      0 y smoveto sfieldw y slineto stroke /y y h sub def
   } repeat
   [ sfieldh nfieldh div ] 0 setdash
   /y sfieldh h 2 div sub def nfieldh 7 add 8 idiv {
      0 y smoveto sfieldw y slineto stroke /y y h sub def
   } repeat
   [] 0 setdash
   sfieldx neg sfieldy neg translate
   doarrows
   /headfont findfont sprstepsize headsize mul scalefont setfont
   /lmargin 0 def /rmargin fieldw def
   /y sprstepsize 16 headsize sub mul fieldh add def
   (UNIVERSITY OF CALIFORNIA MARCHING BAND) centerText
   /h sprstepsize 1.5 mul def
   /mainfont findfont h scalefont setfont
   /lmargin fieldw 4 div def /rmargin fieldw .75 mul def
   /y sprstepsize 8 mul fieldh add def
   (Music) drawTextfield
   /y sprstepsize 4 mul fieldh add def
   (Formation) drawTextfield
   /lmargin 0 def /rmargin fieldw 8 div def
   /y sprstepsize 10 mul fieldh add def
   (date) drawTextfield
   /lmargin fieldw .875 mul def /rmargin fieldw def
   /y sprstepsize 10 mul fieldh add def
   (page) drawTextfield
   /mainfont findfont sprstepsize 3 mul scalefont setfont
   /y sprstepsize 10.5 mul fieldh add def pagenumtext centerText
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
/BeginEPSF {
   /b4_Inc_state save def
   /dict_count countdictstack def
   /op_count count 1 sub def
   userdict begin
   /showpage {} def
   0 setgray 0 setlinecap
   1 setlinewidth 0 setlinejoin
   10 setmiterlimit [] 0 setdash newpath
   /languagelevel where
   {pop languagelevel
   1 ne
      {false setstrokeadjust false setoverprint
      } if
   } if
} bind def
/EndEPSF {
   count op_count sub {pop} repeat
   countdictstack dict_count sub {end} repeat
   b4_Inc_state restore
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
77.85 32.69 translate
   BeginEPSF
   0.99 0.99 scale
   0 0 translate
%%BeginDocument: zllrbach.eps
)";


const std::string springshow_mode_ps_gold2 = R"(%%EndDocument
EndEPSF
drawfield
/mainfont findfont 12.28 scalefont setfont
/lmargin 161.09 def /rmargin 161.09 def
/y -0.42 def
(40) centerText
/lmargin 226.56 def /rmargin 226.56 def
/y -0.42 def
(45) centerText
/lmargin 292.03 def /rmargin 292.03 def
/y -0.42 def
(50) centerText
/lmargin 357.51 def /rmargin 357.51 def
/y -0.42 def
(45) centerText
/lmargin 422.98 def /rmargin 422.98 def
/y -0.42 def
(40) centerText
/y 260.69 def
/x 432.86 def /rmargin 151.21 def
(N) leftText
(N) rightText
/y 195.18 def
/x 432.86 def /rmargin 151.21 def
(M) leftText
(M) rightText
/y 129.68 def
/x 432.86 def /rmargin 151.21 def
(L) leftText
(L) rightText
/y 64.17 def
/x 432.86 def /rmargin 151.21 def
(K) leftText
(K) rightText
/w 3.6828 def
/plinew 4.4194 def
/slinew 4.4194 def
/numberfont findfont 9.94 scalefont setfont
-77.85 -32.69 translate
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
        ContRatio, Get_yard_text, Get_spr_line_text);

    std::set<size_t> picked;
    for (auto i = 0; i < show.GetNumSheets(); ++i)
        picked.insert(i);

    printShowToPS(output, 0, picked, "show");
}

- (void)test_default_equal {
    auto empty_show = CalChart::Show::Create_CC_show();
    XCTAssert(empty_show);

    std::regex find_good_part("(.*)CreationDate(.*)Title(.*)", std::regex::extended);

    {
        std::stringstream output;
        PrintToPS(*empty_show, *Standard_mode, true, true, true, true, output);
        auto output_str = output.str();
        std::smatch part_match;
        std::regex_search(output_str, part_match, find_good_part);

        std::smatch gold_part_match;
        std::regex_search(standard_mode_ps_gold, gold_part_match, find_good_part);

        XCTAssertEqual(part_match[1], gold_part_match[1]);
        XCTAssertEqual(part_match[3], gold_part_match[3]);
    }
    {
        std::stringstream output;
        PrintToPS(*empty_show, *SpringShow_mode, true, true, true, false, output);
        auto output_str = output.str();
        std::smatch part_match;
        std::regex_search(output_str, part_match, find_good_part);

        std::smatch gold_part_match;
        auto real_gold = springshow_mode_ps_gold + zllrbach_eps + springshow_mode_ps_gold2;
        std::regex_search(real_gold, gold_part_match, find_good_part);

        XCTAssertEqual(part_match[1], gold_part_match[1]);
        XCTAssertEqual(part_match[3], gold_part_match[3]);
        std::string s1 = part_match[3];
        std::string g1 = gold_part_match[3];
    }
}

- (void)test_builtin {
    CalChart::Show_UnitTests();
}

- (void)test_assumptions {
    CalChart::ShowModeSprShow& show = dynamic_cast<CalChart::ShowModeSprShow&>(*SpringShow_mode);
    std::cout<<"Offset "<<show.Offset().x<<","<<show.Offset().y<<"\n";
    std::cout<<"FieldOffset "<<show.FieldOffset().x<<","<<show.FieldOffset().y<<"\n";
    std::cout<<"Size "<<show.Size().x<<","<<show.Size().y<<"\n";
    std::cout<<"FieldSize "<<show.FieldSize().x<<","<<show.FieldSize().y<<"\n";
    std::cout<<"MinPosition "<<show.MinPosition().x<<","<<show.MinPosition().y<<"\n";
    std::cout<<"MaxPosition "<<show.MaxPosition().x<<","<<show.MaxPosition().y<<"\n";
    std::cout<<"Border1 "<<show.Border1().x<<","<<show.Border1().y<<"\n";
    std::cout<<"StageX "<<show.StageX()<<"\n";
    std::cout<<"StageY "<<show.StageY()<<"\n";
    std::cout<<"StageW "<<show.StageW()<<"\n";
    std::cout<<"StageH "<<show.StageH()<<"\n";
    std::cout<<"FieldX "<<show.FieldX()<<"\n";
    std::cout<<"FieldY "<<show.FieldY()<<"\n";
    std::cout<<"FieldW "<<show.FieldW()<<"\n";
    std::cout<<"FieldH "<<show.FieldH()<<"\n";
    std::cout<<"TextLeft "<<show.TextLeft()<<"\n";
    std::cout<<"TextRight "<<show.TextRight()<<"\n";
    std::cout<<"TextTop "<<show.TextTop()<<"\n";
    std::cout<<"TextBottom "<<show.TextBottom()<<"\n";
    std::cout<<"StageX "<<show.StageX()<<"\n";
}


@end

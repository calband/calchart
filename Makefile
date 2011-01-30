# Makefile for calchart (UNIX).

PROG = calchart

MAKEDEP = makedepend

CONF_FLAGS = #-DANIM_OUTPUT_POVRAY -DANIM_OUTPUT_RIB
CONF_LIBS = #-lribout

DFLAGS = -g -Wall $(CONF_FLAGS)

#### TOOLS ####
LEX = flex
LFLAGS = -B -i
YACC = bison
YFLAGS = -dv

FIG2EPS = fig2dev -L ps # -P for non-encapsulated

#### Directories ####
SRCDIR = ./src
GENDIR = ./generated
RESDIR = ./resources

HEADERS = $(wildcard $(SRCDIR)/*.h)

SRCS = $(wildcard $(SRCDIR)/*.cpp)

SYNTHETIC_BASES = $(SRCDIR)/contscan.l $(SRCDIR)/contgram.y
SYNTHETIC_SRCS = $(GENDIR)/contscan.cpp $(GENDIR)/contgram.cpp
SYNTHETIC_FILES = $(SYNTHETIC_SRCS) $(GENDIR)/contgram.h $(GENDIR)/contgram.output

OBJDIR = build

OBJS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS)) 
OBJS += $(patsubst $(GENDIR)/%.cpp, $(OBJDIR)/%.o, $(SYNTHETIC_SRCS))

PSFILES = postscript/calchart.ps postscript/setup.sh postscript/vmstatus.sh \
	postscript/zllrbach.fig
RUNTIME = runtime/config runtime/prolog0.ps runtime/prolog1.ps \
	runtime/prolog2.ps runtime/setup2.ps
RUNTIME_ALL = $(RUNTIME) runtime/setup0.ps runtime/setup1.ps runtime/zllrbach.eps
PS_SYNTH_FILES = runtime/setup0.ps runtime/setup1.ps runtime/zllrbach.eps postscript/vmstatus.ps
IMAGES = $(wildcard $(RESDIR)/*.xbm)
IMAGES_X = $(wildcard $(RESDIR)/*.xpm)
IMAGES_BMP = $(IMAGES:.xbm=.bmp)
IMAGES_MSW = $(IMAGES_BMP) calchart.ico
IMAGES_ALL = $(IMAGES) $(IMAGES_X) calchart.ico
IMAGES_SYNTH = $(IMAGES_BMP)

TEXDOCS = docs/charthlp.tex docs/anim.tex docs/install.tex docs/bugs.tex \
	docs/overview.tex docs/refer.tex docs/tex2rtf.ini docs/texhelp.sty

DOCS = $(TEXDOCS) \
	docs/contents.gif docs/up.gif docs/back.gif docs/forward.gif

MOSTSRCS = $(SRCS) $(SYNTHETIC_BASES) $(HEADERS) $(DOCS)
ALLSRCS = $(MOSTSRCS) $(RUNTIME) $(IMAGES_ALL) $(PSFILES) Makefile xbm2xpm \
	makefile.wat calchart.rc install.inf
MSWSRCS = $(MOSTSRCS) contgram.h $(RUNTIME_ALL) $(SYNTHETIC_SRCS) \
	makefile.wat calchart.rc install.inf

BOOSTDIR = /opt/local/include
CXXFLAGS += `wx-config --cflags` $(USER_CXXFLAGS) -I$(RESDIR) -I$(SRCDIR) -I$(BOOSTDIR)
CXX = c++

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(GENDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(DFLAGS) -c $< -o $@

$(GENDIR)/%.cpp: $(SRCDIR)/%.y
	@mkdir -p $(GENDIR)
	$(YACC) $(YFLAGS) $<
	mv -f $*.tab.c $@

$(GENDIR)/%.h: $(SRCDIR)/%.y
	@mkdir -p $(GENDIR)
	$(YACC) $(YFLAGS) $<
	mv -f $*.tab.h $@

$(GENDIR)/%.cpp: $(SRCDIR)/%.l
	@mkdir -p $(GENDIR)
	$(LEX) $(LFLAGS) -t $< > $@

%.bmp: %.xbm
# use ImageMagick to convert
	rm -f $@
	convert $< $@

runtime/%.eps: postscript/%.fig
	$(FIG2EPS) $< $@

all: calchart $(PS_SYNTH_FILES) charthlp.xlp

$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) $(DFLAGS) $(OBJS) -o $@ `wx-config --libs`

$(OBJDIR)/contscan.o: $(GENDIR)/contgram.h

TAGS: $(SRCS) $(HEADERS)
	etags $(SRCS) $(HEADERS)

tags:: TAGS

# Stuff for help files
charthlp.xlp: docs/charthlp.xlp
	cp docs/charthlp.xlp charthlp.xlp

docs/charthlp.xlp: $(DOCS)
	cd docs; tex2rtf charthlp.tex charthlp.xlp -twice -xlp

docs/charthlp.dvi: $(DOCS)
	cd docs; latex charthlp; latex charthlp; makeindex charthlp; latex charthlp

docs/charthlp.ps: docs/charthlp.dvi
	cd docs; dvips -f -r < charthlp.dvi > charthlp.ps
docs/charthlp.ps.gz: docs/charthlp.ps
	rm -f $@
	gzip -c9 $< > $@

docs/charthlp_contents.html: $(DOCS)
	cd docs; tex2rtf charthlp.tex charthlp.html -twice -html
docs/charthlp.html.tar.gz: docs/charthlp_contents.html
	rm -f $@
	tar cf - docs/*.html | gzip -9 > $@

docs/charthlp.tex.tar.gz: $(TEXDOCS)
	rm -f $@
	tar cf - $(TEXDOCS) | gzip -9 > $@

ps: docs/charthlp.ps
html: docs/charthlp_contents.html
xlp: charthlp.xlp

# Stuff to make changing the calchart ps font easier
runtime/setup0.ps: postscript/calchart.ps postscript/setup.sh
	rm -f $@
	cd postscript; chmod +x setup.sh; ./setup.sh > ../$@

runtime/setup1.ps: postscript/calchart.ps postscript/setup.sh
	rm -f $@
	cd postscript; chmod +x setup.sh; ./setup.sh > ../$@

postscript/vmstatus.ps: postscript/calchart.ps postscript/vmstatus.sh
	rm -f $@
	cd postscript; chmod +x vmstatus.sh; ./vmstatus.sh > ../$@

chartsrc.tar.gz: $(ALLSRCS)
	rm -f $@
	mv -f Makefile Makefile.bak
	sed '/^# DO NOT DELETE/q' Makefile.bak > Makefile
	tar cf - $(ALLSRCS) | gzip -9 > $@
	mv -f Makefile.bak Makefile

chartbin.tar.gz: calchart $(RUNTIME_ALL) charthlp.xlp
	rm -f $@
	tar cf - calchart $(RUNTIME_ALL) charthlp.xlp | gzip -9 > $@

chartsrc.zip: $(MSWSRCS) $(IMAGES_MSW)
	rm -f $@
	zip -Dlqr9 $@ $(MSWSRCS)
	zip -q9 $@ $(IMAGES_MSW)

length::
	@echo `cat $(HEADERS) $(SRCS) $(SYNTHETIC_BASES) | wc -l` C++ lines in $(HEADERS) $(SRCS) $(SYNTHETIC_BASES)
	@echo `cat $(PSFILES) | wc -l` PS lines in $(PSFILES)

tar:: chartsrc.tar.gz

distrib:: chartbin.tar.gz docs/charthlp.ps.gz docs/charthlp.html.tar.gz \
	docs/charthlp.tex.tar.gz

zip:: chartsrc.zip

clean::
	rm -f $(OBJS) $(SYNTHETIC_FILES) $(PS_SYNTH_FILES) $(IMAGES_SYNTH) calchart core *~ runtime/*~ *.bak TAGS
	rm -rf $(OBJDIR)

depend::
	rm -f depend
	gcc -MM $(CXXFLAGS) $(SRCS) $(SYNTHETIC_SRCS) > depend
#	$(MAKEDEP) -- $(CXXFLAGS) $(DFLAGS) -- $(SRCS) $(SYNTHETIC_SRCS)

ifeq (depend,$(wildcard depend))
include depend
endif

# DO NOT DELETE

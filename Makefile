# Makefile for calchart (UNIX).

#### TOOLS ####
WXCONFIG = wx-config
LEX = flex
LFLAGS = -B -i
YACC = bison
YFLAGS = -dv

CXXFLAGS += `$(WXCONFIG) --cxxflags`
CXXFLAGS += -I$(RESDIR) -I$(SRCDIR) -I$(BOOSTDIR)
CXXFLAGS += -g -Wall $(CONF_FLAGS)
CXX = `$(WXCONFIG) --cxx`
LIBS = `$(WXCONFIG) --libs`

#### Directories ####
BOOSTDIR = /opt/local/include
SRCDIR = ./src
GENDIR = ./generated
RESDIR = ./resources
OBJDIR = build

#### Files ####
HEADERS = $(wildcard $(SRCDIR)/*.h)
SRCS = $(wildcard $(SRCDIR)/*.cpp)

GENERATED_BASES = $(SRCDIR)/contscan.l $(SRCDIR)/contgram.y
GENERATED_SRCS = $(GENDIR)/contscan.cpp $(GENDIR)/contgram.cpp
GENERATED_FILES = $(GENERATED_SRCS) $(GENDIR)/contgram.h

OBJS += $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS)) 
OBJS += $(patsubst $(GENDIR)/%.cpp, $(OBJDIR)/%.o, $(GENERATED_SRCS))

IMAGES = $(wildcard $(RESDIR)/*.xbm)
IMAGES_X = $(wildcard $(RESDIR)/*.xpm)
IMAGES_BMP = $(IMAGES:.xbm=.bmp)
IMAGES_MSW = $(IMAGES_BMP) calchart.ico
IMAGES_ALL = $(IMAGES) $(IMAGES_X) calchart.ico

DOCSRCDIR = doc_src
DOCSRC = $(wildcard $(DOCSRCDIR)/*)

MOSTSRCS = $(SRCS) $(GENERATED_BASES) $(HEADERS) $(DOCSRC)
ALLSRCS = $(MOSTSRCS) $(IMAGES_ALL) Makefile xbm2xpm \
	makefile.wat calchart.rc install.inf
MSWSRCS = $(MOSTSRCS) contgram.h $(GENERATED_SRCS) \
	makefile.wat calchart.rc install.inf

#### Targets ####
all: calchart

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(GENDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

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

generate: $(GENERATED_FILES)

calchart: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJDIR)/contscan.o: $(GENDIR)/contgram.h

TAGS: $(SRCS) $(HEADERS)
	etags $(SRCS) $(HEADERS)

tags:: TAGS

# Stuff for help files
docs/charthlp.dvi: html $(DOCSRC)
	cd docs; latex charthlp; latex charthlp; makeindex charthlp; latex charthlp

docs/charthlp.ps: docs/charthlp.dvi
	cd docs; dvips -f -r < charthlp.dvi > charthlp.ps

docs/charthlp.ps.gz: docs/charthlp.ps
	rm -f $@
	gzip -c9 $< > $@

docs/charthlp_contents.html: $(DOCSRC)
	mkdir -p docs
	cp $(DOCSRC) docs
	-cd docs; tex2rtf charthlp.tex charthlp.html -twice -html

docs/charthlp.html.tar.gz: docs/charthlp_contents.html
	rm -f $@
	tar cf - docs/*.html | gzip -9 > $@

docs/charthlp.tex.tar.gz: $(TEXDOCS)
	rm -f $@
	tar cf - $(TEXDOCS) | gzip -9 > $@

ps: docs/charthlp.ps
html: docs/charthlp_contents.html

chartsrc.tar.gz: $(ALLSRCS)
	rm -f $@
	mv -f Makefile Makefile.bak
	sed '/^# DO NOT DELETE/q' Makefile.bak > Makefile
	tar cf - $(ALLSRCS) | gzip -9 > $@
	mv -f Makefile.bak Makefile

chartbin.tar.gz: calchart
	rm -f $@
	tar cf - calchart | gzip -9 > $@

chartsrc.zip: $(MSWSRCS) $(IMAGES_MSW)
	rm -f $@
	zip -Dlqr9 $@ $(MSWSRCS)
	zip -q9 $@ $(IMAGES_MSW)

length::
	@echo `cat $(HEADERS) $(SRCS) $(GENERATED_BASES) | wc -l` C++ lines in $(HEADERS) $(SRCS) $(GENERATED_BASES)

tar:: chartsrc.tar.gz

distrib:: chartbin.tar.gz docs/charthlp.ps.gz docs/charthlp.html.tar.gz \
	docs/charthlp.tex.tar.gz

zip:: chartsrc.zip

clean::
	rm -f $(OBJS) $(GENERATED_FILES) calchart core *~ *.bak TAGS
	rm -rf $(OBJDIR)

depend::
	rm -f depend
	gcc -MM $(CXXFLAGS) $(SRCS) $(GENERATED_SRCS) > depend

ifeq (depend,$(wildcard depend))
include depend
endif

# DO NOT DELETE

# Makefile for calchart (Watcom for MS-Windows).

NAME = calchart

COMPMODE = win386
#COMPMODE = nt

WXDIR = d:\source\wxwin
!ifeq COMPMODE win386
!include $(WXDIR)\src\makewat.env
!endif
!ifeq COMPMODE nt
!include $(WXDIR)\src\makew32.env
!endif

LNK = $(name).lnk

.cc.obj: .AUTODEPEND
	*$(CCC) $(CPPFLAGS) /dMSDOS $(IFLAGS) $<

OBJS = main_ui.obj basic_ui.obj show.obj draw.obj print.obj print_ui.obj &
	show_ui.obj undo.obj modes.obj confgr.obj animate.obj anim_ui.obj &
	cont.obj cont_ui.obj ingl.obj contscan.obj contgram.obj
SRCS = main_ui.cc basic_ui.cc show.cc draw.cc print.cc print_ui.cc &
	show_ui.cc undo.cc modes.cc confgr.cc animate.cc anim_ui.cc &
	cont.cc cont_ui.cc ingl.cc contscan.cc contgram.cc
HEADERS = main_ui.h basic_ui.h show.h print_ui.h show_ui.h undo.h &
	modes.h confgr.h animate.h anim_ui.h cont.h cont_ui.h &
	parse.h contgram.h ingl.h platconf.h
HELPFILES = charthlp.hlp charthlp.cnt

all: $(name).exe $(HELPFILES)

$(name).exe: $(OBJS) $(LINKOBJS) $(name).res $(LNK)
	wlink @$(LNK)
	$(BINDCOMMAND) $(RESFLAGS2) $(name).res

$(name).res: $(name).rc
	$(RC) $(RESFLAGS1) $(name).rc

$(LNK) : makefile.wat
	%create $(LNK)
	@%append $(LNK) debug all
	@%append $(LNK) system $(LINKOPTION)
	@%append $(LNK) $(MINDATA)
	@%append $(LNK) $(MAXDATA)
	@%append $(LNK) $(STACK)
	@%append $(LNK) name $(name)
	@%append $(LNK) file $(WXDIR)\lib\$(WXLIB)
	@for %i in ($(EXTRALIBS)) do @%append $(LNK) file %i
	@for %i in ($(OBJS)) do @%append $(LNK) file %i

docs/charthlp.rtf: docs/charthlp.tex
	cd docs
	$(WXDIR)\utils\tex2rtf\src\tex2rtf charthlp.tex charthlp.rtf -twice -winhelp
	cd ..
docs/charthlp.hlp: docs/charthlp.rtf
	cd docs
	hc -n charthlp
	cd ..

charthlp.hlp: docs/charthlp.hlp
	copy docs\charthlp.hlp charthlp.hlp

charthlp.cnt: docs/charthlp.cnt
	copy docs\charthlp.cnt charthlp.cnt

strip: .SYMBOLIC
	wstrip $(name).exe

clean: .SYMBOLIC
	@for %i in ($(OBJS) $(name).exe $(name).res $(LNK) *.err TAGS) do -erase %i

!ifeq COMPMODE win386
distrib: .SYMBOLIC
	-erase $(name).zip
	zip -Dr9 $(name).zip $(name).exe $(HELPFILES) runtime
	zip -j9 $(name).zip $(name).zip d:\watcom\binw\wemu387.386 $(WXDIR)\contrib\ctl3d\ctl3dv2.dll
!endif
!ifeq COMPMODE nt
distrib: .SYMBOLIC
	-erase $(name).zip
	zip -Dr9 $(name).zip $(name).exe $(HELPFILES) runtime
	zip -j9 $(name).zip $(name).zip d:\watcom\binw\wemu387.386 d:\windows\system\ctl3d32.dll
!endif

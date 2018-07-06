# -*- sh -*-
#
#  jed version 0.98-5 or newer is required
#
TXT_FILES = slang.txt cslang.txt cref.txt slangfun.txt
SGML_FILES = slang.sgml cslang.sgml cref.sgml slangfun.sgml
HTML_FILES = slang.html cslang.html cref.html slangfun.html
TEX_FILES = slang.tex cslang.tex #cref.tex slangfun.tex
PS_FILES = slang.ps cslang.ps #cref.ps slangfun.ps

SGML2LATEX = sgml2latex -p letter -o tex
SGML2HTML = sgml2html
SGML2TXT = sgml2txt -f
TM2TXT = tools/$(ARCH)objs/tm2txt --quiet
LATEX = latex

TEXTDIR = ../text
PSDIR = ../ps
HTMLDIR = ../html
SGMLDIR = ../sgml

SUBDIRS = $(TEXTDIR) $(HTMLDIR) $(PSDIR) $(SGMLDIR)
SRCDIR = `pwd`

all: $(SGML_FILES) $(HTML_FILES) $(TEX_FILES) $(TXT_FILES)
text-files: $(TXT_FILES)
#----- SGML Files -----------------------------------------------------------
cslang.sgml : cslang.tm preface.tm copyright.tm
	jed -script tm2sgml.sl cslang
slang.sgml : slang.tm preface.tm copyright.tm
	jed -script tm2sgml.sl slang
cref.sgml : cref.tm
	jed -script tm2sgml.sl cref
slangfun.sgml : slangfun.tm
	jed -script tm2sgml.sl slangfun
#----- HTML Files -----------------------------------------------------------
cslang.html : cslang.sgml
	$(SGML2HTML) cslang.sgml
slang.html : slang.sgml 
	$(SGML2HTML) slang.sgml
cref.html : cref.sgml
	$(SGML2HTML) cref.sgml
slangfun.html : slangfun.sgml
	$(SGML2HTML) slangfun.sgml
#----- TeX Files ------------------------------------------------------------
slang.tex : slang.sgml
	$(SGML2LATEX) slang.sgml
	jed -script ./fixtex.sl slang.tex
cslang.tex : cslang.sgml
	$(SGML2LATEX) cslang.sgml
	jed -script ./fixtex.sl cslang.tex
#----- PS Files -----------------------------------------------------------
cslang.ps : cslang.tex
	$(LATEX) cslang.tex
	$(LATEX) cslang.tex
	dvips -o cslang.ps cslang.dvi
slang.ps : slang.tex
	$(LATEX) slang.tex
	$(LATEX) slang.tex
	dvips -o slang.ps slang.dvi
#----- Text Files -----------------------------------------------------------
cslang.txt: cslang.sgml
	$(SGML2TXT) cslang.sgml
slang.txt: slang.sgml
	$(SGML2TXT) slang.sgml
#
slangfun.txt : slangfun.tm $(TM2TXT)
	cat rtl/*.tm | $(TM2TXT) > slangfun.txt
cref.txt : cref.tm $(TM2TXT)
	$(TM2TXT) < cref.tm > cref.txt
$(TM2TXT) : tools/tm2txt.c
	cd tools; make SRCDIR=$(SRCDIR)
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc rtl/*.BAK rtl/*~
distclean: clean
	-rm -f *.html $(TXT_FILES) $(TEX_FILES) $(SGML_FILES)
	cd tools; make clean
install-txt-files: $(TXT_FILES)
	-mv $(TXT_FILES) ../text

install: all $(PS_FILES)
	-mkdir $(SUBDIRS)
	-mv $(TXT_FILES) ../text
	-mv *.html ../html
	-mv $(PS_FILES) ../ps
	-mv $(SGML_FILES) ../sgml

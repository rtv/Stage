#
#
# $Id: Makefile,v 1.1.8.2 2003-12-06 22:56:06 gerkey Exp $
#
# generic LaTeX makefile
#

LATEX = latex
BIBTEX = bibtex
DVIPS = dvips
PS2PDF = ps2pdf

# Manual name
NAME = stage

all: dvi ps pdf

dvi: $(NAME).dvi

$(NAME).dvi: $(NAME).tex
	$(LATEX) $< && $(BIBTEX) $(NAME) && $(LATEX) $< && $(LATEX) $<

html:
	rm -rf html_docs
	latex2html -split 4 -toc_depth 3 -html_version 3.2,table,math -show_section_numbers -mkdir -dir html_docs $(NAME).tex

#latex2html -split 0 -nonavigation -html_version 3.2,table,math -show_section_numbers -mkdir -dir html_docs $(NAME).tex

ps: $(NAME).ps

$(NAME).ps: $(NAME).dvi
	$(DVIPS) -f -P pdf -t letter $(NAME).dvi > $@

pdf: $(NAME).pdf

$(NAME).pdf: $(NAME).ps
	$(PS2PDF) $<

clean:
	rm -rf $(NAME).ps $(NAME).dvi $(NAME).log \
	      $(NAME).aux $(NAME).blg $(NAME).toc \
	      $(NAME).lot $(NAME).lof \
	      missfont.log $(NAME).bbl $(NAME).pdf *.bak html_docs 

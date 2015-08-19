#!/bin/bash
#
# the basic LaTeX compile toolchain
clear
printf "Compiling LaTeX file\n"
latex source.tex && bibtex source.aux && latex source.tex && bibtex source.aux && latex source.tex && latex source.tex
printf "\nBuilding .pdf file\n"
dvips -Ppdf -G0 -tletter source.dvi -o source.ps &&
ps2pdf -dCompatibilityLevel=1.4 -dMAxSubsetPct=100 -dSubsetFonts=true -dEmbedAllFonts=true -sPAPERSIZE=letter source.ps out.pdf 
printf "\nClean build files\n"
rm -f *.ps *.log *.aux *.dvi *.bbl *.blg
printf "\nDone compiling\n\n"

exit 0

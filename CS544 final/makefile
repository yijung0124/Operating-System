all:
	latex Writeup.tex 
	latex Writeup.tex
	dvips -R -Poutline -t letter Writeup.dvi -o Writeup.ps
	ps2pdf Writeup.ps

clean:
	rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc assignment1 *.o *.out *.log *.aux *~
C=gcc 
CFLAGS=-Wall
all:
	latex Writeup.tex 
	dvips -R -Poutline -t letter Writeup.dvi -o Writeup.ps
	ps2pdf Writeup.ps
	gcc -pthread -o con2 con2.c mt19937ar.c

clean:
	rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg assignment1 *.o *.out *.log *.aux *~
xsettingsd: xsettingsd.cc xsettingsd.h
	g++ -o xsettingsd xsettingsd.cc

clean:
	rm -f xsettingsd

msh379: msh379.cpp
		g++ -o msh379 msh379.cpp
run:	msh379
		./msh379
tar:
		touch submit.tar.gz
		tar -cvf submit.tar msh379.cpp report.pdf makefile
		gzip submit.tar
clean:
		rm -f msh379
		rm -f submit.tar.gz
		rm -f submit.tar
		rm -rf submit

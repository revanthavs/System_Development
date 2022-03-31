
run: a4w22
	./a4w22

a3w22: clean
	gcc a4w22.c -o a4w22

tar:
	touch submit.tar.gz
	tar -cvf submit.tar a3w22.c Report_3.pdf Makefile
	gzip submit.tar

clean:
	rm -r -f a4w22
	rm -f submit.tar.gz
	rm -f submit.tar
	rm -rf submit

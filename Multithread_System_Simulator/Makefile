
run: a4w22
	./a4w22

a4w22: clean
	gcc -pthread a4w22.c -o a4w22

tar:
	touch atmakuri-a4.tar.gz
	tar -cvf atmakuri-a4.tar a4w22.c Report_4.pdf Makefile
	gzip atmakuri-a4.tar

clean:
	rm -r -f a4w22
	rm -f atmakuri-a4.tar.gz
	rm -f atmakuri-a4.tar
	rm -rf atmakuri-a4

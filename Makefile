
run_test : test.o sre.o
	g++ -o $@ $^
	rm -rf *.o

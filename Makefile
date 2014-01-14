
run_test : test.o sre.o utils.o
	g++ -o $@ $^
	rm -rf *.o

buddy:
	@rm -f gawk-3.1.8/our_alloc.c
	@ln ./buddy.c gawk-3.1.8/our_alloc.c
	@cd gawk-3.1.8/ && $(MAKE) check

linked:
	@rm -f gawk-3.1.8/our_alloc.c
	@ln ./linked.c gawk-3.1.8/our_alloc.c
	@cd gawk-3.1.8/ && $(MAKE) check

test_linked: 
	@rm test.o
	@gcc -g -o ./test.o test.c linked.c
	./test.o

test_buddy: 
	@rm test.o
	@gcc -g -o ./test.o test.c buddy.c
	./test.o
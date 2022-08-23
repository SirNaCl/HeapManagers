buddy:
	@rm -f gawk-3.1.8/our_alloc.c
	@ln ./buddy.c gawk-3.1.8/our_alloc.c
	@cd gawk-3.1.8/ && $(MAKE) clean check

linked:
	@rm -f gawk-3.1.8/our_alloc.c
	@ln ./linked.c gawk-3.1.8/our_alloc.c
	@cd gawk-3.1.8/ && $(MAKE) clean check 

test-linked: 
	@git pull
	@gcc -g -o ./test.out test.c linked.c
	./test.out

test-buddy: 
	@git pull
	@gcc -g -o ./test.out test.c buddy.c
	./test.out

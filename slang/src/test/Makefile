TEST_SCRIPTS = syntax arith array strops bstring pack stdio assoc selfload \
  struct nspace ospath ifeval anytype arrmult
TEST_PGM = sltest
SLANGINC = ..
SLANGLIB = ../$(ARCH)objs

run_tests: $(TEST_PGM)
	@echo ""
	@echo "Running tests:"
	@echo ""
	@for X in $(TEST_SCRIPTS); \
	do \
	  ./$(TEST_PGM) $$X.sl; \
	done
	touch sltest.c

sltest: sltest.c $(SLANGLIB)/libslang.a
	$(CC) $(CLFAGS) sltest.c -o sltest -I$(SLANGINC) -L$(SLANGLIB) -lslang -lm
clean: 
	-/bin/rm -f *~ sltest *.o *.log

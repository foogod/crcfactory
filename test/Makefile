CFLAGS = -Wall -Werror -I..

.PHONY: default check clean $(CHECK_TARGETS)
default: check

# Different ways of generating C files from the test crclist:

TEST_CASES = crctest64 crctest32 crctest32fast crctest16 crctest16_8 crctest8

crctest64.c: tests.crclist crclist2test.py
	python3 crclist2test.py 64 uint64_t uint64_t < $< > $@

crctest32.c: tests.crclist crclist2test.py
	python3 crclist2test.py 32 uint32_t uint32_t < $< > $@

crctest32fast.c: tests.crclist crclist2test.py
	python3 crclist2test.py 32 uint_fast32_t uint32_t < $< > $@

crctest16.c: tests.crclist crclist2test.py
	python3 crclist2test.py 16 uint16_t uint16_t < $< > $@

crctest16_8.c: tests.crclist crclist2test.py
	python3 crclist2test.py 8 uint16_t uint8_t < $< > $@

crctest8.c: tests.crclist crclist2test.py
	python3 crclist2test.py 8 uint8_t uint8_t < $< > $@

# Different ways of compiling the C files into executables:

%-O0: %.c ../crcfactory.h
	gcc $(CFLAGS) -O0 -o $@ $<

%-O3: %.c ../crcfactory.h
	gcc $(CFLAGS) -O3 -o $@ $<

%-Ofast: %.c ../crcfactory.h
	gcc $(CFLAGS) -Ofast -o $@ $<

# Stitching all of the above together:

TEST_CFILES = $(addsuffix .c,$(TEST_CASES))
TEST_PROGS_O0 = $(addsuffix -O0,$(TEST_CASES))
TEST_PROGS_O3 = $(addsuffix -O3,$(TEST_CASES))
TEST_PROGS_Ofast = $(addsuffix -Ofast,$(TEST_CASES))
TEST_PROGS = $(TEST_PROGS_O0) $(TEST_PROGS_O3) $(TEST_PROGS_Ofast)
CHECK_TARGETS = $(addprefix check-,$(TEST_PROGS))

# The main recipe.  Run each compiled test program (by invoking all individual the 'check-<progname>' rules)
check: $(CHECK_TARGETS)

# We need to use this funky syntax because otherwise GNU make doesn't know how
# to use .PHONY with $(CHECK_TARGETS) since they're pattern-based rules.
$(CHECK_TARGETS): check-%: %
	./$<

clean: 
	rm -f $(TEST_CFILES)
	rm -f $(TEST_PROGS)

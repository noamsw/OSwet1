#TODO: replace ID with your own IDS, for example: 123456789_123456789
SUBMITTERS := <student1-ID>_<student2-ID>
COMPILER := g++
COMPILER_FLAGS := --std=c++11 -Wall -g -DNDEBUG
SRCS := Commands.cpp signals.cpp smash.cpp
OBJS=$(subst .cpp,.o,$(SRCS))
HDRS := Commands.h signals.h
TESTS_INPUTS := $(wildcard test_input*.txt)
TESTS_OUTPUTS := $(subst input,output,$(TESTS_INPUTS))
SMASH_BIN := smash

test: $(TESTS_OUTPUTS)
$(TESTS_OUTPUTS): $(SMASH_BIN)
$(TESTS_OUTPUTS): test_output%.txt: test_input%.txt test_expected_output%.txt
	./$(SMASH_BIN) < $(word 1, $^) > $@
	diff $@ $(word 2, $^)
	echo $(word 1, $^) ++PASSED++

#test1 :
#	./smash < cd.in > cd.out
#	diff cd.out cd.exp
#
#test2 :
#	./smash < chprompt.in > chprompt.out
#	diff chprompt.out chprompt.exp
#
#test3 :
#	./smash < jobs.in > jobs.out
#	diff jobs.out jobs.exp
#
#test4 :
#	./smash < kill.in > kill.out
#	diff kill.out kill.exp
#
#test5 :
#	./smash < head.in > head.out
#	diff head.out head.exp
#
#test6 :
#	./smash < quit.in > quit.out
#	diff quit.out quit.exp
#
#test7 :
#	./smash < quit.in > quit.out
#	diff quit.out quit.exp
#
#test8 :
#	./smash < redirect.in > redirect.out
#	diff redirect.out redirect.exp
#
#test9 :
#	./smash < showpid.in > showpid.out
#	diff showpid.out showpid.exp
#
#testall: test1 test2 test3 test4 test5 test6 test7 test8 test9

$(SMASH_BIN): $(OBJS)
	$(COMPILER) $(COMPILER_FLAGS) $^ -o $@

$(OBJS): %.o: %.cpp
	$(COMPILER) $(COMPILER_FLAGS) -c $^

zip: $(SRCS) $(HDRS)
	zip $(SUBMITTERS).zip $^ submitters.txt Makefile

clean:
	rm -rf $(SMASH_BIN) $(OBJS) $(TESTS_OUTPUTS) 
	rm -rf $(SUBMITTERS).zip

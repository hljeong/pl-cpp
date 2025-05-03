include makefile_utils/defaults.mk

CC = g++ -Wall -g -std=c++17

.PHONY: test clean cpp update

test: cpp
	@ ./a.out
	@ echo "all tests passed"

cpp:
	@ $(CC) test.cc nfa.cc

clean:
	@ rm -rf a.out

update: git-submodule-update

setup: git-hook-install

include makefile_utils/git.mk

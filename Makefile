SOURCES = $(wildcard *.c)
CFLAGS = -std=c11 -g -lpthread -lm  -Wall -Wextra -Wuninitialized -Wconversion #-Wpedantic
CC = gcc
LDFLAGS = 
FMTFLAGS = ""
LD = gcc
OBJECTS = $(SOURCES:%.c=%.o)
LIBS = 

EXE = shell

default: all

all: $(EXE)

$(EXE): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(EXE) $(LIBS)


# everything needs common.h

$(SOURCES): common.h debug.h


main.o: main.c list.h tokenizer.h

list.o: list.c list.h
tokenizer.o: tokenizer.c tokenizer.h list.c list.h



clean: 
	-rm -f $(EXE)
	-rm -f $(OBJECTS)


fmt:
	if output=$$(git status --porcelain) && [ ! -z "$$output" ]; then \
		echo -n "Current branch has uncommited changes. type \"yes\" to continue: "; \
		read INPUT; \
		INPUT=$$(echo $$INPUT | tr '[:upper:]' '[:lower:]'); \
	fi
	clang-format -i $$(find src/ -iname '*.hpp' -o -iname '*.cpp' -o -iname '*.hpp' | tr '\n' ' ')
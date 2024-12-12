CC=g++
INCS=
LIBS=
FINAL=-lpthread
CFLAGS=-std=c++11 -Ofast -march=native -pipe
#CFLAGS=-std=c++11 -Wall -pedantic -O0 -g -pipe -DDEBUG -D_DEBUG
BINDIR=bin
OBJDIR=obj
SRCDIR=src

.PHONY: clean
all: make_dirs $(BINDIR)/filin

make_dirs:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

include $(wildcard $(OBJDIR)/*.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@
	$(CC) -MM -MP -MT "$@" $(CFLAGS) $(INCS) $< > $(OBJDIR)/$*.d

$(BINDIR)/filin: $(addprefix $(OBJDIR)/, main.o searcher.o Node.o position.o Book.o EPD.o Evaluate.o IOHandler.o Log.o moveList.o moveSorter.o Pondering.o TranspositionTable.o Utilities.o move.o moveGenerator.o)
	$(CC) $(CFLAGS) $(LIBS) $(INCS) $^ -o $@ $(FINAL)

clean:
	rm -rf $(BINDIR)/*
	rm -rf $(OBJDIR)/*

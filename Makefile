# COMP2521 - Assignment 1

# !!! DO NOT MODIFY THIS FILE !!!

CC = clang
CFLAGS = -Wall -Wvla -Werror -gdwarf-4

########################################################################

.PHONY: asan msan nosan

asan: CFLAGS += -fsanitize=address,leak,undefined
asan: all

msan: CFLAGS += -fsanitize=memory,undefined -fsanitize-memory-track-origins
msan: all

nosan: all

########################################################################

.PHONY: all
all: testCatalogue

testCatalogue: testCatalogue.c Catalogue.c Catalogue.h CatalogueStructs.h course.h
	$(CC) $(CFLAGS) -o testCatalogue testCatalogue.c Catalogue.c

.PHONY: clean
clean:
	rm -f testCatalogue


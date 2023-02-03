
TARGET_DIR := target
SOURCE_DIR := src
CPPFLAGS := -Wall -Werror -std=c++20

SRCS := main.cpp

PROGS := $(TARGET_DIR)/main

.PHONY: help
help:
	@echo Available targets:
	@egrep "^\.PHONY:" Makefile | sed "s/.PHONY: */\t/"

.PHONY: all
all: ${PROGS}

${PROGS} : $(TARGET_DIR)/% : $(TARGET_DIR)/%.o
	g++ $< -o $@

.PHONY: run
run: $(TARGET_DIR)/main
	$<

.PHONY: clean
clean:
	rm -f $(TARGET_DIR)/*
	touch $(TARGET_DIR)/dummy

$(TARGET_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	g++ ${CPPFLAGS} -c $< -o $@


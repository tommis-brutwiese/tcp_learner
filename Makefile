
TARGET_DIR := target
SOURCE_DIR := src
CPPFLAGS := -Wall -Werror -std=c++20

SRCS := main.cpp

TARGET_PROGS := main tcp_client tcp_server

PROGS := $(foreach X, $(TARGET_PROGS), $(TARGET_DIR)/$(X))

.PHONY: help
help:
	@echo Available targets:
	@egrep "^\.PHONY:" Makefile | sed "s/.PHONY: */\t/"

.PHONY: all
all: ${PROGS} doc

${PROGS} : $(TARGET_DIR)/% : $(TARGET_DIR)/%.o
	g++ $< -o $@

.PHONY: run
run: $(PROGS)
	$(TARGET_DIR)/main
	$(TARGET_DIR)/tcp_server &
	sleep 1
	$(TARGET_DIR)/tcp_client &
	sleep 5
	killall tcp_server

.PHONY: clean
clean:
	rm -f $(TARGET_DIR)/*
	touch $(TARGET_DIR)/dummy

$(TARGET_DIR)/README.html: README.md
	pandoc $< > $(TARGET_DIR)/README.html

.PHONY: doc
doc: $(TARGET_DIR)/README.html

$(TARGET_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	g++ ${CPPFLAGS} -c $< -o $@



TARGET_DIR := target
SOURCE_DIR := src
INCLUDE_DIR := include
TEST_DIR := test
CPPFLAGS := -Wall -Werror -std=c++20 -g
CFLAGS := -Wall -Werror -std=c11 -g

HEADERS := epoll_helper.hpp guard.hpp sigterm_helper.hpp tcp_event.h tcp_read_buffer.hpp tcp_read_write.h guard.h

HEADER_FILES := $(foreach X, $(HEADERS), $(INCLUDE_DIR)/$(X))

TARGET_PROGS := tcp_client_a tcp_client_b tcp_server test_tcp_event

PROGS   := $(foreach X, $(TARGET_PROGS), $(TARGET_DIR)/$(X))
PROGS_O := $(foreach X, $(PROGS), $(X).o)

.PHONY: help
help:
	@echo Available targets:
	@egrep "^\.PHONY:" Makefile | sed "s/.PHONY: */\t/"

.PHONY: all
all: build test run

.PHONY: build
build: $(PROGS) doc

$(TARGET_DIR)/%: $(TARGET_DIR)/%.o $(TARGET_DIR)/tcp_read_write.o $(TARGET_DIR)/tcp_event.o $(TARGET_DIR)/tcp_read_buffer.o $(TARGET_DIR)/sigterm_helper.o $(TARGET_DIR)/guard.o
	g++ $^ -o $@

.PHONY: run
run: $(TARGET_DIR)/tcp_server $(TARGET_DIR)/tcp_client_b
	killall tcp_client_b || true
	killall tcp_server   || true

	@printf "\n## Starting tcp_server\n"
	$(TARGET_DIR)/tcp_server &
	sleep 0.5

	@printf "\n## Starting tcp_client_b\n"
	@# can also use tcp_client_a - string read
	$(TARGET_DIR)/tcp_client_b &
	
	sleep 7

	# It should be sufficient to kill the server, but the client does not
	# act correctly upon shutdown.
	@printf "\n## Stopping both processes\n"
	
	killall tcp_client_b
	killall tcp_server

.PHONY: test
test: $(TARGET_DIR)/test_tcp_event
	$<

.PHONY: clean
clean:
	rm -f $(TARGET_DIR)/*
	touch $(TARGET_DIR)/dummy

$(TARGET_DIR)/README.html: README.md
	pandoc $< > $(TARGET_DIR)/README.html

.PHONY: doc
doc: $(TARGET_DIR)/README.html

$(TARGET_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	g++ ${CPPFLAGS} -I $(INCLUDE_DIR) -c $< -o $@

$(TARGET_DIR)/%.o: $(SOURCE_DIR)/%.c
	gcc ${CFLAGS} -I $(INCLUDE_DIR) -c $< -o $@

$(TARGET_DIR)/test_tcp_event.o: $(TEST_DIR)/test_tcp_event.c
	gcc ${CFLAGS} -I $(INCLUDE_DIR) -c $< -o $@
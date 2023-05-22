
TARGET_DIR := target
SOURCE_DIR := src
INCLUDE_DIR := include
TEST_DIR := test
CPPFLAGS := -Wall -Werror -std=c++20 -g
CFLAGS := -Wall -Werror -std=c11 -g

SRCS := main.cpp

TARGET_PROGS := main tcp_client_a tcp_client_b tcp_server test_tcp_laus

PROGS := $(foreach X, $(TARGET_PROGS), $(TARGET_DIR)/$(X))

.PHONY: help
help:
	@echo Available targets:
	@egrep "^\.PHONY:" Makefile | sed "s/.PHONY: */\t/"

.PHONY: build
build: ${PROGS} doc

${PROGS} : $(TARGET_DIR)/% : $(TARGET_DIR)/%.o $(TARGET_DIR)/tcp_laus.o $(TARGET_DIR)/tcp_read_buffer.o
	g++ $^ -o $@

.PHONY: run
run: $(PROGS)
	$(TARGET_DIR)/main
	$(TARGET_DIR)/tcp_server &
	sleep 0.5
	
	# can also use tcp_client_a - string read
	$(TARGET_DIR)/tcp_client_b &
	
	sleep 5

	# It should be sufficient to kill the server, but the client does not
	# act correctly upon shutdown.
	
	killall tcp_client_b
	killall tcp_server

.PHONY: test
test: $(TARGET_DIR)/test_tcp_laus
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

$(TARGET_DIR)/tcp_laus.o: $(SOURCE_DIR)/tcp_laus.c
	gcc ${CFLAGS} -I $(INCLUDE_DIR) -c $< -o $@

$(TARGET_DIR)/test_tcp_laus.o: $(TEST_DIR)/test_tcp_laus.c
	gcc ${CFLAGS} -I $(INCLUDE_DIR) -c $< -o $@
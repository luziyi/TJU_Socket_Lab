SRC_DIR := src
OBJ_DIR := obj
# 所有源文件
SRC := $(wildcard $(SRC_DIR)/*.c)
# 所有目标文件
OBJ := $(OBJ_DIR)/y.tab.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/example.o 
# 所有可执行文件
BIN := example liso_server liso_client login register
# C编译器
CC  := gcc
# C预处理器标志
CPPFLAGS := -Iinclude
# 编译器标志
CFLAGS   := -g -Wall -w
# 依赖文件
# DEPS = parse.h y.tab.h


CGI_DIR := cgi-bin
CGI_SRC := $(wildcard $(CGI_DIR)/*.c)
CGI_TAR := cgi-bin

default: all
all : example liso_server liso_client login register

example: $(OBJ)
	$(CC) $^ -o $@

$(SRC_DIR)/lex.yy.c: $(SRC_DIR)/lexer.l
	flex -o $@ $^

$(SRC_DIR)/y.tab.c: $(SRC_DIR)/parser.y
	yacc -d $^
	mv y.tab.c $@
	mv y.tab.h $(SRC_DIR)/y.tab.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(CGI_DIR)/%.o: $(CGI_SRC)/%.c $(CGI_TAR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

liso_server: $(OBJ_DIR)/liso_server.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/y.tab.o 
	$(CC) -Werror $^ -o $@

liso_client: $(OBJ_DIR)/liso_client.o
	$(CC) -Werror $^ -o $@

login: $(CGI_DIR)/login.o 
	$(CC) -Werror $^ -o $@

register: $(CGI_DIR)/register.o
	$(CC) -Werror $^ -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ) $(BIN) $(SRC_DIR)/lex.yy.c $(SRC_DIR)/y.tab.* 
	$(RM) -r $(OBJ_DIR)

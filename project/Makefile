SRC_DIR := src
OBJ_DIR := obj
# 所有源文件
SRC := $(wildcard $(SRC_DIR)/*.c)
# 所有目标文件
OBJ := $(OBJ_DIR)/y.tab.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/example.o 
# 所有可执行文件
BIN := example echo_server echo_client
# C编译器
CC  := gcc
# C预处理器标志
CPPFLAGS := -Iinclude
# 编译器标志
CFLAGS   := -g -Wall
# 依赖文件
# DEPS = parse.h y.tab.h

default: all
all : example echo_server echo_client

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

echo_server: $(OBJ_DIR)/echo_server.o $(OBJ_DIR)/lex.yy.o $(OBJ_DIR)/parse.o $(OBJ_DIR)/y.tab.o 
	$(CC) -Werror $^ -o $@

echo_client: $(OBJ_DIR)/echo_client.o
	$(CC) -Werror $^ -o $@

$(OBJ_DIR):
	mkdir $@

clean:
	$(RM) $(OBJ) $(BIN) $(SRC_DIR)/lex.yy.c $(SRC_DIR)/y.tab.*
	$(RM) -r $(OBJ_DIR)

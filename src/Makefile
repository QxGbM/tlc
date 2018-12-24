# Tiny Language Compiler (tlc)
# Makefile
# 2016年 木村啓二

#PLATFORM = LINUX
#PLATFORM = MAC
PLATFORM = CYGWIN

TARGET = tlc
SRCS = main.c tl_gram.y tl_lex.l util.c util.h ast.c ast.h parse_action.c parse_action.h symtab.c symtab.h cg.c cg.h
OBJS = main.o tl_gram.o tl_lex.o util.o ast.o parse_action.o symtab.o cg.o
FETMPS = tl_lex.c tl_gram.c tl_gram.h

CFLAGS = -O0 -Wall -g

ifeq ($(PLATFORM), LINUX)
TARGET_FLAG = -DTARGET_LINUX
LFLAGS = -ll -ly
else ifeq ($(PLATFORM), MAC)
TARGET_FLAG = -DTARGET_MAC
LFLAGS = -ll -ly
else ifeq ($(PLATFORM), CYGWIN)
TARGET_FLAG = -DTARGET_CYGWIN
LFLAGS = -lfl -ly
endif
.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	gcc -o $@ $(OBJS) $(LFLAGS)

ast.o: ast.c ast.h util.h
cg.o: cg.c ast.h cg.h symtab.h
main.o: main.c ast.h cg.h
parse_action.o: parse_action.c parse_action.h
symtab.o: symtab.c symtab.h ast.h
util.o: util.c util.h
tl_lex.c: tl_lex.l tl_gram.c
tl_gram.c: tl_gram.y ast.h parse_action.h

.c.o:
	gcc $(CFLAGS)  $(TARGET_FLAG) -c $<

.l.c:
	flex -o $@ $<

.y.c:
	bison -d -o $@ $<

clean:
	-rm -f *~ *.o $(TARGET) $(FETMPS)

# コンパイルなどが記載されていませんが，
# makeのデフォルト動作(暗黙のルール)を使っています．
# makeがどういう動作をするのかは-nオプションで確認できます．
# 例： make -n ターゲット
#
CC = gcc
TEX = platex
CFLAGS = -Wall $(COPTIONS)
LDFLAGS = 
LOADLIBES = 
LDLIBS = -lm -lSDL2 
# Joy-Conライブラリを利用するときは以下のライブラリを追加
LDLIBS = -lm -lSDL2 -ljoyconlib -lhidapi-hidraw 

TARGET = ui3kadai
OBJS = $(TARGET).o ui3system.o ui3window.o
HEADS = ui3system.h

# 課題に取り組むにはこのターゲットで
#
# ターゲット指定しないと，最初のターゲットであるこれが選ばれる
#
debug: CPPFLAGS = 
debug: COPTIONS = -g3
debug: $(TARGET)

# 暗黙のルールでの動作は以下の通り
# cソースからオブジェクト
#   $(CC) $(CFLAGS) $(CPPFLAGS) -c n.c -o n.o
# オブジェクトから実行ファイル
#   $(CC) $(LDFLAGS) n.o $(LOADLIBES) $(LDLIBS) -o n
#
$(TARGET): $(OBJS)
$(OBJS): $(HEADS)

# レポート作成用ターゲット
# 例えば repo.tex があるとき，
#  make repo.pdf でpdfまで生成する
#
# texからdviへは暗黙のルール $(TEX) n.tex が使われる
#
%.pdf:%.dvi
	dvipdfmx $<

# コンパイルをやり直したい時などに，
# 中間ファイルなどを削除するターゲット
#   使用例： make clean
#
.PHONY: clean
clean:
	@rm -f $(TARGET) $(OBJS) core *~

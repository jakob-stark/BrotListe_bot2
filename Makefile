main: main.c
	gcc -std=c11 `pkg-config --cflags telebot sqlite3` -o main main.c `pkg-config --libs telebot sqlite3`

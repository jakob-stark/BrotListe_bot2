main: main.c
	gcc `pkg-config --cflags telebot` -o main main.c `pkg-config --libs telebot`

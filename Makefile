CC=gcc
CFLAGS=-g -Wall -pedantic

ALLEGRO_VERSION=5.2.9.1
MINGW_VERSION=14.1.0
FOLDER=C:
FOLDER_NAME=\allegro-$(ALLEGRO_VERSION)-mingw-$(MINGW_VERSION)
PATH_ALLEGRO=$(FOLDER)$(FOLDER_NAME)
LIB_ALLEGRO=\lib
INCLUDE_ALLEGRO=\include

FLAGS=-I include -I $(PATH_ALLEGRO)$(INCLUDE_ALLEGRO) $(CFLAGS)
LIBS=-L $(PATH_ALLEGRO)$(LIB_ALLEGRO) -lallegro_monolith -lallegro_main -lallegro_primitives -lallegro_font -lallegro_ttf -lallegro_image -lallegro_audio -lallegro_acodec

OBJS=src/main.o src/game.o src/paddle.o src/ball.o src/brick.o
TARGET=breakout.exe

all: $(TARGET)

src/main.o: src/main.c include/game.h include/config.h
	$(CC) $(FLAGS) -c src/main.c -o src/main.o

src/game.o: src/game.c include/game.h include/config.h include/paddle.h include/ball.h include/brick.h
	$(CC) $(FLAGS) -c src/game.c -o src/game.o

src/paddle.o: src/paddle.c include/paddle.h include/config.h
	$(CC) $(FLAGS) -c src/paddle.c -o src/paddle.o

src/ball.o: src/ball.c include/ball.h include/config.h
	$(CC) $(FLAGS) -c src/ball.c -o src/ball.o

src/brick.o: src/brick.c include/brick.h include/config.h
	$(CC) $(FLAGS) -c src/brick.c -o src/brick.o

$(TARGET): $(OBJS)
	@echo ----------------------------------------------------------
	@echo Makefile Breakout
	@echo ----------------------------------------------------------
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	del /q $(OBJS) $(TARGET) 2>nul

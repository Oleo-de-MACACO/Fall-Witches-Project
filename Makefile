# A compilação só irá funcionar para Windows e Linux (Debian/Ubuntu e derivados, etc.)

ifeq ($(OS), Windows_NT)
  CC=gcc
  CFLAGS=-O2 -std=c99 -Wall -Wextra
  OUT=Fall_Witches.exe
  INCLUDE=-I include
  LIBS=-lraylib -lopengl32 -lgdi32 -lwinmm
else
  CC=gcc
  CFLAGS=-O2 -std=c99 -Wall -Wextra
  OUT=Fall_Witches
  INCLUDE=-I include
  LIBS=-lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

SRC = $(wildcard src/*.c)

OBJ = $(SRC: .c = .o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(INCLUDE) $(LIBS)

%.o: %.c
	$(CC) -c S< -o S@ $(INCLUDE)

clean:
	rm -f src/*.o

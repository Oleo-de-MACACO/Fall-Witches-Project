#ifndef CLASSES_H
#define CLASSES_H

#include "raylib.h"

typedef enum {
  GUERREIRO,
  MAGO,
  ARQUEIRO
} Classe;

typedef struct {
  char *nome;
  Texture2D txr;
  Classe classe;
  int nivel;
  int vida;
  int ataque;
  int defesa;
  int mana;
  int posx;
  int posy;
  int width;
  int height;
} Player;

void init_player(Player *p, const char *nome, Classe classe);

#endif

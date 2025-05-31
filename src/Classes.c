#include "../include/Classes.h"
#include <string.h>

void init_player(Player *p, const char *nome, Classe classe){
    strcpy(p->nome, nome);
    p->classe = classe;
    p->nivel = 1;

    switch (classe){
        case GUERREIRO:
            p->vida = 150;
            p->ataque = 20;
            p->defesa = 15;
            p->mana = 0;
            break;
        case MAGO:
            p->vida = 80;
            p->ataque = 30;
            p->defesa = 5;
            p->mana = 100;
            break;
        case ARQUEIRO:
            p->vida = 100;
            p->ataque = 25;
            p->defesa = 10;
            p->mana = 50;
            break;
    }
    p->posx = 0;
    p->posy = 0;
}

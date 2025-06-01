#include "../include/Classes.h"
#include <string.h> // Para strncpy, strcpy, memset

// init_player_inventory permanece como antes
void init_player_inventory(Player *p) {
    if (!p) return;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        strcpy(p->inventory[i].name, "");
        p->inventory[i].quantity = 0;
    }
    p->inventory_item_count = 0;
}

/**
 * @brief Inicializa os slots de equipamento de um jogador como vazios.
 * @param p Ponteiro para o jogador.
 */
void init_player_equipment(Player *p) {
    if (!p) return;
    for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
        strcpy(p->equipped_items[i].name, ""); // Indica slot de equipamento vazio
        p->equipped_items[i].quantity = 0;    // Quantidade não é tão relevante para itens equipados unitários
    }
}

void init_player(Player *p, const char *nome_jogador, Classe classe){
    if (!p) return; // Verificação de segurança

    if (nome_jogador != NULL) {
        strncpy(p->nome, nome_jogador, MAX_PLAYER_NAME_LENGTH - 1);
        p->nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0';
    } else {
        strcpy(p->nome, "Herói Anônimo");
    }

    p->classe = classe;
    p->nivel = 1;
    p->exp = 0; // EXP inicial
    p->current_inventory_tab = 0;

    switch (classe){
        case GUERREIRO:
            p->max_vida = 150; p->ataque = 20; p->defesa = 15; p->max_mana = 20;
            break;
        case MAGO:
            p->max_vida = 80; p->ataque = 10; p->defesa = 5; p->max_mana = 100;
            break;
        case ARQUEIRO:
            p->max_vida = 100; p->ataque = 15; p->defesa = 10; p->max_mana = 50;
            break;
        default:
            p->max_vida = 100; p->ataque = 10; p->defesa = 10; p->max_mana = 10;
            break;
    }
    p->vida = p->max_vida;
    p->mana = p->max_mana;

    p->posx = 0; p->posy = 0;
    p->width = 0; p->height = 0;

    init_player_inventory(p);
    init_player_equipment(p); // Inicializa os equipamentos
}
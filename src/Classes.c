#include "../include/Classes.h"
#include <string.h> // Para strncpy, strcpy, memset
#include "raylib.h" // Para TraceLog

// Inicializa o inventário de um jogador (zera todos os slots)
void init_player_inventory(Player *p) {
    if (!p) {
        // TraceLog(LOG_WARNING, "Tentativa de inicializar inventário de jogador NULO."); // Comentado para reduzir logs
        return;
    }
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        strcpy(p->inventory[i].name, ""); 
        p->inventory[i].quantity = 0;
    }
    p->inventory_item_count = 0;
}

// Inicializa os slots de equipamento de um jogador (todos vazios)
void init_player_equipment(Player *p) {
    if (!p) {
        // TraceLog(LOG_WARNING, "Tentativa de inicializar equipamento de jogador NULO."); // Comentado
        return;
    }
    for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
        strcpy(p->equipped_items[i].name, ""); 
        p->equipped_items[i].quantity = 0;    
    }
}

// Inicializa um jogador com nome, classe e atributos base
// Esta função será chamada pela tela de criação de personagem.
void init_player(Player *p, const char *nome_jogador, Classe classe_jogador){
    if (!p) {
        // TraceLog(LOG_WARNING, "Tentativa de inicializar jogador NULO."); // Comentado
        return;
    }

    // Define nome e classe
    p->classe = classe_jogador;
    if (nome_jogador != NULL && strlen(nome_jogador) > 0) {
        strncpy(p->nome, nome_jogador, MAX_PLAYER_NAME_LENGTH - 1);
        p->nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0'; 
    } else {
        // Nomes padrão caso nenhum nome seja fornecido (útil para NPCs ou placeholders)
        switch(p->classe) {
            case GUERREIRO: strcpy(p->nome, "Valente"); break;
            case MAGO:    strcpy(p->nome, "Feiticeiro");    break;
            case ARQUEIRO:  strcpy(p->nome, "Precisão");  break;
            case BARBARO: strcpy(p->nome, "Fúria"); break;
            case LADINO:  strcpy(p->nome, "Sombra");  break;
            case CLERIGO: strcpy(p->nome, "Devoto"); break;
            default:      strcpy(p->nome, "Aventureiro");   break;
        }
    }

    // Atributos básicos de progressão e dinheiro
    p->nivel = 1;
    p->exp = 0;
    p->moedas = 50; // Moedas iniciais padrão

    // Atributos S.P.E.C.I.A.L. e stats de combate baseados na classe
    // Estes são exemplos e devem ser balanceados.
    switch (p->classe){
        case GUERREIRO:
            p->max_vida = 120; p->ataque = 15; p->defesa = 12; p->max_mana = 30;
            p->forca = 8; p->percepcao = 5; p->resistencia = 7;
            p->carisma = 4; p->inteligencia = 3; p->agilidade = 5; p->sorte = 4;
            break;
        case MAGO:
            p->max_vida = 70; p->ataque = 6; p->defesa = 5; p->max_mana = 120;
            p->forca = 3; p->percepcao = 7; p->resistencia = 4;
            p->carisma = 6; p->inteligencia = 8; p->agilidade = 4; p->sorte = 5;
            break;
        case ARQUEIRO:
            p->max_vida = 90; p->ataque = 12; p->defesa = 7; p->max_mana = 60;
            p->forca = 5; p->percepcao = 8; p->resistencia = 5;
            p->carisma = 5; p->inteligencia = 4; p->agilidade = 7; p->sorte = 6;
            break;
        case BARBARO: // Novo
            p->max_vida = 150; p->ataque = 18; p->defesa = 10; p->max_mana = 20;
            p->forca = 9; p->percepcao = 4; p->resistencia = 8;
            p->carisma = 3; p->inteligencia = 2; p->agilidade = 6; p->sorte = 3;
            break;
        case LADINO: // Novo
            p->max_vida = 80; p->ataque = 10; p->defesa = 6; p->max_mana = 40;
            p->forca = 4; p->percepcao = 7; p->resistencia = 4;
            p->carisma = 6; p->inteligencia = 5; p->agilidade = 8; p->sorte = 7;
            break;
        case CLERIGO: // Novo
            p->max_vida = 90; p->ataque = 9; p->defesa = 9; p->max_mana = 90;
            p->forca = 5; p->percepcao = 6; p->resistencia = 6;
            p->carisma = 7; p->inteligencia = 7; p->agilidade = 3; p->sorte = 6;
            break;
        default: 
            p->max_vida = 100; p->ataque = 10; p->defesa = 10; p->max_mana = 10;
            p->forca = 5; p->percepcao = 5; p->resistencia = 5;
            p->carisma = 5; p->inteligencia = 5; p->agilidade = 5; p->sorte = 5;
            // TraceLog(LOG_WARNING, "Classe de jogador não reconhecida (%d), usando atributos padrão.", p->classe); // Comentado
            break;
    }
    p->vida = p->max_vida; 
    p->mana = p->max_mana; 

    // Posição e dimensões (a tela de criação ou o PrepareNewGame definirão as posições iniciais)
    p->posx = 0; 
    p->posy = 0;
    p->width = 80;  
    p->height = 80; 

    init_player_inventory(p);
    init_player_equipment(p);
    
    p->current_inventory_tab = 0; 
}
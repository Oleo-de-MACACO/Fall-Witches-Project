#include "../include/Classes.h"
#include "../include/Inventory.h"
#include <string.h>
#include <stddef.h>

void init_player_inventory(Player *p) {
    if (!p) return;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        strcpy(p->inventory[i].name, "");
        p->inventory[i].quantity = 0;
    }
    p->inventory_item_count = 0;
}

void init_player_equipment(Player *p) {
    if (!p) return;
    for (int i = 0; i < MAX_EQUIP_SLOTS; i++) {
        strcpy(p->equipped_items[i].name, "");
        p->equipped_items[i].quantity = 0;
    }
}

void init_player(Player *p, const char *nome_jogador, Classe classe_jogador, SpriteType sprite_type){
    if (!p) return;

    int oldPosX = p->posx;
    int oldPosY = p->posy;
    memset(p, 0, sizeof(Player));
    p->posx = oldPosX;
    p->posy = oldPosY;

    p->classe = classe_jogador;
    p->spriteType = sprite_type;

    if (nome_jogador != NULL && strlen(nome_jogador) > 0) {
        strncpy(p->nome, nome_jogador, MAX_PLAYER_NAME_LENGTH - 1);
        p->nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0';
    } else {
        switch(p->classe) {
            case GUERREIRO: strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Demolidor" : "Valente"); break;
            case MAGO:    strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Bruxo" : "Feiticeiro");    break;
            case ARQUEIRO:  strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Caçador Infernal" : "Precisao");  break;
            case BARBARO: strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Bruto Vil" : "Furia"); break;
            case LADINO:  strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Sombra Ardente" : "Espiao");  break;
            case CLERIGO: strcpy(p->nome, (sprite_type == SPRITE_TYPE_DEMONIO) ? "Acolito Negro" : "Devoto"); break;
            default:      strcpy(p->nome, "Aventureiro");   break;
        }
    }

    p->nivel = 1; p->exp = 0; p->moedas = 50;

    switch (p->classe){
        case GUERREIRO: p->max_vida=120; p->ataque=15; p->defesa=12; p->max_mana=30; p->max_stamina=100; p->magic_attack=5; p->magic_defense=8; p->forca=8;p->percepcao=5;p->resistencia=7;p->carisma=4;p->inteligencia=3;p->agilidade=5;p->sorte=4; break;
        case MAGO: p->max_vida=70; p->ataque=6; p->defesa=5; p->max_mana=120; p->max_stamina=50; p->magic_attack=20;p->magic_defense=12; p->forca=3;p->percepcao=7;p->resistencia=4;p->carisma=6;p->inteligencia=8;p->agilidade=4;p->sorte=5; break;
        case ARQUEIRO: p->max_vida=90; p->ataque=12; p->defesa=7;p->max_mana=60;p->max_stamina=80;p->magic_attack=6;p->magic_defense=7;p->forca=5;p->percepcao=8;p->resistencia=5;p->carisma=5;p->inteligencia=4;p->agilidade=7;p->sorte=6;break;
        case BARBARO: p->max_vida=150;p->ataque=18;p->defesa=10;p->max_mana=20;p->max_stamina=120;p->magic_attack=3;p->magic_defense=5;p->forca=9;p->percepcao=4;p->resistencia=8;p->carisma=3;p->inteligencia=2;p->agilidade=6;p->sorte=3;break;
        case LADINO: p->max_vida=80;p->ataque=10;p->defesa=6;p->max_mana=40;p->max_stamina=90;p->magic_attack=7;p->magic_defense=6;p->forca=4;p->percepcao=7;p->resistencia=4;p->carisma=6;p->inteligencia=5;p->agilidade=8;p->sorte=7;break;
        case CLERIGO: p->max_vida=90;p->ataque=9;p->defesa=9;p->max_mana=90;p->max_stamina=70;p->magic_attack=15;p->magic_defense=15;p->forca=5;p->percepcao=6;p->resistencia=6;p->carisma=7;p->inteligencia=7;p->agilidade=3;p->sorte=6;break;
        default: p->max_vida=100;p->ataque=10;p->defesa=10;p->max_mana=10;p->max_stamina=75;p->magic_attack=5;p->magic_defense=5;p->forca=5;p->percepcao=5;p->resistencia=5;p->carisma=5;p->inteligencia=5;p->agilidade=5;p->sorte=5; break;
    }
    p->vida = p->max_vida;
    p->mana = p->max_mana;
    p->stamina = p->max_stamina;

    p->width = 36;
    p->height = 54;

    init_player_inventory(p);
    init_player_equipment(p);
    p->current_inventory_tab = TAB_INVENTORY;

    p->currentAnimFrame = 0;
    p->frameTimer = 0.0f;
    p->frameDuration = 0.15f;
    p->facingDir = DIR_DOWN;
    p->isMoving = false;
}

// --- CORREÇÃO: Implementada a função de level up ---
void LevelUpPlayer(Player *player) {
    if (!player) return;

    // O jogador sobe de nível a cada 100 pontos de experiência
    if (player->exp >= 100) {
        player->nivel++;
        player->exp -= 100; // Reseta o XP, mantendo o excedente

        // Aumenta os atributos com base na classe do jogador
        switch(player->classe) {
            case GUERREIRO:
                player->max_vida += 15;
                player->ataque += 3;
                player->defesa += 2;
                player->forca += 2;
                player->resistencia += 1;
                break;
            case MAGO:
                player->max_vida += 8;
                player->max_mana += 20;
                player->magic_attack += 4;
                player->magic_defense += 2;
                player->inteligencia += 2;
                break;
            case ARQUEIRO:
                player->max_vida += 10;
                player->ataque += 2;
                player->agilidade += 2;
                player->percepcao += 1;
                break;
            case BARBARO:
                player->max_vida += 20;
                player->ataque += 4;
                player->forca += 3;
                player->resistencia += 2;
                break;
            case LADINO:
                player->max_vida += 10;
                player->ataque += 1;
                player->agilidade += 3;
                player->sorte += 1;
                break;
            case CLERIGO:
                player->max_vida += 12;
                player->max_mana += 15;
                player->magic_attack += 2;
                player->magic_defense += 2;
                player->inteligencia += 1;
                player->carisma += 1;
                break;
            default: // Caso genérico
                player->max_vida += 10;
                player->ataque += 1;
                player->defesa += 1;
                break;
        }

        // Restaura a vida e a mana do jogador ao subir de nível
        player->vida = player->max_vida;
        player->mana = player->max_mana;

        // Log para confirmar o level up
        TraceLog(LOG_INFO, "LEVEL UP! %s alcançou o nível %d!", player->nome, player->nivel);
    }
}
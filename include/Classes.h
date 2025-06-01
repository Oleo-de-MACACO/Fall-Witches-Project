#ifndef CLASSES_H
#define CLASSES_H

#include "raylib.h"

#define MAX_PLAYER_NAME_LENGTH 50
#define MAX_INVENTORY_SLOTS 10
#define MAX_ITEM_NAME_LENGTH 50

// Enum para os slots de equipamento
typedef enum {
    EQUIP_SLOT_WEAPON = 0,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_ACCESSORY,
    MAX_EQUIP_SLOTS // Número de slots de equipamento
} EquipmentSlotType;

// Estrutura para um item no inventário (reutilizada para equipamento)
typedef struct {
    char name[MAX_ITEM_NAME_LENGTH]; // Nome do item
    int quantity;                  // Quantidade do item (para inventário, geralmente 1 para equipamento)
} InventoryItem; // Poderia ser renomeado para GameItem se usado em ambos os contextos

typedef enum {
  GUERREIRO,
  MAGO,
  ARQUEIRO
} Classe;

typedef struct {
  char nome[MAX_PLAYER_NAME_LENGTH];
  Texture2D txr; // Textura não será salva/carregada diretamente, apenas dados de jogo
  Classe classe;
  int nivel;
  int exp;                        // Pontos de experiência ATUAIS
  int vida;                       // Vida ATUAL
  int max_vida;                   // Vida MÁXIMA
  int ataque;
  int defesa;
  int mana;                       // Mana ATUAL
  int max_mana;                   // Mana MÁXIMA
  int posx;                       // Posição X (geralmente não salva ou salva com cuidado)
  int posy;                       // Posição Y (geralmente não salva ou salva com cuidado)
  int width;
  int height;
  InventoryItem inventory[MAX_INVENTORY_SLOTS]; // Inventário
  int inventory_item_count;
  InventoryItem equipped_items[MAX_EQUIP_SLOTS]; // Itens equipados
  int current_inventory_tab;
} Player;

void init_player(Player *p, const char *nome, Classe classe);
void init_player_inventory(Player *p); // Já existente
void init_player_equipment(Player *p); // Nova função para inicializar equipamento

#endif // CLASSES_H
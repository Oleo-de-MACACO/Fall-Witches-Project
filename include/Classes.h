#ifndef CLASSES_H
#define CLASSES_H

#include <stdbool.h>
#include "raylib.h"

// Constantes para limitar o tamanho de arrays de char, evitando buffer overflows.
#define MAX_PLAYER_NAME_LENGTH 50
#define MAX_ITEM_NAME_LENGTH 50
#define MAX_PATH_LENGTH 128
#define MAX_CHAR_NAME_LENGTH 50
#define MAX_CHAR_SPRITE_FOLDER_LENGTH 128
#define MAX_INVENTORY_SLOTS 10


// Enum para os diferentes tipos de sprites de personagens
typedef enum {
    SPRITE_TYPE_HUMANO,
    SPRITE_TYPE_DEMONIO,
    NUM_SPRITE_TYPES // Contador para o número de tipos de sprite
} SpriteType;

// Enum para a direção que o personagem está virado
typedef enum {
    DIR_DOWN = 0,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP
} CharacterDirection;

// Constantes para o número de frames em cada animação de caminhada
#define NUM_WALK_UP_FRAMES 3
#define NUM_WALK_DOWN_FRAMES 3
#define NUM_WALK_LEFT_FRAMES 2
#define NUM_WALK_RIGHT_FRAMES 2

// Enum para os slots de equipamento do jogador
typedef enum {
    EQUIP_SLOT_WEAPON = 0,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_ACCESSORY,
    MAX_EQUIP_SLOTS
} EquipmentSlotType;

// Estrutura para um item no inventário
typedef struct {
    char name[MAX_ITEM_NAME_LENGTH];
    int quantity;
} InventoryItem;

// Enum para as classes de personagem
typedef enum {
    GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO, CLASSE_COUNT
} Classe;

/**
 * @brief Estrutura principal que contém todos os dados de um jogador.
 */
typedef struct Player {
  // --- Identidade e Atributos Base ---
  char nome[MAX_PLAYER_NAME_LENGTH];
  Classe classe;
  SpriteType spriteType;

  // --- Stats de Progressão ---
  int nivel; int exp;
  int vida; int max_vida;
  int mana; int max_mana;
  int stamina; int max_stamina;
  int ataque; int defesa;
  int magic_attack; int magic_defense;
  int forca; int percepcao; int resistencia;
  int carisma; int inteligencia; int agilidade; int sorte;
  int moedas;

  // --- Posição e Dimensões no Mundo ---
  int posx; int posy;
  int width; int height;

  // --- Inventário e Equipamento ---
  InventoryItem inventory[MAX_INVENTORY_SLOTS];
  int inventory_item_count;
  InventoryItem equipped_items[MAX_EQUIP_SLOTS];
  int current_inventory_tab;

  // --- Recursos de Animação ---
  Texture2D walkUpFrames[NUM_WALK_UP_FRAMES];
  Texture2D walkDownFrames[NUM_WALK_DOWN_FRAMES];
  Texture2D walkLeftFrames[NUM_WALK_LEFT_FRAMES];
  Texture2D walkRightFrames[NUM_WALK_RIGHT_FRAMES];

  // --- Estado da Animação ---
  int currentAnimFrame;
  float frameTimer;
  float frameDuration;
  CharacterDirection facingDir;
  bool isMoving;
} Player;

// --- Protótipos de Funções ---
void init_player(Player *p, const char *nome_jogador, Classe classe_jogador, SpriteType sprite_type);
void init_player_inventory(Player *p);
void init_player_equipment(Player *p);

// --- CORREÇÃO: Adicionado o protótipo da função de level up ---
void LevelUpPlayer(Player *player);

#endif // CLASSES_H
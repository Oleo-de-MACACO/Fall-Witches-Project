#ifndef CLASSES_H
#define CLASSES_H

#include "raylib.h"

#define MAX_PLAYER_NAME_LENGTH 50
#define MAX_INVENTORY_SLOTS 10
#define MAX_ITEM_NAME_LENGTH 50

typedef enum {
    SPRITE_TYPE_HUMANO,
    SPRITE_TYPE_DEMONIO,
    NUM_SPRITE_TYPES
} SpriteType;

typedef enum {
    DIR_DOWN = 0, 
    DIR_LEFT,
    DIR_RIGHT,
    DIR_UP
} CharacterDirection;

#define NUM_WALK_UP_FRAMES 3
#define NUM_WALK_DOWN_FRAMES 3
#define NUM_WALK_LEFT_FRAMES 2
#define NUM_WALK_RIGHT_FRAMES 2

typedef enum { EQUIP_SLOT_WEAPON = 0, EQUIP_SLOT_ARMOR, EQUIP_SLOT_ACCESSORY, MAX_EQUIP_SLOTS } EquipmentSlotType;
typedef struct { char name[MAX_ITEM_NAME_LENGTH]; int quantity; } InventoryItem;
typedef enum { GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO, CLASSE_COUNT } Classe; // Added CLASSE_COUNT

typedef struct {
  char nome[MAX_PLAYER_NAME_LENGTH]; 
  Classe classe;  
  SpriteType spriteType; 

  int nivel; int exp;                           
  int vida; int max_vida;                      
  int mana; int max_mana;                      
  int stamina; int max_stamina;                  
  int ataque; int defesa;                        
  int magic_attack; int magic_defense;                 
  int forca; int percepcao; int resistencia;
  int carisma; int inteligencia; int agilidade; int sorte;
  int moedas;                        
  int posx; int posy;                          
  int width; int height;                        
  InventoryItem inventory[MAX_INVENTORY_SLOTS]; 
  int inventory_item_count;                     
  InventoryItem equipped_items[MAX_EQUIP_SLOTS]; 
  int current_inventory_tab;         

  Texture2D walkUpFrames[NUM_WALK_UP_FRAMES];
  Texture2D walkDownFrames[NUM_WALK_DOWN_FRAMES];
  Texture2D walkLeftFrames[NUM_WALK_LEFT_FRAMES];
  Texture2D walkRightFrames[NUM_WALK_RIGHT_FRAMES];
  
  int currentAnimFrame;        
  float frameTimer;            
  float frameDuration;         
  CharacterDirection facingDir; 
  bool isMoving;                
} Player;

void init_player(Player *p, const char *nome_jogador, Classe classe_jogador, SpriteType sprite_type);
void init_player_inventory(Player *p);
void init_player_equipment(Player *p);

#endif // CLASSES_H
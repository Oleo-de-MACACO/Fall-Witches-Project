#ifndef CLASSES_H
#define CLASSES_H

#include "raylib.h"

#define MAX_PLAYER_NAME_LENGTH 50  // Tamanho máximo para o nome do jogador
#define MAX_INVENTORY_SLOTS 10     // Máximo de slots no inventário
#define MAX_ITEM_NAME_LENGTH 50    // Tamanho máximo para nome de item

// Enum para os slots de equipamento
typedef enum {
    EQUIP_SLOT_WEAPON = 0,    // Slot da Arma
    EQUIP_SLOT_ARMOR,         // Slot da Armadura
    EQUIP_SLOT_ACCESSORY,     // Slot do Acessório
    MAX_EQUIP_SLOTS           // Número de slots de equipamento
} EquipmentSlotType;

// Estrutura para um item no inventário (reutilizada para equipamento)
typedef struct {
    char name[MAX_ITEM_NAME_LENGTH]; // Nome do item
    int quantity;                    // Quantidade do item
} InventoryItem;

// Enum para as classes dos jogadores
typedef enum {
  GUERREIRO,    // Guerreiro
  MAGO,         // Mago
  ARQUEIRO,     // Arqueiro
  BARBARO,      // Bárbaro (NOVO)
  LADINO,       // Ladino (NOVO)
  CLERIGO       // Clérigo (NOVO)
} Classe;

// Estrutura principal do Jogador
typedef struct {
  char nome[MAX_PLAYER_NAME_LENGTH]; // Nome do jogador
  Texture2D txr;                     // Textura do jogador (não salva diretamente no arquivo de save)
  Classe classe;                     // Classe do jogador

  int nivel;                         // Nível atual
  int exp;                           // Pontos de experiência atuais
  
  int vida;                          // Vida atual
  int max_vida;                      // Vida máxima
  int mana;                          // Mana atual
  int max_mana;                      // Mana máxima
  
  int ataque;                        // Ataque base (pode ser influenciado por atributos)
  int defesa;                        // Defesa base (pode ser influenciada por atributos)
  
  // Atributos S.P.E.C.I.A.L. (Força, Percepção, Resistência, Carisma, Inteligência, Agilidade, Sorte)
  int forca;                         // Força: Afeta dano físico, capacidade de carga
  int percepcao;                     // Percepção: Afeta precisão, chance de encontrar itens, iniciativa
  int resistencia;                   // Resistência: Afeta HP, resistência a status negativos
  int carisma;                       // Carisma: Afeta preços em lojas, persuasão, liderança
  int inteligencia;                  // Inteligência: Afeta MP, eficácia de magias, pontos de EXP ganhos
  int agilidade;                     // Agilidade: Afeta esquiva, ordem de turno, velocidade de ataque
  int sorte;                         // Sorte: Afeta chances críticas, drops raros, eventos aleatórios

  int moedas;                        // Quantidade de moedas do jogador

  int posx;                          // Posição X na tela
  int posy;                          // Posição Y na tela
  int width;                         // Largura (para colisão/desenho)
  int height;                        // Altura (para colisão/desenho)
  
  InventoryItem inventory[MAX_INVENTORY_SLOTS]; // Inventário de itens
  int inventory_item_count;                     // Contagem de tipos de itens no inventário
  InventoryItem equipped_items[MAX_EQUIP_SLOTS]; // Itens equipados
  
  int current_inventory_tab;         // Aba atual no inventário/painel do jogador
} Player;

// Protótipos de função
void init_player(Player *p, const char *nome_jogador, Classe classe_jogador); // Inicializa um jogador
void init_player_inventory(Player *p);                                       // Inicializa o inventário do jogador
void init_player_equipment(Player *p);                                       // Inicializa os slots de equipamento

#endif // CLASSES_H
#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "Classes.h"
#include "CharacterManager.h"

// Estrutura interna para manter o estado dos combatentes durante a batalha
// Movida para o .h para que a UI possa acessá-la.
typedef struct {
    char name[MAX_CHAR_NAME_LENGTH];
    int hp;
    int max_hp;
    int mana;
    int max_mana;
    int attack;
    int defense;
    int magic_attack;
    int magic_defense;
    Classe class_id;
    Player* original_player_ref; // Ponteiro para o jogador original
} BattleParticipant;

typedef enum {
    BATTLE_STATE_STARTING,
    BATTLE_STATE_PLAYER_TURN,
    BATTLE_STATE_ENEMY_TURN_ACTION, // Fase onde a IA do inimigo age
    BATTLE_STATE_TURN_TRANSITION,   // Pequena pausa entre os turnos
    BATTLE_STATE_WIN,
    BATTLE_STATE_LOSE,
    BATTLE_STATE_ENDED,
} BattlePhase;

void BattleSystem_Start(Player* players, int numPlayers, MapCharacter* enemy);
void BattleSystem_Update(void);
bool BattleSystem_IsActive(void);
void BattleSystem_End(void);
BattlePhase BattleSystem_GetPhase(void);

// --- Novas Funções para a UI ---
int BattleSystem_GetPlayerCount(void);
const BattleParticipant* BattleSystem_GetPlayerParticipant(int index);
const BattleParticipant* BattleSystem_GetEnemyParticipant(void);
int BattleSystem_GetCurrentTurnPlayerIndex(void);
const char* BattleSystem_GetLastMessage(void);

void BattleSystem_SetPlayerAction(int playerIndex, int moveIndex);

#endif // BATTLE_SYSTEM_H

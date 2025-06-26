#ifndef BATTLE_SYSTEM_H
#define BATTLE_SYSTEM_H

#include "Classes.h"
#include "CharacterManager.h"

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
    Player* original_player_ref;
} BattleParticipant;

// --- CORREÇÃO DA MÁQUINA DE ESTADOS ---
// Removidos estados redundantes para uma lógica de turno mais clara.
typedef enum {
    BATTLE_STATE_STARTING,      // Início da batalha
    BATTLE_STATE_PLAYER_TURN,   // Aguardando input do jogador
    BATTLE_STATE_ENEMY_TURN,    // IA do inimigo age
    BATTLE_STATE_WIN,           // Jogador venceu
    BATTLE_STATE_LOSE,          // Jogador perdeu
    BATTLE_STATE_ENDED,         // Batalha encerrada (fuga, etc.)
} BattlePhase;

void BattleSystem_Start(Player* players, int numPlayers, MapCharacter* enemy);
void BattleSystem_Update(void);
bool BattleSystem_IsActive(void);
void BattleSystem_End(void);
BattlePhase BattleSystem_GetPhase(void);

int BattleSystem_GetPlayerCount(void);
const BattleParticipant* BattleSystem_GetPlayerParticipant(int index);
const BattleParticipant* BattleSystem_GetEnemyParticipant(void);
int BattleSystem_GetCurrentTurnPlayerIndex(void);
const char* BattleSystem_GetLastMessage(void);

void BattleSystem_SetPlayerAction(int playerIndex, int moveIndex);
bool BattleSystem_AttemptEscape(void);

#endif // BATTLE_SYSTEM_H
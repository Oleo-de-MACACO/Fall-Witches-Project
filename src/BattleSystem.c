#include "../include/BattleSystem.h"
#include "../include/ClassSettings.h"
#include "../include/GameProgress.h" // Incluído para funções de progresso
#include "../include/EnemyLoader.h"
#include "../include/Game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Estado Interno ---
static bool s_isBattleActive = false;
static BattlePhase s_currentPhase;

static BattleParticipant s_playerParticipants[MAX_PLAYERS_SUPPORTED];
static int s_playerParticipantCount = 0;
static BattleParticipant s_enemyParticipant;

static int s_currentPlayerTurn = 0;
static float s_turnEndTimer = 0.0f;
#define TURN_TRANSITION_DELAY 1.0f

static char s_battleMessage[256];

// --- Funções Auxiliares ---
static int RollD20(void) { return (rand() % 20) + 1; }

static void SetBattleMessage(const char* message) {
    strncpy(s_battleMessage, message, sizeof(s_battleMessage) - 1);
    s_battleMessage[sizeof(s_battleMessage) - 1] = '\0';
}

static void SetupPlayerParticipant(int index, Player* source) {
    BattleParticipant* p = &s_playerParticipants[index];
    p->original_player_ref = source;
    strncpy(p->name, source->nome, MAX_CHAR_NAME_LENGTH - 1);
    p->hp = source->vida;           p->max_hp = source->max_vida;
    p->mana = source->mana;         p->max_mana = source->max_mana;
    p->attack = source->ataque;     p->defense = source->defesa;
    p->magic_attack = source->magic_attack; p->magic_defense = source->magic_defense;
    p->class_id = source->classe;
}

static void SetupEnemyParticipant(MapCharacter* source) {
    BattleParticipant* e = &s_enemyParticipant;
    e->original_player_ref = NULL;
    strncpy(e->name, source->name, MAX_CHAR_NAME_LENGTH - 1);
    const EnemyData* data = EnemyLoader_GetByName(source->name);
    if (data) {
        e->hp = data->hp; e->max_hp = data->hp;
        e->mana = data->mp; e->max_mana = data->mp;
        e->attack = data->attack; e->defense = data->defense;
        e->magic_attack = data->magic_attack; e->magic_defense = data->magic_defense;
        e->class_id = data->specialty;
    } else {
        TraceLog(LOG_WARNING, "Inimigo '%s' não encontrado em Enemies.txt. Usando stats padrão.", source->name);
        e->hp = 50; e->max_hp = 50; e->mana = 10; e->max_mana = 10;
        e->attack = 10; e->defense = 5; e->magic_attack = 5; e->magic_defense = 5;
        e->class_id = CLASSE_COUNT;
    }
}

static void ApplyBattleResults(void) {
    for (int i = 0; i < s_playerParticipantCount; i++) {
        Player* p_orig = s_playerParticipants[i].original_player_ref;
        if(p_orig) {
            p_orig->vida = s_playerParticipants[i].hp > 0 ? s_playerParticipants[i].hp : 1;
            p_orig->mana = s_playerParticipants[i].mana;
        }
    }
}

static void ProcessPlayerAction(int playerIndex, int moveIndex) {
    BattleParticipant* attacker = &s_playerParticipants[playerIndex];
    BattleParticipant* target = &s_enemyParticipant;
    const ClassMoveset* moveset = ClassSettings_GetMovesForClass(attacker->class_id);

    if (!moveset || moveIndex < 0 || moveIndex >= moveset->move_count) return;
    const Move* move = &moveset->moves[moveIndex];

    if (attacker->mana < move->mana_cost) {
        SetBattleMessage(TextFormat("%s não tem mana suficiente!", attacker->name));
        return;
    }
    attacker->mana -= move->mana_cost;

    int d20_roll = RollD20();
    if (d20_roll == 1) {
        SetBattleMessage(TextFormat("%s tentou usar %s, mas falhou!", attacker->name, move->name));
        return;
    }

    int base_damage = 0;
    if (move->type == MOVE_TYPE_PHYSICAL) base_damage = (int)((float)attacker->attack * move->power_multiplier) - target->defense;
    else if (move->type == MOVE_TYPE_MAGICAL) base_damage = (int)((float)attacker->magic_attack * move->power_multiplier) - target->magic_defense;

    float damage_mod = 1.0f + ((float)d20_roll - 10.5f) / 20.0f;
    if(d20_roll == 20) { damage_mod = 2.0f; }

    int final_damage = (int)((float)base_damage * damage_mod);
    if (final_damage < 1) final_damage = 1;

    if (d20_roll == 20) SetBattleMessage(TextFormat("ACERTO CRÍTICO! %s usa %s!", attacker->name, move->name));
    else SetBattleMessage(TextFormat("%s usa %s.", attacker->name, move->name));

    target->hp -= final_damage;
}

static void ProcessEnemyAction() {
    BattleParticipant* attacker = &s_enemyParticipant;
    BattleParticipant* target = NULL;

    int alive_player_indices[MAX_PLAYERS_SUPPORTED];
    int alive_count = 0;
    for(int i = 0; i < s_playerParticipantCount; i++){
        if(s_playerParticipants[i].hp > 0){
            alive_player_indices[alive_count++] = i;
        }
    }
    if(alive_count > 0){
        target = &s_playerParticipants[alive_player_indices[GetRandomValue(0, alive_count-1)]];
    } else { return; }

    int d20_roll = RollD20();
    if (d20_roll <= 3) {
        SetBattleMessage(TextFormat("O %s ataca %s, mas erra!", attacker->name, target->name));
        return;
    }
    int damage = attacker->attack - target->defense;
    if (damage < 1) damage = 1;
    target->hp -= damage;
    SetBattleMessage(TextFormat("O %s ataca %s, causando %d de dano.", attacker->name, target->name, damage));
}

void BattleSystem_Start(Player* players, int numPlayers, MapCharacter* enemy) {
    s_playerParticipantCount = numPlayers;
    for (int i = 0; i < numPlayers; i++) { SetupPlayerParticipant(i, &players[i]); }
    SetupEnemyParticipant(enemy);

    s_isBattleActive = true;
    s_currentPhase = BATTLE_STATE_STARTING;
    s_currentPlayerTurn = 0;
    s_turnEndTimer = 0.0f;
    SetBattleMessage(TextFormat("Um %s selvagem aparece!", enemy->name));
}

void BattleSystem_Update(void) {
    if (!s_isBattleActive) return;

    if (s_currentPhase != BATTLE_STATE_WIN && s_currentPhase != BATTLE_STATE_LOSE) {
        if (s_enemyParticipant.hp <= 0) {
            s_currentPhase = BATTLE_STATE_WIN;
            SetBattleMessage("VITÓRIA! Você ganhou 50 EXP e 20 moedas.");
            return;
        }
        bool allPlayersKO = true;
        for (int i = 0; i < s_playerParticipantCount; i++) { if (s_playerParticipants[i].hp > 0) allPlayersKO = false; }
        if (allPlayersKO) {
            s_currentPhase = BATTLE_STATE_LOSE;
            SetBattleMessage("Derrota... Você perdeu 10 moedas.");
            return;
        }
    }

    switch (s_currentPhase) {
        case BATTLE_STATE_STARTING:
            s_turnEndTimer += GetFrameTime();
            if (s_turnEndTimer > TURN_TRANSITION_DELAY) {
                s_turnEndTimer = 0.0f;
                s_currentPhase = BATTLE_STATE_PLAYER_TURN;
                SetBattleMessage(TextFormat("Turno de %s.", s_playerParticipants[s_currentPlayerTurn].name));
            }
            break;
        case BATTLE_STATE_PLAYER_TURN:
            if(s_playerParticipants[s_currentPlayerTurn].hp <= 0){
                s_currentPlayerTurn++;
                s_currentPhase = BATTLE_STATE_TURN_TRANSITION;
            }
            break;
        case BATTLE_STATE_TURN_TRANSITION:
            s_turnEndTimer += GetFrameTime();
            if(s_turnEndTimer > TURN_TRANSITION_DELAY){
                s_turnEndTimer = 0.0f;
                if(s_currentPlayerTurn >= s_playerParticipantCount){
                    s_currentPlayerTurn = 0;
                    s_currentPhase = BATTLE_STATE_PLAYER_TURN;
                    SetBattleMessage(TextFormat("Turno de %s.", s_playerParticipants[s_currentPlayerTurn].name));
                } else { s_currentPhase = BATTLE_STATE_ENEMY_TURN_ACTION; }
            }
            break;
        case BATTLE_STATE_ENEMY_TURN_ACTION:
            ProcessEnemyAction();
            s_turnEndTimer = 0.0f;
            s_currentPhase = BATTLE_STATE_TURN_TRANSITION;
            break;
        case BATTLE_STATE_WIN: case BATTLE_STATE_LOSE: break;
        case BATTLE_STATE_ENDED: s_isBattleActive = false; break;
    }
}

void BattleSystem_End(void) {
    // COMENTADO: Funções não declaradas que causam erro de compilação, vai ser uma implementação futura, não agora nessa versão
    /*
    if(s_currentPhase == BATTLE_STATE_WIN) {
        Progress_AddExp(50);
        Progress_AddCoins(20);
    } else if (s_currentPhase == BATTLE_STATE_LOSE) {
        Progress_AddCoins(-10);
    }
    */
    ApplyBattleResults();
    s_isBattleActive = false;
    s_currentPhase = BATTLE_STATE_ENDED;
}

void BattleSystem_SetPlayerAction(int playerIndex, int moveIndex) {
    if (s_currentPhase == BATTLE_STATE_PLAYER_TURN && playerIndex == s_currentPlayerTurn) {
        ProcessPlayerAction(playerIndex, moveIndex);
        s_currentPlayerTurn++;
        s_turnEndTimer = 0.0f;
        s_currentPhase = BATTLE_STATE_TURN_TRANSITION;
    }
}

bool BattleSystem_IsActive(void) { return s_isBattleActive; }
BattlePhase BattleSystem_GetPhase(void) { return s_currentPhase; }
int BattleSystem_GetPlayerCount(void) { return s_playerParticipantCount; }
const BattleParticipant* BattleSystem_GetPlayerParticipant(int index) { if (index < 0 || index >= s_playerParticipantCount) return NULL; return &s_playerParticipants[index]; }
const BattleParticipant* BattleSystem_GetEnemyParticipant(void) { return &s_enemyParticipant; }
int BattleSystem_GetCurrentTurnPlayerIndex(void) { return s_currentPlayerTurn; }
const char* BattleSystem_GetLastMessage(void) { return s_battleMessage; }
#include "../include/ClassAttacks.h"
#include "../include/BattleSystem.h"
#include "../include/EnemyLoader.h"
#include "../include/Game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static bool s_isBattleActive = false;
static BattlePhase s_currentPhase;

static BattleParticipant s_playerParticipants[MAX_PLAYERS_SUPPORTED];
static int s_playerParticipantCount = 0;
static BattleParticipant s_enemyParticipant;

static int s_currentPlayerTurn = 0;
static float s_actionTimer = 0.0f;
#define ACTION_DELAY 1.0f

static char s_battleMessage[256];

static int RollD20(void) { return (rand() % 20) + 1; }

static void SetBattleMessage(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(s_battleMessage, sizeof(s_battleMessage), format, args);
    va_end(args);
    s_battleMessage[sizeof(s_battleMessage) - 1] = '\0';
}

static void SetupPlayerParticipant(int index, Player* source) {
    BattleParticipant* p = &s_playerParticipants[index];
    p->original_player_ref = source;
    strncpy(p->name, source->nome, MAX_CHAR_NAME_LENGTH - 1);
    p->name[MAX_CHAR_NAME_LENGTH - 1] = '\0';
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
    e->name[MAX_CHAR_NAME_LENGTH - 1] = '\0';
    const EnemyData* data = EnemyLoader_GetByName(source->name);
    if (data) {
        e->hp = data->hp; e->max_hp = data->hp;
        e->mana = data->mp; e->max_mana = data->mp;
        e->attack = data->attack; e->defense = data->defense;
        e->magic_attack = data->magic_attack; e->magic_defense = data->magic_defense;
        e->class_id = data->specialty;
    } else {
        TraceLog(LOG_WARNING, "Inimigo '%s' não encontrado. Usando stats padrão.", source->name);
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

static void ProcessPlayerAction(int playerIndex, int attackIndex) {
    BattleParticipant* attacker = &s_playerParticipants[playerIndex];
    BattleParticipant* target = &s_enemyParticipant;
    const ClassAttackSet* attackSet = ClassAttacks_GetAttackSetForClass(attacker->class_id);

    if (!attackSet || attackIndex < 0 || attackIndex >= attackSet->attack_count) {
        SetBattleMessage("Ataque inválido!");
        return;
    }
    const Attack* attack = &attackSet->attacks[attackIndex];

    if (attacker->mana < attack->mana_cost) {
        SetBattleMessage("%s não tem mana suficiente!", attacker->name);
        return;
    }
    attacker->mana -= attack->mana_cost;

    int d20_roll = RollD20();
    if (d20_roll == 1) {
        SetBattleMessage("%s tentou usar %s, mas falhou!", attacker->name, attack->name);
        return;
    }

    int base_damage = 0;
    if (attack->type == ATTACK_TYPE_PHYSICAL) base_damage = (int)((float)attacker->attack * attack->power_multiplier) - target->defense;
    else if (attack->type == ATTACK_TYPE_MAGICAL) base_damage = (int)((float)attacker->magic_attack * attack->power_multiplier) - target->magic_defense;

    float damage_mod = 1.0f + ((float)d20_roll - 10.5f) / 20.0f;
    if(d20_roll == 20) { damage_mod = 2.0f; }

    int final_damage = (int)((float)base_damage * damage_mod);
    if (final_damage < 1) final_damage = 1;

    target->hp -= final_damage;
    if (d20_roll == 20) SetBattleMessage("CRÍTICO! %s usou %s e causou %d de dano!", attacker->name, attack->name, final_damage);
    else SetBattleMessage("%s usou %s e causou %d de dano.", attacker->name, attack->name, final_damage);
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
    if(alive_count == 0) return;
    target = &s_playerParticipants[alive_player_indices[GetRandomValue(0, alive_count-1)]];

    int d20_roll = RollD20();
    if (d20_roll <= 3) {
        SetBattleMessage("O %s atacou %s, mas errou!", attacker->name, target->name);
        return;
    }
    int damage = attacker->attack - target->defense;
    if (damage < 1) damage = 1;
    target->hp -= damage;
    SetBattleMessage("O %s atacou %s, causando %d de dano.", attacker->name, target->name, damage);
}

void BattleSystem_Start(Player* players, int numPlayers, MapCharacter* enemy) {
    s_playerParticipantCount = numPlayers;
    for (int i = 0; i < numPlayers; i++) { SetupPlayerParticipant(i, &players[i]); }
    SetupEnemyParticipant(enemy);
    s_isBattleActive = true;
    s_currentPhase = BATTLE_STATE_STARTING;
    s_currentPlayerTurn = 0;
    s_actionTimer = 0.0f;
    SetBattleMessage("Um %s selvagem aparece!", enemy->name);
}

void BattleSystem_Update(void) {
    if (!s_isBattleActive) return;

    if (s_currentPhase != BATTLE_STATE_WIN && s_currentPhase != BATTLE_STATE_LOSE) {
        if (s_enemyParticipant.hp <= 0) {
            SetBattleMessage("VITÓRIA! Você ganhou 50 EXP e 20 moedas.");
            s_currentPhase = BATTLE_STATE_WIN;
            return;
        }
        bool allPlayersKO = true;
        for (int i = 0; i < s_playerParticipantCount; i++) { if (s_playerParticipants[i].hp > 0) allPlayersKO = false; }
        if (allPlayersKO) {
            SetBattleMessage("Derrota... Você perdeu 10 moedas.");
            s_currentPhase = BATTLE_STATE_LOSE;
            return;
        }
    }

    s_actionTimer += GetFrameTime();
    if (s_actionTimer < ACTION_DELAY) return;
    s_actionTimer = 0.0f;

    switch (s_currentPhase) {
        case BATTLE_STATE_STARTING:
            s_currentPhase = BATTLE_STATE_PLAYER_TURN;
            while(s_playerParticipants[s_currentPlayerTurn].hp <= 0) {
                s_currentPlayerTurn = (s_currentPlayerTurn + 1) % s_playerParticipantCount;
            }
            SetBattleMessage("Turno de %s.", s_playerParticipants[s_currentPlayerTurn].name);
            break;

        case BATTLE_STATE_PLAYER_TURN:
            break;

        case BATTLE_STATE_ENEMY_TURN:
            ProcessEnemyAction();
            s_currentPlayerTurn = 0;
             while(s_playerParticipants[s_currentPlayerTurn].hp <= 0) {
                s_currentPlayerTurn = (s_currentPlayerTurn + 1) % s_playerParticipantCount;
            }
            s_currentPhase = BATTLE_STATE_PLAYER_TURN;
            SetBattleMessage("Turno de %s.", s_playerParticipants[s_currentPlayerTurn].name);
            break;

        case BATTLE_STATE_WIN:
        case BATTLE_STATE_LOSE:
        case BATTLE_STATE_ENDED:
            break;
    }
}

void BattleSystem_SetPlayerAction(int playerIndex, int attackIndex) {
    if (s_currentPhase != BATTLE_STATE_PLAYER_TURN || playerIndex != s_currentPlayerTurn) return;

    ProcessPlayerAction(playerIndex, attackIndex);
    s_actionTimer = 0.0f;

    s_currentPlayerTurn++;
    if (s_currentPlayerTurn >= s_playerParticipantCount) {
        s_currentPhase = BATTLE_STATE_ENEMY_TURN;
        SetBattleMessage("Turno do Inimigo.");
    } else {
        while(s_playerParticipants[s_currentPlayerTurn].hp <= 0) {
            s_currentPlayerTurn++;
            if (s_currentPlayerTurn >= s_playerParticipantCount) {
                s_currentPhase = BATTLE_STATE_ENEMY_TURN;
                SetBattleMessage("Turno do Inimigo.");
                return;
            }
        }
        SetBattleMessage("Turno de %s.", s_playerParticipants[s_currentPlayerTurn].name);
    }
}


bool BattleSystem_AttemptEscape(void) {
    if (s_currentPhase != BATTLE_STATE_PLAYER_TURN) return false;

    SetBattleMessage("%s tenta fugir...", s_playerParticipants[s_currentPlayerTurn].name);
    s_actionTimer = 0.0f;

    if (RollD20() > 10) {
        s_currentPhase = BATTLE_STATE_ENDED;
        SetBattleMessage("Fuga bem-sucedida!");
        return true;
    } else {
        s_currentPhase = BATTLE_STATE_ENEMY_TURN;
        SetBattleMessage("A fuga falhou!");
        return false;
    }
}

void BattleSystem_End(void) {
    if (!s_isBattleActive) return;

    if (s_currentPhase == BATTLE_STATE_WIN) {
        for (int i = 0; i < s_playerParticipantCount; i++) {
            if (s_playerParticipants[i].original_player_ref != NULL && s_playerParticipants[i].hp > 0) {
                s_playerParticipants[i].original_player_ref->exp += 50;
                s_playerParticipants[i].original_player_ref->moedas += 20;
                // --- CORREÇÃO: Chama a função de level up para o jogador ---
                LevelUpPlayer(s_playerParticipants[i].original_player_ref);
            }
        }
    }
    ApplyBattleResults();
    s_isBattleActive = false;
    s_currentPhase = BATTLE_STATE_ENDED;
}

bool BattleSystem_IsActive(void) { return s_isBattleActive; }
BattlePhase BattleSystem_GetPhase(void) { return s_currentPhase; }
int BattleSystem_GetPlayerCount(void) { return s_playerParticipantCount; }
const BattleParticipant* BattleSystem_GetPlayerParticipant(int index) { if (index < 0 || index >= s_playerParticipantCount) return NULL; return &s_playerParticipants[index]; }
const BattleParticipant* BattleSystem_GetEnemyParticipant(void) { return &s_enemyParticipant; }
int BattleSystem_GetCurrentTurnPlayerIndex(void) { return s_currentPlayerTurn; }
const char* BattleSystem_GetLastMessage(void) { return s_battleMessage; }
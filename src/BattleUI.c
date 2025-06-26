#include "../include/ClassAttacks.h"
#include "../include/BattleUI.h"
#include "../include/BattleSystem.h"
#include "raylib.h"

#define FONT_SIZE 20
#define PADDING 10

typedef enum {
    UI_STATE_HIDDEN,
    UI_STATE_MAIN_COMMAND,
    UI_STATE_ATTACK_SELECT,
    UI_STATE_BATTLE_OVER,
} BattleUIState;

static BattleUIState s_uiState;
static int s_mainCommandSelection = 0;
static int s_attackSelection = 0;

static void DrawStatusBox(Rectangle box, const BattleParticipant* participant) {
    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);

    if (!participant || participant->hp <= 0) {
         DrawText("Derrotado", (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, GRAY);
        return;
    }

    DrawText(participant->name, (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, WHITE);
    DrawText(TextFormat("HP: %d / %d", participant->hp, participant->max_hp), (int)box.x + PADDING, (int)box.y + PADDING + 25, FONT_SIZE, participant->hp < participant->max_hp / 4 ? RED : GREEN);
    DrawText(TextFormat("MP: %d / %d", participant->mana, participant->max_mana), (int)box.x + PADDING, (int)box.y + PADDING + 50, FONT_SIZE, BLUE);
}

static void DrawCommandMenu(int selection) {
    const char* commands[] = { "Atacar", "Habilidades", "Item", "Fugir" };
    int numCommands = 4;
    Rectangle box = { 550.0f, 290.0f, 240.0f, 150.0f }; // Corrigido para float

    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);

    for (int i = 0; i < numCommands; i++) {
        Color color = (i == selection) ? YELLOW : WHITE;
        DrawText(TextFormat("%s %s", (i == selection) ? "->" : "  ", commands[i]), (int)box.x + PADDING, (int)box.y + PADDING + (i * 30), FONT_SIZE, color);
    }
}

static void DrawAttackSelectionMenu(const ClassAttackSet* attackSet, int selection) {
    if (!attackSet) return;
    Rectangle box = { 10.0f, 230.0f, 500.0f, 210.0f }; // Corrigido para float

    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);
    DrawText("Escolha a Habilidade:", (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, RAYWHITE);

    for (int i = 0; i < attackSet->attack_count; i++) {
        Color color = (i == selection) ? YELLOW : WHITE;
        DrawText(TextFormat("%s %s (MP: %d)", (i == selection) ? "->" : "  ", attackSet->attacks[i].name, attackSet->attacks[i].mana_cost),
                 (int)box.x + PADDING, (int)box.y + PADDING + 35 + (i * 30), FONT_SIZE, color);
    }
}

static void DrawMessageBox(const char* message) {
    // --- CORREÇÃO: Garantido que todos os valores sejam float ---
    Rectangle box = { (float)PADDING, (float)(450 - 120), (float)(800 - (PADDING * 2)), 110.0f };
    DrawRectangleRec(box, Fade(BLACK, 0.8f));
    DrawRectangleLinesEx(box, 2, WHITE);
    DrawText(message, (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, WHITE);

    if (s_uiState == UI_STATE_BATTLE_OVER) {
        if (((int)(GetTime() * 2.0f)) % 2 == 0) {
            DrawText("Pressione ENTER...", (int)(box.x + box.width - 200), (int)(box.y + box.height - 30), FONT_SIZE-5, LIGHTGRAY);
        }
    }
}

void BattleUI_Init(void) {
    s_uiState = UI_STATE_HIDDEN;
    s_mainCommandSelection = 0;
    s_attackSelection = 0;
}

void BattleUI_Update(void) {
    BattlePhase phase = BattleSystem_GetPhase();

    if (phase == BATTLE_STATE_PLAYER_TURN) {
        if(s_uiState == UI_STATE_HIDDEN) {
            s_uiState = UI_STATE_MAIN_COMMAND;
        }
    } else if (phase == BATTLE_STATE_WIN || phase == BATTLE_STATE_LOSE || phase == BATTLE_STATE_ENDED) {
        s_uiState = UI_STATE_BATTLE_OVER;
    } else {
        s_uiState = UI_STATE_HIDDEN;
    }

    switch (s_uiState) {
        case UI_STATE_MAIN_COMMAND:
            if (IsKeyPressed(KEY_UP)) { s_mainCommandSelection = (s_mainCommandSelection - 1 + 4) % 4; }
            if (IsKeyPressed(KEY_DOWN)) { s_mainCommandSelection = (s_mainCommandSelection + 1) % 4; }
            if (IsKeyPressed(KEY_ENTER)) {
                if (s_mainCommandSelection == 0) {
                    BattleSystem_SetPlayerAction(BattleSystem_GetCurrentTurnPlayerIndex(), 0);
                } else if (s_mainCommandSelection == 1) {
                    s_attackSelection = 0;
                    s_uiState = UI_STATE_ATTACK_SELECT;
                } else if (s_mainCommandSelection == 3) {
                    BattleSystem_AttemptEscape();
                }
            }
            break;

        case UI_STATE_ATTACK_SELECT:
            {
                const BattleParticipant* activePlayer = BattleSystem_GetPlayerParticipant(BattleSystem_GetCurrentTurnPlayerIndex());
                if (!activePlayer) { s_uiState = UI_STATE_MAIN_COMMAND; break; }
                const ClassAttackSet* attackSet = ClassAttacks_GetAttackSetForClass(activePlayer->class_id);
                if (!attackSet || attackSet->attack_count == 0) { s_uiState = UI_STATE_MAIN_COMMAND; break; }

                if (IsKeyPressed(KEY_UP)) { s_attackSelection = (s_attackSelection - 1 + attackSet->attack_count) % attackSet->attack_count; }
                if (IsKeyPressed(KEY_DOWN)) { s_attackSelection = (s_attackSelection + 1) % attackSet->attack_count; }
                if (IsKeyPressed(KEY_ESCAPE)) { s_uiState = UI_STATE_MAIN_COMMAND; }
                if (IsKeyPressed(KEY_ENTER)) {
                    BattleSystem_SetPlayerAction(BattleSystem_GetCurrentTurnPlayerIndex(), s_attackSelection);
                }
            }
            break;

        case UI_STATE_BATTLE_OVER:
            if (IsKeyPressed(KEY_ENTER)) {
                BattleSystem_End();
            }
            break;
        case UI_STATE_HIDDEN:
            break;
    }
}


void BattleUI_Draw(void) {
    ClearBackground(BLACK);
    DrawRectangle(0, 0, 800, 450, Fade(DARKGRAY, 0.5f));

    const BattleParticipant* enemy = BattleSystem_GetEnemyParticipant();
    if (enemy) {
        Rectangle enemyBox = { 550.0f, (float)PADDING, 240.0f, 100.0f }; // Corrigido para float
        DrawStatusBox(enemyBox, enemy);
    }

    for (int i = 0; i < BattleSystem_GetPlayerCount(); i++) {
        const BattleParticipant* player = BattleSystem_GetPlayerParticipant(i);
        if (player) {
            Rectangle playerBox = { (float)PADDING, (float)(PADDING + (i * 110)), 240.0f, 100.0f }; // Corrigido para float
            DrawStatusBox(playerBox, player);
        }
    }

    const char* message = BattleSystem_GetLastMessage();
    if (message[0] != '\0') {
        DrawMessageBox(message);
    }

    if (s_uiState == UI_STATE_MAIN_COMMAND) {
        DrawCommandMenu(s_mainCommandSelection);
    } else if (s_uiState == UI_STATE_ATTACK_SELECT) {
        const BattleParticipant* activePlayer = BattleSystem_GetPlayerParticipant(BattleSystem_GetCurrentTurnPlayerIndex());
        if (activePlayer) {
            DrawAttackSelectionMenu(ClassAttacks_GetAttackSetForClass(activePlayer->class_id), s_attackSelection);
        }
    }
}
#include "../include/BattleUI.h"
#include "../include/BattleSystem.h" // Para obter o estado da batalha
#include "../include/ClassSettings.h"
// #include <stdio.h> // Removido: Cabeçalho não utilizado

// --- Constantes de Layout da UI ---
#define FONT_SIZE 20
#define PADDING 10

// --- Estado Interno da UI ---
// Controla qual menu está ativo (principal, magias, itens, etc.)
typedef enum {
    UI_STATE_MAIN_COMMAND,
    UI_STATE_MOVE_SELECT,
    UI_STATE_ITEM_SELECT,
    UI_STATE_TARGET_SELECT,
    UI_STATE_WAITING,      // Esperando por uma animação ou mensagem
    UI_STATE_BATTLE_OVER
} BattleUIState;

static BattleUIState s_uiState;
static int s_mainCommandSelection = 0;
static int s_moveSelection = 0;

// --- Funções de Desenho Auxiliares ---

static void DrawStatusBox(Rectangle box, const BattleParticipant* participant) {
    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);

    if (!participant || (participant->hp <= 0 && s_uiState != UI_STATE_BATTLE_OVER)) { // Adicionado parênteses para clareza
         DrawText("Derrotado", (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, GRAY);
        return;
    }

    DrawText(participant->name, (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, WHITE);
    DrawText(TextFormat("HP: %d / %d", participant->hp, participant->max_hp), (int)box.x + PADDING, (int)box.y + PADDING + 25, FONT_SIZE, participant->hp < participant->max_hp / 4 ? RED : GREEN);
    DrawText(TextFormat("MP: %d / %d", participant->mana, participant->max_mana), (int)box.x + PADDING, (int)box.y + PADDING + 50, FONT_SIZE, BLUE);
}

static void DrawCommandMenu(int selection) {
    const char* commands[] = { "Atacar", "Magia", "Item", "Fugir" };
    int numCommands = 4;
    Rectangle box = { 550, 450 - 160, 240, 150 };

    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);

    for (int i = 0; i < numCommands; i++) {
        Color color = (i == selection) ? YELLOW : WHITE;
        DrawText(TextFormat("%s %s", (i == selection) ? "->" : "  ", commands[i]), (int)box.x + PADDING, (int)box.y + PADDING + (i * 30), FONT_SIZE, color);
    }
}

static void DrawMoveSelectionMenu(const ClassMoveset* moveset, int selection) {
    if (!moveset) return;
    Rectangle box = { 10, 450 - 220, 500, 210 };

    DrawRectangleRec(box, Fade(DARKBLUE, 0.8f));
    DrawRectangleLinesEx(box, 2, SKYBLUE);
    DrawText("Escolha a Magia:", (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, RAYWHITE);

    for (int i = 0; i < moveset->move_count; i++) {
        Color color = (i == selection) ? YELLOW : WHITE;
        DrawText(TextFormat("%s %s (MP: %d)", (i == selection) ? "->" : "  ", moveset->moves[i].name, moveset->moves[i].mana_cost),
                 (int)box.x + PADDING, (int)box.y + PADDING + 35 + (i * 30), FONT_SIZE, color);
    }
}

static void DrawMessageBox(const char* message) {
    // CORREÇÃO: Convertido para float para evitar erro de narrowing
    Rectangle box = { (float)PADDING, (float)(450 - 120), (float)(800 - (PADDING * 2)), 110.0f };
    DrawRectangleRec(box, Fade(BLACK, 0.8f));
    DrawRectangleLinesEx(box, 2, WHITE);
    DrawText(message, (int)box.x + PADDING, (int)box.y + PADDING, FONT_SIZE, WHITE);

    // Prompt piscante
    if (((int)(GetTime() * 2.0f)) % 2 == 0) {
        DrawText("Pressione ENTER...", (int)(box.x + box.width - 200), (int)(box.y + box.height - 30), FONT_SIZE-5, LIGHTGRAY);
    }
}

// --- Funções Públicas ---

void BattleUI_Init(void) {
    s_uiState = UI_STATE_MAIN_COMMAND;
    s_mainCommandSelection = 0;
    s_moveSelection = 0;
}

void BattleUI_Update(void) {
    const BattleParticipant* activePlayer = BattleSystem_GetPlayerParticipant(BattleSystem_GetCurrentTurnPlayerIndex());
    if (!activePlayer) return;

    const ClassMoveset* moveset = ClassSettings_GetMovesForClass(activePlayer->class_id);

    switch (s_uiState) {
        case UI_STATE_MAIN_COMMAND:
            if (IsKeyPressed(KEY_UP)) { s_mainCommandSelection--; if (s_mainCommandSelection < 0) s_mainCommandSelection = 3; }
            if (IsKeyPressed(KEY_DOWN)) { s_mainCommandSelection++; if (s_mainCommandSelection > 3) s_mainCommandSelection = 0; }
            if (IsKeyPressed(KEY_ENTER)) {
                if (s_mainCommandSelection == 0) { // Atacar
                    BattleSystem_SetPlayerAction(BattleSystem_GetCurrentTurnPlayerIndex(), 0); // Assume que o primeiro movimento é o ataque básico
                    s_uiState = UI_STATE_WAITING;
                } else if (s_mainCommandSelection == 1) { // Magia
                    s_moveSelection = 0;
                    s_uiState = UI_STATE_MOVE_SELECT;
                }
            }
            break;
        case UI_STATE_MOVE_SELECT:
            if (!moveset) { s_uiState = UI_STATE_MAIN_COMMAND; break; }
            if (IsKeyPressed(KEY_UP)) { s_moveSelection--; if (s_moveSelection < 0) s_moveSelection = moveset->move_count - 1; }
            if (IsKeyPressed(KEY_DOWN)) { s_moveSelection++; if (s_moveSelection >= moveset->move_count) s_moveSelection = 0; }
            if (IsKeyPressed(KEY_ESCAPE)) { s_uiState = UI_STATE_MAIN_COMMAND; }
            if (IsKeyPressed(KEY_ENTER)) {
                BattleSystem_SetPlayerAction(BattleSystem_GetCurrentTurnPlayerIndex(), s_moveSelection);
                s_uiState = UI_STATE_WAITING;
            }
            break;
        case UI_STATE_WAITING:
             // A UI espera que o BattleSystem processe os turnos e mude de fase.
             // Se voltarmos ao turno do jogador, a UI volta a mostrar o menu de comando.
            if(BattleSystem_GetPhase() == BATTLE_STATE_PLAYER_TURN){
                s_uiState = UI_STATE_MAIN_COMMAND;
                s_mainCommandSelection = 0;
            } else if (BattleSystem_GetPhase() == BATTLE_STATE_WIN || BattleSystem_GetPhase() == BATTLE_STATE_LOSE) {
                s_uiState = UI_STATE_BATTLE_OVER;
            }
            break;
        case UI_STATE_BATTLE_OVER:
            // Espera o jogador pressionar ENTER para terminar a batalha
            if(IsKeyPressed(KEY_ENTER)) {
                BattleSystem_End();
            }
            break;
        // Outros estados (ITEM_SELECT, TARGET_SELECT) podem ser adicionados aqui.
        default: break;
    }
}

void BattleUI_Draw(void) {
    // Fundo (pode ser substituído por uma imagem de fundo de batalha)
    ClearBackground(BLACK);
    DrawRectangle(0,0,800,450, Fade(DARKGRAY, 0.5f));

    // Desenha Participantes
    const BattleParticipant* enemy = BattleSystem_GetEnemyParticipant();
    if (enemy) {
        Rectangle enemyBox = { 550, PADDING, 240, 100 };
        DrawStatusBox(enemyBox, enemy);
    }

    for (int i = 0; i < BattleSystem_GetPlayerCount(); i++) {
        const BattleParticipant* player = BattleSystem_GetPlayerParticipant(i);
        if (player) {
            Rectangle playerBox = { PADDING, PADDING + (i * 110), 240, 100 };
            DrawStatusBox(playerBox, player);
        }
    }

    // Desenha o menu ou mensagem apropriado
    BattlePhase phase = BattleSystem_GetPhase();
    const char* message = BattleSystem_GetLastMessage();

    if (phase == BATTLE_STATE_WIN) {
        DrawMessageBox("VITÓRIA!");
    } else if (phase == BATTLE_STATE_LOSE) {
        DrawMessageBox("Derrota...");
    } else if (message[0] != '\0' && (s_uiState == UI_STATE_WAITING || s_uiState == UI_STATE_BATTLE_OVER)) {
        DrawMessageBox(message);
    } else if (phase == BATTLE_STATE_PLAYER_TURN && BattleSystem_GetCurrentTurnPlayerIndex() >= 0) {
        if (s_uiState == UI_STATE_MAIN_COMMAND) {
            DrawCommandMenu(s_mainCommandSelection);
        } else if (s_uiState == UI_STATE_MOVE_SELECT) {
            const BattleParticipant* activePlayer = BattleSystem_GetPlayerParticipant(BattleSystem_GetCurrentTurnPlayerIndex());
            if (activePlayer) {
                DrawMoveSelectionMenu(ClassSettings_GetMovesForClass(activePlayer->class_id), s_moveSelection);
            }
        }
    }
}
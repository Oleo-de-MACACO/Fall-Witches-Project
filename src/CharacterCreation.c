#include "../include/CharacterCreation.h"
#include "../include/Classes.h"    
#include "../include/Game.h"       
#include <stdio.h>      
#include <string.h>     
#include "raylib.h" 
#include <stddef.h> // Para NULL

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern GameModeType currentGameMode; 
extern int currentActivePlayers;    

static int creation_step = 0;                                 
static char creation_player_names[MAX_PLAYERS_SUPPORTED][MAX_PLAYER_NAME_LENGTH]; 
static int creation_name_letter_counts[MAX_PLAYERS_SUPPORTED] = {0};    
static Classe creation_player_classes[MAX_PLAYERS_SUPPORTED];           
static int creation_class_selection_idx[MAX_PLAYERS_SUPPORTED] = {0};   
static bool creation_name_edit_mode[MAX_PLAYERS_SUPPORTED] = {false};   

static const Classe AVAILABLE_CLASSES[] = {GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO};
static const int NUM_AVAILABLE_CLASSES = sizeof(AVAILABLE_CLASSES) / sizeof(AVAILABLE_CLASSES[0]); 
static const char* CLASS_DISPLAY_NAMES[] = {"Guerreiro", "Mago", "Arqueiro", "Barbaro", "Ladino", "Clerigo"};

void InitializeCharacterCreation(void) {
    creation_step = 0;
    int playersToSetup = (currentGameMode == GAME_MODE_SINGLE_PLAYER) ? 1 : MAX_PLAYERS_SUPPORTED;
    for (int i = 0; i < playersToSetup; i++) {
        strcpy(creation_player_names[i], ""); 
        creation_name_letter_counts[i] = 0;   
        creation_player_classes[i] = GUERREIRO; 
        creation_class_selection_idx[i] = 0;    
        creation_name_edit_mode[i] = false;     
    }
    if (playersToSetup > 0) creation_name_edit_mode[0] = true;
    TraceLog(LOG_INFO, "[CharacterCreation] Tela de Criacao de Personagem Inicializada para %d jogador(es).", playersToSetup);
}

void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr) {
    int playersToConfigure = (currentGameMode == GAME_MODE_SINGLE_PLAYER) ? 1 : MAX_PLAYERS_SUPPORTED;
    int player_idx_configuring = creation_step / 2;

    if (player_idx_configuring >= playersToConfigure) { 
        int final_confirmation_step = playersToConfigure * 2;
        if (creation_step == final_confirmation_step) {
            if (IsKeyPressed(KEY_ENTER)) { 
                for (int i = 0; i < playersToConfigure; i++) {
                    init_player(&players[i], creation_player_names[i], creation_player_classes[i]);
                }
                if (currentGameMode == GAME_MODE_SINGLE_PLAYER && MAX_PLAYERS_SUPPORTED > 1) {
                    memset(&players[1], 0, sizeof(Player)); 
                }
                if (mapX) *mapX = 0; 
                if (mapY) *mapY = 0; 
                currentActivePlayers = playersToConfigure; 
                
                if (playlist && currentMusicIndex >= 0 && currentMusicIndex < MAX_MUSIC_PLAYLIST_SIZE && 
                    playlist[currentMusicIndex].stream.buffer != NULL && musicIsPlaying_ptr != NULL) { 
                    if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                    PlayMusicStream(playlist[currentMusicIndex]);
                    SetMusicVolume(playlist[currentMusicIndex], currentVolume); 
                    *musicIsPlaying_ptr = 1; 
                } else if (musicIsPlaying_ptr != NULL) { 
                    *musicIsPlaying_ptr = 0; 
                }
                if (currentScreen_ptr) *currentScreen_ptr = GAMESTATE_PLAYING; 
                TraceLog(LOG_INFO, "[CharacterCreation] Criacao Completa (%dP) -> Iniciando Jogo!", playersToConfigure);
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) { 
            creation_step--; 
            if (creation_step < 0) creation_step = 0; 
            int prev_player_idx = creation_step / 2;
            if (creation_step % 2 == 0 && (prev_player_idx < playersToConfigure) ) {
                creation_name_edit_mode[prev_player_idx] = true;
                 SetMouseCursor(MOUSE_CURSOR_IBEAM);
            } else {
                 SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            }
        }
        return; 
    }
    
    if (creation_step % 2 == 0) { 
        creation_name_edit_mode[player_idx_configuring] = true; SetMouseCursor(MOUSE_CURSOR_IBEAM); 
        int key = GetCharPressed(); 
        while (key > 0) { 
            if ((key >= 32) && (key <= 125) && (creation_name_letter_counts[player_idx_configuring] < MAX_PLAYER_NAME_LENGTH - 1)) {
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = (char)key;
                creation_name_letter_counts[player_idx_configuring]++;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0'; 
            } key = GetCharPressed(); 
        }
        if (IsKeyPressed(KEY_BACKSPACE)) { if (creation_name_letter_counts[player_idx_configuring] > 0) { creation_name_letter_counts[player_idx_configuring]--; creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0'; } }
        if (IsKeyPressed(KEY_ENTER)) { if (creation_name_letter_counts[player_idx_configuring] > 0) { creation_name_edit_mode[player_idx_configuring] = false; SetMouseCursor(MOUSE_CURSOR_DEFAULT); creation_step++; } }
    } else { 
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); creation_name_edit_mode[player_idx_configuring] = false; 
        if (IsKeyPressed(KEY_UP)) { creation_class_selection_idx[player_idx_configuring]--; if (creation_class_selection_idx[player_idx_configuring] < 0) creation_class_selection_idx[player_idx_configuring] = NUM_AVAILABLE_CLASSES - 1; }
        if (IsKeyPressed(KEY_DOWN)) { creation_class_selection_idx[player_idx_configuring]++; if (creation_class_selection_idx[player_idx_configuring] >= NUM_AVAILABLE_CLASSES) creation_class_selection_idx[player_idx_configuring] = 0; }
        creation_player_classes[player_idx_configuring] = AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_configuring]];
        if (IsKeyPressed(KEY_ENTER)) { if (player_idx_configuring < playersToConfigure - 1) { creation_step++; if (player_idx_configuring + 1 < playersToConfigure) creation_name_edit_mode[player_idx_configuring + 1] = true; } else { creation_step++; } }
    }
    if (IsKeyPressed(KEY_ESCAPE)) { if (creation_step > 0) { creation_name_edit_mode[player_idx_configuring] = false; creation_step--; int new_idx = creation_step / 2; if (creation_step % 2 == 0) { if (new_idx < playersToConfigure) creation_name_edit_mode[new_idx] = true; SetMouseCursor(MOUSE_CURSOR_IBEAM); } else { SetMouseCursor(MOUSE_CURSOR_DEFAULT); } } else { if (currentScreen_ptr) *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU; } }
}

void DrawCharacterCreationScreen(Player players[]) {
    (void)players; 
    int playersToDisplay = (currentGameMode == GAME_MODE_SINGLE_PLAYER) ? 1 : MAX_PLAYERS_SUPPORTED;
    int player_idx_drawing = creation_step / 2;
    
    ClearBackground(DARKBLUE); 
    DrawText("CRIACAO DE PERSONAGEM", (int)(((float)virtualScreenWidth - (float)MeasureText("CRIACAO DE PERSONAGEM", 30)) / 2.0f), 30, 30, WHITE);
    int boxX = 50; int boxY = 80; 
    int boxWidth = virtualScreenWidth - 100; // boxWidth é usado para calcular previewX

    int final_confirmation_step = playersToDisplay * 2;
    if (creation_step < final_confirmation_step) {
        if (player_idx_drawing < playersToDisplay) {
            DrawText(TextFormat("CONFIGURANDO JOGADOR %d", player_idx_drawing + 1), boxX + 20, boxY + 20, 20, RAYWHITE);
            int current_config_player_idx = player_idx_drawing;
            if (creation_step % 2 == 0) { 
                DrawText("Digite o Nome:", boxX + 20, boxY + 60, 20, LIGHTGRAY);
                DrawRectangleLines(boxX + 20, boxY + 90, boxWidth - 40, 40, WHITE);
                DrawText(creation_player_names[current_config_player_idx], boxX + 25, boxY + 100, 20, YELLOW);
                if (creation_name_edit_mode[current_config_player_idx]) { if (((int)(GetTime()*2.0f)) % 2 == 0) DrawText("_", boxX + 25 + MeasureText(creation_player_names[current_config_player_idx], 20), boxY + 100, 20, YELLOW); }
                DrawText("Pressione ENTER para confirmar o nome.", boxX + 20, boxY + 140, 10, GRAY);
            } else { 
                DrawText(TextFormat("Nome: %s", creation_player_names[current_config_player_idx]), boxX + 20, boxY + 60, 20, YELLOW);
                DrawText("Escolha a Classe:", boxX + 20, boxY + 90, 20, LIGHTGRAY);
                int classOptionY = boxY + 120;
                for (int i = 0; i < NUM_AVAILABLE_CLASSES; i++) { 
                    Color textColor = (i == creation_class_selection_idx[current_config_player_idx]) ? ORANGE : WHITE; 
                    DrawText(TextFormat("%s %s", (i == creation_class_selection_idx[current_config_player_idx]) ? "->" : "  ", CLASS_DISPLAY_NAMES[i]), boxX + 40, classOptionY + i * 25, 20, textColor);
                }
                DrawText("Use SETAS CIMA/BAIXO e ENTER para selecionar.", boxX + 20, classOptionY + NUM_AVAILABLE_CLASSES * 25 + 20, 10, GRAY);
                Player previewPlayer; init_player(&previewPlayer, "", AVAILABLE_CLASSES[creation_class_selection_idx[current_config_player_idx]]);
                int previewX = boxX + (boxWidth / 2) + 30; 
                int previewY = boxY + 90; int previewLineHeight = 12; 
                DrawText("Atributos Base:", previewX, previewY, previewLineHeight, RAYWHITE); previewY += previewLineHeight + 2;
                DrawText(TextFormat("HP:%d MP:%d ST:%d", previewPlayer.max_vida, previewPlayer.max_mana, previewPlayer.max_stamina), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
                DrawText(TextFormat("ATK:%d DEF:%d", previewPlayer.ataque, previewPlayer.defesa), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
                DrawText(TextFormat("M.ATK:%d M.DEF:%d", previewPlayer.magic_attack, previewPlayer.magic_defense), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 5;
                DrawText("S.P.E.C.I.A.L.:", previewX, previewY, previewLineHeight, YELLOW); previewY += previewLineHeight +2;
                DrawText(TextFormat("FOR:%d PER:%d RES:%d", previewPlayer.forca, previewPlayer.percepcao, previewPlayer.resistencia), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
                DrawText(TextFormat("CAR:%d INT:%d AGI:%d SOR:%d", previewPlayer.carisma, previewPlayer.inteligencia, previewPlayer.agilidade, previewPlayer.sorte), previewX, previewY, previewLineHeight, LIGHTGRAY);
            }
        }
    } else { 
        DrawText("CONFIRMAR EQUIPE?", (int)(((float)virtualScreenWidth - (float)MeasureText("CONFIRMAR EQUIPE?", 20)) / 2.0f), boxY + 20, 20, YELLOW);
        int summaryY = boxY + 60; 
        for(int i=0; i < playersToDisplay; i++) { 
            DrawText(TextFormat("Jogador %d: %s (%s)", i+1, creation_player_names[i], CLASS_DISPLAY_NAMES[creation_class_selection_idx[i]]), boxX + 20, summaryY, 20, WHITE);
            summaryY += 30; 
        }
        DrawText("Pressione ENTER para Iniciar o Jogo", (int)(((float)virtualScreenWidth - (float)MeasureText("Pressione ENTER para Iniciar o Jogo", 20)) / 2.0f), virtualScreenHeight - 80, 20, LIME);
    }
    DrawText("Pressione ESC para Voltar/Cancelar", 20, virtualScreenHeight - 30, 10, GRAY);
}
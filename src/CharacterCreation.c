#include "../include/CharacterCreation.h"
#include "../include/Classes.h"    // Para init_player, Classe, MAX_PLAYER_NAME_LENGTH
#include "../include/Game.h"       // Para GameState, Player, Music, MAX_PLAYERS
#include <stdio.h>      
#include <string.h>     

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

static int creation_step = 0;
static char creation_player_names[MAX_PLAYERS][MAX_PLAYER_NAME_LENGTH];
static int creation_name_letter_counts[MAX_PLAYERS] = {0};
static Classe creation_player_classes[MAX_PLAYERS];
static int creation_class_selection_idx[MAX_PLAYERS] = {0};
static bool creation_name_edit_mode[MAX_PLAYERS] = {false};

static const Classe AVAILABLE_CLASSES[] = {GUERREIRO, MAGO, ARQUEIRO, BARBARO, LADINO, CLERIGO};
static const int NUM_AVAILABLE_CLASSES = sizeof(AVAILABLE_CLASSES) / sizeof(AVAILABLE_CLASSES[0]);
static const char* CLASS_DISPLAY_NAMES[] = {"Guerreiro", "Mago", "Arqueiro", "Bárbaro", "Ladino", "Clérigo"};

void InitializeCharacterCreation(void) {
    creation_step = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        strcpy(creation_player_names[i], ""); 
        creation_name_letter_counts[i] = 0;   
        creation_player_classes[i] = GUERREIRO; 
        creation_class_selection_idx[i] = 0;    
        creation_name_edit_mode[i] = false;     
    }
    if (MAX_PLAYERS > 0) creation_name_edit_mode[0] = true;
    TraceLog(LOG_INFO, "[CharacterCreation] Tela de Criação de Personagem Inicializada.");
}

void UpdateCharacterCreationScreen(GameState *currentScreen_ptr, Player players[], int *mapX, int *mapY, Music playlist[], int currentMusicIndex, float currentVolume, int *musicIsPlaying_ptr) {
    int player_idx_configuring = creation_step / 2;

    if (creation_step % 2 == 0) { 
        creation_name_edit_mode[player_idx_configuring] = true; 
        SetMouseCursor(MOUSE_CURSOR_IBEAM); 

        int key = GetCharPressed(); 
        while (key > 0) { 
            if ((key >= 32) && (key <= 125) && (creation_name_letter_counts[player_idx_configuring] < MAX_PLAYER_NAME_LENGTH - 1)) {
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = (char)key;
                creation_name_letter_counts[player_idx_configuring]++;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0'; 
            }
            key = GetCharPressed(); 
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            if (creation_name_letter_counts[player_idx_configuring] > 0) {
                creation_name_letter_counts[player_idx_configuring]--;
                creation_player_names[player_idx_configuring][creation_name_letter_counts[player_idx_configuring]] = '\0';
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            if (creation_name_letter_counts[player_idx_configuring] > 0) { 
                creation_name_edit_mode[player_idx_configuring] = false; 
                SetMouseCursor(MOUSE_CURSOR_DEFAULT); 
                creation_step++; 
                TraceLog(LOG_INFO, "[CharacterCreation] Jogador %d Nome: '%s' confirmado.", player_idx_configuring + 1, creation_player_names[player_idx_configuring]);
            } else {
                TraceLog(LOG_WARNING, "[CharacterCreation] Nome do Jogador %d não pode ser vazio.", player_idx_configuring + 1);
            }
        }
    } else { 
        SetMouseCursor(MOUSE_CURSOR_DEFAULT); 
        creation_name_edit_mode[player_idx_configuring] = false; 

        if (IsKeyPressed(KEY_UP)) {
            creation_class_selection_idx[player_idx_configuring]--;
            if (creation_class_selection_idx[player_idx_configuring] < 0) creation_class_selection_idx[player_idx_configuring] = NUM_AVAILABLE_CLASSES - 1;
        }
        if (IsKeyPressed(KEY_DOWN)) {
            creation_class_selection_idx[player_idx_configuring]++;
            if (creation_class_selection_idx[player_idx_configuring] >= NUM_AVAILABLE_CLASSES) creation_class_selection_idx[player_idx_configuring] = 0;
        }
        creation_player_classes[player_idx_configuring] = AVAILABLE_CLASSES[creation_class_selection_idx[player_idx_configuring]];

        if (IsKeyPressed(KEY_ENTER)) {
            TraceLog(LOG_INFO, "[CharacterCreation] Jogador %d Classe: %s confirmada.", player_idx_configuring + 1, CLASS_DISPLAY_NAMES[creation_class_selection_idx[player_idx_configuring]]);
            if (player_idx_configuring < MAX_PLAYERS - 1) { 
                creation_step++; 
                if (player_idx_configuring + 1 < MAX_PLAYERS) { 
                    creation_name_edit_mode[player_idx_configuring + 1] = true; 
                }
            } else { 
                creation_step++; 
            }
        }
    }

    int final_confirmation_step = MAX_PLAYERS * 2;
    if (creation_step == final_confirmation_step) {
        if (IsKeyPressed(KEY_ENTER)) { 
            for (int i = 0; i < MAX_PLAYERS; i++) {
                init_player(&players[i], creation_player_names[i], creation_player_classes[i]);
            }
            *mapX = 0; 
            *mapY = 0; 
            if (playlist[currentMusicIndex].stream.buffer != NULL) { 
                if(IsMusicStreamPlaying(playlist[currentMusicIndex])) StopMusicStream(playlist[currentMusicIndex]);
                PlayMusicStream(playlist[currentMusicIndex]);
                SetMusicVolume(playlist[currentMusicIndex], currentVolume);
                if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 1; 
            } else {
                if(musicIsPlaying_ptr) *musicIsPlaying_ptr = 0; 
            }
            *currentScreen_ptr = GAMESTATE_PLAYING; 
            TraceLog(LOG_INFO, "[CharacterCreation] Criação de Personagem Completa -> Iniciando Jogo!");
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (creation_step > 0) { 
            creation_name_edit_mode[player_idx_configuring] = false;
            creation_step--; 
            int new_player_idx_configuring_after_escape = creation_step / 2;
            if (creation_step % 2 == 0) { 
                 creation_name_edit_mode[new_player_idx_configuring_after_escape] = true;
                 SetMouseCursor(MOUSE_CURSOR_IBEAM); 
            } else {
                 SetMouseCursor(MOUSE_CURSOR_DEFAULT); 
            }
        } else { 
            *currentScreen_ptr = GAMESTATE_PLAYER_MODE_MENU;
            TraceLog(LOG_INFO, "[CharacterCreation] -> Menu Modo Jogador (ESC no primeiro passo)");
        }
         SetMouseCursor(MOUSE_CURSOR_DEFAULT); 
    }
}

void DrawCharacterCreationScreen(Player players[]) {
    (void)players;

    ClearBackground(DARKBLUE); 

    int player_idx_drawing = creation_step / 2;
    if (player_idx_drawing >= MAX_PLAYERS && creation_step < MAX_PLAYERS * 2) {
         player_idx_drawing = MAX_PLAYERS -1;
    }

    DrawText("CRIAÇÃO DE PERSONAGEM", (int)((float)virtualScreenWidth/2.0f - (float)MeasureText("CRIAÇÃO DE PERSONAGEM", 30)/2.0f), 30, 30, WHITE);

    int boxX = 50;
    int boxY = 80;
    int boxWidth = virtualScreenWidth - 100; 

    if (creation_step < MAX_PLAYERS * 2) {
        int current_config_player_idx = creation_step / 2;
        DrawText(TextFormat("CONFIGURANDO JOGADOR %d", current_config_player_idx + 1), boxX + 20, boxY + 20, 20, RAYWHITE);
        
        if (creation_step % 2 == 0) {
            DrawText("Digite o Nome:", boxX + 20, boxY + 60, 20, LIGHTGRAY);
            DrawRectangleLines(boxX + 20, boxY + 90, boxWidth - 40, 40, WHITE);
            DrawText(creation_player_names[current_config_player_idx], boxX + 25, boxY + 100, 20, YELLOW);
            if (creation_name_edit_mode[current_config_player_idx]) {
                if (((int)(GetTime()*2.0f)) % 2 == 0) { 
                    DrawText("_", boxX + 25 + MeasureText(creation_player_names[current_config_player_idx], 20), boxY + 100, 20, YELLOW);
                }
            }
            DrawText("Pressione ENTER para confirmar o nome.", boxX + 20, boxY + 140, 10, GRAY);
        }
        else { 
            DrawText(TextFormat("Nome: %s", creation_player_names[current_config_player_idx]), boxX + 20, boxY + 60, 20, YELLOW);
            DrawText("Escolha a Classe:", boxX + 20, boxY + 90, 20, LIGHTGRAY);

            int classOptionY = boxY + 120;
            for (int i = 0; i < NUM_AVAILABLE_CLASSES; i++) {
                Color textColor = (i == creation_class_selection_idx[current_config_player_idx]) ? ORANGE : WHITE; 
                DrawText(TextFormat("%s %s", (i == creation_class_selection_idx[current_config_player_idx]) ? "->" : "  ", CLASS_DISPLAY_NAMES[i]),
                         boxX + 40, classOptionY + i * 25, 20, textColor);
            }
            DrawText("Use SETAS CIMA/BAIXO e ENTER para selecionar.", boxX + 20, classOptionY + NUM_AVAILABLE_CLASSES * 25 + 20, 10, GRAY);

            Player previewPlayer; 
            init_player(&previewPlayer, "", AVAILABLE_CLASSES[creation_class_selection_idx[current_config_player_idx]]);
            int previewX = boxX + (boxWidth / 2) + 30; // Ajustado X para melhor espaçamento
            int previewY = boxY + 90;                 
            int previewLineHeight = 12; // Menor para caber mais stats

            DrawText("Atributos Base:", previewX, previewY, previewLineHeight, RAYWHITE); previewY += previewLineHeight + 2;
            DrawText(TextFormat("HP:%d MP:%d ST:%d", previewPlayer.max_vida, previewPlayer.max_mana, previewPlayer.max_stamina), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
            DrawText(TextFormat("ATK:%d DEF:%d", previewPlayer.ataque, previewPlayer.defesa), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
            DrawText(TextFormat("M.ATK:%d M.DEF:%d", previewPlayer.magic_attack, previewPlayer.magic_defense), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 5; // Espaço antes de S.P.E.C.I.A.L.
            
            DrawText("S.P.E.C.I.A.L.:", previewX, previewY, previewLineHeight, YELLOW); previewY += previewLineHeight +2;
            DrawText(TextFormat("FOR:%d PER:%d RES:%d", previewPlayer.forca, previewPlayer.percepcao, previewPlayer.resistencia), previewX, previewY, previewLineHeight, LIGHTGRAY); previewY += previewLineHeight + 2;
            DrawText(TextFormat("CAR:%d INT:%d AGI:%d SOR:%d", previewPlayer.carisma, previewPlayer.inteligencia, previewPlayer.agilidade, previewPlayer.sorte), previewX, previewY, previewLineHeight, LIGHTGRAY);
        }
    } else { 
        DrawText("CONFIRMAR EQUIPE?", (int)((float)virtualScreenWidth/2.0f - (float)MeasureText("CONFIRMAR EQUIPE?", 20)/2.0f), boxY + 20, 20, YELLOW);
        int summaryY = boxY + 60; 
        for(int i=0; i < MAX_PLAYERS; i++) {
            DrawText(TextFormat("Jogador %d: %s (%s)", i+1, creation_player_names[i], CLASS_DISPLAY_NAMES[creation_class_selection_idx[i]]),
                     boxX + 20, summaryY, 20, WHITE);
            summaryY += 30; 
        }
        DrawText("Pressione ENTER para Iniciar o Jogo", (int)((float)virtualScreenWidth/2.0f - (float)MeasureText("Pressione ENTER para Iniciar o Jogo", 20)/2.0f), virtualScreenHeight - 80, 20, LIME);
    }
    DrawText("Pressione ESC para Voltar/Cancelar", 20, virtualScreenHeight - 30, 10, GRAY);
}
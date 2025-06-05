#include "../include/Inventory.h"
#include "../include/Game.h"
#include "../include/Classes.h"
#include "../include/Sound.h" // *** ADICIONADO O INCLUDE QUE FALTAVA ***
#include <string.h>
#include <stdio.h>
#include <raylib.h>
#include <stddef.h>

// Variáveis globais externas (main.c -> Game.h -> aqui)
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;
extern int currentActivePlayers;

static bool first_inventory_update_call = true;
static int music_was_playing_on_inventory_entry = 0;

const char *tab_display_names[MAX_INVENTORY_TABS] = {"Inventario", "Equipamento", "Status"};

bool AddItemToInventory(Player *player, const char *itemName, int quantity) {
    // ... (código da função como antes, sem alterações nesta passagem) ...
    if (!player || !itemName || strlen(itemName) == 0 || quantity <= 0) return false;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) {
            player->inventory[i].quantity += quantity;
            return true;
        }
    }
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strlen(player->inventory[i].name) == 0) {
            strncpy(player->inventory[i].name, itemName, MAX_ITEM_NAME_LENGTH - 1);
            player->inventory[i].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
            player->inventory[i].quantity = quantity;
            int occupied_slots = 0;
            for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
            player->inventory_item_count = occupied_slots;
            return true;
        }
    }
    TraceLog(LOG_WARNING, "Nao foi possivel adicionar o item '%s': inventario cheio.", itemName);
    return false;
}

bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity) {
    // ... (código da função como antes, sem alterações nesta passagem) ...
    if (!player || !itemName || strlen(itemName) == 0 || quantity <= 0) return false;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) {
            if (player->inventory[i].quantity >= quantity) {
                player->inventory[i].quantity -= quantity;
                if (player->inventory[i].quantity == 0) {
                    strcpy(player->inventory[i].name, "");
                    int occupied_slots = 0;
                    for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
                    player->inventory_item_count = occupied_slots;
                }
                return true;
            } else {
                TraceLog(LOG_WARNING, "Nao foi possivel remover %d de '%s': quantidade insuficiente (%d).", quantity, itemName, player->inventory[i].quantity);
                return false;
            }
        }
    }
    TraceLog(LOG_WARNING, "Nao foi possivel remover item '%s': item nao encontrado.", itemName);
    return false;
}

// Parâmetro musicIsPlaying_ptr corrigido para bool*
void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], bool *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr) {
    if (!currentScreen_ptr || !players ) return;

    if (first_inventory_update_call) {
        if (IsSoundModuleMusicPlaying()) { // Função de Sound.h
            music_was_playing_on_inventory_entry = 1;
            PauseCurrentMusic(); // Função de Sound.h
        } else {
            music_was_playing_on_inventory_entry = 0;
        }
        // Lógica legada para playlist, se musicIsPlaying_ptr e playlist não forem NULL
        if (musicIsPlaying_ptr && playlist && currentMusicIndex_ptr) {
            if (*musicIsPlaying_ptr && // Usa bool*
                *currentMusicIndex_ptr >=0 && *currentMusicIndex_ptr < MAX_MUSIC_PLAYLIST_SIZE &&
                playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
                // PauseMusicStream(playlist[*currentMusicIndex_ptr]); // Esta parte é redundante se Sound.c gerencia
                // *musicIsPlaying_ptr = false; // Atualiza o bool*
            }
        }

        for (int i = 0; i < MAX_PLAYERS_SUPPORTED; i++) {
             if (players[i].current_inventory_tab < 0 || players[i].current_inventory_tab >= MAX_INVENTORY_TABS) {
                 players[i].current_inventory_tab = TAB_INVENTORY;
             }
        }
        first_inventory_update_call = false;
    }

    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        if (music_was_playing_on_inventory_entry) {
            ResumeCurrentMusic(); // Função de Sound.h
        }
        if (musicIsPlaying_ptr && playlist && currentMusicIndex_ptr) {
             if (music_was_playing_on_inventory_entry &&
                *currentMusicIndex_ptr >=0 && *currentMusicIndex_ptr < MAX_MUSIC_PLAYLIST_SIZE &&
                playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
                // ResumeMusicStream(playlist[*currentMusicIndex_ptr]); // Redundante se Sound.c gerencia
                // *musicIsPlaying_ptr = true; // Atualiza o bool*
            }
        }
        first_inventory_update_call = true;
        music_was_playing_on_inventory_entry = 0;
        return;
    }

    if (MAX_PLAYERS_SUPPORTED > 0 && currentActivePlayers > 0) {
        if (IsKeyPressed(KEY_A)) { players[0].current_inventory_tab = (players[0].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS; }
        if (IsKeyPressed(KEY_D)) { players[0].current_inventory_tab = (players[0].current_inventory_tab + 1) % MAX_INVENTORY_TABS; }
    }
    if (MAX_PLAYERS_SUPPORTED > 1 && currentActivePlayers > 1) {
        if (IsKeyPressed(KEY_LEFT)) { players[1].current_inventory_tab = (players[1].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS; }
        if (IsKeyPressed(KEY_RIGHT)) { players[1].current_inventory_tab = (players[1].current_inventory_tab + 1) % MAX_INVENTORY_TABS; }
    }
}

void DrawPlayerPanelContent(Player *player, Rectangle panel_bounds) {

    if (!player) return;
    char buffer[128];
    int line_height = 18;
    int text_padding_x = 10;
    int current_y = (int)(panel_bounds.y + 10.0f);

    sprintf(buffer, "HP: %d / %d", player->vida, player->max_vida); DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, ((float)player->vida < (float)player->max_vida * 0.3f) ? RED : LIME); current_y += line_height + 1;
    sprintf(buffer, "MP: %d / %d", player->mana, player->max_mana); DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, SKYBLUE); current_y += line_height + 1;
    sprintf(buffer, "ST: %d / %d", player->stamina, player->max_stamina); DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, YELLOW); current_y += line_height + 1;
    sprintf(buffer, "Moedas: %d", player->moedas); DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, GOLD); current_y += line_height + 8;

    int current_tab_x_pos = (int)(panel_bounds.x + (float)text_padding_x);
    int tab_text_y = current_y;
    for (int i = 0; i < MAX_INVENTORY_TABS; i++) {
        Color tab_color = (player->current_inventory_tab == i) ? YELLOW : GRAY; // Compara int com int
        int text_width = MeasureText(tab_display_names[i], line_height);
        DrawText(tab_display_names[i], current_tab_x_pos, tab_text_y, line_height, tab_color);
        if (player->current_inventory_tab == i) { // Compara int com int
            DrawRectangle((int)((float)current_tab_x_pos - 3.0f), (int)((float)tab_text_y - 3.0f), text_width + 6, line_height + 6, Fade(YELLOW, 0.15f));
            DrawLineEx((Vector2){(float)current_tab_x_pos, (float)(tab_text_y + line_height + 2)}, (Vector2){(float)current_tab_x_pos + (float)text_width, (float)(tab_text_y + line_height + 2)}, 2.0f, YELLOW);
        }
        current_tab_x_pos += text_width + 20;
    }
    DrawLineEx((Vector2){panel_bounds.x + 5.0f, (float)(tab_text_y + line_height + 4)}, (Vector2){panel_bounds.x + panel_bounds.width - 5.0f, (float)(tab_text_y + line_height + 4)}, 1.0f, Fade(LIGHTGRAY, 0.5f));
    current_y = tab_text_y + line_height + 12;
    int col1_x = (int)(panel_bounds.x + (float)text_padding_x);
    int col2_x = (int)(panel_bounds.x + (float)text_padding_x + (panel_bounds.width / 2.0f) - ((float)text_padding_x / 2.0f)) ;
    int initial_content_y = current_y;

    switch ((InventoryTabType)player->current_inventory_tab) {
        case TAB_INVENTORY:
            for (int i_slot = 0; i_slot < MAX_INVENTORY_SLOTS; i_slot++) { // Renomeado i para i_slot para evitar conflito com loop externo se houvesse
                if (current_y + line_height > (int)(panel_bounds.y + panel_bounds.height - 5.0f)) break;
                if (strlen(player->inventory[i_slot].name) > 0) { sprintf(buffer, "- %s (%d)", player->inventory[i_slot].name, player->inventory[i_slot].quantity); DrawText(buffer, col1_x, current_y, line_height, WHITE);
                } else { DrawText("- Vazio -", col1_x, current_y, line_height, DARKGRAY); }
                current_y += line_height + 3;
            } break;
        case TAB_EQUIPMENT:
            sprintf(buffer, "Arma: %s", strlen(player->equipped_items[EQUIP_SLOT_WEAPON].name) > 0 ? player->equipped_items[EQUIP_SLOT_WEAPON].name : "<Nenhuma>"); DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Armadura: %s", strlen(player->equipped_items[EQUIP_SLOT_ARMOR].name) > 0 ? player->equipped_items[EQUIP_SLOT_ARMOR].name : "<Nenhuma>"); DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Acessorio: %s", strlen(player->equipped_items[EQUIP_SLOT_ACCESSORY].name) > 0 ? player->equipped_items[EQUIP_SLOT_ACCESSORY].name : "<Nenhum>"); DrawText(buffer, col1_x, current_y, line_height, WHITE);
            break;
        case TAB_STATUS: {
                int stat_y_col1 = initial_content_y; int stat_y_col2 = initial_content_y; char class_name_str[30];
                strcpy(class_name_str, "Desconhecida");
                switch(player->classe) {
                    case GUERREIRO: strcpy(class_name_str, "Guerreiro"); break; case MAGO: strcpy(class_name_str, "Mago"); break; case ARQUEIRO: strcpy(class_name_str, "Arqueiro"); break;
                    case BARBARO: strcpy(class_name_str, "Barbaro"); break; case LADINO: strcpy(class_name_str, "Ladino"); break; case CLERIGO: strcpy(class_name_str, "Clerigo"); break;
                    default: strcpy(class_name_str, "Nao Definida"); break;
                }
            sprintf(buffer, "Nome: %s", player->nome); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Classe: %s", class_name_str); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Nivel: %d (EXP: %d)", player->nivel, player->exp); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 6;
            sprintf(buffer, "Ataque Fisico: %d", player->ataque); DrawText(buffer, col1_x, stat_y_col1, line_height, ORANGE); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Defesa Fisica: %d", player->defesa); DrawText(buffer, col1_x, stat_y_col1, line_height, SKYBLUE); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Ataque Magico: %d", player->magic_attack); DrawText(buffer, col1_x, stat_y_col1, line_height, MAGENTA); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Defesa Magica: %d", player->magic_defense); DrawText(buffer, col1_x, stat_y_col1, line_height, VIOLET); stat_y_col1 += line_height + 6;
            DrawText("ATRIBUTOS S.P.E.C.I.A.L.:", col1_x, stat_y_col1, line_height, YELLOW);
            stat_y_col2 = stat_y_col1 + line_height + 3; stat_y_col1 += line_height + 3;
            sprintf(buffer, "Forca: %d", player->forca); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Percepcao: %d", player->percepcao); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Resistencia: %d", player->resistencia); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
            sprintf(buffer, "Carisma: %d", player->carisma); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY);
            sprintf(buffer, "Inteligencia: %d", player->inteligencia); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;
            sprintf(buffer, "Agilidade: %d", player->agilidade); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;
            sprintf(buffer, "Sorte: %d", player->sorte); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY);
            } break;
        default: DrawText("Aba selecionada invalida.", col1_x, current_y, line_height, RED); break;
    }
}

// A assinatura de DrawInventoryScreen já espera bool bg_music_playing
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float bg_music_vol, int bg_music_idx, bool bg_music_playing, int bg_mapX, int bg_mapY) {
    (void)players_arr;
    DrawPlayingScreen(background_players_arr, currentActivePlayers, bg_music_vol, bg_music_idx, bg_music_playing, bg_mapX, bg_mapY);
}

void DrawInventoryUIElements(Player players_arr[]) {
    // ... (código da função como antes, sem alterações nesta passagem) ...
    if (!players_arr) return;
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.90f));
    int top_instruction_area_height = 35; int bottom_instruction_area_height = 35;
    int panel_area_y_start = top_instruction_area_height;
    int panel_area_height = virtualScreenHeight - top_instruction_area_height - bottom_instruction_area_height;
    int panel_padding = 10;
    const char *main_title = "PAINEL DOS JOGADORES";
    DrawText(main_title, (int)(((float)virtualScreenWidth - (float)MeasureText(main_title, 20)) / 2.0f), 8, 20, RAYWHITE);
    int playersToDraw = currentActivePlayers;
    if (playersToDraw > 0) {
        if (playersToDraw == 1) {
            Rectangle panel_p1_bounds = {(float)panel_padding, (float)panel_area_y_start, (float)virtualScreenWidth - 2.0f * (float)panel_padding, (float)panel_area_height };
            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);
        } else {
            float panel_width = ((float)virtualScreenWidth - 3.0f * (float)panel_padding) / 2.0f;
            Rectangle panel_p1_bounds = {(float)panel_padding, (float)panel_area_y_start, panel_width, (float)panel_area_height };
            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);
            if (MAX_PLAYERS_SUPPORTED > 1 && playersToDraw > 1) {
                Rectangle panel_p2_bounds = {(float)panel_padding * 2.0f + panel_width, (float)panel_area_y_start, panel_width, (float)panel_area_height };
                DrawRectangleRoundedLinesEx(panel_p2_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
                DrawRectangleRec(panel_p2_bounds, Fade(DARKGRAY, 0.6f));
                DrawPlayerPanelContent(&players_arr[1], panel_p2_bounds);
            }
        }
    }
    const char* closeInstruction = "Pressione 'E' ou 'ESC' para fechar";
    const char* p1NavInstruction = "P1 Abas: A / D";
    const char* p2NavInstruction = "P2 Abas: Setas Esq / Dir";
    int closeWidth = MeasureText(closeInstruction, 10);
    DrawText(closeInstruction, (int)(((float)virtualScreenWidth - (float)closeWidth)/2.0f) , virtualScreenHeight - 25, 10, WHITE);
    if (playersToDraw > 0) DrawText(p1NavInstruction, panel_padding, virtualScreenHeight - 25, 10, YELLOW);
    if (playersToDraw > 1 && MAX_PLAYERS_SUPPORTED > 1) DrawText(p2NavInstruction, virtualScreenWidth - MeasureText(p2NavInstruction, 10) - panel_padding, virtualScreenHeight - 25, 10, YELLOW);
}
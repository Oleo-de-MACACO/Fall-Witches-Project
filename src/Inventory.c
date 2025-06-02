#include "../include/Inventory.h"
#include "../include/Game.h"    
#include "../include/Classes.h"  
#include <string.h>             
#include <stdio.h>              
#include <raylib.h>             

extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

static int music_was_playing_on_inventory_entry = 0;
static bool first_inventory_update_call = true;
const char *tab_display_names[MAX_INVENTORY_TABS] = {"Inventário", "Equipamento", "Status"};

bool AddItemToInventory(Player *player, const char *itemName, int quantity) {
    if (!player || !itemName || strlen(itemName) == 0 || quantity <= 0) return false;
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) {
            player->inventory[i].quantity += quantity; return true;
        }
    }
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strlen(player->inventory[i].name) == 0) {
            strncpy(player->inventory[i].name, itemName, MAX_ITEM_NAME_LENGTH - 1);
            player->inventory[i].name[MAX_ITEM_NAME_LENGTH - 1] = '\0';
            player->inventory[i].quantity = quantity;
            int occupied_slots = 0;
            for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
            player->inventory_item_count = occupied_slots; return true;
        }
    }
    return false;
}

bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity) {
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
                } return true;
            } else { return false; }
        }
    }
    return false; 
}

void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr) {
    if (first_inventory_update_call) {
        music_was_playing_on_inventory_entry = *musicIsPlaying_ptr;
        if (music_was_playing_on_inventory_entry && playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist[*currentMusicIndex_ptr]); *musicIsPlaying_ptr = 0;
        }
        for (int i = 0; i < MAX_PLAYERS; i++) {
             if (players[i].current_inventory_tab < 0 || players[i].current_inventory_tab >= MAX_INVENTORY_TABS) {
                 players[i].current_inventory_tab = TAB_INVENTORY;
             }
        }
        first_inventory_update_call = false;
    }
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_PLAYING;
        if (music_was_playing_on_inventory_entry && playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
            ResumeMusicStream(playlist[*currentMusicIndex_ptr]); *musicIsPlaying_ptr = 1;
        }
        first_inventory_update_call = true; music_was_playing_on_inventory_entry = 0; return;
    }
    if (MAX_PLAYERS > 0) { 
        if (IsKeyPressed(KEY_A)) players[0].current_inventory_tab = (players[0].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        if (IsKeyPressed(KEY_D)) players[0].current_inventory_tab = (players[0].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
    }
    if (MAX_PLAYERS > 1) { 
        if (IsKeyPressed(KEY_LEFT)) players[1].current_inventory_tab = (players[1].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        if (IsKeyPressed(KEY_RIGHT)) players[1].current_inventory_tab = (players[1].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
    }
}

void DrawPlayerPanelContent(Player *player, Rectangle panel_bounds) {
    if (!player) return;
    char buffer[128]; int line_height = 18; int text_padding_x = 10;
    int current_y = (int)(panel_bounds.y + 10.0f); // Explicit cast

    sprintf(buffer, "HP: %d / %d", player->vida, player->max_vida);
    DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, (player->vida < player->max_vida * 0.3f) ? RED : LIME);
    current_y += line_height + 1;
    sprintf(buffer, "MP: %d / %d", player->mana, player->max_mana);
    DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, SKYBLUE);
    current_y += line_height + 1;
    sprintf(buffer, "ST: %d / %d", player->stamina, player->max_stamina);
    DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, YELLOW);
    current_y += line_height + 1;
    sprintf(buffer, "Moedas: %d", player->moedas);
    DrawText(buffer, (int)(panel_bounds.x + (float)text_padding_x), current_y, line_height, GOLD);
    current_y += line_height + 8;

    int tab_start_x = (int)(panel_bounds.x + (float)text_padding_x); int tab_text_y = current_y;
    for (int i = 0; i < MAX_INVENTORY_TABS; i++) {
        Color tab_color = (player->current_inventory_tab == i) ? YELLOW : GRAY;
        DrawText(tab_display_names[i], tab_start_x, tab_text_y, line_height, tab_color);
        if (player->current_inventory_tab == i) {
            DrawRectangle((int)((float)tab_start_x - 3.0f), (int)((float)tab_text_y - 3.0f), MeasureText(tab_display_names[i], line_height) + 6, line_height + 6, Fade(YELLOW, 0.15f));
            DrawLineEx((Vector2){(float)tab_start_x, (float)(tab_text_y + line_height + 2)}, (Vector2){(float)tab_start_x + (float)MeasureText(tab_display_names[i], line_height), (float)(tab_text_y + line_height + 2)}, 2.0f, YELLOW);
        }
        tab_start_x += MeasureText(tab_display_names[i], line_height) + 20;
    }
    DrawLineEx((Vector2){panel_bounds.x + 5.0f, (float)(tab_text_y + line_height + 4)}, (Vector2){panel_bounds.x + panel_bounds.width - 5.0f, (float)(tab_text_y + line_height + 4)}, 1.0f, Fade(LIGHTGRAY, 0.5f));
    current_y = tab_text_y + line_height + 12;
    int col1_x = (int)(panel_bounds.x + (float)text_padding_x);
    int col2_x = (int)(panel_bounds.x + (float)text_padding_x + (panel_bounds.width / 2.0f) - ((float)text_padding_x / 2.0f)) ; 
    int initial_content_y = current_y;

    switch ((InventoryTabType)player->current_inventory_tab) {
        case TAB_INVENTORY: 
            for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
                if (current_y + line_height > (int)(panel_bounds.y + panel_bounds.height - 5.0f)) break;
                if (strlen(player->inventory[i].name) > 0) {
                    sprintf(buffer, "- %s (%d)", player->inventory[i].name, player->inventory[i].quantity);
                    DrawText(buffer, col1_x, current_y, line_height, WHITE);
                } else { DrawText("- Vazio -", col1_x, current_y, line_height, DARKGRAY); }
                current_y += line_height + 3;
            }
            break;
        case TAB_EQUIPMENT: 
            sprintf(buffer, "Arma: %s", strlen(player->equipped_items[EQUIP_SLOT_WEAPON].name) > 0 ? player->equipped_items[EQUIP_SLOT_WEAPON].name : "<Nenhuma>");
            DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Armadura: %s", strlen(player->equipped_items[EQUIP_SLOT_ARMOR].name) > 0 ? player->equipped_items[EQUIP_SLOT_ARMOR].name : "<Nenhuma>");
            DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Acessorio: %s", strlen(player->equipped_items[EQUIP_SLOT_ACCESSORY].name) > 0 ? player->equipped_items[EQUIP_SLOT_ACCESSORY].name : "<Nenhum>"); // Corrigido para ASCII
            DrawText(buffer, col1_x, current_y, line_height, WHITE);
            break;
        case TAB_STATUS: { 
                int stat_y_col1 = initial_content_y; int stat_y_col2 = initial_content_y;
                char class_name_str[30] = "Desconhecida";
                switch(player->classe) { 
                    case GUERREIRO: strcpy(class_name_str, "Guerreiro"); break;
                    case MAGO:    strcpy(class_name_str, "Mago");    break;
                    case ARQUEIRO:  strcpy(class_name_str, "Arqueiro");  break;
                    case BARBARO: strcpy(class_name_str, "Barbaro"); break; // Corrigido para ASCII
                    case LADINO:  strcpy(class_name_str, "Ladino");  break;
                    case CLERIGO: strcpy(class_name_str, "Clerigo"); break;
                    default: strcpy(class_name_str, "Desconhecida"); break; // Adicionado default case explícito
                }
                sprintf(buffer, "Nome: %s", player->nome); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Classe: %s", class_name_str); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Nivel: %d (EXP: %d)", player->nivel, player->exp); DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 6;
                sprintf(buffer, "Ataque Fisico: %d", player->ataque); DrawText(buffer, col1_x, stat_y_col1, line_height, ORANGE); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Defesa Fisica: %d", player->defesa); DrawText(buffer, col1_x, stat_y_col1, line_height, SKYBLUE); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Ataque Magico: %d", player->magic_attack); DrawText(buffer, col1_x, stat_y_col1, line_height, MAGENTA); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Defesa Magica: %d", player->magic_defense); DrawText(buffer, col1_x, stat_y_col1, line_height, VIOLET); stat_y_col1 += line_height + 6; // Corrigido para ASCII
                DrawText("ATRIBUTOS S.P.E.C.I.A.L.:", col1_x, stat_y_col1, line_height, YELLOW);
                stat_y_col2 = stat_y_col1; stat_y_col1 += line_height + 3; stat_y_col2 += line_height + 3; 
                sprintf(buffer, "Forca: %d", player->forca); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Percepcao: %d", player->percepcao); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Resistencia: %d", player->resistencia); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Carisma: %d", player->carisma); DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Inteligencia: %d", player->inteligencia); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1; // Corrigido para ASCII
                sprintf(buffer, "Agilidade: %d", player->agilidade); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;
                sprintf(buffer, "Sorte: %d", player->sorte); DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;
            } break;
        default: DrawText("Aba selecionada invalida.", col1_x, current_y, line_height, RED); break; // Corrigido para ASCII
    }
}

void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY) {
    (void)players_arr; 
    DrawPlayingScreen(background_players_arr, background_musicVolume, background_currentMusicIndex, background_musicIsPlaying, background_mapX, background_mapY);
}

void DrawInventoryUIElements(Player players_arr[]) {
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.90f));
    int top_instruction_area_height = 35; int bottom_instruction_area_height = 35;
    int panel_area_y_start = top_instruction_area_height;
    int panel_area_height = virtualScreenHeight - top_instruction_area_height - bottom_instruction_area_height;
    int panel_padding = 10;
    const char *main_title = "PAINEL DOS JOGADORES";
    DrawText(main_title, (int)(((float)virtualScreenWidth - (float)MeasureText(main_title, 20)) / 2.0f), 8, 20, RAYWHITE);

    if (MAX_PLAYERS > 0 && players_arr != NULL) {
        if (MAX_PLAYERS == 1) { 
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
            if (MAX_PLAYERS > 1) {
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
    if (MAX_PLAYERS > 0) DrawText(p1NavInstruction, panel_padding, virtualScreenHeight - 25, 10, YELLOW);
    if (MAX_PLAYERS > 1) DrawText(p2NavInstruction, virtualScreenWidth - MeasureText(p2NavInstruction, 10) - panel_padding, virtualScreenHeight - 25, 10, YELLOW);
}
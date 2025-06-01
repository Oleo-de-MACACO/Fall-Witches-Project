#include "../include/Inventory.h" // Para as declarações das funções de inventário e InventoryTabType
#include "../include/Game.h"       // Para DrawPlayingScreen (usado por DrawInventoryScreen) e GameState, MAX_PLAYERS
#include "../include/Classes.h"    // Para a struct Player e definições de itens
#include <string.h>             // Para strcmp, strcpy, strncpy, strlen
#include <stdio.h>              // Para sprintf
#include <raylib.h>             // Para funções da Raylib e definições de teclas (KEY_A, KEY_D, etc.)

// Variável estática para rastrear se a música estava tocando quando o inventário foi aberto.
static int music_was_playing_on_inventory_entry = 0;
// Flag para controlar a lógica de "primeira entrada" na tela de inventário.
static bool first_inventory_update_call = true;

// Nomes das abas para exibição
const char *tab_display_names[MAX_INVENTORY_TABS] = {"Inventário", "Equipamento", "Status"};

/**
 * @brief Adiciona um item ao inventário de um jogador.
 * Tenta empilhar com itens existentes do mesmo nome ou usa um novo slot vazio.
 * @param player Ponteiro para o jogador cujo inventário será modificado.
 * @param itemName Nome do item a ser adicionado.
 * @param quantity Quantidade a ser adicionada. Deve ser maior que zero.
 * @return Retorna true se o item foi adicionado (ou empilhado), false caso contrário.
 */
bool AddItemToInventory(Player *player, const char *itemName, int quantity) {
    if (!player) { TraceLog(LOG_WARNING, "Tentativa de adicionar item a um jogador NULL."); return false; }
    if (!itemName || strlen(itemName) == 0) { TraceLog(LOG_WARNING, "Tentativa de adicionar item com nome inválido."); return false; }
    if (quantity <= 0) { TraceLog(LOG_WARNING, "Tentativa de adicionar quantidade inválida (%d) do item %s.", quantity, itemName); return false; }

    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) {
            player->inventory[i].quantity += quantity;
            TraceLog(LOG_INFO, "Item '%s' empilhado para jogador '%s'. Nova quantidade: %d.", itemName, player->nome, player->inventory[i].quantity);
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
            TraceLog(LOG_INFO, "Item '%s' adicionado ao inventário do jogador '%s' (slot %d). Quantidade: %d.", itemName, player->nome, i, quantity);
            return true;
        }
    }
    TraceLog(LOG_WARNING, "Inventário do jogador '%s' cheio. Não foi possível adicionar '%s'.", player->nome, itemName);
    return false;
}

/**
 * @brief Remove uma certa quantidade de um item do inventário de um jogador.
 * @param player Ponteiro para o jogador cujo inventário será modificado.
 * @param itemName Nome do item a ser removido.
 * @param quantity Quantidade a ser removida. Deve ser maior que zero.
 * @return Retorna true se a quantidade especificada do item foi removida, false caso contrário.
 */
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity) {
    if (!player) { TraceLog(LOG_WARNING, "Tentativa de remover item de um jogador NULL."); return false;}
    if (!itemName || strlen(itemName) == 0) { TraceLog(LOG_WARNING, "Tentativa de remover item com nome inválido."); return false;}
    if (quantity <= 0) { TraceLog(LOG_WARNING, "Tentativa de remover quantidade inválida (%d) do item '%s'.", quantity, itemName); return false;}

    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) {
            if (player->inventory[i].quantity >= quantity) {
                player->inventory[i].quantity -= quantity;
                TraceLog(LOG_INFO, "%d unidade(s) do item '%s' removida(s) do jogador '%s'. Restante: %d.", quantity, itemName, player->nome, player->inventory[i].quantity);
                if (player->inventory[i].quantity == 0) {
                    strcpy(player->inventory[i].name, "");
                    int occupied_slots = 0;
                    for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
                    player->inventory_item_count = occupied_slots;
                    TraceLog(LOG_INFO, "Item '%s' esgotado e slot liberado para jogador '%s'.", itemName, player->nome);
                }
                return true;
            } else {
                TraceLog(LOG_WARNING, "Quantidade insuficiente de '%s' (%d) para remover do jogador '%s' (pedido: %d).", itemName, player->inventory[i].quantity, player->nome, quantity);
                return false;
            }
        }
    }
    TraceLog(LOG_WARNING, "Item '%s' não encontrado no inventário do jogador '%s' para remover.", itemName, player->nome);
    return false;
}

/**
 * @brief Atualiza a tela de inventário: lida com fechamento e navegação de abas.
 */
void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr) {
    if (first_inventory_update_call) {
        music_was_playing_on_inventory_entry = *musicIsPlaying_ptr;
        if (music_was_playing_on_inventory_entry && playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist[*currentMusicIndex_ptr]);
            *musicIsPlaying_ptr = 0;
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
            ResumeMusicStream(playlist[*currentMusicIndex_ptr]);
            *musicIsPlaying_ptr = 1;
        }
        first_inventory_update_call = true;
        music_was_playing_on_inventory_entry = 0;
        return;
    }

    if (MAX_PLAYERS > 0) {
        if (IsKeyPressed(KEY_A)) {
            players[0].current_inventory_tab = (players[0].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        }
        if (IsKeyPressed(KEY_D)) {
            players[0].current_inventory_tab = (players[0].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
        }
    }

    if (MAX_PLAYERS > 1) {
        if (IsKeyPressed(KEY_LEFT)) {
            players[1].current_inventory_tab = (players[1].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            players[1].current_inventory_tab = (players[1].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
        }
    }
}

/**
 * @brief Desenha o painel individual de um jogador (HP/MP, Abas, Conteúdo da Aba).
 */
void DrawPlayerPanelContent(Player *player, Rectangle panel_bounds) {
    char buffer[128];
    int line_height = 20;
    int text_padding_x = 15;
    int current_y = panel_bounds.y + 10;

    sprintf(buffer, "HP: %d / %d", player->vida, player->max_vida);
    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, (player->vida < player->max_vida * 0.3f) ? RED : LIME);
    current_y += line_height + 2;
    sprintf(buffer, "MP: %d / %d", player->mana, player->max_mana);
    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, SKYBLUE);
    current_y += line_height + 10;

    int tab_start_x = panel_bounds.x + text_padding_x;
    int tab_text_y = current_y;
    int tab_underline_y = tab_text_y + line_height + 2;

    for (int i = 0; i < MAX_INVENTORY_TABS; i++) {
        Color tab_color = (player->current_inventory_tab == i) ? YELLOW : GRAY;
        DrawText(tab_display_names[i], tab_start_x, tab_text_y, line_height, tab_color);
        if (player->current_inventory_tab == i) {
            DrawRectangle(tab_start_x - 3, tab_text_y - 3, MeasureText(tab_display_names[i], line_height) + 6, line_height + 6, Fade(YELLOW, 0.15f));
            DrawLineEx((Vector2){(float)tab_start_x, (float)tab_underline_y + 2}, (Vector2){(float)tab_start_x + MeasureText(tab_display_names[i], line_height), (float)tab_underline_y + 2}, 2.0f, YELLOW);
        }
        tab_start_x += MeasureText(tab_display_names[i], line_height) + 25;
    }
    DrawLineEx((Vector2){panel_bounds.x + 5, (float)tab_underline_y + 4}, (Vector2){panel_bounds.x + panel_bounds.width - 5, (float)tab_underline_y + 4}, 1.0f, Fade(LIGHTGRAY, 0.5f));
    current_y = tab_underline_y + 15;

    switch ((InventoryTabType)player->current_inventory_tab) {
        case TAB_INVENTORY:
            for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
                if (current_y + line_height > panel_bounds.y + panel_bounds.height - 5) break;
                if (strlen(player->inventory[i].name) > 0) {
                    sprintf(buffer, "- %s (%d)", player->inventory[i].name, player->inventory[i].quantity);
                    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, WHITE);
                } else {
                    DrawText("- Vazio -", panel_bounds.x + text_padding_x, current_y, line_height, DARKGRAY);
                }
                current_y += line_height + 5;
            }
            break;
        case TAB_EQUIPMENT:
            if (current_y + line_height <= panel_bounds.y + panel_bounds.height - 5) DrawText("Arma: <Nenhuma>", panel_bounds.x + text_padding_x, current_y, line_height, WHITE);
            current_y += line_height + 5;
            if (current_y + line_height <= panel_bounds.y + panel_bounds.height - 5) DrawText("Armadura: <Nenhuma>", panel_bounds.x + text_padding_x, current_y, line_height, WHITE);
            current_y += line_height + 5;
            if (current_y + line_height <= panel_bounds.y + panel_bounds.height - 5) DrawText("Acessório: <Nenhum>", panel_bounds.x + text_padding_x, current_y, line_height, WHITE);
            break;
        case TAB_STATUS:
            {
                char class_name_str[20] = "N/A";
                switch(player->classe) {
                    case GUERREIRO: strcpy(class_name_str, "Guerreiro"); break;
                    case MAGO: strcpy(class_name_str, "Mago"); break;
                    case ARQUEIRO: strcpy(class_name_str, "Arqueiro"); break;
                }
                int stat_y = current_y;

                sprintf(buffer, "Nome: %s", player->nome);
                if (stat_y + line_height <= panel_bounds.y + panel_bounds.height - 5) {
                    DrawText(buffer, panel_bounds.x + text_padding_x, stat_y, line_height, WHITE);
                    stat_y += line_height + 2;
                }
                sprintf(buffer, "Classe: %s", class_name_str);
                if (stat_y + line_height <= panel_bounds.y + panel_bounds.height - 5) {
                    DrawText(buffer, panel_bounds.x + text_padding_x, stat_y, line_height, WHITE);
                    stat_y += line_height + 2;
                }
                sprintf(buffer, "Nível: %d (EXP: %d)", player->nivel, player->exp); // Adicionado EXP
                if (stat_y + line_height <= panel_bounds.y + panel_bounds.height - 5) {
                    DrawText(buffer, panel_bounds.x + text_padding_x, stat_y, line_height, WHITE);
                    stat_y += line_height + 8;
                }
                sprintf(buffer, "Ataque: %d", player->ataque);
                 if (stat_y + line_height <= panel_bounds.y + panel_bounds.height - 5) {
                    DrawText(buffer, panel_bounds.x + text_padding_x, stat_y, line_height, ORANGE);
                    stat_y += line_height + 2;
                }
                sprintf(buffer, "Defesa: %d", player->defesa);
                if (stat_y + line_height <= panel_bounds.y + panel_bounds.height - 5) {
                    DrawText(buffer, panel_bounds.x + text_padding_x, stat_y, line_height, SKYBLUE);
                }
            }
            break;
        default:
            DrawText("Aba selecionada inválida.", panel_bounds.x + text_padding_x, current_y, line_height, RED);
            break;
    }
}

/**
 * @brief Desenha a tela de inventário principal.
 */
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY) {
    DrawPlayingScreen(background_players_arr, background_musicVolume, background_currentMusicIndex, background_musicIsPlaying, background_mapX, background_mapY);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.90f));

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int top_instruction_area = 35;
    int bottom_instruction_area = 35;
    int panel_area_y = top_instruction_area;
    int panel_area_height = screenHeight - top_instruction_area - bottom_instruction_area;
    int padding = 10;

    const char *main_title = "PAINEL DOS JOGADORES";
    DrawText(main_title, (screenWidth - MeasureText(main_title, 20)) / 2, 8, 20, RAYWHITE);

    if (MAX_PLAYERS > 0 && players_arr != NULL) {
        if (MAX_PLAYERS == 1) {
            Rectangle panel_p1_bounds = {(float)padding, (float)panel_area_y, (float)screenWidth - 2 * padding, (float)panel_area_height};
            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);
        } else { 
            float panel_width = ((float)screenWidth - 3 * padding) / 2;
            Rectangle panel_p1_bounds = {(float)padding, (float)panel_area_y, panel_width, (float)panel_area_height};
            Rectangle panel_p2_bounds = {(float)padding * 2 + panel_width, (float)panel_area_y, panel_width, (float)panel_area_height};

            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);

            if (MAX_PLAYERS > 1) { // Desenha o segundo painel apenas se houver mais de um jogador
                DrawRectangleRoundedLinesEx(panel_p2_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
                DrawRectangleRec(panel_p2_bounds, Fade(DARKGRAY, 0.6f));
                DrawPlayerPanelContent(&players_arr[1], panel_p2_bounds);
            }
        }
    }

        const char* closeInstruction = "Pressione 'E' para fechar"; // Alterado
    const char* p1NavInstruction = "P1 Abas: A / D";
    const char* p2NavInstruction = "P2 Abas: Setas Esq / Dir";
    int closeWidth = MeasureText(closeInstruction, 10);

    DrawText(closeInstruction, (screenWidth - closeWidth)/2 , screenHeight - 25, 10, WHITE);
    if (MAX_PLAYERS > 0) DrawText(p1NavInstruction, padding, screenHeight - 25, 10, YELLOW);
    if (MAX_PLAYERS > 1) DrawText(p2NavInstruction, screenWidth - MeasureText(p2NavInstruction, 10) - padding, screenHeight - 25, 10, YELLOW);
}

#include "../include/Inventory.h"
#include "../include/Game.h"
#include "../include/Classes.h"
#include <string.h>
#include <stdio.h>
#include <raylib.h>

// --- Variáveis Globais Externas ---
// Dimensões da tela virtual (declaradas em main.c)
extern const int virtualScreenWidth;
extern const int virtualScreenHeight;

// --- Variáveis Estáticas da Tela de Inventário ---
// Guarda se a música estava tocando quando o inventário foi aberto, para poder retomá-la.
static int music_was_playing_on_inventory_entry = 0;
// Flag para controlar a primeira chamada de UpdateInventoryScreen e realizar ações de entrada.
static bool first_inventory_update_call = true;

// Nomes de exibição para as abas do inventário.
const char *tab_display_names[MAX_INVENTORY_TABS] = {"Inventário", "Equipamento", "Status"};

// --- Funções de Gerenciamento do Inventário ---
/**
 * @brief Adiciona um item ao inventário de um jogador específico.
 * Se o item já existir, incrementa a quantidade. Se não, adiciona a um slot vazio.
 * @param player Ponteiro para o jogador.
 * @param itemName Nome do item a ser adicionado.
 * @param quantity Quantidade a ser adicionada (deve ser > 0).
 * @return true se o item foi adicionado/empilhado, false caso contrário (ex: inventário cheio).
 */
bool AddItemToInventory(Player *player, const char *itemName, int quantity) {
    if (!player) { return false; } // Validação: jogador nulo
    if (!itemName || strlen(itemName) == 0) { return false; } // Validação: nome do item inválido
    if (quantity <= 0) { return false; } // Validação: quantidade inválida

    // Tenta encontrar o item no inventário para empilhar
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) { // Item encontrado
            player->inventory[i].quantity += quantity; // Incrementa quantidade
            return true; // Sucesso
        }
    }
    // Se o item não foi encontrado, tenta adicioná-lo a um slot vazio
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strlen(player->inventory[i].name) == 0) { // Slot vazio encontrado
            strncpy(player->inventory[i].name, itemName, MAX_ITEM_NAME_LENGTH - 1); // Copia nome
            player->inventory[i].name[MAX_ITEM_NAME_LENGTH - 1] = '\0'; // Garante terminação nula
            player->inventory[i].quantity = quantity; // Define quantidade

            // Atualiza a contagem de tipos de itens diferentes no inventário
            int occupied_slots = 0;
            for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
            player->inventory_item_count = occupied_slots;
            return true; // Sucesso
        }
    }
    return false; // Falha (inventário cheio e item não encontrado para empilhar)
}

/**
 * @brief Remove uma certa quantidade de um item do inventário de um jogador.
 * @param player Ponteiro para o jogador.
 * @param itemName Nome do item a ser removido.
 * @param quantity Quantidade a ser removida (deve ser > 0).
 * @return true se removido com sucesso, false caso contrário (item não encontrado ou quantidade insuficiente).
 */
bool RemoveItemFromInventory(Player *player, const char *itemName, int quantity) {
    if (!player) { return false;} // Validação: jogador nulo
    if (!itemName || strlen(itemName) == 0) { return false;} // Validação: nome do item inválido
    if (quantity <= 0) { return false;} // Validação: quantidade inválida

    // Procura o item no inventário
    for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
        if (strcmp(player->inventory[i].name, itemName) == 0) { // Item encontrado
            if (player->inventory[i].quantity >= quantity) { // Verifica se há quantidade suficiente
                player->inventory[i].quantity -= quantity; // Decrementa quantidade
                if (player->inventory[i].quantity == 0) { // Se a quantidade zerar, limpa o slot
                    strcpy(player->inventory[i].name, ""); // Limpa nome do item
                    // Atualiza contagem de itens
                    int occupied_slots = 0;
                    for(int j=0; j < MAX_INVENTORY_SLOTS; j++){ if(strlen(player->inventory[j].name) > 0) occupied_slots++; }
                    player->inventory_item_count = occupied_slots;
                }
                return true; // Sucesso
            } else {
                return false; // Quantidade insuficiente para remover
            }
        }
    }
    return false; // Item não encontrado
}

// --- Lógica de Atualização da Tela de Inventário ---
/**
 * @brief Atualiza a lógica da tela de inventário.
 * Lida com entrada do usuário para fechar a tela, navegar entre abas e gerenciar música.
 * @param currentScreen_ptr Ponteiro para o estado atual do jogo.
 * @param players Array de jogadores.
 * @param musicIsPlaying_ptr Ponteiro para a flag de estado da música.
 * @param playlist Array de músicas.
 * @param currentMusicIndex_ptr Ponteiro para o índice da música atual.
 */
void UpdateInventoryScreen(GameState *currentScreen_ptr, Player players[], int *musicIsPlaying_ptr, Music playlist[], int *currentMusicIndex_ptr) {
    // Ações a serem executadas apenas na primeira vez que UpdateInventoryScreen é chamada após entrar na tela
    if (first_inventory_update_call) {
        music_was_playing_on_inventory_entry = *musicIsPlaying_ptr; // Salva o estado da música
        // Pausa a música se estava tocando
        if (music_was_playing_on_inventory_entry && playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
            PauseMusicStream(playlist[*currentMusicIndex_ptr]);
            *musicIsPlaying_ptr = 0; // Atualiza flag
        }
        // Garante que a aba inicial de cada jogador seja válida
        for (int i = 0; i < MAX_PLAYERS; i++) {
             if (players[i].current_inventory_tab < 0 || players[i].current_inventory_tab >= MAX_INVENTORY_TABS) {
                 players[i].current_inventory_tab = TAB_INVENTORY; // Define para a aba padrão (Inventário)
             }
        }
        first_inventory_update_call = false; // Marca que as ações de entrada foram executadas
    }

    // Tecla 'E' ou 'ESC' para fechar o inventário e voltar para a tela de jogo
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE)) {
        *currentScreen_ptr = GAMESTATE_PLAYING; // Muda o estado do jogo
        // Retoma a música se estava tocando antes
        if (music_was_playing_on_inventory_entry && playlist[*currentMusicIndex_ptr].stream.buffer != NULL) {
            ResumeMusicStream(playlist[*currentMusicIndex_ptr]);
            *musicIsPlaying_ptr = 1; // Atualiza flag
        }
        first_inventory_update_call = true; // Reseta para a próxima vez que entrar no inventário
        music_was_playing_on_inventory_entry = 0; // Reseta estado salvo da música
        return; // Sai da função de atualização
    }

    // Navegação entre abas para Jogador 1 (P1) com teclas 'A' (anterior) e 'D' (próxima)
    if (MAX_PLAYERS > 0) {
        if (IsKeyPressed(KEY_A)) { // Aba anterior
            players[0].current_inventory_tab = (players[0].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        }
        if (IsKeyPressed(KEY_D)) { // Próxima aba
            players[0].current_inventory_tab = (players[0].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
        }
    }

    // Navegação entre abas para Jogador 2 (P2) com Setas Esquerda/Direita
    if (MAX_PLAYERS > 1) {
        if (IsKeyPressed(KEY_LEFT)) { // Aba anterior
            players[1].current_inventory_tab = (players[1].current_inventory_tab - 1 + MAX_INVENTORY_TABS) % MAX_INVENTORY_TABS;
        }
        if (IsKeyPressed(KEY_RIGHT)) { // Próxima aba
            players[1].current_inventory_tab = (players[1].current_inventory_tab + 1) % MAX_INVENTORY_TABS;
        }
    }
}

// --- Funções de Desenho da Tela de Inventário ---
/**
 * @brief Desenha o conteúdo do painel individual de um jogador (status, inventário, equipamento).
 * O layout agora usa as dimensões do painel (panel_bounds) que são calculadas
 * com base em virtualScreenWidth/Height na função DrawInventoryScreen.
 * @param player Ponteiro para o jogador cujos dados serão exibidos.
 * @param panel_bounds Retângulo definindo a área do painel deste jogador.
 */
void DrawPlayerPanelContent(Player *player, Rectangle panel_bounds) {
    char buffer[128]; // Buffer para formatar strings de texto
    int line_height = 18; // Altura da linha de texto (ajustada para caber mais informações)
    int text_padding_x = 10; // Preenchimento horizontal para o texto dentro do painel
    int current_y = panel_bounds.y + 10; // Posição Y inicial para o desenho do conteúdo

    // Exibe HP (Vida)
    sprintf(buffer, "HP: %d / %d", player->vida, player->max_vida);
    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, (player->vida < player->max_vida * 0.3f) ? RED : LIME); // Cor muda se HP baixo
    current_y += line_height + 1; // Avança para a próxima linha

    // Exibe MP (Mana)
    sprintf(buffer, "MP: %d / %d", player->mana, player->max_mana);
    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, SKYBLUE);
    current_y += line_height + 1;

    // Exibe Moedas
    sprintf(buffer, "Moedas: %d", player->moedas);
    DrawText(buffer, panel_bounds.x + text_padding_x, current_y, line_height, GOLD);
    current_y += line_height + 8; // Espaçamento maior antes das abas

    // Desenha as Abas (Inventário, Equipamento, Status)
    int tab_start_x = panel_bounds.x + text_padding_x; // Posição X inicial para a primeira aba
    int tab_text_y = current_y; // Posição Y para o texto das abas
    for (int i = 0; i < MAX_INVENTORY_TABS; i++) {
        // Define a cor da aba (amarelo para selecionada, cinza para não selecionada)
        Color tab_color = (player->current_inventory_tab == i) ? YELLOW : GRAY;
        DrawText(tab_display_names[i], tab_start_x, tab_text_y, line_height, tab_color);
        if (player->current_inventory_tab == i) { // Se esta aba está selecionada
            // Desenha um retângulo de fundo sutil e uma linha de destaque sob a aba selecionada
            DrawRectangle((int)(tab_start_x - 3), (int)(tab_text_y - 3), MeasureText(tab_display_names[i], line_height) + 6, line_height + 6, Fade(YELLOW, 0.15f));
            DrawLineEx((Vector2){(float)tab_start_x, (float)(tab_text_y + line_height + 2)}, (Vector2){(float)tab_start_x + MeasureText(tab_display_names[i], line_height), (float)(tab_text_y + line_height + 2)}, 2.0f, YELLOW);
        }
        tab_start_x += MeasureText(tab_display_names[i], line_height) + 20; // Avança X para a próxima aba
    }
    // Linha divisória abaixo das abas
    DrawLineEx((Vector2){panel_bounds.x + 5, (float)(tab_text_y + line_height + 4)}, (Vector2){panel_bounds.x + panel_bounds.width - 5, (float)(tab_text_y + line_height + 4)}, 1.0f, Fade(LIGHTGRAY, 0.5f));
    current_y = tab_text_y + line_height + 12; // Atualiza Y para o conteúdo abaixo das abas

    // Coordenadas para layout de duas colunas na aba Status
    int col1_x = panel_bounds.x + text_padding_x;
    // Calcula a posição da segunda coluna para que haja algum espaço, mas não muito
    int col2_x = panel_bounds.x + text_padding_x + (int)(panel_bounds.width / 2.0f) - (text_padding_x /2) ;
    int initial_content_y = current_y; // Y inicial para o conteúdo das abas

    // Desenha o conteúdo da aba atualmente selecionada
    switch ((InventoryTabType)player->current_inventory_tab) {
        case TAB_INVENTORY: // Aba de Inventário de Itens
            for (int i = 0; i < MAX_INVENTORY_SLOTS; i++) {
                // Para de listar se exceder a altura do painel
                if (current_y + line_height > panel_bounds.y + panel_bounds.height - 5) break;
                if (strlen(player->inventory[i].name) > 0) { // Se o slot contém um item
                    sprintf(buffer, "- %s (%d)", player->inventory[i].name, player->inventory[i].quantity);
                    DrawText(buffer, col1_x, current_y, line_height, WHITE);
                } else { // Slot vazio
                    DrawText("- Vazio -", col1_x, current_y, line_height, DARKGRAY);
                }
                current_y += line_height + 3; // Próxima linha
            }
            break;
        case TAB_EQUIPMENT: // Aba de Equipamentos
            sprintf(buffer, "Arma: %s", strlen(player->equipped_items[EQUIP_SLOT_WEAPON].name) > 0 ? player->equipped_items[EQUIP_SLOT_WEAPON].name : "<Nenhuma>");
            DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Armadura: %s", strlen(player->equipped_items[EQUIP_SLOT_ARMOR].name) > 0 ? player->equipped_items[EQUIP_SLOT_ARMOR].name : "<Nenhuma>");
            DrawText(buffer, col1_x, current_y, line_height, WHITE); current_y += line_height + 3;
            sprintf(buffer, "Acessório: %s", strlen(player->equipped_items[EQUIP_SLOT_ACCESSORY].name) > 0 ? player->equipped_items[EQUIP_SLOT_ACCESSORY].name : "<Nenhum>");
            DrawText(buffer, col1_x, current_y, line_height, WHITE);
            break;
        case TAB_STATUS: // Aba de Status do Jogador
            {
                int stat_y_col1 = initial_content_y; // Y para a primeira coluna de status
                int stat_y_col2 = initial_content_y; // Y para a segunda coluna de status

                char class_name_str[30] = "Desconhecida"; // Nome da classe em string
                switch(player->classe) { // Converte enum Classe para string
                    case GUERREIRO: strcpy(class_name_str, "Guerreiro"); break;
                    case MAGO:    strcpy(class_name_str, "Mago");    break;
                    case ARQUEIRO:  strcpy(class_name_str, "Arqueiro");  break;
                    case BARBARO: strcpy(class_name_str, "Bárbaro"); break;
                    case LADINO:  strcpy(class_name_str, "Ladino");  break;
                    case CLERIGO: strcpy(class_name_str, "Clérigo"); break;
                }
                // Informações básicas do jogador
                sprintf(buffer, "Nome: %s", player->nome);
                DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Classe: %s", class_name_str);
                DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Nível: %d (EXP: %d)", player->nivel, player->exp);
                DrawText(buffer, col1_x, stat_y_col1, line_height, WHITE); stat_y_col1 += line_height + 6;

                // Atributos de combate
                sprintf(buffer, "Ataque: %d", player->ataque);
                DrawText(buffer, col1_x, stat_y_col1, line_height, ORANGE); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Defesa: %d", player->defesa);
                DrawText(buffer, col1_x, stat_y_col1, line_height, SKYBLUE); stat_y_col1 += line_height + 6;

                // Atributos S.P.E.C.I.A.L. em duas colunas
                DrawText("ATRIBUTOS:", col1_x, stat_y_col1, line_height, YELLOW);
                // A segunda coluna começa na mesma altura do título "ATRIBUTOS"
                stat_y_col2 = stat_y_col1;
                stat_y_col1 += line_height + 3; // Avança Y para os atributos da primeira coluna
                stat_y_col2 += line_height + 3; // Avança Y para os atributos da segunda coluna

                sprintf(buffer, "Força: %d", player->forca);
                DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Inteligência: %d", player->inteligencia); // Coluna 2
                DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;

                sprintf(buffer, "Percepção: %d", player->percepcao);
                DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Agilidade: %d", player->agilidade); // Coluna 2
                DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;

                sprintf(buffer, "Resistência: %d", player->resistencia);
                DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY); stat_y_col1 += line_height + 1;
                sprintf(buffer, "Sorte: %d", player->sorte); // Coluna 2
                DrawText(buffer, col2_x, stat_y_col2, line_height, LIGHTGRAY); stat_y_col2 += line_height + 1;

                sprintf(buffer, "Carisma: %d", player->carisma);
                DrawText(buffer, col1_x, stat_y_col1, line_height, LIGHTGRAY);
                // (Não há mais status para a segunda coluna neste layout)
            }
            break;
        default: // Caso de aba inválida (não deve acontecer)
            DrawText("Aba selecionada inválida.", col1_x, current_y, line_height, RED);
            break;
    }
}

/**
 * @brief Desenha a tela de inventário completa.
 * O layout dos painéis agora usa virtualScreenWidth/Height.
 * @param players_arr Array de jogadores cujos painéis serão exibidos.
 * @param background_players_arr Array de jogadores para desenhar o fundo (tela de jogo).
 * @param background_musicVolume Volume da música para o fundo.
 * @param background_currentMusicIndex Índice da música para o fundo.
 * @param background_musicIsPlaying Estado da música para o fundo.
 * @param background_mapX Coordenada X do mapa para o fundo.
 * @param background_mapY Coordenada Y do mapa para o fundo.
 */
void DrawInventoryScreen(Player players_arr[], Player background_players_arr[], float background_musicVolume, int background_currentMusicIndex, int background_musicIsPlaying, int background_mapX, int background_mapY) {
    // 1. Desenha a tela de jogo como fundo (escurecida)
    // DrawPlayingScreen já desenha na tela virtual.
    DrawPlayingScreen(background_players_arr, background_musicVolume, background_currentMusicIndex, background_musicIsPlaying, background_mapX, background_mapY);
    // Aplica um filtro escuro sobre a tela de jogo
    DrawRectangle(0, 0, virtualScreenWidth, virtualScreenHeight, Fade(BLACK, 0.90f));

    // --- Layout dos Painéis do Inventário (usando dimensões virtuais) ---
    // Alturas reservadas para títulos e instruções
    int top_instruction_area_height = 35;
    int bottom_instruction_area_height = 35;
    // Área útil para os painéis dos jogadores
    int panel_area_y_start = top_instruction_area_height;
    int panel_area_height = virtualScreenHeight - top_instruction_area_height - bottom_instruction_area_height;
    int panel_padding = 10; // Espaçamento entre painéis e bordas da tela virtual

    // Título principal da tela de inventário
    const char *main_title = "PAINEL DOS JOGADORES";
    DrawText(main_title, (virtualScreenWidth - MeasureText(main_title, 20)) / 2, 8, 20, RAYWHITE);

    // Desenha os painéis para cada jogador
    if (MAX_PLAYERS > 0 && players_arr != NULL) {
        if (MAX_PLAYERS == 1) { // Se houver apenas um jogador, o painel ocupa quase toda a largura
            Rectangle panel_p1_bounds = {
                (float)panel_padding,
                (float)panel_area_y_start,
                (float)virtualScreenWidth - 2 * panel_padding, // Largura do painel
                (float)panel_area_height                       // Altura do painel
            };
            // Desenha bordas e fundo do painel
            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            // Desenha o conteúdo do painel (status, inventário, etc.)
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);
        } else { // Se houver dois jogadores, divide a tela em dois painéis lado a lado
            float panel_width = ((float)virtualScreenWidth - 3 * panel_padding) / 2; // Largura de cada painel
            // Painel do Jogador 1 (P1)
            Rectangle panel_p1_bounds = {
                (float)panel_padding,
                (float)panel_area_y_start,
                panel_width,
                (float)panel_area_height
            };
            DrawRectangleRoundedLinesEx(panel_p1_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
            DrawRectangleRec(panel_p1_bounds, Fade(DARKGRAY, 0.6f));
            DrawPlayerPanelContent(&players_arr[0], panel_p1_bounds);

            // Painel do Jogador 2 (P2)
            if (MAX_PLAYERS > 1) {
                Rectangle panel_p2_bounds = {
                    (float)panel_padding * 2 + panel_width, // Posição X do segundo painel
                    (float)panel_area_y_start,
                    panel_width,
                    (float)panel_area_height
                };
                DrawRectangleRoundedLinesEx(panel_p2_bounds, 0.05f, 12, 2.0f, Fade(LIGHTGRAY, 0.7f));
                DrawRectangleRec(panel_p2_bounds, Fade(DARKGRAY, 0.6f));
                DrawPlayerPanelContent(&players_arr[1], panel_p2_bounds);
            }
        }
    }

    // --- Instruções na Parte Inferior da Tela ---
    // Posicionadas em relação à virtualScreenHeight.
    const char* closeInstruction = "Pressione 'E' ou 'ESC' para fechar";
    const char* p1NavInstruction = "P1 Abas: A / D";
    const char* p2NavInstruction = "P2 Abas: Setas Esq / Dir";
    int closeWidth = MeasureText(closeInstruction, 10);

    DrawText(closeInstruction, (virtualScreenWidth - closeWidth)/2 , virtualScreenHeight - 25, 10, WHITE);
    if (MAX_PLAYERS > 0) DrawText(p1NavInstruction, panel_padding, virtualScreenHeight - 25, 10, YELLOW);
    if (MAX_PLAYERS > 1) DrawText(p2NavInstruction, virtualScreenWidth - MeasureText(p2NavInstruction, 10) - panel_padding, virtualScreenHeight - 25, 10, YELLOW);
}
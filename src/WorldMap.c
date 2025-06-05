#include "../include/WorldMap.h" // Inclui o header correspondente, que por sua vez inclui Game.h
#include <math.h>                // Para fmaxf (e outras funções matemáticas se necessárias)
// <stdio.h> e <string.h> removidos pois não são diretamente usados neste arquivo.
// Raylib (via Game.h ou WorldMap.h) provê TraceLog se necessário.

// --- Variáveis Globais Externas (definidas em main.c, extern em Game.h) ---
// Estas são acessadas através de Game.h, que é incluído por WorldMap.h
extern const int WORLD_MAP_MIN_X;
extern const int WORLD_MAP_MAX_X;
extern const int WORLD_MAP_MIN_Y;
extern const int WORLD_MAP_MAX_Y;

/**
 * @brief Função auxiliar interna para determinar em qual borda da seção atual um jogador está.
 * Utiliza as dimensões reais da seção e uma tolerância para a detecção.
 * @param player_obj O jogador a ser verificado.
 * @param sectionWidth A largura real da seção atual do mapa.
 * @param sectionHeight A altura real da seção atual do mapa.
 * @return BorderDirection O enum indicando a borda, ou BORDER_NONE.
 */
static BorderDirection GetPlayerBorderCondition_Internal(Player player_obj, float sectionWidth, float sectionHeight) {
    int tolerance = 10; // Margem em pixels para considerar que o jogador está na borda

    // Casts explícitos para (float) para evitar warnings em comparações int com float,
    // embora a promoção automática geralmente funcione, ser explícito é mais seguro.
    if ((float)player_obj.posx <= (float)tolerance) return BORDER_LEFT;
    if ((float)player_obj.posx >= sectionWidth - (float)player_obj.width - (float)tolerance) return BORDER_RIGHT;
    if ((float)player_obj.posy <= (float)tolerance) return BORDER_TOP;
    if ((float)player_obj.posy >= sectionHeight - (float)player_obj.height - (float)tolerance) return BORDER_BOTTOM;

    return BORDER_NONE;
}

/**
 * @brief Verifica e processa transições de mapa com base na posição dos jogadores,
 * modo de jogo e dimensões reais da seção atual.
 * @param players Array de jogadores.
 * @param numActivePlayers Número de jogadores ativos.
 * @param currentMapX_ptr Ponteiro para a coordenada X global do mapa (será atualizada em caso de transição).
 * @param currentMapY_ptr Ponteiro para a coordenada Y global do mapa (será atualizada em caso de transição).
 * @param gameMode O modo de jogo atual.
 * @param sectionActualWidth A largura real da seção do mapa carregada atualmente.
 * @param sectionActualHeight A altura real da seção do mapa carregada atualmente.
 * @return true se uma transição de mapa ocorreu, false caso contrário.
 */
bool WorldMap_CheckTransition(Player players[], int numActivePlayers, int *currentMapX_ptr, int *currentMapY_ptr, GameModeType gameMode, float sectionActualWidth, float sectionActualHeight) {
    if (!players || !currentMapX_ptr || !currentMapY_ptr) return false;
    // Se as dimensões da seção forem inválidas (ex: não carregadas), não tenta transição.
    if (sectionActualWidth <= 0 || sectionActualHeight <= 0) return false;

    bool transition_occurred = false;

    // Lógica para Single Player
    if (gameMode == GAME_MODE_SINGLE_PLAYER) {
        if (numActivePlayers > 0) {
            Player *player = &players[0]; // Jogador principal
            BorderDirection player_border = GetPlayerBorderCondition_Internal(*player, sectionActualWidth, sectionActualHeight);

            if (player_border != BORDER_NONE) {
                int nextMapX = *currentMapX_ptr;
                int nextMapY = *currentMapY_ptr;

                // Calcula as próximas coordenadas do mapa
                switch (player_border) {
                    case BORDER_LEFT:   nextMapX--; break;
                    case BORDER_RIGHT:  nextMapX++; break;
                    case BORDER_TOP:    nextMapY--; break;
                    case BORDER_BOTTOM: nextMapY++; break;
                    default: break; // Não deve acontecer
                }

                // Verifica se as novas coordenadas estão dentro dos limites do mundo
                if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X &&
                    nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {

                    *currentMapX_ptr = nextMapX; // Atualiza mapa global
                    *currentMapY_ptr = nextMapY;
                    transition_occurred = true; // Sinaliza que o mapa mudou, main.c recarregará a seção

                    // Reposiciona o jogador para o lado oposto da nova seção
                    // Casts para float para garantir aritmética de ponto flutuante correta antes da conversão para int.
                    float p_width_f = (float)player->width;
                    float p_height_f = (float)player->height;

                    switch (player_border) {
                        case BORDER_RIGHT:  player->posx = (int)(p_width_f / 2.0f); break;
                        case BORDER_LEFT:   player->posx = (int)(sectionActualWidth - p_width_f - (p_width_f / 2.0f)); break;
                        case BORDER_TOP:    player->posy = (int)(sectionActualHeight - p_height_f - (p_height_f / 2.0f)); break;
                        case BORDER_BOTTOM: player->posy = (int)(p_height_f / 2.0f); break;
                        default: break;
                    }
                    // Clamp final da posição do jogador dentro da nova (mesma dimensão por enquanto) seção
                    if (player->posx < 0) player->posx = 0;
                    if ((float)player->posx > sectionActualWidth - p_width_f) player->posx = (int)(sectionActualWidth - p_width_f);
                    if (player->posy < 0) player->posy = 0;
                    if ((float)player->posy > sectionActualHeight - p_height_f) player->posy = (int)(sectionActualHeight - p_height_f);
                }
            }
        }
    }
    // Lógica para Two Player (Local)
    else if (gameMode == GAME_MODE_TWO_PLAYER && numActivePlayers >= 2) {
        BorderDirection p1_border = GetPlayerBorderCondition_Internal(players[0], sectionActualWidth, sectionActualHeight);
        BorderDirection p2_border = GetPlayerBorderCondition_Internal(players[1], sectionActualWidth, sectionActualHeight);

        if (p1_border != BORDER_NONE && p1_border == p2_border) { // Ambos na mesma borda
            int nextMapX = *currentMapX_ptr;
            int nextMapY = *currentMapY_ptr;
            switch (p1_border) { /* ... (calcula nextMapX/Y como no single player) ... */
                case BORDER_LEFT:   nextMapX--; break; case BORDER_RIGHT:  nextMapX++; break;
                case BORDER_TOP:    nextMapY--; break; case BORDER_BOTTOM: nextMapY++; break;
                default: break;
            }

            if (nextMapX >= WORLD_MAP_MIN_X && nextMapX <= WORLD_MAP_MAX_X &&
                nextMapY >= WORLD_MAP_MIN_Y && nextMapY <= WORLD_MAP_MAX_Y) {
                *currentMapX_ptr = nextMapX;
                *currentMapY_ptr = nextMapY;
                transition_occurred = true;

                for (int i = 0; i < numActivePlayers && i < MAX_PLAYERS_SUPPORTED; i++) { // Processa ambos os jogadores
                    Player *player = &players[i];
                    float p_width_f = (float)player->width;
                    float p_height_f = (float)player->height;

                    switch (p1_border) {
                        case BORDER_RIGHT:  player->posx = (int)(p_width_f / 2.0f); break;
                        case BORDER_LEFT:   player->posx = (int)(sectionActualWidth - p_width_f - (p_width_f / 2.0f)); break;
                        case BORDER_TOP:    player->posy = (int)(sectionActualHeight - p_height_f - (p_height_f / 2.0f)); break;
                        case BORDER_BOTTOM: player->posy = (int)(p_height_f / 2.0f); break;
                        default: break;
                    }
                    if (player->posx < 0) player->posx = 0;
                    if ((float)player->posx > sectionActualWidth - p_width_f) player->posx = (int)(sectionActualWidth - p_width_f);
                    if (player->posy < 0) player->posy = 0;
                    if ((float)player->posy > sectionActualHeight - p_height_f) player->posy = (int)(sectionActualHeight - p_height_f);
                }
            }
        }
    }
    return transition_occurred;
}
#ifndef BATTLE_UI_H
#define BATTLE_UI_H

#include "raylib.h"

/**
 * @brief Inicializa ou reinicia o estado da UI de batalha.
 * Chamada no início de cada batalha.
 */
void BattleUI_Init(void);

/**
 * @brief Atualiza a lógica da UI de batalha.
 * Lida com a entrada do teclado para navegação nos menus.
 */
void BattleUI_Update(void);

/**
 * @brief Desenha todos os elementos da UI de batalha.
 * Desenha caixas de status, menus, mensagens, etc.
 */
void BattleUI_Draw(void);

#endif // BATTLE_UI_H

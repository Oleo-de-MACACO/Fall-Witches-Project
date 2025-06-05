#ifndef SINGLEPLAYER_H
#define SINGLEPLAYER_H

#include "Classes.h"      // Para Player
#include "WorldLoading.h" // Para WorldSection

// *** Assinatura ATUALIZADA para incluir activeSection ***
void SinglePlayer_HandleMovement(Player *player, const WorldSection* activeSection);

#endif // SINGLEPLAYER_H
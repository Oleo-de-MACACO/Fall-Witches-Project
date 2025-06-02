#include "../include/SaveLoad.h"
#include "../include/Classes.h" 
#include <stdio.h>
#include "raylib.h" 

const unsigned int SAVEGAME_FILE_VERSION = 5;

bool EnsureSavesDirectoryExists(void) {
    if (!DirectoryExists(SAVE_DIRECTORY)) {
        TraceLog(LOG_WARNING, "Diretorio de saves '%s' NAO existe. Crie-o manualmente.", SAVE_DIRECTORY); // Corrigido para ASCII
        return false; 
    }
    return true;
}

void GetSaveFileName(char *buffer, int buffer_size, int slot_number) {
    snprintf(buffer, buffer_size, "%s/Save%d.sav", SAVE_DIRECTORY, slot_number + 1);
}

bool DoesSaveSlotExist(int slot_number) {
    if (!DirectoryExists(SAVE_DIRECTORY)) return false; 
    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);
    return FileExists(filename);
}

bool SaveGame(Player players[], int num_players, int slot_number, int currentMapX, int currentMapY) {
    if (!EnsureSavesDirectoryExists()) { 
        TraceLog(LOG_ERROR, "Diretorio de saves '%s' nao encontrado. Impossivel salvar.", SAVE_DIRECTORY); // Corrigido para ASCII
        return false;
    }

    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);
    FILE *saveFile = fopen(filename, "wb"); 
    if (saveFile == NULL) {
        TraceLog(LOG_ERROR, "Falha ao abrir arquivo de save '%s' para escrita.", filename);
        return false;
    }
    size_t items_written;

    items_written = fwrite(&SAVEGAME_FILE_VERSION, sizeof(unsigned int), (size_t)1, saveFile); // Cast para size_t
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever versao do save."); fclose(saveFile); return false; } // Corrigido para ASCII
    items_written = fwrite(&currentMapX, sizeof(int), (size_t)1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapX."); fclose(saveFile); return false; }
    items_written = fwrite(&currentMapY, sizeof(int), (size_t)1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapY."); fclose(saveFile); return false; }
    items_written = fwrite(&num_players, sizeof(int), (size_t)1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever numero de jogadores."); fclose(saveFile); return false; } // Corrigido para ASCII

    for (int i = 0; i < num_players; i++) {
        items_written = fwrite(players[i].nome, sizeof(char), (size_t)MAX_PLAYER_NAME_LENGTH, saveFile); // Cast
        if (items_written != MAX_PLAYER_NAME_LENGTH) goto save_error_player;
        items_written = fwrite(&players[i].classe, sizeof(Classe), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].nivel, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].exp, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].vida, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].max_vida, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].mana, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].max_mana, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].stamina, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].max_stamina, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].magic_attack, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].magic_defense, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].ataque, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].defesa, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].forca, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].percepcao, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].resistencia, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].carisma, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].inteligencia, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].agilidade, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].sorte, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].moedas, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].posx, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].posy, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) {
            items_written = fwrite(&players[i].inventory[j], sizeof(InventoryItem), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        }
        items_written = fwrite(&players[i].inventory_item_count, sizeof(int), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) {
            items_written = fwrite(&players[i].equipped_items[j], sizeof(InventoryItem), (size_t)1, saveFile); if (items_written != 1) goto save_error_player;
        }
    }
    fclose(saveFile);
    TraceLog(LOG_INFO, "[SaveGame] Jogo salvo com sucesso em '%s'. (Versao: %u, Mapa X:%d, Y:%d)", filename, SAVEGAME_FILE_VERSION, currentMapX, currentMapY); // Corrigido para ASCII
    return true;
save_error_player: 
    TraceLog(LOG_ERROR, "[SaveGame] Erro durante a escrita dos dados do jogador no arquivo de save '%s'.", filename);
    fclose(saveFile); 
    return false;
}

bool LoadGame(Player players[], int num_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY) {
    // ... (LoadGame logic with (size_t)1 casts for fread where appropriate)
    if (!DirectoryExists(SAVE_DIRECTORY)){ 
         TraceLog(LOG_WARNING, "[LoadGame] Diretorio de saves '%s' nao encontrado. Impossivel carregar.", SAVE_DIRECTORY); // Corrigido para ASCII
         return false;
    }
    char filename[100]; GetSaveFileName(filename, sizeof(filename), slot_number);
    if (!FileExists(filename)) {
        TraceLog(LOG_WARNING, "[LoadGame] Arquivo de save '%s' nao existe para carregar.", filename);
        return false;
    }
    FILE *loadFile = fopen(filename, "rb"); 
    if (loadFile == NULL) {
        TraceLog(LOG_ERROR, "[LoadGame] Falha ao abrir arquivo de save '%s' para leitura.", filename);
        return false;
    }
    size_t items_read; unsigned int file_version = 0;
    items_read = fread(&file_version, sizeof(unsigned int), (size_t)1, loadFile); // Cast
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler a versao do arquivo de save '%s'.", filename); fclose(loadFile); return false; } // Corrigido para ASCII
    if (file_version != SAVEGAME_FILE_VERSION) { TraceLog(LOG_ERROR, "[LoadGame] Versao do arquivo de save ('%s' v%u) incompativel com a versao do jogo (v%u).", filename, file_version, SAVEGAME_FILE_VERSION); fclose(loadFile); return false; } // Corrigido para ASCII
    items_read = fread(loadedMapX, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapX de '%s'.", filename); fclose(loadFile); return false; }
    items_read = fread(loadedMapY, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapY de '%s'.", filename); fclose(loadFile); return false; }
    int num_players_saved = 0; items_read = fread(&num_players_saved, sizeof(int), (size_t)1, loadFile);
    if (items_read != 1 || num_players_saved > num_players_in_game || num_players_saved <= 0 ) { TraceLog(LOG_ERROR, "[LoadGame] Numero de jogadores em '%s' (%d) incompativel com o jogo (%d) ou erro de leitura.", filename, num_players_saved, num_players_in_game); fclose(loadFile); return false; } // Corrigido para ASCII
    for (int i = 0; i < num_players_saved; i++) {
        items_read = fread(players[i].nome, sizeof(char), (size_t)MAX_PLAYER_NAME_LENGTH, loadFile); if (items_read != MAX_PLAYER_NAME_LENGTH) goto load_error_player; // Cast
        players[i].nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0'; 
        items_read = fread(&players[i].classe, sizeof(Classe), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].nivel, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].exp, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].vida, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].max_vida, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].mana, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].max_mana, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].stamina, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].max_stamina, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].magic_attack, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].magic_defense, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].ataque, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].defesa, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].forca, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].percepcao, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].resistencia, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].carisma, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].inteligencia, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].agilidade, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].sorte, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].moedas, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].posx, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].posy, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) { items_read = fread(&players[i].inventory[j], sizeof(InventoryItem), (size_t)1, loadFile); if (items_read != 1) goto load_error_player; }
        items_read = fread(&players[i].inventory_item_count, sizeof(int), (size_t)1, loadFile); if (items_read != 1) goto load_error_player;
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) { items_read = fread(&players[i].equipped_items[j], sizeof(InventoryItem), (size_t)1, loadFile); if (items_read != 1) goto load_error_player; }
        if (players[i].vida > players[i].max_vida) players[i].vida = players[i].max_vida;
        if (players[i].mana > players[i].max_mana) players[i].mana = players[i].max_mana;
        if (players[i].stamina > players[i].max_stamina) players[i].stamina = players[i].max_stamina;
    }
    fclose(loadFile);
    TraceLog(LOG_INFO, "[LoadGame] Jogo carregado com sucesso de '%s'. (Versao: %u, Mapa X:%d, Y:%d)", filename, file_version, *loadedMapX, *loadedMapY); // Corrigido para ASCII
    return true;
load_error_player: 
    TraceLog(LOG_ERROR, "[LoadGame] Erro durante a leitura dos dados do jogador do arquivo '%s'.", filename);
    fclose(loadFile);
    return false;
}
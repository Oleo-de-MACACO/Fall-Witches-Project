#include "../include/SaveLoad.h"
#include "../include/Classes.h" 
#include <stdio.h>
#include <string.h> 
#include "raylib.h" 
#include <stddef.h> // For size_t and NULL

const unsigned int SAVEGAME_FILE_VERSION = 6; 

static void GetModeSubdirectoryFullPath(GameModeType gameMode, char* path_buffer, size_t buffer_size) {
    const char* subdir_name = (gameMode == GAME_MODE_SINGLE_PLAYER) ? SAVE_SUBDIR_SINGLEPLAYER : SAVE_SUBDIR_TWOPLAYER;
    snprintf(path_buffer, buffer_size, "%s/%s", SAVE_BASE_DIRECTORY, subdir_name);
}

bool EnsureSavesDirectoryExists(GameModeType gameMode) {
    if (!DirectoryExists(SAVE_BASE_DIRECTORY)) {
        TraceLog(LOG_WARNING, "Diretorio base de saves '%s' NAO existe. Crie manualmente.", SAVE_BASE_DIRECTORY);
        return false; 
    }
    char modeSavePath[128];
    GetModeSubdirectoryFullPath(gameMode, modeSavePath, sizeof(modeSavePath));
    if (!DirectoryExists(modeSavePath)) {
        TraceLog(LOG_WARNING, "Diretorio de saves para o modo '%s' ('%s') NAO existe. Crie manualmente.", 
            (gameMode == GAME_MODE_SINGLE_PLAYER ? SAVE_SUBDIR_SINGLEPLAYER : SAVE_SUBDIR_TWOPLAYER), modeSavePath);
        return false; 
    }
    return true;
}

void GetSaveFileName(char *buffer, int buffer_size, int slot_number, GameModeType gameMode) {
    char modePath[128];
    GetModeSubdirectoryFullPath(gameMode, modePath, sizeof(modePath));
    snprintf(buffer, buffer_size, "%s/Save%d.sav", modePath, slot_number + 1);
}

bool DoesSaveSlotExist(int slot_number, GameModeType gameMode) {
    char filename[256]; 
    GetSaveFileName(filename, sizeof(filename), slot_number, gameMode);
    return FileExists(filename);
}

bool SaveGame(Player players[], int num_players_to_save, int slot_number, int currentMapX, int currentMapY, GameModeType gameMode) {
    char filename[256];
    FILE *saveFile = NULL; 
    size_t items_written;

    if (!EnsureSavesDirectoryExists(gameMode)) { 
        TraceLog(LOG_ERROR, "Diretorio de saves para o modo '%s' nao encontrado/criado. Impossivel salvar.", 
            (gameMode == GAME_MODE_SINGLE_PLAYER ? SAVE_SUBDIR_SINGLEPLAYER : SAVE_SUBDIR_TWOPLAYER));
        return false;
    }
    GetSaveFileName(filename, sizeof(filename), slot_number, gameMode);
    saveFile = fopen(filename, "wb"); 
    if (saveFile == NULL) { 
        TraceLog(LOG_ERROR, "Falha ao abrir arquivo de save '%s' para escrita.", filename); 
        return false; 
    }
    
    items_written = fwrite(&SAVEGAME_FILE_VERSION, sizeof(unsigned int), (size_t)1, saveFile); 
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever versao do save."); if(saveFile){fclose(saveFile);} return false; }
    items_written = fwrite(&currentMapX, sizeof(int), (size_t)1, saveFile); 
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapX."); if(saveFile){fclose(saveFile);} return false; }
    items_written = fwrite(&currentMapY, sizeof(int), (size_t)1, saveFile); 
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapY."); if(saveFile){fclose(saveFile);} return false; }
    items_written = fwrite(&num_players_to_save, sizeof(int), (size_t)1, saveFile); 
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever num_players_to_save."); if(saveFile){fclose(saveFile);} return false; }

    for (int i = 0; i < num_players_to_save; i++) {
        items_written = fwrite(players[i].nome, sizeof(char), (size_t)MAX_PLAYER_NAME_LENGTH, saveFile); if (items_written != MAX_PLAYER_NAME_LENGTH) { TraceLog(LOG_ERROR, "SaveGame: Erro nome P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].classe, sizeof(Classe), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro classe P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].nivel, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro nivel P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].exp, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro exp P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].vida, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro vida P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].max_vida, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro max_vida P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].mana, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro mana P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].max_mana, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro max_mana P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].stamina, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro stamina P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].max_stamina, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro max_stamina P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].magic_attack, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro magic_attack P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].magic_defense, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro magic_defense P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].ataque, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro ataque P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].defesa, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro defesa P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].forca, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro forca P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].percepcao, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro percepcao P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].resistencia, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro resistencia P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].carisma, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro carisma P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].inteligencia, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro inteligencia P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].agilidade, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro agilidade P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].sorte, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro sorte P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].moedas, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro moedas P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].posx, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro posx P%d", i); if(saveFile){fclose(saveFile);} return false;}
        items_written = fwrite(&players[i].posy, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro posy P%d", i); if(saveFile){fclose(saveFile);} return false;}
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) { items_written = fwrite(&players[i].inventory[j], sizeof(InventoryItem), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro inventario P%d item %d", i, j); if(saveFile){fclose(saveFile);} return false;} }
        items_written = fwrite(&players[i].inventory_item_count, sizeof(int), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro contagem inventario P%d", i); if(saveFile){fclose(saveFile);} return false;}
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) { items_written = fwrite(&players[i].equipped_items[j], sizeof(InventoryItem), (size_t)1, saveFile); if (items_written != 1) { TraceLog(LOG_ERROR, "SaveGame: Erro equipamento P%d item %d", i, j); if(saveFile){fclose(saveFile);} return false;} }
    }
    fclose(saveFile);
    TraceLog(LOG_INFO, "[SaveGame] Jogo salvo: '%s' (Modo: %s, Jogadores: %d)", filename, (gameMode == GAME_MODE_SINGLE_PLAYER ? "1P" : "2P"), num_players_to_save);
    return true;
}

bool LoadGame(Player players[], int max_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY, GameModeType gameMode, int *numPlayersLoaded) {
    char filename[256]; 
    FILE *loadFile = NULL; 
    size_t items_read; 
    unsigned int file_version = 0;
    int num_players_saved_in_file = 0;
    int i, j; 

    GetSaveFileName(filename, sizeof(filename), slot_number, gameMode);
    if (!FileExists(filename)) { TraceLog(LOG_WARNING, "[LoadGame] Arquivo de save '%s' nao existe.", filename); return false; }
    loadFile = fopen(filename, "rb"); 
    if (loadFile == NULL) { TraceLog(LOG_ERROR, "[LoadGame] Falha ao abrir '%s'.", filename); return false; }
    
    items_read = fread(&file_version, sizeof(unsigned int), (size_t)1, loadFile);
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler versao de '%s'.", filename); if(loadFile){fclose(loadFile);} return false; }
    if (file_version != SAVEGAME_FILE_VERSION) { TraceLog(LOG_ERROR, "[LoadGame] Versao ('%s' v%u) incompativel (esperado v%u).", filename, file_version, SAVEGAME_FILE_VERSION); if(loadFile){fclose(loadFile);} return false; }
    
    items_read = fread(loadedMapX, sizeof(int), (size_t)1, loadFile); 
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapX de '%s'.", filename); if(loadFile){fclose(loadFile);} return false; }
    items_read = fread(loadedMapY, sizeof(int), (size_t)1, loadFile); 
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapY de '%s'.", filename); if(loadFile){fclose(loadFile);} return false; }
    
    items_read = fread(&num_players_saved_in_file, sizeof(int), (size_t)1, loadFile);
    if (items_read != 1 || num_players_saved_in_file <= 0 || num_players_saved_in_file > MAX_PLAYERS_SUPPORTED) { 
        TraceLog(LOG_ERROR, "[LoadGame] Numero invalido de jogadores (%d) no save '%s'.", num_players_saved_in_file, filename); 
        if(loadFile){fclose(loadFile);} return false; 
    }
    if(numPlayersLoaded) *numPlayersLoaded = num_players_saved_in_file;

    if (gameMode == GAME_MODE_SINGLE_PLAYER && num_players_saved_in_file != 1) { 
        TraceLog(LOG_ERROR, "[LoadGame] Save de %d jogadores invalido para modo Single Player.", num_players_saved_in_file); 
        if(loadFile){fclose(loadFile);} return false; 
    }
    
    for (i = 0; i < num_players_saved_in_file; i++) {
        if (i >= max_players_in_game) { TraceLog(LOG_ERROR, "[LoadGame] Contagem de jogadores incompativel em '%s'.", filename); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(players[i].nome, sizeof(char), (size_t)MAX_PLAYER_NAME_LENGTH, loadFile); if (items_read != MAX_PLAYER_NAME_LENGTH) { TraceLog(LOG_ERROR, "LoadGame: Erro nome P%d", i); if(loadFile){fclose(loadFile);} return false;}
        players[i].nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0'; 
        items_read = fread(&players[i].classe, sizeof(Classe), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro classe P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].nivel, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro nivel P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].exp, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro exp P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].vida, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro vida P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].max_vida, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro max_vida P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].mana, sizeof(int), (size_t)1, loadFile);   if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro mana P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].max_mana, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro max_mana P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].stamina, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro stamina P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].max_stamina, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro max_stamina P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].magic_attack, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro magic_attack P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].magic_defense, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro magic_defense P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].ataque, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro ataque P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].defesa, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro defesa P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].forca, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro forca P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].percepcao, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro percepcao P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].resistencia, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro resistencia P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].carisma, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro carisma P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].inteligencia, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro inteligencia P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].agilidade, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro agilidade P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].sorte, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro sorte P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].moedas, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro moedas P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].posx, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro posx P%d", i); if(loadFile){fclose(loadFile);} return false;}
        items_read = fread(&players[i].posy, sizeof(int), (size_t)1, loadFile); if (items_read != 1) { TraceLog(LOG_ERROR, "LoadGame: Erro posy P%d", i); if(loadFile){fclose(loadFile);} return false;}
        for (j=0; j<MAX_INVENTORY_SLOTS; j++) { items_read = fread(&players[i].inventory[j], sizeof(InventoryItem), (size_t)1, loadFile); if (items_read!=1) { TraceLog(LOG_ERROR, "LoadGame: Erro inventario P%d item %d", i, j); if(loadFile){fclose(loadFile);} return false;} }
        items_read = fread(&players[i].inventory_item_count, sizeof(int), (size_t)1, loadFile); if (items_read!=1) { TraceLog(LOG_ERROR, "LoadGame: Erro contagem inventario P%d", i); if(loadFile){fclose(loadFile);} return false;}
        for (j=0; j<MAX_EQUIP_SLOTS; j++) { items_read = fread(&players[i].equipped_items[j], sizeof(InventoryItem), (size_t)1, loadFile); if (items_read!=1) { TraceLog(LOG_ERROR, "LoadGame: Erro equipamento P%d item %d", i, j); if(loadFile){fclose(loadFile);} return false;} }
        if (players[i].vida > players[i].max_vida) players[i].vida = players[i].max_vida;
        if (players[i].mana > players[i].max_mana) players[i].mana = players[i].max_mana;
        if (players[i].stamina > players[i].max_stamina) players[i].stamina = players[i].max_stamina;
    }
    fclose(loadFile);
    TraceLog(LOG_INFO, "[LoadGame] Jogo carregado de '%s'. (Jogadores: %d, Mapa X:%d, Y:%d)", filename, num_players_saved_in_file, *loadedMapX, *loadedMapY);
    return true;
}
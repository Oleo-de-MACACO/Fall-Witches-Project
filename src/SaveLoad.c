#include "../include/SaveLoad.h"
#include "../include/Classes.h"
#include "../include/WalkCycle.h"
#include "../include/GameProgress.h" // *** ADICIONADO PARA ACESSAR g_gameProgress ***
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include <stddef.h>
#include <errno.h>

// *** INCREMENTA A VERSÃO DO SAVE PARA INVALIDAR SAVES ANTIGOS ***
const unsigned int SAVEGAME_FILE_VERSION = 8; // Era 7

// ... (Macros e funções auxiliares como na Resposta 44) ...
#define FWRITE_CHECK(ptr, size, count, stream, file_ptr_for_close, error_message_prefix, filename_param) \
    items_written = fwrite(ptr, size, count, stream); \
    if (items_written != count) { \
        TraceLog(LOG_ERROR, "[SaveGame] Erro %s para '%s'. Escrito: %zu, Esperado: %zu. Erro: %s", error_message_prefix, filename_param, items_written, (size_t)count, strerror(errno)); \
        if(file_ptr_for_close) { fclose(file_ptr_for_close); } \
        return false; \
    }
#define FREAD_CHECK(ptr, size, count, stream, file_ptr_for_close, error_message_prefix, filename_param) \
    items_read = fread(ptr, size, count, stream); \
    if (items_read != count) { \
        if (feof(stream)) { TraceLog(LOG_ERROR, "[LoadGame] Fim de arquivo inesperado ao %s de '%s'.", error_message_prefix, filename_param); } \
        else if (ferror(stream)) { TraceLog(LOG_ERROR, "[LoadGame] Erro de I/O ao %s de '%s'. Erro: %s", error_message_prefix, filename_param, strerror(errno)); } \
        else { TraceLog(LOG_ERROR, "[LoadGame] Erro ao %s de '%s'. Lido: %zu, Esperado: %zu", error_message_prefix, filename_param, items_read, (size_t)count); } \
        if(file_ptr_for_close) { fclose(file_ptr_for_close); } return false; \
    }
static const char* GetModeSubdirectoryFullPathInternal(GameModeType gameMode, char* path_buffer, size_t buffer_size) { /* ... */
    if (!path_buffer || buffer_size == 0) return NULL; const char* subdir_name = (gameMode == GAME_MODE_SINGLE_PLAYER) ? SAVE_SUBDIR_SINGLEPLAYER : SAVE_SUBDIR_TWOPLAYER;
    int written = snprintf(path_buffer, buffer_size, "%s/%s", SAVE_BASE_DIRECTORY, subdir_name);
    if (written < 0 || (size_t)written >= buffer_size) { if (buffer_size > 0) path_buffer[0] = '\0'; return NULL; } return path_buffer;
}
bool EnsureSavesDirectoryExists(GameModeType gameMode) { /* ... */
    if (!DirectoryExists(SAVE_BASE_DIRECTORY)) { TraceLog(LOG_WARNING, "Diretorio base '%s' NAO existe. Crie manualmente.", SAVE_BASE_DIRECTORY); return false; }
    char modeSavePath[128]; if (GetModeSubdirectoryFullPathInternal(gameMode, modeSavePath, sizeof(modeSavePath)) == NULL) { return false; }
    if (!DirectoryExists(modeSavePath)) { TraceLog(LOG_WARNING, "Diretorio de saves para modo '%s' ('%s') NAO existe. Crie manualmente.", (gameMode == GAME_MODE_SINGLE_PLAYER ? SAVE_SUBDIR_SINGLEPLAYER : SAVE_SUBDIR_TWOPLAYER), modeSavePath); return false; }
    return true;
}
void GetSaveFileName(char *buffer, int buffer_size, int slot_number, GameModeType gameMode) { /* ... */
    if (!buffer || buffer_size == 0) return; char modePath[128];
    if (GetModeSubdirectoryFullPathInternal(gameMode, modePath, sizeof(modePath)) == NULL) { if (buffer_size > 0) { buffer[0] = '\0'; } return; }
    snprintf(buffer, (unsigned)buffer_size, "%s/Save%d.sav", modePath, slot_number + 1);
}
bool DoesSaveSlotExist(int slot_number, GameModeType gameMode) { /* ... */
    char filename[256]; GetSaveFileName(filename, sizeof(filename), slot_number, gameMode); if (filename[0] == '\0') return false; return FileExists(filename);
}

bool SaveGame(Player players[], int num_players_to_save, int slot_number, int currentMapX, int currentMapY, GameModeType gameMode) {
    char filename[256]; FILE *saveFile = NULL; size_t items_written;
    if (!EnsureSavesDirectoryExists(gameMode)) { return false; }
    GetSaveFileName(filename, sizeof(filename), slot_number, gameMode);
    if (filename[0] == '\0') { return false; }
    saveFile = fopen(filename, "wb");
    if (saveFile == NULL) { TraceLog(LOG_ERROR, "Falha ao abrir '%s': %s", filename, strerror(errno)); return false; }

    FWRITE_CHECK(&SAVEGAME_FILE_VERSION, sizeof(unsigned int), 1, saveFile, saveFile, "versao", filename);
    FWRITE_CHECK(&currentMapX, sizeof(int), 1, saveFile, saveFile, "mapX", filename);
    FWRITE_CHECK(&currentMapY, sizeof(int), 1, saveFile, saveFile, "mapY", filename);
    FWRITE_CHECK(&num_players_to_save, sizeof(int), 1, saveFile, saveFile, "num_players", filename);

    // Salva os dados de progresso do jogo (ex: mapas visitados)
    FWRITE_CHECK(&g_gameProgress, sizeof(GameProgress), 1, saveFile, saveFile, "progresso do jogo", filename);

    for (int i = 0; i < num_players_to_save; i++) {
        // ... (todos os FWRITE_CHECK para os dados do jogador como antes)
        FWRITE_CHECK(players[i].nome, sizeof(char), MAX_PLAYER_NAME_LENGTH, saveFile, saveFile, "nome", filename);
        FWRITE_CHECK(&players[i].classe, sizeof(Classe), 1, saveFile, saveFile, "classe", filename);
        FWRITE_CHECK(&players[i].spriteType, sizeof(SpriteType), 1, saveFile, saveFile, "spriteType", filename);
        FWRITE_CHECK(&players[i].nivel, sizeof(int), 1, saveFile, saveFile, "nivel", filename);
        FWRITE_CHECK(&players[i].exp, sizeof(int), 1, saveFile, saveFile, "exp", filename);
        FWRITE_CHECK(&players[i].vida, sizeof(int), 1, saveFile, saveFile, "vida", filename);
        FWRITE_CHECK(&players[i].max_vida, sizeof(int), 1, saveFile, saveFile, "max_vida", filename);
        FWRITE_CHECK(&players[i].mana, sizeof(int), 1, saveFile, saveFile, "mana", filename);
        FWRITE_CHECK(&players[i].max_mana, sizeof(int), 1, saveFile, saveFile, "max_mana", filename);
        FWRITE_CHECK(&players[i].stamina, sizeof(int), 1, saveFile, saveFile, "stamina", filename);
        FWRITE_CHECK(&players[i].max_stamina, sizeof(int), 1, saveFile, saveFile, "max_stamina", filename);
        FWRITE_CHECK(&players[i].magic_attack, sizeof(int), 1, saveFile, saveFile, "magic_attack", filename);
        FWRITE_CHECK(&players[i].magic_defense, sizeof(int), 1, saveFile, saveFile, "magic_defense", filename);
        FWRITE_CHECK(&players[i].ataque, sizeof(int), 1, saveFile, saveFile, "ataque", filename);
        FWRITE_CHECK(&players[i].defesa, sizeof(int), 1, saveFile, saveFile, "defesa", filename);
        FWRITE_CHECK(&players[i].forca, sizeof(int), 1, saveFile, saveFile, "forca", filename);
        FWRITE_CHECK(&players[i].percepcao, sizeof(int), 1, saveFile, saveFile, "percepcao", filename);
        FWRITE_CHECK(&players[i].resistencia, sizeof(int), 1, saveFile, saveFile, "resistencia", filename);
        FWRITE_CHECK(&players[i].carisma, sizeof(int), 1, saveFile, saveFile, "carisma", filename);
        FWRITE_CHECK(&players[i].inteligencia, sizeof(int), 1, saveFile, saveFile, "inteligencia", filename);
        FWRITE_CHECK(&players[i].agilidade, sizeof(int), 1, saveFile, saveFile, "agilidade", filename);
        FWRITE_CHECK(&players[i].sorte, sizeof(int), 1, saveFile, saveFile, "sorte", filename);
        FWRITE_CHECK(&players[i].moedas, sizeof(int), 1, saveFile, saveFile, "moedas", filename);
        FWRITE_CHECK(&players[i].posx, sizeof(int), 1, saveFile, saveFile, "posx", filename);
        FWRITE_CHECK(&players[i].posy, sizeof(int), 1, saveFile, saveFile, "posy", filename);
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) { FWRITE_CHECK(&players[i].inventory[j], sizeof(InventoryItem), 1, saveFile, saveFile, "inventario_item", filename); }
        FWRITE_CHECK(&players[i].inventory_item_count, sizeof(int), 1, saveFile, saveFile, "contagem_inventario", filename);
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) { FWRITE_CHECK(&players[i].equipped_items[j], sizeof(InventoryItem), 1, saveFile, saveFile, "equipamento_item", filename); }

    }
    fclose(saveFile);
    TraceLog(LOG_INFO, "[SaveGame] Jogo salvo: '%s'", filename);
    return true;
}

bool LoadGame(Player players[], int max_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY, GameModeType gameMode, int *numPlayersLoaded) {
    char filename[256]; FILE *loadFile = NULL; size_t items_read;
    unsigned int file_version = 0; int num_players_saved_in_file = 0;
    GetSaveFileName(filename, sizeof(filename), slot_number, gameMode);
    if (filename[0] == '\0' || !FileExists(filename)) { return false; }
    loadFile = fopen(filename, "rb");
    if (loadFile == NULL) { return false; }

    FREAD_CHECK(&file_version, sizeof(unsigned int), 1, loadFile, loadFile, "versao", filename);
    if (file_version != SAVEGAME_FILE_VERSION) {
        TraceLog(LOG_ERROR, "Versao do save incompativel: %u (esperado: %u)", file_version, SAVEGAME_FILE_VERSION);
        fclose(loadFile); return false;
    }
    FREAD_CHECK(loadedMapX, sizeof(int), 1, loadFile, loadFile, "mapX", filename);
    FREAD_CHECK(loadedMapY, sizeof(int), 1, loadFile, loadFile, "mapY", filename);
    FREAD_CHECK(&num_players_saved_in_file, sizeof(int), 1, loadFile, loadFile, "num_players_saved", filename);

    // Carrega os dados de progresso do jogo
    FREAD_CHECK(&g_gameProgress, sizeof(GameProgress), 1, loadFile, loadFile, "progresso do jogo", filename);

    if (num_players_saved_in_file <= 0 || num_players_saved_in_file > MAX_PLAYERS_SUPPORTED) { /* ... erro ... */ fclose(loadFile); return false; }
    if(numPlayersLoaded) *numPlayersLoaded = num_players_saved_in_file;
    if (gameMode == GAME_MODE_SINGLE_PLAYER && num_players_saved_in_file != 1) { /* ... erro ... */ fclose(loadFile); return false; }

    for (int i = 0; i < num_players_saved_in_file; i++) {
        if (i >= max_players_in_game) { /* ... erro ... */ fclose(loadFile); return false; }
        memset(&players[i], 0, sizeof(Player));
        // ... (todos os FREAD_CHECK para os dados do jogador como antes) ...
        FREAD_CHECK(players[i].nome,sizeof(char),MAX_PLAYER_NAME_LENGTH,loadFile,loadFile,"nome",filename); players[i].nome[MAX_PLAYER_NAME_LENGTH-1]='\0';
        FREAD_CHECK(&players[i].classe,sizeof(Classe),1,loadFile,loadFile,"classe",filename); FREAD_CHECK(&players[i].spriteType,sizeof(SpriteType),1,loadFile,loadFile,"spriteType",filename);
        FREAD_CHECK(&players[i].nivel,sizeof(int),1,loadFile,loadFile,"nivel",filename); FREAD_CHECK(&players[i].exp,sizeof(int),1,loadFile,loadFile,"exp",filename);
        FREAD_CHECK(&players[i].vida,sizeof(int),1,loadFile,loadFile,"vida",filename); FREAD_CHECK(&players[i].max_vida,sizeof(int),1,loadFile,loadFile,"max_vida",filename);
        FREAD_CHECK(&players[i].mana,sizeof(int),1,loadFile,loadFile,"mana",filename); FREAD_CHECK(&players[i].max_mana,sizeof(int),1,loadFile,loadFile,"max_mana",filename);
        FREAD_CHECK(&players[i].stamina,sizeof(int),1,loadFile,loadFile,"stamina",filename); FREAD_CHECK(&players[i].max_stamina,sizeof(int),1,loadFile,loadFile,"max_stamina",filename);
        FREAD_CHECK(&players[i].magic_attack,sizeof(int),1,loadFile,loadFile,"magic_attack",filename); FREAD_CHECK(&players[i].magic_defense,sizeof(int),1,loadFile,loadFile,"magic_defense",filename);
        FREAD_CHECK(&players[i].ataque,sizeof(int),1,loadFile,loadFile,"ataque",filename); FREAD_CHECK(&players[i].defesa,sizeof(int),1,loadFile,loadFile,"defesa",filename);
        FREAD_CHECK(&players[i].forca,sizeof(int),1,loadFile,loadFile,"forca",filename); FREAD_CHECK(&players[i].percepcao,sizeof(int),1,loadFile,loadFile,"percepcao",filename);
        FREAD_CHECK(&players[i].resistencia,sizeof(int),1,loadFile,loadFile,"resistencia",filename); FREAD_CHECK(&players[i].carisma,sizeof(int),1,loadFile,loadFile,"carisma",filename);
        FREAD_CHECK(&players[i].inteligencia,sizeof(int),1,loadFile,loadFile,"inteligencia",filename); FREAD_CHECK(&players[i].agilidade,sizeof(int),1,loadFile,loadFile,"agilidade",filename);
        FREAD_CHECK(&players[i].sorte,sizeof(int),1,loadFile,loadFile,"sorte",filename); FREAD_CHECK(&players[i].moedas,sizeof(int),1,loadFile,loadFile,"moedas",filename);
        FREAD_CHECK(&players[i].posx,sizeof(int),1,loadFile,loadFile,"posx",filename); FREAD_CHECK(&players[i].posy,sizeof(int),1,loadFile,loadFile,"posy",filename);
        for(int j=0;j<MAX_INVENTORY_SLOTS;j++){FREAD_CHECK(&players[i].inventory[j],sizeof(InventoryItem),1,loadFile,loadFile,"inventario_item",filename);}
        FREAD_CHECK(&players[i].inventory_item_count,sizeof(int),1,loadFile,loadFile,"contagem_inventario",filename);
        for(int j=0;j<MAX_EQUIP_SLOTS;j++){FREAD_CHECK(&players[i].equipped_items[j],sizeof(InventoryItem),1,loadFile,loadFile,"equipamento_item",filename);}

        LoadCharacterAnimations(&players[i]); // Define width/height da textura
    }
    fclose(loadFile);
    TraceLog(LOG_INFO,"[LoadGame] Jogo carregado de '%s'",filename);
    return true;
}
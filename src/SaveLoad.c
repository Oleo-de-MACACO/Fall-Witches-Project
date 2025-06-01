#include "../include/SaveLoad.h"
#include "../include/Classes.h" 
#include <stdio.h>
#include "raylib.h" 

// Versão do arquivo de save. INCREMENTAR se a estrutura mudar!
// V1: Stats básicos, inventário, posições X,Y jogador
// V2: Coordenadas globais do mapa (currentMapX, currentMapY)
// V3: (Não formalmente versionado, mas implícito para nome/classe)
// V4: Adicionado nome (string), classe (enum), S.P.E.C.I.A.L. (7 ints), moedas (int)
const unsigned int SAVEGAME_FILE_VERSION = 4; 

// Garante que o diretório de saves exista.
bool EnsureSavesDirectoryExists(void) {
    if (!DirectoryExists(SAVE_DIRECTORY)) {
        TraceLog(LOG_WARNING, "Diretório de saves '%s' NÃO existe. Crie-o manualmente.", SAVE_DIRECTORY);
        return false; 
    }
    return true;
}

// Gera o nome completo do arquivo para um slot de save.
void GetSaveFileName(char *buffer, int buffer_size, int slot_number) {
    snprintf(buffer, buffer_size, "%s/Save%d.sav", SAVE_DIRECTORY, slot_number + 1);
}

// Verifica se um arquivo de save para o slot especificado existe.
bool DoesSaveSlotExist(int slot_number) {
    if (!DirectoryExists(SAVE_DIRECTORY)) return false; 
    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);
    return FileExists(filename);
}

// Salva os dados do jogo no slot especificado.
bool SaveGame(Player players[], int num_players, int slot_number, int currentMapX, int currentMapY) {
    if (!EnsureSavesDirectoryExists()) { 
        TraceLog(LOG_ERROR, "Diretório de saves '%s' não encontrado. Impossível salvar.", SAVE_DIRECTORY);
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

    // 1. Versão do arquivo de Save
    items_written = fwrite(&SAVEGAME_FILE_VERSION, sizeof(unsigned int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever versão do save."); fclose(saveFile); return false; }
    
    // 2. Coordenadas Globais do Mapa
    items_written = fwrite(&currentMapX, sizeof(int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapX."); fclose(saveFile); return false; }
    items_written = fwrite(&currentMapY, sizeof(int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever currentMapY."); fclose(saveFile); return false; }

    // 3. Número de Jogadores
    items_written = fwrite(&num_players, sizeof(int), 1, saveFile);
    if (items_written != 1) { TraceLog(LOG_ERROR, "Erro ao escrever número de jogadores."); fclose(saveFile); return false; }

    // 4. Dados de cada Jogador
    for (int i = 0; i < num_players; i++) {
        // Nome do Jogador
        items_written = fwrite(players[i].nome, sizeof(char), MAX_PLAYER_NAME_LENGTH, saveFile);
        if (items_written != MAX_PLAYER_NAME_LENGTH) goto save_error_player;

        // Classe do Jogador
        items_written = fwrite(&players[i].classe, sizeof(Classe), 1, saveFile); 
        if (items_written != 1) goto save_error_player;

        // Stats básicos
        items_written = fwrite(&players[i].nivel, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].exp, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].vida, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].max_vida, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].mana, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].max_mana, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].ataque, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].defesa, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        
        // S.P.E.C.I.A.L. Stats
        items_written = fwrite(&players[i].forca, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].percepcao, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].resistencia, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].carisma, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].inteligencia, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].agilidade, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].sorte, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;

        // Moedas
        items_written = fwrite(&players[i].moedas, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;

        // Posição na tela
        items_written = fwrite(&players[i].posx, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;
        items_written = fwrite(&players[i].posy, sizeof(int), 1, saveFile); if (items_written != 1) goto save_error_player;

        // Inventário
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) {
            items_written = fwrite(&players[i].inventory[j], sizeof(InventoryItem), 1, saveFile);
            if (items_written != 1) goto save_error_player;
        }
        items_written = fwrite(&players[i].inventory_item_count, sizeof(int), 1, saveFile);
        if (items_written != 1) goto save_error_player;

        // Equipamento
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) {
            items_written = fwrite(&players[i].equipped_items[j], sizeof(InventoryItem), 1, saveFile);
            if (items_written != 1) goto save_error_player;
        }
    }

    fclose(saveFile);
    TraceLog(LOG_INFO, "[SaveGame] Jogo salvo com sucesso em '%s'. (Mapa X:%d, Y:%d)", filename, currentMapX, currentMapY);
    return true;

save_error_player: 
    TraceLog(LOG_ERROR, "[SaveGame] Erro durante a escrita dos dados do jogador no arquivo de save '%s'.", filename);
    fclose(saveFile); 
    return false;
}

// Carrega os dados do jogo do slot especificado.
bool LoadGame(Player players[], int num_players_in_game, int slot_number, int *loadedMapX, int *loadedMapY) {
    if (!DirectoryExists(SAVE_DIRECTORY)){ 
         TraceLog(LOG_WARNING, "[LoadGame] Diretório de saves '%s' não encontrado. Impossível carregar.", SAVE_DIRECTORY);
         return false;
    }

    char filename[100];
    GetSaveFileName(filename, sizeof(filename), slot_number);

    if (!FileExists(filename)) {
        TraceLog(LOG_WARNING, "[LoadGame] Arquivo de save '%s' não existe para carregar.", filename);
        return false;
    }

    FILE *loadFile = fopen(filename, "rb"); 
    if (loadFile == NULL) {
        TraceLog(LOG_ERROR, "[LoadGame] Falha ao abrir arquivo de save '%s' para leitura.", filename);
        return false;
    }

    size_t items_read;
    unsigned int file_version = 0;

    items_read = fread(&file_version, sizeof(unsigned int), 1, loadFile);
    if (items_read != 1 || file_version != SAVEGAME_FILE_VERSION) {
        TraceLog(LOG_ERROR, "[LoadGame] Versão incompatível ou erro ao ler versão de '%s'. Esperado: %u, Encontrado: %u.", filename, SAVEGAME_FILE_VERSION, file_version);
        fclose(loadFile); return false;
    }

    items_read = fread(loadedMapX, sizeof(int), 1, loadFile);
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapX de '%s'.", filename); fclose(loadFile); return false; }
    items_read = fread(loadedMapY, sizeof(int), 1, loadFile);
    if (items_read != 1) { TraceLog(LOG_ERROR, "[LoadGame] Erro ao ler loadedMapY de '%s'.", filename); fclose(loadFile); return false; }

    int num_players_saved = 0;
    items_read = fread(&num_players_saved, sizeof(int), 1, loadFile);
    if (items_read != 1 || num_players_saved > num_players_in_game || num_players_saved <= 0 ) { // Jogo deve suportar o num de jogadores salvos
        TraceLog(LOG_ERROR, "[LoadGame] Número de jogadores em '%s' (%d) incompatível com o jogo (%d) ou erro de leitura.", filename, num_players_saved, num_players_in_game);
        fclose(loadFile); return false;
    }
    
    for (int i = 0; i < num_players_saved; i++) {
        // Nome do Jogador
        items_read = fread(players[i].nome, sizeof(char), MAX_PLAYER_NAME_LENGTH, loadFile);
        if (items_read != MAX_PLAYER_NAME_LENGTH) goto load_error_player;
        players[i].nome[MAX_PLAYER_NAME_LENGTH - 1] = '\0'; 

        // Classe do Jogador
        items_read = fread(&players[i].classe, sizeof(Classe), 1, loadFile);
        if (items_read != 1) goto load_error_player;
        
        // Stats básicos
        items_read = fread(&players[i].nivel, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].exp, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].vida, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].max_vida, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].mana, sizeof(int), 1, loadFile);   if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].max_mana, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].ataque, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].defesa, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;

        // S.P.E.C.I.A.L. Stats
        items_read = fread(&players[i].forca, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].percepcao, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].resistencia, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].carisma, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].inteligencia, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].agilidade, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].sorte, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;

        // Moedas
        items_read = fread(&players[i].moedas, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        
        // Posição na tela
        items_read = fread(&players[i].posx, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;
        items_read = fread(&players[i].posy, sizeof(int), 1, loadFile); if (items_read != 1) goto load_error_player;

        // Inventário
        for (int j = 0; j < MAX_INVENTORY_SLOTS; j++) {
            items_read = fread(&players[i].inventory[j], sizeof(InventoryItem), 1, loadFile);
            if (items_read != 1) goto load_error_player;
        }
        items_read = fread(&players[i].inventory_item_count, sizeof(int), 1, loadFile);
        if (items_read != 1) goto load_error_player;

        // Equipamento
        for (int j = 0; j < MAX_EQUIP_SLOTS; j++) {
            items_read = fread(&players[i].equipped_items[j], sizeof(InventoryItem), 1, loadFile);
            if (items_read != 1) goto load_error_player;
        }

        if (players[i].vida > players[i].max_vida) players[i].vida = players[i].max_vida;
        if (players[i].mana > players[i].max_mana) players[i].mana = players[i].max_mana;
    }

    fclose(loadFile);
    TraceLog(LOG_INFO, "[LoadGame] Jogo carregado com sucesso de '%s'. (Mapa X:%d, Y:%d)", filename, *loadedMapX, *loadedMapY);
    return true;

load_error_player: 
    TraceLog(LOG_ERROR, "[LoadGame] Erro durante a leitura dos dados do jogador do arquivo '%s'.", filename);
    fclose(loadFile);
    return false;
}
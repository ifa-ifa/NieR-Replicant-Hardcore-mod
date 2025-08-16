#include <Windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <MinHook.h>
#include "shared.h"
#include "settings.h"

Settings settings;
LPCWSTR ProcessName = L"NieR Replicant ver.1.22474487139.exe";
uintptr_t processBaseAddress = (uintptr_t)GetModuleHandleW(ProcessName);


/*!
 * ----------------------------------------------------------------------
 * canUseItem
 *
 * @brief Called on item use. Determines wherever or not an item can be used.
 * @param param_1: ?
 * @param item_id: fixed id for the item used
 * @param item_inventory_index: index of the item in the players inventory. Can change if you have 0 of certain items, the items afterwards get pushed forwards
 * @param param_4: ?
 * @return: 1 on valid item use. 0 if you try to cure poison when you are not poisoned. 162048 if you try use an item when health is full
 *
 * This hook allows us to block item usage if our conditions are not met.
 * ----------------------------------------------------------------------
 */
typedef uint64_t(__fastcall* canUseItem)(void*, int32_t, int64_t, int64_t);
canUseItem canUseItem_hooked = canUseItem(processBaseAddress + 0x3b6fb0);
canUseItem canUseItem_original;
uint64_t __fastcall canUseItem_detoured(void* param_1, int32_t item_id, int64_t item_inventory_index, uint64_t param_4)
{

    // prevent this mod from triggering for things like fishing
    if (item_id > 34) return canUseItem_original(param_1, item_id, item_inventory_index, param_4); 

    float* current_mp = (float*)(processBaseAddress + 0x4374A78);
    float* max_mp = (float*)(processBaseAddress + 0x122D2DC);
    DWORD* current_hp = (DWORD*)(processBaseAddress + 0x4374A6C);
    DWORD* max_hp = (DWORD*)(processBaseAddress + 0x122D2D8);
    bool* isPlayerKaine = (bool*)(processBaseAddress + 0x4375850);

    if (*isPlayerKaine == 1) {
        if (settings.debug) std::cout << "Player is kaine, resuming default logic " << "\n";
        return canUseItem_original(param_1, item_id, item_inventory_index, param_4);
    }

    if (item_id < 3 and *current_hp == *max_hp) { 
        if (settings.debug) std::cout << "Healing item used at full health " << "\n";
        return 162048;
    }

    float epsilon = 0.02f;
    bool has_enough_mp = (*current_mp >= *max_mp * (settings.mp_multiplier_required_for_item - epsilon));

    if (has_enough_mp) {
        if (settings.debug) std::cout << "Has enough MP" << "\n";

        *current_mp = (*current_mp * settings.mp_multiplier_on_item_use) - settings.mp_reduction_on_item_use;
        return canUseItem_original(param_1, item_id, item_inventory_index, param_4);
    }

    if (settings.debug) std::cout << "Not enough MP" << "\n";
    return 162048;
}


/*!
 * ----------------------------------------------------------------------
 * loadStats
 *
 * @brief Called on exiting loading screen or level up. Calculates new stats.
 * @param param_1: player param data
 * @param param_2: current level (0 indexed)
 * @param param_3: ?
 * @param param_4: ?
 *
 * This hook allows us to alter player stats after the game has generated and saved them.
 * ------------------------------------------------------------------------
 */

typedef void(__fastcall* loadStats)(void*, int32_t, char, char);
loadStats loadStats_hooked = loadStats(processBaseAddress + 0x407ad0);
loadStats loadStats_original;
void __fastcall loadStats_detoured(void* PlayerParam, int32_t currentLevel, char param_3, char param_4) {

    loadStats_original(PlayerParam, currentLevel, param_3, param_4);

    float* current_mp = (float*)(processBaseAddress + 0x4374A78);
    float* max_mp = (float*)(processBaseAddress + 0x122D2DC);
    int* max_hp = (int*)(processBaseAddress + 0x122D2D8);



    int* attack = (int*)(processBaseAddress + 0x122D2FC);
    int* magic_attack = (int*)(processBaseAddress + 0x122D300);
    int* defense = (int*)(processBaseAddress + 0x122D308);
    int* magic_defense = (int*)(processBaseAddress + 0x122D30C);



    if (settings.use_fixed_max_mp) {
        *max_mp = settings.fixed_max_mp;
    }
    else {
        *max_mp = (*max_mp * settings.max_mp_multiplier);
    }

    if (settings.use_fixed_max_hp) {
        *max_hp = settings.fixed_max_hp;
    }
    else {
        *max_hp = (*max_hp * settings.max_hp_multiplier);
    }
    *current_mp = *max_mp;

    *attack *= settings.attack_stat_multiplier;
    *magic_attack *= settings.magic_attack_stat_multiplier;
    *defense *= settings.defense_stat_multiplier;
    *magic_defense *= settings.magic_defense_stat_multiplier;

}


/*!
 * ----------------------------------------------------------------------
 * getMpRegenDifficultyFactor
 *
 * @brief MP regen rate is affected by difficulty. This function retrieves that factor.
 * @param param_1: static GameDifficultSingleton
 * @return returns 1 on medium and hard, 3 on easy.
 *
 * This hook allows us to alter the mp regen scaling to our liking.
 * ----------------------------------------------------------------------
 */
typedef float(__fastcall* getMpRegenDifficultyFactor)(void*);
getMpRegenDifficultyFactor getMpRegenDifficultyFactor_hooked = getMpRegenDifficultyFactor(processBaseAddress + 0x65fbe0);
getMpRegenDifficultyFactor getMpRegenDifficultyFactor_original;
float __fastcall getMpRegenDifficultyFactor_detoured(void* gameDifficultSingleton) {

    // return as a multiple of the original to respect the ingame difficulty selection
    return settings.passive_mp_recovery_multiplier * getMpRegenDifficultyFactor_original(gameDifficultSingleton);
}



typedef void(__fastcall* spawnEnemyGroup)(int32_t, uint8_t, float);
spawnEnemyGroup spawnEnemyGroup_original = spawnEnemyGroup(processBaseAddress + 0x425ba0);


/*!
 * ----------------------------------------------------------------------
 * onEnemyMeleeHit
 *
 * @brief Called when an enemy is hit by the player with a melee attack.
 * @param param_1: hitEvent*
 *
 * Hooking this allows us to implement MP recovery on melee hits.
 * ----------------------------------------------------------------------
 */
typedef void(__fastcall* onEnemyMeleeHit)(void*);
onEnemyMeleeHit onEnemyMeleeHit_hooked = onEnemyMeleeHit(processBaseAddress + 0x6b2180);


onEnemyMeleeHit onEnemyMeleeHit_original;

void __fastcall onEnemyMeleeHit_detoured(void* hitEvent) {

    if (!settings.enable_mp_recovery_on_hit) {
        return onEnemyMeleeHit_original(hitEvent);
    }


    float* current_mp = (float*)(processBaseAddress + 0x4374A78);
    float* max_mp = (float*)(processBaseAddress + 0x122D2DC);
    *current_mp = *current_mp + (*max_mp * settings.multiplier_mp_recovery_on_hit) + settings.fixed_mp_recovery_on_hit;

    onEnemyMeleeHit_original(hitEvent);
}


/*!
 * ----------------------------------------------------------------------
 * ----------------------------- Mod Setup ------------------------------
 * ----------------------------------------------------------------------
 */



template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

DWORD WINAPI SetupHooks(LPVOID lpParam) {

    int ret = settings.LoadFromFile();

    if (settings.debug) {
        AllocConsole();
        FILE* console;
        freopen_s(&console, "CONOUT$", "w", stdout);
    }

    switch (ret) {
    case 0:
        std::cout << "Settings loaded successfully from INI file.\n";
        break;
    case 1:
        std::cout << "WARNING: INI file exists but is corrupt. Using default settings for this session. Correct the error, or delete the ini file to regenerate a new one on restart.\n";
        break;
    case 2:
        std::cout << "INI file not found. A new one has been created with default settings.\n";
        break;
    }

    ret = MH_Initialize();
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when initializing minhook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }
    std::cout << "initialized minhook" << "\n";


    ret = MH_CreateHookEx(canUseItem_hooked, &canUseItem_detoured, &canUseItem_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating canUseItem hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(loadStats_hooked, &loadStats_detoured, &loadStats_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating loadStats hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(getMpRegenDifficultyFactor_hooked, &getMpRegenDifficultyFactor_detoured, &getMpRegenDifficultyFactor_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(onEnemyMeleeHit_hooked, &onEnemyMeleeHit_detoured, &onEnemyMeleeHit_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating enemy_hit_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_EnableHook(MH_ALL_HOOKS);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when enabling hooks. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }
    std::cout << "hooks enabled" << "\n";

    return TRUE;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {

        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, SetupHooks, hModule, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
        else {
            MessageBoxW(nullptr, L"Thread creation failed", L"Error", MB_ICONERROR);
        }
    }

    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
    }

    return TRUE;
}

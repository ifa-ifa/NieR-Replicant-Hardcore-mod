#include <Windows.h>
#include "inireader.h"
#include "MinHook.h"
#include <string>
#include <iostream>
#include <fstream>


std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return std::string();
    }
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}


struct Settings {

    bool debug;      // opens terminal for debugging

    FLOAT mp_multiplier_required_for_item; // using item will fail if current mp is below this multiplier (due to implementation of change_current_mp hook, this shouldn't be set to exactly 1, it should be a little less)
    FLOAT mp_multiplier_on_item_use; // on item use, currrent mp is multiplied by this value. applied before reduxtion
    FLOAT mp_reduction_on_item_use;    // use this and set multiplier to 1.0 if you want a fixed reduction on item use. Or use a combination of both. applied after the multiplier

    bool use_fixed_max_mp; // use a permenant max hp. Setting this to true will cause multiplier and reduction to be ignored;
    FLOAT fixed_max_mp ;    // permenant max mp. Need use_fixed_max_mp to be TRUE
    FLOAT max_mp_multiplier; // max mp is multiplied by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied before the reduction
    FLOAT max_mp_reduction;  // max mp is reduced by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied after the multiplier

    FLOAT max_hp_multiplier; // max hp is multiplied by this value

    FLOAT passive_mp_recovery_multiplier;

    bool enable_mp_recovery_on_hit;

    FLOAT fixed_mp_recovery_on_hit;
    FLOAT multiplier_mp_recovery_on_hit; // amount added on hit as multiplier of max mp

    int LoadFromFile(const std::string& path = "NieR_Replicant_Hardcore.ini") {  // ret 0 = values loaded from file,   ret 1 = default values loaded
        INIReader reader(path);

        if (reader.ParseError() != 0) {

            if (reader.ParseError() > 0) {
                MessageBoxW(0, (std::wstring(L"Error when reading INI file. Using default values. Error on line:") + std::to_wstring(reader.ParseError())).c_str(), L" Minhook Error", 0);

            }

            debug = false;      // opens terminal for debugging

            mp_multiplier_required_for_item = 1; // using item will fail if current mp is below this multiplier 
            mp_multiplier_on_item_use = 0.0; // on item use, currrent mp is multiplied by this value. applied before reduxtion
            mp_reduction_on_item_use = 0;    // use this and set multiplier to 1.0 if you want a fixed reduction on item use. Or use a combination of both. applied after the multiplier

            use_fixed_max_mp = true; // use a permenant max hp. Setting this to true will cause multiplier and reduction to be ignored;
            fixed_max_mp = 100;    // permenant max mp. Need use_fixed_max_mp to be TRUE
            max_mp_multiplier = 1; // max mp is multiplied by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied before the reduction
            max_mp_reduction = 0;  // max mp is reduced by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied after the multiplier

            max_hp_multiplier = 0.4375; // max hp is multiplied by this value

            passive_mp_recovery_multiplier = 0.4375;

            enable_mp_recovery_on_hit = true;

            fixed_mp_recovery_on_hit = 0;
            multiplier_mp_recovery_on_hit = 0.0625; // amount added on hit as multiplier of max mp
            return 1;
        }
        else if (reader.ParseError() == 0) {
            debug = reader.GetBoolean("main", "debug", false);

            mp_multiplier_required_for_item = reader.GetReal("main", "mp_multiplier_required_for_item", 0.99f);
            mp_multiplier_on_item_use = reader.GetReal("main", "mp_multiplier_on_item_use", 0.0);
            mp_reduction_on_item_use = reader.GetReal("main", "mp_reduction_on_item_use", 0.0);

            use_fixed_max_mp = reader.GetBoolean("main", "use_fixed_max_mp", true);
            fixed_max_mp = reader.GetReal("main", "fixed_max_mp", 100);
            max_mp_multiplier = reader.GetReal("main", "max_mp_multiplier", 1);
            max_mp_reduction = reader.GetReal("main", "max_mp_reduction", 0);

            max_hp_multiplier = reader.GetReal("main", "max_hp_multiplier", 0.4375);

            passive_mp_recovery_multiplier = reader.GetReal("main", "passive_mp_recovery_multiplier", 0.4375);

            enable_mp_recovery_on_hit = reader.GetBoolean("main", "enable_mp_recovery_on_hit", true);

            fixed_mp_recovery_on_hit = reader.GetReal("main", "fixed_mp_recovery_on_hit", 0);
            multiplier_mp_recovery_on_hit = reader.GetReal("main", "multiplier_mp_recovery_on_hit", 0.0625);
            return 0;
        }


    }
};


Settings settings;
LPCWSTR ProcessName = L"NieR Replicant ver.1.22474487139.exe";                      
uintptr_t processBaseAddress = (uintptr_t)GetModuleHandleW(ProcessName);
HANDLE handle = GetCurrentProcess();


typedef uint64_t(__fastcall* use_item)(int64_t, int32_t,int64_t,int64_t);                           
use_item use_item_hooked = use_item(processBaseAddress + 0x3b6fb0);         
use_item use_item_original;
uint64_t __fastcall use_item_detoured(int64_t param_1, int32_t item_id, int64_t item_inventory_index, uint64_t param_4)                                     
{
    // param_1: ??? 
    // item_id: fixed id for the item used
    // item_inventory_order: index of the item in the players inventory. Can change if you have 0 of certain items, the items afterwards get pushed forwards
    // param_4: ???
    // return: 1 on successful item use. returns 0 if you try to cure poison when you are not poisoned. returns 162048 if you try use an item when health is full

    std::cout << "use_item" << "\n" << param_1 << "\n" << item_id << "\n" << item_inventory_index << "\n" << param_4 << "\n\n";

    float* current_mp = (float*)(processBaseAddress + 0x4374A78);
    float* fixed_max_mp = (float*)(processBaseAddress + 0x122D2DC);
    DWORD* current_hp = (DWORD*)(processBaseAddress + 0x4374A6C);
    DWORD* max_hp = (DWORD*)(processBaseAddress + 0x122D2D8);
    float* kaine_health = (float*)(processBaseAddress + 0x4375855);

    if (item_id < 3 and *current_hp == *max_hp) { // if item used is a healing one and the player is full health
        return 162048;
    }

    float epsilon = 0.02f;
    bool has_enough_mp = (*current_mp >= *fixed_max_mp * (settings.mp_multiplier_required_for_item-epsilon));
    bool is_kaine = *kaine_health > 0;

    if (has_enough_mp or is_kaine) { // mp requirement ignored if playing as kaine
        *current_mp = (*current_mp * settings.mp_multiplier_on_item_use) - settings.mp_reduction_on_item_use;
        return use_item_original(param_1, item_id, item_inventory_index, param_4);
    }

    return 162048;
}


typedef void(__fastcall* change_max_hp)(int64_t*, int32_t);
change_max_hp change_max_hp_hooked = change_max_hp(processBaseAddress + 0x4084b0);
change_max_hp change_max_hp_original;
void __fastcall change_max_hp_detoured(int64_t* param_1, int32_t param_2) {

    // param_1 ???
    // param_2  base value for calculating HP

    std::cout << "change_max_hp" << "\n" << param_1 << "\n" << param_2 << "\n\n";

   DWORD max_hp = 0;
   change_max_hp_original(param_1, (int32_t)((float)param_2*settings.max_hp_multiplier));

}

typedef void(__fastcall* change_max_mp)(int64_t*, int32_t, char, char);
change_max_mp change_max_mp_hooked = change_max_mp(processBaseAddress + 0x407ad0);
change_max_mp change_max_mp_original;
void __fastcall change_max_mp_detoured(int64_t* param_1, int32_t param_2, char param_3, char param_4) {

    //param_1: ???
    //param_2: base value for calculating MP
    //param_4: ???

    if (settings.debug) std::cout << "change_max_mp" << "\n" << *param_1 << "\n" << param_2 << "\n" << (int)param_3 << "\n" << (int)param_4 << "\n\n";

    change_max_mp_original(param_1, param_2, param_3, param_4); 

    float* max_mp = (float*)(processBaseAddress + 0x122D2DC);
    if (settings.use_fixed_max_mp==FALSE) {
        float* current_mp = (float*)(processBaseAddress + 0x4374A78);
        *max_mp = (*current_mp * settings.max_mp_multiplier) - settings.max_mp_reduction;
    }
    else {
        *max_mp = settings.fixed_max_mp;
    }
}


typedef void(__fastcall* change_current_mp)(int64_t, uint32_t, float);
change_current_mp change_current_mp_hooked = change_current_mp(processBaseAddress + 0x3be2a0);
change_current_mp change_current_mp_original;
void __fastcall change_current_mp_detoured(int64_t param_1, uint32_t param_2, float param_3) {

    //param_1: ???
    //param_2: ???
    //param_3: new_mp

    //if (Settings::debug) std::cout << "change_current_mp" << "\n" << param_1 << "\n" << param_2 << "\n" << param_3 << "\n\n";

    float* current_mp = (float*)(processBaseAddress + 0x4374A78);

    if ((*current_mp + 2 > param_3) and (param_3 > *current_mp)) {     //if new mp is greater than current, but not too great otherwise it would be Orb / postbox recovery
        change_current_mp_original(param_1, param_2, (param_3-*current_mp)* settings.passive_mp_recovery_multiplier+*current_mp);
    }
    else {
        change_current_mp_original(param_1, param_2, param_3);
    }
}


typedef void(__fastcall* hit_enemy)(int64_t**);
hit_enemy hit_enemy_hooked = hit_enemy(processBaseAddress + 0x6b2180);
hit_enemy hit_enemy_original;
void __fastcall hit_enemy_detoured(int64_t** param_1) {
    std::cout << "hit_enemy" << "\n" << param_1 << "\n\n";


    float* current_mp = (float*)(processBaseAddress + 0x4374A78);
    float* max_mp = (float*)(processBaseAddress + 0x122D2DC);
    *current_mp = *current_mp + (*max_mp * settings.multiplier_mp_recovery_on_hit) + settings.fixed_mp_recovery_on_hit;
    
    hit_enemy_original(param_1);
}

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

DWORD WINAPI SetupHooks(LPVOID lpParam) {


    int ret;
    ret = settings.LoadFromFile();

    if (settings.debug) {
        AllocConsole();
        FILE* console;
        freopen_s(&console, "CONOUT$", "w", stdout);
    }


    if (ret == 0) {
        std::cout << "settings loaded from ini file \n" ;
    }
    else {
        std::cout << "could not load values from file, default values loaded instead \n";
    }
    
    ret = MH_Initialize();
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when initializing minhook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }
    std::cout << "initialized minhook" << "\n";


    ret = MH_CreateHookEx(use_item_hooked, &use_item_detoured, &use_item_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating use_item hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(change_max_mp_hooked, &change_max_mp_detoured, &change_max_mp_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating change_max_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(change_max_hp_hooked, &change_max_hp_detoured, &change_max_hp_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating change_max_hp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(change_current_mp_hooked, &change_current_mp_detoured, &change_current_mp_original);
    if (ret != MH_OK)
    {
        MessageBoxW(0, (std::wstring(L"Error when creating change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
        return FALSE;
    }

    ret = MH_CreateHookEx(hit_enemy_hooked, &hit_enemy_detoured, &hit_enemy_original);
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

#include <Windows.h>
#include "MinHook.h"
#include<string>
#include<iostream>
#include<fstream>

void print_hex(const char* string)
{
    unsigned char* p = (unsigned char*)string;

    for (int i = 0; i < strlen(string); ++i) {
        if (!(i % 16) && i)
            printf("\n");

        printf("0x%02x ", p[i]);
    }
    printf("\n\n");
}



namespace Settings{
        const BOOL debug = FALSE;      // opens terminal for debugging

        const FLOAT mp_multiple_required_for_item = 0.999; // using item will fail if current mp is below this multiplier (due to implementation of change_current_mp hook, this shouldn't be set to exactly 1, it should be a little less)
        const FLOAT mp_multiplier_on_item_use = 0.0; // on item use, currrent mp is multiplied by this value. applied before reduxtion
        const FLOAT mp_reduction_on_item_use = 0;    // use this and set multiplier to 1.0 if you want a fixed reduction on item use. Or use a combination of both. applied after the multiplier

        const BOOL use_fixed_max_mp = TRUE; // use a permenant max hp. Setting this to true will cause multiplier and reduction to be ignored;
        const FLOAT fixed_max_mp = 100;    // permenant max mp. Need use_fixed_max_mp to be TRUE
        const FLOAT max_mp_multiplier = 1; // max mp is multiplied by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied before the reduction
        const FLOAT max_mp_reduction = 0;  // max mp is reduced by this value. fixed_max_mp must be set to FALSE for this to do anything. Applied after the multiplier

        const FLOAT max_hp_multiplier = 0.4375; // max hp is multiplied by this value
     
        const FLOAT passive_mp_recovery_multiplier = 0.4375;

        const BOOL enable_mp_recovery_on_hit = TRUE;
         
        const FLOAT fixed_mp_recovery_on_hit = 0;
        const FLOAT multiplier_mp_recovery_on_hit = 0.0625; // amount added on hit as multiplier of max mp
};  

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

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

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
    
    float current_mp = 0;
    float fixed_max_mp = 0;
    DWORD current_hp = 0;
    DWORD max_hp = 0;
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x122D2DC), &fixed_max_mp, sizeof(float), NULL);
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &current_mp, sizeof(float), NULL);
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x122D2D8), &max_hp, sizeof(DWORD), NULL);
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A6C), &current_hp, sizeof(DWORD), NULL);
    
    if (item_id < 3 and current_hp == max_hp) { // if item used is a healing one and the player is full health
        return 162048;
    }

    char current_weapon[50];
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x27FA39C), &current_weapon, 50, NULL);
    char cmp_string[50] = {0x4b, 0x61, 0x69, 0x6e, 0xc3, 0xa9, 0x27, 0x73, 0x20, 0x53, 0x77, 0x6f, 0x72, 0x64};


    if ((current_mp >= fixed_max_mp*Settings::mp_multiple_required_for_item) or (strcmp(cmp_string, current_weapon) == 0)) {  // Checking the player has the required mp or if in certain parts of playthrough E

        float new_mp = (current_mp * Settings::mp_multiplier_on_item_use) - Settings::mp_reduction_on_item_use;
        WriteProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &new_mp, sizeof(float), NULL);
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
   change_max_hp_original(param_1, (int32_t)((float)param_2*Settings::max_hp_multiplier));

}

typedef void(__fastcall* change_max_mp)(int64_t*, int32_t, char, char);
change_max_mp change_max_mp_hooked = change_max_mp(processBaseAddress + 0x407ad0);
change_max_mp change_max_mp_original;
void __fastcall change_max_mp_detoured(int64_t* param_1, int32_t param_2, char param_3, char param_4) {
    //param_1: ???
    //param_2:  base value for calculating MP
    //param_4: ???

    if (Settings::debug) std::cout << "change_max_mp" << "\n" << *param_1 << "\n" << param_2 << "\n" << (int)param_3 << "\n" << (int)param_4 << "\n\n";

    change_max_mp_original(param_1, param_2, param_3, param_4); 

    float new_mp;
    if (Settings::use_fixed_max_mp==FALSE) {
        float current_mp = 0;
        ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &current_mp, sizeof(float), NULL);
        new_mp = (current_mp * Settings::max_mp_multiplier) - Settings::max_mp_reduction;
    }
    else {
        new_mp = Settings::fixed_max_mp;
    }
    WriteProcessMemory(handle, (PVOID)(processBaseAddress + 0x122D2DC), &new_mp, sizeof(float), NULL);
}


typedef void(__fastcall* change_current_mp)(int64_t, uint32_t, float);
change_current_mp change_current_mp_hooked = change_current_mp(processBaseAddress + 0x3be2a0);
change_current_mp change_current_mp_original;
void __fastcall change_current_mp_detoured(int64_t param_1, uint32_t param_2, float param_3) {
    //param_1: ???
    //param_2: ???
    //param_3: new_mp

    //if (Settings::debug) std::cout << "change_current_mp" << "\n" << param_1 << "\n" << param_2 << "\n" << param_3 << "\n\n";

    float current_mp = 0;
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &current_mp, sizeof(float), NULL);
    if ((current_mp + 2 >param_3) and (param_3 > current_mp)) {     //if new mp is greater than current, but not too great otherwise it would be Orb / postbox recovery
        change_current_mp_original(param_1, param_2, (param_3-current_mp)*Settings::passive_mp_recovery_multiplier+current_mp);
    }
    else {
        change_current_mp_original(param_1, param_2, param_3);
    }
}


typedef void(__fastcall* hit_enemy)(int64_t**);
hit_enemy hit_enemy_hooked = hit_enemy(processBaseAddress + 0x6b2180);
hit_enemy hit_enemy_original;
void __fastcall hit_enemy_detoured(int64_t** param_1) {
    if (Settings::debug) std::cout << "hit_enemy" << "\n" << param_1 << "\n\n";


    float current_mp = 0;
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &current_mp, sizeof(float), NULL);
    float max_mp = 0;
    ReadProcessMemory(handle, (PVOID)(processBaseAddress + 0x122D2DC), &max_mp, sizeof(float), NULL);
    float new_mp = current_mp + (max_mp * Settings::multiplier_mp_recovery_on_hit) + Settings::fixed_mp_recovery_on_hit;
    WriteProcessMemory(handle, (PVOID)(processBaseAddress + 0x4374A78), &new_mp, sizeof(float), NULL);
    
    hit_enemy_original(param_1);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (Settings::debug) {
        AllocConsole();
        FILE* console;
        freopen_s(&console, "CONOUT$", "w", stdout);
    }
    int ret;
    std::string error;
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        if (processBaseAddress == 0) {
            return FALSE;
        }
        
        ret = MH_Initialize();
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when initializing minhook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "initialized minhook" << "\n";
        if (DisableThreadLibraryCalls(hModule) == FALSE) {
            error = GetLastErrorAsString();
            MessageBoxW(0, (std::wstring(L"Error when disabling thread library calls. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L" Windows Error", 0);
            return FALSE;
        }
        std::cout << "disabled thread library calls" << "\n";

        ret = MH_CreateHookEx(use_item_hooked, &use_item_detoured, &use_item_original);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when creating use_item hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "use_item hook created" << "\n";

        ret = MH_EnableHook(use_item_hooked);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when enabling use_item hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "use_item hook enabled" << "\n";

        ret = MH_CreateHookEx(change_max_mp_hooked, &change_max_mp_detoured, &change_max_mp_original);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when creating change_max_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_max_mp hook created" << "\n";
        ret = MH_EnableHook(change_max_mp_hooked);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when enabling change_max_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_max_mp hook enabled" << "\n";

        ret = MH_CreateHookEx(change_max_hp_hooked, &change_max_hp_detoured, &change_max_hp_original);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when creating change_max_hp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_max_hp hook created" << "\n";
        ret = MH_EnableHook(change_max_hp_hooked);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when enabling change_max_hp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_max_hp hook enabled" << "\n";

        ret = MH_CreateHookEx(change_current_mp_hooked, &change_current_mp_detoured, &change_current_mp_original);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when creating change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_current_mp hook created" << "\n";
        ret = MH_EnableHook(change_current_mp_hooked);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when enabling change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "change_current_mp hook enabled" << "\n";

        ret = MH_CreateHookEx(hit_enemy_hooked, &hit_enemy_detoured, &hit_enemy_original);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when creating enemy_hit_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "enemy_hit hook created" << "\n";
        ret = MH_EnableHook(hit_enemy_hooked);
        if (ret != MH_OK)
        {
            MessageBoxW(0, (std::wstring(L"Error when enabling enemy_hit hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
            return FALSE;
        }
        std::cout << "enemy_hit hook enabled" << "\n";
    }

        
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
          
          ret = MH_DisableHook(use_item_hooked);
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when disabling use_item hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
              return FALSE;
          }     

          std::cout << "disabled use_item hook" << "\n";
          ret = MH_DisableHook(change_max_hp_hooked);
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when disabling change_max_hp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);     

              return FALSE;
          }
          std::cout << "disabled change_max_hp hook" << "\n";       

          ret = MH_DisableHook(change_max_mp_hooked);
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when disabling change_max_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
              return FALSE;
          }
          std::cout << "disabled change_max_mp hook" << "\n";

          ret = MH_DisableHook(change_current_mp_hooked);
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when disabling change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
              return FALSE;
          }
          std::cout << "disabled change_current_mp hook" << "\n";

          ret = MH_DisableHook(hit_enemy_hooked);
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when disabling change_current_mp hook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);
              return FALSE;
          }
          std::cout << "disabled enemy_hit hook" << "\n";
        

          ret = MH_Uninitialize();
          if (ret != MH_OK)
          {
              MessageBoxW(0, (std::wstring(L"Error when uninitializing minhook. Error code:") + std::to_wstring(ret)).c_str(), L" Minhook Error", 0);       
              return FALSE;
          }
          std::cout << "uninitialized minhook" << "\n";
          
        }
  
    return TRUE;
}

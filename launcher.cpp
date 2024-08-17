#include<Windows.h>
#include<tlhelp32.h>
#include<detours/detours.h>
#include<string>
#include<iostream>


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

STARTUPINFO si;
PROCESS_INFORMATION pi;
LPCSTR app_path = "NieR Replicant ver.1.22474487139.exe";
LPCSTR dll_path = "NieR_Replicant_Hardcore.dll";




BOOL main(int argc, char* argv[])
{
    if (GetFileAttributes(dll_path) == INVALID_FILE_ATTRIBUTES) {
        MessageBoxW(0, L"Error when launching process. Error message: dll not found", L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }


    BOOL ret = CreateProcess(
        app_path,       // Application name
        NULL,           // Command line
        NULL,           // Process Attributes
        NULL,           // Thread Attributes
        FALSE,          // handle inheritance
        0,              // creation flags
        NULL,           // environment
        NULL,           // starting directory
        &si,            // Startup info
        &pi             // Process Information
        );
   
    std::string error;
    if (!ret) {
        
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when launching process. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }

    /*
    The following section is necesary becuase sometimes the process immediately spawned by the executable is only a loader that loads the main game process and then terminates.
    This code waits 10 seconds for the main window to appear, finds process id from that window handle and then the process handle from the id
    */
    Sleep(7500);
    HWND window_handle = FindWindowW(NULL, L"NieR Replicant ver.1.22474487139...");
    if (window_handle == NULL) {
        MessageBoxW(0, L"Error when opening window handle. Error message: Handle is null.", L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }
    DWORD process_id;
    GetWindowThreadProcessId(window_handle, &process_id);
    if (process_id == NULL) {
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when getting window thread process id. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }
    HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);

    if (process_handle == NULL) {
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when opening handle. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }
    

    LPVOID lpBaseAddress = VirtualAllocEx(process_handle, NULL, (int64_t)1 << 12, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (lpBaseAddress == NULL) {
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when allocating memory in target process. Error message: ") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }

    if (!WriteProcessMemory(process_handle, lpBaseAddress, (LPCVOID)dll_path, strlen(dll_path), nullptr)) {
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when overwriting arguments in target process. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
    }

    HMODULE hKernel32 = GetModuleHandleW(L"Kernel32");
    if (hKernel32 == NULL) {
        VirtualFreeEx(process_handle, lpBaseAddress, 0x0, MEM_RELEASE);
        lpBaseAddress = NULL;

        CloseHandle(process_handle);
        process_handle = NULL;
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when loading dll into target process. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }
    FARPROC pLoadLibraryA = GetProcAddress(hKernel32, "LoadLibraryA");
    if (pLoadLibraryA == NULL) {
        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when finding address of LoadLibraryW. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }

    
    HANDLE thread_handle = CreateRemoteThread(process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)pLoadLibraryA, lpBaseAddress, NULL, NULL);
    if (thread_handle == NULL) {
        VirtualFreeEx(process_handle, lpBaseAddress, 0x0, MEM_RELEASE);
        lpBaseAddress = nullptr;

        CloseHandle(process_handle);
        process_handle = NULL;

        error = GetLastErrorAsString();
        MessageBoxW(0, (std::wstring(L"Error when creating remote thread. Error message:") + std::wstring(error.begin(), error.end())).c_str(), L"nnr_hardcore_launcher.exe Windows Error", 0);
        return FALSE;
    }


    
    return TRUE;
}



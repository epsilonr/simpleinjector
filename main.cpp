#include <iostream>

#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

DWORD Process(const char* processName) {
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	do {
		if (!strcmp(entry.szExeFile, processName)) {
			DWORD dwId = entry.th32ProcessID;
			CloseHandle(hProcessId);
			return dwId;
		}
	} while (Process32Next(hProcessId, &entry));
	return 0;
}

int main() {
	cout << "Your dll must be named like 'default.dll'" << endl;
	char dllPath[MAX_PATH];
	GetFullPathName("default.dll", sizeof(dllPath), dllPath, 0);

	DWORD dwProcess = Process("csgo.exe");
	cout << "Waiting for csgo.exe..." << endl;
	while (dwProcess == 0) {
		Sleep(100);
		dwProcess = Process("csgo.exe");
	}
	cout << "csgo.exe has been found." << endl;

	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, dwProcess);

	LPVOID ntOpenFile = GetProcAddress(LoadLibraryW(L"ntdll"), "NtOpenFile");
	if (ntOpenFile) {
		char originalBytes[5];
		memcpy(originalBytes, ntOpenFile, 5);
		WriteProcessMemory(hProcess, ntOpenFile, originalBytes, 5, NULL);
	}

	LPVOID allocatedMem = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(hProcess, allocatedMem, dllPath, sizeof(dllPath), NULL);
	CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, allocatedMem, 0, 0);

	cout << "Injected succesfully." << endl;

	Sleep(3000);
	CloseHandle(hProcess);
	return 0;
}

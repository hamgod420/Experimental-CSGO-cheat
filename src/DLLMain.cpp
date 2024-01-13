#include <thread>
#include "Hooks.h"
#include "Utils\Utils.h"
#include "Utils\GlobalVars.h"

HINSTANCE HThisModule;

DWORD WINAPI attach(void* instance) {
	while (!GetModuleHandleW(L"serverbrowser.dll"))
		Sleep(200);

	try {
		Hooks::Init();
	}

	catch (const std::runtime_error& err) {
		FreeLibraryAndExitThread(static_cast<HMODULE>(instance), 0);
	}
	
	while (!GetAsyncKeyState(VK_END))
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

	FreeLibraryAndExitThread(static_cast<HMODULE>(instance), 0);
}

BOOL WINAPI detach() {
	Hooks::Restore();
	return TRUE;
}

BOOL APIENTRY DllMain(void* instance, uintptr_t reason, void* reserved) {
	DisableThreadLibraryCalls(static_cast<HMODULE>(instance));

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, attach, instance, NULL, nullptr);
		break;

	case DLL_PROCESS_DETACH:
		detach();
		break;
	}

	return true;
}

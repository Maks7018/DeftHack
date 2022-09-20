#include <Windows.h>

#include "DeftHack_fixed.h"

using mono_get_root_domain_type = void* (__cdecl*)();
using mono_thread_attach_type = void* (__cdecl*)(void* domain);
using mono_image_open_from_data_type = void* (__cdecl*)(void* data, uint32_t data_len, uint32_t need_copy, uint32_t* status);
using mono_assembly_load_from_type = void* (__cdecl*)(void* image, const char* fname, uint32_t* status);
using mono_class_from_name_type = void* (__cdecl*)(void* image, const char* name_space, const char* name);
using mono_class_get_method_from_name_type = void* (__cdecl*)(void* klass, const char* name, int param_count);
using mono_runtime_invoke_type = void* (__cdecl*)(void* method, void* obj, void** params, void** exc);

unsigned long main_thread( void* )
{
	HMODULE monoModuleHandle = GetModuleHandle(L"mono-2.0-bdwgc.dll");
	if (monoModuleHandle == nullptr)
	{
		return -1;
	}

	mono_get_root_domain_type mono_get_root_domain = reinterpret_cast<mono_get_root_domain_type>(
		GetProcAddress(monoModuleHandle, "mono_get_root_domain"));
	mono_thread_attach_type mono_thread_attach = reinterpret_cast<mono_thread_attach_type>(
		GetProcAddress(monoModuleHandle, "mono_thread_attach"));
	mono_image_open_from_data_type mono_image_open_from_data = reinterpret_cast<mono_image_open_from_data_type>(
		GetProcAddress(monoModuleHandle, "mono_image_open_from_data"));
	mono_assembly_load_from_type mono_assembly_load_from = reinterpret_cast<mono_assembly_load_from_type>(
		GetProcAddress(monoModuleHandle, "mono_assembly_load_from"));
	mono_class_from_name_type mono_class_from_name = reinterpret_cast<mono_class_from_name_type>(
		GetProcAddress(monoModuleHandle, "mono_class_from_name"));
	mono_class_get_method_from_name_type mono_class_get_method_from_name = reinterpret_cast<mono_class_get_method_from_name_type>(
		GetProcAddress(monoModuleHandle, "mono_class_get_method_from_name"));
	mono_runtime_invoke_type mono_runtime_invoke = reinterpret_cast<mono_runtime_invoke_type>(
		GetProcAddress(monoModuleHandle, "mono_runtime_invoke"));

	mono_thread_attach(mono_get_root_domain());

	void* image = mono_image_open_from_data(DeftHack_fixed, sizeof(DeftHack_fixed), 1, nullptr);
	mono_assembly_load_from(image, "DeftHack.dll", nullptr);

	void* method = mono_class_get_method_from_name(
		mono_class_from_name(image, "SosiHui", "BinaryOperationBinder"), "DynamicObject", 0);

	mono_runtime_invoke(method, nullptr, nullptr, nullptr);

	PVOID imageBase;
	FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(RtlPcToFileHeader(main_thread, &imageBase)), 0);
	return 0;
}

bool DllMain( HMODULE module_instance, DWORD call_reason, void* )
{
	if ( call_reason != DLL_PROCESS_ATTACH )
		return false;

	wchar_t file_name[ MAX_PATH ] = L"";
	GetModuleFileNameW( module_instance, file_name, _countof( file_name ) );
	LoadLibraryW( file_name );

	return true;
}

extern "C" __declspec( dllexport )
LRESULT wnd_hk( int code, WPARAM wparam, LPARAM lparam )
{
	// handle race condition from calling hook multiple times
	static auto done_once = false;

	const auto pmsg = reinterpret_cast< MSG* >( lparam );

	if ( !done_once && pmsg->message == 0x5b0 )
	{
		UnhookWindowsHookEx( reinterpret_cast< HHOOK >( lparam ) );

		// you can just one line this since CloseHandle doesn't throw unless it's under debug mode
		if ( const auto handle = CreateThread( nullptr, 0, &main_thread, nullptr, 0, nullptr ); handle != nullptr )
			CloseHandle( handle );

		done_once = true;
	}

	// call next hook in queue
	return CallNextHookEx( nullptr, code, wparam, lparam );
}

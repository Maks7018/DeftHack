#include <chrono>
#include <thread>
#include "be_bypass.hpp"

#pragma comment(lib, "LDE64x64.lib")

int wmain( int argc, wchar_t** argv )
{
	const auto dll_name = L"CodakCheat.dll";
	const auto window_class = L"UnityWndClass";

	if ( !std::filesystem::exists( dll_name ) )
	{
		printf( "[!] dll path supplied does not exist" );
		return -1;
	}

	printf( "[~] spenldid implanter poc\n" );

	// we need to enable the debug privilege
	if ( !impl::enable_privilege( L"SeDebugPrivilege" ) )
		return -1;

	printf( "[~] enabled debug privilege!\n" );

	//
	// summary:
	//		get BEService.exe data: process id, base address, read bytes from disk then find a suitable executable section.
	//
	if ( !be_bypass::initialize( ) )
		return -1;

	//
	// summary:
	//		find full path to strings, allocate space in the process for them, then copy them in.
	//
	if ( !be_bypass::prepare_image( dll_name ) )
		return -1;

	//
	// summary:
	//		prepare the shellcode, find a place to deploy it in, deploy it, then hook the original.
	//
	if ( !be_bypass::deploy_image( ) )
		return -1;

	//
	// summary:
	//		wait on the game to start, load the image in, get the needed export then call it.
	//
	if ( !be_bypass::inject_image( window_class, dll_name ) )
		return -1;

	printf( "[~] splendid implanter out!\n" );

	return 0;
}
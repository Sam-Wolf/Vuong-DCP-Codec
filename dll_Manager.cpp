#include "lagarith.h"
#include <sstream>
#include "log.h"
#include <filesystem>
namespace fs = std::experimental::filesystem;

unsigned int notification_level = 3;
unsigned long compression_method1 = 'LAGS';
unsigned long compression_method2 = 'LAGS';
unsigned long compression_method3 = 'LAGS';
char dll_path[MAX_PATH] = "";


unsigned long decompression_method1 = 'LAGS';
unsigned long decompression_method2 = 'LAGS';
unsigned long decompression_method3 = 'LAGS';



//Added by Samuel Wolf - Begin
DLL_Info dll_info[256];



bool contains_DLL_ID(DWORD Compression_ID, unsigned int length) {
	for (unsigned int i = 0; i < length; i++)
	{
		if (Compression_ID == dll_info[i].variant_IDs)
			return 1;
	}

	return 0;
}

unsigned int LoadDLLS() {

	std::string path = "H:\\comp_dll\\";

	f_Name Name;
	f_DWORD ID;

	int count_dll = 0;
	for (auto & p : fs::directory_iterator(path)) {
		count_dll++;
	}


	int i = 0;
	for (auto & p : fs::directory_iterator(path)) {

		const char * dll_path = (p.path().string().c_str());

		HINSTANCE hGetProcIDDLL = LoadLibrary((p.path().string().c_str()));
		if (!hGetProcIDDLL) {
			if (notification_level > 3)
				MessageBoxA(NULL, "could not load the dynamic library", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
		}
		else {

			// resolve function address here
			Name = (f_Name)GetProcAddress(hGetProcIDDLL, "GetCompressName");
			if (!Name) {
				if (notification_level > 0)
					MessageBoxA(NULL, "could not locate the function", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			}

			ID = (f_DWORD)GetProcAddress(hGetProcIDDLL, "GetCompressDWORD");
			if (!ID) {
				if (notification_level > 0)
					MessageBoxA(NULL, "could not locate the function - DWORD", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			}

			if (Name && ID) {

				if (!contains_DLL_ID(ID(), i)) {
					dll_info[i].variant_Names = new char[256];
					strcpy(dll_info[i].variant_Names,Name());
					dll_info[i].variant_IDs = ID();
					dll_info[i].variant_Paths = new char[256];
					char test[256] = {};
					strcpy(dll_info[i].variant_Paths , dll_path);


					//MessageBoxA(NULL, dll_info[i].variant_Paths, "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
					i++;

				}
				
			}
		}

		FreeLibrary(hGetProcIDDLL);

	}

	return i;
}

unsigned int GetDll(DWORD Compression_ID, unsigned int length)
{
	for (unsigned int i = 0; i < length; i++)
	{
		if (Compression_ID == dll_info[i].variant_IDs)
			return i;
	}

	return length;
}
//Added by Samuel Wolf - End
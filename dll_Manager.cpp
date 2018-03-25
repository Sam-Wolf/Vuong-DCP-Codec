// Vuong-DCP Codec, copyright 2018 by Samuel Wolf
//
//This program is free software : you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.If not, see <http://www.gnu.org/licenses/>.


#include "lagarith.h"
#include <sstream>
#include <filesystem>
namespace fs = std::experimental::filesystem;

unsigned int notification_level = 3;
unsigned long compression_method1 = MAKEFOURCC('L','A','G','S');
unsigned long compression_method2 = MAKEFOURCC('L','A','G','S');
unsigned long compression_method3 = MAKEFOURCC('L','A','G','S');


unsigned long decompression_method1 = MAKEFOURCC('L','A','G','S');
unsigned long decompression_method2 = MAKEFOURCC('L','A','G','S');
unsigned long decompression_method3 = MAKEFOURCC('L','A','G','S');

DLL_Info dll_info[256];

char * LoadRegistrySettings() {
	HKEY regkey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\vuong-dcp", 0, KEY_READ, &regkey) == ERROR_SUCCESS) {
		char  data_path[260] = { 0 };
		DWORD cbValueLength = sizeof(data_path);
		RegQueryValueEx(regkey, "DLL_Path", NULL, NULL, reinterpret_cast<LPBYTE>(&data_path), &cbValueLength);
		RegCloseKey(regkey);

		if (notification_level >= 3)
			MessageBoxA(NULL, data_path, "Vuong-DCP", MB_ICONINFORMATION | MB_OK);


		return data_path;
	}

}

bool contains_DLL_ID(DWORD Compression_ID, unsigned int length) {
	for (unsigned int i = 0; i < length; i++)
	{
		if (Compression_ID == dll_info[i].variant_IDs)
			return 1;
	}

	return 0;
}

// Added by Samuel Wolf - Begin
unsigned int EncodeTemplate(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char *  ending = in + length;
	unsigned char * const count = out;

	do {
		// getting input data
		int in_val = *in;
		in++;

		//writing output data
		*out = in_val;
		out++;

	} while (in < ending);

	return (unsigned int)(out - count);
}

void DecodeTemplate(const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {

	const unsigned char * const ending = out + length;

	do {
		// getting input data
		int in_val = *in;
		in++;

		//writing output data
		*out = in_val;
		out++;

	} while (out < ending);
}
// Added by Samuel Wolf -End

DWORD CodecInst::Load_DLL_Funtions(DWORD compresseion_method, f_Encode * Encode, f_Decode * Decode){

	if (compresseion_method == MAKEFOURCC('N','O','N','E')) {
		*Encode = (f_Encode)[](const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
			return EncodeTemplate(in, out, length); };

		*Decode = (f_Decode)[](const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
			DecodeTemplate(in,out,length);
		};
	}
	else if (compresseion_method == MAKEFOURCC('L','A','G','S')) {
		//unsigned int a = 0;
		//*Encode = (f_Encode)[](const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
		//	return (unsigned int)0; };

		//*Decode = (f_Decode)[](const unsigned char * __restrict in, unsigned char * __restrict out, const unsigned int length) {
		//	DecodeTemplate(in, out, length);
		//};

	}
	else {
		int dll_count = LoadDLLS();
		unsigned int comp_index = GetDll(compresseion_method, dll_count);

		HINSTANCE hGetProc = LoadLibrary(dll_info[comp_index].variant_Paths);
		if (!hGetProc) {

			std::stringstream stream;
			stream << "Could not load the dynamic library \n"
				<< "Path:" << dll_info[comp_index].variant_Paths;
			std::string result(stream.str());
			MessageBoxA(NULL, result.c_str(), "Vuong-DCP", MB_ICONINFORMATION | MB_OK);

			return ICERR_ERROR;
		}
		// resolve function address here
		*Decode = (f_Decode)GetProcAddress(hGetProc, "Decode");
		if (!*Decode) {
			MessageBoxA(NULL, "Could not locate the DECODE function", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			return ICERR_ERROR;
		}

		// resolve function address here
		*Encode = (f_Encode)GetProcAddress(hGetProc, "Encode");
		if (!*Encode) {
			MessageBoxA(NULL, "Could not locate the ENCODE function", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			return ICERR_ERROR;
		}

		if (notification_level >= 2) {
			std::stringstream stream;
			stream << "Loaded DLL \n"
				"\n Name: " << dll_info[comp_index].variant_Names <<
				"\n ID:" << (char)(dll_info[comp_index].variant_IDs >> 0) << (char)(dll_info[comp_index].variant_IDs >> 8) << (char)(dll_info[comp_index].variant_IDs >> 16) << (char)(dll_info[comp_index].variant_IDs >> 24) <<
				"\n Path: " << dll_info[comp_index].variant_Paths;
			std::string result(stream.str());
			MessageBoxA(NULL, result.c_str(), "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
		}
	}

	return ICERR_OK;

}



unsigned int LoadDLLS() {

	dll_info[0].variant_Names = "NONE";
	dll_info[0].variant_IDs = MAKEFOURCC('N','O','N','E');
	dll_info[0].variant_Paths = "";

	dll_info[1].variant_Names = "Lagarith";
	dll_info[1].variant_IDs = MAKEFOURCC('L','A','G','S');
	dll_info[1].variant_Paths = "";

	char path[260] = "";
	strcpy(path,LoadRegistrySettings());

	f_Name Name;
	f_DWORD ID;

	if (notification_level >= 3)
			MessageBoxA(NULL, path, "Vuong-DCP", MB_ICONINFORMATION | MB_OK);

	int i = 2;
	for (auto & p : fs::directory_iterator(path)) {

		const char * dll_path = (p.path().string().c_str());

	

		HINSTANCE hGetProcIDDLL = LoadLibrary((p.path().string().c_str()));
		if (!hGetProcIDDLL) {
			if (notification_level >= 3)
				MessageBoxA(NULL, "could not load the dynamic library - Load dlls", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
		}
		else {

			// resolve function address here
			Name = (f_Name)GetProcAddress(hGetProcIDDLL, "getCompressName");
			if (!Name) {
				if (notification_level > 0)
					MessageBoxA(NULL, "could not locate the function", "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			}

			ID = (f_DWORD)GetProcAddress(hGetProcIDDLL, "getCompressID");
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
					strcpy(dll_info[i].variant_Paths , p.path().string().c_str());


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
//	Lagarith v1.3.25, copyright 2011 by Ben Greenwood.
//	http://lags.leetcode.net/codec.html
//
//  Modified by Samuel Wolf
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either version 2
//	of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#pragma once
#include "interface.h"
#include "lagarith.h"
#include <commctrl.h>
#include <shellapi.h>
#include <Windowsx.h>
#include <intrin.h>

// Added by Samuel Wolf - Begin
#include <sstream>
#include <filesystem>
#include <iostream>
#include <windows.h>
#include <shlobj.h>
namespace fs = std::experimental::filesystem;
// Added by Samuel Wolf - End

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}


#define return_badformat() return (DWORD)ICERR_BADFORMAT;
//#define return_badformat() { char msg[256];sprintf(msg,"Returning error on line %d", __LINE__);MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION); return (DWORD)ICERR_BADFORMAT; }

// Test for MMX, SSE, SSE2, and SSSE3 support
bool DetectFlags(){
	int CPUInfo[4];
	__cpuid(CPUInfo,1);
	//SSE3 = (CPUInfo[2]&(1<< 0))!=0;
	SSSE3= (CPUInfo[2]&(1<< 9))!=0;
#ifndef X64_BUILD
	SSE  = (CPUInfo[3]&(1<<25))!=0;
	SSE2 = (CPUInfo[3]&(1<<26))!=0;
	return (CPUInfo[3]&(1<<23))!=0;
#else
	return true;
#endif	
}

CodecInst::CodecInst(){
#ifndef LAGARITH_RELEASE
	if ( started == 0x1337){
		char msg[128];
		sprintf_s(msg,128,"Attempting to instantiate a codec instance that has not been destroyed");
		MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
	}
#endif

	buffer=NULL;
	prev=NULL;
	buffer2=NULL;
	length=0;
	nullframes=false;
	use_alpha=false;
	lossy_option=0;
	started=0;
	cObj.buffer=NULL;
	multithreading=0;
	memset(threads,0,sizeof(threads));
	performance.count=0;
	performance.time_a=0xffffffffffffffff;
	performance.time_b=0xffffffffffffffff;
	decode_DLL_Loaded = 0;
	comp_skip = 12;

}

HMODULE hmoduleLagarith=0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID) {
	hmoduleLagarith = (HMODULE) hinstDLL;
	return TRUE;
}


char *mode_options[]={"RGBA","RGB (Default)","YUY2","YV12"};


HWND CreateTooltip(HWND hwnd){
    // initialize common controls
	INITCOMMONCONTROLSEX	iccex;		// struct specifying control classes to register
    iccex.dwICC		= ICC_WIN95_CLASSES;
    iccex.dwSize	= sizeof(INITCOMMONCONTROLSEX);
    InitCommonControlsEx(&iccex);

#ifdef X64_BUILD
	HINSTANCE	ghThisInstance=(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
#else 
	HINSTANCE	ghThisInstance=(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);
#endif
    HWND		hwndTT;					// handle to the tooltip control

    // create a tooltip window
	hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
							hwnd, NULL, ghThisInstance, NULL);
	
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// set some timeouts so tooltips appear fast and stay long (32767 seems to be a limit here)
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM)(DWORD)TTDT_INITIAL, (LPARAM)500);
	SendMessage(hwndTT, TTM_SETDELAYTIME, (WPARAM)(DWORD)TTDT_AUTOPOP, (LPARAM)30*1000);

	return hwndTT;
}

struct { UINT item; UINT tip; } item2tip[] = {
	{ IDC_NULLFRAMES,	IDS_TIP_NULLFRAMES	},
	{ IDC_SUGGEST,		IDS_TIP_SUGGEST		},
	{ IDC_MULTI,		IDS_TIP_MULTI		}, 
	{ IDC_LOSSY_OPTIONS,IDS_TIP_LOSSY_OPTION},
	{ IDC_NOUPSAMPLE,	IDS_TIP_NOUPSAMPLE},
	{ 0,0 }
};

int AddTooltip(HWND tooltip, HWND client, UINT stringid){

#ifdef X64_BUILD
	HINSTANCE ghThisInstance=(HINSTANCE)GetWindowLongPtr(client,GWLP_HINSTANCE);
#else
	HINSTANCE ghThisInstance=(HINSTANCE)GetWindowLong(client, GWL_HINSTANCE);
#endif

	TOOLINFO				ti;			// struct specifying info about tool in tooltip control
    static unsigned int		uid	= 0;	// for ti initialization
	RECT					rect;		// for client area coordinates
	TCHAR					buf[2000];	// a static buffer is sufficent, TTM_ADDTOOL seems to copy it

	// load the string manually, passing the id directly to TTM_ADDTOOL truncates the message :(
	if ( !LoadString(ghThisInstance, stringid, buf, 2000) ) return -1;

	// get coordinates of the main client area
	GetClientRect(client, &rect);
	
    // initialize members of the toolinfo structure
	ti.cbSize		= sizeof(TOOLINFO);
	ti.uFlags		= TTF_SUBCLASS;
	ti.hwnd			= client;
	ti.hinst		= ghThisInstance;		// not necessary if lpszText is not a resource id
	ti.uId			= uid;
	ti.lpszText		= buf;

	// Tooltip control will cover the whole window
	ti.rect.left	= rect.left;    
	ti.rect.top		= rect.top;
	ti.rect.right	= rect.right;
	ti.rect.bottom	= rect.bottom;
	
	// send a addtool message to the tooltip control window
	SendMessage(tooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);	
	return uid++;
}

// Modified by Samuel Wolf 
void StoreRegistrySettings(bool nullframes, bool suggestrgb, bool multithread, bool noupsample, int mode, int comp1, int comp2, int comp3, int notification){
	DWORD dp;
	HKEY regkey;
	char * ModeStrings[4] = {"RGBA","RGB","YUY2","YV12"};

	if ( RegCreateKeyEx(HKEY_CURRENT_USER,"Software\\vuong-dcp",0,NULL,REG_OPTION_NON_VOLATILE,KEY_WRITE,NULL,&regkey,&dp) == ERROR_SUCCESS){
		RegSetValueEx(regkey,"NullFrames",0,REG_DWORD,(unsigned char*)&nullframes,4);
		RegSetValueEx(regkey,"SuggestRGB",0,REG_DWORD,(unsigned char*)&suggestrgb,4);
		RegSetValueEx(regkey,"Multithread",0,REG_DWORD,(unsigned char*)&multithread,4);
		RegSetValueEx(regkey,"NoUpsample",0,REG_DWORD,(unsigned char*)&noupsample,4);
		RegSetValueEx(regkey,"Mode",0,REG_SZ,(unsigned char*)ModeStrings[mode],4);

		// Added by Samuel Wolf - Begin
		RegSetValueEx(regkey, "CompressionMethod1", 0, REG_DWORD, (unsigned char*)&comp1,4);
		RegSetValueEx(regkey, "CompressionMethod2", 0, REG_DWORD, (unsigned char*)&comp2, 4);
		RegSetValueEx(regkey, "CompressionMethod3", 0, REG_DWORD, (unsigned char*)&comp3, 4);
		RegSetValueEx(regkey, "Notification_Level", 0, REG_DWORD, (unsigned char*)&notification, 4);
		// Added by Samuel Wolf - END

		RegCloseKey(regkey);
	}
}

//Added by Samuel Wolf
void StoreRegistrySettingsPATH(char * path, int size) {
	DWORD dp;
	HKEY regkey;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\vuong-dcp", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &regkey, &dp) == ERROR_SUCCESS) {

		RegSetValueEx(regkey, "DLL_Path", 0, REG_SZ, (unsigned char*)path, size);


		RegCloseKey(regkey);
	}
}

// Modified by Samuel Wolf 
void LoadRegistrySettings(bool * nullframes, bool * suggestrgb, bool * multithread, bool * noupsample, int * mode, int * comp1, int * comp2, int * comp3, char * path, unsigned int * notification){
	HKEY regkey;
	char * ModeStrings[4] = {"RGBA","RGB","YUY2","YV12"};
	unsigned char data[]={0,0,0,0,0,0,0,0};
	DWORD size=sizeof(data);
	if ( RegOpenKeyEx(HKEY_CURRENT_USER,"Software\\vuong-dcp",0,KEY_READ,&regkey) == ERROR_SUCCESS){
		if ( nullframes ){
			RegQueryValueEx(regkey,"NullFrames",0,NULL,data,&size);
			*nullframes = (data[0]>0);
			size=sizeof(data);
			*(int*)data=0;
		}
		if ( suggestrgb){
			RegQueryValueEx(regkey,"SuggestRGB",0,NULL,data,&size);
			*suggestrgb = (data[0]>0);
			size=sizeof(data);
			*(int*)data=0;
		}
		if ( multithread ){
			RegQueryValueEx(regkey,"Multithread",0,NULL,data,&size);
			*multithread = (data[0]>0);
			size=sizeof(data);
			*(int*)data=0;
		}
		if ( noupsample ){
			RegQueryValueEx(regkey,"NoUpsample",0,NULL,data,&size);
			*noupsample = (data[0]>0);
			size=sizeof(data);
			*(int*)data=0;
		}
		if ( mode ){
			RegQueryValueEx(regkey,"Mode",0,NULL,data,&size);

			int cmp = *(int*)data;
			if ( cmp == 'ABGR' ){
				*mode=0;
			} else if ( cmp == '2YUY'){
				*mode=2;
			} else if ( cmp == '21VY'){
				*mode=3;
			} else {
				*mode=1;
			}


			// Added by Samuel Wolf - Begin
			if (comp1) {
				RegQueryValueEx(regkey, "CompressionMethod1", 0, NULL, data, &size);

				int cmp = *(int*)data;
				*comp1 = cmp;
			}
			if (comp2) {
				RegQueryValueEx(regkey, "CompressionMethod2", 0, NULL, data, &size);

				int cmp = *(int*)data;
				*comp2 = cmp;
			}
			if (comp3) {
				RegQueryValueEx(regkey, "CompressionMethod3", 0, NULL, data, &size);

				int cmp = *(int*)data;
				*comp3 = cmp;
			}

			if (path) {
				char  data_path[260] = {0};
				DWORD cbValueLength = sizeof(data_path);
				RegQueryValueEx(regkey, "DLL_Path", NULL, NULL, reinterpret_cast<LPBYTE>(&data_path), &cbValueLength);
				path = (char*)data_path;
				
			}
			if (notification) {
				RegQueryValueEx(regkey, "Notification_Level", 0, NULL, data, &size);
				int d = *(int*)data;
				*notification = d;
			}
			// Added by Samuel Wolf - End

		}
		RegCloseKey(regkey);
	} else {
		bool nf = GetPrivateProfileInt("settings", "nullframes", false, "vuong-dcp.ini")>0;
		bool nu = GetPrivateProfileInt("settings", "noupsample", false, "vuong-dcp.ini")>0;
		bool suggest = GetPrivateProfileInt("settings", "suggest", false, "vuong-dcp.ini")>0;
		bool mt = GetPrivateProfileInt("settings", "multithreading", false, "vuong-dcp.ini")>0;
		int m = GetPrivateProfileInt("settings", "lossy_option", 1, "vuong-dcp.ini");
		if (m < 0 || m > 4){
			m=1;
		} else if ( m == 4){
			m=3;
		}	
		
		if ( nullframes ) *nullframes = nf;
		if ( suggestrgb ) *suggestrgb = suggest;
		if ( multithread) *multithread = mt;
		if ( mode )	*mode = m;

		// Added by Samuel Wolf - Begin
		int c1 = GetPrivateProfileInt("settings", "CompressionMethod1", 'ENON', "vuong-dcp.ini");
		int c2 = GetPrivateProfileInt("settings", "CompressionMethod2", 'ENON', "vuong-dcp.ini");
		int c3 = GetPrivateProfileInt("settings", "CompressionMethod3", 'ENON', "vuong-dcp.ini");
		char p[MAX_PATH] = {};
		GetPrivateProfileString("settings", "DLL_Path", "C:\\",p, sizeof("C:\\"),"vuong-dcp.ini" );
		int n = GetPrivateProfileInt("settings", "Notification_Level", 0, "vuong-dcp.ini");

		StoreRegistrySettings(nf,suggest,mt,nu,m,c1,c2,c3,n);
		StoreRegistrySettingsPATH(p, sizeof("C:\\"));
		if (comp1)	*comp1 = c1;
		if (comp2)	*comp2 = c2; 
		if (comp3)	*comp3 = c3;
		if (path)	path = p;
		if (notification)	*notification = n;
		// Added by Samuel Wolf - End
	}
}



// Modified by Samuel Wolf 
static BOOL CALLBACK ConfigureDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_INITDIALOG) {

		bool suggestrgb=false;
		bool noupsample=false;
		bool nullframes=false;
		bool multithread=false;
		int mode=1;
		int comp1 = 'ENON';
		int comp2 = 'ENON';
		int comp3 = 'ENON';
		char path_dll[MAX_PATH] = "";
		
		LoadRegistrySettings(&nullframes,&suggestrgb,&multithread,&noupsample,&mode, &comp1, &comp2, &comp3, path_dll,&notification_level);

		HWND hwndItem = GetDlgItem(hwndDlg, IDC_LOSSY_OPTIONS);
		for(int i=0; i<4; i++)
			SendMessage(hwndItem, CB_ADDSTRING, 0, (LPARAM)mode_options[i]);

		// Added by Samuel Wolf - Begin
		HWND notificationItem = GetDlgItem(hwndDlg, IDC_NOTIFICATION_OPTIONS);
		SendMessage(notificationItem, CB_ADDSTRING, 0, (LPARAM)"0");
		SendMessage(notificationItem, CB_ADDSTRING, 0, (LPARAM)"1");
		SendMessage(notificationItem, CB_ADDSTRING, 0, (LPARAM)"2");
		SendMessage(notificationItem, CB_ADDSTRING, 0, (LPARAM)"3");
		SendMessage(notificationItem, CB_SETCURSEL, notification_level, 1);


		HWND compItem1 = GetDlgItem(hwndDlg, IDC_VARIANT_OPTIONS);
		HWND compItem2 = GetDlgItem(hwndDlg, IDC_VARIANT_OPTIONS2);
		HWND compItem3 = GetDlgItem(hwndDlg, IDC_VARIANT_OPTIONS3);
		

		unsigned int dll_count = LoadDLLS();

		//MessageBoxA(NULL, dll_info[0].variant_Names, "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
		for (int i = 0; i < dll_count; i++) {
			SendMessage(compItem1, CB_ADDSTRING, 0, (LPARAM)dll_info[i].variant_Names);
			SendMessage(compItem2, CB_ADDSTRING, 0, (LPARAM)dll_info[i].variant_Names);
			SendMessage(compItem3, CB_ADDSTRING, 0, (LPARAM)dll_info[i].variant_Names);
	
			if (notification_level > 0) {
				std::stringstream stream;
				stream << "Loaded DLL \n"
					"\n Name: " << dll_info[i].variant_Names <<
					"\n ID:" << (char)(dll_info[i].variant_IDs >> 0) << (char)(dll_info[i].variant_IDs >> 8) << (char)(dll_info[i].variant_IDs >> 16) << (char)(dll_info[i].variant_IDs >> 24) <<
					"\n Path: " << dll_info[i].variant_Paths;
				std::string result(stream.str());
				MessageBoxA(NULL, result.c_str(), "Vuong-DCP", MB_ICONINFORMATION | MB_OK);
			}



		}
			

		HWND hwndEditBuffer = GetDlgItem(hwndDlg, IDC_LZ77_BUFFER_OPTIONS);
		HWND hwndEditLook = GetDlgItem(hwndDlg, IDC_LZ77_LOOHAHEAD_OPTIONS);

		comp1 = GetDll(comp1,dll_count);
		SendMessage(compItem1, CB_SETCURSEL, comp1, 1);

		comp2 = GetDll(comp2, dll_count);
		SendMessage(compItem2, CB_SETCURSEL, comp2, 1);

		comp3 = GetDll(comp3, dll_count);
		SendMessage(compItem3, CB_SETCURSEL, comp3, 1);
		// Added by Samuel Wolf - End

		CheckDlgButton(hwndDlg, IDC_NULLFRAMES,nullframes);
		CheckDlgButton(hwndDlg, IDC_SUGGEST,suggestrgb);
		CheckDlgButton(hwndDlg, IDC_MULTI,multithread);
		CheckDlgButton(hwndDlg, IDC_NOUPSAMPLE, noupsample);
		HWND suggest = GetDlgItem(hwndDlg, IDC_SUGGEST);
		Button_Enable(suggest,!noupsample);
		SendMessage(hwndItem, CB_SETCURSEL, mode, 1);
		
		HWND hwndTip = CreateTooltip(hwndDlg);
		for (int l=0; item2tip[l].item; l++ )
			AddTooltip(hwndTip, GetDlgItem(hwndDlg, item2tip[l].item),	item2tip[l].tip);
		SendMessage(hwndTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)350);
	} else if (uMsg == WM_COMMAND) {
		HWND suggest = GetDlgItem(hwndDlg, IDC_SUGGEST);
		Button_Enable(suggest,IsDlgButtonChecked(hwndDlg, IDC_NOUPSAMPLE) != BST_CHECKED);
		if (LOWORD(wParam)==IDC_OK){

			bool suggestrgb=(IsDlgButtonChecked(hwndDlg, IDC_SUGGEST) == BST_CHECKED);
			bool noupsample=(IsDlgButtonChecked(hwndDlg, IDC_NOUPSAMPLE) == BST_CHECKED);
			bool nullframes=(IsDlgButtonChecked(hwndDlg, IDC_NULLFRAMES) == BST_CHECKED);
			bool multithread=(IsDlgButtonChecked(hwndDlg, IDC_MULTI) == BST_CHECKED);
			int mode = (int)SendDlgItemMessage(hwndDlg, IDC_LOSSY_OPTIONS, CB_GETCURSEL, 0, 0);
			if ( mode <0 || mode >3 )
				mode=1;

			// Added by Samuel Wolf
			int selected_comp1 = (int)SendDlgItemMessage(hwndDlg, IDC_VARIANT_OPTIONS, CB_GETCURSEL, 0, 0);
			int selected_comp2 = (int)SendDlgItemMessage(hwndDlg, IDC_VARIANT_OPTIONS2, CB_GETCURSEL, 0, 0);
			int selected_comp3 = (int)SendDlgItemMessage(hwndDlg, IDC_VARIANT_OPTIONS3, CB_GETCURSEL, 0, 0);
			int c1 = dll_info[selected_comp1].variant_IDs;
			int c2 = dll_info[selected_comp2].variant_IDs;
			int c3 = dll_info[selected_comp3].variant_IDs;
			int n = (int)SendDlgItemMessage(hwndDlg, IDC_NOTIFICATION_OPTIONS, CB_GETCURSEL, 0, 0);


			StoreRegistrySettings(nullframes, suggestrgb, multithread, noupsample, mode, c1, c2, c3,n);
		
			EndDialog(hwndDlg, 0);
		} else if ( LOWORD(wParam)==IDC_CANCEL ){
			EndDialog(hwndDlg, 0);
		} else if ( LOWORD(wParam)==IDC_HOMEPAGE ){
			ShellExecute(NULL, "open", "https://github.com/Sam-Wolf/Vuong-DCP-Lossless-Codec/wiki", NULL, NULL, SW_SHOW);
		} 
		// Added by Samuel WOlf
		else if (LOWORD(wParam) == IDC_DLL_FOLDER) {

			TCHAR path[MAX_PATH];

			BROWSEINFO bi = { 0 };
			bi.lpszTitle = ("Browse for folder...");
			bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
			bi.lpfn = BrowseCallbackProc;
			bi.lParam = (LPARAM)"C:\\";


			LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

			if (pidl != 0)
			{
				//get the name of the folder and put it in path
				SHGetPathFromIDList(pidl, path);

				//free memory used
				IMalloc * imalloc = 0;
				if (SUCCEEDED(SHGetMalloc(&imalloc)))
				{
					imalloc->Free(pidl);
					imalloc->Release();
				}
				
				StoreRegistrySettingsPATH(path,sizeof(path));
			}

		}

	} else if ( uMsg == WM_CLOSE ){
		EndDialog(hwndDlg, 0);
	}
	return 0;
}

BOOL CodecInst::QueryConfigure() {
	return TRUE; 
}

DWORD CodecInst::Configure(HWND hwnd) {
	DialogBox(hmoduleLagarith, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, (DLGPROC)ConfigureDialogProc);
	return ICERR_OK;
}


CodecInst* Open(ICOPEN* icinfo) {
	if (icinfo && icinfo->fccType != ICTYPE_VIDEO)
		return NULL;

	CodecInst* pinst = new CodecInst();

	if (icinfo) icinfo->dwError = pinst ? ICERR_OK : ICERR_MEMORY;

	return pinst;
}

CodecInst::~CodecInst(){
	try {
		if ( started == 0x1337 ){
			if (buffer2){
				CompressEnd();
			} else {
				DecompressEnd();
			}
		}
		started =0;
	} catch ( ... ) {};
}

DWORD Close(CodecInst* pinst) {
	try {
		if ( pinst && !IsBadWritePtr(pinst,sizeof(CodecInst)) ){
			delete pinst;
		}
	} catch ( ... ){};
    return 1;
}

// Ignore attempts to set/get the codec state but return successful.
// We do not want Lagarith settings to be application specific, but
// some programs assume that the codec is not configurable if GetState
// and SetState are not supported.
DWORD CodecInst::GetState(LPVOID pv, DWORD dwSize){
	if ( pv == NULL ){
		return 1;
	} else if ( dwSize < 1 ){
		return ICERR_BADSIZE;
	}
	memset(pv,0,1);
	return 1;
}

// See GetState comment
DWORD CodecInst::SetState(LPVOID pv, DWORD dwSize) {
	if ( pv ){
		return ICERR_OK;
	} else {
		return 1;
	}
}

// check if the codec can compress the given format to the desired format
DWORD CodecInst::CompressQuery(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	// check for valid format and bitdepth
	if ( lpbiIn->biCompression == 0) {
		if ( lpbiIn->biBitCount != 24 && lpbiIn->biBitCount != 32){
			return_badformat()
		}
	} else if ( lpbiIn->biCompression == FOURCC_YUY2 || lpbiIn->biCompression == FOURCC_UYVY || lpbiIn->biCompression == FOURCC_YV16 ){
		if ( lpbiIn->biBitCount != 16 ) {
			return_badformat()
		}
	} else if ( lpbiIn->biCompression == FOURCC_YV12 ){
		if ( lpbiIn->biBitCount != 12 ) {
			return_badformat()
		}

	} else {
		/*char msg[128];
		int x = lpbiIn->biCompression;
		char * y = (char*)&x;
		sprintf(msg,"Unknown format, %c%c%c%c",y[0],y[1],y[2],y[3]);
		MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);*/
		return_badformat()
	}

	LoadRegistrySettings(&nullframes,NULL,&multithreading,NULL,&lossy_option, &comp_method1, &comp_method2, &comp_method3,p_dll,&notification_level);

	use_alpha = (lossy_option==0);
	if ( lossy_option ){
		lossy_option--;
		if ( lossy_option == 1 && lpbiIn->biBitCount <= 16 ){
			lossy_option=0;
		} else if ( lpbiIn->biBitCount == 12 ){
			lossy_option=0;
		}

		if ( lossy_option == 2 && (lpbiIn->biCompression == FOURCC_UYVY || lpbiIn->biCompression == FOURCC_YV16) ){ // down sampling routines only accept YUV2
			return_badformat()
		}
	}

	// Make sure width is mod 4 for YUV formats
	if ( (lpbiIn->biBitCount < 24 || lossy_option > 0) && lpbiIn->biWidth%4 ){
		return_badformat()
	}

	// Make sure the height is acceptable for YV12 formats
	if ( lossy_option > 1 || lpbiIn->biBitCount < 16 ){
		if ( lpbiIn->biHeight % 2 ){
			return_badformat();
		}
	}

	// See if the output format is acceptable if need be
	if ( lpbiOut ){

		if ( lpbiOut->biSize < sizeof(BITMAPINFOHEADER) )
			return_badformat()

		if ( lpbiOut->biBitCount != 24 && lpbiOut->biBitCount != 32 && lpbiOut->biBitCount != 16 && lpbiOut->biBitCount != 12 )
			return_badformat()
		if ( lpbiIn->biHeight != lpbiOut->biHeight )
			return_badformat()
		if ( lpbiIn->biWidth != lpbiOut->biWidth )
			return_badformat()
		if ( lpbiOut->biCompression != FOURCC_LAGS )
			return_badformat()
		if ( use_alpha && lpbiIn->biBitCount == 32 && lpbiIn->biBitCount != lpbiOut->biBitCount )
			return_badformat()
		if ( !lossy_option && lpbiIn->biBitCount < lpbiOut->biBitCount && !(lpbiIn->biBitCount == 32 && lpbiOut->biBitCount == 24 ) )
			return_badformat()
		if ( lossy_option==1 && lpbiOut->biBitCount < 16 )
			return_badformat()

	}

	if ( !DetectFlags() ){
		return_badformat()
	}

	return (DWORD)ICERR_OK;
}

// Modified by Samuel Wolf 
// return the intended compress format for the given input format
DWORD CodecInst::CompressGetFormat(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if ( !lpbiOut){
		return sizeof(BITMAPINFOHEADER)+sizeof(UINT32);	
	}

	// make sure the input is an acceptable format
	if ( CompressQuery(lpbiIn, NULL) == ICERR_BADFORMAT ){
		return_badformat()
	}

	LoadRegistrySettings(&nullframes,NULL,&multithreading,NULL,&lossy_option, &comp_method1, &comp_method2, &comp_method3, p_dll, &notification_level);


	// Added by Samuel Wolf
	compression_method1 = comp_method1;
	compression_method2 = comp_method2;
	compression_method3 = comp_method3;

	if ( lossy_option ){
		lossy_option--;

		if ( lossy_option == 1 && lpbiIn->biBitCount <= 16 ){
			lossy_option=0;
		} else if ( lpbiIn->biBitCount == 12 ){
			lossy_option=0;
		}

		if ( lossy_option >= 2 && (lpbiIn->biCompression == FOURCC_UYVY || lpbiIn->biCompression == FOURCC_YV16 )){ // down sampling routines only accept YUV2
			return_badformat()
		}
	}

	*lpbiOut = *lpbiIn;
	lpbiOut->biSize = sizeof(BITMAPINFOHEADER)+sizeof(UINT32);
	lpbiOut->biPlanes = 1;
    lpbiOut->biCompression = FOURCC_LAGS;

	if ( lpbiIn->biBitCount != 24 ){
		lpbiOut->biSizeImage = lpbiIn->biWidth * lpbiIn->biHeight * lpbiIn->biBitCount/8;
	} else {
		lpbiOut->biSizeImage = align_round(lpbiIn->biWidth * lpbiIn->biBitCount/8,4)* lpbiIn->biHeight;
	}
	lpbiOut->biBitCount = lpbiIn->biBitCount;

	*(UINT32*)(&lpbiOut[1])=lossy_option;
	return (DWORD)ICERR_OK;
}

// Modified by Samuel Wolf 
// return information about the codec
DWORD CodecInst::GetInfo(ICINFO* icinfo, DWORD dwSize) {
	if (icinfo == NULL)
		return sizeof(ICINFO);

	if (dwSize < sizeof(ICINFO))
		return 0;

	icinfo->dwSize          = sizeof(ICINFO);
	icinfo->fccType         = ICTYPE_VIDEO;
	icinfo->fccHandler		= FOURCC_LAGS;
	icinfo->dwFlags			= VIDCF_FASTTEMPORALC | VIDCF_FASTTEMPORALD;
	icinfo->dwVersion		= 0x00010000;
	icinfo->dwVersionICM	= ICVERSION;

	// Added by Samuel Wolf - Begin
	memcpy(icinfo->szName, L"Vuong-DCP", sizeof(L"Vuong-DCP"));
	memcpy(icinfo->szDescription, L"Vuong-DCP", sizeof(L"Vuong-DCP"));
	// Added by Samuel Wolf - End

	return sizeof(ICINFO);
}

// check if the codec can decompress the given format to the desired format
DWORD CodecInst::DecompressQuery(const LPBITMAPINFOHEADER lpbiIn, const LPBITMAPINFOHEADER lpbiOut){

	if ( lpbiIn->biCompression != FOURCC_LAGS ){
		return_badformat();
	}

	if ( lpbiIn->biBitCount == 16 || lpbiIn->biBitCount == 12 ){
		if ( lpbiIn->biWidth%4 ){
			return_badformat();
		}
		if ( lpbiIn->biBitCount == 12 && lpbiIn->biHeight%2 ){
			return_badformat();
		}
	}

	// make sure the input bitdepth is valid
	if ( lpbiIn->biBitCount != 24 && lpbiIn->biBitCount != 32 && lpbiIn->biBitCount != 16 && lpbiIn->biBitCount != 12 ){
		return_badformat();
	}

	if ( !DetectFlags() ){
		return_badformat()
	}

	if ( !lpbiOut ){
		return (DWORD)ICERR_OK;
	}

	bool noupsample=false;
	LoadRegistrySettings(&nullframes,NULL,&multithreading,&noupsample,NULL,NULL,NULL,NULL,NULL, &notification_level);


	

	unsigned int lossy=0;
	if ( lpbiIn->biSize == sizeof(BITMAPINFOHEADER)+4 ){
		lossy=*(UINT32*)(&lpbiIn[1]);
		if ( lossy == 3 ){
			lossy=2;
		} else if ( lossy > 3 ){
			lossy=0;
		}
	}
	lossy_option=lossy;

	//char msg[128];
	//char fcc[4];
	//*(unsigned int*)(&fcc[0])=lpbiOut->biCompression;
	//sprintf(msg,"Format = %d, BiComp= %c%c%c%c",lpbiOut->biBitCount,fcc[3],fcc[2],fcc[1],fcc[0] );
	//MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);

	// make sure the output format is one that can be decoded to
	if ( lpbiOut->biCompression != 0 && lpbiOut->biCompression != FOURCC_YV12 && lpbiOut->biCompression != FOURCC_YUY2 ){
		return_badformat();
	}

	// make sure the output bitdepth is valid
	if ( lpbiOut->biBitCount != 32 && lpbiOut->biBitCount != 24 && lpbiOut->biBitCount != 16 && lpbiOut->biBitCount != 12 ){
		return_badformat();
	}
	// make sure that no down sampling is being performed
	if ( !lossy ){
		if ( lpbiOut->biBitCount < 24 && lpbiOut->biBitCount < lpbiIn->biBitCount){
			return_badformat();
		}
	} else {
		if ( lossy == 1 && lpbiOut->biBitCount < 16 ){
			return_badformat();
		} else if ( lpbiOut->biBitCount < 12 ){
			return_badformat();
		}
	}

	if ( noupsample ){
		// make sure no colorspace conversion is being done
		if ( !lossy ){
			if ( lpbiIn->biBitCount < RGB24 && lpbiOut->biBitCount != lpbiIn->biBitCount){
				return_badformat();
			}
			if ( lpbiIn->biBitCount >= RGB24 && lpbiOut->biBitCount != RGB32){
				return_badformat();
			}
		} else {
			if ( lossy == 1 && lpbiOut->biBitCount != 16 ){
				return_badformat();
			} else if ( lossy == 2 && lpbiOut->biBitCount != 12 ){
				return_badformat();
			}
		}
	}

	// check for invalid widths/heights
	if ( lossy > 0 || lpbiOut->biBitCount < 24 ){
		if ( lpbiIn->biWidth % 2 )
			return_badformat();
		if ( lpbiIn->biWidth % 4 ){
			if ( lossy == 1 && lpbiOut->biBitCount != 16 )
				return_badformat();
			if ( lossy == 2 && lpbiOut->biBitCount != 12 )
				return_badformat();
			if ( lossy > 2 )
				return_badformat();
		}
		if ( lossy > 1 || lpbiOut->biBitCount < 16 ){
			if ( lpbiIn->biHeight % 2 ){
				return_badformat();
			}
		}
	}

	if ( lpbiOut->biCompression == 0 && lpbiOut->biBitCount != 24 && lpbiOut->biBitCount != 32){
			return_badformat();
	}
	if ( lpbiOut->biCompression == FOURCC_YUY2 && lpbiOut->biBitCount != 16 ){
			return_badformat();
	}
	if ( lpbiOut->biCompression == FOURCC_YV12 && lpbiOut->biBitCount != 12 ){
			return_badformat();
	}

	if ( lpbiIn->biHeight != lpbiOut->biHeight )
		return_badformat();
	if ( lpbiIn->biWidth != lpbiOut->biWidth )
		return_badformat();

	return (DWORD)ICERR_OK;
}

// return the default decompress format for the given input format 
DWORD CodecInst::DecompressGetFormat(const LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut){

	if ( DecompressQuery(lpbiIn, NULL ) != ICERR_OK){
		return_badformat();
	}

	if ( !lpbiOut)
		return sizeof(BITMAPINFOHEADER);

	bool suggest=false;
	bool noupsample=false;
	LoadRegistrySettings(&nullframes,&suggest,&multithreading,&noupsample,NULL,NULL,NULL,NULL,NULL,&notification_level);


	unsigned int lossy=0;
	if ( lpbiIn->biSize == sizeof(BITMAPINFOHEADER)+4 ){
		lossy=*(UINT32*)(&lpbiIn[1]);
		if ( lossy >= 4 )
			lossy=0;
		if ( lossy == 3)
			lossy=2;
	}
	lossy_option=lossy;

	*lpbiOut = *lpbiIn;
	lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
	lpbiOut->biPlanes = 1;

	// suggest RGB32 if source is RGB, or if "always suggest RGB" is selected
	if ( (!lossy && (lpbiIn->biBitCount == 24 || lpbiIn->biBitCount == 32)) || suggest ){
		lpbiOut->biBitCount = 32;
		lpbiOut->biSizeImage = lpbiIn->biWidth * lpbiIn->biHeight * 4;
		lpbiOut->biCompression = 0;

	// suggest YUY2 if source is YUY2
	} else if ( lpbiIn->biBitCount == 16 || lossy == 1) {
		lpbiOut->biBitCount=16;
		lpbiOut->biCompression = FOURCC_YUY2;
		lpbiOut->biSizeImage = lpbiIn->biWidth * lpbiIn->biHeight * 2;

	// suggest YV12 if source is YV12
	} else if ( lpbiIn->biBitCount == 12 || lossy == 2){
		lpbiOut->biBitCount=12;
		lpbiOut->biCompression = FOURCC_YV12;
		lpbiOut->biSizeImage = lpbiIn->biWidth * lpbiIn->biHeight + lpbiIn->biWidth * lpbiIn->biHeight/2;
	} else {
		return_badformat();
	}

	return (DWORD)ICERR_OK;
}

DWORD CodecInst::DecompressGetPalette(LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut) {
	return_badformat()
}

//MessageBox (HWND_DESKTOP, msg, "Error", MB_OK | MB_ICONEXCLAMATION);

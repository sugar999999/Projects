// ConsoleApplication1.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//


#include "stdafx.h"

using namespace System;
using namespace System::IO;




int main()
{	
	size_t testCount = 0;
	TCHAR szBufM[256];
	TCHAR szBufP[256];
	HANDLE hSnapProc;
	HANDLE hSnapMod;
	MODULEENTRY32 me;
	PROCESSENTRY32 pe;
	hSnapProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapProc == INVALID_HANDLE_VALUE) Console::WriteLine("N/A");
	

	me.dwSize = sizeof(MODULEENTRY32);
	pe.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapProc, &pe);
	
	do {
		wsprintf(szBufP, TEXT("%s\t%d"), pe.szExeFile, pe.th32ProcessID);
		for (testCount = 0; testCount < wcslen(szBufP); testCount++)
			std::cout << static_cast<char>(szBufP[testCount]);
		std::cout << std::endl;
		hSnapMod = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe.th32ProcessID);
		if (hSnapMod == INVALID_HANDLE_VALUE) {
			std::cout << "N/A" << std::endl << std::endl;
			continue;
		}
		else {
			Module32First(hSnapMod, &me);
		}
		do {
			wsprintf(szBufM, TEXT("%s"), me.szModule);
			for (testCount = 0; testCount < wcslen(szBufM); testCount++)
				std::cout << static_cast<char>(szBufM[testCount]);
			std::cout << ' ';
		} while (Module32Next(hSnapMod, &me));
		
		std::cout << std::endl << std::endl;
	} while (Process32Next(hSnapProc, &pe));

	CloseHandle(hSnapProc);
	CloseHandle(hSnapMod);

	getchar();

	return 0;
}
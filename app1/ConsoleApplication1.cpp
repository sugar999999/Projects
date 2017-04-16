// tests.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"


DWORD EnumerateFileHandles(ULONG pid)
{
	///////////////////////////////////////////////////////////////////////// 
	// 
	//  

	// ライブラリをインポート 
	// 

	HINSTANCE hNtDll = LoadLibrary(_T("ntdll.dll"));
	assert(hNtDll != NULL);

	PFN_NTQUERYSYSTEMINFORMATION NtQuerySystemInformation =
		(PFN_NTQUERYSYSTEMINFORMATION)GetProcAddress(hNtDll,
			"NtQuerySystemInformation");
	assert(NtQuerySystemInformation != NULL);

	PFN_NTQUERYINFORMATIONFILE NtQueryInformationFile =
		(PFN_NTQUERYINFORMATIONFILE)GetProcAddress(hNtDll,
			"NtQueryInformationFile");

	PFN_NTQUERYOBJECT NtQueryObject = 
		(PFN_NTQUERYOBJECT)GetProcAddress(hNtDll, "NtQueryObject");


	///////////////////////////////////////////////////////////////////////// 
	// 
	//  

	DWORD dwFiles = 0;

	// 
	// 
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, FALSE, pid);
	if (hProcess == NULL)
	{
		printf("\\<unable to open process> %x\n", GetLastError());
		hNtDll = NULL;
		NtQuerySystemInformation = NULL;
		NtQueryInformationFile = NULL;
		return -1;
	}

	HANDLE hToken = NULL;
	PTOKEN_OWNER pTokenOwner;
	DWORD dwLength;
	TCHAR ownerName[MAX_PATH];
	DWORD dwSizeName;
	TCHAR domainName[MAX_PATH];
	DWORD dwSizedomain;
	SID_NAME_USE sidname;

	OpenProcessToken(hProcess, TOKEN_QUERY, &hToken);
	if (hToken == NULL) {
		printf("\\<unable to open process> %x\n", GetLastError());
	}

	GetTokenInformation(hToken, TokenOwner, NULL, 0, &dwLength);
	
	pTokenOwner = (PTOKEN_OWNER)LocalAlloc(LPTR, dwLength);
	if (GetTokenInformation(hToken, TokenOwner, pTokenOwner, dwLength, &dwLength) == 0) {
		//printf("\\<unable to open process> %x\n", GetLastError());


	}
	dwSizeName = sizeof(ownerName) / sizeof(TCHAR);
	dwSizedomain = sizeof(domainName) / sizeof(TCHAR);
	LookupAccountSid(NULL, pTokenOwner->Owner, ownerName, &dwSizeName, domainName, &dwSizedomain, &sidname);

	printf("%ls\\%ls\n", domainName, ownerName);

	///////////////////////////////////////////////////////////////////////// 
	// システムハンドル情報取得 
	//  

	DWORD nSize = 4096, nReturn;
	PSYSTEM_HANDLE_INFORMATION pSysHandleInfo = (PSYSTEM_HANDLE_INFORMATION)
		HeapAlloc(GetProcessHeap(), 0, nSize);

	// バッファサイズの調整
	//
	while (NtQuerySystemInformation(SystemHandleInformation, pSysHandleInfo,
		nSize, &nReturn) == STATUS_INFO_LENGTH_MISMATCH)
	{
		HeapFree(GetProcessHeap(), 0, pSysHandleInfo);
		nSize *= 2;
		pSysHandleInfo = (SYSTEM_HANDLE_INFORMATION*)HeapAlloc(
			GetProcessHeap(), 0, nSize);
	}

	// 取得したハンドルを走査
	// 
	for (ULONG i = 0; i < pSysHandleInfo->NumberOfHandles; i++)
	{
		PSYSTEM_HANDLE pHandle = &(pSysHandleInfo->Handles[i]);

		// 
		if (pHandle->ProcessId == pid)
		{
			dwFiles++;
		
			HANDLE hCopy;
			if (!DuplicateHandle(hProcess, (HANDLE)pHandle->Handle,
				GetCurrentProcess(), &hCopy, PROCESS_QUERY_INFORMATION, FALSE, 0)) // PROCESS_DUP_HANDLE?
				continue;

			//以下のハンドルの場合は無視しないとハングしたりするので無視
			if ((pHandle->GrantedAccess == 0x0012019f)
				|| (pHandle->GrantedAccess == 0x001a019f)
				|| (pHandle->GrantedAccess == 0x00120189)
				|| (pHandle->GrantedAccess == 0x00120089)
				|| (pHandle->GrantedAccess == 0x00100000)
				|| (pHandle->GrantedAccess == 0x00000000))
			{
				continue;
			}

			//オブジェクト情報の取得
			DWORD nObSize = 4096;
			NTSTATUS status;
			std::unique_ptr<char[]> pObjectTypeBuf(new char[nObSize]);
			while ((status = NtQueryObject(
				hCopy,
				(OBJECT_INFORMATION_CLASS)ObjectTypeInformation,
				pObjectTypeBuf.get(),
				nObSize,
				&nObSize)) == STATUS_INFO_LENGTH_MISMATCH)
			{
				nObSize += 4096;
				pObjectTypeBuf.reset(new char[nObSize]);
			}
			if (!NT_SUCCESS(status)) {
				
				continue;
			}

			//オブジェクト名の取得
			nObSize = 4096;
			std::unique_ptr<char[]> pObjectNameBuf(new char[nObSize]);
			while ((status = NtQueryObject(
				hCopy,
				(OBJECT_INFORMATION_CLASS)ObjectNameInformation,
				pObjectNameBuf.get(),
				nObSize,
				&nObSize) == STATUS_INFO_LENGTH_MISMATCH))
			{
				nObSize += 4096;
				pObjectNameBuf.reset(new char[nObSize]);
			}
			if (!NT_SUCCESS(status)) {
				
				continue;
			}

			OBJECT_TYPE_INFORMATION *objectTypeInfo = (OBJECT_TYPE_INFORMATION*)pObjectTypeBuf.get();
			UNICODE_STRING objectName = *(PUNICODE_STRING)pObjectNameBuf.get();
			if (!objectName.Length) {
				continue;
			}



			IO_STATUS_BLOCK ioStatus2;
			PFILE_ACCESS_INFORMATION pAccessInfo = (PFILE_ACCESS_INFORMATION)
				malloc(MAX_PATH * 2 * 2);
			DWORD dwAccessInfoSize = MAX_PATH * 2 * 2;
			TCHAR accessName[6];

			if (NtQueryInformationFile(hCopy, &ioStatus2, pAccessInfo,
				dwAccessInfoSize, FileAccessInformation) == STATUS_SUCCESS)
			{
				if (pAccessInfo->AccessFlags == 0) {
					wsprintf(accessName, L"(---)");
				}
				else if (pAccessInfo->AccessFlags == 1) {
					wsprintf(accessName, L"(R--)");
				}
				else if (pAccessInfo->AccessFlags == 2) {
					wsprintf(accessName, L"(-W-)");
				}
				else if (pAccessInfo->AccessFlags == 3) {
					wsprintf(accessName, L"(RW-)");
				}
				else if (pAccessInfo->AccessFlags == 4) {
					wsprintf(accessName, L"(--D)");
				}
				else if (pAccessInfo->AccessFlags == 5) {
					wsprintf(accessName, L"(R-D)");
				}
				else if (pAccessInfo->AccessFlags == 6) {
					wsprintf(accessName, L"(-WD)");
				}
				else if (pAccessInfo->AccessFlags == 7) {
					wsprintf(accessName, L"(RWD)");
				}

			}
			else {
				DWORD err = GetLastError();
			}



			// 
			// 
			IO_STATUS_BLOCK ioStatus;
			PFILE_NAME_INFORMATION pNameInfo = (PFILE_NAME_INFORMATION)
				malloc(MAX_PATH * 2 * 2);
			DWORD dwInfoSize = MAX_PATH * 2 * 2;

			if (NtQueryInformationFile(hCopy, &ioStatus, pNameInfo,
				dwInfoSize, FileNameInformation) == STATUS_SUCCESS)
			{
				// 
				WCHAR wszFileName[MAX_PATH + 1];
				StringCchCopyNW(wszFileName, MAX_PATH + 1,
					pNameInfo->FileName, 
					pNameInfo->FileNameLength / 2);

				printf("%+4lx:%-10ls%ls\t%ls\n", pHandle->Handle, objectTypeInfo->Name.Buffer, accessName, wszFileName);
			}
			else {
				if(wcscmp(L"Section", objectTypeInfo->Name.Buffer) == 0)
					printf("%+4lx:%-10ls\t%ls\n", pHandle->Handle, objectTypeInfo->Name.Buffer, objectName.Buffer);
			}

			

			free(pAccessInfo);
			free(pNameInfo);
			CloseHandle(hCopy);
		}
	}

	if (hToken != NULL)CloseHandle(hToken);
	HeapFree(GetProcessHeap(), 0, pTokenOwner);
	CloseHandle(hProcess);

	HeapFree(GetProcessHeap(), 0, pSysHandleInfo);

	return dwFiles;
}

#pragma endregion 


void EnableDebugPrivilege()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp = { 0 };

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

	LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL) == 0) {
		printf("ERROR %x\n\n\n", GetLastError());
	}
	CloseHandle(hToken);
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE procSnap = NULL;
	PROCESSENTRY32 procEntry;
	EnableDebugPrivilege();

	procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

	if (procSnap == INVALID_HANDLE_VALUE) {
		return 1;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(procSnap, &procEntry)) {
		goto END;
	}
	Process32Next(procSnap, &procEntry);

	do {
		ULONG pid = procEntry.th32ProcessID;
		printf("-------------------------------------------\n%ls PID:%d ", procEntry.szExeFile, pid);
		DWORD dwFiles = EnumerateFileHandles(pid);



	} while (Process32Next(procSnap, &procEntry));
	
	

	
END:
	CloseHandle(procSnap);

	getchar();
	return 0;
}

// test.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

struct SYSTEM_HANDLE_INFORMATION
{
	ULONG	ProcessId;
	BYTE	ObjectTypeNumber;
	BYTE	Flags;
	USHORT	Handle;
	PVOID	Object;
	ACCESS_MASK	GrantedAccess;
};

typedef NTSTATUS(WINAPI*QuerySysInfo)(ULONG, PVOID, ULONG, PULONG);

bool GetSystemHandleTable(std::vector<SYSTEM_HANDLE_INFORMATION>& handles)
{
	struct SYS_HANDLE_INFO_EX
	{
		ULONG size;
		SYSTEM_HANDLE_INFORMATION handles[1];
	};

	__if_not_exists(SystemHandleInformation)
	{
		const UINT SystemHandleInformation = 16;
	}
	std::vector<BYTE> buffer(32000);
	DWORD size = 0;
	NTSTATUS status = 0;
	QuerySysInfo qsi = reinterpret_cast<QuerySysInfo>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQuerySystemInformation"));
	while ((status = qsi(SystemHandleInformation, &buffer[0], buffer.size(), &size)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		if (size)
		{
			buffer.resize(size);
		}
		else
		{
			buffer.resize(buffer.size() * 2);
		}
	}
	if (status == STATUS_SUCCESS)
	{
		SYS_HANDLE_INFO_EX* handleInf = reinterpret_cast<SYS_HANDLE_INFO_EX*>(&buffer[0]);
		handles.resize(handleInf->size);
		std::copy(&(handleInf->handles[0]), &(handleInf->handles[handleInf->size]), handles.begin());
	}
	return status == STATUS_SUCCESS;
}

namespace detail
{
	struct ProcessIDMatcher
	{
		DWORD id;
		ProcessIDMatcher(DWORD id) : id(id) {}
		bool operator()(SYSTEM_HANDLE_INFORMATION& handle) const
		{
			return handle.ProcessId != id;
		}
	};
}

void GetProcessHandleTable(DWORD procID, std::vector<SYSTEM_HANDLE_INFORMATION>& procHandles)
{
	std::vector<SYSTEM_HANDLE_INFORMATION> handles;
	if (GetSystemHandleTable(handles))
	{
		std::remove_copy_if(
			handles.begin(),
			handles.end(),
			std::back_inserter(procHandles),
			detail::ProcessIDMatcher(procID)
		);
	}

}
int main()
{
	LPSYSTEM_INFO lpSystemInfo = new SYSTEM_INFO;

	GetSystemInfo(lpSystemInfo);

	std::vector<SYSTEM_HANDLE_INFORMATION> handles;
	GetSystemHandleTable(handles);
	


    return 0;

}



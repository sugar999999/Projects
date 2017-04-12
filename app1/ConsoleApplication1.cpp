
#include "stdafx.h"


int main(){

	PMIB_TCPTABLE_OWNER_PID pTcpTable = NULL;
	PMIB_UDPTABLE_OWNER_PID pUdpTable = NULL;
	DWORD dwSize = 0;
	BOOL bOrder = TRUE;
	ULONG ulAf = AF_INET;
	MIB_TCPROW_OWNER_PID rowTcp;
	MIB_UDPROW_OWNER_PID rowUdp;
	DWORD dwRet;


	dwRet = GetExtendedTcpTable(NULL, &dwSize, bOrder, ulAf, TCP_TABLE_OWNER_PID_ALL, 0);

	pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);

	dwRet = GetExtendedTcpTable(pTcpTable, &dwSize, bOrder, ulAf, TCP_TABLE_OWNER_PID_ALL, 0);

	printf("%-6s %-23s %-23s %6s\n",

		"プロトコル",
		"ローカル アドレス",
		"外部アドレス",
		"PID");


	for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
		
		

		rowTcp = pTcpTable->table[i];

		char bufLocalAddr[16];
		char bufRemoteAddr[16];
		char lpszHostName[50] = "";
		char lpszServtName[50] = "";

		inet_ntop(ulAf, &rowTcp.dwLocalAddr, bufLocalAddr, 16);
		inet_ntop(ulAf, &rowTcp.dwRemoteAddr, bufRemoteAddr, 16);

		int rc = getnameinfo((struct sockaddr *)bufRemoteAddr, 4, lpszHostName, 50, lpszServtName, 50, 0);

		printf("%-6s %15s:%-8d %15s:%-8d %-6u\n",

			"TCP",
			bufLocalAddr,
			ntohs((u_short)(rowTcp.dwLocalPort)),

			lpszServtName, //bufRemoteAddr
			ntohs((u_short)(rowTcp.dwRemotePort)),
			rowTcp.dwOwningPid);
	}
	
	free(pTcpTable);

	
	
	dwRet = GetExtendedUdpTable(NULL, &dwSize, bOrder, ulAf, UDP_TABLE_OWNER_PID, 0);

	pUdpTable = (PMIB_UDPTABLE_OWNER_PID)malloc(dwSize);

	dwRet = GetExtendedUdpTable(pUdpTable, &dwSize, bOrder, ulAf, UDP_TABLE_OWNER_PID, 0);

	for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++) {

		rowUdp = pUdpTable->table[i];


		char bufLocalAddr[16];
		inet_ntop(ulAf, &rowUdp.dwLocalAddr, bufLocalAddr, 16);


		printf("%-6s %15s:%-8d %15c:%-8c %-6u\n",

			"UDP",
			bufLocalAddr,
			ntohs((u_short)(rowUdp.dwLocalPort)),

			'*',
			'*',
			rowUdp.dwOwningPid);
	}

	free(pUdpTable);


	getchar();
	return 0;
}

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

#define Assert(Expression) if(!(Expression)) {*(int*)0 = 0;}

#include "http_parser.h"
#include "png_parser.h"

int SendData(SOCKET sckt, const void *data, int datalen)
{
    const char *ptr = static_cast<const char*>(data);
    while (datalen > 0) {
        int bytes = send(sckt, ptr, datalen, 0);
        if (bytes <=0) return -1;
        ptr += bytes;
        datalen -= bytes;
    }
    return 0;
}

int main(int nArg, char** Args)
{
#if 1
	WSADATA WsaData;
	if(WSAStartup(MAKEWORD(2,2), &WsaData) != 0) 
	{
        printf("WSAStartup failed.\n");
        exit(1);
    }
	
	addrinfo Hint = {};
	Hint.ai_family = AF_INET;
	Hint.ai_socktype = SOCK_STREAM;
	Hint.ai_protocol = IPPROTO_TCP;
	Hint.ai_flags = AI_PASSIVE;
	
	char* Port = "8080";
	
	addrinfo* Result;
	
	int ErrCode = getaddrinfo(0, Port, &Hint, &Result);
	if(ErrCode != 0)
	{
		printf("getaddrinfo() failed: %d\n", ErrCode);
		WSACleanup();
		exit(1);
	}
	
	SOCKET ListenSocket = socket(Result->ai_family, Result->ai_socktype, Result->ai_protocol);
	if(ListenSocket == INVALID_SOCKET)
	{
		printf("Create socket failed: %d\n", WSAGetLastError());
		freeaddrinfo(Result);
		WSACleanup();
		exit(1);
	}
	
	if(bind(ListenSocket, Result->ai_addr, (int)Result->ai_addrlen) == SOCKET_ERROR)
	{
		printf("Binding socket failed: %d\n", WSAGetLastError());
		freeaddrinfo(Result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
	
	freeaddrinfo(Result);
	
	bool Running = true;
	while(Running)
	{
		if(listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
		{
			printf("Listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			exit(1);
		}
		else
		{
			printf("Listening\n");
		}
		
		sockaddr_storage their_addr;
		int SinSize = sizeof(their_addr);
		SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&their_addr, &SinSize);
		if(ClientSocket == INVALID_SOCKET)
		{
			printf("Accept failed: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			exit(1);
		}
		
		char RecvBuffer[10000];
		int BytesReceived = recv(ClientSocket, RecvBuffer, 10000, 0);
		printf("REQUEST: %s\n", RecvBuffer);
		
		file_info FileInfo = GetFileInfo(RecvBuffer);
		file_data File = ReadWholeFile(FileInfo.Path, FileInfo.FileType);
		if(File.StatusCode == STATUS_OK)
		{
			char ResponseBuffer[100000] = {};
			http_response Response = {};
			Response.StatusCode = STATUS_OK;
			Response.FileLength = File.Size;
			Response.Buffer = ResponseBuffer;
			
			MakeHTTPResponse(&Response, &File);
			send(ClientSocket, Response.Buffer, (int)strlen(Response.Buffer) + 1, 0);
			printf("\nRESPONSE: %s\n", Response.Buffer);
			
			switch(FileInfo.FileType)
			{
				case FILE_PNG:
				case FILE_JPG:
				{
					char* Buffer = (char*)File.Data;
					for(size_t i = 0; i < File.Size; ++i)
					{
						send(ClientSocket, Buffer++, 1, 0);
					}
				} break;
				default:
				{
					send(ClientSocket, (char*)File.Data, (int)File.Size, 0);
				}
			}
			free(FileInfo.Path);
		}
		else if(File.StatusCode == STATUS_NOTFOUND)
		{
			char ResponseBuffer[10000];
			http_response Response = {};
			Response.StatusCode = STATUS_NOTFOUND;
			Response.FileLength = File.Size;
			Response.Buffer = ResponseBuffer;
			
			MakeHTTPResponse(&Response, &File);
			printf("\nFailed: %s\n", Response.Buffer);
			
			send(ClientSocket, Response.Buffer, (int)strlen(Response.Buffer) + 1, 0);
		}
	}
#endif
	return 0;
}
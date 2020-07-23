#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;
typedef float real32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

enum status_code
{
	STATUS_OK,
	STATUS_NOTFOUND,
};

enum file_type
{
	FILE_HTML,
	FILE_CSS,
	FILE_PNG,
	FILE_JPG,
	FILE_JS,
};

struct file_info
{
	char* Path;
	file_type FileType;
};

struct file_data
{
    void* Data;
    size_t Size;
	file_info Info;
	status_code StatusCode;
};

char* GetURL(char* ReceiveBuffer);
file_type GetFileType(char* FileName);

file_info GetFileInfo(char* FileName)
{
	file_info Info = {};
	Info.FileType = GetFileType(FileName);
	Info.Path = GetURL(FileName);
	
	return Info;
}

file_data ReadWholeFile(char* Path, file_type FileType)
{
	
	FILE* FileHandle = fopen(Path, "rb");
	file_data Result = {};
		
	if (FileHandle)
	{
		fseek(FileHandle, 0L, SEEK_END);
		size_t FileSizeInBytes = ftell(FileHandle);
		fseek(FileHandle, 0L, SEEK_SET);				

		char* Buffer = (char*)malloc(FileSizeInBytes);
				
		Result.Info = {Path, FileType};
		Result.Size = FileSizeInBytes + 1;
		Result.StatusCode = STATUS_OK;
		
		if (!Buffer)
		{
			printf("Failed to allocate file buffer\n");
			Result = {};
			Result.StatusCode = STATUS_NOTFOUND;
			free(Buffer);
		}
		
		if(fread((void*)Buffer, 1, Result.Size, FileHandle) == -1) 
		{
			printf("Failed to read file\n");
			Result = {};
			Result.StatusCode = STATUS_NOTFOUND;
			free(Buffer);
			fclose(FileHandle);
			return Result;
		}
		
		Result.Data = Buffer;
		fclose(FileHandle);
	}
	else
	{
		Result = {};
		Result.StatusCode = STATUS_NOTFOUND;
	}
	
	return Result;
}

struct http_response
{
	char* Buffer;
	int Length;
	size_t FileLength;
	status_code StatusCode;
};

struct header
{
	char* Key;
	char* Value;
};

header Header(char* Key, char* Value)
{
	header H = {Key, Value};
	return H;
}

char* GetNextCharacter(char* Buffer, size_t Length, char* From, char C)
{
    char* Current = From;
    while (((Current - Buffer) < (int)(Length)) && (*Current != C)) Current++;

    if (Current >= (Buffer + Length))
        return 0;

    return Current;
}

void PushCharacter(http_response* Response, char C)
{
    Response->Buffer[Response->Length++] = C;
}

void PushString(http_response* Response, char* String)
{
    char* Current = String;
    while(*Current != '\0')
    {
        PushCharacter(Response, *Current++);
    }
}

void PushString(http_response* Response, char* String, size_t Length)
{
    for (int Index = 0;
         Index != Length;
         ++Index)
    {
        PushCharacter(Response, String[Index]);
    }
}

void PushInitialResponseLine(http_response* Response)
{
	switch(Response->StatusCode)
	{
		case STATUS_OK:
		PushString(Response, "HTTP/1.1 200 OK\n");
		break;
		case STATUS_NOTFOUND:
		PushString(Response, "HTTP/1.1 404 Not Found\n");
		break;
	}
}

char* GetURL(char* ReceiveBuffer)
{
	int Index = 0;
	while(ReceiveBuffer[Index++] != ' ');
	char Buffer[100] = {};
	http_response URL = {};
	URL.Buffer = Buffer;
	PushString(&URL, "..");
	while(ReceiveBuffer[Index] != ' ')
	{
		PushCharacter(&URL, ReceiveBuffer[Index]);
		++Index;
	}
	char* Result = (char*)malloc(100);
	strcpy(Result, Buffer);
	return Result;
}

file_type GetFileType(char* FileName)
{
	file_type Type = FILE_HTML;
	int Index = 0;
	while(FileName[Index++] != '.');
	
	char* FileType = &FileName[Index];
	char* FileTypeEnd = GetNextCharacter(FileName, strlen(FileName), 
											FileType, ' ');
	size_t Length = FileTypeEnd - FileType;
	
	printf("FileType: %c\n", FileType[0]);
	
	if(strncmp(FileType, "html", Length) == 0)
	{
		Type = FILE_HTML;
	}
	else if(strncmp(FileType, "css", Length) == 0)
	{
		Type = FILE_CSS;
	}
	else if(strncmp(FileType, "png", Length) == 0)
	{
		Type = FILE_PNG;
	}
	else if(strncmp(FileType, "jpg", Length) == 0)
	{
		Type = FILE_JPG;
	}
	else if(strncmp(FileType, "js", Length) == 0)
	{
		Type = FILE_JS;
	}
	
	return Type;
}

char* GetCurrentDate()
{
	SYSTEMTIME SystemTime = {};
	GetSystemTime(&SystemTime);
	char Date[100];
	http_response DateResponse = {};
	DateResponse.Buffer = Date;
	switch(SystemTime.wDayOfWeek)
	{
		case 0: PushString(&DateResponse, "Sun, "); break;
		case 1: PushString(&DateResponse, "Mon, "); break;
		case 2: PushString(&DateResponse, "Tue, "); break;
		case 3: PushString(&DateResponse, "Wed, "); break;
		case 4: PushString(&DateResponse, "Thu, "); break;
		case 5: PushString(&DateResponse, "Fri, "); break;
		case 6: PushString(&DateResponse, "Sat, "); break;
	}
	char Day[5] = {};
	sprintf(Day, "%d ", SystemTime.wDay);
	PushString(&DateResponse, Day);
	switch(SystemTime.wMonth)
	{
		case 1: PushString(&DateResponse, "Jan "); break;
		case 2: PushString(&DateResponse, "Feb "); break;
		case 3: PushString(&DateResponse, "Mar "); break;
		case 4: PushString(&DateResponse, "Apr "); break;
		case 5: PushString(&DateResponse, "May "); break;
		case 7: PushString(&DateResponse, "Jun "); break;
		case 6: PushString(&DateResponse, "Jul "); break;
		case 8: PushString(&DateResponse, "Aug "); break;
		case 9: PushString(&DateResponse, "Sep "); break;
		case 10: PushString(&DateResponse, "Oct "); break;
		case 11: PushString(&DateResponse, "Nov "); break;
		case 12: PushString(&DateResponse, "Dec "); break;
	}
	char Year[10] = {};
	sprintf(Year, "%d ", SystemTime.wYear);
	PushString(&DateResponse, Year);
	char Time[20] = {};
	sprintf(Time, "%02d:%02d", 
				SystemTime.wHour, SystemTime.wMinute);
	PushString(&DateResponse, &Time[0]);
				
	return DateResponse.Buffer;
}

void PushHeader(http_response* Response, header HeaderLine)
{
	PushString(Response, HeaderLine.Key);
	PushCharacter(Response, ':');
	PushCharacter(Response, ' ');
	PushString(Response, HeaderLine.Value);
	PushCharacter(Response, '\r');
	PushCharacter(Response, '\n');
}

void MakeHTTPResponse(http_response* Response, file_data* File)
{
	PushInitialResponseLine(Response);
	PushHeader(Response, Header("Date", GetCurrentDate()));
	switch(File->Info.FileType)
	{
		case FILE_HTML:
		PushHeader(Response, Header("Content-Type", "text/html"));
		break;
		case FILE_CSS:
		PushHeader(Response, Header("Content-Type", "text/css"));
		break;
		case FILE_PNG:
		PushHeader(Response, Header("accept-ranges", "bytes"));
		PushHeader(Response, Header("Content-Type", "image/png"));
		break;
		case FILE_JPG:
		PushHeader(Response, Header("accept-ranges", "bytes"));
		PushHeader(Response, Header("Content-Type", "image/jpg"));
		break;
		case FILE_JS:
		{
			PushHeader(Response, Header("accept-ranges", "bytes"));
			PushHeader(Response, Header("Content-Type", "text/javascript"));
		} break;
	}
	char ContentLength[100] = {};
	sprintf(ContentLength, "%zd", Response->FileLength);
	PushHeader(Response, Header("Content-Length", ContentLength));
	PushHeader(Response, Header("Server", "WebServer/0.1 (Windows) (10)"));
	PushHeader(Response, Header("Connection", "keep-alive"));
	PushCharacter(Response, '\r');
	PushCharacter(Response, '\n');
	PushCharacter(Response, '\r');
	PushCharacter(Response, '\n');
}

#endif
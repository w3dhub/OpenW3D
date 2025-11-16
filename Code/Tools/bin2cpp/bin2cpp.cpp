/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "windows.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char _StringBuf[1024];

void Print_Usage(void);
void Write(HANDLE file,const char * format,...);
void Write_Bytes(HANDLE file,const char * bytes,int bytecount);


/*
** This program simply reads in the specified file and creates a cpp and h file
** which define an array of bytes that contain the contents of the input file.
** The cpp and h files will have the same root filename with the extension replaced.
*/

int main(int argc, char* argv[])
{
	if (argc != 2) {
		Print_Usage();
		return 1;
	}

	/*
	** Open the input file
	*/
	HANDLE inputfile = CreateFile(argv[1],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);	
	if (inputfile == INVALID_HANDLE_VALUE) {
		printf("Could not open file: %s\n",argv[1]);
		Print_Usage();
		return 1;
	}

	DWORD inputsize = GetFileSize(inputfile,NULL);

	/*
	** Open up the two output files
	*/
	char drive[_MAX_FNAME];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char cpp_fname[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT];
	char h_fname[_MAX_DRIVE + _MAX_DIR + _MAX_FNAME + _MAX_EXT];

	_splitpath(argv[1],drive,dir,fname,NULL);
	_makepath(cpp_fname,drive,dir,fname,"cpp");
	_makepath(h_fname,drive,dir,fname,"h");

	HANDLE hfile = CreateFile(h_fname,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		printf("Could not create file: %s\n",h_fname);
		Print_Usage();
		return 1;
	}

	HANDLE cppfile = CreateFile(cpp_fname,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if (cppfile == INVALID_HANDLE_VALUE) {
		printf("Could not create file: %s\n",h_fname);
		Print_Usage();
		return 1;
	}
	
	/*
	** Write the cpp file
	*/
	Write(cppfile,"\n\nconst unsigned char %s[%d] = \n",fname,inputsize);
	Write(cppfile,"{\n");

	/*
	** Read the file, writing it in text form into the cpp file
	*/
	const int LINELEN = 16;
	char bytes[LINELEN];
	DWORD bytesread = 0;
	
	while(bytesread < inputsize) {
		
		DWORD readsize = min(LINELEN,inputsize - bytesread);
		DWORD actualread;

		ReadFile(inputfile,bytes,readsize,&actualread,NULL);
		assert(actualread == readsize);
		bytesread += readsize;

		Write_Bytes(cppfile,bytes,readsize);
	}

	/*
	** read in remainder
	*/
	Write(cppfile,"};\n");


	/*
	** Write the h file
	*/
	Write(hfile,"\n\nextern const unsigned char %s[%d];\n\n",fname,inputsize);

	/*
	** Close the files
	*/
	CloseHandle(inputfile);
	CloseHandle(hfile);
	CloseHandle(cppfile);
	return 0;
}


void Write(HANDLE file,const char * format,...)
{
	va_list	va;
	va_start(va, format);
	vsprintf(_StringBuf, format, va);
	assert((strlen(_StringBuf) < sizeof(_StringBuf)));
	va_end(va);

	unsigned int byteswritten;
	WriteFile(file,_StringBuf,strlen(_StringBuf),&byteswritten,NULL);
}

void Write_Bytes(HANDLE file,const char * bytes,int bytecount)
{
	Write(file,"\t");
	for (int i=0;i<bytecount;i++) {
		unsigned char val = (unsigned char)bytes[i];
		Write(file,"0x%2.2X,",val);
	}
	Write(file,"\n");
}

void Print_Usage(void)
{
	printf("USAGE: bin2h <inputfilename>\n");
}

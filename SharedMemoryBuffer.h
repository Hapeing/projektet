#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <tchar.h>
class SharedMemoryBuffer
{
	const char* pBuf;
	HANDLE FileMapHandle;
	size_t bufferSize;
	
public:
	void DebugPrintAdress();


	SharedMemoryBuffer();
	SharedMemoryBuffer(std::string name, size_t bufferSize);
	char* Init(std::string name, size_t bufferSize);
	char* Init(const wchar_t* name, size_t bufferSize);

	/*
	_in_	void*	dest
	_in_	void*	data
	_in_	size_t	length
	Assumes enough space in buffer
	*/
	bool Send(const PVOID& dest, const PVOID data, const size_t& size);

	/*
	_in_	size_t	bytesIntoBuffer
	_in_	void*	data
	_in_	size_t	length
	Assumes enough space in buffer
	*/
	bool Send(const size_t, const PVOID data, const size_t& size);

	/*
	_out_	void*	dest
	_in_	void*	buffer
	_in_	size_t	size
	*/
	bool Recieve(PVOID dest, const PVOID buf, const size_t& size);

	/*
	_out_	void*	dest
	_in_	size_t	bytesIntoBuffer
	_in_	size_t	size
	*/
	bool Recieve(PVOID dest, const size_t bytesIntoBuffer, const size_t& size);

	char* GetBuffer();

	~SharedMemoryBuffer();
};


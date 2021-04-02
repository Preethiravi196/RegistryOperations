#pragma once
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <cstdio>
#include <vector>
#include <strsafe.h>
#include <string>
#include <bitset>
#include <atlstr.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define BUFFER 8192

using namespace std;

typedef basic_string<TCHAR> tstring;

class RegistryOperationHandler
{
private:
	HKEY hkey_root;
	wstring sub_key;

public:
	RegistryOperationHandler( HKEY , wstring );

	RegistryOperationHandler();

	BOOL Path_validation();

	BOOL RegistryAdd(wstring);

	BOOL RegistryAdd(int,wstring);

	BOOL RegistryDelete(wstring);

	BOOL RegistryDelete(LPTSTR);

	BOOL RegistryModify( int , wstring , wstring );

	BOOL RegistryModify( wstring );

	BOOL RegisterCopy(wstring );

	BOOL RegisterCopy(wstring , wstring);

	void RegistryDisplay(HKEY , int );

	void print_tab(int n);

	long set_value(DWORD, wstring);

	wstring get_string_data();

	DWORD get_dword_data();

	string get_multi_string_data();
	
	bitset<16> get_binary_data();

	string get_error_message(LONG);

};
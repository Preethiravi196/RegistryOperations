#include "RegistryOperationHandler.h"

RegistryOperationHandler::RegistryOperationHandler(HKEY hkey_root, wstring sub_key)
{
	this->hkey_root = hkey_root;
	this->sub_key = sub_key;
}

RegistryOperationHandler::RegistryOperationHandler()
{
	hkey_root = NULL;
	sub_key = _T("");
}

BOOL RegistryOperationHandler::Path_validation()
{
	HKEY hkey;

	if (RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL RegistryOperationHandler::RegistryAdd(wstring new_sub_key)
{
	HKEY hKey = NULL;
	BOOL result = FALSE;

	if (sub_key.length() > 0)
	{
		sub_key = sub_key + L"\\" + new_sub_key;
	}
	else
	{
		sub_key = new_sub_key;
	}

	//Step 1: Open the key
	LONG sts = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_READ, &hKey);

	//Step 2: If failed, create the key
	if (ERROR_NO_MATCH == sts || ERROR_FILE_NOT_FOUND == sts)
	{
		cout << "\nCreating registry key " << endl;

		LONG res = RegCreateKeyEx(hkey_root, sub_key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);

		if (ERROR_SUCCESS != res)
		{
			cout << "\nCould not create registry key!! Error: " << get_error_message(res) << endl;
			result = FALSE;
		}
		else
		{
			RegCloseKey(hKey);
			cout << "\nSuccess: Key created" << endl;
			result = TRUE;
		}
	}
	else if (ERROR_SUCCESS != sts)
	{
		cout << "\nCannot open registry key!! Error: " << get_error_message(sts) << endl;
		result = FALSE;
	}

	return result;
}

BOOL RegistryOperationHandler::RegistryAdd(int option, wstring valuename)
{
	DWORD dtype = REG_SZ;

	if (option == 1)
	{
		dtype = REG_SZ;
	}
	else if (option == 2)
	{
		dtype = REG_BINARY;
	}
	else if (option == 3)
	{
		dtype = REG_DWORD;
	}
	else if (option == 4)
	{
		dtype = REG_MULTI_SZ;
	}

	LONG lRes = set_value(dtype, valuename);

	if (lRes != ERROR_SUCCESS)
	{
		cout << "\nCreation of new value is not success!! Error: " << get_error_message(lRes) << endl;
		return FALSE;
	}
	else
	{
		cout << "\nNew value added!!";
		return TRUE;
	}
}

BOOL RegistryOperationHandler::RegistryDelete(LPTSTR lpSubKey)
{
	LPTSTR lpEnd;
	LONG lResult;
	DWORD dwSize;
	TCHAR szName[MAX_PATH];
	HKEY hKey;

	// First, see if we can delete the key without having
	// to recurse.

	lResult = RegDeleteKey(hkey_root, lpSubKey);

	if (lResult == ERROR_SUCCESS)
	{
		return TRUE;
	}

	lResult = RegOpenKeyEx(hkey_root, lpSubKey, 0, KEY_READ, &hKey);

	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	// Check for an ending slash and add one if it is missing.

	lpEnd = lpSubKey + lstrlen(lpSubKey);

	if (*(lpEnd - 1) != TEXT('\\'))
	{
		*lpEnd = TEXT('\\');
		lpEnd++;
		*lpEnd = TEXT('\0');
	}

	// Enumerate the keys

	dwSize = MAX_PATH;
	lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, NULL);

	if (lResult == ERROR_SUCCESS)
	{
		do {

			*lpEnd = TEXT('\0');
			StringCchCat(lpSubKey, MAX_PATH * 2, szName);

			if (!RegistryDelete(lpSubKey))
			{
				break;
			}

			dwSize = MAX_PATH;

			lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL, NULL, NULL, NULL);

		} while (lResult == ERROR_SUCCESS);
	}

	lpEnd--;
	*lpEnd = TEXT('\0');

	RegCloseKey(hKey);

	// Try again to delete the key.

	lResult = RegDeleteKey(hkey_root, lpSubKey);

	if (lResult == ERROR_SUCCESS)
	{
		return TRUE;
	}

	return FALSE;

}

BOOL RegistryOperationHandler::RegistryDelete(wstring valuename)
{
	HKEY hkey = NULL;
	LONG res;

	if ((res = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &hkey)) == ERROR_SUCCESS)
	{
		if ((res = RegDeleteValue(hkey, valuename.c_str())) == ERROR_SUCCESS)
		{
			return TRUE;
		}
	}
	cout << "Error: " << get_error_message(res) << endl;
	return FALSE;
}

BOOL RegistryOperationHandler::RegistryModify(wstring valuename)
{
	HKEY hkey;
	DWORD dtype;
	LONG lRes;
	BOOL result;

	if (ERROR_SUCCESS == (lRes = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &hkey)))
	{
		if (ERROR_SUCCESS == (lRes = RegGetValueW(hkey_root, sub_key.c_str(), valuename.c_str(), RRF_RT_ANY, &dtype, NULL, NULL)))
		{
			lRes = set_value(dtype, valuename);
		}
	}
	result = (lRes == ERROR_SUCCESS) ? TRUE : FALSE;

	if (!result)
	{
		cout << "\nWriting failure!! Error : " << get_error_message(lRes) << endl;
	}

	return result;
}

BOOL RegistryOperationHandler::RegistryModify(int option, wstring old_name, wstring new_name)
{
	HKEY hkey = NULL;
	LONG res;

	if (option == 1)
	{
		if (ERROR_SUCCESS == (res = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &hkey)))
		{
			res = RegRenameKey(hkey, old_name.c_str(), new_name.c_str());
		}
	}
	else
	{
		char value[255];
		DWORD BufferSize = BUFFER;
		DWORD dtype;

		if (ERROR_SUCCESS == (res = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &hkey)))
		{
			if (ERROR_MORE_DATA == (res = RegGetValueW(hkey_root, sub_key.c_str(), old_name.c_str(), RRF_RT_ANY, &dtype, (PVOID)(&value), &BufferSize)))
			{
				BufferSize = size(value);
			}

			if (ERROR_SUCCESS == (res = RegGetValueW(hkey_root, sub_key.c_str(), old_name.c_str(), RRF_RT_ANY, &dtype, (PVOID)(&value), &BufferSize)))
			{
				res = RegSetValueEx(hkey, new_name.c_str(), 0, dtype, (LPBYTE)&value, BufferSize);
			}
		}
		(res == ERROR_SUCCESS) ? RegistryDelete(old_name) : res;
	}

	BOOL result = (res == ERROR_SUCCESS) ? TRUE : FALSE;

	if (!result)
	{
		cout << "\nWriting failure!! Error : " << get_error_message(res) << endl;
	}

	return result;

}

BOOL RegistryOperationHandler::RegisterCopy(wstring des_sub_key)
{
	HKEY destination, source;
	LONG res;
	BOOL result = FALSE;

	if ((res = RegOpenKeyEx(hkey_root, des_sub_key.c_str(), 0, KEY_ALL_ACCESS, &destination)) != ERROR_SUCCESS)
	{
		res = RegCreateKeyEx(hkey_root, des_sub_key.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &destination, NULL);
	}
	if ((res = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &source)) == ERROR_SUCCESS)
	{
		if ((res = RegCopyTree(source, nullptr, destination)) == ERROR_SUCCESS)
		{
			RegCloseKey(destination);
			RegCloseKey(source);

			cout << "Copy of sub tree created\n";
			result = TRUE;
		}
		else
		{
			cout << "Error: " << get_error_message(res) << endl;
		}
	}

	return result;
}

BOOL RegistryOperationHandler::RegisterCopy(wstring des_sub_key_path, wstring valuename)
{
	HKEY s_hkey, d_hkey;
	DWORD dtype;
	char value[255];
	DWORD BufferSize = BUFFER;
	LONG res;
	BOOL result;

	if (ERROR_SUCCESS == (res = RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_ALL_ACCESS, &s_hkey)))
	{
		if (ERROR_MORE_DATA == (res = RegGetValueW(hkey_root, sub_key.c_str(), valuename.c_str(), RRF_RT_ANY, &dtype, (PVOID)(&value), &BufferSize)))
		{
			BufferSize = size(value);
		}

		if (ERROR_SUCCESS == (res = RegGetValueW(hkey_root, sub_key.c_str(), valuename.c_str(), RRF_RT_ANY, &dtype, (PVOID)(&value), &BufferSize)))
		{
			if (ERROR_SUCCESS == (res = RegOpenKeyEx(hkey_root, des_sub_key_path.c_str(), 0, KEY_ALL_ACCESS, &d_hkey)))
			{
				res = RegSetValueEx(d_hkey, valuename.c_str(), 0, dtype, (LPBYTE)&value, BufferSize);
			}
		}
	}
	result = (res == ERROR_SUCCESS) ? TRUE : FALSE;

	if (result)
	{
		cout << "Error: " << get_error_message(res) << endl;
	}
	return result;

}

void RegistryOperationHandler::RegistryDisplay(HKEY hKey, int RecursiveDepth)
{
	TCHAR  achKey[MAX_KEY_LENGTH], achValue[MAX_VALUE_NAME];
	DWORD  cbName, cSubKeys = 0, cValues, cchMaxValue, cbMaxValueData, index, cchValue = MAX_VALUE_NAME;
	LONG retCode;

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, &cValues, &cchMaxValue, &cbMaxValueData, NULL, NULL);

	if (retCode != ERROR_SUCCESS)
		return;

	// Enumerate the key values. 
	if (cValues)
	{
		print_tab(RecursiveDepth - 1);
		printf("values\n");

		for (index = 0; index < cValues; ++index)
		{
			cchValue = MAX_VALUE_NAME;
			retCode = RegEnumValue(hKey, index, achValue, &cchValue, NULL, NULL, NULL, NULL);
			if (retCode == ERROR_SUCCESS)
			{
				print_tab(RecursiveDepth - 1);
				_tprintf(TEXT("-> %s\n"), achValue);
			}
		}
	}

	// Enumerate the subkeys.
	if (cSubKeys)
	{
		for (index = 0; index < cSubKeys; ++index)
		{
			cbName = MAX_KEY_LENGTH;
			retCode = RegEnumKeyEx(hKey, index, achKey, &cbName, NULL, NULL, NULL, NULL);

			if (retCode == ERROR_SUCCESS)
			{
				print_tab(RecursiveDepth - 1);
				_tprintf(TEXT("(%d) %s\n"), index + 1, achKey);

				HKEY hTestKey;
				if (RegOpenKeyEx(hKey, achKey, 0, KEY_ALL_ACCESS, &hTestKey) == ERROR_SUCCESS)
				{
					RegistryDisplay(hTestKey, RecursiveDepth + 1);
					RegCloseKey(hTestKey);
				}
			}
		}
	}
}

void RegistryOperationHandler::print_tab(int n)
{
	for (int i = 0; i < n; i++)
		std::cout << "\t";
}

LONG RegistryOperationHandler::set_value(DWORD dtype, wstring valuename)
{
	long lRes = !ERROR_SUCCESS;
	HKEY hkey;

	if (ERROR_SUCCESS == RegOpenKeyEx(hkey_root, sub_key.c_str(), 0, KEY_SET_VALUE, &hkey))
	{
		if (dtype == REG_SZ)
		{
			wstring data = get_string_data();
			lRes = RegSetValueEx(hkey, valuename.c_str(), 0, REG_SZ, (LPBYTE)(data.c_str()), (data.size() + 1) * sizeof(wchar_t));
		}
		else if (dtype == REG_BINARY)
		{
			bitset<16> data = get_binary_data();
			lRes = RegSetValueEx(hkey, valuename.c_str(), 0, REG_BINARY, (LPBYTE)(&data), sizeof(data));
		}
		else if (dtype == REG_DWORD)
		{
			DWORD data = get_dword_data();
			lRes = RegSetValueEx(hkey, valuename.c_str(), 0, REG_DWORD, (LPBYTE)(&data), sizeof(data));

		}
		else if (dtype == REG_MULTI_SZ)
		{
			string s_data = get_multi_string_data();
			TCHAR data[MAX_PATH * 2];
			_tcscpy_s(data, CA2T(s_data.c_str()));

			lRes = RegSetValueEx(hkey, valuename.c_str(), 0, REG_MULTI_SZ, (LPBYTE)&data, (_tcsclen(data) * 2) + sizeof(TCHAR));
		}

		RegCloseKey(hkey);
		return lRes;

	}
}

wstring RegistryOperationHandler::get_string_data()
{
	wstring data;

	cout << "Enter the value: ";
	getline(wcin, data);

	return data;
}

DWORD RegistryOperationHandler::get_dword_data()
{
	DWORD data;

	cout << "Enter the value: ";
	cin >> data;

	return data;
}

string RegistryOperationHandler::get_multi_string_data()
{
	cout << "Enter the value: ";
	string input;
	string line;

	while (getline(cin, line))
	{
		if (line == "$")
			break;

		line = line + "\n";
		input += line;
	}

	return input;
}

bitset<16> RegistryOperationHandler::get_binary_data()
{
	bitset<16> data;

	cout << "Enter the value: ";
	cin >> data;

	return data;
}

string RegistryOperationHandler::get_error_message(LONG res)
{
	LPSTR messageBuffer = nullptr;

	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, res, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

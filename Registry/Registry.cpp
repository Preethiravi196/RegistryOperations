#include "RegistryOperationHandler.h"

HKEY Return_new_root()
{
	HKEY Hkeyroot = HKEY_CURRENT_USER;
	string key_name;
	int root_type_option;

	cout << "\nChoose the type of root:\n1.HKEY_CLASSES_ROOT\n2.HKEY_CURRENT_USER\n3.HKEY_LOCAL_MACHINE\n4.HKEY_USERS\nYour option: ";
	cin >> root_type_option;

	switch (root_type_option)
	{
	case 1:
	{
		Hkeyroot = HKEY_CLASSES_ROOT;
		break;
	}
	case 2:
	{
		Hkeyroot = HKEY_CURRENT_USER;
		break;
	}
	case 3:
	{
		Hkeyroot = HKEY_LOCAL_MACHINE;
		break;
	}
	case 4:
	{
		Hkeyroot = HKEY_USERS;
		break;
	}
	default:
	{
		cout << "Invalid option!!By default the root is HKEY_CURRENT_USER\n\n";
		break;
	}
	}
	return Hkeyroot;
}

void message_on_operation(BOOL result , string operation)
{
	if (result)
	{
		cout << endl << operation << "operation success!!\n";
	}
	else
	{
		cout << endl << operation << "operation failure!!\n";
	}

}

int main()
{
	HKEY hkeyroot;
	
	int option;
	char ch;
	do
	{
		wstring subkey_path = _T("");

		cout << "\nRegistry operations:\n1.Add\n2.Delete\n3.Display\n4.Modify\n5.Copy\nYour option: ";
		cin >> option;
		
		if (option >= 1 && option <= 5)
		{
			hkeyroot = Return_new_root();
			cin.ignore();
			cout << "Enter the sub-key path: ";
			getline(wcin, subkey_path);

			RegistryOperationHandler registry_operation_handler_obj(hkeyroot, subkey_path);

			if (registry_operation_handler_obj.Path_validation())
			{
				switch (option)
				{
				case 1:
				{
					int add_option;
					BOOL result = FALSE;

					cout << "What do you want to add? Choose an option:\n1.Sub-key\t2.Value\nYour option: ";
					cin >> add_option;

					if (add_option == 1)
					{
						wstring new_sub_key = _T("");

						cin.ignore();
						cout << "Enter the name of new sub-key: ";
						getline(wcin, new_sub_key);

						result = registry_operation_handler_obj.RegistryAdd(new_sub_key);
					}
					else if (add_option == 2)
					{
						int val_option;
						wstring valuename;

						cout << "\nChoose the type of data:\n1.REG_SZ\n2.REG_BINARY\n3.REG_DWORD\n4.REG_MULTI_SZ\nYour option: ";
						cin >> val_option;
						
						if (val_option >= 1 && val_option <= 4)
						{
							cin.ignore();

							cout << "\nEnter the name of data: ";
							getline(wcin, valuename);

							result = registry_operation_handler_obj.RegistryAdd(val_option, valuename);
						}
						else
						{
							cout << "Invalid option!!\n";
						}
					}
					else
					{
						cout << "Invalid option!!\n";
					}
					
					message_on_operation( result , "Add ");
					break;
				}
				case 2:
				{
					int del_option;
					BOOL result = FALSE;

					cout << "What do you want to delete? Choose an option:\n1.Sub-key\t2.Value\nYour Option: ";
					cin >> del_option;

					if (del_option == 1)
					{
						result = registry_operation_handler_obj.RegistryDelete((LPTSTR)subkey_path.c_str());
					}
					else if (del_option == 2)
					{
						wstring valuename;
						cin.ignore();
						cout << "Enter the name of data: ";
						getline(wcin, valuename);

						result = registry_operation_handler_obj.RegistryDelete(valuename);
					}
					else
					{
						cout << "Invalid option!!\n";
					}
					
					message_on_operation(result , "Delete ");
					break;
				}
				case 3:
				{
					HKEY hTestKey;
					const TCHAR* subkey = subkey_path.c_str();

					StringCchCat((LPWSTR)subkey, MAX_PATH * 2, _T("\\"));

					if (RegOpenKeyEx(HKEY_CURRENT_USER, subkey , 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY , &hTestKey) == ERROR_SUCCESS)
					{
						wcout << "Path: " << subkey_path << "\n" ;

						registry_operation_handler_obj.RegistryDisplay(hTestKey, 1);
						RegCloseKey( hTestKey );
					}
					break;
				}
				case 4:
				{
					int mod_option;
					BOOL result = FALSE;

					cout << "\nWhat do you want to modify? Choose an option:\n1.Sub-key name\t2.Name of key\t3.Value of key\nYour Option: ";
					cin >> mod_option;

					if (mod_option == 1 || mod_option == 2 )
					{
						wstring old_name, new_name;

						cin.ignore();
						cout << "Enter the name which you want to change: ";
						getline(wcin, old_name);

						cout << "Enter the new name: ";
						getline(wcin, new_name);

						result = registry_operation_handler_obj.RegistryModify(option , old_name, new_name);
					}
					else if (mod_option == 3)
					{
						wstring valuename;

						cin.ignore();
						cout << "Enter the name of the key to change value: ";
						getline(wcin, valuename);

						result = registry_operation_handler_obj.RegistryModify(valuename);
					}
					else
					{
						cout << "Invalid option!!\n";
					}
					
					message_on_operation(result , "Modify ");
					break;
				}
				case 5:
				{
					BOOL result = FALSE;
					int copy_option;

					cout << "What do you want to copy? Choose an option:\n1.Sub-tree\t2.Value\nYour option: ";
					cin >> copy_option;

					if (copy_option == 1)
					{
						wstring des_sub_key = _T("");
						cin.ignore();
						cout << "Enter the sub-key path where you want to copy the sub-tree: ";
						getline(wcin, des_sub_key);

						result = registry_operation_handler_obj.RegisterCopy(des_sub_key);
					}
					else if(copy_option == 2)
					{
						wstring des_sub_key_path = _T("") , valuename;

						cin.ignore();
						cout << "Enter the name of key which you want to copy: ";
						getline(wcin, valuename);

						cout << "Enter the sub-key path where you want to copy the sub-tree: ";
						getline(wcin, des_sub_key_path);

						result = registry_operation_handler_obj.RegisterCopy( des_sub_key_path , valuename);
					}
					else
					{
						cout << "Invalid option!!\n";
					}
					message_on_operation(result, "Copy ");
					break;
				}
				}
			}
			else
			{
				cout << "\nInvalid registry path!!\n";
			}
		}
		else
		{
			cout << "Invalid option!!\n";
		}

		cout << "\n\nDo you want to continue (Y or N): ";
		cin >> ch;

	} while (ch == 'Y' || ch == 'y');

}
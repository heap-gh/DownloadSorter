
#include "DownloadSorter.h"

namespace fs = boost::filesystem;

DownloadSorter::DownloadSorter()
{
    
    this->exit = false;
    this->changes = false;

    // get the download folder path
    if (!getDownloadFolder())
        this->exit = true;

    // get the windows username
    if (!getUserName())
        this->exit = true;

    // check instrallation folder and settings/config file
    if (!locateInstallationFolderAndFiles())
        this->exit = true;

    // load settings from json

    

    // check if ready to start

    if (!this->exit)
    {
        if (loadSettings(false) &&
            loadConfig())
        {
            this->initialized = true;
            checkChanges();
        }
        else
        {
            this->exit = true;
        }
    }
    else
    {
        
    }

}


/*
    A function that can handle some non-critical errors from StatusCode.h
*/

bool DownloadSorter::handleError(int statusCode)
{

    switch (statusCode)
    {

    /*
        delete old path and create a new directory
    */

    case D_BASEDIR_NOT_DIRECTORY:
    {

        // delete "this->applicationBasePath" and create a new folder in "this->applicationBasePath"

        if (!RemoveDirectoryW(this->applicationBasePath.c_str()))
        {
            DWORD error = GetLastError();
            if (error == ERROR_DIR_NOT_EMPTY)
            {
                std::cout << "Failed to delete folder: Directory is not empty." << std::endl;
            }
            else
            {
                std::cout << "Failed to delete folder. Error code: " << error << std::endl;
            }
            return false;
        }

        if (!CreateDirectory(this->applicationBasePath.c_str(), NULL))
        {
            std::cout << "Failed to create folder. Error code: " << GetLastError() << std::endl;
            return false;
        }

        return true;

    } break;

    /*
        Create a new directory
    */

    case D_BASEDIR_NOT_FOUND:
    {

        if (!CreateDirectory(this->applicationBasePath.c_str(), NULL))
        {
            std::cout << "Failed to create folder. Error code: " << GetLastError() << std::endl;
            return false;
        }

        return true;

    } break;

    /*
        Create a new config file
    */

    case D_CONFIGFILE_NOT_FOUND:
    {
        

        HANDLE hFile = CreateFile(this->applicationConfigFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create file. Error code: " << GetLastError() << std::endl;
            this->exit = true;
            return false;
        }

        CloseHandle(hFile);

        return true;

    } break;

    /*
        Create a new settings file
    */

    case D_SETTINGS_FILE_NOT_FOUND:
    {

        HANDLE hFile = CreateFile(this->applicationSettingsFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create file. Error code: " << GetLastError() << std::endl;
            this->exit = true;
            return false;
        }

        CloseHandle(hFile);

        return true;

    } break;


    default:
    {

        return true;

    } break;

    }
    

}


/*
    Loads settings from "applicationSettingsFilePath"

    defaultSettings == true:
        The default settings from settings object are kept and the settings.json file is filled with these

    defaultSettings == false:
        The settings from the settings.json file are read and saved to this->settings
*/

bool DownloadSorter::loadSettings(bool defaultSettings)
{

    if (defaultSettings)
    {
        json data;
        // write the default data from settings struct to json file
        data["loadDefaultConfig"] = true;

        this->configJSON = data;

        std::ofstream configFile(this->applicationSettingsFilePath.c_str());
        if (!configFile.is_open())
        {
            std::wcout << "Failed to open file for writing." << std::endl;
            DWORD error = GetLastError();
            std::wcout << "Error code: " << error << std::endl;
            LPWSTR errorMessageBuffer = nullptr;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMessageBuffer, 0, NULL);
            if (errorMessageBuffer)
            {
                std::wcout << "Error message: " << errorMessageBuffer << std::endl;
                LocalFree(errorMessageBuffer);
            }
            this->exit = true;
            return false;
        }

        // Write JSON data to the file
        configFile << std::setw(4) << data << std::endl;

        // Close the file stream
        configFile.close();
        
    }
    else
    {
        std::ifstream file(this->applicationSettingsFilePath);

        // Check if the file is opened successfully
        if (!file.is_open())
        {
            std::cout << "Failed to open file." << std::endl;
            return false;
        }

        // Read the content of the file into a JSON object
        json data;
        file >> data;

        file.close();

        // Write the data to the settings
        this->settings.loadDefaultConfig = data["loadDefaultConfig"];
            
        this->settingsJSON = data;

        std::cout << this->settings.loadDefaultConfig << "\n";
    }

    return true;

}


bool DownloadSorter::getUserName()
{

    wchar_t username[UNLEN + 1];
    DWORD size = UNLEN + 1;

    if (!GetUserName(username, &size))
    {
        this->errors.push_back("Unable to get username");
        this->errorCodes.push_back(D_USERNAME);
        
        return false;
    }

    std::wcout << username << "\n";
    this->windowsUsername = username;
    return true;
}


bool DownloadSorter::install()
{


    return true;
}


bool DownloadSorter::locateInstallationFolderAndFiles()
{

    // search user directory

    wchar_t userDir[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, userDir) != S_OK)
    {
        this->errors.push_back("Failed to retrieve user directory");
        this->errorCodes.push_back(D_USERDIR_NOT_LOCATED);
        this->exit = true;
        return false;
    }
    
    // std::wcout << std::wstring(userDir) << "\n";

    // check if directory exists

    this->applicationBasePath = std::wstring(userDir) + L"\\DownloadSorter";

    DWORD ftyp = GetFileAttributesW(this->applicationBasePath.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
    {
        this->errors.push_back("Failed to locate application directory");
        this->errorCodes.push_back(D_BASEDIR_NOT_FOUND);
        if (!handleError(D_BASEDIR_NOT_FOUND))
        {
            this->exit = true;
            return false;
        }
    }
    else if (!(ftyp & FILE_ATTRIBUTE_DIRECTORY))
    {
        this->errors.push_back("Failed to located directory");
        this->errorCodes.push_back(D_BASEDIR_NOT_DIRECTORY);
        if (!handleError(D_BASEDIR_NOT_DIRECTORY))
        {
            this->exit = true;
            return false;
        }

    }
    

    // check if settings file exists

    this->applicationSettingsFilePath = this->applicationBasePath + L"\\settings.json";
    
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(this->applicationSettingsFilePath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        this->errorCodes.push_back(D_SETTINGS_FILE_NOT_FOUND);
        this->errors.push_back("Could not find setttings file");
        if (!handleError(D_SETTINGS_FILE_NOT_FOUND))
        {
            this->exit = true;
            return false;
        }

        // load default settings
        loadSettings(true);

    }


    // check if conifg file exists

    this->applicationConfigFilePath = this->applicationBasePath + L"\\config.json";

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(this->applicationConfigFilePath.c_str()) && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        this->errorCodes.push_back(D_CONFIGFILE_NOT_FOUND);
        this->errors.push_back("Could not find config file");
        if (!handleError(D_CONFIGFILE_NOT_FOUND))
        {
            this->exit = true;
            return false;
        }

    }


    return true;
}


bool DownloadSorter::loadConfig()
{

    if (settings.loadDefaultConfig)
    {

        wchar_t* picturePath = nullptr;
        if (SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &picturePath) != S_OK)
        {
            std::cout << "Failed to retrieve Pictures directory." << std::endl;
            return false;
        }

        std::wstring picturesDir = picturePath;
        CoTaskMemFree(picturePath); // Free allocated memory


        json data;
        data[".jpg"] = to_utf8(picturesDir);

        std::ofstream configFile(this->applicationConfigFilePath.c_str());
        if (!configFile.is_open())
        {
            std::wcout << "Failed to open file for writing." << std::endl;
            DWORD error = GetLastError();
            std::wcout << "Error code: " << error << std::endl;
            LPWSTR errorMessageBuffer = nullptr;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                            NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&errorMessageBuffer, 0, NULL);
            if (errorMessageBuffer)
            {
                std::wcout << "Error message: " << errorMessageBuffer << std::endl;
                LocalFree(errorMessageBuffer);
            }
            this->exit = true;
            return false;
        }

        // Write JSON data to the file
        configFile << std::setw(4) << data << std::endl;

        // Close the file stream
        configFile.close();

        
        this->configJSON = data;

    }
    else
    {
        
        // just load the config 
        std::ifstream file(this->applicationSettingsFilePath);

        // Check if the file is opened successfully
        if (!file.is_open())
        {
            std::cout << "Failed to open file." << std::endl;
            return false;
        }

        // Read the content of the file into a JSON object
        json data;
        file >> data;

        file.close();


        this->configJSON = data;

        std::cout << this->settings.loadDefaultConfig << "\n";
    }


    return true;


}


bool DownloadSorter::checkChanges()
{

    try
    {
        for (fs::directory_entry& entry : fs::directory_iterator(this->downloadFolderPath))
        {
            if (fs::is_regular_file(entry))
            {
                //std::cout << "File: " << entry.path().extension().string() << std::endl;
                try
                {
                    std::string path = getRulePath(entry.path().extension().string());
                    
                    // move file to path

                }   
                catch (nlohmann::json_abi_v3_11_3::detail::type_error)
                {
                    std::cout << "XD\n";
                }
            }
        }
    }
    catch (boost::filesystem::filesystem_error& ex)
    { 
        std::cout << ex.what() << "\n";
    }


    return true;
}

/*
    Returns the destination folder for a certain file extension
*/
std::string DownloadSorter::getRulePath(std::string extension)
{
    return this->configJSON[extension];
}


bool DownloadSorter::getDownloadFolder()
{


    const wchar_t* userProfile = _wgetenv(L"USERPROFILE");

    if (userProfile != nullptr)
    {
        this->downloadFolderPath = std::wstring(userProfile) + L"\\Downloads";
        std::wcout << L"Downloads folder path: " << downloadFolderPath << std::endl;
    }
    else
    {
        std::wcout << L"Failed to retrieve user profile directory." << std::endl;
        return false;

    }
    

    return true;

}



void DownloadSorter::start()
{

    while (true)
    {
        
        checkChanges();
        
        Sleep(2000);

    }


}
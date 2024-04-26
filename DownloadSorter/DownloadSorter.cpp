
#include "DownloadSorter.h"

using json = nlohmann::json;

DownloadSorter::DownloadSorter()
{
    
    this->exit = false;
    this->changes = false;

    if (!getDownloadFolder())
        this->exit = true;

    if (!getUserName())
        this->exit = true;

    if (!locateInstallationFolderAndFiles())
        this->exit = true;

    // load settings from json

    // ready to start

    if (!this->exit)
    {
        loadConfig();
    }


    

    for (int x = 0; x < this->errors.size(); x++)
        std::cout << this->errors.at(x);
    

}



bool DownloadSorter::handleError(int statusCode)
{

    switch (statusCode)
    {

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

    case D_BASEDIR_NOT_FOUND:
    {

        if (!CreateDirectory(this->applicationBasePath.c_str(), NULL))
        {
            std::cout << "Failed to create folder. Error code: " << GetLastError() << std::endl;
            return false;
        }

        return true;

    } break;

    case D_CONFIGFILE_NOT_FOUND:
    {
        // create directory

        HANDLE hFile = CreateFile(this->applicationConfigFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create file. Error code: " << GetLastError() << std::endl;
            return false;
        }

        CloseHandle(hFile);

        return true;

    } break;

    case D_SETTINGS_FILE_NOT_FOUND:
    {

        HANDLE hFile = CreateFile(this->applicationSettingsFilePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            std::cout << "Failed to create file. Error code: " << GetLastError() << std::endl;
            return false;
        }

        loadDefaultSettings();

    } break;


    default:
    {

        return true;

    } break;

    }
    

}


bool DownloadSorter::loadDefaultSettings()
{

    // implement


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
    
    std::wcout << std::wstring(userDir) << "\n";

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

    }
    else
    {
        


    }


    return true;


}


bool DownloadSorter::checkChanges()
{


    return true;
}


bool DownloadSorter::getDownloadFolder()
{


    // Buffer to hold the path
    TCHAR path[MAX_PATH];

    // Get the Download folder path
    if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, path) != S_OK)
    {
        std::cout << "Error: Unable to get Download folder path." << std::endl;
        this->errors.push_back("Unable to get Download folder path!");
        this->errorCodes.push_back(D_DOWNLOADPATH_NOT_FOUND);
        return false;
    }

    // Append 'Downloads' folder to the path
    std::wstring downloadFolder = path;
    downloadFolder += L"\\Downloads";

    std::wcout << downloadFolder << "\n";

    this->downloadFolderPath = downloadFolder;

    return true;

}
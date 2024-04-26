#pragma once

#include <windows.h>
#include <vector>
#include <iostream>
#include <ShlObj.h>
#include <thread>
#include <mutex>
#include <lmcons.h>
#include <fstream>

#include "Settings.h"
#include "nlohmann/json.hpp"
#include "StatusCodes.h"
#include "helper.h"

class DownloadSorter
{

	Settings					settings;
	std::wstring				downloadFolderPath;
	std::vector<std::string>	errors;
	std::vector<unsigned int>   errorCodes;
	std::wstring				windowsUsername;
	std::wstring				applicationBasePath;
	std::wstring				applicationConfigFilePath;
	std::wstring				applicationSettingsFilePath;
	bool						exit;
	bool						changes;
	bool						requireInstall;

public:
	DownloadSorter();

private:
	
	bool locateInstallationFolderAndFiles();
	bool install();
	bool loadConfig();
	bool checkChanges();
	bool getDownloadFolder();
	bool getUserName();
	bool handleError(int statusCode);
	bool loadDefaultSettings();

};
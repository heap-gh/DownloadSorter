#pragma once

#pragma warning(disable: 4996)

#include <windows.h>
#include <vector>
#include <iostream>
#include <ShlObj.h>
#include <thread>
#include <mutex>
#include <lmcons.h>
#include <fstream>
#include <boost/filesystem.hpp>

#include "Settings.h"
#include "nlohmann/json.hpp"
#include "StatusCodes.h"
#include "helper.h"

using json = nlohmann::json;

class DownloadSorter
{

public:
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
	bool						initialized						= false;
	json						configJSON;
	json						settingsJSON;

public:
	DownloadSorter();

private:
	
	bool				locateInstallationFolderAndFiles();
	bool				install();
	bool				loadConfig();
	bool				checkChanges();
	bool				getDownloadFolder();
	bool				getUserName();
	bool				handleError(int statusCode);
	bool				loadSettings(bool defaultSettings);
	std::string			getRulePath(std::string extension);


public:
	void start();

};
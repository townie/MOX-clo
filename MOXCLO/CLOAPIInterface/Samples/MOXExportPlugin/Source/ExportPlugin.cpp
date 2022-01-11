#include "stdafx.h"

#include "ExportPlugin.h"

#include "CLOAPIInterface.h"

#include <string>
#include <fstream>
#include <map>

#if defined(__APPLE__)
#include <unistd.h>
#include <pwd.h>
#endif

#include <ctime>
#include <iostream>
#include <unistd.h>



using namespace std;
using namespace CLOAPI;

static std::string base64_encode(const std::string &in) {

	std::string out;

	int val = 0, valb = -6;
	for (unsigned char c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
	while (out.size() % 4) out.push_back('=');
	return out;
}

string getHomePath()
{
	string homePath = "C:/";

#if defined(__APPLE__)
	const char *homeDir = getenv("HOME");

	if (homeDir == nullptr)
	{
		struct passwd* pwd = getpwuid(getuid());
		if (pwd)
		   homeDir = pwd->pw_dir;
	}

	if (homeDir)
    {
		homePath = homeDir;
        homePath = homePath + "/";
    }
	else
    {
		homePath = "/usr/local/";
    }
#endif

	return homePath;
}



void PostDataToMOX(string FilePath, string MOXID)
{
    if (!REST_API)
        return;

    vector<pair<string, string>> headerNameAndValueList;

    if (MOXID.size() > 0)
    {
        string ExportUrl = "localhost:5000/api/v1/clo/" + MOXID + "/";
        if (UTILITY_API)
             UTILITY_API->DisplayMessageBox(ExportUrl);
        string response = REST_API->CallRESTPostWithMultipartFormData(ExportUrl,
                                                                      FilePath, headerNameAndValueList,
                                                                      "Uploading to MOX");
    }
}


vector<std::string> ExportOBJForMOX()
{
    if (!EXPORT_API)
        return  std::vector<std::string>  { "NoData" };

    Marvelous::ImportExportOption options;
    options.bExportAvatar = false;
    options.bExportGarment = true;
    options.bSaveInZip = false;

    // the other options are given as default. please refer to ImportExportOption class in ExportAPI.h

    vector<string> exportedFilePathList;
    string baseFolder = getHomePath() + "MOXexport/";

    exportedFilePathList = EXPORT_API->ExportOBJ(options);

    return exportedFilePathList;
}



string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}



void MOXExportAndSave()
{
    vector<std::string> ExportPaths = ExportOBJForMOX();

    string MOXID = gen_random(20);
    for (auto& path: ExportPaths){
        PostDataToMOX(path, MOXID);
    }
}


extern CLO_PLUGIN_SPECIFIER void DoFunction()
{

	MOXExportAndSave();
	
}								 	

extern CLO_PLUGIN_SPECIFIER void DoFunctionAfterLoadingCLOFile(const char* fileExtenstion)
{
	if (UTILITY_API != nullptr)
		UTILITY_API->DisplayMessageBox("DoFunctionAferLoadingProject starts... for file type -  " + string(fileExtenstion));
}

extern CLO_PLUGIN_SPECIFIER const char* GetActionName()
{
	const char* actionName = "Plugin";

    actionName = "Export To MOX LOCALHOST";

	return actionName;
}

extern CLO_PLUGIN_SPECIFIER const char* GetObjectNameTreeToAddAction()
{
	const char* objetNameTree = "menu_Setting / menuPlug_In";

	return objetNameTree;
}

extern CLO_PLUGIN_SPECIFIER int GetPositionIndexToAddAction()
{
	return 1; // 0: Above, 1: below (default = 0)
}

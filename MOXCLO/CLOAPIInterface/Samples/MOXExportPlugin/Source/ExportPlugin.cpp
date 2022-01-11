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
#include <regex>


using namespace std;
using namespace CLOAPI;

// Utility Functions

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

string GetEnv()
{
    // Make this dynamic
    // return "prod";
    return "dev";
}


string GetBaseUrl()
{
    if (GetEnv() == "dev")
    {
        return "localhost:5000";
    }

    return "https://www.mox.shopping";
}

string RemoveBrackets(string s)
{
    s.pop_back();
    s.erase(0,1);
    return s;
}

string StantizeString(string s)
{
    std::regex newlines_re("\n+");

    s =  std::regex_replace(s, newlines_re, "");
    std::regex string_re("\"");
    string space = " ";
    if (space.compare(&s[0]))
    {
        s.erase(0,1);
    }
    return std::regex_replace(s, string_re, "");

}

vector<std::string> ParseKeyValue(string KeyValue)
{

    string delimiter = ":";

    size_t pos = 0;
    string key;
    string value;
    while ((pos = KeyValue.find(delimiter)) != string::npos) {
        key = KeyValue.substr(0, pos);
        KeyValue.erase(0, pos + delimiter.length());

    }
    //    UTILITY_API->DisplayMessageBox(key + ":" + KeyValue);
    return  std::vector<std::string> { StantizeString(key), StantizeString(KeyValue)};
}


string GetValueFromMetadataString(string SearchKey)
{
    SearchKey = "\"" + SearchKey + "\"";
    string s = RemoveBrackets(UTILITY_API->GetMetaDataForCurrentGarment());

    string delimiter = ",";

    size_t pos = 0;
    string token;
    string match = "";
    vector<std::string> key_value;

    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        key_value = ParseKeyValue(token);
        if (key_value[0].compare(StantizeString(SearchKey))){
            return StantizeString(key_value[1]);
        }

        s.erase(0, pos + delimiter.length());
    }

    return match;
}


string GenerateUUID(const int len) {
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

// MOX API Functions
void UploadFileToMOX(string FilePath, string GarmentUUID)
{
    if (!REST_API)
        return;

    vector<pair<string, string>> headerNameAndValueList;

    if (GarmentUUID.size() > 0)
    {
        string ExportUrl = GetBaseUrl() + "/api/v1/clo/" + GarmentUUID + "/";
        string response = REST_API->CallRESTPostWithMultipartFormData(ExportUrl,
                                                                      FilePath, headerNameAndValueList,
                                                                      "Uploading to MOX");
    }
}


void PostMetaDataToMOX(string MetaData, string GarmentUUID)
{
    if (!REST_API)
        return;

    vector<pair<string, string>> headerNameAndValueList;
    headerNameAndValueList.push_back(make_pair("Cache-Control", "no-cache"));
    headerNameAndValueList.push_back(make_pair("Content-Type", "application/x-www-form-urlencoded"));

    if (GarmentUUID.size() > 0)
    {
        string MetaDataURL = GetBaseUrl() + "/api/v1/clo/" + GarmentUUID + "/metadata/";

        string response = REST_API->CallRESTPost(MetaDataURL, &MetaData,  headerNameAndValueList, "HTTP Post2 test");
    }
}


// CLO Export functions

void SetupAvatar()
{
    if (UTILITY_API == nullptr)
        return;

    UTILITY_API->SetShowHideAvatar(true);
}


vector<std::string> ExportObjectData()
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


vector<std::string> ExportGLTFData()
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

    exportedFilePathList = EXPORT_API->ExportGLTF(baseFolder + "gltf.gltf", options, false);

    return exportedFilePathList;
}


string ExportPoseData()
{

    string exportPath = getHomePath() + "MOXexport/pose.pose";

    string resultPath = "";
    if (EXPORT_API)
        resultPath = EXPORT_API->ExportPose(exportPath);
    return resultPath;

}
string GetOrSetGarmentUUID()
{
    string GarmentUUIDMetaDataKey = "MOXGarmentUUID";

    string GarmentUUID = GetValueFromMetadataString(GarmentUUIDMetaDataKey);

    UTILITY_API->DisplayMessageBox("GarmentUUID -> " + GarmentUUID + "PREEEE");

    if (GarmentUUID.size() == 0 )
    {
        GarmentUUID = GenerateUUID(20);
        UTILITY_API->ChangeMetaDataValueForCurrentGarment(GarmentUUIDMetaDataKey, GarmentUUID);
    }

    GarmentUUID = GetValueFromMetadataString(GarmentUUIDMetaDataKey);

    return GarmentUUID;

}
string ExportTechPack()
{
    Marvelous::ExportTechpackOption option(Marvelous::CLO_TECH_PACK);
    option.m_bSaveZpac = true;
    option.m_bSaveZprj = true;
    string exportPath = getHomePath() + "MOXexport/TechPack.zip";
    EXPORT_API->ExportTechPack(exportPath, option);
    return exportPath;
}


// MAIN
void MOXExportAndSave()
{
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MOXLastProcessStart", "");
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MOXLastProcessFinish", "");

    //SetupAvatar();

    string GarmentUUID = GetOrSetGarmentUUID();

    string MetaData = UTILITY_API->GetMetaDataForCurrentGarment();

    PostMetaDataToMOX(MetaData, GarmentUUID);

    vector<std::string> ObjPaths = ExportObjectData();

    for (auto& path: ObjPaths){
        UploadFileToMOX(path, GarmentUUID);
    }

    vector<std::string> GLTFPaths =  ExportGLTFData();
    for (auto& path: GLTFPaths){
        UploadFileToMOX(path, GarmentUUID);
    }

    string TechPackExportPath = ExportTechPack();
    UploadFileToMOX(TechPackExportPath, GarmentUUID);
}


// CLO plugin functions
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

    if (GetEnv() == "dev"){
        actionName = "Export To MOX LOCALHOST";
    }
    else {
        actionName = "Export To MOX Production";
    }

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

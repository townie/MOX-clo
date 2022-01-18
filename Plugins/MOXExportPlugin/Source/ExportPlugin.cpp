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
#include <filesystem>

using namespace std;
using namespace CLOAPI;

// Utility Functions
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
        struct passwd *pwd = getpwuid(getuid());
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

string TechPackKey()
{
    return "TechPack";
}

string RemoveBrackets(string s)
{
    s.pop_back();
    s.erase(0, 1);
    return s;
}

string StantizeString(string s)
{
    // remove new lines
    std::regex newlines_re("\n+");
    s = std::regex_replace(s, newlines_re, "");

    // remove " single quote
    std::regex string_re("\"");
    s = std::regex_replace(s, string_re, "");

    // remove whitesapce
    std::regex space_re(" ");
    s = std::regex_replace(s, space_re, "");

    return s;
}

vector<std::string> ParseKeyValue(string KeyValue)
{

    string delimiter = ":";

    size_t pos = 0;
    string key;
    string value;
    while ((pos = KeyValue.find(delimiter)) != string::npos)
    {
        key = KeyValue.substr(0, pos);
        KeyValue.erase(0, pos + delimiter.length());
    }
    return std::vector<std::string>{StantizeString(key), StantizeString(KeyValue)};
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

    while ((pos = s.find(delimiter)) != string::npos)
    {
        token = s.substr(0, pos);
        key_value = ParseKeyValue(token);
        if (StantizeString(key_value[0]).compare(StantizeString(SearchKey)) == 0)
        {
            return StantizeString(key_value[1]);
        }

        s.erase(0, pos + delimiter.length());
    }

    return match;
}

string GenerateUUID(const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
    {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

string GetAuthFile()
{   string ReaderBuffer;
    string Output;
    string configPath = getHomePath() + "/.mox_clo/config.txt";
    ifstream ConfigFile(configPath);
    // Use a while loop together with the getline() function to read the file line by line
    while (getline (ConfigFile, ReaderBuffer)) {
      // Output the text from the file
        Output = Output + ReaderBuffer;
    }
    ConfigFile.close();
    return Output;
}


string GetAuthHeaders()
{
    string id = "Test"; // set the clo id
    string pw = "Test"; // set the clo pw with md5 hashed in lower cases
    string creds = GetAuthFile();
    if (creds.size() < 1)
        creds = base64_encode(id + ":" + pw);

    string basicAuthorizationString = "Basic " + creds;

    return basicAuthorizationString;
}


// MOX API Functions
void UploadFileToMOX(string FilePath, string GarmentUUID, string RevisionNumber)
{
    if (!REST_API)
        return;

    vector<pair<string, string>> headerNameAndValueList;
    headerNameAndValueList.push_back(make_pair("Authorization", GetAuthHeaders()));


    if (GarmentUUID.size() > 0)
    {
        string ExportUrl = GetBaseUrl() + "/api/external/v1/clo/garment/" + GarmentUUID + "/" + RevisionNumber + "/";
        string response = REST_API->CallRESTPostWithMultipartFormData(ExportUrl, FilePath, headerNameAndValueList, "Uploading to MOX");
    }
}

void PostMetaDataToMOX(string MetaData, string GarmentUUID)
{
    if (!REST_API)
        return;

    vector<pair<string, string>> headerNameAndValueList;
    headerNameAndValueList.push_back(make_pair("Cache-Control", "no-cache"));
    headerNameAndValueList.push_back(make_pair("Content-Type", "application/x-www-form-urlencoded"));
    headerNameAndValueList.push_back(make_pair("Authorization", GetAuthHeaders()));

    if (GarmentUUID.size() > 0)
    {
        string MetaDataURL = GetBaseUrl() + "/api/external/v1/clo/garment/" + GarmentUUID + "/metadata/";

        string response = REST_API->CallRESTPost(MetaDataURL, &MetaData, headerNameAndValueList, "Sending MetaData to MOX");
    }
}

string RequestNewRevision(string GarmentUUID)
{
    if (!REST_API)
        return "";

    vector<pair<string, string>> headerNameAndValueList;
    headerNameAndValueList.push_back(make_pair("Cache-Control", "no-cache"));
    headerNameAndValueList.push_back(make_pair("Content-Type", "application/x-www-form-urlencoded"));
    headerNameAndValueList.push_back(make_pair("Authorization", GetAuthHeaders()));

    string RevisionURL = GetBaseUrl() + "/api/external/v1/clo/garment/" + GarmentUUID + "/revision/new/";

    string response = REST_API->CallRESTGet(RevisionURL, headerNameAndValueList, "Requesting a new Garment Revision");

    string s = response.c_str();
    string rev = s.erase(0, s.find(":::") + 3);
    return rev;
}

void SetRevision(string GarmentUUID, string RevisionNumber)
{
    vector<pair<string, string>> headerNameAndValueList;
    headerNameAndValueList.push_back(make_pair("Cache-Control", "no-cache"));
    headerNameAndValueList.push_back(make_pair("Authorization", GetAuthHeaders()));

    string RevisionURL = GetBaseUrl() + "/api/external/v1/clo/garment/" + GarmentUUID + "/revision/" + RevisionNumber+ "/";

    string response = REST_API->CallRESTGet(RevisionURL, headerNameAndValueList, "Setting a new Garment Revision");
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
        return std::vector<std::string>{"NoData"};

    Marvelous::ImportExportOption options;
    options.bExportAvatar = false;
    options.bExportGarment = true;
    options.bSaveInZip = false;
    options.bMetaData = true;
    options.scale = 0.1f; // OBJs need to be stored in m
    // the other options are given as default. please refer to ImportExportOption class in ExportAPI.h

    vector<string> exportedFilePathList;
    string baseFolder = getHomePath() + "MOXexport/";

    exportedFilePathList = EXPORT_API->ExportOBJ(baseFolder + "obj.obj", options);

    return exportedFilePathList;
}

vector<std::string> ExportGLTFData()
{
    if (!EXPORT_API)
        return std::vector<std::string>{"NoData"};

    Marvelous::ImportExportOption options;
    options.bExportAvatar = false;
    options.bExportGarment = true;
    options.bSaveInZip = false;
    options.bMetaData = true;
    options.scale = 0.001f; // GLTFs need to be stored in mm

    // the other options are given as default. please refer to ImportExportOption class in ExportAPI.h

    vector<string> exportedFilePathList;
    string baseFolder = getHomePath() + "MOXexport/";

    exportedFilePathList = EXPORT_API->ExportGLTF(baseFolder + "gltf.gltf", options, false);

    return exportedFilePathList;
}

string GetOrSetGarmentUUID()
{
    string GarmentUUIDMetaDataKey = "MOXGarmentUUID";

    string GarmentUUID = GetValueFromMetadataString(GarmentUUIDMetaDataKey);

    if (GarmentUUID.size() == 0)
    {
        GarmentUUID = GenerateUUID(20);
        UTILITY_API->ChangeMetaDataValueForCurrentGarment(GarmentUUIDMetaDataKey, GarmentUUID);
    }

    GarmentUUID = GetValueFromMetadataString(GarmentUUIDMetaDataKey);

    return GarmentUUID;
}

vector<std::string> ComputeTechPackPaths(string BaseUri)
{
    vector<std::string> paths;
    vector<std::string> ExportedObjectExtensions{".json", ".zprj", ".zpac", ".png"};
    for (auto &ext : ExportedObjectExtensions)
    {
        paths.push_back(BaseUri + TechPackKey() + ext);
    }
    return paths;
}

vector<std::string> ExportTechPack()
{
    Marvelous::ExportTechpackOption option(Marvelous::CLO_TECH_PACK);
    option.m_bSaveZpac = true;
    option.m_bSaveZprj = true;

    string exportRoot = getHomePath() + "MOXexport/";
    EXPORT_API->ExportTechPack(exportRoot + TechPackKey() + ".json", option);

    return ComputeTechPackPaths(exportRoot);
}

void CaptureMoxMetadata()
{
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MOXManaged", "True");
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("ProjectName", UTILITY_API->GetProjectName());
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MajorVersion", to_string(UTILITY_API->GetMajorVersion()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MinorVersion", to_string(UTILITY_API->GetMinorVersion()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("PatchVersion", to_string(UTILITY_API->GetPatchVersion()));
}

void CaptureAvatarMetadata()
{
    vector<string> avatarNameList = EXPORT_API->GetAvatarNameList();
    vector<int> avatarGenderList = EXPORT_API->GetAvatarGenderList();

    if (avatarNameList.size() == avatarGenderList.size()){
        for (size_t i = 0; i < avatarNameList.size(); ++i)
        {
            UTILITY_API->ChangeMetaDataValueForCurrentGarment("Avatar_" + to_string(i) + ":Name:", avatarNameList[i]);

            if (avatarGenderList[i] == 0)
                UTILITY_API->ChangeMetaDataValueForCurrentGarment("Avatar_" + to_string(i) + ":Gender:", "male");
            else if (avatarGenderList[i] == 1)
                UTILITY_API->ChangeMetaDataValueForCurrentGarment("Avatar_" + to_string(i) + ":Gender:", "female");
        }
    }
}


// MAIN
void MOXExportAndSave()
{
    // Setup
    CaptureMoxMetadata();
    CaptureAvatarMetadata();


    // Get the UUID from metadata or create one if first time
    string GarmentUUID = GetOrSetGarmentUUID();

    // Get the raw metadata
    string MetaData = UTILITY_API->GetMetaDataForCurrentGarment();

    PostMetaDataToMOX(MetaData, GarmentUUID);

    // Setup New Revision to assciate data with
    string RevisionNumber = RequestNewRevision(GarmentUUID);

    // Actually export all the data
    vector<std::string> ObjPaths = ExportObjectData();
    for (auto &path : ObjPaths)
    {
        UploadFileToMOX(path, GarmentUUID, RevisionNumber);
    }
    vector<std::string> GLTFPaths = ExportGLTFData();
    for (auto &path : GLTFPaths)
    {
        UploadFileToMOX(path, GarmentUUID, RevisionNumber);
    }
    vector<std::string> TechPackExportPaths = ExportTechPack();
    for (auto &path : TechPackExportPaths)
    {
        UploadFileToMOX(path, GarmentUUID, RevisionNumber);
    }

    SetRevision(GarmentUUID, RevisionNumber);
   // UTILITY_API->DisplayMessageBox("Successful Upload to MOX Garment: " + GarmentUUID + ", Revision: " + RevisionNumber);
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("MOXGarmentRevision", RevisionNumber);


}

// CLO plugin functions
extern CLO_PLUGIN_SPECIFIER void DoFunction()
{
    MOXExportAndSave();
}

extern CLO_PLUGIN_SPECIFIER void DoFunctionAfterLoadingCLOFile(const char *fileExtenstion)
{
    if (UTILITY_API != nullptr)
        UTILITY_API->DisplayMessageBox("DoFunctionAferLoadingProject starts... for file type -  " + string(fileExtenstion));
}

extern CLO_PLUGIN_SPECIFIER const char *GetActionName()
{
    const char *actionName = "Plugin";

    if (GetEnv() == "dev")
    {
        actionName = "Export To MOX LOCALHOST";
    }
    else
    {
        actionName = "Export To MOX Production";
    }

    return actionName;
}

extern CLO_PLUGIN_SPECIFIER const char *GetObjectNameTreeToAddAction()
{
    const char *objetNameTree = "menu_Setting / menuPlug_In";

    return objetNameTree;
}

extern CLO_PLUGIN_SPECIFIER int GetPositionIndexToAddAction()
{
    return 1; // 0: Above, 1: below (default = 0)
}

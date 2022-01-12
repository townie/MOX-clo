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

// MAIN
void Main()
{
    // Setup
    //UTILITY_API->ChangeMetaDataValueForCurrentGarment( , );
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("GetAvatarCount", to_string(EXPORT_API->GetAvatarCount()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("GetSizeCount", to_string(EXPORT_API->GetSizeCount()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("GetCurrentSizeIndex", to_string(EXPORT_API->GetCurrentSizeIndex()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("GetColorwayCount", to_string(EXPORT_API->GetColorwayCount()));
    UTILITY_API->ChangeMetaDataValueForCurrentGarment("GetCurrentColorwayIndex", to_string(EXPORT_API->GetCurrentColorwayIndex()));

    for (auto &name : EXPORT_API->GetAvatarNameList())
    {
        UTILITY_API->DisplayMessageBox("GetAvatarNameList -> " + name);
    }
    for (auto &name : EXPORT_API->GetSizeNameList())
    {
        UTILITY_API->DisplayMessageBox("GetSizeNameList -> " + name);
    }



    UTILITY_API->DisplayMessageBox(UTILITY_API->GetMetaDataForCurrentGarment());

    if (EXPORT_API->GetAvatarCount() > 0)
    {
        UTILITY_API->DisplayMessageBox(" UTILITY_API->SetShowHideAvatar(true)");
        UTILITY_API->SetShowHideAvatar(true);
        UTILITY_API->CreateProgressBar();
        UTILITY_API->SetProgress("Cleanup", 1);
        UTILITY_API->SetProgress("Cleanup", 100);
        UTILITY_API->DeleteProgressBar(false);

        UTILITY_API->DisplayMessageBox(" UTILITY_API->SetShowHideAvatar(false)");
        UTILITY_API->SetShowHideAvatar(false);
        UTILITY_API->CreateProgressBar();
        UTILITY_API->SetProgress("Cleanup", 1);
        UTILITY_API->SetProgress("Cleanup", 100);
        UTILITY_API->DeleteProgressBar(false);
        UTILITY_API->DisplayMessageBox(" UTILITY_API->SetShowHideAvatar(true)");
        UTILITY_API->SetShowHideAvatar(true);
        UTILITY_API->CreateProgressBar();
        UTILITY_API->SetProgress("Cleanup", 1);
        UTILITY_API->SetProgress("Cleanup", 100);
        UTILITY_API->DeleteProgressBar(false);
        UTILITY_API->DisplayMessageBox(" UTILITY_API->SetShowHideAvatar(false)");
        UTILITY_API->SetShowHideAvatar(false);
        UTILITY_API->CreateProgressBar();
        UTILITY_API->SetProgress("Cleanup", 1);
        UTILITY_API->SetProgress("Cleanup", 100);
        UTILITY_API->DeleteProgressBar(false);
    }



    //UTILITY_API->ResetClothArrangement();



}

// CLO plugin functions
extern CLO_PLUGIN_SPECIFIER void DoFunction()
{
    Main();
}

extern CLO_PLUGIN_SPECIFIER void DoFunctionAfterLoadingCLOFile(const char *fileExtenstion)
{
    if (UTILITY_API != nullptr)
        UTILITY_API->DisplayMessageBox("DoFunctionAferLoadingProject starts... for file type -  " + string(fileExtenstion));
}

extern CLO_PLUGIN_SPECIFIER const char *GetActionName()
{
    const char *actionName = "MOX DEBUG TEST";



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

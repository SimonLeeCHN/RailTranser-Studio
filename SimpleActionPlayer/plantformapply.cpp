#include "plantformapply.h"
#include "windows.h"
#include "iostream"
#include "shlobj.h"

//-------------------------------------------------------------------------
// 注册文件图标关联
//-------------------------------------------------------------------------
// @strExt				[in]: 需要检测的文件后缀(.txt)
// @strAppKey			[in]: 扩展名在注册表中的键值(txtfile)
// @strAppName			[in]: 需要关联的引用程序(c:\app\app.exe)
// @strDefaultIcon		[in]: 关联的图标 扩展名为strAppName的图标文件(例如: "C:/MyApp/MyApp.exe,0")
// @strDescribe			[in]: 描述
//-------------------------------------------------------------------------
void RegisterFileRelation(char *strExt,char *strAppKey,char *strAppName, char *strDefaultIcon, char *strDescribe)
{
    char strTemp[_MAX_PATH];
    HKEY hKey;

    RegCreateKey(HKEY_CLASSES_ROOT,strExt,&hKey);
    RegSetValue(hKey,"",REG_SZ,strAppKey,strlen(strAppKey)+1);
    RegCloseKey(hKey);

    RegCreateKey(HKEY_CLASSES_ROOT,strAppKey,&hKey);
    RegSetValue(hKey,"",REG_SZ,strDescribe,strlen(strDescribe)+1);
    RegCloseKey(hKey);

    sprintf(strTemp,"%s\\DefaultIcon",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,"",REG_SZ,strDefaultIcon,strlen(strDefaultIcon)+1);
    RegCloseKey(hKey);

    sprintf(strTemp,"%s\\Shell",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    RegSetValue(hKey,"",REG_SZ,"Open",strlen("Open")+1);
    RegCloseKey(hKey);

    sprintf(strTemp,"%s\\Shell\\Open\\Command",strAppKey);
    RegCreateKey(HKEY_CLASSES_ROOT,strTemp,&hKey);
    sprintf(strTemp,"%s \"%%1\"",strAppName);
    RegSetValue(hKey,"",REG_SZ,strTemp,strlen(strTemp)+1);
    RegCloseKey(hKey);
    SHChangeNotify(SHCNE_ASSOCCHANGED,SHCNF_IDLIST,NULL,NULL);
}

//
//  PlatformHelper.h
//  xZone
//
//  Created by Sharezer on 15/1/17.
//
//

#ifndef __xZone__PlatformHelper__
#define __xZone__PlatformHelper__

#include "cocos2d.h"
#include "json/document.h"

class PlatformHelper {
public:
	static bool getIsTV();
	static std::string getPlatformIP();
	static void getNetworkState();

	static void setDataPath(const std::string& path);
    static bool getAppList(rapidjson::Document& doc);

	static void copyAssetsFileToPath(const std::string& source, const std::string& saveFile);
	static void showToast(const std::string& str);
	static bool uninstallApp(const std::string& package);
    static void installApp(const std::string& path, const std::string& name);
	static void installApp(const std::string& fullFileName);
    //用xPress安装APK和数据包
    static void installApkAndData(const std::string& apkPath, const std::string& dataPath);
	static void runApp(const std::string& package);

	//SDø®◊‹¥Û–°
	static std::string getSDTotalSize();
	//sdø® £”‡»›¡ø
	static std::string getSDAvailableSize();
	//ª˙…Ìƒ⁄¥Ê¥Û–°
	static std::string getRomTotalSize();
	//ª˙…Ìƒ⁄¥Ê £”‡»›¡ø
	static std::string getRomAvailableSize();
};

#define PH PlatformHelper

#endif /* defined(__xZone__PlatformHelper__) */

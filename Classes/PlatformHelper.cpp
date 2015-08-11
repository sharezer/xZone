//
//  PlatformHelper.cpp
//  xZone
//
//  Created by Sharezer on 15/1/17.
//
//

#include "PlatformHelper.h"
#include "Global.h"
#include "LsTools.h"
#include "DataManager.h"
#include "ZoneManager.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#endif

USING_NS_CC;

void PlatformHelper::showToast(const std::string& str)
{
	LS_LOG("%s", str.c_str());
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniShowToast",
		"(Ljava/lang/String;)V");
	if (ret) {
		jobject para = info.env->NewStringUTF(str.c_str());
		info.env->CallStaticVoidMethod(info.classID, info.methodID, para);
	}
#endif
}

bool PlatformHelper::uninstallApp(const std::string& package)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"uninstallApk",
		"(Ljava/lang/String;)V");
	if (ret) {
		jobject para = info.env->NewStringUTF(package.c_str());
		info.env->CallStaticVoidMethod(info.classID, info.methodID, para);
	}
	return ret;
#endif
	return false;
}

void PlatformHelper::installApkAndData(const std::string& apkPath, const std::string& dataPath)
{
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    JniMethodInfo info;
    bool ret = JniHelper::getStaticMethodInfo(info,
                                              "com/xmic/xZone/MyJni",
                                              "installApkAndData",
                                              "(Ljava/lang/String;Ljava/lang/String;)V");
    if (ret) {
        jobject paraPath1 = info.env->NewStringUTF(LsTools::lsStandardPath(apkPath).c_str());
        jobject paraPath2 = info.env->NewStringUTF(LsTools::lsStandardPath(dataPath).c_str());
        info.env->CallStaticVoidMethod(info.classID, info.methodID, paraPath1, paraPath2);
    }
#endif
}

void PlatformHelper::installApp(const std::string& fullFileName)
{
	std::string tempPath = LsTools::lsStandardPath(fullFileName);
	size_t index = tempPath.find_last_of("/");
	std::string dir;
	std::string name;
	if (index != std::string::npos){
		dir = tempPath.substr(0, index + 1);
		name = tempPath.substr(index + 1, tempPath.length());
		installApp(dir, name);
	}
}

void PlatformHelper::installApp(const std::string& path, const std::string& name)
{
	std::string standPath = LsTools::lsStandardPath(path);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"installApk",
		"(Ljava/lang/String;Ljava/lang/String;)V");
	if (ret) {
		jobject paraPath = info.env->NewStringUTF(standPath.c_str());
		jobject paraApk = info.env->NewStringUTF(name.c_str());
		info.env->CallStaticVoidMethod(info.classID, info.methodID, paraPath, paraApk);
	}
#endif
	LS_LOG("install end");
}

bool PlatformHelper::getIsTV()
{
	bool ret = false;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;

	if (JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniGetIsTV",
		"()Z")) {
		ret = info.env->CallStaticBooleanMethod(info.classID, info.methodID);
	}
#endif
	return ret;
}

void PlatformHelper::getNetworkState()
{
	bool ret = false;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;

	if (JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniNetwork",
		"()Z")) {
		ret = info.env->CallStaticBooleanMethod(info.classID, info.methodID);
	}
#endif
	DataManager::getInstance()->_sUserData.netState = ret;
}


#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
extern "C"
{
	JNIEXPORT void JNICALL Java_com_xmic_xZone_MyJni_flushNetwork(JNIEnv*  env, jobject thizs)
	{
		if (!ZM_MAIN)
			return;
		
		//LS_LOG("flushNetwork");
		std::thread t1(PH::getNetworkState);
		t1.detach();
	}

	JNIEXPORT void JNICALL Java_com_xmic_xZone_MyJni_flushUI(JNIEnv*  env, jobject thizs)
	{
		ZM->setFlushAppListWithJNI(true);
	}
};
#endif

void PlatformHelper::setDataPath(const std::string& path)
{
	LS_LOG("%s", path.c_str());
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniSetDataPath",
		"(Ljava/lang/String;)V");
	if (ret) {
		jobject paraPath = info.env->NewStringUTF(path.c_str());
		info.env->CallStaticVoidMethod(info.classID, info.methodID, paraPath);
	}
#endif
}

std::string PlatformHelper::getPlatformIP()
{
	std::string ip;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniGetIP",
		"()Ljava/lang/String;");
	if (ret) {
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID, info.methodID);
		ip = JniHelper::jstring2string(jstr);
	}
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	WSADATA wsaData;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	char hostname[256];
	ret = gethostname(hostname, sizeof(hostname));
	HOSTENT* host = gethostbyname(hostname);
	ip = inet_ntoa(*(in_addr*)*host->h_addr_list);
#else
    ip = "";
#endif
	LS_LOG("IP: %s", ip.c_str());
	return ip;
}

bool PlatformHelper::getAppList(rapidjson::Document& doc)
{
	bool bRet = false;
	std::string checkFile = LsTools::getDataPath() + s_LocalGameFileName;
	std::string str = "install.json";
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniNewGetGameList",
		"(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	if (ret) {
		LS_LOG("%s", str.c_str());
		jobject para1 = info.env->NewStringUTF(str.c_str());
		jobject para2 = info.env->NewStringUTF(checkFile.c_str());
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID,
			info.methodID,
			para1,
			para2);
		//使用jstring2string函数将返回的jstring类型的值转化为c++中的string类型
		//std::string text = JniHelper::jstring2string(jstr);
		//log("%s",text.c_str());
	}
#endif
	str = LsTools::getDataPath() + str;
	rapidjson::Document install, temp;
	if (FileUtils::getInstance()->isFileExist(str)
		&& LsTools::readJsonWithFile(str.c_str(), install)
		&& install.HasMember("package")) {
		rapidjson::Value& installA = install["package"];

		//LsTools::sortJsonArray(installA, "priority", false);
		std::string myGameFile = LsTools::getDataPath() + "myGame.json";
		if (FileUtils::getInstance()->isFileExist(myGameFile)){
			if (LsTools::readJsonWithFile(myGameFile.c_str(), temp) && temp.HasMember("package")){
				//目的:
				//1、删除已经被删除的数据（在temp中有，而install中没有）
				//2、添加新增的数据（在install中有，temp中没有）
				//3、更新已安装列表中属于Zone的（将install 中 state = 1，而temp中不为1或3的变成1）

				rapidjson::Value& tempA = temp["package"];
				for (unsigned int i = 0; i < installA.Size(); i++){
					rapidjson::Value& val = installA[i];
					for (unsigned int j = 0; j < tempA.Size(); j++){
						rapidjson::Value& tempVal = tempA[j];
						val[s_customOrder] = tempVal[s_customOrder].GetInt() == 0 ? 0 : tempVal[s_customOrder].GetInt();

						if (strcmp(val["packageName"].GetString(), tempVal["packageName"].GetString()) == 0){
							switch ((AppStateEnum)tempVal["state"].GetInt())
							{
							case AppStateEnum::Noon:
							case AppStateEnum::MYGAME:
								break;
							case AppStateEnum::PLUS:
							case AppStateEnum::DELETE:
								val["state"] = tempVal["state"];
								break;
							default:
								CCASSERT(0, "unknow state");
								break;
							}
							break;
						}
					}
				}
			}
		}
		
		LsTools::sortJsonArray(installA, s_customOrder, true);
		for (unsigned int i = 0, j = 1; i < installA.Size(); i++){
			rapidjson::Value& tempV = installA[i];
			tempV[s_customOrder] = tempV[s_customOrder].GetInt() == 0 ? 0 : j++;
		}

		if (LsTools::saveJsonFile(myGameFile.c_str(), install))
			bRet = LsTools::readJsonWithFile(myGameFile.c_str(), doc);

		s_isThreadEnd = true;
	}

	return bRet;
}

void PlatformHelper::runApp(const std::string& package)
{
	LS_LOG("package name : %s", package.c_str());
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"runApk",
		"(Ljava/lang/String;)V");
	if (ret) {
		jobject para = info.env->NewStringUTF(package.c_str());
		info.env->CallStaticVoidMethod(info.classID, info.methodID, para);
	}
#endif
}

void PlatformHelper::copyAssetsFileToPath(const std::string& source, const std::string& saveFile)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniCopyAssetsFileToPath",
		"(Ljava/lang/String;Ljava/lang/String;)V");
	if (ret) {
		jobject para1 = info.env->NewStringUTF(source.c_str());
		jobject para2 = info.env->NewStringUTF(LsTools::lsStandardPath(saveFile).c_str());
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID,
			info.methodID,
			para1,
			para2);
	}
#endif
}

std::string PlatformHelper::getSDTotalSize()
{
	std::string size;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniSDTotalSize",
		"()Ljava/lang/String;");
	if (ret) {
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID, info.methodID);
		size = JniHelper::jstring2string(jstr);
	}
#else
	size = "100 GB";
#endif
	return size;
}

std::string PlatformHelper::getSDAvailableSize()
{
	std::string size;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniSDAvailableSize",
		"()Ljava/lang/String;");
	if (ret) {
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID, info.methodID);
		size = JniHelper::jstring2string(jstr);
	}
#else
	size = "100 GB";
#endif
	return size;
}

std::string PlatformHelper::getRomTotalSize()
{
	std::string size;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniRomTotalSize",
		"()Ljava/lang/String;");
	if (ret) {
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID, info.methodID);
		size = JniHelper::jstring2string(jstr);
	}
#else
	size = "100 GB";
#endif
	return size;
}

std::string PlatformHelper::getRomAvailableSize()
{
	std::string size;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo info;
	bool ret = JniHelper::getStaticMethodInfo(info,
		"com/xmic/xZone/MyJni",
		"jniRomAvailableSize",
		"()Ljava/lang/String;");
	if (ret) {
		jstring jstr = (jstring)info.env->CallStaticObjectMethod(info.classID, info.methodID);
		size = JniHelper::jstring2string(jstr);
	}
#else
	size = "100 GB";
#endif
	return size;
}
//
//  LsTools.h
//  xZone
//
//  Created by Sharezer on 15/1/17.
//
//

#ifndef __xZone__LsTools__
#define __xZone__LsTools__

#include "cocos2d.h"
#include "Global.h"

#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "audio/include/SimpleAudioEngine.h"
#include "UIPageViewEx.h"
#include "DLGameList.h"
#include "QR_Encode.h"
#include "PlatformHelper.h"
#include "DataManager.h"
#include "NetManager.h"
#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "external/unzip/unzip.h"

#endif

#define formatStr(format, ...) LsTools::formatToString(format, __VA_ARGS__)
#define int2str(value) formatStr("%d", value)
#define int22str(value) formatStr("%02d", value)
#define long2str(value) formatStr("%ld", value)
#define longlong2str(value) formatStr("%lld", value)
#define float2str(value) formatStr("%f", value)
#define floatFormat(format, value) formatStr(format, value)
#define float2str2(value, number_after_dot) formatStr(formatStr("%%.%df", number_after_dot).c_str(), value)

#define MY_BUFFER_SIZE 8192
#define MY_MAX_FILENAME 512

static const int MAX_FORMAT_LENGTH = 256 * 1024;

#define LS_DEBUG
#ifdef LS_DEBUG
#define LS_LOG(format, ...) LsTools::mylog(format, ##__VA_ARGS__)
#else
#define LS_LOG(format, ...) \
    do {                    \
    } while (0)
#endif // LS_DEBUG
class LsTools {
public:
    static std::string formatToString(const char* format, ...)
    {
        std::string strResult = { 0 };
        if (format != nullptr) {
            //方法1
            /*
			 va_list marker;
			 //初始化变量参数
			 va_start(marker, format);
			 //获取格式化字符串长度
			 size_t nLength = _vscprintf(format, marker) + 1;
			 //创建用于存储格式化字符串的字符数组
			 std::vector<char> vBuffer(nLength, '\0');
			 int nWritten = vsnprintf_s(&vBuffer[0], vBuffer.size(), nLength, format, marker);
			 if (nWritten > 0)
			 strResult = &vBuffer[0];
			 va_end(marker);
			 */

            va_list ap;
            va_start(ap, format);
            char* pBuf = (char*)malloc(MAX_FORMAT_LENGTH);
            if (pBuf != nullptr) {
                vsnprintf(pBuf, MAX_FORMAT_LENGTH, format, ap);
                strResult = pBuf;
                free(pBuf);
            }
            va_end(ap);
        }
        return strResult;
    }

    /**
	 *  数据缓存路径
	 *
	 *  @return 路径地址
	 */
    static std::string getDataPath()
    {
        std::string path;
        if (cocos2d::Application::getInstance()->getTargetPlatform() == cocos2d::Application::Platform::OS_ANDROID) {
            path = "/sdcard/xZone/";
        }
        else
            path = cocos2d::FileUtils::getInstance()->getWritablePath() + "/xZone/";
        return lsStandardPath(path);
    }
	/**
	*  返回本地文件大小
	*
	*  @return 本地文件大小
	*/
	static long getLocalFileLength(std::string path)
	{
		if (!cocos2d::FileUtils::getInstance()->isFileExist(path))
			return 0;
		FILE *fp = fopen(path.c_str(), "r");
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fclose(fp);

		return length;
	}
     
	/**

	*  返回下载百分比
	*
	*  @return 百分比
	*/
	static float getDownloadPercent(std::string size, std::string &package)
	{
		//apk;data
		std::vector<std::string> svc = parStringByChar(size);
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		if (!data) return 0;

		std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
		std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

		long localApkSize = getLocalFileLength(apkPath);

		if (svc.size() > 1){//存在数据包
			long localDataSize = getLocalFileLength(dataPath);
			return (float)(localApkSize+localDataSize) / ( atol(svc[0].c_str()) + atol(svc[1].c_str()) ) * 100.0f;
		}
		return (float)((float)localApkSize / (float)atol(svc[0].c_str())) * 100.0f;
	}

	/**
	*  安装APK
	*
	*  
	*/
	static void installAPK(std::string package)
	{
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		if (!data) return;
		//apk;data
		std::vector<std::string> svc = parStringByChar(data->_size);

		std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
		std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

		if (svc.size() > 1){//存在数据包
			PlatformHelper::installApkAndData(apkPath, dataPath);
			return;
		}
		PlatformHelper::installApp(apkPath);
	}

	/**
	*  删除APK及数据包
	*
	*
	*/
	static void deleteApkAndData(std::string package)
	{
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		if (!data) return;
		//apk;data
		std::vector<std::string> svc = parStringByChar(data->_size);

		std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
		std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

		if (svc.size() > 1){//存在数据包
			cocos2d::FileUtils::getInstance()->removeFile(dataPath);
		}
		cocos2d::FileUtils::getInstance()->removeFile(apkPath);
	}

	/**
	 *  解析字符串数据
	 *
	 *  @param str   目标字符串
	 *  @param ch	字符串分隔标志
	 */

	static std::vector<std::string> parStringByChar(std::string &str, char ch = ';')
	{
		std::vector<std::string> des;
		des.clear();
		int basePos = 0;
		int found = str.find(";", basePos);
		if (found == std::string::npos)
			des.push_back(str);
		while (found != std::string::npos)
		{
			std::string t = std::string(str.substr(basePos, found - basePos));
			size_t startPos = t.find_last_of(".");
			
			if (startPos != std::string::npos){
				std::string strExt = &t[startPos];
				if (strExt == ".png")
					t = t.substr(0, startPos) + ".jpg";
			}
			
			des.push_back(t);
			basePos = found + 1;
			found = str.find(ch, basePos);
		}
		if (basePos < (int)str.size() - 1)
			des.push_back(str.substr(basePos, str.size() - basePos));

		return des;
	}

    /**
	 *  分割字符串
	 *
	 *  @param str   目标字符串
	 *  @param width	显示宽度
	 */
    static std::vector<std::string> getSubString(std::string& str, double width)
    {
        std::vector<std::string> res;
        res.clear();

        auto tmpTxt = cocos2d::Label::createWithTTF(str, s_labelTTF, 24); //cocos2d::ui::Text::create(str, "msyh.ttf", 24);
        size_t stringLength = cocos2d::StringUtils::getCharacterCountInUTF8String(str);
        double textLong = tmpTxt->getContentSize().width;

        double overPer = width / textLong;
        int leftLength = stringLength * overPer;
        res.push_back(cocos2d::ui::Helper::getSubStringOfUTF8String(str, 0, leftLength));
        res.push_back(cocos2d::ui::Helper::getSubStringOfUTF8String(str, leftLength, stringLength - leftLength));

        return res;
    }

    /**
	*  去除尾部的0
	*
	*  @param str   目标字符串
	*  @param ch	去除的字符
	*/
    static std::string removeAllLeftZero(std::string str, char ch)
    {
        std::string res = str;
        std::size_t p = str.find_last_not_of(ch);
        if (p == std::string::npos)
            p = 0;
        else if (p > 0 && str[p] == '.')
            p = p;
        else
            p = p + 1;
        res = str.substr(0, p);

        return res;
    }
    /**
	 *  按钮缩放动画
	 *
	 *  @return 动画实例
	 */
    static cocos2d::Action* simpleButtonAction()
    {
        auto scale1 = cocos2d::ScaleTo::create(1.5f, 1.2f, 1.0f);
        auto scale2 = cocos2d::ScaleTo::create(1.5f, 1.0f, 1.2f);

        return cocos2d::RepeatForever::create(cocos2d::Sequence::createWithTwoActions(scale1, scale2));
    }

    /**
	 *  旋转720度入场动画
	 *
	 *  @param target   目标
	 *  @param duration 耗时
	 */
    static void rotoZoomAction(cocos2d::Node* target, float duration, cocos2d::CallFunc* callback)
    {
        target->setScale(0.001f);
        auto rotozoom = cocos2d::Sequence::create(cocos2d::Spawn::create(cocos2d::ScaleTo::create(duration, 1.0f),
                                                      cocos2d::RotateBy::create(duration, 360 * 2),
                                                      nullptr),
            cocos2d::DelayTime::create(duration), nullptr);
        if (callback)
            target->runAction(cocos2d::Sequence::createWithTwoActions(rotozoom, callback));
        else
            target->runAction(rotozoom);
    }

    /**
	 *  缩放入场动画
	 *
	 *  @param target   目标
	 *  @param duration 耗时
	 */
    static void scaleZoomAction(cocos2d::Node* target, float duration, cocos2d::CallFunc* callback)
    {
        target->setScale(3.0f);
        auto scaleTo = cocos2d::ScaleTo::create(duration, 1.0f);
        auto ease = cocos2d::EaseInOut::create(scaleTo, 2.5f);

        /*auto action1 = cocos2d::ScaleTo::create(duration / 6, 0.8f);
		auto action2 = cocos2d::ScaleTo::create(duration / 3, 1.1f);
		auto action3 = cocos2d::ScaleTo::create(duration / 2, 1.0f);*/

        if (callback)
            target->runAction(cocos2d::Sequence::create(ease, callback, nullptr));
        else
            target->runAction(cocos2d::Sequence::create(ease, nullptr));
    }

    /**
	 *  生成帧动画
	 *
	 *  @param filename               每帧图片的文件名格式（目前i从0开始）
	 *  @param frameCount             总帧数
	 *  @param delay                  每帧间隔
	 *  @param isRestoreOriginalFrame 结束是否回到开始帧
	 *
	 *  @return animate
	 */
    static cocos2d::Animate* simpleAnimation(const std::string& filename,
        int frameCount, float delay,
        bool isRestoreOriginalFrame = true)
    {
        auto animation = cocos2d::Animation::create();
        for (int i = 0; i < frameCount; i++) {
            char szName[100] = { 0 };
            sprintf(szName, filename.c_str(), i);
            animation->addSpriteFrameWithFile(szName);
        }
        animation->setDelayPerUnit(delay);
        animation->setRestoreOriginalFrame(isRestoreOriginalFrame);
        animation->setLoops(100);
        return cocos2d::Animate::create(animation);
    }

    /**
	 *  解压压缩包，读取里面的图片纹理（只能在win32下使用）
	 *  FileUtils::getInstance()->getFileDataFromZip(）返回空。
	 *
	 *  @param zipFilePath 压缩文件路径
	 *  @param filename    图片的文件名
	 *
	 *  @return 2d纹理
	 */
    static cocos2d::Texture2D* getTextureFromZip(const std::string& zipFilePath, const std::string& filename)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        ssize_t bufferSize;
        std::string path = cocos2d::FileUtils::getInstance()->fullPathForFilename(zipFilePath);
        unsigned char* buffer = cocos2d::FileUtils::getInstance()->getFileDataFromZip(
            zipFilePath,
            filename,
            &bufferSize);
        cocos2d::Image* image = new cocos2d::Image();
        image->initWithImageData(buffer, bufferSize);

        cocos2d::Texture2D* texture = new cocos2d::Texture2D();
        texture->initWithImage(image);
        image->release();

        return texture;
#else
        return NULL;
#endif
    }

    /**
	 *  解压压缩包
	 *
	 *  @param zipFilePath 压缩文件路径
	 *  @return 是否成功
	 */
    static bool unZip(const std::string& zipFilePath, const std::string& file, const std::string& savePath)
    {
        std::string zipFileName = lsStandardPath(zipFilePath + file);
        unzFile zipfile = cocos2d::unzOpen(zipFileName.c_str());
        if (!zipfile) {
            LS_LOG("can not open zip file %s", zipFileName.c_str());
            return false;
        }
        // Get info about the zip file
        cocos2d::unz_global_info global_info;
        if (cocos2d::unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
            LS_LOG("can not read file global info of %s", zipFileName.c_str());
            cocos2d::unzClose(zipfile);
            return false;
        }
        // Buffer to hold data read from the zip file
        char readBuffer[MY_BUFFER_SIZE];
        LS_LOG("start uncompressing");
        // Loop to extract all files.
        uLong i;
        for (i = 0; i < global_info.number_entry; ++i) {
            // Get info about current file.
            cocos2d::unz_file_info fileInfo;
            char fileName[MY_MAX_FILENAME];
            if (cocos2d::unzGetCurrentFileInfo(zipfile, &fileInfo, fileName,
                    MY_MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
                LS_LOG("can not read file info");
                cocos2d::unzClose(zipfile);
                return false;
            }
            std::string fullPath;
            /*if (savePath.empty())
			 fullPath = zipFilePath + fileName;
			 else*/
            fullPath = savePath + fileName;

            // Check if this entry is a directory or a file.
            const size_t filenameLength = strlen(fileName);
            if (fileName[filenameLength - 1] == '/') {
                // Entry is a direcotry, so create it.
                // If the directory exists, it will failed scilently.
                if (!cocos2d::FileUtils::getInstance()->createDirectory(fullPath.c_str())) {
                    LS_LOG("can not create directory %s", fullPath.c_str());
                    cocos2d::unzClose(zipfile);
                    return false;
                }
            }
            else {
                //There are not directory entry in some case.
                //So we need to test whether the file directory exists when uncompressing file entry
                //, if does not exist then create directory
                const std::string fileNameStr(fileName);
                size_t startIndex = 0;
                size_t index = fileNameStr.find("/", startIndex);
                while (index != std::string::npos) {
                    //const std::string dir = _storagePath + fileNameStr.substr(0, index);
                    const std::string dir = lsStandardPath(zipFilePath + fileNameStr.substr(0, index));
                    FILE* out = fopen(dir.c_str(), "r");
                    if (!out) {
                        if (!cocos2d::FileUtils::getInstance()->createDirectory(dir.c_str())) {
                            LS_LOG("can not create directory %s", dir.c_str());
                            cocos2d::unzClose(zipfile);
                            return false;
                        }
                        else
                            LS_LOG("create directory %s", dir.c_str());
                    }
                    else
                        fclose(out);
                    startIndex = index + 1;
                    index = fileNameStr.find("/", startIndex);
                }
                // Entry is a file, so extract it.
                // Open current file.
                if (cocos2d::unzOpenCurrentFile(zipfile) != UNZ_OK) {
                    LS_LOG("can not open file %s", fileName);
                    cocos2d::unzClose(zipfile);
                    return false;
                }
                // Create a file to store current file.
                FILE* out = fopen(fullPath.c_str(), "wb");
                if (!out) {
                    LS_LOG("can not open destination file %s", fullPath.c_str());
                    cocos2d::unzCloseCurrentFile(zipfile);
                    cocos2d::unzClose(zipfile);
                    return false;
                }
                // Write current file content to destinate file.
                int error = UNZ_OK;
                do {
                    error = cocos2d::unzReadCurrentFile(zipfile, readBuffer, MY_BUFFER_SIZE);
                    if (error < 0) {
                        LS_LOG("can not read zip file %s, error code is %d", fileName, error);
                        cocos2d::unzCloseCurrentFile(zipfile);
                        cocos2d::unzClose(zipfile);
                        return false;
                    }

                    if (error > 0)
                        fwrite(readBuffer, error, 1, out);
                } while (error > 0);

                fclose(out);
            }
            cocos2d::unzCloseCurrentFile(zipfile);
            // Goto next entry listed in the zip file.
            if ((i + 1) < global_info.number_entry) {
                if (cocos2d::unzGoToNextFile(zipfile) != UNZ_OK) {
                    LS_LOG("can not read next file");
                    cocos2d::unzClose(zipfile);
                    return false;
                }
            }
        }
        LS_LOG("end uncompressing");
        cocos2d::unzClose(zipfile);
        return true;
    }

    /**
	 *  保存Json文件
	 *
	 *  @param fileName 文件名
	 *  @param doc      Json对象
	 *
	 *  @return 是否保存成功
	 */
    static bool saveJsonFile(const char* fileName, rapidjson::Document& doc)
    {
        bool bRet = false;
        do {
            //LS_LOG("file path : %s", fileName);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            doc.Accept(writer);
            std::string str = buffer.GetString();

            FILE* fp = std::fopen(lsStandardPath(fileName).c_str(), "wb");
            CCASSERT(fp != nullptr, "file open error");
            //fwrite(str.c_str(), str.length(), 1, fp);
            fputs(str.c_str(), fp);
            fclose(fp);
            bRet = true;
        } while (0);
        return bRet;
    }

    /**
	 *  删除Json数组的某一个对象
	 *
	 *  @param index 第几个
	 *  @param value Json对象
	 *
	 *  @return 是否读取成功
	 */
    static bool removeJsonArrayItem(int index, rapidjson::Value& value)
    {
        bool bRet = false;
        int size = value.Size();
        if (index < size) {
            for (unsigned int i = index; i + 1 < value.Size(); i++)
                value[i] = value[i + 1];
            value.PopBack();
            bRet = true;
        }
        return bRet;
    }

    /**
	 *  将char*转成Json对象
	 *
	 *  @param str 要转换的字符
	 *  @param doc 转换后的Json对象
	 *
	 *  @return 是否转换成功
	 */
    static bool readJsonWithString(const char* str, rapidjson::Document& doc)
    {
        bool bRet = false;
        do {
            std::string content(str);
            replaceString(content, ":null", ":\"\"");
            doc.Parse<0>(content.c_str());
            CCASSERT(!doc.HasParseError(), "HasParseError");
            bRet = true;
        } while (0);
        return bRet;
    }

    /**
	 *  读取Json文件
	 *
	 *  @param fileName 文件名
	 *  @param doc      转换后的Json对象
	 *
	 *  @return 是否读取成功
	 */
    static bool readJsonWithFile(const char* fileName, rapidjson::Document& doc)
    {
        bool bRet = false;
        if (cocos2d::FileUtils::getInstance()->isFileExist(fileName)) {
            std::string contentStr = cocos2d::FileUtils::getInstance()->getStringFromFile(fileName);
            //LS_LOG("%s connent:%s", fileName, contentStr.c_str());
            bRet = readJsonWithString(contentStr.c_str(), doc);
        }

        return bRet;
    }

    /**
	 *  将json的Value转成String
	 *
	 *  @param node   Json格式的Value
	 *  @param strret 转换后的String
	 *
	 *  @return 0表示成功，-1表示失败
	 */
    static int valueToString(const rapidjson::Value& node, std::string& strret)
    {
        strret.clear();
        char tmp[64] = { 0 };
        if (node.IsString()) {
            strret = node.GetString();
            return 0;
        }
        else if (node.IsDouble()) {
            //sprintf_s(tmp,63, "%.2lf", node.GetDouble());
            sprintf(tmp, "%.2lf", node.GetDouble());
            strret = tmp;
            return 0;
        }
        else if (node.IsNumber()) {
            sprintf(tmp, "%.0lf", node.GetDouble());
            strret = tmp;
            return 0;
        }
        else if (node.IsFalse()) {
            strret = "false";
            return 0;
        }
        else if (node.IsTrue()) {
            strret = "true";
            return 0;
        }
        return -1;
    }
    /**
	 *  查找Json数据中的值
	 *
	 *  @param node  Json格式的Value
	 *  @param key	 关键字
	 *  @param value 值
	 *
	 *  @return -1表示失败，其他情况表示数组中第几个
	 */
    static int getIndexWithJsonArray(rapidjson::Value& node, const char* key, const char* value)
    {
        int index = -1;
        if (node.IsArray()) {
            for (unsigned int i = 0; i < node.Size(); i++) {
                //const char* temp = node[i][key].GetString();
                //LS_LOG("%s%s", value, temp);
                if (strcmp(value, node[i][key].GetString()) == 0) {
                    index = i;
                    break;
                }
            }
        }
        CCASSERT(index != -1, "not find value in array");
        return index;
    }

    static void sortJsonArray(rapidjson::Value& node, const char* key, bool isAsc = true)
    {
        CCASSERT(node.IsArray(), "is not array");
        rapidjson::Value temp;
        int dir = isAsc ? 1 : -1;
        for (unsigned int j = 0; j < node.Size(); j++)
            for (unsigned int i = 0; i < node.Size() - 1; i++) {
                CCASSERT(node[i].HasMember(key), "not member");
                CCASSERT(node[i][key].IsInt(), "not int");
                if (dir * node[i][key].GetInt() > dir * node[i + 1][key].GetInt()) {
                    temp = node[i];
                    node[i] = node[i + 1];
                    node[i + 1] = temp;
                }
            }
    }

    /**
	 *  播放音效
	 *
	 *  @param type 音效类型
	 */
    static void playEffect(EffectType type)
    {

        if (!DataManager::getInstance()->_sUserData.soundState)
            return;

        switch (type) {
        case EffectType::BUTTON:
            SAE->playEffect("cell_switch1.wav");
            break;
        case EffectType::NONE:
            SAE->stopAllEffects();
            break;
        default:
            break;
        }
    }

    /**
	 *  播放背景音乐
	 *
	 *  @param type 音乐类型
	 */
    static void playMusic(MusicType type)
    {
        //    if (!_sUserData.soundState)
        //        return;
        //    //TODO : play effect
        //        switch (type) {
        //            case EffectType::BUTTON:
        //                SAE->playEffect("effectBallDead.mp3");
        //                break;
        //            default:
        //                break;
        //        }
    }

    /**
	 *  停止所有的音乐
	 */
    static void stopAllMusic()
    {
        SAE->stopBackgroundMusic();
    }

    /**
	 *  停止所有的音效
	 */
    static void stopAllEffect()
    {
        SAE->stopAllEffects();
    }

    /**
	 *  重置随机种子
	 */
    static void resetRand()
    {
        timeval psv;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        cocos2d::gettimeofday(&psv, NULL); //3.0获取本地时间
#else
        gettimeofday(&psv, NULL); //3.0获取本地时间
#endif
        //gettimeofdayCocos2d( &psv, NULL );//2.0获取本地时间
        //根据时间产生随机种子
        unsigned int tsrans = (unsigned int)(psv.tv_sec * 1000 + psv.tv_usec / 1000);
        srand(tsrans); //设定随机数种子
    }

    /**
	 *  获取系统时间
	 */
    static const char* getSysTime()
    {
        struct tm* tm;
        const time_t timep = time(NULL);

        tm = localtime(&timep);
        int year = tm->tm_year + 1900;
        int month = tm->tm_mon + 1;
        int day = tm->tm_mday;
        int hour = tm->tm_hour;
        int min = tm->tm_min;
        int sec = tm->tm_sec;

        return formatStr("%d/%02d/%02d %02d:%02d:%02d", year, month, day, hour, min, sec).c_str();
    }

    /**
	 *  获取系统时间
	 */
    static MyTime getMyTime()
    {
        struct tm* tm;
        const time_t timep = time(NULL);

        MyTime time;
        tm = localtime(&timep);
        time.year = tm->tm_year + 1900;
        time.month = tm->tm_mon + 1;
        time.day = tm->tm_mday;
        time.hour = tm->tm_hour;
        time.min = tm->tm_min;
        time.sec = tm->tm_sec;
        time.wday = tm->tm_wday;

        if (time.wday == 0)
            time.wday = 7;

        return time;
    }

    /**
	 *  获取KeyCode对应的键名
	 *
	 *  @param keyCode 键值
	 *
	 *  @return 键名
	 */
    static const char* keyCode2Char(cocos2d::EventKeyboard::KeyCode keyCode)
    {
        return s_KeyCodeStr[static_cast<int>(keyCode)];
    }

    /**
	 *  获取手柄按钮对应的键名
	 *
	 *  @param key 键值
	 *
	 *  @return 键名
	 */
    static const char* jostickCode2Char(int key)
    {
        return s_JoystickStr[key - 1000];
    }

    /**
	 *  获取分页控件下一页的焦点控件
	 *
	 *  @param pageView 分页控件
	 *  @param keyCode  按键
	 *  @return 下一个焦点控件
	 */
    static cocos2d::Node* getNextPageFocus(UIPageViewEx* pageView, cocos2d::EventKeyboard::KeyCode keyCode)
    {
        cocos2d::Node* curFocus = nullptr;
        if (pageView)
            curFocus = static_cast<cocos2d::Node*>(pageView->getUserObject());
        if (!curFocus)
            return curFocus;

        switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT: */ {
                long index = pageView->getCurPageIndex();
                index++;
                if (index >= pageView->getPages().size())
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 1);
            }
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT: */ {
                ssize_t index = pageView->getCurPageIndex();
                index--;
                if (index < 0)
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 0);
            }
            break;
        default:
            break;
        }

        return curFocus;
    }

    static cocos2d::Node* getNextPageFocus2(DLGameList* pageView, cocos2d::EventKeyboard::KeyCode keyCode)
    {
        cocos2d::Node* curFocus = nullptr;
        if (pageView)
            curFocus = static_cast<cocos2d::Node*>(pageView->getUserObject());
        if (!curFocus)
            return curFocus;

        switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT: */ {
                long index = pageView->getCurPageIndex();
                index++;
                if (index >= pageView->getPages().size())
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 1);
            }
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT: */ {
                long index = pageView->getCurPageIndex();
                index--;
                if (index < 0)
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 0);
            }
            break;
        default:
            break;
        }

        return curFocus;
    }

    /**
	 *  获取分页控件下一页的焦点控件
	 *
	 *  @param pageView 分页控件
	 *  @param curFocus 当前焦点
	 *  @param keyCode  按键
	 *  @return 下一个焦点控件
	 */
    static cocos2d::Node* getNextPageFocus(UIPageViewEx* pageView, cocos2d::Node* curFocus, cocos2d::EventKeyboard::KeyCode keyCode)
    {
        switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT: */ {
                long index = pageView->getCurPageIndex();
                index++;
                if (index >= pageView->getPages().size())
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 1);
            }
            break;
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP:
            /*case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT: */ {
                long index = pageView->getCurPageIndex();
                index--;
                if (index < 0)
                    break;
                pageView->scrollToPage(index);
                return pageView->getNextPageFocus(curFocus, 0);
            }
            break;
        default:
            break;
        }

        return curFocus;
    }
    /**
	 *  获取此方向上的下一个焦点
	 *  新焦点的缩放变色、选中框、以及其他操作要另外处理
	 *
	 *  @param dir       方向
	 *  @param tableNode 列表父节点
	 *
	 *  @return 返回新的聚焦节点
	 */
    static cocos2d::Node* getNextFocus(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Node* tableNode)
    {
        if (!tableNode)
            return nullptr;
        //LS_LOG("dir : %f, %f", dir.x, dir.y);
        cocos2d::Node* temp = nullptr;
        cocos2d::Node* focus = dynamic_cast<cocos2d::Node*>(tableNode->getUserObject());
        Vec2Dir dir = dirKeyCode2Vec2(keyCode);
        //遍历所有的子节点，找到下一个焦点
        for (auto& node : tableNode->getChildren()) {

            //排除倒影
            std::string::size_type idx = node->getName().find("dy_");
            if (idx != std::string::npos || node->getName() == "_select")
                continue;

            //当前焦点为空时，取左上角第一个
            if (!focus) {
                temp = getCornerFocus(CornerType::TOP_LEFT, tableNode);
                continue;
            }
            else {
                //节点是当前焦点，或者不在当前焦点的移动轨迹上，查找下一个
                if (node == focus || (dir.x && dir.x * (node->getPositionX() - focus->getPositionX()) <= 0)
                    || (dir.y && dir.y * (node->getPositionY() - focus->getPositionY()) <= 0)) {
                    continue;
                }

                auto rect1 = cocos2d::Rect(fabsf(dir.y) * (focus->getPositionX() - focus->getContentSize().width / 2.0f),
                    fabsf(dir.x) * (focus->getPositionY() - focus->getContentSize().height / 2.0f),
                    focus->getContentSize().width,
                    focus->getContentSize().height);
                auto rect2 = cocos2d::Rect(fabsf(dir.y) * (node->getPositionX() - node->getContentSize().width / 2.0f),
                    fabsf(dir.x) * (node->getPositionY() - node->getContentSize().height / 2.0f),
                    node->getContentSize().width, node->getContentSize().height);
                if (!rect1.intersectsRect(rect2))
                    continue;

                if (!temp) {
                    temp = node;
                    continue;
                }

                //算出最短距离差
                float ds = dir.x * (temp->getPositionX() - dir.x * temp->getContentSize().width / 2.0f)
                    - dir.x * (focus->getPositionX() + dir.x * focus->getContentSize().width / 2.0f)
                    + dir.y * (temp->getPositionY() - dir.y * temp->getContentSize().height / 2.0f)
                    - dir.y * (focus->getPositionY() + dir.y * focus->getContentSize().height / 2.0f);
                float dsNew = dir.x * (node->getPositionX() - dir.x * node->getContentSize().width / 2.0f)
                    - dir.x * (focus->getPositionX() + dir.x * focus->getContentSize().width / 2.0f)
                    + dir.y * (node->getPositionY() - dir.y * node->getContentSize().height / 2.0f)
                    - dir.y * (focus->getPositionY() + dir.y * focus->getContentSize().height / 2.0f);

                if (dsNew < ds - MIN_FOCUS_DS
                    || (fabsf(ds - dsNew) < MIN_FOCUS_DS
                           && ((dir.x && fabsf(dir.x) * (node->getPositionY() - temp->getPositionY()) > 0)
                                  || (dir.y && fabsf(dir.y) * (node->getPositionX() - temp->getPositionX()) < 0))))
                    temp = node;
            }
        }

        //没有下一个焦点
        if (!temp || temp == focus) {
            LS_LOG("end");
            return nullptr;
        }

        if (focus) {
            focus->setColor(cocos2d::Color3B::WHITE);
            focus->setScale(1.0f);
        }
        focus = temp;

        tableNode->setUserObject(focus);
        return focus;
    }

    /**
	 *  test 获取角落焦点
	 *
	 *  @param CornerType	上下左右四个角
	 *  @param tableNode	列表父节点
	 *
	 *  @return 获取到的焦点
	 */
    static cocos2d::Node* getCornerFocus(CornerType type, cocos2d::Node* tableNode)
    {
        cocos2d::Node* temp = nullptr;
        auto screenSize = cocos2d::Director::getInstance()->getVisibleSize();
        cocos2d::Vec2 targetPos;
        switch (type) {
        case CornerType::NOON:
            break;
        case CornerType::TOP_LEFT:
            targetPos = cocos2d::Vec2(0, screenSize.height);
            break;
        case CornerType::TOP_RIGHT:
            targetPos = cocos2d::Vec2(screenSize.width, screenSize.height);
            break;
        case CornerType::BOTTOM_LEFT:
            targetPos = cocos2d::Vec2(0, 0);
            break;
        case CornerType::BOTTOM_RIGHT:
            targetPos = cocos2d::Vec2(0, 0);
            break;
        default:
            break;
        }

        for (auto& node : tableNode->getChildren())
            if (!temp || targetPos.distance(temp->getPosition()) > targetPos.distance(node->getPosition()))
                temp = node;

        tableNode->setUserObject(temp);
        return temp;
    }

    /**
	 *  将KeyCode的方向，转成向量
	 *
	 *  @param keyCode 方向键值
	 *
	 *  @return 方向向量
	 */
    static Vec2Dir dirKeyCode2Vec2(cocos2d::EventKeyboard::KeyCode keyCode)
    {
        Vec2Dir vec;
        switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_LEFT:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT:
        case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
            vec = VEC2_LEFT;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_RIGHT:
        case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
            vec = VEC2_RIGTH;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_UP:
        case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP: {
            vec = VEC2_UP;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_DOWN:
        case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN: {
            vec = VEC2_DOWN;
        } break;
        default: {
            vec = VEC2_CENTER;
        } break;
        }
        return vec;
    }

    /**
	 *  将KeyCode的方向，转成枚举类型
	 *
	 *  @param keyCode 方向键值
	 *
	 *  @return 枚举类型的值
	 */
    static Vec2DirEnum dirKeyCode2Vec2Enum(cocos2d::EventKeyboard::KeyCode keyCode)
    {
        Vec2DirEnum vec = Vec2DirEnum::EDIR_CENTER;
        switch (keyCode) {
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_LEFT:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT:
        case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW: {
            vec = Vec2DirEnum::EDIR_LEFT;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_RIGHT:
        case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
            vec = Vec2DirEnum::EDIR_RIGHT;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_UP:
        case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP: {
            vec = Vec2DirEnum::EDIR_UP;
        } break;
        case cocos2d::EventKeyboard::KeyCode::KEY_KP_DOWN:
        case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
        case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN: {
            vec = Vec2DirEnum::EDIR_DOWN;
        } break;
        default: {
            vec = Vec2DirEnum::EDIR_CENTER;
        } break;
        }
        return vec;
    }

    /**
	 *  根据Tag，递归搜索节点
	 *
	 *  @param root 父节点
	 *  @param tag  <#tag description#>
	 *
	 *  @return 找到的节点
	 */
    static cocos2d::Node* seekNodeByTag(cocos2d::Node* root, int tag)
    {
        if (!root)
            return nullptr;
        if (root->getTag() == tag)
            return root;

        const auto& arrayRootChildren = root->getChildren();
        for (auto& child : arrayRootChildren) {
            cocos2d::Node* res = seekNodeByTag(child, tag);
            if (res != nullptr)
                return res;
        }

        return nullptr;
    }

    /**
	 *  根据名字，递归搜索节点
	 *
	 *  @param root 父节点
	 *  @param name 搜索名
	 *
	 *  @return 找到的节点
	 */
    static cocos2d::Node* seekNodeByName(cocos2d::Node* root, const std::string& name)
    {
        if (!root)
            return nullptr;
        if (root->getName() == name)
            return root;

        const auto& arrayRootChildren = root->getChildren();
        for (auto& subWidget : arrayRootChildren) {
            cocos2d::Node* res = seekNodeByName(subWidget, name);
            if (res != nullptr)
                return res;
        }
        return nullptr;
    }

    /**
	 *  删除文件
	 *
	 *  @param path 路径
	 */
    static void removeFileForPath(const std::string& path)
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        std::string command = "rm -r ";
        command += "\"" + lsStandardPath(path) + "\"";
        system(command.c_str());
#else
        cocos2d::FileUtils::getInstance()->removeFile(lsStandardPath(path));
#endif
    }

    static void copyFileForPath(const std::string& source, const std::string& path)
    {
        CCASSERT(cocos2d::FileUtils::getInstance()->isFileExist(source), "");
        LS_LOG("source: %s \n %s", cocos2d::FileUtils::getInstance()->fullPathForFilename(source).c_str(), path.c_str());
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        PH::copyAssetsFileToPath(source, path);
#else
        std::ifstream in(cocos2d::FileUtils::getInstance()->fullPathForFilename(source).c_str(), std::ios::in | std::ifstream::binary);
        std::ofstream out(lsStandardPath(path), std::ios::out | std::ofstream::binary);
        if (!in || !out) {
            CCASSERT(0, "open file error");
        }
        out << in.rdbuf();
        in.close();
        out.close();
#endif
    }
    static cocos2d::DrawNode* createQRAndDraw(CQR_Encode& QREncode, float size, char* source, cocos2d::Size& contentSize)
    {
        auto ret = createQRAndDraw(QREncode, size, source);
        contentSize.width = (QREncode.m_nSymbleSize + QR_MARGIN * 2) * size;
        contentSize.height = (QREncode.m_nSymbleSize + QR_MARGIN * 2) * size;
        return ret;
    }
    /**
	 *  生成二维码，并绘制
	 *
	 *  @param QREncode 二维码编码数据
	 *  @param size     像素点大小
	 *  @param source   内容
	 *
	 *  @return 绘制的节点
	 */
    static cocos2d::DrawNode* createQRAndDraw(CQR_Encode& QREncode, float size, char* source)
    {
        //CQR_Encode QREncode;
        if (!QREncode.EncodeData(0, 0, 1, -1, source))
            return nullptr;

        auto qrNode = cocos2d::DrawNode::create();
        for (int i = 0; i < QREncode.m_nSymbleSize; ++i) {
            for (int j = 0; j < QREncode.m_nSymbleSize; ++j) {
                if (QREncode.m_byModuleData[i][j])
                    qrNode->drawSolidRect(cocos2d::Vec2((i + QR_MARGIN) * size, (j + QR_MARGIN) * size),
                        cocos2d::Vec2(((i + QR_MARGIN) + 1) * size, ((j + QR_MARGIN) + 1) * size), cocos2d::Color4F(0, 0, 0, 1));
                else
                    qrNode->drawSolidRect(cocos2d::Vec2((i + QR_MARGIN) * size, (j + QR_MARGIN) * size),
                        cocos2d::Vec2(((i + QR_MARGIN) + 1) * size, ((j + QR_MARGIN) + 1) * size), cocos2d::Color4F(1, 1, 1, 1));
            }
        }

        //绘制外框
        qrNode->drawSolidRect(cocos2d::Vec2(0, 0), cocos2d::Vec2((QREncode.m_nSymbleSize + QR_MARGIN * 2) * size,
                                                       (QR_MARGIN)*size),
            cocos2d::Color4F(1, 1, 1, 1));
        qrNode->drawSolidRect(cocos2d::Vec2(0, 0), cocos2d::Vec2((QR_MARGIN)*size,
                                                       (QREncode.m_nSymbleSize + QR_MARGIN * 2) * size),
            cocos2d::Color4F(1, 1, 1, 1));
        qrNode->drawSolidRect(cocos2d::Vec2((QREncode.m_nSymbleSize + QR_MARGIN) * size, 0),
            cocos2d::Vec2((QREncode.m_nSymbleSize + QR_MARGIN * 2) * size,
                                  (QREncode.m_nSymbleSize + QR_MARGIN * 2) * size),
            cocos2d::Color4F(1, 1, 1, 1));
        qrNode->drawSolidRect(cocos2d::Vec2(0, (QREncode.m_nSymbleSize + QR_MARGIN) * size),
            cocos2d::Vec2((QREncode.m_nSymbleSize + QR_MARGIN * 2) * size,
                                  (QREncode.m_nSymbleSize + QR_MARGIN * 2) * size),
            cocos2d::Color4F(1, 1, 1, 1));

        /*qrNode->setPosition(cocos2d::Vec2((SCREEN.width - size * QREncode.m_nSymbleSize) / 2,
		 SCREEN.height - (SCREEN.height - size * QREncode.m_nSymbleSize) / 2));*/
        qrNode->setScaleY(-1);
        return qrNode;
    }
    /**
	 *  string 转 char*
	 *
	 *  @param str 内容
	 */
    static char* str2charp(std::string& str)
    {
        char* pstr = new char[str.length() + 1];
        strcpy(pstr, str.c_str());
        return pstr;
    }

    static cocos2d::Action* delayAndCallFunc(float dt, const std::function<void()>& func)
    {
        auto delay = cocos2d::DelayTime::create(dt);
        auto callback = cocos2d::CallFunc::create(func);
        return cocos2d::Sequence::createWithTwoActions(delay, callback);
    }

    static cocos2d::Action* delayAndCallFunc(const std::function<void()>& func)
    {
        return delayAndCallFunc(2.0f, func);
    }

    /**
	*  计算节点以及子节点的大小
	*
	*  @param node 计算的节点
	* 
	*/
    static cocos2d::Rect calNodeContentSize(cocos2d::Node* node)
    {
        if (node->getChildrenCount() == 0)
            return node->getBoundingBox();
        else {
            cocos2d::Rect rect = cocos2d::Rect::ZERO;
            for (auto& child : node->getChildren()) {
                cocos2d::Rect childRect = calNodeContentSize(child);
                rect.merge(childRect);
            }
            return RectApplyAffineTransform(rect, node->getNodeToParentAffineTransform());
        }
    }

    /**
	*  
	* 测试硬件是否支持GL_STENCIL24_DEPTH8
	*/
    static bool isValidFor_Stencil24Depth8()
    {
        static bool isFirst = true;
        static bool isValid = false;
        if (!isFirst)
            return isValid;

        //check
        GLuint fbo, rb;
        GLint oldFbo, oldRBO;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFbo);
        glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRBO);
        //new
        int w = 1, h = 1;
        auto texture = new (std::nothrow) cocos2d::Texture2D();
        texture->initWithData(0, 4, cocos2d::Texture2D::PixelFormat::RGBA8888, w, h, cocos2d::Size((float)w, (float)h));
        CHECK_GL_ERROR_DEBUG(); // clean possible GL error
        // generate FBO
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        CHECK_GL_ERROR_DEBUG(); // clean possible GL error
        // associate texture with FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getName(), 0);
        CHECK_GL_ERROR_DEBUG(); // clean possible GL error
        //create and attach depth buffer
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
        // if depth format is the one with stencil part, bind same render buffer as stencil attachment
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb);
        CHECK_GL_ERROR_DEBUG(); // clean possible GL error
        // check if it worked (probably worth doing :) )
        isValid = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        //detete texture, fbo, renderbuffer
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rb);
        texture->release();
        glBindRenderbuffer(GL_RENDERBUFFER, oldRBO);
        glBindFramebuffer(GL_FRAMEBUFFER, oldFbo);
        CHECK_GL_ERROR_DEBUG(); // clean possible GL error
        isFirst = false;

        return isValid;
    }
    /**
	*  节点保存成PNG本地图片
	*
	*  @param node 保存的节点
	*  @param fileName 保存的图片名称
	*  @param isRecursion 是否递归计算子节点的包围盒
	*/
    static cocos2d::RenderTexture* nodeToSavePng(cocos2d::Node* node, std::string fileName, bool isRecursion = true)
    {
        if (!node)
            return nullptr;

        //1. 保存当前节点的位置和锚点
        auto nowPos = node->getPosition();
        auto nowAnchor = node->getAnchorPoint();
        //2. 设置当前节点的位置为0，并设置锚点为0
        cocos2d::Rect rect = node->getBoundingBox();
        node->setPosition(cocos2d::Vec2(nowAnchor.x * rect.size.width, nowAnchor.y * rect.size.height));

        rect = node->getBoundingBox();
        //3. 递归计算node节点的包围盒大小
        {
            if (isRecursion)
                rect = cocos2d::utils::getCascadeBoundingBox(node);
        }
        float w = rect.size.width;
        float h = rect.size.height;
        cocos2d::Vec2 tempPos = node->getPosition() + cocos2d::Vec2(-rect.origin.x, -rect.origin.y);
        node->setPosition(tempPos);
        //4. 生成RenderTexture，并遍历当前要保存的节点，在G3上不支持GL_DEPTH24_STENCIL8
        auto rt = cocos2d::RenderTexture::create(w, h, cocos2d::Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
        rt->begin();
        node->visit();
        rt->end();
        cocos2d::Director::getInstance()->getRenderer()->render();
        //rt->saveToFile(fileName, cocos2d::Image::Format::PNG, true);
        //rt->newImage(true)->saveToFile(fileName);
        //5. 恢复当前节点的位置和锚点
        node->setPosition(nowPos);

        return rt;
    }

    static std::string lsStandardPath(const char* path)
    {
        std::string pathStr(path);
        return lsStandardPath(pathStr);
    }

    static std::string lsStandardPath(const std::string& path)
    {
        //LS_LOG("normal path: %s", path.c_str());
        std::string ret = path;
        std::string::size_type pos = ret.find("\\\\");
        //step1:所有多个反斜杠转成一个反斜杠
        while (pos != std::string::npos) {
            replaceString(ret, "\\\\", "\\");
            pos = ret.find("\\\\");
        }
        //step2:所有反斜杠转成一个斜杠
        pos = ret.find("\\");
        while (pos != std::string::npos) {
            replaceString(ret, "\\", "/");
            pos = ret.find("\\");
        }
        //step3:所有多个斜杠转成一个斜杠
        pos = ret.find("//");
        while (pos != std::string::npos) {
            replaceString(ret, "//", "/");
            pos = ret.find("//");
        }

        //LS_LOG("standard path: %s",ret.c_str());
        return ret;
    }

    static void replaceString(std::string& str, const std::string& search, const std::string& replace)
    {
        std::string::size_type pos = 0;
        while ((pos = str.find(search, pos)) != std::string::npos) {
            str.replace(pos, search.size(), replace);
            pos++;
        }
        //LS_LOG("replaceString: %s", str.c_str());
    }

    static void mylog(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        _mylog(format, args);
        va_end(args);
    }

    //创建下载临时文件信息
    static void createDownloadInfor(std::string path, int state)
    {
        //创建临时下载信息文件
        rapidjson::Document gameDownloadDoc;
        gameDownloadDoc.SetArray();
        rapidjson::Document::AllocatorType& allocator = gameDownloadDoc.GetAllocator();
        rapidjson::Value object(rapidjson::kObjectType);
        object.AddMember("downloadState", state, allocator); //未下载状态
        object.AddMember("totalAllFileSize", 0, allocator); //下载文件大小
        object.AddMember("localDownloadFileSize", 0, allocator); //本地已下载文件的大小
        object.AddMember("downloadPercent", 0, allocator); //下载进度大小
        object.AddMember("fileName", "", allocator); //下载到本地的文件名
        gameDownloadDoc.PushBack(object, allocator);
        LsTools::saveJsonFile(path.c_str(), gameDownloadDoc);
    }

private:
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
    static const int CCLOG_STRING_TAG = 1;
    static void mySendLogToWindow(const char* log)
    {
        // Send data as a message
        COPYDATASTRUCT myCDS;
        myCDS.dwData = CCLOG_STRING_TAG;
        myCDS.cbData = (DWORD)strlen(log) + 1;
        myCDS.lpData = (PVOID)log;
        if (cocos2d::Director::getInstance()->getOpenGLView()) {
            HWND hwnd = cocos2d::Director::getInstance()->getOpenGLView()->getWin32Window();
            SendMessage(hwnd,
                WM_COPYDATA,
                (WPARAM)(HWND)hwnd,
                (LPARAM)(LPVOID)&myCDS);
        }
    }
#else
    static void mySendLogToWindow(const char* log)
    {
    }
#endif

    static void _mylog(const char* format, va_list args)
    {
        char buf[MAX_FORMAT_LENGTH];

        vsnprintf(buf, MAX_FORMAT_LENGTH - 3, format, args);
        strcat(buf, "\n");

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        __android_log_print(ANDROID_LOG_DEBUG, "cocos2d-x debug info", "%s", buf);

#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_WINRT || CC_TARGET_PLATFORM == CC_PLATFORM_WP8
        WCHAR wszBuf[MAX_FORMAT_LENGTH] = { 0 };
        MultiByteToWideChar(CP_UTF8, 0, buf, -1, wszBuf, sizeof(wszBuf));
        OutputDebugStringW(wszBuf);
        WideCharToMultiByte(CP_ACP, 0, wszBuf, -1, buf, sizeof(buf), nullptr, FALSE);
        printf("%s", buf);
        mySendLogToWindow(buf);
        fflush(stdout);
#else
        // Linux, Mac, iOS, etc
        fprintf(stdout, "%s", buf);
        fflush(stdout);
#endif

        cocos2d::Director::getInstance()->getConsole()->log(buf);
    }
};

#endif /* defined(__xZone__LsTools__) */

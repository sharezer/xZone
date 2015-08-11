//
//  NetManager.cpp
//  xZone
//
//  Created by Sharezer on 15/3/6.
//
//

#include "NetManager.h"
#include "network/HttpClient.h"
#include "ZoneManager.h"
#include "GameDetail.h"
#include "DataManager.h"

#include <curl/curl.h>
#include <curl/easy.h>

//smn
std::mutex g_mutex;

USING_NS_CC;
USING_NS_NET;

NetManager::NetManager()
	: _isLoadFile(false)
	, _isSendCommend(false)
{
	_commendList.clear();
	_loadList.clear();
}

NetManager::~NetManager()
{
	_commendList.clear();
	_loadList.clear();
}

static NetManager* s_sharedNetManager;

NetManager* NetManager::getInstance()
{
	if (!s_sharedNetManager) {
		s_sharedNetManager = new NetManager();
		CCASSERT(s_sharedNetManager, "FATAL: Not enough memory");
		s_sharedNetManager->init();
	}

	return s_sharedNetManager;
}

void NetManager::destroyInstance()
{
	CC_SAFE_DELETE(s_sharedNetManager);
}

bool NetManager::init()
{
	HttpClient::getInstance()->setTimeoutForConnect(10);
	return true;
}

size_t myLoadPackage(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE *fp = (FILE*)userdata;
	size_t written = fwrite(ptr, size, nmemb, fp);
	return written;
}

int myProgressFunc(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	static int percent = 0;
	int tmp = (int)(nowDownloaded / totalToDownload * 100);

	if (percent != tmp)
	{
		percent = tmp;
		/*
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([tmp]{
		ZM_LOADING->setPercent(tmp);
		});
		*/
	}

	return 0;
}

void NetManager::loadFile(std::string adds, std::string savePath, std::string saveName,
	std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip)
{
	/*if (_isLoadFile)
		return;
		_isLoadFile = true;*/
	//ZM_LOADING->resetPercent();
	SENDLOAD::iterator it = find(_loadList.begin(), _loadList.end(), adds);
	if (it != _loadList.end())
	{
		LS_LOG("loadFile is find:%s", adds.c_str());
		return;
	}
	_loadList.push_back(adds);

	std::thread t1(&NetManager::doLoadFileCURL, this, adds, savePath, saveName, succesCB, failCB, isAutoUnZip);
	t1.detach();
}


void NetManager::doloadFileHttp(std::string adds, std::string savePath, std::string saveName,
	std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip)
{
	auto request = new HttpRequest();

	request->setUrl(adds.c_str());
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback([=](HttpClient* sender, HttpResponse* response) {
		if (!response->isSucceed()) {

			if (failCB)
				LS_SAFE_CALLBACK(failCB);
			else{
				ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
				ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}
			loadEnd(adds);
			return;
		}
		std::vector<char> *data = response->getResponseData();
		int data_length = data->size();
		if (0 != strlen(response->getHttpRequest()->getTag()) && data_length > 0) {
			// dump data
			std::vector<char>* buffer = response->getResponseData();
			std::string bufffff(buffer->begin(), buffer->end());

			//保存到本地文件
			std::string saveFile = savePath + saveName;
			CCLOG("path: %s", saveFile.c_str());
			FILE* fp = fopen(saveFile.c_str(), "wb+");
			fwrite(bufffff.c_str(), 1, buffer->size(), fp);
			fclose(fp);

			if (!isAutoUnZip)
				LS_SAFE_CALLBACK(succesCB);
			else if (LsTools::unZip(savePath, saveName, savePath))
			{
				LsTools::removeFileForPath(saveFile);
				LS_SAFE_CALLBACK(succesCB);
			}
		}
		else{
			if (failCB)
				LS_SAFE_CALLBACK(failCB);
			else{
				ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
				ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}
		}
		loadEnd(adds);
	});

	request->setTag("LoadFile");
	HttpClient::getInstance()->setTimeoutForConnect(3000);
	HttpClient::getInstance()->send(request);
	request->release();
}

void NetManager::doLoadFileCURL(std::string adds, std::string savePath, std::string saveName, std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip)
{
	long length = NetManager::getInstance()->getDownloadFileLength(adds); // 获取远程文件大小
	std::string tempFile = LsTools::lsStandardPath(savePath + "temp" + saveName);
	std::string saveFile = LsTools::lsStandardPath(savePath + saveName);
	LS_LOG("length %d", length);
	if (length < 0)
	{
		LS_SAFE_CALLBACK(failCB);
		LsTools::removeFileForPath(tempFile);
		return;
	}

	auto curl = curl_easy_init();
	if (!curl)
	{
		LS_SAFE_CALLBACK(failCB);
		LsTools::removeFileForPath(tempFile);
		/*if (failCB)
			LS_SAFE_CALLBACK(failCB);
			else{
			ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
			ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}*/
		loadEnd(adds);
		return;
	}
	
	FILE* fp = fopen(tempFile.c_str(), "wb");

	if (!fp)
	{
		LS_SAFE_CALLBACK(failCB);
		LsTools::removeFileForPath(tempFile);
		/*if (failCB)
			LS_SAFE_CALLBACK(failCB);
			else{
			ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
			ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}*/
		//LS_LOG("can not create file %s", saveFile.c_str());
		loadEnd(adds);
		return;
	}

	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, adds.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3000);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, myLoadPackage);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, myProgressFunc);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 5L);
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);

	if (res == CURLE_OK)
	{
		fclose(fp);
		rename(tempFile.c_str(), saveFile.c_str());
		
		if (isAutoUnZip && LsTools::unZip(savePath, saveName, savePath))
			LsTools::removeFileForPath(saveFile);

		LS_SAFE_CALLBACK(succesCB);
		loadEnd(adds);

		/*Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, adds, succesCB]{
			LS_LOG("succeed downloading package %s", adds.c_str());
			NM->_isLoadFile = false;
			if (succesCB)
			succesCB();
			});*/
	}
	else
	{
		fclose(fp);
		LS_SAFE_CALLBACK(failCB);
		LsTools::removeFileForPath(tempFile);
		loadEnd(adds);
	}
}

void NetManager::loadEnd(std::string adds)
{
	_isLoadFile = false;
	SENDLOAD::iterator it = find(_loadList.begin(), _loadList.end(), adds);
	if (it != _loadList.end())
		_loadList.erase(it);

	_loadList.unique();
}

void NetManager::sendCommend(CommendEnum commend, std::string data,
	std::function<void(const rapidjson::Value&)> succesCB, std::function<void()> failCB)
{
	/*if (_isSendCommend)
		return;
		_isSendCommend = true;*/
	std::string url = ZM->getServerAddress() + "?" + formatStr("commend=%d", commend);
	if (!data.empty())
		url += formatStr("%s", data.c_str());
	SENDCOMMEND::iterator it = find(_commendList.begin(), _commendList.end(), url);
	if (it != _commendList.end())
	{
		LS_LOG("Is find:%s", url.c_str());
		return;
	}
	_commendList.push_back(url);

	std::thread t1(&NetManager::doSendCommendHttp, this, commend, data, succesCB, failCB);
	t1.detach();
}

void NetManager::doSendCommendCURL(CommendEnum commend, std::string data,
	std::function<void(const rapidjson::Value&)> succesCB, std::function<void()> failCB)
{

}

void NetManager::doSendCommendHttp(CommendEnum commend, std::string data,
	std::function<void(const rapidjson::Value&)> succesCB, std::function<void()> failCB)
{
	std::string url = ZM->getServerAddress() + "?" + formatStr("commend=%d", commend);
	if (!data.empty())
		url += formatStr("%s", data.c_str());

	LS_LOG("SEND:%s", url.c_str());

	auto request = new HttpRequest();
	request->setUrl(url.c_str());
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback([=](HttpClient* sender, HttpResponse* response) {
		if (!response->isSucceed()) {
			_isSendCommend= false;
			if (failCB)
				LS_SAFE_CALLBACK(failCB);
			else{
				ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
				ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}
			return;
		}
		std::vector<char> *data = response->getResponseData();
		int data_length = data->size();
		LS_LOG("data_length: %d", data_length);
		if (0 != strlen(response->getHttpRequest()->getTag()) && data_length > 0) {
			std::string res;
			for (int i = 0; i < data_length; ++i)
			{
				res += (*data)[i];
			}
			res += '\0';
			//LS_LOG("GET:%s", res.c_str());

			Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, res, commend, succesCB, failCB]{
				rapidjson::Document d;
				LsTools::readJsonWithString(res.c_str(), d);
				if (d.HasMember("state") && d["commend"].GetInt() == static_cast<int>(commend) && d["state"].GetInt() == 1)
				{
					const rapidjson::Value& value = d["data"];
					NetManager::getInstance()->_isSendCommend = false;
					if (succesCB)
						succesCB(value);
				}
				else
					LS_SAFE_CALLBACK(failCB);
			});
		}
		else{
			if (failCB)
				LS_SAFE_CALLBACK(failCB);
			else{
				ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("netError"));
				ZM_DIALOG->setDialogType(DialogType::AUTO_CLOSE);
			}
		}

		SENDCOMMEND::iterator it = find(_commendList.begin(), _commendList.end(), url);
		if (it != _commendList.end())
			_commendList.erase(it);

		_commendList.unique();
		_isSendCommend = false;
	});

	request->setTag("Data");
	HttpClient::getInstance()->setTimeoutForConnect(30);
	HttpClient::getInstance()->send(request);
	request->release();
}

//smn
long NetManager::getDownloadFileLength(std::string url)
{
	double length = -1;
	CURL *handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
	curl_easy_setopt(handle, CURLOPT_HEADER, 0);//  0 不打印日志 1打印日志
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);

	if (curl_easy_perform(handle) == CURLE_OK)
	{
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);

		LS_LOG("filesize : %0.0f bytes", length);
	}
	curl_easy_cleanup(handle);

	return (long)length;
}

int myProgressFuncForGame(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	static int percent = 0;
	int tmp = (int)(nowDownloaded / totalToDownload * 100);

	bool isCancel = false;

	if (percent != tmp)
	{
		percent = tmp;
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([tmp, ptr, totalToDownload, nowDownloaded, &isCancel]{
			GameDetailData *data = static_cast<GameDetailData*>(ptr);
			if (data)
			{
				if (nowDownloaded == 0)
					return;
				//update ui
				if (ZM_THIRD && ZM->getThirdState() == ThirdState::GAME_DETAIL)
				{
					ZM_THIRD->flushUI();
				}
			}
		});
	}

	GameDetailData *data = static_cast<GameDetailData*>(ptr);
	if (data)
		isCancel = ((data->_apkState == DOWNLOAD_CANCEL) || (data->_apkState == DOWNLOAD_STOP));

	if (isCancel)
		return CURL_READFUNC_PAUSE;
	else
		return 0;
}
bool NetManager::downloadWithCurl(std::string url, std::string path, std::string package)
{
	FILE *fp = NULL;
	if (FileUtils::getInstance()->isFileExist(path.c_str()))// 以二进制形式追加
	{
		fp = fopen(path.c_str(), "ab+");
	}
	else // 二进制写
	{
		fp = fopen(path.c_str(), "wb");
	}

	if (!fp)
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
			LS_LOG("can not create file %s", path.c_str());
		});
		return false;
	}

	// 读取本地文件下载大小
	long localFileLength = LsTools::getLocalFileLength(path);//已经下载的大小
	LS_LOG("filePath:%s, length:%ld", path.c_str(), localFileLength); //4397779 //3377875

	GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);

	CURL *handle = curl_easy_init();

	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 300);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, myLoadPackage);//写文件回调方法
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp);// 写入文件对象
	curl_easy_setopt(handle, CURLOPT_RESUME_FROM, localFileLength);// 从本地大小位置进行请求数据
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, myProgressFuncForGame);//下载进度回调方法
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, data);// 传入本类对象
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 5);
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 1L);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 5L);

	CURLcode res = curl_easy_perform(handle);

	curl_easy_cleanup(handle);

	if (res == CURLE_OK)
	{
		fclose(fp);

		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, url, package, path]{
			//LS_LOG("succeed downloading package %s", url.c_str());
			GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
			float percent = LsTools::getDownloadPercent(data->_size, data->_basic.packageName);
			if (percent == 100.0f)
			{
				data->_apkState = (int)DOWNLOAD_SUCCESS;
				//update ui
				if (ZM_THIRD && ZM->getThirdState() == ThirdState::GAME_DETAIL)
					ZM_THIRD->flushUI();

				if (DataManager::getInstance()->_sUserData.isAutoInstall)
					LsTools::installAPK(package);
		//			PlatformHelper::installApp(path);
			}
		});
		return true;
	}
	else if (res == CURLE_ABORTED_BY_CALLBACK)
	{
		fclose(fp);

		//删除中间下载文件
		if (data && (data->_apkState == DOWNLOAD_CANCEL)){
			std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
			std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

			FileUtils::getInstance()->removeFile(apkPath);
			FileUtils::getInstance()->removeFile(dataPath);
		}

		return false;
	}
	else
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, res, package]{
			LS_LOG("error when download： %d", res);
			GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
			data->_apkState = (int)DOWNLOAD_UNDEFINE;
			//update ui
			if (ZM_THIRD && ZM->getThirdState() == ThirdState::GAME_DETAIL)
				ZM_THIRD->flushUI();
		});
		fclose(fp);
		return false;
	}
}

int NetManager::downloadWithFile(std::string url, std::string package)// 启动线程的方法
{
	//update download num

#ifdef USE_ANSY_CURLE
	std::thread t1(&NetManager::threadFunctionApk, this, url, package);
	t1.detach();

	//std::thread t2(&NetManager::threadFunctionData, this, DataManager::getInstance()->getApkDataPath(package), package);
	//t2.detach();
#else
	//
#endif
	return 0;
}

void* NetManager::threadFunctionApk(std::string url, std::string package)// 被启动的线程函数，注意必须是静态方法
{
	LS_LOG("download apk thread start");

	//
	auto downloadFunC = [](std::string url, std::string package)
	{
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		if (!data) return;

		std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
		std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

		std::string akpURL = url;
		std::string dataUrl = DataManager::getInstance()->getApkDataPath(package);
		std::vector<std::string> svc = LsTools::parStringByChar(data->_size);
		bool ret = true;

		//1. download apk
		long apkLength = atol(svc[0].c_str()); // 获取远程文件大小
		long apkLocalLen = LsTools::getLocalFileLength(apkPath);
		if (apkLength <= 0){
			LS_LOG("apk file fail...");
			return;
		}
		if (apkLocalLen < apkLength)
			ret = NetManager::getInstance()->downloadWithCurl(akpURL, apkPath, package); //直接下载APK 进行堵塞线程
		if (!ret)//暂停/下载失败，就直接返回
			return;

		//2. download data
		if (svc.size() > 1){
			long dataLength = atol(svc[1].c_str());
			long dataLocalLen = LsTools::getLocalFileLength(dataPath);
			if (dataLength <= 0){
				LS_LOG("apk file fail...");
				return;
			}
			if (dataLocalLen < dataLength)
				ret = NetManager::getInstance()->downloadWithCurl(dataUrl, dataPath, package); //直接下载APK 进行堵塞线程
		}

	};
	downloadFunC(url, package);
	//

	return nullptr;
}

void* NetManager::threadFunctionData(std::string url, std::string package)// 被启动的线程函数，注意必须是静态方法
{
	LS_LOG("thread download Data start");

	//
	auto downloadFunC = [](std::string url, std::string package)
	{
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		if (!data) return;

		std::string fullName = LsTools::lsStandardPath(LsTools::getDataPath() + "/data/game/apk/" + package + ".zip");
		LS_LOG("--1--  LocalFullFileName:%s", fullName.c_str());

		std::vector<std::string> svc = LsTools::parStringByChar(data->_size);
		if (svc.size() > 1){
			long length = atol(svc[1].c_str()); // 获取远程文件大小

			LS_LOG("--2-- download length: %ld", length);
			if (length <= 0)
			{
				LS_LOG("download file fail...");
				return;
			}

			LS_LOG("--4-");
			bool ret = false;
			ret = NetManager::getInstance()->downloadWithCurl(url, fullName, package); //直接下载APK 进行堵塞线程
		}
	};
	downloadFunC(url, package);
	//

	return nullptr;
}

void NetManager::submitDownloadFail(std::string packageName)
{
	sendCommend(CommendEnum::DOWNLOAD_FAIL, formatStr("&game=%s", packageName.c_str()), nullptr);
}

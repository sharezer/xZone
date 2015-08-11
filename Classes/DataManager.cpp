//
//  DataManager.cpp
//  xZone
//
//  Created by Sharezer on 15/1/17.
//
//

#include "DataManager.h"
#include "LsTools.h"
#include "GameDetail.h"
#include "ZoneManager.h"

USING_NS_CC;

DataManager::DataManager()
    : _languageDic(nullptr)
{
    _sUserData.reset();
    _sGameDetailData.clear();
	_applist.clear();
	//_localGameDoc.Clear();
}

DataManager::~DataManager()
{
}

static DataManager* s_sharedDataManager;

DataManager* DataManager::getInstance()
{
    if (!s_sharedDataManager) {
        s_sharedDataManager = new DataManager();
        CCASSERT(s_sharedDataManager, "FATAL: Not enough memory");
        s_sharedDataManager->init();
    }

    return s_sharedDataManager;
}

void DataManager::destroyInstance()
{
	CC_SAFE_DELETE(s_sharedDataManager);
}

bool DataManager::init()
{
	_sUserData.musicState = UD->getBoolForKey("music", true);
	_sUserData.soundState = UD->getBoolForKey("sound", true);
	_sUserData.isFirst = UD->getBoolForKey("first", true);
	_sUserData.category = UD->getIntegerForKey("category", 0);
	_sUserData.gamePackageName = UD->getStringForKey("gamePackage", "");

    return true;
}

void DataManager::initData()
{
	initChinese();

	rapidjson::Document doc;
	std::string localGamePath = LsTools::getDataPath() + s_LocalGameFileName;
	if (LsTools::readJsonWithFile(s_GameFileName, doc)){

		//添加收藏字段
		/*rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		for (unsigned int i = 0; i < doc.Size(); ++i){
			rapidjson::Value& sv = doc[i];
			sv.AddMember("collect", 0, allocator);
		}*/

		//从本地的LocalGame.json复制收集到新的数据上
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
		bool isLocalGameFileExist = FileUtils::getInstance()->isFileExist(localGamePath);
		rapidjson::Document local;
		if (isLocalGameFileExist)
			LsTools::readJsonWithFile(localGamePath.c_str(), local);

		for (unsigned int i = 0; i < doc.Size(); ++i){
			rapidjson::Value& v = doc[i];
			int collect = 0;
			if (isLocalGameFileExist){
				for (unsigned int j = 0; j < local.Size(); ++j){
					rapidjson::Value& l = local[j];
					if (!strcmp(v["package"].GetString(), l["package"].GetString()))
						collect = l[s_collectKey].GetInt();
				}
			}
			v.AddMember(s_collectKey, collect, allocator);
		}
		LsTools::saveJsonFile(localGamePath.c_str(), doc);
		initDetailData();
	}
	initAppList();
}

std::vector<GameDetailData>& DataManager::getDetailGameByType(std::string type)
{
    static std::vector<GameDetailData> selectData;
    selectData.clear();

	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		std::vector<std::string> res = LsTools::parStringByChar(_sGameDetailData[i]._type);
		for (std::size_t j = 0; j < res.size(); ++j){
			if (res[j].find(type) != std::string::npos){
				selectData.push_back(_sGameDetailData[i]);
				break;
			}
		}
    }

    return selectData;
}

std::vector<GameBasicData>& DataManager::getBasicGameByType(std::string type)
{
    static std::vector<GameBasicData> selectData;
    selectData.clear();

	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		std::vector<std::string> res = LsTools::parStringByChar(_sGameDetailData[i]._type);
		for (std::size_t j = 0; j < res.size(); ++j){
			if (res[j].find(type) != std::string::npos){
				selectData.push_back(_sGameDetailData[i]._basic);
				break;
			}
		}
	}

    return selectData;
}

std::vector<GameBasicData>& DataManager::getBasicGameOfDownload()
{
	static std::vector<GameBasicData> selectData;
	selectData.clear();

	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		if (_sGameDetailData[i]._apkState == DOWNLOAD_ING || 
			_sGameDetailData[i]._apkState == DOWNLOAD_STOP ||
			_sGameDetailData[i]._apkState == DOWNLOAD_SUCCESS)
		{
			selectData.push_back(_sGameDetailData[i]._basic);
		}
	}

	return selectData;
}

std::vector<GameDetailData*>& DataManager::getDownloadingGame()
{
	static std::vector<GameDetailData*> selectData;
	selectData.clear();

	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		if (_sGameDetailData[i]._apkState == DOWNLOAD_ING)
		{
			selectData.push_back(&_sGameDetailData[i]);
		}
	}

	return selectData;
}

void DataManager::updateDownloadStateByMaxCount(std::string package)
{
	std::vector<GameDetailData*> &data = getDownloadingGame();
	int maxD = DataManager::getInstance()->_sUserData.dlMaxCount;
	if ((int)data.size() > maxD){
		for (unsigned int i = 0, j = 0; (i < data.size() - maxD) && (j < data.size()); j++){
			if (package.size() > 0 && data[j]->_basic.packageName == package)
				continue;
			data[j]->_apkState = DOWNLOAD_STOP;
			i++;
		}
	}
}

std::vector<std::string>& DataManager::getGameFileListByType(std::string type)
{
	static std::vector<std::string> selectData;
	selectData.clear();

	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		std::vector<std::string> res = LsTools::parStringByChar(_sGameDetailData[i]._type);
		for (std::size_t j = 0; j < res.size(); ++j){
			if (res[j].find(type) != std::string::npos){
				selectData.push_back(_sGameDetailData[i]._fileName);
				break;
			}
		}
	}

	return selectData;
}

GameBasicData* DataManager::getBasicGameByPackage(std::string package)
{
	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		if (_sGameDetailData[i]._basic.packageName == package)
			return &_sGameDetailData[i]._basic;
	}
	return nullptr;
}

GameDetailData* DataManager::getDetailGameByPackage(std::string package)
{
	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		if (_sGameDetailData[i]._basic.packageName == package)
			return &_sGameDetailData[i];
	}
	return nullptr;
}

GameDetailData* DataManager::getDetailGameByID(std::string id)
{
	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i) {
		if (_sGameDetailData[i]._id == id)
			return &_sGameDetailData[i];
	}
	return nullptr;
}

std::string DataManager::getApkDownloadURL(std::string packageName)
{
	if (packageName.empty())
		return packageName;
	
	std::string url;
	GameDetailData *data = getDetailGameByPackage(packageName);
	std::string apkName = formatStr("%s_V_%s&step=1", packageName.c_str(), data->_version.c_str());
	url = ZM->getServerAddress() + "?" + formatStr("commend=%d&xDownload=%s", CommendEnum::DOWNLOAD_GAME, apkName.c_str());
	return url;
}

std::string DataManager::getApkDownloadURLNoVersion(std::string package)
{
	if (package.empty())
		return package;

	std::string url;
	url = ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_GAME_NO_VERSION, package.c_str());
	return url;
}


void DataManager::flushDetailData()
{
	for (unsigned int i = 0; i < _sGameDetailData.size(); ++i){
		auto& data = _sGameDetailData[i];
		rapidjson::Value& v = _localGameDoc[i];

		v["ID"].SetString(data._id.c_str());
		v["name"].SetString(data._basic._name.c_str());
		v["fileName"].SetString(data._fileName.c_str());
		v["type"].SetString(data._type.c_str());
		v["summary"].SetString(data._summary.c_str());
		v["screenhot"].SetString(data._screenHot.c_str());
		v["score"].SetString(data._score.c_str());
		v["size"].SetString(data._size.c_str());
		v["language"].SetString(data._language.c_str());
		v["starNum"].SetString(data._starNum.c_str());
		v["downloadNum"].SetString(data._downloadNum.c_str());
		v["fromAndress"].SetString(data._fromAddress.c_str());
		v["downloadURL"].SetString(data._downloadURL.c_str());
		v["Network"].SetString(data._network.c_str());
		v["Model"].SetString(data._model.c_str());
		v["OperationModel"].SetString(data._operModel.c_str());
		v["OpeartionExperience"].SetString(data._operExper.c_str());
		v["informationNum"].SetString(data._information.c_str());
		v["Version"].SetString(data._version.c_str());
		v["platform"].SetString(data._platform.c_str());
		v["manufacturer"].SetString(data._manufacturer.c_str());
		v["publisher"].SetString(data._publisher.c_str());
		v["addTime"].SetString(data._time.c_str());
		v["package"].SetString(data._basic.packageName.c_str());
		v["newAdd"].SetString(data._isNewAdd.c_str());
		v["hot"].SetString(data._isHot.c_str());
		v[s_collectKey].SetInt(data._isCollect);//-1->未定义，0->未收藏，1->已收藏
	}
	std::string localGamePath = LsTools::getDataPath() + s_LocalGameFileName;
	LsTools::saveJsonFile(localGamePath.c_str(), _localGameDoc);
}

int DataManager::isDownloadFullGame(std::string package)
{
	auto data = getDetailGameByPackage(package);
	if (!data) return DOWNLOAD_UNLOAD;

	std::string fullName = LsTools::getDataPath() + "/data/game/apk/" + data->_basic.packageName + "_V_" + data->_version + ".apk";
	std::string dataFileName = LsTools::getDataPath() + "/data/game/apk/" + data->_basic.packageName + ".zip";
	long localApkSize = LsTools::getLocalFileLength(fullName);
	long webApkSize = NetManager::getInstance()->getDownloadFileLength(DataManager::getInstance()->getApkDownloadURL(data->_basic.packageName));
	long localDataSize = LsTools::getLocalFileLength(dataFileName);
	long webDataSize = NetManager::getInstance()->getDownloadFileLength(DataManager::getInstance()->getApkDataPath(data->_basic.packageName));
	if (webApkSize < 0){
		webApkSize = NetManager::getInstance()->getDownloadFileLength(DataManager::getInstance()->getApkDownloadURLNoVersion(data->_basic.packageName));
		data->_downloadURL = DataManager::getInstance()->getApkDownloadURLNoVersion(data->_basic.packageName);
	}
	else{
		data->_downloadURL = DataManager::getInstance()->getApkDownloadURL(data->_basic.packageName);
	}

	long resultApkSize = (webApkSize >= 0 ? webApkSize : 0);
	long resultDataSize = (webDataSize >= 0 ? webDataSize : 0);
	data->_size = long2str(resultApkSize) + std::string(";") + long2str(resultDataSize);

	if (webApkSize == -1)//链接失效
		NetManager::getInstance()->submitDownloadFail(data->_basic.packageName);

	std::string version = DataManager::getInstance()->isApkInstall(data->_basic.packageName);
	if ((version == data->_version) || (version.find(data->_version) != std::string::npos))
		return DOWNLOAD_OPEN;
	if (localApkSize > 0)
	{
		if (webApkSize > 0 && webApkSize <= localApkSize &&
			(webDataSize < 0 || (webDataSize <= localDataSize)))
			return DOWNLOAD_SUCCESS;
		else 
			return DOWNLOAD_STOP;
	}
	else
		return DOWNLOAD_UNLOAD;
}

void DataManager::initDetailData()
{
	//读起游戏列表数据
	_sGameDetailData.clear();

	LsTools::readJsonWithFile(s_LocalGameFileName, _localGameDoc);
	for (unsigned int i = 0; i < _localGameDoc.Size(); ++i) {
		GameDetailData data;
		rapidjson::Value& v = _localGameDoc[i];
		data._id = v["ID"].GetString();
		data._basic._name = v["name"].GetString();
		data._fileName = v["fileName"].GetString();
		data._type = v["type"].GetString();
		data._summary = v["summary"].GetString();
		data._basic._icon = LsTools::lsStandardPath(std::string(s_GameIconPath) + "/" + v["package"].GetString() + ".png")/*v["icon"].GetString()*/;
		data._screenHot = v["screenshot"].GetString();
		data._score = LsTools::removeAllLeftZero(v["score"].GetString(), '0');
		data._size = LsTools::removeAllLeftZero(v["size"].GetString(), '0');
		data._language = v["language"].GetString();
		data._starNum = v["starNum"].GetString();
		data._downloadNum = v["downloadNum"].GetString();
		data._fromAddress = v["fromAndress"].GetString();
		data._downloadURL = v["downloadURL"].GetString();
		data._network = v["network"].GetString();
		data._model = v["model"].GetString();
		data._operModel = v["operationModel"].GetString();
		data._operExper = v["opeartionExperience"].GetString();
		data._information = v["informationNum"].GetString();
		data._version = v["version"].GetString();
		data._platform = v["platform"].GetString();
		data._manufacturer = v["manufacturer"].GetString();
		data._publisher = v["publisher"].GetString();
		data._time = v["addTime"].GetString();
		data._basic._number = i;
		data._basic.packageName = v["package"].GetString();
		data._isNewAdd = v["newAdd"].GetString();
		data._isHot = v["hot"].GetString();
		data._isCollect = v[s_collectKey].GetInt();
		data._apkState = DOWNLOAD_UNKNOW;
		data._basic._isStencil = false;

		_sGameDetailData.push_back(data);
	}
	/*for (int i = 0; i < (int)_sGameDetailData.size(); ++i)
		_sGameDetailData[i]._apkState = isDownloadFullGame(&_sGameDetailData[i]);*/
}

void DataManager::initAppList()
{
	LS_LOG("init app list");
	_applist.clear();

	if (PH::getAppList(_appDoc)){
		rapidjson::Value& p = DataManager::getInstance()->_appDoc["package"];
		for (unsigned int i = 0; i < p.Size(); ++i) {
			//LS_LOG("initAppList: %d", i);
			AppStruct app;
			rapidjson::Value& v = p[i];
			app.name = v["name"].GetString();
			app.package = v["packageName"].GetString();
			app.appIcon = v["appIcon"].GetString();
			app.path = v["path"].GetString();
			app.customOrder = v["customOrder"].GetInt();
			app.state = v["state"].GetInt();
			app.version = v["version"].GetString();

			_applist.push_back(app);
		}
	}
}

void DataManager::flushAppList()
{
	for (unsigned int i = 0; i < _applist.size(); ++i){
		auto& data = _applist[i];
		rapidjson::Value& v = _appDoc["package"][i];
		v["name"].SetString(data.name.c_str());
		v["packageName"].SetString(data.package.c_str());
		v["appIcon"].SetString(data.appIcon.c_str());
		v["path"].SetString(data.path.c_str());
		v["customOrder"].SetInt(data.customOrder);
		v["state"].SetInt(data.state);
		v["version"].SetString(data.version.c_str());
	}
	std::string savePath = LsTools::getDataPath() + "myGame.json";
	LsTools::saveJsonFile(savePath.c_str(), _appDoc);
}

std::string DataManager::isApkInstall(std::string package)
{
	for (auto& app : _applist)
	{
		if (app.package != package)
		continue;
		return app.version;
	}
	return "";
}

std::string DataManager::getApkDataPath(std::string package)
{
    return ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_GAME_DATA, package.c_str());
}

void DataManager::flushUserData()
{
    UD->setBoolForKey("music", _sUserData.musicState);
    UD->setBoolForKey("sound", _sUserData.soundState);
    UD->setBoolForKey("first", _sUserData.isFirst);
    UD->setIntegerForKey("category", _sUserData.category);
	UD->setStringForKey("gamePackage", _sUserData.gamePackageName);

    UD->flush();
}

void DataManager::saveGameVersionForKey(const std::string& key, const std::string& version)
{
	UD->setStringForKey(key.c_str(), version);
	UD->flush();
}

std::string DataManager::getGameVersionForKey(const std::string& key)
{
	return UD->getStringForKey(key.c_str(), "");
}
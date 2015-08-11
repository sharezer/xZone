//
//  Upgrade.cpp
//  xZone
//
//  Created by Sharezer on 15/3/3.
//
//

#include "Upgrade.h"
#include "ZoneManager.h"
#include "LsTools.h"
#include "Global.h"
#include "NetManager.h"
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#endif
#include "DataManager.h"
USING_NS_CC;
USING_NS_CC_EXT;

#define GAME_LIST_VERSION_KEY	"GAME_LIST_VERSION"

Upgrade::Upgrade()
	: _pathToSave("")
	, _showDownloadInfo(nullptr)
{
}

Upgrade::~Upgrade() {}

bool Upgrade::init()
{
	if (!Layer::init())
		return false;
	Size winSize = Director::getInstance()->getWinSize();
	initDownloadDir();

	auto sprite = Sprite::create("Start.png");
	sprite->setPosition(MY_SCREEN_CENTER);
	this->addChild(sprite);

	_showDownloadInfo = Label::createWithTTF("", s_labelTTF, 48);
	_showDownloadInfo->setPosition(Vec2(winSize.width / 2, winSize.height / 2 - 20));
	this->addChild(_showDownloadInfo);

	if (!s_isUserNet) {
		onSuccess();
		return true;
	}
	check(nullptr);

	return true;
}

void Upgrade::onError(AssetsManager::ErrorCode errorCode)
{
	switch (errorCode) {
	case AssetsManager::ErrorCode::CREATE_FILE:
		LS_LOG("CREATE_FILE");
		reCheck();
		break;
	case AssetsManager::ErrorCode::NETWORK:
		LS_LOG("NETWORK");
		reCheck();
		break;
	case AssetsManager::ErrorCode::NO_NEW_VERSION:
		//_showDownloadInfo->setString("NO_NEW_VERSION");
		ZM_LOADING->setPercent(80);
		onSuccess();
		break;
	case AssetsManager::ErrorCode::UNCOMPRESS:
		LS_LOG("UNCOMPRESS");
		reCheck();
		break;
	default:
		break;
	}
}

void Upgrade::onProgress(int percent)
{
	if (percent < 0)
		return;
	char progress[20];
	sprintf(progress, "download %d", percent);
	ZM_LOADING->setPercent(percent * 0.9f);
	//_showDownloadInfo->setString(progress);
}

void Upgrade::onSuccess()
{
	//_showDownloadInfo->setString("Success");
	if (!s_isUserNet)
		start();
	else
		NetManager::getInstance()->sendCommend(CommendEnum::GAME_LIST,
		"",
		[=](const rapidjson::Value& value){
		if (this && this == ZM->_upgrade){
			std::string version = value["version"].GetString();
			ZM_LOADING->setPercent(90);

			if (DataManager::getInstance()->getGameVersionForKey(GAME_LIST_VERSION_KEY) != version || !FileUtils::getInstance()->isFileExist(s_GameFileName)) {
				this->loadGameJson(version);
				/*NetManager::getInstance()->loadFile(ZM->getServerAddress() + "?" + formatStr("commend=%d", CommendEnum::DOWNLOAD_ICON_ALL),
				LsTools::getDataPath() + "/data/game/",
				"icon.zip",
				[&, version]() {
				this->loadGameJson(version);
				},
				[&]() {	start(); },
				true);*/
			}
			else
				start();
		}
	});
}

void Upgrade::loadGameJson(std::string version)
{
	/*NM->loadFile(ZM->getServerAddress() + "?" + formatStr("commend=%d", CommendEnum::UPGRADE_GAME_JSON),
		LsTools::getDataPath(),
		"game.json",
		[=]() {
		ZM->initMainLayer();
		DM->saveGameVersionForKey(GAME_LIST_VERSION_KEY, version);
		},
		[=]() {
		ZM->initMainLayer();
		},
		false);*/
	NetManager::getInstance()->sendCommend(CommendEnum::UPGRADE_GAME_JSON,
		"",
		[=](const rapidjson::Value& value){
		if (this && this == ZM->_upgrade){
			std::string path = LsTools::getDataPath() + s_GameFileName;
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			value.Accept(writer);
			std::string str = buffer.GetString();

			FILE* fp = std::fopen(path.c_str(), "wb");
			CCASSERT(fp != nullptr, "file open error");
			//fwrite(str.c_str(), str.length(), 1, fp);
			fputs(str.c_str(), fp);
			fclose(fp);
			start();
			DataManager::getInstance()->saveGameVersionForKey(GAME_LIST_VERSION_KEY, version);
		}	
	}, [=]() {
		start();
	});
}

void Upgrade::initDownloadDir()
{
	_pathToSave = LsTools::lsStandardPath(LsTools::getDataPath() + DOWNLOAD_FIEL);

	if (!FileUtils::getInstance()->isDirectoryExist(_pathToSave)){
		FileUtils::getInstance()->createDirectory(_pathToSave);
		getAssetManager()->deleteVersion();
	}
}

std::string _getUpgradeResUrl(){
	return ZM->getServerAddress() + "?" + formatStr("commend=%d&temp=.zip", CommendEnum::UPGRADE_RES);
}

std::string _getUpgradeCheckUrl(){
	return ZM->getServerAddress() + "?" + formatStr("commend=%d", CommendEnum::UPGRADE);
}

AssetsManager* Upgrade::getAssetManager()
{
	static AssetsManager* assetManager = nullptr;
	if (!assetManager) {
		//        std::string urlRes = ZM->getServerAddress() + "res.zip";

		assetManager = new AssetsManager(_getUpgradeResUrl().c_str(), _getUpgradeCheckUrl().c_str(), _pathToSave.c_str());
		assetManager->setDelegate(this);
		assetManager->setConnectionTimeout(5);
	}
	return assetManager;
}

void Upgrade::reset(cocos2d::Ref* sender)
{
	_showDownloadInfo->setString("");
	if (FileUtils::getInstance()->isDirectoryExist(_pathToSave))
		LsTools::removeFileForPath(_pathToSave);
	getAssetManager()->deleteVersion();
	initDownloadDir();
}

void Upgrade::check(cocos2d::Ref* sender)
{
	ZM->_isLoading = true;
	ZM_LOADING->showBG(false);
	ZM_LOADING->_loadSprite->setVisible(false);
	ZM_LOADING->resetPercent();

	ZM_BASE->runAction(LsTools::delayAndCallFunc(0.25f, [&]() {
		_showDownloadInfo->setString("");

		getAssetManager()->setPackageUrl(_getUpgradeResUrl().c_str());
		getAssetManager()->setVersionFileUrl(_getUpgradeCheckUrl().c_str());

		getAssetManager()->update();
	}));
}

void Upgrade::reCheck()
{
	ZM->_isLoading = false;
	ZM->setUsingCustomEventEnable(true);
	ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("reCheck"));
	ZM_DIALOG->setDialogType(DialogType::TWO_BTN);
	ZM_DIALOG->okEvent = [&]() {
		ZM->setUsingCustomEventEnable(false);
		this->check(nullptr);
	};
	ZM_DIALOG->cancelEvent = [&]() {
		ZM->setUsingCustomEventEnable(false);
		if (DataManager::getInstance()->getGameVersionForKey("GAME_LIST_VERSION").empty())
		{
			Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
			exit(0);
#endif
		}
		else
			start();
	};
}

void Upgrade::start()
{
	ZM_LOADING->setPercent(100);
	ZM_BASE->runAction(LsTools::delayAndCallFunc(0.1f, [&]() { ZM->initMainLayer(); }));
}

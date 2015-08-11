//
//  DLControl.cpp
//  xZone
//
//  Created by Sharezer on 15/2/9.
//
//

#include "DLControl.h"
#include "ZoneManager.h"
#include "json/document.h"
#include "PlatformHelper.h"
#include "LsTools.h"
#include "DataManager.h"
#include "NetManager.h"

USING_NS_CC;
USING_NS_GUI;

#define S_GD DataManager::getInstance()->_sGameDetailData

DLControl::DLControl() : _focusState(0), _gamePageView(nullptr)
{
    
}

DLControl::~DLControl()
{
    
}

bool DLControl::init()
{
    if (!BaseLayer::init()) {
        return false;
    }
    
    _type = LayerType::SECOND;
    return true;
}

bool DLControl::initUI()
{
    bool bRet = false;
    do {
        _rootNode = CSLoader::createNode(s_DownloadControl);
        this->addChild(_rootNode);
        
        bRet = true;
    } while (0);
    return bRet;
}

bool DLControl::initData()
{
    bool bRet = false;
    do {
		//update apk path
		std::string fullName = LsTools::lsStandardPath(LsTools::getDataPath() + "/data/game/apk/");
		for (unsigned int i = 0; i < S_GD.size(); ++i){
			std::string name = fullName + S_GD[i]._basic.packageName + "_V_" + S_GD[i]._version + ".apk";
			if (S_GD[i]._apkState == DOWNLOAD_UNKNOW && 
				FileUtils::getInstance()->isFileExist(name))
				S_GD[i]._apkState = DataManager::getInstance()->isDownloadFullGame(S_GD[i]._basic.packageName);
		}
		//
		DLGameList::GameListParams params;
		params._leftMargin = 38;
		params._colsDiff = 30;//行间距
		params._rowsDiff = 30;//列间距
		params._topMargin = 29;//顶间距
		params._bottomMargin = 29;
		params._rightMargin = 120;
		params._extenSize = 50;

		if (_gamePageView)
		{
			_gamePageView->removeFromParent();
			_gamePageView = nullptr;
		}
		std::vector<GameBasicData> data = DataManager::getInstance()->getBasicGameOfDownload();
		_gamePageView = DLGameList::create(data, params);
		_gamePageView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
		_gamePageView->setPosition(Vec2(1005, 532));
		_gamePageView->setTouchEnabled(true);
		_gamePageView->addEventListener(CC_CALLBACK_2(DLControl::pageViewEvent, this));
		_gamePageView->changeFocus(nullptr);
		for (auto &pages : _gamePageView->getPages())
		{
			for (auto &child : pages->getChildren())
			{
				auto img = static_cast<ui::ImageView*>(child);
				img->addTouchEventListener(CC_CALLBACK_2(DLControl::clickGameItem, this));
				auto leftT = static_cast<ui::ImageView*>(LsTools::seekNodeByName(img, "leftB"));
				leftT->addTouchEventListener(CC_CALLBACK_2(DLControl::clickOpItem, this));
				auto rightT = static_cast<ui::ImageView*>(LsTools::seekNodeByName(img, "rightB"));
				rightT->addTouchEventListener(CC_CALLBACK_2(DLControl::clickOpItem, this));
				_mapFocusZOrder.insert(std::pair<int, int>(img->getTag(), img->getLocalZOrder()));
			}
		}

		_rootNode->addChild(_gamePageView);
		//update title
		auto num = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Text_3"));
		auto pageT = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Text_4"));
		num->setString(int2str(data.size()) + DataManager::getInstance()->valueForKey("kuan"));
		if (data.size() > 0){
			pageT->setString("1/" + int2str(_gamePageView->getPagesCount()));
		}
		else{
			pageT->setString("0/0");
		}
		//update memory tips(223)
		auto deviceImg = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Image_3"));
		auto deviceTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Text_7"));
		auto sysImg = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Image_4"));
		auto sysTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Text_8"));
		std::string  sdTotalSize = PlatformHelper::getSDTotalSize();
		std::string  sdValSize = PlatformHelper::getSDAvailableSize();
		std::string  sysTotalSize = PlatformHelper::getRomTotalSize();
		std::string  sysValSize = PlatformHelper::getRomAvailableSize();
		float sdPercent = getSizePercent(sdTotalSize, sdValSize);
		float sysPercent = getSizePercent(sysTotalSize, sysValSize);
		deviceTxt->setString(DataManager::getInstance()->valueForKey("avail") + std::string(" ") + sdValSize);
		sysTxt->setString(DataManager::getInstance()->valueForKey("avail") + std::string(" ") + sysValSize);
		deviceImg->setScaleX(sdPercent);
		sysImg->setScaleX(sysPercent);

        bRet = true;
    } while (0);
    return bRet;
}

void DLControl::onClickOK()
{
	if (_focus && _focusState == 1)
		clickOpItem(LsTools::seekNodeByName(_focus, "leftB"), ui::Widget::TouchEventType::ENDED);
	else if (_focus && _focusState == 2)
		clickOpItem(LsTools::seekNodeByName(_focus, "rightB"), ui::Widget::TouchEventType::ENDED);
}

void DLControl::flushUI()
{
	if (_focus && _focus->getUserData()){
		auto packName = *(static_cast<std::string*>(_focus->getUserData()));
		if (!DataManager::getInstance()->isApkInstall(packName).empty() && DataManager::getInstance()->isApkInstall(packName) != ""){
			GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(packName);
			data->_apkState = DOWNLOAD_OPEN;

			if (DataManager::getInstance()->_sUserData.isInstallFinishDel)
				LsTools::deleteApkAndData(packName);
		}
	}

	_focus = nullptr;
	_focusState = 0;
	initData();
}

void DLControl::onClickMenu()
{
    
}

void DLControl::changeFocus(::EventKeyboard::KeyCode keyCode)
{
	Vec2DirEnum dir = LsTools::dirKeyCode2Vec2Enum(keyCode);
	switch (dir)
	{
	case Vec2DirEnum::EDIR_LEFT:
	{
		if (_focusState == 2)
		{
			_gamePageView->changeFocus(_focus, 1);
			_focusState = 1;
			return;
		}
	}
		break;
	case Vec2DirEnum::EDIR_RIGHT:
	{
		if (_focusState == 1)
		{
			_gamePageView->changeFocus(_focus, 2);
			_focusState = 2;
			return;
		}
	}
		break;
	case Vec2DirEnum::EDIR_CENTER:
	case Vec2DirEnum::EDIR_UP:
		break;
	case Vec2DirEnum::EDIR_DOWN:
	{
		if (_focusState == 0 && _focus)
		{
			_gamePageView->changeFocus(_focus, 1);
			_focusState = 1;
			return;
		}
	}
		break;
	default:
		break;
	}
	_focusState = 0;

	Node *next = LsTools::getNextFocus(keyCode, _gamePageView->getCurPage());
	if (!next)//null
		next = LsTools::getNextPageFocus2(_gamePageView, keyCode);

	if (next)
	{
		commonFocusAction(next);
		ZM->showSelect(true, next->getContentSize() * FOCUS_SCALE_NUM, _gamePageView->convertToWorldSpace(next->getPosition()));
		_gamePageView->setUserObject(_focus);
		_gamePageView->changeFocus(next, 0);
	}
	else
		ZM->showSelect(false);
}

void DLControl::pageViewEvent(Ref* sender, ui::PageView::EventType type)
{
	if (type != ui::PageView::EventType::TURNING)
		return;

//	auto pageView = dynamic_cast<DLGameList*>(sender);
	auto pageT = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "Text_4"));
	pageT->setString(int2str(_gamePageView->getCurPageIndex()+1) + "/" + int2str(_gamePageView->getPagesCount()));
}

void DLControl::clickGameItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED)
		return;
	auto btn = static_cast<ui::ImageView*>(sender);

	if (btn != _focus)
	{
		_focusState = 0;
		auto leftB = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "leftB"));
		auto rightB = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "rightB"));
		if (leftB && rightB){
			leftB->loadTexture("MyGame_button.png");
			rightB->loadTexture("MyGame_button.png");
		}

		commonFocusAction(btn);
		ZM->showSelect(true, _focus->getContentSize() * FOCUS_SCALE_NUM, _gamePageView->convertToWorldSpace(_focus->getPosition()));
		_gamePageView->changeFocus(_focus);
		_gamePageView->getCurPage()->setUserObject(_focus);
		return;
	}
}

void DLControl::clickOpItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if ((type != ui::Widget::TouchEventType::ENDED) || (!_focus) || (!_focus->getUserData()))
		return;
	auto packName = *(static_cast<std::string*>(_focus->getUserData()));
	auto data = DataManager::getInstance()->getDetailGameByPackage(packName);
	auto leftB = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "leftB"));
	auto rightB = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "rightB"));
	auto btn = static_cast<ui::ImageView*>(sender);
	if (leftB == btn){
		if (_focusState != 1){
			leftB->loadTexture("MyGame_button02.png");
			rightB->loadTexture("MyGame_button.png");
			_focusState = 1;
			return;
		}
		auto leftTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_focus, "leftT"));
		auto rightTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_focus, "rightT"));
		if (data->_apkState == DOWNLOAD_ING){//ing
			data->_apkState = DOWNLOAD_STOP;//stop
			leftTxt->setString(DataManager::getInstance()->valueForKey("continue"));
			rightTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
			//调用暂停
			_gamePageView->updateFocusUI(packName);
		}
		else if (data->_apkState == DOWNLOAD_STOP){//stop
			data->_apkState = DOWNLOAD_ING;//ing
			leftTxt->setString(DataManager::getInstance()->valueForKey("stop"));
			rightTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
			//调用继续
			_gamePageView->updateFocusUI(packName);
			NetManager::getInstance()->downloadWithFile(data->_downloadURL, packName);
			//download max
			DataManager::getInstance()->updateDownloadStateByMaxCount(packName);
		}
		else if (data->_apkState == DOWNLOAD_SUCCESS){//download success
			//安装游戏
			std::string fullName = LsTools::lsStandardPath(LsTools::getDataPath() + "/data/game/apk/");
			if (data)
				packName += "_V_" + data->_version;
			packName += ".apk";
			LS_LOG("path: %s, name: %s", fullName.c_str(), (packName).c_str());
			//PlatformHelper::installApp(fullName, packName);
			LsTools::installAPK(packName);
			//回调更新列表
			//this->flushUI();
		}
	}
	else if (rightB == btn){
		if (_focusState != 2){
			leftB->loadTexture("MyGame_button.png");
			rightB->loadTexture("MyGame_button02.png");
			_focusState = 2;
			return;
		}
		if (data->_apkState == DOWNLOAD_ING || data->_apkState == DOWNLOAD_STOP){//ing
			data->_apkState = DOWNLOAD_CANCEL;//cancel
			removeApkFile(data->_basic.packageName);
			//更新下载列表
			this->flushUI();
			changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
		}
		else if (data->_apkState == DOWNLOAD_SUCCESS){//download success
			data->_apkState = DOWNLOAD_UNLOAD;//cancel
			removeApkFile(data->_basic.packageName);
			//更新下载列表
			this->flushUI();
			changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
		}
	}
}

void DLControl::removeApkFile(std::string &package)
{
	auto data = DataManager::getInstance()->getDetailGameByPackage(package);

	std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + package + "_V_" + data->_version + ".apk";
	std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + package + ".zip";

	FileUtils::getInstance()->removeFile(apkPath);
	FileUtils::getInstance()->removeFile(dataPath);
}

void DLControl::getSizeByMB(std::string sizeStr, double &size)
{
	if (sizeStr == "PB"){
		size *= (1024 * 1024 * 1024);
	}
	else if (sizeStr == "TB"){
		size *= (1024 * 1024);
	}
	else if (sizeStr == "GB"){
		size *= 1024;
	}
	else if (sizeStr == "KB"){
		size /= 1024.0;
	}
	else if (sizeStr == "B"){
		size /=  (1024.0 * 1024.0);
	}
}

float DLControl::getSizePercent(std::string total, std::string avail)
{
	size_t pos = total.find_first_of(' ');
	std::string totalSize = total.substr(0, pos);
	std::string totalBase = total.substr(pos + 1, total.size() - pos);
	pos = avail.find_first_of(' ');
	std::string availSize = avail.substr(0, pos);
	std::string availBase = avail.substr(pos + 1, avail.size() - pos);
	double aSize = atof(availSize.c_str());//有效容量
	double tSize = atof(totalSize.c_str());//总的容量

	getSizeByMB(availBase, aSize);
	getSizeByMB(totalBase, tSize);

	return (float)aSize / tSize;
}

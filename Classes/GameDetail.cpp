//
//  GameDetail.cpp
//  xZone
//
//  Created by Sharezer on 15/2/3.
//
//

#include "GameDetail.h"
#include "ZoneManager.h"
#include "DataManager.h"
#include "GCategoryDetailWidget.h"
#include "LsTools.h"
#include "xRichText.h"
#include "QR_Encode.h"
#include "Global.h"
#include "NetManager.h"
#include "GCategoryDetail.h"
#include "xRichText.h"

#define kInAngleZ 270
#define kInDeltaZ 90
#define kOutAngleZ 0
#define kOutDeltaZ 90

USING_NS_CC;

GameDetail* GameDetail::create(std::string& package)
{
    GameDetail* pRet = new GameDetail(package);
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    }
    else {
        delete pRet;
        pRet = NULL;
        return NULL;
    }
}

GameDetail::GameDetail(std::string& package)
    : _gamePackage(package)
    , _gameBG(nullptr)
    , _state(ShowState::NORMAL_STATE)
    , _downloadProgressRoot(nullptr)
    , _focusName(nullptr)
    , _drawNode(nullptr)
    , _iconNode(nullptr)
    , _focusNum(DetailFocusType::DL_BTN)
    , _isHaveVideo(false)
{
    _videoPlayFlagV.clear();
}

GameDetail::~GameDetail()
{
    _videoPlayFlagV.clear();
}

bool GameDetail::init()
{
    if (!BaseLayer::init()) {
        return false;
    }
    //    LayerType tyep = getType();
    _type = LayerType::THIRD;
    ZM_STATE->setVisible(false);

    NetManager::getInstance()->sendCommend(CommendEnum::GAME_DETAIL, formatStr("&game=%s", _gamePackage.c_str()),
        [=](const rapidjson::Value& value) {
            if (this && this == ZM_THIRD) {

                if (value.HasMember("version")) {
                    std::string version = value["version"].GetString();
                    if (DataManager::getInstance()->getGameVersionForKey(_gamePackage) != version)
                        downloadScreenShoot();
                }

                this->_isHaveVideo = (value.HasMember("video") && value["video"].GetInt() == 1);
                this->showVideoPlayFlag(_isHaveVideo);
            }
        });

    return true;
}

void GameDetail::downloadScreenShoot()
{
    std::string str = ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_SCREEN_SHOT, _gamePackage.c_str());
    NetManager::getInstance()->loadFile(str,
        LsTools::getDataPath() + "/data/game/screenhot/",
        _gamePackage + ".zip",
        []() { ZM->flushUI(
                   false); },
        nullptr,
        true);
}

bool GameDetail::clickDownloadButton()
{
    auto downloadTxt = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "8_leftDownload_Txt"));
    auto downloadProgRoot = static_cast<cocos2d::Node*>(LsTools::seekNodeByName(_rootNode, "downloadProgressRoot"));
    auto progTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_downloadProgressRoot, "percentName"));

    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);
    long webSize = atol(data->_size.c_str());
    if (webSize <= 0)
        return false;

    switch (data->_apkState) {
    case DOWNLOAD_ING: //下载中->取消下载
    {
#ifdef USE_ANSY_CURLE
        data->_apkState = (int)DOWNLOAD_CANCEL;
#endif
        downloadTxt->setString(DataManager::getInstance()->valueForKey("download"));
        downloadProgRoot->setVisible(false);
    } break;
    case DOWNLOAD_SUCCESS: //下载成功->安装并打开
    {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("install"));
        downloadProgRoot->setVisible(false);
        std::string fullName = LsTools::lsStandardPath(LsTools::getDataPath() + "/data/game/apk/");
        std::string apkName = _gamePackage;
        if (data)
            apkName += "_V_" + data->_version;
        apkName += ".apk";
        LS_LOG("path: %s, name: %s", fullName.c_str(), apkName.c_str());
        if (DataManager::getInstance()->_sUserData.isAutoInstall)
            LsTools::installAPK(_gamePackage);
        //			PlatformHelper::installApp(fullName, apkName);
        //
    } break;
    case DOWNLOAD_UNKNOW: //
    case DOWNLOAD_STOP: //暂停下载->重新下载
    case DOWNLOAD_UNDEFINE: //下载失败->重新下载
    case DOWNLOAD_CANCEL: //取消下载->重新下载
    case DOWNLOAD_UNLOAD: //未下载->下载中
    {
#ifdef USE_ANSY_CURLE
        NetManager::getInstance()->downloadWithFile(DataManager::getInstance()->getApkDownloadURL(_gamePackage), _gamePackage);
        data->_apkState = (int)DOWNLOAD_ING;
        DataManager::getInstance()->updateDownloadStateByMaxCount(_gamePackage);
#endif
        downloadTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
        downloadProgRoot->setVisible(true);
        progTxt->setString("0%");
    } break;
    case DOWNLOAD_OPEN: {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("open"));
        downloadProgRoot->setVisible(false);
        PlatformHelper::runApp(_gamePackage);
    } break;
    }

    return true;
}

bool GameDetail::initUI()
{
    _rootNode = CSLoader::createNode(s_GameDetailUI);
    this->addChild(_rootNode);
    ZM->showSecond(false);
    ZM->showSelect(false);

    CCASSERT(!(_gamePackage == ""), "game package null");
    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);

    //update download apkState
    if (data && data->_apkState == DOWNLOAD_UNKNOW) {
        data->_apkState = DataManager::getInstance()->isDownloadFullGame(data->_basic.packageName);
    }

    //create screen node
    {
        initScreenNode(data);
    }
    //create myFav node
    {
        initFavNode();
    }
    //set icon
    {
        initICON(data);
    }
    //download and collect
    {
        initDownloadUI(data);
        initCollectNode(data);
    }
    //init for game
    {
        initGameInfor(data);
        initSummayText(data);
    }
    //init qrEncode
    {
        initQrencode(data);
    }
    {
        updateDownloadProg();
    }

    return true;
}

bool GameDetail::initData()
{
    return true;
}

void GameDetail::flushUI()
{
    LS_LOG("GameDetail::flushUI");
    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);
    std::vector<std::string> scrVec = LsTools::parStringByChar(data->_screenHot);

    for (unsigned int i = 0; i < 2; ++i) {
        std::string file = "com_idle02.png";
        if (i < scrVec.size() && FileUtils::getInstance()->isFileExist(LsTools::lsStandardPath(std::string(s_GameScreenPath) + "/" + data->_basic.packageName + std::string("/") + scrVec[i]))) {
            file = LsTools::lsStandardPath(std::string(s_GameScreenPath)
                + "/" + data->_basic.packageName + std::string("/") + scrVec[i]);
        }

        auto screenShot = dynamic_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, formatStr("screenShot_%d", i)));
        screenShot->loadTexture(file);
        float wSca = 651.0f / screenShot->getContentSize().width;
        float hSca = 454.0f / screenShot->getContentSize().height;
        float minScale = (wSca > hSca ? wSca : hSca);
        screenShot->setScale(minScale);
        //隐藏提示文字
        auto tips = LsTools::seekNodeByName(screenShot->getParent(), "downloadTips");
        if (tips && (file != "com_idle02.png"))
            tips->setVisible(false);
    }

    //刷新图标
    auto gameIcon = dynamic_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "7_leftIcon"));
    std::string iconPath = data->_basic._icon;
    if (gameIcon && FileUtils::getInstance()->isFileExist(iconPath))
        gameIcon->loadTexture(iconPath);

    //刷新推荐图标
    std::vector<GameDetailData>& favData = DataManager::getInstance()->getDetailGameByType(int2str(DataManager::getInstance()->_sUserData.category));
    for (unsigned int i = 0, j = 0; j < 6 && i < favData.size(); ++i) {
        GameDetailData tempData = favData[i];
        if (tempData._basic.packageName == _gamePackage)
            continue;

        auto favIcon = dynamic_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, formatStr("favIcon_%d", j)));
        std::string faviIconPath = tempData._basic._icon;
        if (favIcon && FileUtils::getInstance()->isFileExist(faviIconPath))
            favIcon->loadTexture(faviIconPath);
        j++;
    }

    auto downloadTxt = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "8_leftDownload_Txt"));
    if (!DataManager::getInstance()->isApkInstall(_gamePackage).empty() && DataManager::getInstance()->isApkInstall(_gamePackage) != "") {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("open"));
        data->_apkState = DOWNLOAD_OPEN;

        if (DataManager::getInstance()->_sUserData.isInstallFinishDel)
            LsTools::deleteApkAndData(_gamePackage);
    }
    //更新进度条
    updateDownloadProg();

    showVideoPlayFlag(_isHaveVideo);
}

void GameDetail::showVideoPlayFlag(bool isShow)
{
    for (auto& flag : _videoPlayFlagV) {
        auto parent = flag->getParent();
        flag->setPosition(Vec2(parent->getContentSize().width / 2.0f, parent->getContentSize().height / 2.0f));
        flag->setVisible(isShow);
        flag->setScale(1 / parent->getScale());
    }
}

std::string GameDetail::getApkPath()
{
    std::string fullName = LsTools::lsStandardPath(LsTools::getDataPath() + "/data/game/apk/");
    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);
    if (!data)
        return "";
    std::string apkName = _gamePackage;
    apkName += "_V_" + data->_version + ".apk";
    return LsTools::lsStandardPath(fullName + apkName);
}

void GameDetail::onClickBack()
{
    if (_state == ShowState::NORMAL_STATE) {
        ZM->changeThirdState(ThirdState::NONE);
        ZM_STATE->setVisible(true);
        if (ZM_SECOND) {
            /*if (ZM->getSecondState() == SecondState::HOT_GAME ||
				ZM->getSecondState() == SecondState::NEW_GAME ||
				ZM->getSecondState() == SecondState::COLLECT_GAME)
				ZM->changeSecondState(ZM->getSecondState());*/

            ZM->showSecond(true);
            ZM_SECOND->showCurFocus();
        }
        else {
            ZM->showMain(true);
            ZM_MAIN->showCurFocus();
        }
    }
    else {
        backNormalState();
    }
}

void GameDetail::onClickMenu()
{
}

void GameDetail::onClickOK()
{
    if (_focus)
        clickOkItem(_focus, ui::Widget::TouchEventType::ENDED);
}

#define NORMAL_NUM_FUCUS                                                      \
    if (_focus) {                                                             \
        _focus->setScale(FOCUS_NORMAL_SCALE);                                 \
        _focus->setLocalZOrder(_focus->getLocalZOrder() - FOCUS_Z_ORDER_ADD); \
    }

#define SCALE_NUM_FOCUS(node, isLittle)                               \
    node->setScale(FOCUS_SCALE_NUM);                                  \
    node->setLocalZOrder(node->getLocalZOrder() + FOCUS_Z_ORDER_ADD); \
    ZM->showSelect(true,                                              \
        node->getContentSize() * FOCUS_SCALE_NUM,                     \
        _rootNode->convertToWorldSpace(node->getPosition()),          \
        isLittle);

void GameDetail::changeFocus(EventKeyboard::KeyCode keyCode)
{
    Vec2Dir dir = LsTools::dirKeyCode2Vec2(keyCode);
    if (_state == ShowState::SCREEN_STATE) {
        switch (_focusNum) {
        case DetailFocusType::SCREEN_SHOT1: {
            _focusNum = DetailFocusType::SCREEN_SHOT2;
            _focus = LsTools::seekNodeByName(_rootNode, "3_screen_02");
            SCALE_NUM_FOCUS(_focus, true);
            createFullScreen();
            return;
        } break;
        case DetailFocusType::SCREEN_SHOT2: {
            _focusNum = DetailFocusType::SCREEN_SHOT1;
            _focus = LsTools::seekNodeByName(_rootNode, "3_screen_01");
            SCALE_NUM_FOCUS(_focus, true);
            createFullScreen();
            return;
        } break;
        default:
            break;
        }
    }

    NORMAL_NUM_FUCUS;

    Node* next = gameDetailGetNextFocus(keyCode);
    std::string name = (next ? next->getName() : "");
    LS_LOG("name : %s", name.c_str());
    LS_LOG("_focusNum: %d", _focusNum);
    if (next) {
        SCALE_NUM_FOCUS(next, true);
        if (_focusNum == DetailFocusType::GAME_ICON)
            runIconQR();

        nameRunAction(next);
        _focus = next;
    }
    else {
        LS_LOG("next null");
        ZM->showSelect(false);
    }
    LS_LOG("focus..");
}
void GameDetail::updateDownloadProg()
{
#ifdef USE_ANSY_CURLE
    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);

    auto downloadTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "8_leftDownload_Txt"));
    auto downloadProgRoot = static_cast<Node*>(LsTools::seekNodeByName(_rootNode, "downloadProgressRoot"));
    auto stopBG = LsTools::seekNodeByName(downloadProgRoot, "stopBG");
    auto progTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_downloadProgressRoot, "percentName"));
    stopBG->setVisible(false);
    progTxt->setVisible(true);

    switch (data->_apkState) {
    case DOWNLOAD_UNLOAD: //未下载
    case DOWNLOAD_UNDEFINE: //下载失败
    case DOWNLOAD_CANCEL: //取消下载
    {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("download"));
        downloadProgRoot->setVisible(false);
    } break;
    case DOWNLOAD_STOP: //暂停下载
    {
        stopBG->setVisible(true);
        progTxt->setVisible(false);
        downloadProgRoot->setVisible(true);
        downloadTxt->setString(DataManager::getInstance()->valueForKey("continue"));
    } break;
    case DOWNLOAD_ING: //下载中
    {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
        downloadProgRoot->setVisible(true);
    } break;
    case DOWNLOAD_SUCCESS: //下载成功
    {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("install"));
        downloadProgRoot->setVisible(false);
    } break;
    case DOWNLOAD_OPEN: //安装成功
    {
        downloadTxt->setString(DataManager::getInstance()->valueForKey("open"));
        downloadProgRoot->setVisible(false);
    } break;
    }

    if (_downloadProgressRoot) {
        auto prog = static_cast<ProgressTimer*>(LsTools::seekNodeByName(_downloadProgressRoot, "loadingPrograss"));
        prog->setPercentage(LsTools::getDownloadPercent(data->_size, _gamePackage));
        int temp = (int)prog->getPercentage();
        progTxt->setString(int2str(temp) + std::string("%"));
    }
#endif
}

//以下这段代码写的太烂了，我都醉了，以后想到好的方法再来优化吧。
Node* GameDetail::gameDetailGetNextFocus(EventKeyboard::KeyCode keyCode)
{
    Vec2Dir dir = LsTools::dirKeyCode2Vec2(keyCode);
    if (_focus) { //有焦点
        switch (_focusNum) {
        case DetailFocusType::DL_BTN: {
            if (dir.equals(VEC2_RIGTH)) //right
            {
                _focusNum = DetailFocusType::COLLECT;
                return LsTools::seekNodeByName(_rootNode, "9_leftFav_bg");
            }
            else if (dir.equals(VEC2_UP)) //up
            {
                _focusNum = DetailFocusType::GAME_ICON;
                return LsTools::seekNodeByName(_rootNode, "7_leftIcon");
            }
            else {
                _focusNum = DetailFocusType::DL_BTN;
                return LsTools::seekNodeByName(_rootNode, "8_leftDownload");
            }
        } break;
        case DetailFocusType::COLLECT: {
            if (dir.equals(VEC2_LEFT)) //left
            {
                _focusNum = DetailFocusType::DL_BTN;
                return LsTools::seekNodeByName(_rootNode, "8_leftDownload");
            }
            else if (dir.equals(VEC2_RIGTH)) //right
            {
                _focusNum = DetailFocusType::SCREEN_SHOT1;
                return LsTools::seekNodeByName(_rootNode, "3_screen_01");
            }
            else if (dir.equals(VEC2_UP)) //up
            {
                _focusNum = DetailFocusType::GAME_ICON;
                return LsTools::seekNodeByName(_rootNode, "7_leftIcon");
            }
            else {
                _focusNum = DetailFocusType::COLLECT;
                return LsTools::seekNodeByName(_rootNode, "9_leftFav_bg");
            }
        } break;
        case DetailFocusType::SCREEN_SHOT1: {
            if (dir.equals(VEC2_LEFT)) //left
            {
                _focusNum = DetailFocusType::COLLECT;
                return LsTools::seekNodeByName(_rootNode, "9_leftFav_bg");
            }
            else if (dir.equals(VEC2_RIGTH)) //right
            {
                _focusNum = DetailFocusType::SCREEN_SHOT2;
                return LsTools::seekNodeByName(_rootNode, "3_screen_02");
            }
            else if (dir.equals(VEC2_DOWN)) //down
            {
                _focusNum = DetailFocusType::RECOMMEND;
                //favData.size() < 0 is always false
                /*
				std::vector< GameDetailData >& favData = DataManager::getInstance()->getDetailGameByType(int2str(DataManager::getInstance()->_sUserData.category));
				if (favData.size() < 0)
				{
					_focusNum = DetailFocusType::SCREEN_SHOT2;
					return _focus;
				}
                */

                return LsTools::seekNodeByName(_rootNode, "6_game_01");
            }
            else {
                _focusNum = DetailFocusType::SCREEN_SHOT1;
                return LsTools::seekNodeByName(_rootNode, "3_screen_01");
            }
        } break;
        case DetailFocusType::SCREEN_SHOT2: {
            if (dir.equals(VEC2_LEFT)) //left
            {
                _focusNum = DetailFocusType::SCREEN_SHOT1;
                return LsTools::seekNodeByName(_rootNode, "3_screen_01");
            }
            else if (dir.equals(VEC2_DOWN)) //down
            {
                _focusNum = DetailFocusType::RECOMMEND;
                std::vector<GameDetailData>& favData = DataManager::getInstance()->getDetailGameByType(int2str(DataManager::getInstance()->_sUserData.category));
                //favData.size() < 0 is always false
                /*
				if (favData.size() < 0)
				{
					_focusNum = DetailFocusType::SCREEN_SHOT2;
					return _focus;
				}
                 */
                if (favData.size() < 4)
                    return LsTools::seekNodeByName(_rootNode, "6_game_01");
                else
                    return LsTools::seekNodeByName(_rootNode, "6_game_04");
            }
            else {
                _focusNum = DetailFocusType::SCREEN_SHOT2;
                return LsTools::seekNodeByName(_rootNode, "3_screen_02");
            }
        } break;
        case DetailFocusType::RECOMMEND: {
            std::string fName = _focus->getName();
            int id = (int)atoi(fName.substr(fName.size() - 1, 1).c_str());
            if (dir.equals(VEC2_LEFT)) //left
            {
                _focusNum = DetailFocusType::RECOMMEND;
                if (id < 3)
                    return LsTools::seekNodeByName(_rootNode, "6_game_01");
                else
                    return LsTools::seekNodeByName(_rootNode, std::string("6_game_0") + int2str(id - 1));
            }
            else if (dir.equals(VEC2_RIGTH)) //right
            {
                std::vector<GameDetailData>& favData = DataManager::getInstance()->getDetailGameByType(int2str(DataManager::getInstance()->_sUserData.category));
                _focusNum = DetailFocusType::RECOMMEND;
                if (id + 1 > (int)favData.size() - 1)
                    return _focus;
                if (id < 6)
                    return LsTools::seekNodeByName(_rootNode, std::string("6_game_0") + int2str(id + 1));
                else
                    return LsTools::seekNodeByName(_rootNode, std::string("6_game_06"));
            }
            else if (dir.equals(VEC2_UP)) //up
            {
                if (id < 4) {
                    _focusNum = DetailFocusType::SCREEN_SHOT1;
                    return LsTools::seekNodeByName(_rootNode, "3_screen_01");
                }
                else {
                    _focusNum = DetailFocusType::SCREEN_SHOT2;
                    return LsTools::seekNodeByName(_rootNode, "3_screen_02");
                }
            }
            else {
                _focusNum = DetailFocusType::RECOMMEND;
                return _focus;
            }
        } break;
        case DetailFocusType::GAME_ICON: {
            if (dir.equals(VEC2_DOWN)) //down
            {
                _focusNum = DetailFocusType::DL_BTN;
                return LsTools::seekNodeByName(_rootNode, "8_leftDownload");
            }
            else {
                return LsTools::seekNodeByName(_rootNode, "7_leftIcon");
            }
        } break;
        default:
            break;
        }
    }
    else { //无焦点，默认从简介控件开始
        _focusNum = DetailFocusType::DL_BTN;
        return LsTools::seekNodeByName(_rootNode, "8_leftDownload");
    }
    return nullptr;
}

void GameDetail::clickOkItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type != ui::Widget::TouchEventType::ENDED)
        return;
    if (_state == ShowState::SCREEN_STATE) {
        backNormalState();
        return;
    }

    _focus = static_cast<Node*>(sender);
    if (!_focus)
        return;

    switch (_focusNum) {
    case DetailFocusType::DL_BTN:
        clickDownloadButton();
        break;
    case DetailFocusType::COLLECT: {
        auto data = DataManager::getInstance()->getDetailGameByPackage(DataManager::getInstance()->_sUserData.gamePackageName);
        data->_isCollect = (data->_isCollect + 1) % 2;
        DataManager::getInstance()->flushDetailData();
        auto favImg = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "9_leftFav_bg_coll"));
        favImg->loadTexture(formatStr("Favorite0%d.png", 2 + data->_isCollect));
    } break;
    case DetailFocusType::SCREEN_SHOT1:
    case DetailFocusType::SCREEN_SHOT2:
        createFullScreen();
        break;
    case DetailFocusType::RECOMMEND: {
        LS_LOG("fav");
        if (_focus->getUserData()) {
            GameDetailData* temp = static_cast<GameDetailData*>(_focus->getUserData());
            DataManager::getInstance()->_sUserData.gamePackageName = temp->_basic.packageName;
            DataManager::getInstance()->flushUserData();
            GCatetoryDetailLayer::showGameDetail();
        }
    } break;
    case DetailFocusType::GAME_ICON:
        runIconQR();
        break;
    default:
        break;
    }
}

void GameDetail::clickOkItemByTouch(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type != ui::Widget::TouchEventType::ENDED)
        return;

    if (_state == ShowState::SCREEN_STATE) {
        backNormalState();
        return;
    }

    auto btn = static_cast<Node*>(sender);
    if (!btn)
        return;

    if (btn->getName() == "8_leftDownload") //download ui
    {
        if (_focus == btn) {
            clickOkItem(_focus, type);
            return;
        }
        _focusNum = DetailFocusType::DL_BTN;
        NORMAL_NUM_FUCUS;
        _focus = btn;
        SCALE_NUM_FOCUS(_focus, true);
    }
    else if (btn->getName() == "9_leftFav_bg_coll") //collect ui
    {
        if (_focus && _focus->getName() == "9_leftFav_bg") {
            clickOkItem(_focus, type);
            return;
        }
        _focusNum = DetailFocusType::COLLECT;
        NORMAL_NUM_FUCUS;
        _focus = btn->getParent();
        SCALE_NUM_FOCUS(_focus, true);
    }
    else if (btn->getName() == "screenShot_0") //screen 01
    {
        if (_focus && _focus->getName() == "3_screen_01") {
            clickOkItem(_focus, type);
            return;
        }
        _focusNum = DetailFocusType::SCREEN_SHOT1;
        NORMAL_NUM_FUCUS;
        _focus = btn->getParent();
        SCALE_NUM_FOCUS(_focus, true);
    }
    else if (btn->getName() == "screenShot_1") //screen 02
    {
        if (_focus && _focus->getName() == "3_screen_02") {
            clickOkItem(_focus, type);
            return;
        }
        _focusNum = DetailFocusType::SCREEN_SHOT2;
        NORMAL_NUM_FUCUS;
        _focus = btn->getParent();
        SCALE_NUM_FOCUS(_focus, true);
    }
    else if (btn->getName().substr(0, 8) == "6_game_0") //faver node
    {
        if (_focus == btn) {
            clickOkItem(_focus, type);
            return;
        }
        _focusNum = DetailFocusType::RECOMMEND;
        NORMAL_NUM_FUCUS;
        _focus = btn;
        SCALE_NUM_FOCUS(_focus, true);
    }
    else if (btn->getName() == "7_leftIcon") //game icon
    {
        _focusNum = DetailFocusType::GAME_ICON;
        NORMAL_NUM_FUCUS;
        _focus = btn;
        SCALE_NUM_FOCUS(_focus, true);
        runIconQR();
    }
    nameRunAction(_focus);
}
#undef NORMAL_NUM_FUCUS
#undef SCALE_NUM_FOCUS

void GameDetail::nameRunAction(cocos2d::Node* node)
{
    if (!node)
        return;

    if (_focusName && _focusName->getContentSize().width > 184) {
        _focusName->stopAllActions();
        _focusName->setPositionX(_nameExtWidth);
    }

    auto name = static_cast<ui::Text*>(LsTools::seekNodeByName(node, "gameName"));
    if (name && name->getContentSize().width > 184) {
        float x = name->getPositionX();
        _nameExtWidth = x;
        auto moveBy1 = MoveBy::create(3, Vec2(-2 * x, 0));
        auto moveBy2 = MoveBy::create(3, Vec2(2 * x, 0));
        auto timeDelay = DelayTime::create(2);
        auto seq = Sequence::create(moveBy1, timeDelay, moveBy2, timeDelay, nullptr);
        name->runAction(RepeatForever::create(seq));
        _focusName = name;
    }
}

void GameDetail::createDownloadProg(cocos2d::Vec2 pos)
{
    auto iconNode = LsTools::seekNodeByName(_rootNode, "7_leftIcon");
    _downloadProgressRoot = Node::create();
    _downloadProgressRoot->setName("downloadProgressRoot");
    _downloadProgressRoot->setPosition(pos);
    iconNode->addChild(_downloadProgressRoot, 1);

    auto back = ui::ImageView::create("download_black.png");
    back->setName("Icon_Black");
    _downloadProgressRoot->addChild(back);

    auto prograssBG = Sprite::create("progress01.png");
    prograssBG->setAnchorPoint(Vec2(0.5f, 0.5f));
    prograssBG->setName("LoadingPrograssBG");
    _downloadProgressRoot->addChild(prograssBG);

    auto prograss = ProgressTimer::create(Sprite::create("progress02.png"));
    prograss->setAnchorPoint(Vec2(0.5f, 0.5f));
    prograss->setName("loadingPrograss");
    prograss->setPercentage(0);
    _downloadProgressRoot->addChild(prograss);

    auto percentTxt = cocos2d::ui::Text::create("0%", s_labelTTF, 36);
    percentTxt->setAnchorPoint(Vec2(0.5f, 0.5f));
    percentTxt->setName("percentName");
    _downloadProgressRoot->addChild(percentTxt);

    auto stopBG = cocos2d::ui::ImageView::create("progress03.png");
    stopBG->setName("stopBG");
    stopBG->setVisible(false);
    _downloadProgressRoot->addChild(stopBG);

    _downloadProgressRoot->setVisible(false);
}

void GameDetail::createFullScreen()
{
    if (FileUtils::getInstance()->isFileExist(formatStr("%s.mp4", _gamePackage.c_str())) || _isHaveVideo) {
        ZM->playVideo(_gamePackage);
        return;
    }

    GameDetailData* data = DataManager::getInstance()->getDetailGameByPackage(_gamePackage);
    if (!data || !_focus)
        return;

    std::vector<std::string> scrVec = LsTools::parStringByChar(data->_screenHot);
    std::string name = _focus->getName();
    int i = -1;
    if (name == "screenShot_0" || name == "3_screen_01")
        i = 0;
    else if (name == "screenShot_1" || name == "3_screen_02")
        i = 1;
    if (i == -1)
        return;

    std::string url = LsTools::lsStandardPath(std::string(s_GameScreenPath) + "/"
        + data->_basic.packageName + std::string("/") + scrVec[i]);
    if (!FileUtils::getInstance()->isFileExist(url))
        return;

    auto sprite = Sprite::create(url);
    if (!sprite)
        return;

    if (_focus) {
        ZM->showSelect(false);
        _focus->setScale(FOCUS_NORMAL_SCALE);
        _focus->setLocalZOrder(_focus->getLocalZOrder() - FOCUS_Z_ORDER_ADD);
    }

    _state = ShowState::SCREEN_STATE;

    if (!_gameBG) {
        _gameBG = Node::create();
        _gameBG->setName("gameBG");
        _rootNode->addChild(_gameBG, 100);
    }
    _gameBG->removeAllChildren();

    auto bg = LayerColor::create(Color4B(0, 0, 0, 200), MY_SCREEN.width, MY_SCREEN.height);
    float minScale = 1280.f / sprite->getContentSize().width;
    sprite->setScale(minScale);
    sprite->setPosition(MY_SCREEN_CENTER);

    _gameBG->addChild(bg);
    _gameBG->addChild(sprite);

    //    auto vp = cocos2d::experimental::ui::VideoPlayer::create();
    //    vp->setPosition(Vec2(MY_SCREEN_CENTER.x, MY_SCREEN_CENTER.y));
    //    //_videoPlayer->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    //    vp->setFullScreenEnabled(true);
    //    vp->setFileName("com.pastagames.ro1mobile.mp4");
    //    //_videoPlayer->setTouchEnabled(false);
    //    //_videoPlayer->setKeepAspectRatioEnabled(!_videoPlayer->isKeepAspectRatioEnabled());
    //    vp->setContentSize(Size(MY_SCREEN.width, MY_SCREEN.height));
    //
    //    //_videoPlayer->setContentSize(Size(MY_SCREEN.width, MY_SCREEN.height
    //    _gameBG->addChild(vp);
}

void GameDetail::backNormalState()
{
    if (_gameBG)
        _gameBG->removeAllChildren();
    _state = ShowState::NORMAL_STATE;
    if (_focus) {
        _focus->setScale(FOCUS_SCALE_NUM);
        _focus->setLocalZOrder(_focus->getLocalZOrder() + FOCUS_Z_ORDER_ADD);
        ZM->showSelect(true,
            _focus->getContentSize() * FOCUS_SCALE_NUM,
            _rootNode->convertToWorldSpace(_focus->getPosition()),
            true);
    }
}

//init
void GameDetail::initScreenNode(GameDetailData* data)
{
    std::vector<std::string> scrVec = LsTools::parStringByChar(data->_screenHot);
    std::string sten = "Screenshot.png";
    if (scrVec.size() < 2) {
        LS_LOG("scrVec.size() < 2");
    }

    for (unsigned int i = 0; i < scrVec.size() && i < 2; ++i) {
        std::string url = LsTools::lsStandardPath(std::string(s_GameScreenPath) + "/"
            + data->_basic.packageName + std::string("/") + scrVec[i]);
        if (!FileUtils::getInstance()->isFileExist(url)) {
            downloadScreenShoot(); //有避免多次下载，可以直接使用
            url = "com_idle02.png";
        }

        auto stenNode = Sprite::create(sten);
        auto clipNode = ClippingNode::create(stenNode);
        auto imgNode = ui::ImageView::create(url);
        imgNode->setTouchEnabled(true);
        imgNode->addTouchEventListener(CC_CALLBACK_2(GameDetail::clickOkItemByTouch, this));
        auto downloadTxt = ui::Text::create(DataManager::getInstance()->valueForKey("loading"), "msyh.ttf", 27);
        downloadTxt->setColor(Color3B(127, 155, 193));
        downloadTxt->setName("downloadTips");
        downloadTxt->setPositionY(-132);
        if (url != "com_idle02.png")
            downloadTxt->setVisible(false);

        imgNode->setName(formatStr("screenShot_%d", i));
        clipNode->addChild(imgNode);
        clipNode->addChild(downloadTxt);
        clipNode->setName(std::string("3_screen_0") + std::string(int2str(i + 1)));
        float wSca = 651.0f / imgNode->getContentSize().width;
        float hSca = 454.0f / imgNode->getContentSize().height;
        float minScale = (wSca > hSca ? wSca : hSca);
        clipNode->setContentSize(Size(651, 454));
        imgNode->setScale(minScale);
        Vec2 pos = Vec2(795.5f + i * 668, 550);
        clipNode->setPosition(pos);
        clipNode->setAlphaThreshold(0.1f);
        _rootNode->addChild(clipNode);

        auto videoFlag = Sprite::create("Home_Push_Play.png");
        videoFlag->setPosition(Vec2(imgNode->getContentSize().width / 2.0f, imgNode->getContentSize().height / 2.0f));
        videoFlag->setVisible(_isHaveVideo);
        videoFlag->setName("videoPlay");
        videoFlag->setScale(1 / minScale);
        imgNode->addChild(videoFlag);
        _videoPlayFlagV.pushBack(videoFlag);
    }
}
void GameDetail::initFavNode()
{
    std::vector<GameDetailData>& favData = DataManager::getInstance()->getDetailGameByType(int2str(DataManager::getInstance()->_sUserData.category));
    auto favRootNode = Node::create();
    favRootNode->setName("6_game");
    _rootNode->addChild(favRootNode);
    for (unsigned int i = 0, j = 0; j < 6 && i < favData.size(); ++i) {
        GameDetailData tempData = favData[i];
        if (tempData._basic.packageName == _gamePackage)
            continue;

        auto backIcon = ui::ImageView::create("Icon_MyGame.png");
        backIcon->setName(std::string("6_game_0") + std::string(int2str(j + 1)));
        backIcon->setScale9Enabled(true);
        backIcon->setCapInsets(Rect(
            SELECT_CAP_SIZE,
            SELECT_CAP_SIZE,
            backIcon->getContentSize().width - 2 * SELECT_CAP_SIZE,
            backIcon->getContentSize().height - 2 * SELECT_CAP_SIZE));
        backIcon->setAnchorPoint(Vec2(0.5f, 0.5f));
        backIcon->setContentSize(Size(184, 218));
        backIcon->setUserData(&favData[i]);

        std::string iconPath = tempData._basic._icon;
        if (!FileUtils::getInstance()->isFileExist(FileUtils::getInstance()->fullPathForFilename(iconPath))) {
            iconPath = s_DefaultIcon;
            NetManager::getInstance()->loadFile(ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_ICON, tempData._basic.packageName.c_str()),
                LsTools::lsStandardPath(formatStr("%s/%s/", LsTools::getDataPath().c_str(), s_GameIconPath)),
                tempData._basic.packageName + ".png",
                [&] {
                    ZM->flushUI(false);
                },
                nullptr, false);
        }

        auto imgNode = ui::ImageView::create(iconPath);
        imgNode->setPosition(Vec2(92, 34 + 92));
        imgNode->setContentSize(Size(184, 184));
        imgNode->ignoreContentAdaptWithSize(false);
        imgNode->setName(formatStr("favIcon_%d", j));

        Vec2 pos = Vec2(578.5f + j * 218.5f, 182 - 17);
        backIcon->setPosition(pos);
        backIcon->addChild(imgNode);

        auto tName = ui::Text::create(tempData._basic._name, s_labelTTF, 24);
        tName->setAnchorPoint(Vec2(0.5f, 0.5f));
        tName->setName("gameName");

        if (tName->getContentSize().width > 184) //名字超出背景框，那么就左右移动名字
        {
            auto sten = Sprite::create("Icon_GameDetail_txt.png");
            auto nClip = ClippingNode::create(sten);
            nClip->setPosition(Vec2(92, backIcon->getBottomBoundary() - 34));
            nClip->addChild(tName);
            backIcon->addChild(nClip);

            float tmp = (tName->getContentSize().width - 184) / 2.0f;
            tmp += 10;
            tName->setPositionX(tmp);
        }
        else {
            tName->setPosition(Vec2(92, backIcon->getBottomBoundary() - 34));
            backIcon->addChild(tName);
        }

        backIcon->setTouchEnabled(true);
        backIcon->addTouchEventListener(CC_CALLBACK_2(GameDetail::clickOkItemByTouch, this));
        favRootNode->addChild(backIcon);

        ++j;
    }
}

void GameDetail::initICON(GameDetailData* data)
{
    std::string iconPath = data->_basic._icon;
    if (!FileUtils::getInstance()->isFileExist(iconPath)) {
        iconPath = s_DefaultIcon;
        NetManager::getInstance()->loadFile(ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_ICON, data->_basic.packageName.c_str()),
            LsTools::lsStandardPath(formatStr("%s/%s/", LsTools::getDataPath().c_str(), s_GameIconPath)),
            data->_basic.packageName + ".png",
            [&] {
                ZM->flushUI(false);
            },
            nullptr, false);
    }

    auto imgNode = ui::ImageView::create(iconPath);
    imgNode->setContentSize(Size(270, 270));
    imgNode->ignoreContentAdaptWithSize(false);
    imgNode->setPosition(Vec2(273, 861));
    imgNode->setName(std::string("7_leftIcon"));
    imgNode->setTouchEnabled(true);
    imgNode->addTouchEventListener(CC_CALLBACK_2(GameDetail::clickOkItemByTouch, this));

    _rootNode->addChild(imgNode);
    //create prograss node
    _iconNode = imgNode;
    createDownloadProg(Vec2(135, 135));
}
void GameDetail::initGameInfor(GameDetailData* data)
{
    auto scoTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "10_leftSco"));
    scoTex->setString(data->_score);

    auto typeTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "11_leftType"));
    std::vector<std::string> typeDes = LsTools::parStringByChar(data->_type);
    std::string typeList = "";
    for (unsigned int i = 0; i < typeDes.size() && i < 1; ++i) {
        if (i == typeDes.size() - 1)
            typeList += std::string(DataManager::getInstance()->valueForKey(typeDes[i]));
        else
            typeList += std::string(DataManager::getInstance()->valueForKey(typeDes[i])) + std::string(";");
    }
    auto netModel = std::string(DataManager::getInstance()->valueForKey(data->_network));
    auto dunStr = std::string(DataManager::getInstance()->valueForKey("dun"));
    typeTex->setString(DataManager::getInstance()->valueForKey(typeDes[0]) + dunStr + netModel);

    auto sizeTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "12_leftSize"));
    std::vector<std::string> svc = LsTools::parStringByChar(data->_size);
    long sizeTmp = atol(svc[0].c_str());
    if (svc.size() > 1)
        sizeTmp += atol(svc[1].c_str());
    float st = (float)(sizeTmp) / (1024.f * 1024.f);
    sizeTex->setString(float2str2(st, 1) + std::string("M"));

    auto versionTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "13_leftVersion"));
    std::string version = data->_version;
    if (version.size() > 0) {
        if (!(version[0] >= '0' && version[0] <= '9')) {
            version = version.substr(1, version.size());
        }
    }
    versionTex->setString(version);

    auto languageTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "14_leftLanguage"));
    std::string language = "";
    std::vector<std::string> des = LsTools::parStringByChar(data->_language);
    for (unsigned int i = 0; i < des.size(); ++i) {
        if (i == des.size() - 1)
            language += std::string(DataManager::getInstance()->valueForKey(des[i]));
        else
            language += std::string(DataManager::getInstance()->valueForKey(des[i])) + std::string("、");
    }
    languageTex->setString(language);

    auto downloadNumTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "15_leftDownloadNum"));
    std::string str = data->_downloadNum + std::string(DataManager::getInstance()->valueForKey("ci"));
    downloadNumTex->setString(str);

    auto remoteBtn = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "16_remote"));
    auto joyBtn = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "16_joy"));
    auto controlBtn = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "16_control"));
    std::vector<std::string> state = LsTools::parStringByChar(data->_operModel);
    float basePosX = 172;
    for (unsigned int i = 0; i < state.size(); ++i) {
        if (state[i] == "sb02") { //模拟器
            remoteBtn->setPositionX(basePosX);
            remoteBtn->setVisible(true);
            basePosX += 57;
        }
        else if (state[i] == "sb00") { //手柄
            joyBtn->setPositionX(basePosX);
            joyBtn->setVisible(true);
            basePosX += 57;
        }
        else if (state[i] == "sb01") { //遥控器
            controlBtn->setPositionX(basePosX);
            controlBtn->setVisible(true);
            basePosX += 57;
        }
    }
    auto model = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "16_model"));
    model->setVisible(true);
    model->setPositionX(basePosX);
    if (data->_model == "ms00") { //单人
        model->loadTexture("State_single.png");
    }
    else { //多人
        model->loadTexture("State_multi.png");
    }

    auto nameTex = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "1_name"));
    nameTex->setString(data->_basic._name);
}
void GameDetail::initSummayText(GameDetailData* data)
{
    auto summary = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "summay"));
    summary->setString(data->_summary);
}

void GameDetail::initDownloadUI(GameDetailData* data)
{
    //	auto downloadTxt = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "8_leftDownload_Txt"));
    auto downloadBtn = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "8_leftDownload"));
    downloadBtn->addTouchEventListener(CC_CALLBACK_2(GameDetail::clickOkItemByTouch, this));
}

void GameDetail::initCollectNode(GameDetailData* data)
{
    //	auto favSprite = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "9_leftFav_bg"));
    auto favBtn = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "9_leftFav_bg_coll"));
    favBtn->addTouchEventListener(CC_CALLBACK_2(GameDetail::clickOkItemByTouch, this));
    auto favImg = static_cast<cocos2d::ui::ImageView*>(LsTools::seekNodeByName(this->getRootNode(), "9_leftFav_bg_coll"));
    favImg->loadTexture(formatStr("Favorite0%d.png", 2 + data->_isCollect));
}

void GameDetail::initQrencode(GameDetailData* data)
{
    auto nameNode = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "1_name"));
    //	auto sumNode = static_cast<cocos2d::ui::Text*>(LsTools::seekNodeByName(_rootNode, "2_summay"));
    std::string apkName = formatStr("%s_V_%s", _gamePackage.c_str(), data->_version.c_str());
    std::string str = ZM->getServerAddress() + "?" + formatStr("commend=%d&xDownload=%s", CommendEnum::DOWNLOAD_GAME, apkName.c_str());

    //二维码
    CQR_Encode qrEncode;
    float size = 6;
    Size qrSize;

    auto dr = LsTools::createQRAndDraw(qrEncode, size, LsTools::str2charp(str), qrSize);
    dr->setPosition(qrSize.width * -0.5f, qrSize.height * 0.5f);
    auto drParent = Node::create();
    drParent->addChild(dr);
    //drParent->setScale(30.0f / qrEncode.m_nSymbleSize);
    //drParent->setPosition(Vec2(1463 + 651.0f / 2.0f - 6 * 31, nameNode->getTopBoundary()));
    drParent->setScale(_iconNode->getContentSize().width / qrSize.width);
    drParent->setPosition(_iconNode->getPosition());

    auto tip = ui::Text::create(DataManager::getInstance()->valueForKey("encode"), s_labelTTF, 20);
    tip->setAnchorPoint(Vec2(0.5f, 1));
    tip->setPosition(Vec2(drParent->getPositionX() + 3 * 31, nameNode->getTopBoundary() - 6 * 31 - 5));
    tip->setName("encodeName");
    tip->setVisible(false);
    _rootNode->addChild(tip);
    _rootNode->addChild(drParent);

    _drawNode = drParent;
    _drawNode->setVisible(false);
}

void GameDetail::runIconQR()
{
    float duration = 1.0f;
    float dr = 53;
    auto actionIn = Sequence::create(DelayTime::create(duration * 0.5f),
        Show::create(),
        RotateTo::create(duration * 0.5f, Vec3(0, 0, 0)),
        //OrbitCamera::create(duration * 0.5f, 1, 0, kInAngleZ, kInDeltaZ, 0, 0),
        nullptr);

    auto actionOut = Sequence::create(
        RotateTo::create(duration * 0.5f, Vec3(0, 360 - dr, 0)),
        //OrbitCamera::create(duration * 0.5f, 1, 0, kOutAngleZ, kOutDeltaZ, 0, 0),
        Hide::create(),
        DelayTime::create(duration * 0.5f),
        nullptr);

    _iconNode->stopAllActions();
    _drawNode->stopAllActions();
    auto inNode = _drawNode->isVisible() ? _iconNode : _drawNode;
    auto outNode = _drawNode->isVisible() ? _drawNode : _iconNode;
    outNode->setRotation3D(Vec3(0, 0, 0));
    inNode->setRotation3D(Vec3(0, 180 - dr, 0));
    outNode->runAction(actionOut);
    inNode->runAction(actionIn);
}
//
//  ZoneManager.cpp
//  xZone
//
//  Created by Sharezer on 15/1/19.
//
//

#include "ZoneManager.h"
#include "BaseLayer.h"
#include "LsTools.h"
#include "DataManager.h"
#include "MainLayer.h"
#include "GameDetail.h"
#include "GCategoryDetail.h"
#include "MyGame.h"
#include "DLControl.h"
#include "Upgrade.h"
#include "GameList.h"
#include "Setting.h"
#include "About.h"
#include "UIPageViewEx.h"

#include "ContentTo.h"
#include "PlatformHelper.h"
#include "base/CCGameController.h"
#include "extensions/cocos-ext.h"


USING_NS_CC;
USING_NS_GUI;

ZoneManager::ZoneManager()
    : _secondState(SecondState::NONE)
    , _subSecondState(SecondState::NONE)
    , _thirdState(ThirdState::NONE)
    , _scene(nullptr)
    , _main(nullptr)
    , _second(nullptr)
    , _third(nullptr)
    , _dialog(nullptr)
    , _select(nullptr)
    , _loading(nullptr)
    , _upgrade(nullptr)
    , _isLoading(false)
    , _keyCode(EventKeyboard::KeyCode::KEY_NONE)
    , _data(nullptr)
    , _time(nullptr)
    , _eventCD(0.0f)
    , _state(nullptr)
    , _customEventEnable(false)
    , _isChangeState(false)
    , _isFlushAppListWithJNI(false)
    , _jostickCount(0)
    , _base(nullptr)
    , _serverAddress("")
    , _inputSecretCode("")
    , _vpState(VideoPlayerState::NONE)
    , _pageViewEx(nullptr)
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    , _videoPlayer(nullptr)
#endif
{
    _josticks.clear();
}

ZoneManager::~ZoneManager() { _josticks.clear(); }

static ZoneManager* s_sharedZoneManager = NULL;

ZoneManager* ZoneManager::getInstance()
{
    if (!s_sharedZoneManager) {
        s_sharedZoneManager = new ZoneManager();
        CCASSERT(s_sharedZoneManager, "FATAL: Not enough memory");
        s_sharedZoneManager->init();
    }

    return s_sharedZoneManager;
}

void ZoneManager::destroyInstance() { CC_SAFE_DELETE(s_sharedZoneManager); }

bool ZoneManager::init()
{
    Director::getInstance()->getScheduler()->schedule(
        CC_SCHEDULE_SELECTOR(ZoneManager::onUpdate), this, 0, false);

    // Director::getInstance()->getScheduler()->schedule(SEL_SCHEDULE(&ZoneManager::onUpdate),
    // s_sharedZoneManager, 0.1f, false);
    // Director::getInstance()->getScheduler()->scheduleUpdateForTarget( this, 0,
    // false);
    // schedule(CC_SCHEDULE_SELECTOR(MainLayer::updateTime));
    initEventListener();

    return true;
}

void ZoneManager::appRun()
{
    auto scene = Scene::create();
    Director::getInstance()->runWithScene(scene);
    _scene = scene;

    _base = Layer::create();
    _scene->addChild(_base);

    DataManager::getInstance()->initChinese();
    auto loading = Loading::create();
    _base->addChild(loading, static_cast<int>(LayerType::LOADING));
    _loading = loading;
    showLoading(false);

    auto dialog = Dialog::create();
    _base->addChild(dialog, static_cast<int>(LayerType::DIALOG));
    dialog->setVisible(false);
    _dialog = dialog;

    showSelect(false);

    auto layer = Upgrade::create();
    _base->addChild(layer, static_cast<int>(LayerType::UPGRADE));
    _upgrade = layer;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//    initVideo();
#endif
}

void ZoneManager::reUpgrade()
{
    _base->removeAllChildren();

    auto layer = Upgrade::create();
    _base->addChild(layer, static_cast<int>(LayerType::UPGRADE));
}

void ZoneManager::initMainLayer()
{
    ZM->_isLoading = false;
    ZM_LOADING->showBG(true);
    ZM_LOADING->_loadSprite->setVisible(true);
    ZM_LOADING->setLoadingBarVisiable(false);

    LsTools::resetRand();
    LsTools::readJsonWithFile("config.json", _config);
    DataManager::getInstance()->initData();

    if (_main) {
        _main->removeFromParent();
        _main = nullptr;
    }
    if (_second) {
        _second->removeFromParent();
        _second = nullptr;
    }
    if (_third) {
        _third->removeFromParent();
        _third = nullptr;
    }

    _main = MainLayer::create();
    _base->addChild(_main, static_cast<int>(LayerType::MAIN));

    auto state = CSLoader::createNode("State.csb");
    _base->addChild(state, static_cast<int>(LayerType::STATE));
    _state = state;
    _time = dynamic_cast<Text*>(LsTools::seekNodeByName(state, "time"));
    _data = dynamic_cast<Text*>(LsTools::seekNodeByName(state, "date"));
    onUpdateState(0);
    Director::getInstance()->getScheduler()->schedule(
        CC_SCHEDULE_SELECTOR(ZoneManager::onUpdateState), this, 3.0f, false);

    if (_upgrade) {
        _upgrade->removeFromParent();
        _upgrade = nullptr;
    }
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    auto btn = ui::Button::create("b1.png", "b2.png");
    btn->setPosition(Vec2(100, 100));
    btn->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type) {
        if (type != ui::Widget::TouchEventType::ENDED)
            return;
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_BACK, nullptr);
    });
    _scene->addChild(btn, 99);
#endif
}

bool ZoneManager::checkIsEventWork()
{
    if ((!_customEventEnable && _upgrade) || _eventCD > 0.00001 || _isChangeState) {
        LS_LOG("event cd");
        return false;
    }

    _eventCD = s_eventCD;
    return true;
}

void ZoneManager::initEventListener()
{
    //µ„ª˜ ¬º˛
    {
        auto listener = EventListenerTouchOneByOne::create();
        listener->setSwallowTouches(true);
        listener->onTouchBegan = [&](Touch* touch, Event* event) {
            // TODO : ‘› ±∆¡±Œ¥•∆¡ ¬º˛
            LS_LOG("ZM touch began");
            if (!checkIsEventWork())
                return true;
            return false;
        };
        Director::getInstance()
            ->getEventDispatcher()
            ->addEventListenerWithFixedPriority(listener, -1);
    }
    //º¸≈ÃªÚ“£øÿ∆˜ ¬º˛
    {
        auto listener = EventListenerKeyboard::create();
        listener->onKeyPressed = CC_CALLBACK_2(ZoneManager::onKeyPressed, this);
        listener->onKeyReleased = CC_CALLBACK_2(ZoneManager::onKeyReleased, this);

        //_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
        Director::getInstance()
            ->getEventDispatcher()
            ->addEventListenerWithFixedPriority(listener, 1);
    }
    // ÷±˙ ¬º˛
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        auto listener = EventListenerController::create();

        listener->onConnected = CC_CALLBACK_2(ZoneManager::onConnectController, this);
        listener->onDisconnected = CC_CALLBACK_2(ZoneManager::onDisconnectedController, this);
        listener->onKeyDown = CC_CALLBACK_3(ZoneManager::onKeyDown, this);
        listener->onKeyUp = CC_CALLBACK_3(ZoneManager::onKeyUp, this);
        listener->onAxisEvent = CC_CALLBACK_3(ZoneManager::onAxisEvent, this);

        //_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
        Director::getInstance()
            ->getEventDispatcher()
            ->addEventListenerWithFixedPriority(listener, 1);
        Controller::startDiscoveryController();
#endif
    }
}

void ZoneManager::changeSecondState(SecondState state, bool isRunAction)
{
    _isChangeState = true;
    showSelect(false);
    if (_second) {
        _second->removeFromParent();
        _second = nullptr;
        _subSecondState = _secondState;
        _pageViewEx = nullptr;
        if (_third)
            changeThirdState(ThirdState::NONE);
    }

    switch (state) {
    case SecondState::NONE:
        break;

    case SecondState::CATEGORY_DETAIL:
        _second = GCatetoryDetailLayer::create(
            DataManager::getInstance()->_sUserData.category);
        break;
    case SecondState::MY_GAME:
        _second = MyGame::create();
        break;
    case SecondState::COLLECT_GAME:
        _second = GameList::create(ListModel::COLLECT_GAME);
        break;
    case SecondState::HOT_GAME:
        _second = GameList::create(ListModel::HOT_GAME);
        break;
    case SecondState::NEW_GAME:
        _second = GameList::create(ListModel::NEW_GAME);
        break;
    case SecondState::DOWNLOAD_CONTROL:
        _second = DLControl::create();
        break;
    case SecondState::SETTING:
        _second = Setting::create();
        break;
    case SecondState::ABOUNT:
        _second = About::create();
        break;
    default:
        break;
    }

    if (_second) {
        _base->addChild(_second, static_cast<int>(LayerType::SECOND));
        _second->setType(LayerType::SECOND);
        _secondState = state;

        _second->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        _second->setPosition(MY_SCREEN_CENTER);
        _second->_rootNode->setPosition(-MY_SCREEN_CENTER);
        if (isRunAction)
            LsTools::scaleZoomAction(
                _second, CHANGE_STATE_DURATION, CallFunc::create([&]() {
                    if (_secondState != SecondState::ABOUNT)
                        _second->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
                }));

        /*
            LsTools::rotoZoomAction(_second,
            CHANGE_STATE_DURATION * 2.0f,
            CallFunc::create([&](){
            _second->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN); }));
            */

        showMain(false);
        // Director::getInstance()->getTextureCache()->removeUnusedTextures();
    }
    _isChangeState = false;
}

void ZoneManager::changeThirdState(ThirdState state, bool isRunAction)
{
    _isChangeState = true;
    showSelect(false);

    if (_third) {
        _third->removeFromParent();
        _third = nullptr;
        _pageViewEx = nullptr;
    }
    _thirdState = state;
    switch (state) {
    case ThirdState::NONE:
        break;
    case ThirdState::GAME_DETAIL:
        _third = GameDetail::create(
            DataManager::getInstance()->_sUserData.gamePackageName);
        break;
    case ThirdState::GAME_ADD:
        _third = MyGameAdd::create();
        break;
    default:
        break;
    };
    if (_third) {
        _base->addChild(_third, static_cast<int>(LayerType::THIRD));
        _third->setType(LayerType::THIRD);
        _thirdState = state;

        _third->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
        _third->setPosition(MY_SCREEN_CENTER);
        _third->_rootNode->setPosition(-MY_SCREEN_CENTER);
        if (isRunAction)
            LsTools::scaleZoomAction(
                _third, CHANGE_STATE_DURATION, CallFunc::create([&]() {
                    _third->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
                }));

        showMain(false);
        // Director::getInstance()->getTextureCache()->removeUnusedTextures();
    }
    _isChangeState = false;
}

void ZoneManager::flushUI(bool isChangeFocus)
{
    if (_thirdState != ThirdState::NONE && _third)
        _third->flushUI();

    if (_secondState != SecondState::NONE && _second)
        _second->flushUI();

    BaseLayer* work = nullptr;
    if (_thirdState != ThirdState::NONE)
        work = _third;
    else if (_secondState != SecondState::NONE)
        work = _second;
    else if (_main)
        work = _main;

    if (work && isChangeFocus)
        work->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
}

void ZoneManager::showMain(bool isShow)
{
    auto node = static_cast<MainLayer*>(_main);
    node->_hideNode->setVisible(isShow);
    if (isShow) {
        _main->showCurFocus();
        // Director::getInstance()->getTextureCache()->removeUnusedTextures();
    }
}

void ZoneManager::showSecond(bool isShow)
{
    if (_second)
        _second->setVisible(isShow);
    if (isShow)
        _second->showCurFocus();
}

void ZoneManager::showLoading(bool isShow)
{
    if (_loading)
        _loading->setVisible(isShow);
}

void ZoneManager::showSelect(bool show, Size size, Vec2 pos, bool isLittle)
{
    if (!_select) {
        _select = SelectBox::create();
        _select->setName("_select");
        _base->addChild(_select, static_cast<int>(LayerType::SELECT));
    }
    _select->showSelect(show, size, pos, isLittle);
}

/*
void ZoneManager::showSelectWithTarget(bool show, cocos2d::Node* targer, Size
size, Vec2 pos)
{
if (!_select) {
_select = SelectBox::create();
_base->addChild(_select, static_cast<int>(LayerType::SELECT));
}

_select->retain();
if (_select->getParent())
_select->removeFromParent();

_select->setName("_select");
targer->getParent()->addChild(_select, static_cast<int>(LayerType::SELECT));
_select->release();
if (pos == Vec2::ZERO)
_select->showSelect(show, targer->getContentSize() * FOCUS_SCALE_NUM,
targer->getPosition());
else
_select->showSelect(show, size, pos);
}
*/

void ZoneManager::onUpdate(float dt)
{
    _eventCD = _eventCD > 0 ? (_eventCD - dt) : 0;

    // bool isShowLoading = _isLoading || NM->_isLoadFile || NM->_isSendCommend ?
    // true : false;
    bool isShowLoading = _isLoading ? true : false;
    showLoading(isShowLoading);

    if (_isFlushAppListWithJNI) {
        _isFlushAppListWithJNI = false;
        DataManager::getInstance()->initAppList();
        ZM->flushUI(true);
    }
}

void ZoneManager::onUpdateState(float dt)
{
    if (!_state || !_state->isVisible())
        return;

    auto time = LsTools::getMyTime();
    _time->setString(formatStr("%02d:%02d", time.hour, time.min));
    _data->setString(formatStr(
        "%02d%s%02d%s\n%s", time.month,
        DataManager::getInstance()->valueForKey("month"), time.day,
        DataManager::getInstance()->valueForKey("day"),
        DataManager::getInstance()->valueForKey(formatStr("xq%d", time.wday))));

    flushState();
}

int stateToInt(bool state)
{
    int ret = state ? 2 : 1;
    return ret;
}

void ZoneManager::flushState()
{
    if (!_state || !_state->isVisible())
        return;

    Sprite* sprite = nullptr;
    auto stateNode = _state->getChildByName("state");
    if (!stateNode)
        return;
    std::string fileName = "";
    sprite = stateNode->getChildByName<Sprite*>("joystick");
    fileName = formatStr(
        "State_Joystick0%d.png",
        stateToInt(DataManager::getInstance()->_sUserData.joystickState));
    sprite->setTexture(fileName);
    sprite = stateNode->getChildByName<Sprite*>("remote");
    fileName = formatStr("State_Remote0%d.png",
        stateToInt(DataManager::getInstance()->_sUserData.remoteState));
    sprite->setTexture(fileName);
    sprite = stateNode->getChildByName<Sprite*>("net");
    fileName = formatStr("State_WIFI0%d.png",
        stateToInt(DataManager::getInstance()->_sUserData.netState));
    sprite->setTexture(fileName);
    sprite = stateNode->getChildByName<Sprite*>("download");
    sprite->setVisible(DataManager::getInstance()->_sUserData.downloadState);
}

void ZoneManager::onKeyDown(Controller* controller, int keyCode, Event* event)
{
    LS_LOG("%s", LsTools::jostickCode2Char(keyCode));
    switch (keyCode) {
    case Controller::Key::JOYSTICK_LEFT_X:
        break;
    case Controller::Key::JOYSTICK_LEFT_Y:
        break;
    case Controller::Key::JOYSTICK_RIGHT_X:
        break;
    case Controller::Key::JOYSTICK_RIGHT_Y:
        break;
    case Controller::Key::BUTTON_A: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_ENTER, nullptr);
    } break;
    case Controller::Key::BUTTON_B: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_BACK, nullptr);
    } break;
    case Controller::Key::BUTTON_X: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_ENTER, nullptr);
    } break;
    case Controller::Key::BUTTON_Y: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_MENU, nullptr);
    } break;
    case Controller::Key::BUTTON_DPAD_UP: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_DPAD_UP, nullptr);
    } break;
    case Controller::Key::BUTTON_DPAD_DOWN: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_DPAD_DOWN, nullptr);
    } break;
    case Controller::Key::BUTTON_DPAD_LEFT: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_DPAD_LEFT, nullptr);
    } break;
    case Controller::Key::BUTTON_DPAD_RIGHT: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_DPAD_RIGHT, nullptr);
    } break;
    case Controller::Key::BUTTON_DPAD_CENTER:
        break;
    case Controller::Key::BUTTON_LEFT_SHOULDER:
        break;
    case Controller::Key::BUTTON_RIGHT_SHOULDER:
        break;
    case Controller::Key::AXIS_LEFT_TRIGGER:
        break;
    case Controller::Key::AXIS_RIGHT_TRIGGER:
        break;
    case Controller::Key::BUTTON_LEFT_THUMBSTICK:
        break;
    case Controller::Key::BUTTON_RIGHT_THUMBSTICK:
        break;
    case Controller::Key::BUTTON_START: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_ENTER, nullptr);
    } break;
    case Controller::Key::BUTTON_SELECT: {
        this->onKeyReleased(EventKeyboard::KeyCode::KEY_BACK, nullptr);
    } break;
    default:
        break;
    }
}

void ZoneManager::onKeyUp(Controller* controller, int keyCode, Event* event)
{
    LS_LOG("keyCode : %d", keyCode);
}

void ZoneManager::onAxisEvent(Controller* controller, int keyCode,
    Event* event)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    const auto& ketStatus = controller->getKeyStatus(keyCode);
    switch (keyCode) {
    case Controller::Key::JOYSTICK_LEFT_X:
        LS_LOG("setPositionX : %f", ketStatus.value);
        break;
    case Controller::Key::JOYSTICK_LEFT_Y:
        LS_LOG("setPositionY : %f", -ketStatus.value);
        break;
    case Controller::Key::JOYSTICK_RIGHT_X:
        LS_LOG("setPositionX : %f", ketStatus.value);
        break;
    case Controller::Key::JOYSTICK_RIGHT_Y:
        LS_LOG("setPositionY : %f", ketStatus.value);
        break;
    case Controller::Key::AXIS_LEFT_TRIGGER:
        LS_LOG("setOpacity : %f", (200 * controller->getKeyStatus(keyCode).value));
        break;
    case Controller::Key::AXIS_RIGHT_TRIGGER:
        LS_LOG("setOpacity : %f", (200 * controller->getKeyStatus(keyCode).value));
        break;
    default:
        break;
    }
#endif
}

void ZoneManager::onConnectController(Controller* controller, Event* event)
{
    if (controller == nullptr || event == nullptr) {
        return;
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
// receive back key
// controller->receiveExternalKeyEvent(4, true);
// receive menu key
// controller->receiveExternalKeyEvent(82, true);
#endif
    _jostickCount++;
    bool isJostick = _jostickCount > 0 ? true : false;
    DataManager::getInstance()->_sUserData.joystickState = isJostick;
    LS_LOG("jostick count: %d", _jostickCount);
}

void ZoneManager::onDisconnectedController(Controller* controller,
    Event* event)
{
    if (controller == nullptr || event == nullptr)
        return;
    _jostickCount--;
    bool isJostick = _jostickCount > 0 ? true : false;
    DataManager::getInstance()->_sUserData.joystickState = isJostick;
    LS_LOG("jostick count: %d", _jostickCount);
}

void ZoneManager::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event) {}

void ZoneManager::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
    if (!checkIsEventWork())
        return;
    // LS_LOG("%s", LsTools::keyCode2Char(keyCode));

    if (isVideoPlay()) {
        if (keyCode == EventKeyboard::KeyCode::KEY_BACK || keyCode == EventKeyboard::KeyCode::KEY_BACKSPACE || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER || keyCode == EventKeyboard::KeyCode::KEY_DPAD_CENTER || keyCode == EventKeyboard::KeyCode::KEY_ENTER) {
            stopVideo();
        }
        return;
    }

    BaseLayer* layer = nullptr;
    if (_dialog->isVisible())
        layer = _dialog;
    else if (_loading->isVisible())
        layer = _loading;
    else if (_third)
        layer = _third;
    else if (_second)
        layer = _second;
    else
        layer = _main;

    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_KP_LEFT:
    case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
    case EventKeyboard::KeyCode::KEY_DPAD_LEFT: {
        _inputSecretCode += "L";
        layer->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_LEFT);
    } break;
    case EventKeyboard::KeyCode::KEY_KP_RIGHT:
    case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
    case EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
        _inputSecretCode += "R";
        layer->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_RIGHT);
    } break;
    case EventKeyboard::KeyCode::KEY_KP_UP:
    case EventKeyboard::KeyCode::KEY_UP_ARROW:
    case EventKeyboard::KeyCode::KEY_DPAD_UP: {
        _inputSecretCode += "U";
        layer->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_UP);
    } break;
    case EventKeyboard::KeyCode::KEY_KP_DOWN:
    case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
    case EventKeyboard::KeyCode::KEY_DPAD_DOWN: {
        _inputSecretCode += "D";
        layer->changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
    } break;
    case EventKeyboard::KeyCode::KEY_KP_ENTER:
    case EventKeyboard::KeyCode::KEY_DPAD_CENTER:
    case EventKeyboard::KeyCode::KEY_ENTER: {
        _inputSecretCode += "O";
        layer->onClickOK();
    } break;
    case EventKeyboard::KeyCode::KEY_F1:
    case EventKeyboard::KeyCode::KEY_MENU: {
        _inputSecretCode += "M";
        layer->onClickMenu();
    } break;
    case EventKeyboard::KeyCode::KEY_BACK:
    case EventKeyboard::KeyCode::KEY_BACKSPACE: {
        _inputSecretCode += "B";
        layer->onClickBack();
        _inputSecretCode = "";
    } break;
    default:
        _inputSecretCode = "";
        break;
    }
    _keyCode = keyCode;
    LsTools::playEffect(EffectType::BUTTON);
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//void ZoneManager::initVideo()
//{
//    _vpState = VideoPlayerState::NONE;
//    if (_videoPlayer) {
//        _videoPlayer->stop();
//        _videoPlayer->removeFromParent();
//        _videoPlayer = nullptr;
//    }
//
//}

void ZoneManager::videoEventCallback(
    Ref* sender, experimental::ui::VideoPlayer::EventType eventType)
{
    LS_LOG("videoEventCallback");
    switch (eventType) {
    case experimental::ui::VideoPlayer::EventType::PLAYING: {
        _vpState = VideoPlayerState::PLAYING;
        _isLoading = false;
        LS_LOG("PLAYING");
    } break;
    case experimental::ui::VideoPlayer::EventType::PAUSED: {
        _vpState = VideoPlayerState::PAUSE;
        LS_LOG("PAUSED");
    } break;
    case experimental::ui::VideoPlayer::EventType::STOPPED: {
        LS_LOG("STOPPED");
        _isLoading = false;
        _vpState = VideoPlayerState::STOP;
        stopVideo();
    } break;
    case experimental::ui::VideoPlayer::EventType::COMPLETED: {
        _vpState = VideoPlayerState::STOP;
        LS_LOG("COMPLETED");
        _isLoading = false;
        stopVideo();
    } break;
    default:
        break;
    }
}

#endif

void ZoneManager::playVideo(std::string package)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

    _isLoading = true;
    _videoPlayer = cocos2d::experimental::ui::VideoPlayer::create();
    _videoPlayer->setPosition(Vec2(MY_SCREEN_CENTER.x, MY_SCREEN_CENTER.y));
    _videoPlayer->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
    _videoPlayer->setFullScreenEnabled(false);
    _videoPlayer->setContentSize(Size(MY_SCREEN.width, MY_SCREEN.height));
    _base->addChild(_videoPlayer, static_cast<int>(LayerType::VIDEO));

    _videoPlayer->addEventListener(
        CC_CALLBACK_2(ZoneManager::videoEventCallback, this));

    if (FileUtils::getInstance()->isFileExist(
            formatStr("%s.mp4", package.c_str()))) {
        _videoPlayer->setFileName(formatStr("%s.mp4", package.c_str()));
        LS_LOG("setFileName");
    }
    else {
        _videoPlayer->setURL(ZM->getServerAddress() + "?" + formatStr("commend=%d&game=%s", CommendEnum::GET_VIDEO,
                                                                package.c_str()));
        LS_LOG("setURL");
    }

    _videoPlayer->setFullScreenEnabled(true);
    _videoPlayer->setPosition(Vec2(MY_SCREEN_CENTER.x, MY_SCREEN_CENTER.y));
    auto action = LsTools::delayAndCallFunc(0.3f, [=] { _videoPlayer
                                                            ->play(); });
    int actionTag = 1122;
    action->setTag(actionTag);
    _videoPlayer->stopAllActionsByTag(actionTag);
    _videoPlayer->runAction(action);
#endif
}

void ZoneManager::stopVideo()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    //    if (isVideoPlay())
    //        _videoPlayer->stop();
    _vpState = VideoPlayerState::NONE;
    if (_videoPlayer) {
        //        _videoPlayer->stop();
        _videoPlayer->removeFromParent();
        _videoPlayer = nullptr;
        _isLoading = false;
    }
#endif
}

bool ZoneManager::isVideoPlay()
{
    bool bRet = false;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    //    if (_videoPlayer && (_vpState == VideoPlayerState::PLAYING || _vpState == VideoPlayerState::PAUSE)) {
    //        bRet = true;
    //        LS_LOG("is video play");
    //    }
    if (_videoPlayer) {
        bRet = true;
        LS_LOG("is video play");
    }
#endif
    return bRet;
}
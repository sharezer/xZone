//
//  ZoneManager.h
//  xZone
//
//  Created by Sharezer on 15/1/19.
//
//

#ifndef __xZone__ZoneManager__
#define __xZone__ZoneManager__

#include "cocos2d.h"
#include "Global.h"
#include "base/CCGameController.h"
#include "json/document.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#define ZM			ZoneManager::getInstance()
#define ZM_LOADING	ZM->_loading
#define ZM_MAIN		ZM->_main
#define ZM_SECOND	ZM->_second
#define ZM_THIRD	ZM->_third
//#define ZM_UPGRADE	ZM->_upgrade
#define ZM_SCENE	ZM->_scene
#define ZM_SELECT	ZM->_select
#define ZM_DIALOG	ZM->_dialog
#define ZM_CONFIG	ZM->_config
#define ZM_STATE	ZM->_state
#define ZM_BASE		ZM->_base
#define ZM_PAGEVIEW ZM->_pageViewEx

class BaseLayer;
class Upgrade;
class Dialog;
class Loading;
class SelectBox;
class UIPageViewEx;
class ZoneManager : public cocos2d::Ref{
public:
    static ZoneManager* getInstance();
	static void destroyInstance();

    /**
     *  开始入口
     */
    void appRun();

	void reUpgrade();

    /**
     *  第二层切换状态
     *
     *  @param state <#state description#>
     */
	void changeSecondState(SecondState state, bool isRunAction = true);
	/**
     *  第三层切换状态
     *
     *  @param state <#state description#>
     */
    void changeThirdState(ThirdState state, bool isRunAction = true);

    void showMain(bool isShow);
	void showSecond(bool isShow);
	
    /**
     *  选择框
     *
     *  @param show 显示或隐藏
     */
    void showSelect(bool show, cocos2d::Size size = cocos2d::Size::ZERO, cocos2d::Vec2 pos = cocos2d::Vec2::ZERO, bool isLittle = false);
	//void showSelectWithTarget(bool show, cocos2d::Node* targer, cocos2d::Size size = cocos2d::Size::ZERO, cocos2d::Vec2 pos = cocos2d::Vec2::ZERO);
    void showDialog(const char* msg, const char* title) { cocos2d::MessageBox(msg, title); };

	void initMainLayer();
	void initEventListener();
	
	void flushState();
	void flushUI(bool isChangeFocus = false);
	void flushPageViewEx();

	void playVideo(std::string package);
	void stopVideo();
	bool isVideoPlay();

CC_CONSTRUCTOR_ACCESS:
    ZoneManager();
    ~ZoneManager();
    bool init();

private:
	void showLoading(bool isShow);
	void onUpdate(float dt);

	void onUpdateState(float dt);

	bool checkIsEventWork();
    /**
     *  手柄按下
     *
     *  @param controller <#controller description#>
     *  @param keyCode    <#keyCode description#>
     *  @param event      <#event description#>
     */
    void onKeyDown(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);
    /**
     *  手柄按钮抬起
     *
     *  @param controller <#controller description#>
     *  @param keyCode    <#keyCode description#>
     *  @param event      <#event description#>
     */
    void onKeyUp(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);
    /**
     *  手柄多发事件，一般用于摇杆
     *
     *  @param controller <#controller description#>
     *  @param keyCode    <#keyCode description#>
     *  @param event      <#event description#>
     */
    void onAxisEvent(cocos2d::Controller* controller, int keyCode, cocos2d::Event* event);
    /**
     *  手柄连接
     *
     *  @param controller <#controller description#>
     *  @param event      <#event description#>
     */
    void onConnectController(cocos2d::Controller* controller, cocos2d::Event* event);
    /**
     *  手柄断开连接
     *
     *  @param controller <#controller description#>
     *  @param event      <#event description#>
     */
    void onDisconnectedController(cocos2d::Controller* controller, cocos2d::Event* event);

    /**
     *  遥控器(键盘)事件（android没有）
     *
     *  @param keyCode <#keyCode description#>
     *  @param event   <#event description#>
     */
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);
    /**
     *  遥控器(键盘)事件
     *
     *  @param keyCode <#keyCode description#>
     *  @param event   <#event description#>
     */
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event);


#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
//	void initVideo();

	void videoEventCallback(cocos2d::Ref* sender, cocos2d::experimental::ui::VideoPlayer::EventType eventType);
	cocos2d::experimental::ui::VideoPlayer* _videoPlayer;
#endif

public:
    CC_SYNTHESIZE_READONLY(SecondState, _secondState, SecondState);
    CC_SYNTHESIZE_READONLY(ThirdState, _thirdState, ThirdState);
    CC_SYNTHESIZE_READONLY(SecondState, _subSecondState, SSState);
	CC_SYNTHESIZE(std::string, _serverAddress, ServerAddress);
	CC_SYNTHESIZE(bool, _customEventEnable, UsingCustomEventEnable);
	CC_SYNTHESIZE(bool, _isFlushAppListWithJNI, FlushAppListWithJNI);
	CC_SYNTHESIZE(std::string, _inputSecretCode, InputSecretCode);

    BaseLayer* _main;
    BaseLayer* _second;
    BaseLayer* _third;

    Dialog* _dialog;
	Loading* _loading;
	UIPageViewEx* _pageViewEx;
	Upgrade* _upgrade;
	cocos2d::Node* _state;

    cocos2d::Scene* _scene;
	cocos2d::Layer* _base;
    //cocos2d::Node* _select;
	SelectBox* _select;

	bool _isLoading;
	bool _isChangeState;
	cocos2d::EventKeyboard::KeyCode _keyCode;
	rapidjson::Document _config;
	std::vector<cocos2d::Controller*> _josticks;
protected:
	
	float _eventCD;
	
	int _jostickCount;
	cocos2d::ui::Text* _time;
	cocos2d::ui::Text* _data;
    
private:
    VideoPlayerState _vpState;
};

#endif /* defined(__xZone__ZoneManager__) */

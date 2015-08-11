//
//  MyGame.h
//  xZone
//
//  Created by Sharezer on 15/2/7.
//
//

#ifndef __xZone__MyGame__
#define __xZone__MyGame__

#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"

class UIPageViewEx;
class MyGame : public BaseLayer {
public:
    CREATE_FUNC(MyGame);
    
	virtual void onClickBack() override;
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
    
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	virtual void flushUI();
	void doFocusChange(cocos2d::EventKeyboard::KeyCode keyCode);
	void touchAction(cocos2d::Node* newFoucs);
	//根据包名从游戏列表移除游戏
	void deleteAppInList(std::string package);
	//更新游戏显示信息
	void updateGameTitle();
CC_CONSTRUCTOR_ACCESS:
    MyGame();
    virtual ~MyGame();
    virtual bool init() override;
private:
    virtual bool initUI() override;
    virtual bool initData() override;

    void clickItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void onUpdate(float dt);

	void pageViewEvent(cocos2d::Ref* sender, cocos2d::ui::PageView::EventType type);

	void resetSelect(bool isEdit);
private:
	UIPageViewEx* _cdPageView;
	std::vector<GameBasicData> _vectorData;
	cocos2d::EventKeyboard::KeyCode _subKeyCode;
	cocos2d::Node* _btnNode;
	bool _isInEdit;
	cocos2d::Vector<cocos2d::ui::ImageView*> _vectorSelect;
	bool _isFocusOnBtn;
public:
	unsigned int _beginTouchTime;
};



class MyGameAdd : public BaseLayer{
public:
	CREATE_FUNC(MyGameAdd);
	virtual void onClickMenu() override;
	virtual void onClickOK() override;
	virtual void onClickBack() override;

	virtual void flushUI();
	virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	//根据包名添加到游戏列表
	void addAppInList(std::string& package);
CC_CONSTRUCTOR_ACCESS:
	MyGameAdd();
	virtual ~MyGameAdd();
	virtual bool init() override;
private:
	virtual bool initUI() override;
	virtual bool initData() override;

	void clickItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
public:
	UIPageViewEx* _cdPageView;
	std::vector<GameBasicData> _vectorData;
};

#endif /* defined(__xZone__MyGame__) */

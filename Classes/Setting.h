//
//  DLControl.h
//  xZone
//
//  Created by Sharezer on 15/6/9.
//
//

#ifndef __xZone__Setting__
#define __xZone__Setting__

#include "cocos2d.h"

#include "BaseLayer.h"
#include "DataManager.h"

class Setting : public BaseLayer
{
public:
	CREATE_FUNC(Setting);
    
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;

	void setSoundEnable(bool isEnable){
		DataManager::getInstance()->_sUserData.soundState = isEnable;
		DataManager::getInstance()->flushUserData();
	};
	void setAutoInstall(bool isEnable){
		DataManager::getInstance()->_sUserData.isAutoInstall = isEnable;
		DataManager::getInstance()->flushUserData();
	}

	void setInstallFinishDel(bool isEnable){
		DataManager::getInstance()->_sUserData.isInstallFinishDel = isEnable;
		DataManager::getInstance()->flushUserData();
	}

	void setDLMaxCount(int count){
		DataManager::getInstance()->_sUserData.dlMaxCount = count;
		DataManager::getInstance()->flushUserData();
	}

CC_CONSTRUCTOR_ACCESS:
	Setting();
	virtual ~Setting();
    virtual bool init() override;

	void clickLeft();
	void clickRight();
    
private:
    virtual bool initUI() override;
    virtual bool initData() override;

	void clickOkItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	cocos2d::Node *getNextFocus(cocos2d::Vec2 dir);
};
#endif /* defined(__xZone__Setting__) */

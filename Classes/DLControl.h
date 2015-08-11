//
//  DLControl.h
//  xZone
//
//  Created by Sharezer on 15/2/9.
//
//

#ifndef __xZone__DLControl__
#define __xZone__DLControl__

#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"
#include "DLGameList.h"

class DLControl : public BaseLayer
{
public:
    CREATE_FUNC(DLControl);
    
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
	virtual void flushUI();
    
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
CC_CONSTRUCTOR_ACCESS:
    DLControl();
    virtual ~DLControl();
    virtual bool init() override;
    
private:
    virtual bool initUI() override;
    virtual bool initData() override;

	void pageViewEvent(cocos2d::Ref* sender, cocos2d::ui::PageView::EventType type);
	void clickGameItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void clickOpItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

	void removeApkFile(std::string &package);

	void getSizeByMB(std::string sizeStr, double &size);
	float getSizePercent(std::string total, std::string availSize);

private:
	DLGameList* _gamePageView;
	int _focusState; //0->null, 1->left, 2->right
};
#endif /* defined(__xZone__DLControl__) */

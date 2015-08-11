//
//  About.h
//  xZone
//
//  Created by Sharezer on 15/6/15.
//
//

#ifndef __xZone__About__
#define __xZone__About__

#include "cocos2d.h"

#include "BaseLayer.h"
#include "DataManager.h"

class About : public BaseLayer
{
public:
	CREATE_FUNC(About);
    
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;

CC_CONSTRUCTOR_ACCESS:
	About();
	virtual ~About();
    virtual bool init() override;
    
private:
    virtual bool initUI() override;
    virtual bool initData() override;

	void clickItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
private:
	cocos2d::ui::ScrollView* _scrollView;
};
#endif /* defined(__xZone__About__) */

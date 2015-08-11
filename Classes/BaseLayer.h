//
//  BaseLayer.h
//  xZone
//
//  Created by Sharezer on 15/1/16.
//
//

#ifndef __xZone__BaseLayer__
#define __xZone__BaseLayer__

#include "cocos2d.h"
#include "Global.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

class BaseLayer : public cocos2d::Layer {
public:
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) = 0;
    
    virtual void onClickBack();
    virtual void onClickMenu() = 0;
    virtual void onClickOK() = 0;
	virtual void flushUI(){};
	void commonFocusAction(cocos2d::Node* newFoucs);
	void removeFocusAction(bool isHideSelect = true);
    virtual void showCurFocus();
	//初试化的时候把所有的ZOrder先存下来
	//virtual void initMapFocusZOrder(cocos2d::Node* parent);
	bool isExistOrder(const int& keyName);
	virtual void onEnter() override;
CC_CONSTRUCTOR_ACCESS:
    BaseLayer();
    virtual ~BaseLayer();
    virtual bool init() override;

private:
    virtual bool initUI() = 0;
    virtual bool initData() = 0;

public:
    cocos2d::Node* _rootNode;
    cocos2d::Node* _focus;

	//用于存储focus的ZOrder
	typedef std::map<int, int> OrderMap;
	OrderMap _mapFocusZOrder;
    CC_SYNTHESIZE(LayerType, _type, Type);
};

class Dialog : public BaseLayer{
public:
	CREATE_FUNC(Dialog);

	void setContent(const std::string& str);
	void setDialogType(DialogType type);
	virtual void onClickBack() override;
	virtual void onClickMenu() override;
	virtual void onClickOK() override;

	virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;

CC_CONSTRUCTOR_ACCESS:
	Dialog();
	virtual ~Dialog();
	virtual bool init() override;

private:
	virtual bool initUI() override;
	virtual bool initData() override;
	void clickBtn(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
private:
	cocos2d::ui::Button* _btnOK;
	cocos2d::ui::Button* _btnBack;
	DialogType _dialogType;
	cocos2d::ui::TextField* _ipTextField;
public:
	std::function<void()> okEvent;
	std::function<void()> cancelEvent;
};

class Loading : public BaseLayer {
public:
	CREATE_FUNC(Loading);

	virtual void onClickMenu() override;
	virtual void onClickOK() override;
	virtual void onClickBack() override;

	void setPercent(float percent);
	void resetPercent();
	void setLoadingBarVisiable(bool isVisiable){ _percent->setVisible(isVisiable); };
	virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	inline void showBG(bool isShow){ _bg->setVisible(isShow); };

CC_CONSTRUCTOR_ACCESS:
	Loading();
	virtual ~Loading();
	virtual bool init() override;

private:
	virtual bool initUI() override;
	virtual bool initData() override;
	void clickItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

private:
	cocos2d::ui::LoadingBar* _percent;
	cocos2d::ui::Layout* _bg;
public:
	cocos2d::Sprite* _loadSprite;
};

class SelectBox : public cocos2d::Node {
public:
	CREATE_FUNC(SelectBox);

	void showSelect(bool show, cocos2d::Size size = cocos2d::Size::ZERO, cocos2d::Vec2 pos = cocos2d::Vec2::ZERO, bool isLittle = false);
CC_CONSTRUCTOR_ACCESS:
	SelectBox();
	virtual ~SelectBox();
	virtual bool init() override;

public:
	cocos2d::ui::ImageView* _boxB;//大
	cocos2d::ui::ImageView* _boxL;//小
	cocos2d::ui::ImageView* _box;
};

class UIImageViewWithTexture : public cocos2d::ui::ImageView
{
	DECLARE_CLASS_GUI_INFO

public:
	/**
	* Default constructor
	*/
	UIImageViewWithTexture();

	/**
	* Default destructor
	*/
	virtual ~UIImageViewWithTexture();

	static UIImageViewWithTexture* create();
	static UIImageViewWithTexture* create(const std::string& imageFileName, cocos2d::ui::TextureResType texType = cocos2d::ui::TextureResType::LOCAL);
	static UIImageViewWithTexture* create(cocos2d::Texture2D *texture);
	void loadTexture(cocos2d::Texture2D *texture);
};


#endif /* defined(__xZone__BaseLayer__) */

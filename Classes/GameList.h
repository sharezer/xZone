//
//  GameList.h
//  xZone
//
//  Created by Sharezer on 15/4/24.
//
//

#ifndef __xZone__GameList__
#define __xZone__GameList__

#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"


enum class ListModel
{
	NOON,
	COLLECT_GAME,
	HOT_GAME,
	NEW_GAME
};

class UIPageViewEx;
class GameList : public BaseLayer {
public:
	static GameList* create(ListModel model);

	virtual void onClickBack() override;
	virtual void onClickMenu() override;
	virtual void onClickOK() override;

	virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	void touchAction(cocos2d::Node* newFoucs);
	//更新游戏显示信息
	void updateGameTitle();

	bool initWithModel(ListModel model);
CC_CONSTRUCTOR_ACCESS:
	GameList();
	virtual ~GameList();

private:
	virtual bool initUI() override;
	virtual bool initData() override;

	void clickItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

	void pageViewEvent(cocos2d::Ref* sender, cocos2d::ui::PageView::EventType type);
private:
	UIPageViewEx* _cdPageView;
	std::vector<GameBasicData> _vectorData;
	cocos2d::EventKeyboard::KeyCode _subKeyCode;
public:

	rapidjson::Document _doc;
	CC_SYNTHESIZE_READONLY(ListModel, _listModel, ListModel);
};

#endif /* defined(__xZone__GameList__) */

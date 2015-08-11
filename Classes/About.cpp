//
//  About.cpp
//  xZone
//
//  Created by Sharezer on 15/6/15.
//
//

#include "About.h"
#include "LsTools.h"
#include "ZoneManager.h"

USING_NS_CC;
USING_NS_GUI;

About::About()
	: _scrollView(nullptr)
{

}

About::~About()
{
}

bool About::init()
{
	if (!BaseLayer::init()) {
		return false;
	}

	_type = LayerType::SECOND;
	return true;
}

bool About::initUI()
{
	bool bRet = false;
	do {
		_rootNode = CSLoader::createNode(s_AboutUI);
		this->addChild(_rootNode);
		_scrollView = _rootNode->getChildByName<ui::ScrollView*>("ScrollView");

		rapidjson::Document doc;
		LsTools::readJsonWithFile("mzsm.json", doc);

		auto title = Label::createWithTTF(doc["title"].GetString(), s_labelTTF, 40);
		title->setHorizontalAlignment(TextHAlignment::CENTER);
		title->setPosition(Vec2(_scrollView->getContentSize().width / 2, _scrollView->getContentSize().height * 1.85f));
		_scrollView->addChild(title);

		TTFConfig ttfConfig(s_labelTTF, 36);
		auto content = Label::createWithTTF(ttfConfig, doc["content"].GetString(), TextHAlignment::LEFT, _scrollView->getContentSize().width * 0.9f);
		content->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
		content->setPosition(Vec2(_scrollView->getContentSize().width / 2, _scrollView->getContentSize().height * 1.8f));
		_scrollView->addChild(content);

		bRet = true;
	} while (0);
	return bRet;
}

bool About::initData()
{
	bool bRet = false;
	do {
		auto listener = EventListenerTouchOneByOne::create();
		listener->onTouchBegan = [&](Touch*, Event*){
			//this->runAction(LsTools::delayAndCallFunc(0.5f, [&]{this->onClickBack(); }));
			this->onClickBack();
			return true;
		};

		_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
		bRet = true;
	} while (0);
	return bRet;
}

void About::onClickOK()
{
	onClickBack();
	//clickItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void About::clickItem(Ref* sender, ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED)
		return;
	onClickBack();
}

void About::onClickMenu()
{
}

void About::changeFocus(::EventKeyboard::KeyCode keyCode)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_UP:
		_scrollView->scrollToTop(2.0f, true);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_DOWN:
		_scrollView->scrollToBottom(2.0f, true);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_LEFT:
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_DPAD_RIGHT:
		break;
	default:
		break;
	}
}
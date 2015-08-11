//
//  GameList.cpp
//  xZone
//
//  Created by Sharezer on 15/4/24.
//
//

#include "GameList.h"
#include "ZoneManager.h"
#include "json/document.h"
#include "LsTools.h"
#include "UIPageViewEx.h"
#include "DataManager.h"
#include "NetManager.h"

USING_NS_CC;

GameList::GameList()
	: _cdPageView(nullptr)
	, _subKeyCode(EventKeyboard::KeyCode::KEY_NONE)
	, _listModel(ListModel::NOON)
{
}

GameList::~GameList() {}

GameList* GameList::create(ListModel model)
{
	GameList* pRet = new(std::nothrow) GameList();
	if (pRet && pRet->initWithModel(model))
	{
		pRet->autorelease();
		return pRet;
	}
	else
	{
		delete pRet;
		pRet = NULL;
		return NULL;
	}
}


bool GameList::initWithModel(ListModel model)
{
	_type = LayerType::SECOND;
	_listModel = model;
	auto scene = Scene::createWithPhysics();

	if (!BaseLayer::init()) {
		return false;
	}

	return true;
}

bool GameList::initUI()
{
	_rootNode = CSLoader::createNode(s_MyGameUI);
	_rootNode->getChildByName("Node_btn")->setVisible(false);
	{
		LsTools::seekNodeByName(_rootNode, "opTips1")->setVisible(false);
		LsTools::seekNodeByName(_rootNode, "OpTips2")->setVisible(false);
		LsTools::seekNodeByName(_rootNode, "OpTips3")->setVisible(false);
	}

	this->addChild(_rootNode);

	return true;
}

bool GameList::initData()
{
	for (unsigned int i = 0; i < DataManager::getInstance()->_sGameDetailData.size(); i++) {
		auto& data = DataManager::getInstance()->_sGameDetailData[i];
		if (_listModel == ListModel::NOON)
			return false;
		else if (_listModel == ListModel::COLLECT_GAME && data._isCollect == 0)
			continue;
		else if (_listModel == ListModel::HOT_GAME && data._isHot == "0")
			continue;
		else if (_listModel == ListModel::NEW_GAME && data._isNewAdd == "0")
			continue;

		GameBasicData basicData = data._basic;
		//basicData._name = val["name"].GetString();
		//basicData._icon = formatStr("%s/%s.png", s_GameIconPath, val["package"].GetString());
		//basicData._number = i;
		//basicData.packageName = val["package"].GetString();
		_vectorData.push_back(basicData);
	}
	auto title = _rootNode->getChildByName<ui::Text*>("MyGameTex");
	switch (_listModel)
	{
	case ListModel::NOON:
		title->setString("");
		break;
	case ListModel::COLLECT_GAME:
		title->setString(DataManager::getInstance()->valueForKey("title1"));
		break;
	case ListModel::HOT_GAME:
		title->setString(DataManager::getInstance()->valueForKey("title2"));
		break;
	case ListModel::NEW_GAME:
		title->setString(DataManager::getInstance()->valueForKey("title3"));
		break;
	default:
		break;
	}

	UIPageViewEx::PageViewParams params;
	params._rows = 2;
	params._cols = 5;
	params._leftMargin = 38;
	params._colsDiff = 30;//行间距
	params._rowsDiff = 30;//列间距
	params._topMargin = 29;//顶间距
	params._bottomMargin = 29;
	params._rightMargin = 120;
	params._extenSize = 50;
	params._dir = UIPageViewEx::VERTICAL;
	params._isShowScoll = true;
	params._isShowScore = true;
	params._sType = UIPageViewEx::VER_TYPE;
	params._isShowAleadyInstall = true;
	//params._isLastUseFullImg = true;

	_cdPageView = UIPageViewEx::create(_vectorData, params);
	_rootNode->addChild(_cdPageView);
	_cdPageView->setTouchEnabled(true);
	_cdPageView->setClippingType(ui::Layout::ClippingType::SCISSOR);
	_cdPageView->setCustomScrollThreshold(MY_SCREEN.height / 8);
	_cdPageView->setUsingCustomScrollThreshold(true);
	_cdPageView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_cdPageView->setPosition(Vec2(961.5f + 41, 531));
	_cdPageView->addEventListener(CC_CALLBACK_2(GameList::pageViewEvent, this));
	Size t = _cdPageView->getContentSize();
	_mapFocusZOrder.clear();
	for (auto &pages : _cdPageView->getPages())
		for (auto &child : pages->getChildren())
		{
			_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
			auto img = static_cast<ui::ImageView*>(child);
			img->addTouchEventListener(CC_CALLBACK_2(GameList::clickItem, this));
		}

	updateGameTitle();

	return true;
}

void GameList::onClickMenu()
{
}

void GameList::clickItem(Ref* sender, ui::Widget::TouchEventType type)
{
	if (ZM_DIALOG->isVisible() || !sender || ui::Widget::TouchEventType::ENDED != type)
		return;

	auto view = static_cast<ui::ImageView*>(sender);

	if (_focus != view){
		touchAction(view);
		return;
	}

	std::string packageName = *(std::string*)view->getUserData();
	DataManager::getInstance()->_sUserData.gamePackageName = packageName;

	GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(DataManager::getInstance()->_sUserData.gamePackageName);
	if (!data || std::atof(data->_size.c_str()) <= 0)
		return;

	ZM->changeThirdState(ThirdState::GAME_DETAIL);
	
}

void GameList::onClickBack()
{
	BaseLayer::onClickBack();
}

void GameList::onClickOK()
{
	if (_focus)
		clickItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void GameList::changeFocus(EventKeyboard::KeyCode keyCode)
{
	auto next = LsTools::getNextFocus(keyCode, _cdPageView->getCurPage());
	if (!next)
		next = LsTools::getNextPageFocus(_cdPageView, _focus, keyCode);
	touchAction(next);
}

void GameList::touchAction(cocos2d::Node* newFoucs)
{
	commonFocusAction(newFoucs);
	if (_focus){
		ZM->showSelect(true,
			_focus->getContentSize() * FOCUS_SCALE_NUM,
			_cdPageView->convertToWorldSpace(_focus->getPosition()));
		_cdPageView->getCurPage()->setUserObject(_focus);
	}
}

void GameList::pageViewEvent(Ref* sender, ui::PageView::EventType type)
{
	if (type != ui::PageView::EventType::TURNING)
		return;
	updateGameTitle();
}

void GameList::updateGameTitle()
{
	{
		//update title
		auto size = _vectorData.size();
		auto gameNumTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "GameNumtxt"));
		std::string str = std::string(int2str(size)) + std::string(DataManager::getInstance()->valueForKey("kuan"));
		gameNumTxt->setString(str);
	}
	{
		//update page infor
		if (_cdPageView){
			auto size = _cdPageView->getPagesCount();
			size = size == 0 ? 1 : size;
			size = _vectorData.size() == 0 ? 0 : size;
			auto curr = _cdPageView->getCurPageIndex();
			curr = _vectorData.size() == 0 ? -1 : curr;

			auto gamePageTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "gamePageTxt"));
			std::string str = std::string(int2str(curr + 1)) + std::string("/") + std::string(int2str(size));
			gamePageTxt->setString(str);
		}
	}
}

//
//  MyGame.cpp
//  xZone
//
//  Created by Sharezer on 15/2/7.
//
//

#include "MyGame.h"
#include "ZoneManager.h"
#include "json/document.h"
#include "PlatformHelper.h"
#include "LsTools.h"
#include "UIPageViewEx.h"
#include "DataManager.h"

USING_NS_CC;

#define LONG_TOUCH_DELAY 800	//长按生效时间

#pragma region MyGame

MyGame::MyGame()
	: _cdPageView(nullptr)
	, _beginTouchTime(0)
	, _subKeyCode(EventKeyboard::KeyCode::KEY_NONE)
	, _btnNode(nullptr)
	, _isInEdit(false)
	, _isFocusOnBtn(false)
{
	_vectorSelect.clear();
}

MyGame::~MyGame() {
	_vectorSelect.clear();
}

bool MyGame::init()
{
	if (!BaseLayer::init()) {
		return false;
	}

	_type = LayerType::SECOND;

	_btnNode = _rootNode->getChildByName("Node_btn");
	_btnNode->setVisible(false);
	for (auto& child : _btnNode->getChildren()){
		_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
		auto btn = static_cast<ui::Button*>(child);
		btn->addTouchEventListener(CC_CALLBACK_2(MyGame::clickItem, this));
	}

	updateGameTitle();
	getScheduler()->schedule(CC_SCHEDULE_SELECTOR(MyGame::onUpdate), this, 0.5f, false);
	return true;
}

bool MyGame::initUI()
{
	_rootNode = CSLoader::createNode(s_MyGameUI);
	addChild(_rootNode);

	return true;
}

void MyGame::flushUI()
{
	_focus = nullptr;
	initData();
	updateGameTitle();
	if (ZM->getThirdState() == ThirdState::GAME_ADD)
		_cdPageView->setVisible(false);
}

bool MyGame::initData()
{
	_vectorData.clear();

	LS_LOG("app size: %d", DataManager::getInstance()->_applist.size());
	if (DataManager::getInstance()->_applist.size() == 0) {
		ZM->_isLoading = true;
		return true;
	}

	for (unsigned int i = 0; i < DataManager::getInstance()->_applist.size(); i++) {
		GameBasicData data;
		AppStruct app = DataManager::getInstance()->_applist[i];
		if (app.state != (int)AppStateEnum::MYGAME && app.state != (int)AppStateEnum::PLUS)
			continue;

		data._name = app.name;
		data._icon = formatStr("%s/%s.png", s_GameIconPath, app.package.c_str());
		data._number = i;
		data.packageName = app.package;
		data._isStencil = false;
		_vectorData.push_back(data);
	}

	GameBasicData addData;
	addData._icon = "Icon_MyGame_add.png";
	_vectorData.push_back(addData);

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

	params._sType = UIPageViewEx::VER_TYPE;
	params._dir = UIPageViewEx::VERTICAL;
	params._isShowScoll = true;
	params._isLastUseFullImg = true;

	if (_cdPageView){
		_cdPageView->removeFromParent();
		_cdPageView = nullptr;
	}
	_cdPageView = UIPageViewEx::create(_vectorData, params, true);
	_rootNode->addChild(_cdPageView);
	_cdPageView->setTouchEnabled(true);
	_cdPageView->setClippingType(ui::Layout::ClippingType::SCISSOR);
	_cdPageView->setCustomScrollThreshold(MY_SCREEN.height / 8);
	_cdPageView->setUsingCustomScrollThreshold(true);
	_cdPageView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_cdPageView->setPosition(Vec2(961.5f + 41, 531));
	_cdPageView->addEventListener(CC_CALLBACK_2(MyGame::pageViewEvent, this));

	_mapFocusZOrder.clear();
	for (auto &pages : _cdPageView->getPages())
		for (auto &child : pages->getChildren())
		{
			_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
			auto img = static_cast<ui::ImageView*>(child);
			img->addTouchEventListener(CC_CALLBACK_2(MyGame::clickItem, this));
		}
	updateGameTitle();

	return true;
}

void MyGame::onClickMenu()
{
	_isInEdit = !_isInEdit;
	resetSelect(_isInEdit);
}

void MyGame::resetSelect(bool isEdit)
{
	for (auto &pages : _cdPageView->getPages())
		for (auto &child : pages->getChildren()){
			auto btn = static_cast<ui::Button*>(LsTools::seekNodeByName(child, "selectBtn"));
			if (!btn)
				continue;
			btn->setVisible(isEdit);
			btn->setBright(true);
		}
	_btnNode->setVisible(isEdit);
	_isInEdit = isEdit;
	_vectorSelect.clear();

	if (_isFocusOnBtn){
		_isFocusOnBtn = false;
		if (_focus->getParent() == _btnNode){
			removeFocusAction();
			touchAction(dynamic_cast<Node*>(_cdPageView->getCurPage()->getUserObject()));
			_btnNode->setUserObject(nullptr);
		}
	}
}

void MyGame::clickItem(Ref* sender, ui::Widget::TouchEventType type)
{
	if (ZM_DIALOG->isVisible() || !sender)
		return;

	switch (type)
	{
	case cocos2d::ui::Widget::TouchEventType::BEGAN:{
		timeval psv;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		cocos2d::gettimeofday(&psv, NULL); //3.0获取本地时间
#else
		gettimeofday(&psv, NULL); //3.0获取本地时间
#endif
		_beginTouchTime = (unsigned int)(psv.tv_sec * 1000 + psv.tv_usec / 1000);
	}break;
	case cocos2d::ui::Widget::TouchEventType::MOVED:
		break;
	case cocos2d::ui::Widget::TouchEventType::ENDED:{
		_beginTouchTime = 0;
		auto view = static_cast<ui::ImageView*>(sender);
		_isFocusOnBtn = view->getParent() == _btnNode ? true : false;

		if (_focus != view)
		{
			touchAction(view);
			return;
		}

		auto userDate = view->getUserData();
		std::string package = userDate != nullptr ? *(std::string*)userDate : "";
		if (userDate && package.empty()){
			auto scale = ScaleTo::create(CHANGE_STATE_DURATION, 0);
			auto callback = CallFunc::create([&]{_cdPageView->setVisible(false); });
			_cdPageView->runAction(Sequence::createWithTwoActions(scale, callback));
			ZM->changeThirdState(ThirdState::GAME_ADD);
			break;
		}
		//我的游戏编辑
		if (_isInEdit)
		{
			if (view->getParent() == _btnNode)
			{
				LS_LOG("btn name: %s", view->getName().c_str());
				//卸载
				if (view->getName() == "Button_1")
				{
					for (auto &child : _vectorSelect)
					{
						std::string packageName = *(std::string*)child->getUserData();
						PH::uninstallApp(packageName);
					}
				}
				//移除
				else if (view->getName() == "Button_2"){
					//rapidjson::Document doc = DM->getAppList();
					for (auto &child : _vectorSelect)
					{
						std::string packageName = *(std::string*)child->getUserData();
						//deleteAppInList(packageName.c_str(), doc);
						deleteAppInList(packageName);
					}
					DataManager::getInstance()->flushAppList();
					ZM->flushUI(true);
				}
				
				resetSelect(false);
			}
			else{
				auto btn = dynamic_cast<ui::Button*>(LsTools::seekNodeByName(view, "selectBtn"));
				if (!btn)
					break;

				btn->setBright(!btn->isBright());
				if (btn->isBright())
					_vectorSelect.eraseObject(view, false);
				else
					_vectorSelect.pushBack(view);
			}
		}
		//启动游戏
		else
			PH::runApp(package);
	}break;
	case cocos2d::ui::Widget::TouchEventType::CANCELED:
		_beginTouchTime = 0;
		break;
	default:
		break;
	}
}

void MyGame::onClickBack()
{
	if (_isInEdit)
		resetSelect(false);
	else
		BaseLayer::onClickBack();
}

void MyGame::onClickOK()
{
	if (_focus)
		clickItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void MyGame::onUpdate(float dt)
{
	if (_beginTouchTime != 0)
	{
		timeval psv;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		cocos2d::gettimeofday(&psv, NULL); //3.0获取本地时间
#else
		gettimeofday(&psv, NULL); //3.0获取本地时间
#endif
		unsigned newTime = (unsigned int)(psv.tv_sec * 1000 + psv.tv_usec / 1000);
		LS_LOG("time: %d", newTime - _beginTouchTime);
		if (newTime - _beginTouchTime > LONG_TOUCH_DELAY)
		{
			onClickMenu();
			_beginTouchTime = 0;
		}
	}
}

void MyGame::changeFocus(EventKeyboard::KeyCode keyCode)
{
	if (!_cdPageView) return;
	doFocusChange(keyCode);
}

void MyGame::doFocusChange(EventKeyboard::KeyCode keyCode)
{

	if (!_isInEdit){
		auto next = LsTools::getNextFocus(keyCode, _cdPageView->getCurPage());
		if (!next)
			next = LsTools::getNextPageFocus(_cdPageView, _focus, keyCode);

		if (next && next != _focus)
			touchAction(next);
}
	else{
		if (!_isFocusOnBtn){
			auto next = LsTools::getNextFocus(keyCode, _cdPageView->getCurPage());
			if (!next)
				next = LsTools::getNextPageFocus(_cdPageView, _focus, keyCode);

			if (next && next != _focus)
				touchAction(next);
			else if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_DOWN){
				_isFocusOnBtn = true;
				_btnNode->setUserObject(nullptr);
				removeFocusAction();
				doFocusChange(keyCode);
			}
		}
		else{
			if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_UP){
				_isFocusOnBtn = false;
				removeFocusAction();
				_btnNode->setUserObject(nullptr);
				touchAction(dynamic_cast<Node*>(_cdPageView->getCurPage()->getUserObject()));
			}
			else{
				auto next = LsTools::getNextFocus(keyCode, _btnNode);
				if (next)
					touchAction(next);
			}
		}
	}
}

void MyGame::touchAction(cocos2d::Node* newFoucs)
{
	auto oldFocus = _focus;
	commonFocusAction(newFoucs);

	if (_focus){
		if (_isFocusOnBtn){
			ZM->showSelect(true,
				_focus->getContentSize() * FOCUS_SCALE_NUM,
				_focus->getPosition());
			if (oldFocus && oldFocus->getParent() != _btnNode)
				oldFocus->getParent()->setUserObject(oldFocus);
		}
		else{
			ZM->showSelect(true,
				_focus->getContentSize() * FOCUS_SCALE_NUM,
				_cdPageView->convertToWorldSpace(_focus->getPosition()));
		}
	}
}

/*调用完记得
DM->flushAppList();
ZM->flushUI();
*/
void MyGame::deleteAppInList(std::string package)
{
	if (package.empty())
		return;
	for (auto& app : DataManager::getInstance()->_applist){
		if (app.package != package)
			continue;

		switch ((AppStateEnum)app.state)
		{
		case  AppStateEnum::Noon:
		case  AppStateEnum::PLUS:
			app.state = (int)AppStateEnum::Noon;
			break;
		case  AppStateEnum::MYGAME:
		case  AppStateEnum::DELETE:
			app.state = (int)AppStateEnum::DELETE;
			break;
		default:
			break;
		}

		break;
	}
}

void MyGame::pageViewEvent(Ref* sender, ui::PageView::EventType type)
{
	if (type != ui::PageView::EventType::TURNING)
		return;
	updateGameTitle();
}

void MyGame::updateGameTitle()
{
	{
		auto size = _vectorData.size() - 1;
		auto gameNumTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "GameNumtxt"));
		std::string str = std::string(int2str(size)) + std::string(DataManager::getInstance()->valueForKey("kuan"));
		gameNumTxt->setString(str);
	}
	{
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

#pragma endregion MyGame


#pragma region MyGameAdd


MyGameAdd::MyGameAdd()
	:_cdPageView(nullptr)
{

}

MyGameAdd::~MyGameAdd() {}

bool MyGameAdd::init()
{
	if (!BaseLayer::init()) {
		return false;
	}

	_type = LayerType::THIRD;
	if (ZM->getThirdState() != ThirdState::GAME_ADD)
		PH::showToast(DataManager::getInstance()->valueForKey("AddAPP"));

	return true;
}

bool MyGameAdd::initUI()
{
	_rootNode = CSLoader::createNode(s_MyGameAddUI);
	this->addChild(_rootNode);

	return true;
}

void MyGameAdd::flushUI()
{
	_focus = nullptr;
	initData();
}

bool MyGameAdd::initData()
{
	_vectorData.clear();

	for (unsigned int i = 0; i < DataManager::getInstance()->_applist.size(); i++){
		GameBasicData data;
		AppStruct app = DataManager::getInstance()->_applist[i];
		if (app.state == (int)AppStateEnum::MYGAME || app.state == (int)AppStateEnum::PLUS)
			continue;

		data._name = app.name;
		data._icon = formatStr("%s/%s.png", s_GameIconPath, app.package.c_str());
		data._number = i;
		data._isStencil = false;
		data.packageName = app.package;
		_vectorData.push_back(data);

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

	if (_cdPageView){
		_cdPageView->removeFromParent();
		_cdPageView = nullptr;
	}
	_cdPageView = UIPageViewEx::create(_vectorData, params);
	_rootNode->addChild(_cdPageView);
	_cdPageView->setClippingType(ui::Layout::ClippingType::SCISSOR);
	Vec2 pagePos = MY_SCREEN_CENTER + Vec2(40, 0) - Vec2(_cdPageView->getContentSize().width / 2, _cdPageView->getContentSize().height / 2);
	_cdPageView->setPosition(pagePos);
	_mapFocusZOrder.clear();
	for (auto &pages : _cdPageView->getPages())
	{
		for (auto &child : pages->getChildren())
		{
			_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
			auto img = static_cast<ui::ImageView*>(child);
			img->addTouchEventListener(CC_CALLBACK_2(MyGameAdd::clickItem, this));
		}
	}

	return true;
}

void MyGameAdd::onClickMenu()
{

}

void MyGameAdd::clickItem(Ref* sender, ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED || !sender)
		return;
	auto view = static_cast<ui::ImageView*>(sender);
	if (_focus != view){
		commonFocusAction(view);
		if (_focus){
			ZM->showSelect(true,
				_focus->getContentSize() * FOCUS_SCALE_NUM,
				_cdPageView->convertToWorldSpace(_focus->getPosition()));
			_cdPageView->getCurPage()->setUserObject(_focus);
		}
		return;
	}

	std::string packageName = *(std::string*)view->getUserData();
	addAppInList(packageName);
}

void MyGameAdd::onClickOK()
{
	clickItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void MyGameAdd::changeFocus(EventKeyboard::KeyCode keyCode)
{
	auto next = LsTools::getNextFocus(keyCode, _cdPageView->getCurPage());
	if (!next)
		next = LsTools::getNextPageFocus(_cdPageView, _focus, keyCode);

	commonFocusAction(next);
	if (_focus){
		ZM->showSelect(true,
			_focus->getContentSize() * FOCUS_SCALE_NUM,
			_cdPageView->convertToWorldSpace(_focus->getPosition()));
	}
}

void MyGameAdd::onClickBack()
{
	//BaseLayer::onClickBack();
	ZM->changeSecondState(SecondState::MY_GAME);
}
/*调用完记得
DM->flushAppList();
ZM->flushUI();
*/
void MyGameAdd::addAppInList(std::string& package)
{
	for (auto& app : DataManager::getInstance()->_applist){
		if (app.package != package)
			continue;

		switch ((AppStateEnum)app.state){
		case  AppStateEnum::Noon:
		case  AppStateEnum::PLUS:
			app.state = (int)AppStateEnum::PLUS;
			break;
		case  AppStateEnum::MYGAME:
		case  AppStateEnum::DELETE:
			app.state = (int)AppStateEnum::MYGAME;
			break;
		default:
			break;
		}
		DataManager::getInstance()->flushAppList();
		ZM->flushUI(true);
	}
}

#pragma endregion MyGameAdd
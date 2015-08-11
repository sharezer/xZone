//
//  MainLayer.cpp
//  xZone
//
//  Created by Sharezer on 15/1/19.
//
//

#include "MainLayer.h"
#include "LsTools.h"
#include "GameDetail.h"
#include "ZoneManager.h"
#include "DataManager.h"

#include "PlatformHelper.h"
#include "NetManager.h"
#include "QR_Encode.h"

USING_NS_CC;
USING_NS_GUI;

#define CENTER_RECOMEND_TAG	22
#define TAB_BTN_SCALE	1.3f
MainLayer::MainLayer()
	: _tabNode(nullptr)
	, _hideNode(nullptr)
	, _pageView(nullptr)
	, _isFocusNotOnTab(false)
	, _tabIndex(ENUM_PAGE::MAIN_PAGE)
	, _recommend(nullptr)
	, _recommendIndex(0)
	, _specIndex(0)
	, _isUseSpec(false)
	, _navi_Scelect(nullptr)
	, _bg(nullptr)
{
}

MainLayer::~MainLayer()
{
	CC_SAFE_RELEASE_NULL(_recommend);
}

bool MainLayer::init()
{
	_recommend2Doc = ZM_CONFIG["homeRecomend2"];
	_recommendDoc = ZM_CONFIG["homeRecommend"];

	if (!BaseLayer::init())
		return false;

	_type = LayerType::MAIN;

	schedule(CC_SCHEDULE_SELECTOR(MainLayer::updateRecommend), 3.0f);
	PH::getNetworkState();

	return true;
}

bool MainLayer::initUI()
{
	_rootNode = CSLoader::createNode("MainScene.csb");
	_hideNode = _rootNode->getChildByName<Node*>("Node_Main");
	this->addChild(_rootNode);

	for (auto& child : LsTools::seekNodeByName(_rootNode, "Node_btn")->getChildren()) {
		auto btn = dynamic_cast<Button*>(child);
		btn->addTouchEventListener(CC_CALLBACK_2(MainLayer::clickMainBtn, this));
	}

	_pageView = static_cast<PageView*>(LsTools::seekNodeByName(_rootNode, "PageView"));
	CCASSERT(_pageView, "page view");
	_pageView->setCustomScrollThreshold(MY_SCREEN.width / 8);
	_pageView->setUsingCustomScrollThreshold(true);
	_pageView->addEventListener(CC_CALLBACK_2(MainLayer::pageViewEvent, this));
	_pageView->setClippingType(Layout::ClippingType::SCISSOR);
	_mapFocusZOrder.clear();
	for (ssize_t i = 0; i < PAGE_COUNT; i++) {
		auto page = _pageView->getPage(i);

		if (i == MAIN_PAGE) {
			for (auto& child : page->getChildren()) {
				//排除倒影
				std::string::size_type idx = child->getName().find("dy_");
				if (idx != std::string::npos)
					continue;
				_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
				auto view = dynamic_cast<ImageView*>(child);
				view->addTouchEventListener(CC_CALLBACK_2(MainLayer::clickMainPageItem, this));
				if (view->getTag() == 22){
					_recommend = view;
					CC_SAFE_RETAIN(_recommend);
				}
			}
		}
		else if (i == MAIN_PAGE_PLUS){
			for (auto& child : page->getChildren()) {
				//排除倒影
				std::string::size_type idx = child->getName().find("dy_");
				if (idx != std::string::npos)
					continue;
				_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
				auto view = dynamic_cast<ImageView*>(child);
				view->addTouchEventListener(CC_CALLBACK_2(MainLayer::clickMainPlusPageItem, this));
			}
		}
		else if (i == CATEGORY_PAGE) {
			for (auto& child : page->getChildren()) {
				std::string::size_type idx = child->getName().find("dy_");
				if (idx != std::string::npos)
					continue;
				_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
				auto view = dynamic_cast<ImageView*>(child);
				view->addTouchEventListener(CC_CALLBACK_2(MainLayer::clickCategoryPageItem, this));
			}
		}
		else if (i == MANAGE_PAGE) {
			for (auto& child : page->getChildren()) {
				std::string::size_type idx = child->getName().find("dy_");
				if (idx != std::string::npos)
					continue;
				_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
				auto view = dynamic_cast<ImageView*>(child);
				if (view->getName() == "3")	//Remote QR
				{
					CQR_Encode code;
					float size = 5.5f;
					std::string ip = PH::getPlatformIP();
					std::string str = ZM->getServerAddress() + "?" + formatStr("commend=%d&xLinkIP=%s", CommendEnum::DOWNLOAD_REMOTE, ip.c_str());
					LS_LOG("CQR_Encode:%s", str.c_str());
					auto drawNode = LsTools::createQRAndDraw(code, size, LsTools::str2charp(str));
					drawNode->setPosition(cocos2d::Vec2((view->getContentSize().width - size * code.m_nSymbleSize) / 2 - 6,
						view->getContentSize().height - (view->getContentSize().height - size * code.m_nSymbleSize) / 2 + 45));
					view->addChild(drawNode);
				}
				else
					view->addTouchEventListener(CC_CALLBACK_2(MainLayer::clickManagerPageItem, this));
			}
		}
	}
	_tabNode = _pageView->getPage(_pageView->getCurPageIndex());
	_navi_Scelect = dynamic_cast<Sprite*>(LsTools::seekNodeByName(_rootNode, "Navi_Scelect"));
	_bg = _rootNode->getChildByName<Sprite*>("background");

	//倒影
	if (_isUseSpec)
	{
		initSpec();

		for (int i = 0; i < PAGE_COUNT; i++)
		{
			auto sprite = dynamic_cast<Sprite*>(LsTools::seekNodeByName(_rootNode, formatStr("dy_%d", i)));
			sprite->setVisible(true);
			GLchar * fragSource = (GLchar*)String::createWithContentsOfFile(
				FileUtils::getInstance()->fullPathForFilename("blend.fsh").c_str())->getCString();
			auto glprogram = GLProgram::createWithByteArrays(ccPositionTextureColor_noMVP_vert, fragSource);
			auto program = GLProgramState::getOrCreateWithGLProgram(glprogram);
			V3F_C4B_T2F_Quad quad = sprite->getQuad();
			float minTex = (quad.tl.texCoords.v > quad.bl.texCoords.v ? quad.bl.texCoords.v : quad.tl.texCoords.v);
			float maxTex = (quad.tl.texCoords.v < quad.bl.texCoords.v ? quad.bl.texCoords.v : quad.tl.texCoords.v);
			program->setUniformFloat("minTex", minTex);
			program->setUniformFloat("maxTex", maxTex);
			sprite->setGLProgramState(program);
		}
	}

	return true;
}

bool MainLayer::initData()
{
	changeTab(MAIN_PAGE);
	updateRecommend(0);

	return true;
}

void MainLayer::initSpec()
{
	if (!_isUseSpec)
		return;

	auto delay = DelayTime::create(0.5f);
	auto callback = CallFunc::create([=](){
		if (_specIndex == PAGE_COUNT) {
			_pageView->setPositionX(0);
			_specIndex = 0;
		}
		else {
			this->createSpec(_specIndex);
			_specIndex++;
			initSpec();
		}});
	this->runAction(Sequence::createWithTwoActions(delay, callback));
}

void MainLayer::onClickBack()
{

	ZM_DIALOG->setContent(DataManager::getInstance()->valueForKey("mainExit"));
	ZM_DIALOG->setDialogType(DialogType::TWO_BTN);
	ZM_DIALOG->okEvent = [&]() {
		Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		exit(0);
#endif
	};
	ZM_DIALOG->cancelEvent = [&]() {
	};

	/*rapidjson::Document doc;
	 PH::newGetGameListInZone(doc);*/

	//std::thread t1(&MainLayer::threadTest, this);
	//t1.join();
	//this->threadTest();

	//LS_LOG("is tv %d", PH::getIsTV());
	//_pageView->setPositionX(0);
	/*std::thread t1(&MainLayer::threadTest, this);
	 t1.join();
	 LS_LOG("%s", DM->valueForKey("noon"));
	 LS_LOG("%s", LsTools::getSysTime());*/

	/*PH::runApp("org.cocos2dx.cpp_tests");
	 RenderTexture* rt = RenderTexture::create(64, 64, Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
	 rt->begin();
	 this->visit();
	 rt->end();
	 rt->saveToFile("1.png");*/

}

void MainLayer::onClickMenu()
{
	//utils::captureScreen([&](bool s, const std::string aa){LS_LOG("%s", aa.c_str()); }, LsTools::getDataPath() + "1.png");
	/*RenderTexture* rt = RenderTexture::create(64, 64, Texture2D::PixelFormat::RGBA8888, GL_DEPTH24_STENCIL8);
	rt->begin();
	this->visit();
	rt->end();
	rt->saveToFile("1.png", cocos2d::Image::Format::PNG, true);*/
	//PlatformHelper::installApp(LsTools::getDataPath() + "/data/game/apk/aaa.apk");
	//NetManager::getInstance()->submitDownloadFail("com.cronlygames.tank.sd");
	/*LS_LOG("getSDTotalSize: %s", PH::getSDTotalSize().c_str());
	LS_LOG("getSDAvailableSize: %s", PH::getSDAvailableSize().c_str());
	LS_LOG("getRomTotalSize: %s", PH::getRomTotalSize().c_str());
	LS_LOG("getRomAvailableSize: %s", PH::getRomAvailableSize().c_str());

	LS_LOG("server address : %s", ZM->getServerAddress().c_str());*/

	//LS_LOG("texture info: %s", Director::getInstance()->getTextureCache()->getCachedTextureInfo().c_str());
	//Director::getInstance()->getTextureCache()->reloadTexture("Icon_MyGame.png");
	/*auto sprite = Sprite::create("Icon_MyGame.png");
	sprite->setPosition(MY_SCREEN_CENTER);
	this->addChild(sprite);*/


	
}

void MainLayer::onClickOK()
{
	if (!_focus)
		return;
	switch (_pageView->getCurPageIndex()) {
	case MAIN_PAGE: {
		clickMainPageItem(_focus, Widget::TouchEventType::ENDED);
	} break;
	case MAIN_PAGE_PLUS: {
		clickMainPlusPageItem(_focus, Widget::TouchEventType::ENDED);
	} break;
	case CATEGORY_PAGE: {
		clickCategoryPageItem(_focus, Widget::TouchEventType::ENDED);
	} break;
	case MANAGE_PAGE: {
		clickManagerPageItem(_focus, Widget::TouchEventType::ENDED);
	} break;
	default:
		break;
	}
}

void MainLayer::clickMainPageItem(Ref* sender, Widget::TouchEventType type)
{
	if (type != Widget::TouchEventType::ENDED)
		return;
	auto view = static_cast<ImageView*>(sender);

	if (_focus != view)
	{
		_isFocusNotOnTab = true;
		commonFocusAction(view);
		ZM->showSelect(true, view->getContentSize() * FOCUS_SCALE_NUM, view->getPosition());
		_tabNode = _pageView->getPage(_pageView->getCurPageIndex());
		//_tabNode->setUserObject(view);
		return;
	}

	int index = atoi(view->getName().c_str());
	LS_LOG("cur page index %d, Name: %d", (int)_pageView->getCurPageIndex(), index);
	if (index == 1)
		ZM->changeSecondState(SecondState::MY_GAME);
	else if (index == 0)
		ZM->changeSecondState(SecondState::COLLECT_GAME);
	else if (index == 3)
		ZM->changeSecondState(SecondState::HOT_GAME);
	else if (index == 4)
		ZM->changeSecondState(SecondState::NEW_GAME);
	else{
        //ZM->playVideo(_recommendDoc[_recommendIndex]["package"].GetString());
		DataManager::getInstance()->_sUserData.gamePackageName = _recommendDoc[_recommendIndex]["package"].GetString();

		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(DataManager::getInstance()->_sUserData.gamePackageName);
		if (!data)
		return;

		ZM->changeThirdState(ThirdState::GAME_DETAIL);
	}
}

void MainLayer::clickMainPlusPageItem(Ref* sender, Widget::TouchEventType type)
{
	if (type != Widget::TouchEventType::ENDED)
		return;

	auto view = static_cast<ImageView*>(sender);

	if (_focus != view)
	{
		_isFocusNotOnTab = true;
		commonFocusAction(view);
		ZM->showSelect(true, view->getContentSize() * FOCUS_SCALE_NUM, view->getPosition());
		_tabNode = _pageView->getPage(_pageView->getCurPageIndex());
		//_tabNode->setUserObject(view);
		return;
	}
	int index = atoi(view->getName().c_str());
	LS_LOG("cur page index %d, Name: %d， package: %s", (int)_pageView->getCurPageIndex(), index, _recommend2Doc[index]["package"].GetString());

	DataManager::getInstance()->_sUserData.gamePackageName = _recommend2Doc[index]["package"].GetString();
	if (DataManager::getInstance()->_sUserData.gamePackageName.empty())
		return;
	GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(DataManager::getInstance()->_sUserData.gamePackageName);

	if (!data)
		return;

	ZM->changeThirdState(ThirdState::GAME_DETAIL);
}

void MainLayer::clickCategoryPageItem(Ref* sender, Widget::TouchEventType type)
{
	if (type != Widget::TouchEventType::ENDED)
		return;
	auto view = static_cast<ImageView*>(sender);

	if (_focus != view)
	{
		_isFocusNotOnTab = true;
		commonFocusAction(view);
		ZM->showSelect(true, view->getContentSize() * FOCUS_SCALE_NUM, view->getPosition());
		_tabNode = _pageView->getPage(_pageView->getCurPageIndex());
		//_tabNode->setUserObject(view);
		return;
	}

	LS_LOG("cur page index %d, Name: %s", (int)_pageView->getCurPageIndex(), view->getName().c_str());
	DataManager::getInstance()->_sUserData.category = atoi(view->getName().c_str());
	DataManager::getInstance()->flushUserData();

	ZM->changeSecondState(SecondState::CATEGORY_DETAIL);

}

void MainLayer::clickManagerPageItem(Ref* sender, Widget::TouchEventType type)
{
	if (type != Widget::TouchEventType::ENDED)
		return;
	auto view = static_cast<ImageView*>(sender);

	if (_focus != view)
	{
		_isFocusNotOnTab = true;
		commonFocusAction(view);
		ZM->showSelect(true, view->getContentSize() * FOCUS_SCALE_NUM, view->getPosition());
		_tabNode = _pageView->getPage(_pageView->getCurPageIndex());
		//_tabNode->setUserObject(view);
		return;
	}

	LS_LOG("cur page index %d, Name: %s", (int)_pageView->getCurPageIndex(), view->getName().c_str());
	int index = atoi(view->getName().c_str());
	switch (index)
	{
	case 0:
		ZM->changeSecondState(SecondState::DOWNLOAD_CONTROL);		break;
	case 1:
		ZM->changeSecondState(SecondState::SETTING);
		break;
	case 2:
		ZM->changeSecondState(SecondState::ABOUNT);
		break;
	case 3:
		break;
	default:
		break;
	}
}

void MainLayer::clickMainBtn(Ref* sender, Widget::TouchEventType type)
{
	if (type != Widget::TouchEventType::ENDED)
		return;

	auto btn = dynamic_cast<Button*>(sender);
	std::string name = btn->getName();
	char ch = name[name.length() - 1];
	int index = atoi(formatStr("%c", ch).c_str());

	_isFocusNotOnTab = false;
	removeFocusAction();
	_tabNode->setUserObject(nullptr);

	index = index == MAIN_PAGE ? index : index + 1;
	changeTab(index);
}

void MainLayer::doFocusChange(EventKeyboard::KeyCode keyCode, bool isStartOnEnd)
{
	Node* next = nullptr;

	if (isStartOnEnd)
		next = LsTools::getCornerFocus(keyCode == EventKeyboard::KeyCode::KEY_DPAD_RIGHT ?
		CornerType::TOP_LEFT : CornerType::TOP_RIGHT, _tabNode);
	else
		next = LsTools::getNextFocus(keyCode, _tabNode);

	int exNormalZOrder = 0;
	if (_focus)
		exNormalZOrder = _mapFocusZOrder.at(_focus->getTag());
	//是否有下一个焦点
	if (next) {
		commonFocusAction(next);
		ZM->showSelect(true, next->getContentSize() * FOCUS_SCALE_NUM, next->getPosition());
	}
	else {
		//找不到焦点

		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_DPAD_UP: {
			//向上进入分页选择
			_isFocusNotOnTab = false;
			removeFocusAction();
			_tabNode->setUserObject(nullptr);
			getCurTabBtn()->setScale(TAB_BTN_SCALE);
		} break;
		case EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
			//向右，进入下一个分页，无下一个分页，则不处理
			int index = _pageView->getCurPageIndex();
			index++;
			if (index >= PAGE_COUNT)
				return;
			ZM_SELECT->_box->setPositionX(-MY_SCREEN.width * 0.5f);
			removeFocusAction(false);
			_tabNode->setUserObject(nullptr);

			changeTab(index);
			doFocusChange(keyCode, false);
		} break;
		case EventKeyboard::KeyCode::KEY_DPAD_LEFT: {
			//向左，进入下一个分页，无下一个分页，则不处理
			int index = _pageView->getCurPageIndex();
			index--;
			if (index < 0)
				return;
			ZM_SELECT->_box->setPositionX(MY_SCREEN.width * 1.5f);
			removeFocusAction(false);
			_tabNode->setUserObject(nullptr);

			changeTab(index);
			doFocusChange(keyCode, true);
		} break;
		default:
			break;
		}
	}
	//createSpecRecomend();
}


void MainLayer::changeFocus(EventKeyboard::KeyCode keyCode)
{
	if (_isFocusNotOnTab)
		doFocusChange(keyCode);
	else {
		if (keyCode == EventKeyboard::KeyCode::KEY_DPAD_UP)
			return;
		int index = _pageView->getCurPageIndex();

		switch (keyCode) {
		case EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
			index++;
			if (index >= PAGE_COUNT)
				return;
			index = index == MAIN_PAGE_PLUS ? index + 1 : index;
			changeTab(index);
			
		} break;
		case EventKeyboard::KeyCode::KEY_DPAD_LEFT: {
			index--;
			if (index < 0)
				return;
			index = index == MAIN_PAGE_PLUS ? MAIN_PAGE : index;
			changeTab(index);
		} break;
		case EventKeyboard::KeyCode::KEY_DPAD_DOWN: {
			_isFocusNotOnTab = true;
			getCurTabBtn()->setScale(1.0f);
			changeFocus(keyCode);
		} break;
		default:
			break;
		}
	}
	//_navi_Scelect->setVisible(!_isFocusNotOnTab);
}

void MainLayer::createSpec(int index)
{
	//return;
	if (!_hideNode->isVisible() || !_isUseSpec)
		return;
	_pageView->setPositionX(-MY_SCREEN.width * index);
	auto rt = RenderTexture::create(_pageView->getContentSize().width,
		_pageView->getContentSize().height,
		Texture2D::PixelFormat::RGBA8888,
		GL_DEPTH24_STENCIL8);
	rt->beginWithClear(0, 0, 0, 0, 0);
	_pageView->getPage(index)->visit(_director->getRenderer(), _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW), true);
	rt->end();
	rt->getSprite()->setScaleY(-1);
	// 	_renderPageViewTex->saveToFile("pageview.png");

	auto sprite = dynamic_cast<Sprite*>(LsTools::seekNodeByName(_rootNode, formatStr("dy_%d", index)));
	sprite->setTexture(rt->getSprite()->getTexture());
	//sprite->setTextureRect(rt->getSprite()->getTextureRect());

	if (index == 0)//main page
	{
		auto pane1 = LsTools::seekNodeByName(_pageView, "Panel_0");
		auto img3 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "3"));
		auto img0 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "0"));
		auto img2 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "2"));
		auto img1 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "1"));

		Rect rect(img1->getLeftBoundary(), img1->getBottomBoundary(),
			img3->getRightBoundary() - img1->getLeftBoundary(),
			img2->getContentSize().height);

		sprite->setTextureRect(Rect(rect.origin.x, rect.origin.y, rect.size.width, 300));
		float bot = img2->getBottomBoundary() - (sprite->getTextureRect().getMaxY() - sprite->getTextureRect().getMinY()) / 2 - 5;
		//sprite->setPosition(_pageView->getContentSize().width / 2, bot);
	}
	else/* if (curPage == 1)*///game category page
	{
		auto pane2 = LsTools::seekNodeByName(_pageView, "Panel_1");
		auto img0 = (ui::ImageView*)(LsTools::seekNodeByName(pane2, "0"));
		auto img5 = (ui::ImageView*)(LsTools::seekNodeByName(pane2, "5"));

		Rect rect(img0->getLeftBoundary(), img0->getBottomBoundary(),
			img5->getRightBoundary() - img0->getLeftBoundary(),
			img5->getContentSize().height);

		sprite->setTextureRect(Rect(rect.origin.x, rect.origin.y, rect.size.width, 300));
		float bot = img0->getBottomBoundary() - (sprite->getTextureRect().getMaxY() - sprite->getTextureRect().getMinY()) / 2 - 5;
		//sprite->setPosition(_pageView->getContentSize().width / 2, bot);
	}
}

void MainLayer::createSpecRecomend()
{
	if (!_hideNode->isVisible() || _pageView->getCurPageIndex() != MAIN_PAGE || !_isUseSpec)
		return;

	auto rt = RenderTexture::create(_pageView->getContentSize().width,
		_pageView->getContentSize().height,
		Texture2D::PixelFormat::RGBA8888,
		GL_DEPTH24_STENCIL8);
	rt->beginWithClear(0, 0, 0, 0, 0);
	_pageView->getPage(MAIN_PAGE)->visit(_director->getRenderer(), _director->getMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW), true);
	rt->end();
	rt->getSprite()->setScaleY(-1);

	auto sprite = dynamic_cast<Sprite*>(LsTools::seekNodeByName(_rootNode, formatStr("dy_%d", MAIN_PAGE)));
	sprite->setTexture(rt->getSprite()->getTexture());

	auto pane1 = LsTools::seekNodeByName(_pageView, "Panel_0");
	auto img3 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "3"));
	auto img0 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "0"));
	auto img2 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "2"));
	auto img1 = (ui::ImageView*)(LsTools::seekNodeByName(pane1, "1"));

	Rect rect(img1->getLeftBoundary(), img1->getBottomBoundary(),
		img3->getRightBoundary() - img1->getLeftBoundary(),
		img2->getContentSize().height);

	sprite->setTextureRect(Rect(rect.origin.x, rect.origin.y, rect.size.width, 300));
	float bot = img2->getBottomBoundary() - (sprite->getTextureRect().getMaxY() - sprite->getTextureRect().getMinY()) / 2 - 5;
}

void MainLayer::changeTab(int index)
{
	if (index >= PAGE_COUNT)
		return;
	LS_LOG("index %d", index);

	_pageView->scrollToPage(index);
	_tabNode = _pageView->getPage(index);

	index = index == MAIN_PAGE ? index : index - 1;
	auto btn = LsTools::seekNodeByName(_rootNode, "Node_btn")->getChildByName<Button*>(formatStr("Button_%d", index));
	_navi_Scelect->stopAllActions();
	_navi_Scelect->runAction(MoveTo::create(0.2f, Vec2(btn->getPositionX(), _navi_Scelect->getPositionY())));

	//_navi_Scelect->setVisible(!_isFocusNotOnTab);
}

void MainLayer::pageViewEvent(Ref* sender, PageView::EventType type)
{
	if (type != PageView::EventType::TURNING)
		return;
	auto pageView = dynamic_cast<PageView*>(sender);
	if (_focus && _focus->getParent() != pageView->getPage(pageView->getCurPageIndex()))
	{
		_isFocusNotOnTab = false;
		removeFocusAction();
		_tabNode->setUserObject(nullptr);
	}

	auto node = LsTools::seekNodeByName(_rootNode, "Node_btn");
	for (auto& child : node->getChildren())
	{
		auto btn = dynamic_cast<Button*>(child);
		btn->setBright(true);
		btn->setScale(1.0f);
	}

	auto btn = getCurTabBtn();
	btn->setBright(false);
	if (!_isFocusNotOnTab){
		btn->setScale(TAB_BTN_SCALE);
		_navi_Scelect->stopAllActions();
		_navi_Scelect->setPositionX(btn->getPositionX());
	}
		
	
	
	//_navi_Scelect->setPositionX(btn->getPositionX());
	//changeTab(pageView->getCurPageIndex());
	createSpecRecomend();
}

ui::Button* MainLayer::getCurTabBtn()
{
	int index = _pageView->getCurPageIndex();
	auto node = LsTools::seekNodeByName(_rootNode, "Node_btn");
	index = index == MAIN_PAGE ? index : index - 1;
	auto btn = node->getChildByName<Button*>(formatStr("Button_%d", index));
	return btn;
}

void MainLayer::updateRecommend(float dt)
{
	if ((_focus && _focus->getTag() == CENTER_RECOMEND_TAG) || !_recommend || !_hideNode->isVisible())
		return;
	/*if (NM->_isLoadFile || NM->_isSendCommend)
		return;*/
	if (_pageView->getCurPageIndex() != MAIN_PAGE)
		return;

	_recommendIndex++;
	if (_recommendIndex >= 5)
		_recommendIndex = 0;
	std::string fileName = _recommendDoc[_recommendIndex]["png"].GetString();
	if (fileName.empty() || !FileUtils::getInstance()->isFileExist(fileName))
		return;

	_recommend->loadTexture(fileName);
	for (auto& child : LsTools::seekNodeByName(_rootNode, "Node_Dot")->getChildren())
	{
		auto sprite = dynamic_cast<Sprite*>(child);
		sprite->setTexture("Common_Dot01.png");
	}
	auto dot = dynamic_cast<Sprite*>(LsTools::seekNodeByName(
		_rootNode,
		formatStr("Common_Dot_%02d", _recommendIndex + 1)));
	dot->setTexture("Common_Dot02.png");
	auto title = _recommend->getChildByName<Sprite*>("title");
	title->setTexture(_recommendDoc[_recommendIndex]["name"].GetString());

	_recommend->getChildByName<ui::Text*>("intro")->setString(_recommendDoc[_recommendIndex]["intro"].GetString());
	float gameScore = _recommendDoc[_recommendIndex]["score"].GetDouble();

	int sw = (int)gameScore % 10;
	int gw = (int)(gameScore * 10) % 10;
	_recommend->getChildByName<ui::Text*>("score_1")->setString(formatStr("%d", sw));
	_recommend->getChildByName<ui::Text*>("score_2")->setString(formatStr(".%d", gw));

	createSpecRecomend();

}

//
//  GCategoryDetail.cpp
//  xZone
//
//  Created by 苏明南 on 15/1/29.
//
//

#include "GCategoryDetail.h"
#include "ZoneManager.h"
#include "GCategoryDetailWidget.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include "LsTools.h"
#include "DataManager.h"
#include "NetManager.h"
#include "UIPageViewEx.h"
#include "GameDetail.h"

USING_NS_CC;

GCatetoryDetailLayer* GCatetoryDetailLayer::create( int &categoryName )
{
    GCatetoryDetailLayer *pRet = new GCatetoryDetailLayer(categoryName);
    if (pRet && pRet->init() )
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

GCatetoryDetailLayer::GCatetoryDetailLayer(int &categoryName) : _categoryName(categoryName), _categoryTitle(nullptr), _pageView(nullptr){}
GCatetoryDetailLayer::~GCatetoryDetailLayer()   
{
	CC_SAFE_RELEASE_NULL(_categoryTitle);
}

bool GCatetoryDetailLayer::init()
{
    if (!BaseLayer::init()) {
        return false;
    }

    _type = LayerType::SECOND;
    return true;
}

bool GCatetoryDetailLayer::initUI()
{
	LS_LOG("initUI()");

    _rootNode = CSLoader::createNode(s_GameCategoryDetail);
    this->addChild(_rootNode);
    
	ZM->showSelect(false);
	//添加华丽丽的游戏标题
	_categoryTitle = GCategoryDetailTitle::create(_rootNode, _categoryName);
	_categoryTitle->setTag(GAME_CATEGORY_DETAIL_TITLE);
	_mapFocusZOrder.clear();
	CC_SAFE_RETAIN(_categoryTitle);
	{
        //添加华丽丽的游戏列表
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
		params._fontSize = 33;
		params._isShowAleadyInstall = true;

		if (_pageView){
			_pageView->removeFromParent();
			_pageView = nullptr;
		}

		_pageView = UIPageViewEx::create(DataManager::getInstance()->getBasicGameByType(int2str(_categoryName)), params);
		_pageView->setAnchorPoint(Vec2(0.5f, 0.5f));
		_rootNode->addChild(_pageView);
		_pageView->setPosition(Vec2(1005, 532));
		//_pageView->setPosition(Vec2(191, 168));

		_pageView->setTouchEnabled(true);
		_pageView->setClippingType(ui::Layout::ClippingType::SCISSOR);
		_pageView->addEventListener(CC_CALLBACK_2(GCatetoryDetailLayer::pageViewEvent, this));

		for (auto &pages : _pageView->getPages())
        {
			for (auto &child : pages->getChildren())
			{
				auto img = static_cast<ui::ImageView*>(child);
				img->addTouchEventListener(CC_CALLBACK_2(GCatetoryDetailLayer::clickGameDetailItem, this));
				_mapFocusZOrder.insert(std::pair<int, int>(img->getTag(), img->getLocalZOrder()));
			}
        }
		_categoryTitle->setPageTips(_pageView->getCurPageIndex()+1, _pageView->getPagesCount());
    }

    return true;
}

bool GCatetoryDetailLayer::initData()
{
    return true;
}

void GCatetoryDetailLayer::flushUI()
{

}

void GCatetoryDetailLayer::onClickBack()
{
	BaseLayer::onClickBack();
}

void GCatetoryDetailLayer::onClickMenu()
{
	if (_focus && _focus->getUserData())
	{
		std::string package = *(std::string*)_focus->getUserData();
#ifdef USE_ANSY_CURLE
		GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(package);
		//update download apkState
		if (data && data->_apkState == DOWNLOAD_UNKNOW)
		{
			data->_apkState = DataManager::getInstance()->isDownloadFullGame(data->_basic.packageName);
		}

		if (data && ((data->_apkState == DOWNLOAD_UNLOAD) || 
			(data->_apkState == DOWNLOAD_CANCEL) || 
			(data->_apkState == DOWNLOAD_STOP) ||
			(data->_apkState == DOWNLOAD_UNDEFINE))){
			NetManager::getInstance()->downloadWithFile(DataManager::getInstance()->getApkDownloadURL(package), package);
			data->_apkState = DOWNLOAD_ING;
			DataManager::getInstance()->updateDownloadStateByMaxCount(package);
		}
#endif
	}
}

void GCatetoryDetailLayer::onClickOK()
{
	if (_focus)
		clickGameDetailItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void GCatetoryDetailLayer::clickGameDetailItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
    if (type != ui::Widget::TouchEventType::ENDED)
        return;
    auto btn = static_cast<ui::ImageView*>(sender);

	if (btn != _focus)
	{
		commonFocusAction(btn);
		ZM->showSelect(true, _focus->getContentSize() * FOCUS_SCALE_NUM, _pageView->convertToWorldSpace(_focus->getPosition()));
		_pageView->changeFocus(_focus);
		_pageView->getCurPage()->setUserObject(_focus);
		return;
	}

//    LS_LOG("game package: %d", *(std::string*)btn->getUserData());
	DataManager::getInstance()->_sUserData.gamePackageName = *(std::string*)btn->getUserData();
    DataManager::getInstance()->flushUserData();

	showGameDetail();
    LS_LOG("focus event... ... ");
}

void GCatetoryDetailLayer::pageViewEvent(Ref* sender, ui::PageView::EventType type)
{
	if (type != ui::PageView::EventType::TURNING)
		return;
	auto pageView = dynamic_cast<UIPageViewEx*>(sender);
	_categoryTitle->setPageTips(pageView->getCurPageIndex()+1, pageView->getPagesCount());
}

void GCatetoryDetailLayer::showGameDetail()
{
	ZM->changeThirdState(ThirdState::GAME_DETAIL);
}

void GCatetoryDetailLayer::changeFocus(EventKeyboard::KeyCode keyCode)
{
   
	Node *next = LsTools::getNextFocus(keyCode, _pageView->getCurPage());
	if (!next)//null
		next = LsTools::getNextPageFocus(_pageView, keyCode);//next = LsTools::getNextPageFocus(_pageView, _focus, keyCode);

	if (next)
	{
		commonFocusAction(next);
		ZM->showSelect(true, next->getContentSize() * FOCUS_SCALE_NUM, _pageView->convertToWorldSpace(next->getPosition()));
		_pageView->setUserObject(_focus);
		_pageView->changeFocus(next);
	}
	else
		ZM->showSelect(false);
}

void GCatetoryDetailLayer::showCurFocus()
{
	if (_focus)
		ZM->showSelect(true,
		this->_focus->getContentSize() * FOCUS_SCALE_NUM,
		_pageView->convertToWorldSpace(this->_focus->getPosition()));
	else
		ZM->showSelect(false);
}

void GCatetoryDetailLayer::updateAfterDownloadSuc()
{

}

Node *GCatetoryDetailLayer::getChildByTag( int tag )
{
    CCASSERT( _rootNode, "GCatetoryDetailLayer::_rootNode" );
    return _rootNode->getChildByTag( tag );
}

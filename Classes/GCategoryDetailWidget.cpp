//
//  GCategoryDetailWidget.cpp
//  xZone
//
//  Created by 苏明南 on 15/1/30.
//
//

#include "GCategoryDetailWidget.h"
#include "GCategoryDetail.h"
#include "DataManager.h"
#include "LsTools.h"
#include "ZoneManager.h"
#include "Global.h"

USING_NS_CC;

//游戏标题、数目、操作提示和华丽丽的分割线
GCategoryDetailTitle* GCategoryDetailTitle::create( cocos2d::Node *root, int categoryName )
{
    GCategoryDetailTitle *pRet = new GCategoryDetailTitle(root, categoryName);
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

GCategoryDetailTitle::GCategoryDetailTitle( cocos2d::Node *root, int categoryName ) : _root(root), _categoryName(categoryName) {}

GCategoryDetailTitle::~GCategoryDetailTitle() {}

bool GCategoryDetailTitle::init()
{
    if (!Node::init()) return false;
        //setTag
    {
		auto num = static_cast<ui::Text*>(LsTools::seekNodeByName(_root, "Text_GategoryGameNum"));
		num->setTag(CATETORYNUM);
    }

    //更新游戏列表名称和游戏数
    {
        updateUI();
    }

    return true;
}

void GCategoryDetailTitle::setVisible(bool visible)
{
    Node::setVisible( visible );
    _root->getChildByTag( CATEGORYNAME )->setVisible( visible );
    _root->getChildByTag( CATETORYNUM )->setVisible( visible );
    _root->getChildByTag( CATETORYLINE )->setVisible( visible );
    _root->getChildByTag( CATEGORYTIPS )->setVisible( visible );
}

bool GCategoryDetailTitle::isVisible() const
{
    return Node::isVisible();
}

void GCategoryDetailTitle::setPosAndSize(cocos2d::Vec2 &pos, cocos2d::Size &size)
{
	auto line = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_root, "10"));
	auto cTips_R = (LsTools::seekNodeByName(_root, "Category_Tips"));
	auto pTips_R = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_root, "Page_BG"));
	auto oTips_R = (LsTools::seekNodeByName(_root, "Op_Tips"));

	line->setContentSize(Size(size.width, line->getContentSize().height));
	line->setPosition(pos);

	Vec2 temp = Vec2(pos.x + size.width/2, pos.y + pTips_R->getContentSize().height / 2 + 5);
	pTips_R->setPosition(temp);

	auto fenlei = static_cast<ui::Text*>(LsTools::seekNodeByName(cTips_R, "Text_GameCategoryTitle"));
	auto cNumBg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(cTips_R, "Text_CategoryNum_BG"));
	fenlei->setPosition(Vec2(line->getLeftBoundary() + fenlei->getContentSize().width/2 + 10,temp.y));
	cNumBg->setPosition(Vec2(fenlei->getRightBoundary() + cNumBg->getContentSize().width / 2 + 3, temp.y));

	auto an1 = static_cast<ui::Text*>(LsTools::seekNodeByName(oTips_R, "Text_OperationTips_1"));
	auto menuBg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(oTips_R, "menu_Bg"));
	auto jian1 = static_cast<ui::Text*>(LsTools::seekNodeByName(oTips_R, "Text_OperationTips_2"));
	auto an2 = static_cast<ui::Text*>(LsTools::seekNodeByName(oTips_R, "Text_OperationTips_3"));
	auto okBg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(oTips_R, "ok_bg"));
	auto jian2 = static_cast<ui::Text*>(LsTools::seekNodeByName(oTips_R, "Text_OperationTips_4"));
	jian2->setPosition(Vec2(line->getRightBoundary() - jian2->getContentSize().width / 2 - 8, temp.y));
	okBg->setPosition(Vec2(jian2->getLeftBoundary() - okBg->getContentSize().width / 2, temp.y));
	an2->setPosition(Vec2(okBg->getLeftBoundary() - an2->getContentSize().width / 2, temp.y));
	jian1->setPosition(Vec2(an2->getLeftBoundary() - jian1->getContentSize().width / 2 - 18, temp.y));
	menuBg->setPosition(Vec2(jian1->getLeftBoundary() - menuBg->getContentSize().width / 2 - 3, temp.y));
	an1->setPosition(Vec2(menuBg->getLeftBoundary() - an1->getContentSize().width / 2 - 2, temp.y));
}

void GCategoryDetailTitle::setPageTips(int curPage, int pageNum)
{
	auto pTips_R = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_root, "Page_BG"));
	auto pageText = static_cast<ui::Text*>(LsTools::seekNodeByName(pTips_R, "PageView"));
	if (pageNum == 0) curPage = 0;
	std::string tex = std::string(int2str(curPage)) + "/" + std::string(int2str(pageNum));
	pageText->setString(tex);
}

void GCategoryDetailTitle::updateUI()
{
	std::string catName = std::string("fl") + std::string(int2str(_categoryName));
	int size = DataManager::getInstance()->getDetailGameByType(catName).size();

	auto cName = static_cast<ui::Text*>(LsTools::seekNodeByName(_root, "Text_GameCategoryTitle"));
	auto cNum = static_cast<ui::Text*>(LsTools::seekNodeByName(_root, "Text_GategoryGameNum"));
	cName->setString(DataManager::getInstance()->valueForKey(catName));
	cNum->setString(std::string(int2str(size)) + std::string(DataManager::getInstance()->valueForKey("kuan")));
}

//游戏详情
GCategoryGameDetail* GCategoryGameDetail::create(cocos2d::Node *root, std::string package)
{
    GCategoryGameDetail *pRet = new GCategoryGameDetail( root, package );
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

GCategoryGameDetail::GCategoryGameDetail(cocos2d::Node *root, std::string package) :_root(root), _currentGamePackage(package), _gameDetailInsRoot(NULL) 
{}

GCategoryGameDetail::~GCategoryGameDetail() {}

bool GCategoryGameDetail::init()
{
    if (!Node::init()) return false;
    _gameDetailInsRoot = CSLoader::createNode(s_GameDetailIndroduce);
    _root->addChild( _gameDetailInsRoot );
    
    ui::ImageView *node = _gameDetailInsRoot->getChildByName<ui::ImageView*>( "Sprite_GameSelect" );
    std::string des = node->getDescription();
    
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Sprite_GameSelect" )->setTag( CATEGORY_GAME_IMAGE_NAME );
    
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Common_Star01" )->setTag( CATEGORY_GAME_START_BASE + 0 );
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Common_Star02" )->setTag( CATEGORY_GAME_START_BASE + 1 );
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Common_Star03" )->setTag( CATEGORY_GAME_START_BASE + 2 );
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Common_Star04" )->setTag( CATEGORY_GAME_START_BASE + 3 );
    _gameDetailInsRoot->getChildByName<ui::Scale9Sprite*>( "Common_Star05" )->setTag( CATEGORY_GAME_START_BASE + 4 );
    
    _gameDetailInsRoot->getChildByName<ui::Text*>( "Text_GameScroll" )->setTag( CATEGORY_GAME_SCROLL );
    _gameDetailInsRoot->getChildByName<ui::Text*>( "Text_GameSize" )->setTag( CATEGORY_GAME_SIZE );
    _gameDetailInsRoot->getChildByName<ui::Text*>( "Text_GameVersion" )->setTag( CATEGORY_GAME_VERSION);
    _gameDetailInsRoot->getChildByName<ui::Text*>( "Text_GameLanguage" )->setTag( CATEGORY_GAME_LANGUAGE );
    _gameDetailInsRoot->getChildByName<ui::Text*>( "Text_GameDownload" )->setTag( CATEGORY_GAME_DOWNLOAD);
    
    updateUI();
    
	auto downPercent = static_cast<ui::Text*>(LsTools::seekNodeByName(_gameDetailInsRoot, "downPercent"));
	downPercent->setVisible(false);

	/*_loadSprite = Sprite::create("loading.png");
	_loadSprite->setPosition(downPercent->getPosition());
	downPercent->getParent()->addChild(_loadSprite);
	auto rotate = RotateBy::create(1.0f, 360);
	_loadSprite->runAction(RepeatForever::create(rotate));
	_loadSprite->setVisible(false);*/

    return  true;
}

void GCategoryGameDetail::setVisible(bool visible)
{
    Node::setVisible( visible );
    
    _gameDetailInsRoot->setVisible( visible );
}

bool GCategoryGameDetail::isVisible() const
{
    return Node::isVisible();
}

void GCategoryGameDetail::updateUI()
{
    if (_currentGamePackage == "")
    {
        setVisible( false );
        return;
    }
    setVisible( true );
	GameDetailData *data = DataManager::getInstance()->getDetailGameByPackage(_currentGamePackage);;
    
	std::string language = "";
	std::vector<std::string> des = LsTools::parStringByChar(data->_language);
	for (unsigned int i = 0; i < des.size(); ++i)
		language += std::string(DataManager::getInstance()->valueForKey(des[i])) + ";";

    std::string iconName	= data->_basic._icon;
	int         starNum		= atoi(data->_starNum.c_str());
	std::string       score		= data->_score;
	std::string       size = data->_size;
	std::string version		= data->_version;
	int         downNum		= atoi(data->_downloadNum.c_str());
    
    _gameDetailInsRoot->getChildByName<ui::ImageView*>( "Sprite_GameSelect" )->loadTexture( std::string(iconName) );
    for (int i = 0; i < 5; i++)
    {
        ui::ImageView* sprite = (ui::ImageView*)_gameDetailInsRoot->getChildByTag(CATEGORY_GAME_START_BASE+i);
        if (i < starNum)
            sprite->loadTexture( s_Star00 );
        else
            sprite->loadTexture( s_Star01 );
    }
    _gameDetailInsRoot->getChildByName<ui::Text*>("Text_GameScroll")->setString(score);
    _gameDetailInsRoot->getChildByName<ui::Text*>("Text_GameSize")->setString( size + std::string("M"));
    
    _gameDetailInsRoot->getChildByName<ui::Text*>("Text_GameVersion")->setString(version);
    _gameDetailInsRoot->getChildByName<ui::Text*>("Text_GameLanguage")->setString(language);
    _gameDetailInsRoot->getChildByName<ui::Text*>("Text_GameDownload")->setString(int2str(downNum));
}


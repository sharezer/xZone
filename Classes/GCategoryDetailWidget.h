//
//  GCategoryDetailWidget.h
//  xZone
//
//  Created by 苏明南 on 15/1/30.
//
//

#ifndef __xZone__GCategoryDetailWidget__
#define __xZone__GCategoryDetailWidget__

#include <stdio.h>
#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"

#include "DataManager.h"

class GCatetoryDetailLayer;

//列表名字，游戏数据、分割线和操作提示
class GCategoryDetailTitle : public cocos2d::Node
{
public:
    enum GCategoryDetailTitleTag
    {
        CATETORYLINE = 5010,
        CATETORYNUM  = 5011,
        CATEGORYNAME = 5012,
        CATEGORYTIPS = 5013
    };
    static GCategoryDetailTitle* create( cocos2d::Node *root, int categoryName );
		
public:
    virtual void setVisible(bool visible);
    virtual bool isVisible() const;
	void setPosAndSize(cocos2d::Vec2 &pos, cocos2d::Size &size);
	void setPageTips(int curPage, int pageNum);
    
CC_CONSTRUCTOR_ACCESS:
    GCategoryDetailTitle( cocos2d::Node *root, int category );
    virtual ~GCategoryDetailTitle();
    virtual bool init();
    void updateUI();
    
private:
    cocos2d::Node *_root;
    int            _categoryName;
};

//选中游戏的显示信息
class GCategoryGameDetail : public cocos2d::Node
{
public:
    enum GCategoryGameDetailTag
    {
        CATEGORY_GAME_START_BASE = 5100,
        CATEGORY_GAME_SCROLL     = 5105,
        CATEGORY_GAME_SIZE       = 5106,
        CATEGORY_GAME_VERSION    = 5107,
        CATEGORY_GAME_LANGUAGE   = 5108,
        CATEGORY_GAME_DOWNLOAD   = 5109,
        CATEGORY_GAME_IMAGE_NAME = 5110,
        CATEGORY_GAME_MAX        = 5111
    };
    static GCategoryGameDetail* create(cocos2d::Node *root, std::string package = "");   //-1 -> no select
    
public:
	void setCurrentGame(std::string package) { _currentGamePackage = package; updateUI(); }
    std::string  getCurrentGame() { return _currentGamePackage; }
    
    virtual void setVisible(bool visible);
    virtual bool isVisible() const;
    
CC_CONSTRUCTOR_ACCESS:
	GCategoryGameDetail(cocos2d::Node *root, std::string package);
    virtual ~GCategoryGameDetail();
    virtual bool init();
    void updateUI();
    
private:
    cocos2d::Node *_root;
    cocos2d::Node *_gameDetailInsRoot;
	cocos2d::Sprite *_loadSprite;
    std::string   _currentGamePackage;
};


#endif /* defined(__xZone__GCategoryDetailWidget__) */

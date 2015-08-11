//
//  GCategoryDetail.h
//  xZone
//
//  Created by 苏明南 on 15/1/29.
//
//

#ifndef __xZone__GCategoryDetail__
#define __xZone__GCategoryDetail__

#include <stdio.h>

#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"

class UIPageViewEx;
class GCategoryGameDetail;
class GCategoryDetailTitle;
//游戏列表详情页面
class GCatetoryDetailLayer : public BaseLayer {
public:
    enum ChildList{
        GAME_CATEGORY_DETAIL_TITLE = 5000,
        GAME_CATEGORY_DETAIL_LIST  = 5001,
        GAME_CATEGORY_DETAIL_GAME  = 5002
    };
public:
    //CREATE_FUNC(GCatetoryDetailLayer);
    static GCatetoryDetailLayer* create( int &categoryName );
    
	virtual void onClickBack();
    virtual void onClickMenu() override;
    virtual void onClickOK() override;

	virtual void flushUI() override;
    
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	virtual void showCurFocus();

	void updateAfterDownloadSuc();
	static void showGameDetail();

public:
    virtual cocos2d::Node *getChildByTag( int tag );
    
CC_CONSTRUCTOR_ACCESS:
    GCatetoryDetailLayer( int &categoryName );
    virtual ~GCatetoryDetailLayer();
    virtual bool init() override;
    
private:
    virtual bool initUI() override;
    virtual bool initData() override;

    /**
     *  触发游戏子选项
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickGameDetailItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void pageViewEvent(cocos2d::Ref* sender, cocos2d::ui::PageView::EventType type);

private:
	UIPageViewEx			  *_pageView;
	GCategoryDetailTitle	  *_categoryTitle;
    GCategoryGameDetail       *_gameDetailInd;
    int                       _categoryName;
};


#endif /* defined(__xZone__GCategoryDetail__) */

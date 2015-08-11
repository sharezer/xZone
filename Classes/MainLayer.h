//
//  MainLayer.h
//  xZone
//
//  Created by Sharezer on 15/1/19.
//
//

#ifndef __xZone__MainLayer__
#define __xZone__MainLayer__

#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"
#include "json/document.h"

class MainLayer : public BaseLayer {
public:
    CREATE_FUNC(MainLayer);
    
    virtual void onClickBack() override;
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
    
    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;

	//create ��Ӱ
	void createSpec(int index);
	void createSpecRecomend();

CC_CONSTRUCTOR_ACCESS:
    MainLayer();
    virtual ~MainLayer();
    virtual bool init() override;
    
private:
    virtual bool initUI() override;
    virtual bool initData() override;
	void initSpec();

	void updateRecommend(float dt);
	void changeTab(int index);
    /**
     *  ������ҳ��ѡ��
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickMainPageItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

	/**
     *  ������ҳ�����Ƽ�ҳ��ѡ��
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickMainPlusPageItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

    /**
     *  ����������ѡ��
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickCategoryPageItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

    /**
     *  ����������ѡ��
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickManagerPageItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);

	/**
     *  ��ҳ��ť����¼�
     *
     *  @param sender <#sender description#>
     *  @param type   <#type description#>
     */
    void clickMainBtn(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
    
    /**
     *  �л�����ʱ����ز���
     *
     *  @param keyCode      ����
     *  @param isStartOnEnd �Ƿ�����һ����ʼ�����������л�����һ��Pageʱʹ��
     */
    void doFocusChange(cocos2d::EventKeyboard::KeyCode keyCode, bool isStartOnEnd = false);
	void pageViewEvent(cocos2d::Ref* sender, cocos2d::ui::PageView::EventType type);
	
private:
    cocos2d::Node* _tabNode;
	cocos2d::ui::PageView* _pageView;
	cocos2d::ui::ImageView* _recommend;
	cocos2d::Sprite* _navi_Scelect;
	cocos2d::ui::Button* getCurTabBtn();

	int _recommendIndex;
	rapidjson::Value _recommendDoc, _recommend2Doc;

    enum ENUM_PAGE{
        MAIN_PAGE = 0,
		MAIN_PAGE_PLUS,
        CATEGORY_PAGE,
        MANAGE_PAGE,
        PAGE_COUNT
    };
    
	//�����ж��Ƿ����л���ҳ��״̬
    bool _isFocusNotOnTab;

    ENUM_PAGE _tabIndex;
	int _specIndex;

	bool _isUseSpec;
public:
	cocos2d::Node* _hideNode;
	cocos2d::Sprite* _bg;
};

#endif /* defined(__xZone__MainLayer__) */

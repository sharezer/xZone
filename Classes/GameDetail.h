//
//  GameDetail.h
//  xZone
//
//  Created by Sharezer on 15/2/3.
//
//

#ifndef __xZone__GameDetail__
#define __xZone__GameDetail__

#include "cocos2d.h"
#include "BaseLayer.h"
#include "LsTools.h"

class GCategoryGameDetail;
struct GameDetailData;

enum class DetailFocusType
{
	DL_BTN = 0,			//����
	COLLECT,		//�ղ�
	SCREEN_SHOT1,	//��ͼ1
	SCREEN_SHOT2,	//��ͼ2
	RECOMMEND,		//����Ȥ��Ϸ
	GAME_ICON
};

class GameDetail : public BaseLayer {
public:
	enum ShowState
	{
		NORMAL_STATE = 0,
		SCREEN_STATE = 1
	};
    static GameDetail* create(std::string &package);

	virtual void onClickBack();
    virtual void onClickMenu() override;
    virtual void onClickOK() override;
	//��װ�ɹ����ͼ������ɵ���
	virtual void flushUI() override;

    virtual void changeFocus(cocos2d::EventKeyboard::KeyCode keyCode) override;
	//�������֮�����UI
	void updateDownloadProg();

	cocos2d::Node *getRootNode() { return _rootNode; }

    void showVideoPlayFlag(bool isShow);
CC_CONSTRUCTOR_ACCESS:
	GameDetail(std::string &package);
    virtual ~GameDetail();
    virtual bool init() override;

private:
    virtual bool initUI() override;
    virtual bool initData() override;
	cocos2d::Node* gameDetailGetNextFocus(cocos2d::EventKeyboard::KeyCode keyCode);
	void clickOkItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void clickOkItemByTouch(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type);
	void nameRunAction(cocos2d::Node *node);

	//�������ذ�ť
	bool clickDownloadButton();

	void createDownloadProg(cocos2d::Vec2 pos);
	void createFullScreen();
	void backNormalState();
	//init ui
	void initScreenNode(GameDetailData *data);
	void initFavNode();
	void initICON(GameDetailData *data);
	void initGameInfor(GameDetailData *data);
	void initSummayText(GameDetailData *data);
	void initDownloadUI(GameDetailData *data);
	void initCollectNode(GameDetailData *data);
	void initQrencode(GameDetailData *data);

	std::string getApkPath();

	void downloadScreenShoot();
	void runIconQR();
private:
	cocos2d::Node      * _gameBG;
	cocos2d::Node	   * _downloadProgressRoot;
	ShowState			_state;
	DetailFocusType	_focusNum;
    std::string			_gamePackage;
	cocos2d::ui::Text *_focusName;
	float				_nameExtWidth;

	cocos2d::Node* _drawNode;
	cocos2d::Node* _iconNode;
    bool _isHaveVideo;
    cocos2d::Vector<cocos2d::Sprite*> _videoPlayFlagV;
};

#endif /* defined(__xZone__GameDetail__) */

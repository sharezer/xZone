//
//  DLManager.h
//  xPressZone
//
//  Created by 苏明南 on 15/6/13.
//
//

#ifndef __xPressZone__DLManager__
#define __xPressZone__DLManager__

#include <stdio.h>
#include "cocos2d.h"
#include "BaseLayer.h"
#include "Global.h"

class DLGameList : public cocos2d::ui::PageView{
public:
	struct GameListParams{
	public:
		float _fontSize;	//字体的大小，默认为20
		float _extenSize;	//字体背景的高度，默认为30
		int _rows;	//每页的行数，默认为3
		int _cols;	//每页的列数，默认为4
		float _rowsDiff;	//每行之间的间隙大小，默认为10
		float _colsDiff;	//每列之间的间隙大小，默认为5
		float _leftMargin;	//左边距，默认为5
		float _rightMargin;	//右边距，默认为5
		float _topMargin;	//顶边距，默认为5
		float _bottomMargin;//底边距，默认为5
		cocos2d::Size _imgSize;	//图标ICON显示大小，默认为(270, 270)
		std::string   _iconBg;	//图标ICON的背景图片，默认为Icon_MyGame.png
		std::string	  _fontName;  //字体名，默认为msyh
		GameListParams() : _fontSize(30), _extenSize(30), _rows(2), _cols(5), _rowsDiff(10), 
			_colsDiff(5), _leftMargin(5), _rightMargin(5), _topMargin(5), _bottomMargin(5),
			_imgSize(270, 270), _iconBg("Icon_MyGame.png"), _fontName("msyh.ttf")
		{}
	};
	enum GAME_TAG{
		TAG_LINE = 8887,
		TAG_BASE = 8888
	};

public:
	static DLGameList* create(std::vector< GameBasicData > &data, GameListParams par);
	//以下函数继承或者重写父类方法，目的是为了实现能够在垂直方向上的分页效果
public:
	/**
	* scroll pageview to index.
	*
	* @param idx    index of page.
	*/
	void scrollToPage(ssize_t idx);

	virtual void update(float dt) override;
	virtual void setContentSize(const cocos2d::Size& contentSize);
	//获取页数
	int getPagesCount() { return getPages().size(); }

	virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags) override;

protected:
	float getPositionYByIndex(ssize_t idx)const;

	virtual bool scrollPages(float touchOffset);
	void movePages(float offset);
	void updateAllPagesPosition();
	void autoScroll(float dt);

	virtual void handleMoveLogic(cocos2d::Touch *touch);
	virtual void handleReleaseLogic(cocos2d::Touch *touch);
	virtual void interceptTouchEvent(cocos2d::ui::Widget::TouchEventType event, cocos2d::ui::Widget* sender, cocos2d::Touch *touch);

	virtual void onSizeChanged() override;

	virtual void doLayout() override;
	//end
public:
	cocos2d::ui::Layout *getCurPage() { return getPage(getCurPageIndex()); }
	cocos2d::Node *getNextPageFocus(cocos2d::Node *curFocus, int dir = 0);
	void changeFocus(cocos2d::Node *focus, int state = 0);
	void updateFocusUI(std::string package);
	void updateAllUI(float dt);

CC_CONSTRUCTOR_ACCESS:
	DLGameList(std::vector< GameBasicData > &data, GameListParams par);
	virtual ~DLGameList();
	virtual bool init();
	void updateUI();
	cocos2d::Node *getLeftNextPageFocus(cocos2d::Node *curFocus);
	cocos2d::Node *getRightNextPageFocus(cocos2d::Node *curFocus);

	cocos2d::Vec2 getImgPosition(int col, int row);
	void reSetContentSize();

	void initScrollControl();
	cocos2d::ui::ImageView *initIconBackground(int i, int rn, int cn);
	cocos2d::Node *initIconImg(cocos2d::ui::ImageView* backIcon, int i);
	bool initGameName(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, int i);
	cocos2d::Node *initDLProgress(cocos2d::ui::ImageView* iconImg, int i);
	cocos2d::Node *initGameSize(cocos2d::ui::ImageView* iconImg, int i);
	cocos2d::Node *initOpControl(cocos2d::ui::ImageView* backIcon, int i);
	void setScale9Sprite(cocos2d::ui::ImageView *img, float w, float h);

private:
	std::vector< GameBasicData > _data;
	cocos2d::Node *_scrollLine;
	cocos2d::Node *_focus;
	GameListParams _params;
};

#endif /* defined(__xPressZone__DLManager__) */

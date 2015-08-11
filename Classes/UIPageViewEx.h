//
//  UIPageViewEx.h
//  xZone
//
//  Created by 苏明南 on 15/3/19.
//
//

#ifndef __xZone__UIPageViewEx__
#define __xZone__UIPageViewEx__

#include <stdio.h>
#include "cocos2d.h"
#include "Global.h"
#include "BaseLayer.h"

//游戏列表
class UIPageViewEx : public cocos2d::ui::PageView
{
public:
	enum PageDirection
	{
		VERTICAL,	//垂直(left->up,right->down)
		HORIZONTAL, //水平(默认情况)
	};
	enum ScoreType
	{
		HOR_TYPE,	//跟名字水平对齐（默认情况）
		VER_TYPE,	//跟名字垂直对齐（斜对齐）
	};
	/*
	定义游戏列表的属性以及规则
	*/
	struct PageViewParams
	{
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
		std::string   _stencilBg; //图标ICON的模板图片，默认为Icon_MyGame.png
		std::string	  _fontName;  //字体名，默认为msyh
		PageDirection _dir;	//分页的方向，默认为水平方向
		ScoreType	  _sType;	//对齐方式，默认为水平对齐
		cocos2d::Size _sSize;	//分数背景大小
		std::string   _sBg;		//分数背景图标
		bool _isShowScore;	//是否显示评分，默认不显示
		bool _isShowScoll;	//是否显示滚动条，默认不显示
		bool _isLastUseFullImg;	//是否在最后一个图标上显示文字，默认不显示
		bool _isShowAleadyInstall;	//是否显示已安装按钮，默认不显示

		PageViewParams(float fs = 20, float es = 30, int r = 3, int c = 4, float rd = 10,
			float cd = 5, float lm = 5, float rm = 5, float tm = 5, float bm = 5, 
			PageDirection dir = PageDirection::HORIZONTAL, bool ss = false, 
			bool sl = false, cocos2d::Size isize = cocos2d::Size(270, 270), 
			std::string ib = "Icon_MyGame.png", std::string sb = "Icon_MyGame.png", 
			std::string fn = s_labelTTF, bool isUse = false) :
			_fontSize(fs), _extenSize(es), _rows(r), _cols(c), _rowsDiff(rd), _colsDiff(cd),
			_leftMargin(lm), _rightMargin(rm), _topMargin(tm), _bottomMargin(bm),
			_dir(dir), _isShowScore(ss), _isShowScoll(sl), _imgSize(isize), _iconBg(ib), _stencilBg(sb),
			_fontName(fn), _isLastUseFullImg(isUse), _sType(ScoreType::HOR_TYPE), _sSize(90, 40), _sBg("Common_BG_Black.png"),
			_isShowAleadyInstall(false)
		{}
	};

public:
	enum GCategoryDetailScrollTag
	{
		CATEGORY_PAGE_VIEW = 5050,
		CATEGORY_PAGE_LINE = 5051,
		CATEGORY_PAGE_LINE_SEL = 5052,
		CATEGORY_BASE_NO = 5053
	};
	static UIPageViewEx* create(std::vector< GameBasicData > &data, PageViewParams par, bool isSelect = false);
	//以下函数继承或者重写父类方法，目的是为了实现能够在垂直方向上的分页效果
public:
	/**
	* scroll pageview to index.
	*
	* @param idx    index of page.
	*/
	void scrollToPage(ssize_t idx);
	void flushIcon();
	virtual void update(float dt) override;
	virtual void setContentSize(const cocos2d::Size& contentSize);
	//获取页数
	int getPagesCount() { return getPages().size(); }

	virtual void visit(cocos2d::Renderer *renderer, const cocos2d::Mat4 &parentTransform, uint32_t parentFlags) override;
	void addFlushItem(std::string key);
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

public:
	bool setGameData(std::vector< GameBasicData > &data);
	const std::vector< GameBasicData > &getGameData() const { return _data; }

	cocos2d::ui::Layout *getCurPage() { return getPage(getCurPageIndex()); }
	virtual cocos2d::Node *getNextPageFocus(cocos2d::Node *curFocus, int dir = 0);
	void changeFocus(cocos2d::Node *focus);
CC_CONSTRUCTOR_ACCESS:
	UIPageViewEx(std::vector< GameBasicData > &data, PageViewParams par, bool isSelect);
	virtual ~UIPageViewEx();
	virtual bool init();
	void updateUI();
	virtual cocos2d::Node *getLeftNextPageFocus(cocos2d::Node *curFocus);
	virtual cocos2d::Node *getRightNextPageFocus(cocos2d::Node *curFocus);

	cocos2d::Vec2 getImgPosition(int col, int row, bool isUseText);
	void reSetContentSize();

	cocos2d::ui::ImageView *createImageView(std::string url, int i, int col, int row, bool isUseText);
	cocos2d::ui::ImageView *createImageView(cocos2d::RenderTexture *rt, int i, int col, int row, bool isUseText);
	bool createRootFromDisk(cocos2d::Node *root, int i, int colNum, int rowNum, std::string path, bool isUseText);

	void initScrollControl();
	cocos2d::ui::ImageView *initIconBackground(int i, int rn, int cn, bool b);
	cocos2d::Node *initIconImg(cocos2d::ui::ImageView* backIcon, int i, bool b);
	bool initGameName(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, int i, bool b);
	cocos2d::Node *initTextScore(float score, cocos2d::Node *cnode, cocos2d::ui::Text *tnode, cocos2d::Node *back);
	cocos2d::Node *initSelImg(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, bool b);
	cocos2d::Node *initInstallImg(cocos2d::ui::ImageView *backIcon, GameBasicData &data);

	void updateIcons(float dt);
private:
	std::vector< GameBasicData > _data;
	cocos2d::Node *_scrollLine;
	cocos2d::ui::ImageView *_focus;
	float _nameExtWidth;
	PageViewParams _params;
	bool _isShowSelImg;
	cocos2d::Map<std::string, cocos2d::ui::ImageView*> _iconMap;
	std::mutex _iconMutex;
	std::list<std::string> _flushList;
	
};

#endif /* defined(__xZone__UIPageViewEx__) */

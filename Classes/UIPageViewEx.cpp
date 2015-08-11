//
//  UIPageViewEx.cpp
//  xZone
//
//  Created by 苏明南 on 15/3/19.
//
//

#include "UIPageViewEx.h"
#include "DataManager.h"
#include "LsTools.h"
#include "ZoneManager.h"

USING_NS_CC;

UIPageViewEx* UIPageViewEx::create(std::vector< GameBasicData > &data, PageViewParams par, bool isSelect)
{
	UIPageViewEx *pRet = new UIPageViewEx(data, par, isSelect);
	if (pRet && pRet->init())
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

UIPageViewEx::UIPageViewEx(std::vector< GameBasicData > &data, PageViewParams par, bool isSelect)
	: _scrollLine(nullptr)
	, _isShowSelImg(isSelect)
	, _params(par)
	, _focus(nullptr)
	, _nameExtWidth(0)
	, _data(data)
{
	//setGameData(data);
}

UIPageViewEx::~UIPageViewEx()
{
	_iconMap.clear();
	_flushList.clear();
}

bool UIPageViewEx::init()
{
	if (!ui::PageView::init()) return false;

	CCASSERT(!(_params._rows < 0), "UIPageViewEx::rows < 0");
	CCASSERT(!(_params._cols < 0), "UIPageViewEx::cols < 0");

	this->setTag(CATEGORY_PAGE_VIEW);
	//2. 更新游戏库
	setGameData(_data);
	initScrollControl();
	ZM_PAGEVIEW = this;

	//Add by LS on 2015-06-26 start
	setCustomScrollThreshold(MY_SCREEN.height / 8);
	setUsingCustomScrollThreshold(true);
	//Add by LS on 2015-06-26 end

	schedule(CC_SCHEDULE_SELECTOR(UIPageViewEx::updateIcons), 0.05f);
	return true;
}

bool UIPageViewEx::setGameData(std::vector< GameBasicData > &data)
{
	_data = data;
	updateUI();

	return true;
}

//0->left,1->right
Node *UIPageViewEx::getNextPageFocus(cocos2d::Node *curFocus, int dir /* = 0 */)
{
	switch (dir)
	{
	case 0://left，跳到上一页
		return getLeftNextPageFocus(curFocus);
	case 1://right，跳到下一页
		return getRightNextPageFocus(curFocus);
	}
	return nullptr;
}

void UIPageViewEx::changeFocus(cocos2d::Node *focus)
{
	if (!focus)
		return;

	if (_focus && _focus->getContentSize().width > _params._imgSize.width)
	{
		_focus->stopAllActions();
		_focus->setPositionX(_nameExtWidth);
	}

	auto name = static_cast<ui::ImageView*>(LsTools::seekNodeByName(focus, "gameName"));
	if (name && name->getContentSize().width > 270)
	{
		float x = name->getPositionX();
		_nameExtWidth = x;
		auto moveBy1 = MoveBy::create(x / 25.0f, Vec2(-2 * x, 0));
		auto moveBy2 = MoveBy::create(x / 25.0f, Vec2( 2 * x, 0));
		auto timeDelay = DelayTime::create(1);
		auto seq = Sequence::create(moveBy1, timeDelay, moveBy2, timeDelay, nullptr);
		name->runAction(RepeatForever::create(seq));
		_focus = name;
	}
}

void UIPageViewEx::updateUI()
{
	Director::getInstance()->getTextureCache()->reloadTexture(_params._iconBg);
	Director::getInstance()->getTextureCache()->reloadTexture(s_DefaultIcon);
	//设置控件的显示大小
	{
		reSetContentSize();
	}
	//1. 删除列表控件下的所有游戏控件
	removeAllPages();
	//add new game data
	{
		int OnePageNum = _params._rows * _params._cols;
		//添加华丽丽的游戏控件
		int pageCount = _data.size() / OnePageNum + (_data.size() % OnePageNum ? 1 : 0);
		for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex)
		{
			ui::Layout* layout = ui::Layout::create();
			layout->setContentSize(getContentSize());
			for (int i = pageIndex * OnePageNum; i < (int)_data.size() && i < (pageIndex + 1) * OnePageNum; ++i)
			{
				GameBasicData data = _data[i];
				int j = i % OnePageNum;
				int colNum = j / _params._cols;	//在当前页下的行号和列号
				int rowNum = j % _params._cols;

				bool isUseText = !(_params._isLastUseFullImg && i == _data.size() - 1);
				std::string path = LsTools::lsStandardPath(LsTools::getDataPath()
					+ "/data/game/iconCopy/" + _data[i].packageName + ".png");

				if (createRootFromDisk(layout, i, colNum, rowNum, path, isUseText))
					continue;

				//创建游戏背景节点
				auto backIcon = initIconBackground(i, rowNum, colNum, isUseText);
				//创建游戏图标节点
				auto iconNode = initIconImg(backIcon, i, isUseText);
				//创建名字节点，返回值表示是否名字超过底图的宽度
				bool isExit = initGameName(backIcon, iconNode, i, isUseText);

#ifdef USE_NODE_SAVE
				//if ((!isExit && isUseText) && !_params._isDirectIcon && LsTools::isValidFor_Stencil24Depth8())
				if ((!isExit && isUseText) && data._isStencil && LsTools::isValidFor_Stencil24Depth8())
				{
					auto rt = LsTools::nodeToSavePng(backIcon, path, false);
					//auto img = createImageView(path, i, colNum, rowNum, isUseText);
					auto img = createImageView(rt, i, colNum, rowNum, isUseText);
					auto installNode = initInstallImg(img, data);
					//创建选中节点
					auto selNode = initSelImg(img, iconNode, isUseText);
					layout->addChild(img);
				}
				else
				{
					auto installNode = initInstallImg(backIcon, data);
					//创建选中节点
					auto selNode = initSelImg(backIcon, iconNode, isUseText);
					layout->addChild(backIcon);
				}
#else
				auto installNode = initInstallImg(backIcon, data);
				//创建选中节点
				auto selNode = initSelImg(backIcon, iconNode, isUseText);
				layout->addChild(backIcon);
#endif
			}
			insertPage(layout, pageIndex);
		}
	}
}

void UIPageViewEx::addFlushItem(std::string key)
{
	_flushList.push_back(key);
}

cocos2d::Node *UIPageViewEx::getLeftNextPageFocus(cocos2d::Node *curFocus)
{
	int pageIndex = getCurPageIndex();
	switch (_params._dir)
	{
	case HORIZONTAL:
	{
		//获取上一页的焦点控件在哪一行
		cocos2d::ui::Layout *layout = getPage(pageIndex + 1);
		int num = (int)(curFocus->getTag() - CATEGORY_BASE_NO + 1) - (pageIndex + 1) * (_params._cols * _params._rows);
		int rowNo = (num / _params._cols) + 1;
		layout->setUserObject(nullptr);
		//设置当前页的焦点
		layout = getCurPage();
		int curNum = rowNo * _params._cols + pageIndex * (_params._rows * _params._cols) - 1;
		cocos2d::Node *curNode = layout->getChildByTag(CATEGORY_BASE_NO + curNum);
		layout->setUserObject(curNode);
		return curNode;
	}
	case VERTICAL:
	{
		//获取上一页的焦点控件在哪一列
		cocos2d::ui::Layout *layout = getPage(pageIndex + 1);
		int num = (int)(curFocus->getTag() - CATEGORY_BASE_NO + 1) - (pageIndex + 1) * (_params._cols * _params._rows);
		int colNo = ((num - 1) % _params._cols) + 1;
		layout->setUserObject(nullptr);
		//设置当前页的焦点
		layout = getCurPage();
		int curNum = (_params._rows * _params._cols) - (_params._cols - colNo + 1) + pageIndex * (_params._rows * _params._cols);
		cocos2d::Node *curNode = layout->getChildByTag(CATEGORY_BASE_NO + curNum);
		layout->setUserObject(curNode);
		return curNode;
	}
	}
	return nullptr;
}

cocos2d::Node *UIPageViewEx::getRightNextPageFocus(cocos2d::Node *curFocus)
{
	int pageIndex = getCurPageIndex();
	switch (_params._dir)
	{
	case HORIZONTAL:
	{
		//获取上一页的焦点控件在哪一行
		cocos2d::ui::Layout *layout = getPage(pageIndex - 1);
		int num = (int)(curFocus->getTag() - CATEGORY_BASE_NO) - (pageIndex - 1) * (_params._rows * _params._cols);
		int rowNo = (num / _params._cols) + 1;
		layout->setUserObject(nullptr);
		//设置当前页的焦点
		layout = getCurPage();
		int curNum = (rowNo - 1) * _params._cols + pageIndex * (_params._rows * _params._cols);
		if (layout->getChildrenCount() < curNum - pageIndex * (_params._rows * _params._cols))
			curNum = pageIndex * (_params._rows * _params._cols);
		cocos2d::Node *curNode = layout->getChildByTag(CATEGORY_BASE_NO + curNum);
		layout->setUserObject(curNode);
		return curNode;
	}
	break;
	case VERTICAL:
	{
		//获取上一页的焦点控件在哪一列
		cocos2d::ui::Layout *layout = getPage(pageIndex - 1);
		int num = (int)(curFocus->getTag() - CATEGORY_BASE_NO) - (pageIndex - 1) * (_params._rows * _params._cols);
		int colNo = (num % _params._cols);
		layout->setUserObject(nullptr);
		//设置当前页的焦点
		layout = getCurPage();
		if (layout->getChildrenCount() < colNo + 1)
			colNo = layout->getChildrenCount() - 1;
		cocos2d::Node *curNode = layout->getChildByTag(CATEGORY_BASE_NO + colNo + pageIndex * (_params._rows * _params._cols));
		layout->setUserObject(curNode);
		return curNode;
	}
	break;
	}
	return nullptr;
}

cocos2d::Vec2 UIPageViewEx::getImgPosition(int col, int row, bool isUseText)
{
	int dety = col * _params._colsDiff;
	int detx = row * _params._rowsDiff;
	Vec2 position = (Vec2(row * _params._imgSize.width + detx + _params._imgSize.width / 2.0f + _params._leftMargin,
		getContentSize().height - col * _params._imgSize.height - dety - _params._imgSize.height / 2.0f
		- col * _params._extenSize - _params._extenSize / 2 - _params._topMargin));

	return position;
}

void UIPageViewEx::reSetContentSize()
{
	float width = _params._cols * _params._imgSize.width + (_params._cols - 1) * _params._rowsDiff
		+ _params._leftMargin + _params._rightMargin;
	float height = _params._rows * _params._imgSize.height + (_params._rows - 1) * _params._colsDiff
		+ _params._rows * _params._extenSize + _params._topMargin + _params._bottomMargin;

	setContentSize(Size(width, height));
}

cocos2d::ui::ImageView *UIPageViewEx::createImageView(std::string url, int i, int col, int row, bool isUseText)
{
	auto img = ui::ImageView::create(url);
	img->setName(int2str(i));
	img->setScale9Enabled(true);
	img->setCapInsets(Rect(
		SELECT_CAP_SIZE,
		SELECT_CAP_SIZE,
		img->getContentSize().width - 2 * SELECT_CAP_SIZE,
		img->getContentSize().height - 2 * SELECT_CAP_SIZE));
	img->setPosition(getImgPosition(col, row, isUseText));
	img->setUserData(&_data[i].packageName);
	img->setTag(CATEGORY_BASE_NO + i);
	img->setTouchEnabled(true);

	return img;
}

cocos2d::ui::ImageView *UIPageViewEx::createImageView(cocos2d::RenderTexture *rt, int i, int col, int row, bool isUseText)
{
	rt->getSprite()->getTexture()->setAntiAliasTexParameters();
	auto img = UIImageViewWithTexture::create(rt->getSprite()->getTexture());
	img->setName(int2str(i));
	img->setScale9Enabled(true);
	img->setCapInsets(Rect(
		SELECT_CAP_SIZE,
		SELECT_CAP_SIZE,
		img->getContentSize().width - 2 * SELECT_CAP_SIZE,
		img->getContentSize().height - 2 * SELECT_CAP_SIZE));
	img->setPosition(getImgPosition(col, row, isUseText));
	img->setUserData(&_data[i].packageName);
	img->setTag(CATEGORY_BASE_NO + i);
	img->setTouchEnabled(true);

	return img;
}

bool UIPageViewEx::createRootFromDisk(cocos2d::Node *root, int i, int colNum, int rowNum, std::string path, bool isUseText)
{
	//if (params.isDirectIcon&& (!LsTools::isValidFor_Stencil24Depth8()))
	if (!_data[i]._isStencil && (!LsTools::isValidFor_Stencil24Depth8()))
		return false;

#ifdef USE_NODE_SAVE
	auto name = ui::Text::create(_data[i]._name, _params._fontName, _params._fontSize);
	auto isCreate = (name->getContentSize().width > _params._imgSize.width);
	if (!isCreate || !isUseText)
	{
		if (FileUtils::getInstance()->isFileExist(path))
		{
			auto img = createImageView(path, i, colNum, rowNum, isUseText);
			auto installNode = initInstallImg(img, _data[i]);
			root->addChild(img);
			return true;
		}
	}
#endif

	return false;
}

void UIPageViewEx::initScrollControl()
{
	if (_params._isShowScoll && !_scrollLine)
	{
		//create scroll line
		{
			_scrollLine = Node::create();
			_scrollLine->setName("scrollLine");
			this->addChild(_scrollLine);
			{
				auto lineBack = ui::ImageView::create("Common_rollline02.png");
				lineBack->setAnchorPoint(cocos2d::Vec2(0.5f, 0));
				_scrollLine->addChild(lineBack);
				lineBack->setTag(CATEGORY_PAGE_LINE);
			}
			{
				auto lineSel = ui::Scale9Sprite::create("Common_rollline01.png");
				lineSel->setTag(CATEGORY_PAGE_LINE_SEL);
				lineSel->setName("lineSelect");
				_scrollLine->addChild(lineSel);
			}
		}
	}
	//
	if (_scrollLine && _params._isShowScoll)
	{
		auto lineSel = static_cast<ui::Scale9Sprite*>(_scrollLine->getChildByTag(CATEGORY_PAGE_LINE_SEL));
		float lineHeight = 0;
		if (getPageCount() == 0)
			lineHeight = getContentSize().height;
		else
			lineHeight = getContentSize().height / getPageCount();
		lineSel->setContentSize(Size(10, lineHeight));
		lineSel->setPositionY(getContentSize().height - lineHeight / 2);
	}
}

cocos2d::ui::ImageView *UIPageViewEx::initIconBackground(int i, int rn, int cn, bool b)
{
	auto backIcon = ui::ImageView::create(_params._iconBg);
	backIcon->setName(std::string(int2str(i)));
	backIcon->setScale9Enabled(true);
	backIcon->setCapInsets(Rect(
		SELECT_CAP_SIZE,
		SELECT_CAP_SIZE,
		backIcon->getContentSize().width - 2 * SELECT_CAP_SIZE,
		backIcon->getContentSize().height - 2 * SELECT_CAP_SIZE));
	backIcon->setAnchorPoint(Vec2(0.5f, 0.5f));
	backIcon->setContentSize(Size(_params._imgSize.width, _params._imgSize.height + _params._extenSize));
	backIcon->setPosition(getImgPosition(cn, rn, b));
	backIcon->setUserData(&_data[i].packageName);
	backIcon->setTag(CATEGORY_BASE_NO + i);
	backIcon->setTouchEnabled(true);

	return backIcon;
}

cocos2d::Node *UIPageViewEx::initIconImg(cocos2d::ui::ImageView* backIcon, int i, bool b)
{
	Size addSize(0, 0);
	if (!b)
		addSize.height = _params._extenSize;
	LS_LOG("package: %s", _data[i].packageName.c_str());
	std::string iconPath = LsTools::lsStandardPath(_data[i]._icon);
	auto gameBt = cocos2d::ui::ImageView::create();

	if (!_data[i].packageName.empty() && _data[i].packageName.size() > 0)
	{
		_iconMap.insert(iconPath, gameBt);
		if (!FileUtils::getInstance()->isFileExist(iconPath)){
			_iconMap.insert(iconPath, gameBt);
			NetManager::getInstance()->loadFile(ZM->getServerAddress() + "?" +
				formatStr("commend=%d&game=%s", CommendEnum::DOWNLOAD_ICON, _data[i].packageName.c_str()),
				LsTools::lsStandardPath(formatStr("%s/%s/", LsTools::getDataPath().c_str(), s_GameIconPath)),
				_data[i].packageName + ".png",
				[=]{
				/*std::string tempFile = LsTools::lsStandardPath(formatStr("%s/%s/%s_temp.png", LsTools::getDataPath().c_str(), s_GameIconPath, _data[i].packageName.c_str()));
				std::string saveFile = LsTools::lsStandardPath(formatStr("%s/%s/%s.png", LsTools::getDataPath().c_str(), s_GameIconPath, _data[i].packageName.c_str()));
				LsTools::copyFileForPath(tempFile, saveFile);*/
				if (ZM_PAGEVIEW)
					ZM_PAGEVIEW->addFlushItem(iconPath);
				//LsTools::removeFileForPath(tempFile);
			}, nullptr, false);
		}
		else
			addFlushItem(iconPath);
		iconPath = s_DefaultIcon;
	}

	gameBt->loadTexture(LsTools::lsStandardPath(iconPath));
	gameBt->setContentSize(_params._imgSize + addSize);
	gameBt->ignoreContentAdaptWithSize(false);

	if (_data[i]._isStencil)//需要做模板缓存切图
	{
		auto stencil = Sprite::create(_params._stencilBg);
		auto clipNode = ClippingNode::create(stencil);
		clipNode->setContentSize(_params._imgSize + addSize);

		clipNode->setPosition(Vec2(clipNode->getContentSize().width / 2,
			backIcon->getContentSize().height - clipNode->getContentSize().height / 2));
		clipNode->setAlphaThreshold(0.1f);
		clipNode->addChild(gameBt);
		backIcon->addChild(clipNode, 1);
		return clipNode;
	}
	else//直接使用ICON
	{
		gameBt->setPosition(Vec2(gameBt->getContentSize().width / 2,
			backIcon->getContentSize().height - gameBt->getContentSize().height / 2));
		backIcon->addChild(gameBt, 1);
		return gameBt;
	}
}

bool UIPageViewEx::initGameName(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, int i, bool b)
{
	if (b)
	{
		auto name = ui::Text::create(_data[i]._name, _params._fontName, _params._fontSize);
		name->setName("gameName");
		Vec2 pos = Vec2(icon->getPositionX(), _params._extenSize / 2);
		if (_params._isShowScore)
		{
			float score = std::atof(DataManager::getInstance()->getDetailGameByPackage(_data[i].packageName)->_score.c_str());
			backIcon->addChild(initTextScore(score, icon, name, backIcon), 1);
		}

		if (name->getContentSize().width > _params._imgSize.width)
		{
			auto sten = Sprite::create("Icon_MyGame_txt.png");
			auto nClip = ClippingNode::create(sten);
			nClip->addChild(name);
			nClip->setName(backIcon->getName() + "game_Name");
			nClip->setPosition(pos);

			float tmp = (name->getContentSize().width - 270) / 2.0f;
			tmp += 10;
			name->setPositionX(tmp);

			backIcon->addChild(nClip, 1);
			return true;
		}
		name->setPosition(pos);
		backIcon->addChild(name, 1);
		return false;
	}
	return false;
}

cocos2d::Node *UIPageViewEx::initTextScore(float score, cocos2d::Node *cnode, cocos2d::ui::Text *tnode, Node *back)
{
	float width = tnode->getContentSize().width;
	Vec2 pos = Vec2(cnode->getPositionX(), _params._extenSize / 2);
	switch (_params._sType)
	{
	case ScoreType::HOR_TYPE:
	{
		auto scoreText = ui::Text::create(floatFormat("%0.1f", score), _params._fontName, _params._fontSize);
		Vec2 sText = Vec2(width - scoreText->getContentSize().height / 2 - 15, _params._extenSize / 2);
		scoreText->setPosition(sText);
		return scoreText;
	}
	break;
	case ScoreType::VER_TYPE:
	{
		auto scoreBG = LayerColor::create(Color4B(0, 0, 0, 180), _params._sSize.width, _params._sSize.height);
		scoreBG->setAnchorPoint(Vec2(0.5f, 0.5f));
		pos = Vec2(_params._imgSize.width - _params._sSize.width / 2, cnode->getPosition().y -
			_params._imgSize.height / 2 + _params._sSize.height / 2);
		scoreBG->setPosition(pos - Vec2(_params._sSize.width / 2, _params._sSize.height / 2));

		auto str = std::string(floatFormat("%0.1f", score)) + std::string(DataManager::getInstance()->valueForKey("score"));
		auto scoreText = ui::Text::create(str, _params._fontName, _params._fontSize);
		scoreBG->addChild(scoreText);
		scoreText->setPosition(Vec2(_params._sSize.width / 2.0f, _params._sSize.height / 2.0f));

		return scoreBG;
	}
	break;
	}
	return nullptr;
}

cocos2d::Node *UIPageViewEx::initSelImg(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, bool b)
{
	//添加游戏选中的图片信息
	{
		if (_isShowSelImg && b)//Vec2(right-7, top-2);
		{
			auto selBtn = ui::Button::create("MyGame_select01.png", "MyGame_select02.png", "MyGame_select02.png");
			selBtn->setName("selectBtn");
			selBtn->setVisible(false);
			selBtn->setTouchEnabled(false);
			float x = icon->getPositionX() + icon->getContentSize().width / 2 - selBtn->getContentSize().width / 2 - 7;
			float y = icon->getPositionY() + icon->getContentSize().height / 2 - selBtn->getContentSize().height / 2 - 2;
			selBtn->setPosition(Vec2(x, y));
			backIcon->addChild(selBtn, 1);
			return selBtn;
		}
	}

	return nullptr;
}

cocos2d::Node *UIPageViewEx::initInstallImg(cocos2d::ui::ImageView *backIcon, GameBasicData &data)
{
	std::string version = DataManager::getInstance()->isApkInstall(data.packageName);
	if (!_params._isShowAleadyInstall || version.empty())
		return nullptr;
	auto aleadyInstall = ui::ImageView::create("Class_Detail_Install.png");
	if (!aleadyInstall) return nullptr;

	aleadyInstall->setAnchorPoint(Vec2(0.5f, 0.5f));
	Size s = aleadyInstall->getContentSize();
	aleadyInstall->setPosition(Vec2(s.width / 2.0f, backIcon->getContentSize().height - s.height / 2.0f));
	backIcon->addChild(aleadyInstall, 1);

	return aleadyInstall;
}

void UIPageViewEx::scrollToPage(ssize_t idx)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::scrollToPage(idx);
		return;
	}
	if (idx < 0 || idx >= this->getPageCount())
	{
		return;
	}
	_curPageIdx = idx;
	Layout* curPage = _pages.at(idx);
	_autoScrollDistance = -curPage->getPosition().y;
	_autoScrollSpeed = fabs(_autoScrollDistance) / 0.2f;
	_autoScrollDirection = _autoScrollDistance > 0 ? AutoScrollDirection::LEFT : AutoScrollDirection::RIGHT;
	_isAutoScrolling = true;

	if (_scrollLine && _params._isShowScoll)
	{
		auto lineSel = static_cast<ui::Scale9Sprite*>(_scrollLine->getChildByTag(CATEGORY_PAGE_LINE_SEL));
		float lineHeight = 0;
		if (getPageCount() == 0)
			lineHeight = getContentSize().height;
		else
			lineHeight = getContentSize().height / getPageCount();
		lineSel->setPositionY(getContentSize().height - lineHeight * _curPageIdx - lineHeight / 2);
	}
}

void UIPageViewEx::update(float dt)
{
	if (_isAutoScrolling)
	{
		this->autoScroll(dt);
	}
}

void UIPageViewEx::updateIcons(float dt)
{
	if (_iconMap.size() <= 0){
		unschedule(CC_SCHEDULE_SELECTOR(UIPageViewEx::updateIcons));
		return;
	}

	if (_flushList.size() > 0)
	{
		if (FileUtils::getInstance()->isFileExist(_flushList.front()) && _iconMap.find(_flushList.front()) != _iconMap.end()){
			//auto textureName = FileUtils::getInstance()->fullPathForFilename(_flushList.front());
			auto textureName = _flushList.front();
			LS_LOG("updateIcons: %s", textureName.c_str());
			auto item = _iconMap.at(textureName);
			item->loadTexture(textureName);
			_iconMap.erase(textureName);
			_flushList.pop_front();
		}
	}


	//if (_flushList.size() > 0)
	//{
	//	for (auto &child : _flushList){
	//		//LS_LOG("child:%s", child.c_str());
	//		if (FileUtils::getInstance()->isFileExist(child) && _iconMap.find(child) != _iconMap.end()){
	//			_iconMap.at(child)->loadTexture(child);
	//			_iconMap.erase(child);
	//		}
	//	}
	//	_flushList.clear();
	//}
}

void UIPageViewEx::setContentSize(const cocos2d::Size& contentSize)
{
	PageView::setContentSize(contentSize);
	//update scroll line
	if (_scrollLine && _params._isShowScoll)
	{
		_scrollLine->setPosition(contentSize.width - 30, 0);
		auto lineBack = static_cast<ui::ImageView*>(_scrollLine->getChildByTag(CATEGORY_PAGE_LINE));
		auto size = Size(1, contentSize.height);
		lineBack->setContentSize(size);
		//	auto lineBack = static_cast<DrawNode*>(_scrollLine->getChildByTag(CATEGORY_PAGE_LINE));
		//	lineBack->clear();
		//	lineBack->drawLine(Vec2(0, 0), Vec2(0, contentSize.height), Color4F::WHITE);
	}
}

void UIPageViewEx::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	if (parentFlags & FLAGS_TRANSFORM_DIRTY)
	{
		_clippingRectDirty = true;
		getClippingRect();
	}
	ui::Layout::visit(renderer, parentTransform, parentFlags);
}


float UIPageViewEx::getPositionYByIndex(ssize_t idx)const
{
	return (getContentSize().height * (_curPageIdx - idx));
}

bool UIPageViewEx::scrollPages(float touchOffset)
{
	if (_params._dir == HORIZONTAL)
		return PageView::scrollPages(touchOffset);

	if (this->getPageCount() <= 0)
	{
		return false;
	}

	if (!_leftBoundaryChild || !_rightBoundaryChild)
	{
		return false;
	}
	float topB = _leftBoundaryChild->getTopBoundary();
	float botB = _rightBoundaryChild->getBottomBoundary();

	float realOffset = touchOffset;

	switch (_touchMoveDirection)
	{
	case TouchDirection::LEFT: // up

		if (_rightBoundaryChild->getBottomBoundary() + touchOffset >= _rightBoundary)
		{
			realOffset = _rightBoundary - _rightBoundaryChild->getBottomBoundary();
			movePages(realOffset);
			return false;
		}
		break;

	case TouchDirection::RIGHT: // down

		if (_leftBoundaryChild->getTopBoundary() + touchOffset <= _leftBoundary)
		{ 
			realOffset = _leftBoundary - _leftBoundaryChild->getTopBoundary();
			movePages(realOffset);
			return false;
		}
		break;
	default:
		break;
	}

	movePages(realOffset);
	return true;
}

void UIPageViewEx::movePages(float offset)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::movePages(offset);
		return;
	}
	for (auto& page : this->getPages())
	{
		page->setPosition(Vec2(page->getPosition().x,
			page->getPosition().y + offset));
	}
}

void UIPageViewEx::updateAllPagesPosition()
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::updateAllPagesPosition();
		return;
	}
	ssize_t pageCount = this->getPageCount();

	if (pageCount <= 0)
	{
		_curPageIdx = 0;
		return;
	}

	if (_curPageIdx >= pageCount)
	{
		_curPageIdx = pageCount - 1;
	}

	float pageHeight = getContentSize().height;
	for (int i = 0; i < pageCount; i++)
	{
		Layout* page = _pages.at(i);
		page->setPosition(Vec2(0, (_curPageIdx - i) * pageHeight));
	}
}

void UIPageViewEx::autoScroll(float dt)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::autoScroll(dt);
		return;
	}
	switch (_autoScrollDirection)
	{
	case AutoScrollDirection::LEFT://up,+
	{
		float step = _autoScrollSpeed*dt;
		if (_autoScrollDistance - step <= 0.0f)
		{
			step = _autoScrollDistance;
			_autoScrollDistance = 0.0f;
			_isAutoScrolling = false;
		}
		else
		{
			_autoScrollDistance -= step;
		}
		scrollPages(step);
		if (!_isAutoScrolling)
		{
			pageTurningEvent();
		}
		break;
	}
	break;
	case AutoScrollDirection::RIGHT://down,-
	{
		float step = _autoScrollSpeed*dt;
		if (_autoScrollDistance + step >= 0.0f)
		{
			step = -_autoScrollDistance;
			_autoScrollDistance = 0.0f;
			_isAutoScrolling = false;
		}
		else
		{
			_autoScrollDistance += step;
		}
		scrollPages(-step);

		if (!_isAutoScrolling)
		{
			pageTurningEvent();
		}

		break;
	}
	default:
		break;
	}
}

void UIPageViewEx::handleMoveLogic(Touch *touch)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::handleMoveLogic(touch);
		return;
	}

	Vec2 touchPoint = touch->getLocation();

	float offset = 0.0;
	offset = touchPoint.y - touch->getPreviousLocation().y;

	if (offset < 0)
	{
		_touchMoveDirection = TouchDirection::RIGHT;
	}
	else if (offset > 0)
	{
		_touchMoveDirection = TouchDirection::LEFT;
	}
	scrollPages(offset);
}

void UIPageViewEx::handleReleaseLogic(Touch *touch)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::handleReleaseLogic(touch);
		return;
	}

	if (this->getPageCount() <= 0)
	{
		return;
	}
	Widget* curPage = dynamic_cast<Widget*>(this->getPages().at(_curPageIdx));
	if (curPage)
	{
		Vec2 curPagePos = curPage->getPosition();
		ssize_t pageCount = this->getPageCount();
		float curPageLocation = curPagePos.y;
		float pageHeight = getContentSize().height;
		if (!_usingCustomScrollThreshold) {
			_customScrollThreshold = pageHeight / 2.0;
		}
		float boundary = _customScrollThreshold;
		if (curPageLocation >= boundary)//left(up)
		{
			if (_curPageIdx >= pageCount - 1)
			{
				scrollPages(-curPageLocation);
			}
			else
			{
				scrollToPage(_curPageIdx + 1);
			}
		}
		else if (curPageLocation <= -boundary)//right(down)
		{
			if (_curPageIdx <= 0)
			{
				scrollPages(-curPageLocation);
			}
			else
			{
				scrollToPage(_curPageIdx - 1);
			}
		}
		else
		{
			scrollToPage(_curPageIdx);
		}
	}
}

void UIPageViewEx::interceptTouchEvent(TouchEventType event, Widget* sender, Touch *touch)
{
	if (_params._dir == HORIZONTAL)
	{
		PageView::interceptTouchEvent(event, sender, touch);
		return;
	}

	Vec2 touchPoint = touch->getLocation();

	switch (event)
	{
	case TouchEventType::BEGAN:
	{
		_touchBeganPosition = touch->getLocation();
		_isInterceptTouch = true;
	}
	break;
	case TouchEventType::MOVED:
	{
		float offset = 0;
		offset = fabs(sender->getTouchBeganPosition().y - touchPoint.y);
		_touchMovePosition = touch->getLocation();
		if (offset > _childFocusCancelOffset)
		{
			sender->setHighlighted(false);
			handleMoveLogic(touch);
		}
	}
	break;
	case TouchEventType::CANCELED:
	case TouchEventType::ENDED:
	{
		_touchEndPosition = touch->getLocation();
		handleReleaseLogic(touch);
		if (sender->isSwallowTouches())
		{
			_isInterceptTouch = false;
		}
	}
	break;
	}
}

void UIPageViewEx::onSizeChanged()
{
	switch (_params._dir)
	{
	case HORIZONTAL:
		PageView::onSizeChanged();
		break;
	case VERTICAL:{
		Layout::onSizeChanged();
		_leftBoundary = getContentSize().height;
		_rightBoundary = 0.0f;
		_doLayoutDirty = true;
	}break;
	default:
		break;
	}
}

void UIPageViewEx::doLayout()
{
	if (!_doLayoutDirty)
	{
		return;
	}

	updateAllPagesPosition();
	updateAllPagesSize();
	updateBoundaryPages();


	_doLayoutDirty = false;
}

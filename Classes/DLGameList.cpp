//
//  DLManager.cpp
//  xPressZone
//
//  Created by 苏明南 on 15/6/13.
//
//

#include "DLGameList.h"
#include "DataManager.h"
#include "LsTools.h"
#include "ZoneManager.h"

USING_NS_CC;
USING_NS_GUI;

DLGameList* DLGameList::create(std::vector< GameBasicData > &data, GameListParams par)
{
	DLGameList *pRet = new DLGameList(data, par);
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

DLGameList::DLGameList(std::vector< GameBasicData > &data, GameListParams par)
	: _scrollLine(nullptr)
	, _params(par)
	, _focus(nullptr)
	, _data(data)
{
	
}

DLGameList::~DLGameList()
{
}

bool DLGameList::init()
{
	if (!ui::PageView::init()) return false;

	CCASSERT(!(_params._rows < 0), "DLGameList::rows < 0");
	CCASSERT(!(_params._cols < 0), "DLGameList::cols < 0");

	this->setTag(1002);
	//2. 更新游戏库
	updateUI();
	initScrollControl();
	//添加定时器更新UI
	this->schedule(CC_SCHEDULE_SELECTOR(DLGameList::updateAllUI), 1.0f);

	return true;
}

void DLGameList::scrollToPage(ssize_t idx)
{
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

	if (_scrollLine)
	{
		auto lineSel = static_cast<ui::Scale9Sprite*>(_scrollLine->getChildByTag(TAG_LINE));
		float lineHeight = 0;
		if (getPageCount() == 0)
			lineHeight = getContentSize().height;
		else
			lineHeight = getContentSize().height / getPageCount();
		lineSel->setPositionY(getContentSize().height - lineHeight * _curPageIdx - lineHeight / 2);
	}
}

void DLGameList::update(float dt)
{
	if (_isAutoScrolling)
	{
		this->autoScroll(dt);
	}
}

void DLGameList::setContentSize(const cocos2d::Size& contentSize)
{
	PageView::setContentSize(contentSize);
	//update scroll line
	if (_scrollLine)
	{
		_scrollLine->setPosition(contentSize.width - 30, 0);
		auto lineBack = static_cast<ui::ImageView*>(_scrollLine->getChildByTag(TAG_LINE));
		auto size = Size(1, contentSize.height);
		lineBack->setContentSize(size);
	}
}

void DLGameList::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	if (parentFlags & FLAGS_TRANSFORM_DIRTY)
	{
		_clippingRectDirty = true;
		getClippingRect();
	}
	ui::Layout::visit(renderer, parentTransform, parentFlags);
}

float DLGameList::getPositionYByIndex(ssize_t idx)const
{
	return (getContentSize().height * (_curPageIdx - idx));
}

bool DLGameList::scrollPages(float touchOffset)
{
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
			realOffset = _leftBoundaryChild->getTopBoundary() - _leftBoundary;
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

void DLGameList::movePages(float offset)
{
	for (auto& page : this->getPages())
	{
		page->setPosition(Vec2(page->getPosition().x,
			page->getPosition().y + offset));
	}
}

void DLGameList::updateAllPagesPosition()
{
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

void DLGameList::autoScroll(float dt)
{
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

void DLGameList::handleMoveLogic(Touch *touch)
{
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

void DLGameList::handleReleaseLogic(Touch *touch)
{
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

void DLGameList::interceptTouchEvent(TouchEventType event, Widget* sender, Touch *touch)
{
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

void DLGameList::onSizeChanged()
{
	Layout::onSizeChanged();
	_leftBoundary = getContentSize().height;
	_rightBoundary = 0.0f;
	_doLayoutDirty = true;
}

void DLGameList::doLayout()
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

//0->left,1->right
Node *DLGameList::getNextPageFocus(cocos2d::Node *curFocus, int dir /* = 0 */)
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
//0->normal, 1->left, 2->right
void DLGameList::changeFocus(cocos2d::Node *focus, int state)
{
	if (!focus)
		return;
	//
	auto leftImg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "leftB"));
	auto rightImg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "rightB"));
	if (state == 1 && leftImg && rightImg){//left focus
		leftImg->loadTexture("MyGame_button02.png");
		rightImg->loadTexture("MyGame_button.png");
		return;
	}
	else if (state == 2 && leftImg && rightImg){//right focus
		leftImg->loadTexture("MyGame_button.png");
		rightImg->loadTexture("MyGame_button02.png");
		return;
	}
	if (leftImg && rightImg){
		leftImg->loadTexture("MyGame_button.png");
		rightImg->loadTexture("MyGame_button.png");
	}
	//
	auto nameNode = LsTools::seekNodeByName(_focus, "gameName");
	auto leftNode = LsTools::seekNodeByName(_focus, "leftB");
	auto rightNode = LsTools::seekNodeByName(_focus, "rightB");
	if (nameNode && leftNode && rightNode){
		nameNode->setVisible(true);
		leftNode->setVisible(false);
		rightNode->setVisible(false);
	}
	nameNode = LsTools::seekNodeByName(focus, "gameName");
	leftNode = LsTools::seekNodeByName(focus, "leftB");
	rightNode = LsTools::seekNodeByName(focus, "rightB");
	if (nameNode && leftNode && rightNode){
		nameNode->setVisible(false);
		leftNode->setVisible(true);
		rightNode->setVisible(true);
	}
	//
	_focus = focus;
}

void DLGameList::updateFocusUI(std::string package)
{
	auto data = DataManager::getInstance()->getDetailGameByPackage(package);
	if (!_focus || !data) return;

	auto preText = static_cast<ui::Text*>(LsTools::seekNodeByName(_focus, "percentName"));
	auto stopImg = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "stopBG"));
	if (data->_apkState == DOWNLOAD_ING){//ing
		preText->setVisible(true);
		stopImg->setVisible(false);
	}
	else if (data->_apkState == DOWNLOAD_STOP){//stop
		preText->setVisible(false);
		stopImg->setVisible(true);
	}
}

void DLGameList::updateAllUI(float dt)
{
	for (unsigned int i = 0; i < _data.size(); ++i)
	{
		auto data = DataManager::getInstance()->getDetailGameByPackage(_data[i].packageName);
		if (!data) continue;

		std::string apkPath = LsTools::getDataPath() + "/data/game/apk/" + data->_basic.packageName + "_V_" + data->_version + ".apk";
		std::string dataPath = LsTools::getDataPath() + "/data/game/apk/" + data->_basic.packageName + ".zip";

		std::vector<std::string> svc = LsTools::parStringByChar(data->_size);
		long webTotal = atol(svc[0].c_str());
		long locTobal = LsTools::getLocalFileLength(apkPath);
		if (svc.size() > 0){
			webTotal += atol(svc[1].c_str());
			locTobal += LsTools::getLocalFileLength(dataPath);
		}

		float webSizeMB = (float)webTotal / (1024.0f * 1024.0f);
		float localSizeMB = (float)locTobal / (1024.0f * 1024.0f);
		float percent = LsTools::getDownloadPercent(data->_size, data->_basic.packageName);
		auto backIcon = static_cast<ui::ImageView*>(LsTools::seekNodeByName(this, std::string("gameRoot_") + int2str(i)));
		if (!backIcon) continue;

		auto proTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(backIcon, "percentName"));
		auto proPre = static_cast<ProgressTimer*>(LsTools::seekNodeByName(backIcon, "loadingPrograss"));
		auto proRoot = LsTools::seekNodeByName(backIcon, "dlProgressRoot");
		auto proStop = LsTools::seekNodeByName(backIcon, "stopBG");
		auto leftTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(backIcon, "leftT"));
		auto rightTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(backIcon, "rightT"));
		auto sizeTxt = static_cast<ui::Text*>(LsTools::seekNodeByName(backIcon, "game_size"));

		proRoot->setVisible(true);
		switch (data->_apkState)
		{
		case DOWNLOAD_ING:
			proStop->setVisible(false);
			proTxt->setVisible(true);
			proPre->setPercentage(percent);
			proTxt->setString(int2str((int)percent) + "%");
			leftTxt->setString(DataManager::getInstance()->valueForKey("stop"));
			rightTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
			sizeTxt->setString(float2str2(localSizeMB, 1) + "M / " + float2str2(webSizeMB, 1) + "M");
			break;
		case DOWNLOAD_STOP:
			proStop->setVisible(true);
			proTxt->setVisible(false);
			leftTxt->setString(DataManager::getInstance()->valueForKey("continue"));
			rightTxt->setString(DataManager::getInstance()->valueForKey("cancel"));
			sizeTxt->setString(float2str2(localSizeMB, 1) + "M / " + float2str2(webSizeMB, 1) + "M");
			break;
		case DOWNLOAD_SUCCESS:
			proRoot->setVisible(false);
			leftTxt->setString(DataManager::getInstance()->valueForKey("install"));
			rightTxt->setString(DataManager::getInstance()->valueForKey("delete"));
			sizeTxt->setString(float2str2(webSizeMB, 1) + "M");
			break;
		case DOWNLOAD_UNDEFINE:
			data->_apkState = DOWNLOAD_STOP;
			break;
		}

	}
}

void DLGameList::updateUI()
{
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

				//创建游戏背景节点
				auto backIcon = initIconBackground(i, rowNum, colNum);
				//创建游戏图标节点
				auto iconNode = static_cast<ui::ImageView*>(initIconImg(backIcon, i));
				//创建名字节点，返回值表示是否名字超过底图的宽度
				bool isExit = initGameName(backIcon, iconNode, i);
				//创建游戏下载进度条
				initDLProgress(iconNode, i);
				//创建两个操作按钮
				initOpControl(backIcon, i);
				//创建游戏大小
				initGameSize(iconNode, i);

				layout->addChild(backIcon);
			}
			insertPage(layout, pageIndex);
		}
	}
	updateAllUI(0);
}

cocos2d::Node *DLGameList::getLeftNextPageFocus(cocos2d::Node *curFocus)
{
	int pageIndex = getCurPageIndex();

	//获取上一页的焦点控件在哪一列
	cocos2d::ui::Layout *layout = getPage(pageIndex + 1);
	int num = (int)(curFocus->getTag() - TAG_BASE + 1) - (pageIndex + 1) * (_params._cols * _params._rows);
	int colNo = ((num - 1) % _params._cols) + 1;
	layout->setUserObject(nullptr);
	//设置当前页的焦点
	layout = getCurPage();
	int curNum = (_params._rows * _params._cols) - (_params._cols - colNo + 1) + pageIndex * (_params._rows * _params._cols);
	cocos2d::Node *curNode = layout->getChildByTag(TAG_BASE + curNum);
	layout->setUserObject(curNode);
	return curNode;
}

cocos2d::Node *DLGameList::getRightNextPageFocus(cocos2d::Node *curFocus)
{
	int pageIndex = getCurPageIndex();

	//获取上一页的焦点控件在哪一列
	cocos2d::ui::Layout *layout = getPage(pageIndex - 1);
	int num = (int)(curFocus->getTag() - TAG_BASE) - (pageIndex - 1) * (_params._rows * _params._cols);
	int colNo = (num % _params._cols);
	layout->setUserObject(nullptr);
	//设置当前页的焦点
	layout = getCurPage();
	if (layout->getChildrenCount() < colNo + 1)
		colNo = layout->getChildrenCount() - 1;
	cocos2d::Node *curNode = layout->getChildByTag(TAG_BASE + colNo + pageIndex * (_params._rows * _params._cols));
	layout->setUserObject(curNode);
	return curNode;
}

cocos2d::Vec2 DLGameList::getImgPosition(int col, int row)
{
	int dety = col * _params._colsDiff;
	int detx = row * _params._rowsDiff;
	Vec2 position = (Vec2(row * _params._imgSize.width + detx + _params._imgSize.width / 2.0f + _params._leftMargin,
		getContentSize().height - col * _params._imgSize.height - dety - _params._imgSize.height / 2.0f
		- col * _params._extenSize - _params._extenSize / 2 - _params._topMargin));

	return position;
}

void DLGameList::reSetContentSize()
{
	float width = _params._cols * _params._imgSize.width + (_params._cols - 1) * _params._rowsDiff
		+ _params._leftMargin + _params._rightMargin;
	float height = _params._rows * _params._imgSize.height + (_params._rows - 1) * _params._colsDiff
		+ _params._rows * _params._extenSize + _params._topMargin + _params._bottomMargin;

	setContentSize(Size(width, height));
}
void DLGameList::initScrollControl()
{
	if (!_scrollLine)
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
				lineBack->setTag(TAG_LINE);
			}
			{
				auto lineSel = ui::Scale9Sprite::create("Common_rollline01.png");
				lineSel->setName("lineSelect");
				_scrollLine->addChild(lineSel);
			}
		}
	}
	//
	if (_scrollLine)
	{
		auto lineSel = static_cast<ui::Scale9Sprite*>(LsTools::seekNodeByName(_scrollLine, "lineSelect"));
		float lineHeight = 0;
		if (getPageCount() == 0)
			lineHeight = getContentSize().height;
		else
			lineHeight = getContentSize().height / getPageCount();
		lineSel->setContentSize(Size(10, lineHeight));
		lineSel->setPositionY(getContentSize().height - lineHeight / 2);
	}
}

cocos2d::ui::ImageView *DLGameList::initIconBackground(int i, int rn, int cn)
{
	auto backIcon = ui::ImageView::create(_params._iconBg);
	backIcon->setName(std::string("gameRoot_") + std::string(int2str(i)));
	setScale9Sprite(backIcon, SELECT_CAP_SIZE, SELECT_CAP_SIZE);
	backIcon->setAnchorPoint(Vec2(0.5f, 0.5f));
	backIcon->setContentSize(Size(_params._imgSize.width, _params._imgSize.height + _params._extenSize));
	backIcon->setPosition(getImgPosition(cn, rn));
	backIcon->setUserData(&_data[i].packageName);
	backIcon->setTag(TAG_BASE + i);
	backIcon->setTouchEnabled(true);

	return backIcon;
}

cocos2d::Node *DLGameList::initIconImg(cocos2d::ui::ImageView* backIcon, int i)
{
	std::string iconPath = LsTools::lsStandardPath(_data[i]._icon);
	if (!FileUtils::getInstance()->isFileExist(FileUtils::getInstance()->fullPathForFilename(iconPath)))
		iconPath = s_DefaultIcon;
	auto gameBt = cocos2d::ui::ImageView::create(LsTools::lsStandardPath(iconPath));
	gameBt->setContentSize(_params._imgSize);
	gameBt->ignoreContentAdaptWithSize(false);

	gameBt->setPosition(Vec2(gameBt->getContentSize().width / 2,
		backIcon->getContentSize().height - gameBt->getContentSize().height / 2));
	backIcon->addChild(gameBt, 1);
	return gameBt;
}

bool DLGameList::initGameName(cocos2d::ui::ImageView* backIcon, cocos2d::Node *icon, int i)
{
	auto name = ui::Text::create(_data[i]._name, _params._fontName, _params._fontSize);
	name->setName("gameName");
	Vec2 pos = Vec2(icon->getPositionX(), _params._extenSize / 2);

	if (name->getContentSize().width > _params._imgSize.width)
	{
		float currentSize = _params._fontSize;
		currentSize--;
		while (currentSize > 0){
			name->setFontSize(currentSize);
			if (name->getContentSize().width < _params._imgSize.width)
				break;
			currentSize--;
		}
	}
	name->setPosition(pos);
	backIcon->addChild(name, 1);
	return false;
}

cocos2d::Node *DLGameList::initDLProgress(cocos2d::ui::ImageView* iconImg, int i)
{
	auto dlProgressRoot = Node::create();
	dlProgressRoot->setName("dlProgressRoot");
	dlProgressRoot->setPosition(Vec2(135, 135));
	iconImg->addChild(dlProgressRoot, 1);

	auto back = ui::ImageView::create("download_black.png");
	back->setName("Icon_Black");
	dlProgressRoot->addChild(back);

	auto prograssBG = Sprite::create("progress01.png");
	prograssBG->setAnchorPoint(Vec2(0.5f, 0.5f));
	prograssBG->setName("LoadingPrograssBG");
	dlProgressRoot->addChild(prograssBG);

	GameDetailData *curData = DataManager::getInstance()->getDetailGameByPackage(_data[i].packageName);
	std::string fullName = LsTools::getDataPath() + "/data/game/apk/" + _data[i].packageName + "_V_" + curData->_version + ".apk";

	std::vector<std::string> svc = LsTools::parStringByChar(curData->_size);
	long webSize = atol(svc[0].c_str());
	float ws = (float)webSize / (1024.f * 1024.f);
	long localSize = LsTools::getLocalFileLength(fullName);
	float ls = (float)localSize / (1024.0f * 1024.0f);
	float percent = LsTools::getDownloadPercent(curData->_size, curData->_basic.packageName);

	auto prograss = ProgressTimer::create(Sprite::create("progress02.png"));
	prograss->setAnchorPoint(Vec2(0.5f, 0.5f));
	prograss->setName("loadingPrograss");
	prograss->setPercentage(percent);
	dlProgressRoot->addChild(prograss);

	auto percentTxt = cocos2d::ui::Text::create(int2str((int)percent) + "%", "msyh.ttf", 36);
	percentTxt->setAnchorPoint(Vec2(0.5f, 0.5f));
	percentTxt->setName("percentName");
	dlProgressRoot->addChild(percentTxt);

	auto stopBG = cocos2d::ui::ImageView::create("progress03.png");
	stopBG->setName("stopBG");
	dlProgressRoot->addChild(stopBG);

	return dlProgressRoot;
}

cocos2d::Node *DLGameList::initGameSize(cocos2d::ui::ImageView* iconImg, int i)
{
	auto back = LayerColor::create(Color4B(0, 0, 0, 100), 270, 30);
	back->setName("GameSize_Black");

	auto sizeStr = DataManager::getInstance()->getDetailGameByPackage(_data[i].packageName)->_size;
	std::vector<std::string> svc = LsTools::parStringByChar(sizeStr);
	long sizeTmp = atol(svc[0].c_str());
	if (svc.size() > 1)
		sizeTmp += atol(svc[1].c_str());

	float ws = (float)(sizeTmp) / (1024.f * 1024.f);
	auto gSize = cocos2d::ui::Text::create(std::string(float2str2(ws, 1)) + "M", "msyh.ttf", 27);
	gSize->setName("game_size");
	gSize->setTextColor(Color4B(0, 251, 151, 255));
	gSize->setPosition(Vec2(135, 15));
	back->addChild(gSize);

	iconImg->addChild(back, 23);
	return back;
}

cocos2d::Node *DLGameList::initOpControl(cocos2d::ui::ImageView* backIcon, int i)
{
	auto leftB = ui::ImageView::create("MyGame_button.png");
	setScale9Sprite(leftB, 20, 20);
	leftB->setContentSize(Size(92, 50));
	leftB->setName("leftB");
	leftB->setTouchEnabled(true);
	auto leftT = ui::Text::create(DataManager::getInstance()->valueForKey("continue"), "msyh.ttf", 27);
	leftT->setName("leftT");
	leftT->setPosition(Vec2(46, 25));
	leftB->addChild(leftT, 1);
	leftB->setPosition(Vec2(77, 25));
	leftB->setVisible(false);
	backIcon->addChild(leftB, 1);

	auto rightB = ui::ImageView::create("MyGame_button.png");
	rightB->setTouchEnabled(true);
	setScale9Sprite(rightB, 20, 20);
	rightB->setContentSize(Size(92, 50));
	rightB->setName("rightB");
	auto rightT = ui::Text::create(DataManager::getInstance()->valueForKey("cancel"), "msyh.ttf", 27);
	rightT->setName("rightT");
	rightT->setPosition(Vec2(46, 25));
	rightB->addChild(rightT, 1);
	rightB->setPosition(Vec2(193, 25));
	rightB->setVisible(false);
	backIcon->addChild(rightB, 1);

	return nullptr;
}

void DLGameList::setScale9Sprite(cocos2d::ui::ImageView *img, float w, float h)
{
	img->setScale9Enabled(true);
	img->setCapInsets(Rect(
		w,
		h,
		img->getContentSize().width - 2 * w,
		img->getContentSize().height - 2 * h));
}


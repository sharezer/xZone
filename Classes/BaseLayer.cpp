//
//  BaseLayer.cpp
//  xZone
//
//  Created by Sharezer on 15/1/16.
//
//

#include "BaseLayer.h"
#include "ZoneManager.h"
#include "LsTools.h"
#include "ContentTo.h"

USING_NS_CC;
USING_NS_GUI;

#define FOCUS_ACTION_TAG 123

#pragma region BaseLayer

BaseLayer::BaseLayer()
	: _rootNode(nullptr)
	, _type(LayerType::NONE)
	, _focus(nullptr)
{
	_mapFocusZOrder.clear();
}

BaseLayer::~BaseLayer()
{
	_mapFocusZOrder.clear();
}

bool BaseLayer::init()
{
	if (this->initUI() && this->initData())
		return true;
	return false;
}

void BaseLayer::onEnter()
{
	Layer::onEnter();
	if (_type != LayerType::SECOND && _type != LayerType::THIRD)
		changeFocus(EventKeyboard::KeyCode::KEY_DPAD_DOWN);
}

//void BaseLayer::initMapFocusZOrder(cocos2d::Node* parent)
//{
//	_mapFocusZOrder.clear();
//	for (auto& child : parent->getChildren())
//		_mapFocusZOrder.insert(std::pair<int, int>(child->getTag(), child->getLocalZOrder()));
//}

void BaseLayer::onClickBack()
{
	if (_type == LayerType::THIRD) {
		ZM->changeThirdState(ThirdState::NONE);
		if (ZM_SECOND){
			ZM->showSecond(true);
		}
		else
		{
			ZM->showMain(true);
			ZM_MAIN->showCurFocus();
		}
	}
	else if (_type == LayerType::SECOND) {
		ZM->changeSecondState(SecondState::NONE);
		ZM->showMain(true);
		ZM_MAIN->showCurFocus();
	}
}

bool BaseLayer::isExistOrder(const int& keyName)
{
	BaseLayer::OrderMap::iterator it;
	it = _mapFocusZOrder.find(keyName);
	if (it == _mapFocusZOrder.end())
		return false;
	return true;
}

void BaseLayer::commonFocusAction(cocos2d::Node* newFoucs)
{
	if (_focus) {
		_focus->stopActionByTag(FOCUS_ACTION_TAG);
		_focus->setScale(FOCUS_NORMAL_SCALE);
		if (isExistOrder(_focus->getTag()))
			_focus->setLocalZOrder(_mapFocusZOrder.at(_focus->getTag()));
		_focus->getParent()->setUserObject(nullptr);
	}

	_focus = newFoucs ? newFoucs : _focus;

	if (_focus){
		_focus->stopActionByTag(FOCUS_ACTION_TAG);
		_focus->setScale(FOCUS_NORMAL_SCALE);
		if (isExistOrder(_focus->getTag()))
			_focus->setLocalZOrder(_mapFocusZOrder.at(_focus->getTag()) + FOCUS_Z_ORDER_ADD);
		/*auto action = OrbitCamera::create(2.0f, 1, 0, 0, 360, 0, 0);
		action->setTag(FOCUS_ACTION_TAG);*/

		auto action = ScaleTo::create(0.25f, FOCUS_SCALE_NUM);
		action->setTag(FOCUS_ACTION_TAG);
		_focus->runAction(action);

		ZM->showSelect(true,
			_focus->getContentSize() * FOCUS_SCALE_NUM,
			_focus->getParent()->convertToWorldSpace(_focus->getPosition()));
		//ZM->showSelectWithTarget(true, _focus, _focus->getContentSize() * FOCUS_SCALE_NUM, _focus->getParent()->convertToWorldSpace(_focus->getPosition()));
		_focus->getParent()->setUserObject(_focus);
	}
	else
		ZM->showSelect(false);
}

void BaseLayer::removeFocusAction(bool isHideSelect)
{
	if (_focus) {
		_focus->stopActionByTag(FOCUS_ACTION_TAG);
		_focus->setScale(FOCUS_NORMAL_SCALE);
		if (isExistOrder(_focus->getTag()))
			_focus->setLocalZOrder(_mapFocusZOrder.at(_focus->getTag()));
		_focus->getParent()->setUserData(nullptr);
	}
	_focus = nullptr;
	if (isHideSelect)
		ZM->showSelect(false);
}

void BaseLayer::showCurFocus()
{
	if (_focus)
	{
		ZM->showSelect(true,
			this->_focus->getContentSize() * FOCUS_SCALE_NUM,
			_focus->getParent()->convertToWorldSpace(_focus->getPosition()));
	}
	else
		ZM->showSelect(false);
}

#pragma endregion BaseLayer

/********************************* Dialog ***********************************/
#pragma region Dialog

Dialog::Dialog()
	: _btnOK(nullptr)
	, _btnBack(nullptr)
	, _ipTextField(nullptr)
	, _dialogType(DialogType::NONE)
{
}

Dialog::~Dialog() {}

bool Dialog::init()
{
	if (!BaseLayer::init()) {
		return false;
	}
	//    LayerType tyep = getType();
	_type = LayerType::DIALOG;
	return true;
}

bool Dialog::initUI()
{
	_rootNode = CSLoader::createNode("Dialog.csb");
	this->addChild(_rootNode);
	_ipTextField = _rootNode->getChildByName<ui::TextField*>("EditBox");
	if (_ipTextField) _ipTextField->setVisible(false);

	auto btnNode = _rootNode->getChildByName("Node_btn");
	_btnOK = btnNode->getChildByName<ui::Button*>("btn_ok");
	_btnBack = btnNode->getChildByName<ui::Button*>("btn_back");
	_btnOK->addTouchEventListener(CC_CALLBACK_2(Dialog::clickBtn, this));
	_btnBack->addTouchEventListener(CC_CALLBACK_2(Dialog::clickBtn, this));

	return true;
}

bool Dialog::initData()
{
	auto text = _rootNode->getChildByName<ui::Text*>("content");
	text->setString("");
	return true;
}

void Dialog::setContent(const std::string& str)
{
	auto text = _rootNode->getChildByName<ui::Text*>("content");
	text->setString(str);
}

void Dialog::setDialogType(DialogType type)
{
	ZM_DIALOG->setVisible(true);
	ZM->showSelect(false);
	_dialogType = type;
	_btnOK->setBright(false);
	_btnOK->setVisible(false);
	_btnBack->setBright(false);
	_btnBack->setVisible(false);

	_focus = nullptr;
	this->stopAllActions();
	switch (type)
	{
	case DialogType::NONE:
		onClickBack();
		break;
	case DialogType::AUTO_CLOSE:{
		ZM_DIALOG->setVisible(true);
		auto delay = DelayTime::create(1.5f);
		auto callback = CallFunc::create([&](){this->onClickBack(); });
		this->runAction(Sequence::createWithTwoActions(delay, callback));
	}break;
	case DialogType::ONE_BTN:{
		ZM_DIALOG->setVisible(true);
		_btnOK->setVisible(true);
		_focus = _btnOK;
	}break;
	case DialogType::TWO_BTN:{
		ZM_DIALOG->setVisible(true);
		_btnBack->setVisible(true);
		_btnOK->setVisible(true);
		_focus = _btnOK;
	}break;
	default:
		break;
	}
	if (_focus)
	{
		auto btn = static_cast<ui::Button*>(_focus);
		btn->setBright(true);
		//ZM->showSelect(true, Size(_focus->getContentSize().width * 0.9f, _focus->getContentSize().height * 0.65f), _focus->getPosition(), true);
	}
	
}

void Dialog::onClickMenu()
{
	if (_ipTextField) _ipTextField->setVisible(false);
}

void Dialog::onClickOK()
{
	if (_focus)
		clickBtn(_focus, Widget::TouchEventType::ENDED);
}

void Dialog::onClickBack()
{
	if (ZM->_upgrade && _ipTextField){
		if (_ipTextField->isVisible())
			return;

		std::size_t pos = ZM->getInputSecretCode().find("MMMMMMMMMB");
		if (pos != std::string::npos){
			_ipTextField->setVisible(true);
			return;
		}
	}

	ZM_DIALOG->setVisible(false);
	if (cancelEvent)
		cancelEvent();

	cancelEvent = nullptr;
	okEvent = nullptr;
	BaseLayer* layer = nullptr;
	if (ZM->_upgrade)
		return;

	if (ZM_THIRD)
		layer = ZM_THIRD;
	else if (ZM_SECOND)
		layer = ZM_SECOND;
	else
		layer = ZM_MAIN;
	layer->showCurFocus();
}

void Dialog::changeFocus(EventKeyboard::KeyCode keyCode)
{
	if (_dialogType == DialogType::NONE || _dialogType == DialogType::AUTO_CLOSE)
		return;

	_btnOK->setBright(false);
	_btnBack->setBright(false);
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_DPAD_RIGHT: {
		if (_dialogType == DialogType::TWO_BTN)
			_focus = _btnBack;
		else
			_focus = _btnOK;
	} break;
	case EventKeyboard::KeyCode::KEY_DPAD_LEFT: {
		_focus = _btnOK;
	} break;
	default:
		break;
	}

	if (_focus)
	{
		auto btn = static_cast<ui::Button*>(_focus);
		btn->setBright(true);
		//ZM->showSelect(true, Size(_focus->getContentSize().width * 0.9f, _focus->getContentSize().height * 0.65f), _focus->getPosition(), true);
	}
}

void Dialog::clickBtn(Ref* sender, Widget::TouchEventType type)
{
	if (_dialogType == DialogType::NONE || _dialogType == DialogType::AUTO_CLOSE)
		return;
	if (type != Widget::TouchEventType::ENDED)
		return;
	if (ZM->_upgrade && _ipTextField && _ipTextField->isVisible()){
		_ipTextField->setVisible(false);
		LS_LOG("new server address: %s", _ipTextField->getString().c_str());
		ZM->setServerAddress(formatStr("http://%s/xZone/", _ipTextField->getString().c_str()));
	}

	auto btn = dynamic_cast<Button*>(sender);
	std::string name = btn->getName();
	if (name == "btn_ok" && okEvent) {
		okEvent();
		okEvent = nullptr;
		cancelEvent = nullptr;
	}

	this->onClickBack();
}
#pragma endregion Dialog

/********************************* Loading ***********************************/
#pragma region Loading
Loading::Loading()
	: _percent(nullptr)
	, _bg(nullptr)
	, _loadSprite(nullptr)
{
}

Loading::~Loading() {}

bool Loading::init()
{
	if (!BaseLayer::init()) {
		return false;
	}
	//    LayerType tyep = getType();
	_type = LayerType::LOADING;
	return true;
}

bool Loading::initUI()
{
	_rootNode = CSLoader::createNode("Loading.csb");
	this->addChild(_rootNode);
	CCASSERT(_rootNode, "root is null");

	_percent = _rootNode->getChildByName<ui::LoadingBar*>("percent");
	CCASSERT(_percent, "percent is null");
	
	_bg = _rootNode->getChildByName<ui::Layout*>("Panel_1");
	CCASSERT(_bg, "bg is null");
	auto tl = CSLoader::createTimeline("Loading.csb");
	tl->gotoFrameAndPlay(0, 74, true);
	tl->setTimeSpeed(1.0f);
	_rootNode->runAction(tl);

	_loadSprite = _rootNode->getChildByName<Sprite*>("loadSprite");
	
	return true;
}

void Loading::resetPercent()
{
	if (!_percent)
		return;
	setLoadingBarVisiable(true);
	setPercent(0.0f);
}

bool Loading::initData()
{
	return true;
}

void Loading::onClickMenu()
{
}

void Loading::clickItem(Ref *sender, ::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED)
		return;
	this->onClickOK();
}

void Loading::onClickOK()
{

}

void Loading::onClickBack()
{
    ZM->_isLoading = false;
	if (ZM->getSecondState() == SecondState::MY_GAME) {
		ZM->_second->onClickBack();
	}
}

void Loading::changeFocus(EventKeyboard::KeyCode keyCode)
{

}

void Loading::setPercent(float percent)
{
	if (percent > 100.0f || percent < 0.0f)
		return;
	
	_percent->setPercent(percent);
}

#pragma endregion Loading

#pragma region SelectBox

SelectBox::SelectBox(){}
SelectBox::~SelectBox(){}

bool SelectBox::init()
{
	auto box = ImageView::create("Common_Scelect.png");
	box->setCapInsets(Rect(
		SELECT_CAP_SIZE,
		SELECT_CAP_SIZE,
		box->getContentSize().width - 2 * SELECT_CAP_SIZE,
		box->getContentSize().height - 2 * SELECT_CAP_SIZE));
	box->setScale9Enabled(true);
	auto action = RepeatForever::create(Sequence::createWithTwoActions(FadeTo::create(0.8f, 120),
		FadeTo::create(0.8f, 255)));
	CC_SAFE_RETAIN(action);
	box->runAction(action);
	this->addChild(box);
	_boxB = box;

	_box = box;

	auto box2 = ImageView::create("Common_Scelect02.png");
	box2->setCapInsets(Rect(
		SELECT_CAP_SIZE,
		SELECT_CAP_SIZE,
		box2->getContentSize().width - 2 * SELECT_CAP_SIZE,
		box2->getContentSize().height - 2 * SELECT_CAP_SIZE));
	box2->setScale9Enabled(true);
	/*auto action = RepeatForever::create(Sequence::createWithTwoActions(FadeTo::create(1.0f, 100),
		FadeTo::create(1.0, 255)));
		CC_SAFE_RETAIN(action);*/
	box2->runAction(action->clone());
	this->addChild(box2);
	_boxL = box2;

	_boxB->setVisible(false);
	_boxL->setVisible(false);

	return true;
}

void SelectBox::showSelect(bool show, Size size, Vec2 pos, bool isLittle)
{

	if (!_box)
		return;
	//v1.0
	/*
	if (_select->getActionByTag(SELECT_FOCUS_ACTION_TAG)) {
	if (!show) {
	_select->stopActionByTag(SELECT_FOCUS_ACTION_TAG);
	ZM->_select->setVisible(show);
	}
	}
	else if (show) {
	//Ñ¡ÖÐÉÁË¸
	auto action = RepeatForever::create(Blink::create(1.0f, 1));
	action->setTag(SELECT_FOCUS_ACTION_TAG);
	_select->stopAllActions();
	_select->runAction(action);

	ZM->_select->setVisible(show);
	}
	*/

	//v2.0
	float speed = 3500.0f;
	float scaleSize = 1.5f;

	_box = isLittle ? _boxL : _boxB;
	_boxB->setVisible(!isLittle);
	_boxB->setPosition(_box->getPosition());
	_boxL->setVisible(isLittle);
	_boxL->setPosition(_box->getPosition());

	float capDs = 17.0f;
	auto newSize = Size(size.width + scaleSize * SELECT_CAP_SIZE - capDs, size.height + scaleSize * SELECT_CAP_SIZE - capDs);
	_box->stopActionByTag(SELECT_FOCUS_MOVE_TAG);
	if (show)
	{
		if (this->isVisible())
		{
			float time = pos.distance(_box->getPosition()) / speed;
			//float time = 0.2f;
			auto move = MoveTo::create(time, pos);
			auto contentTo = ContentTo::create(time, newSize);
			auto action = Spawn::create(move, contentTo, nullptr);
			action->setTag(SELECT_FOCUS_MOVE_TAG);
			_box->runAction(action);
		}
		else
		{
			_box->setPosition(pos);
			_box->setContentSize(newSize);
		}
	}
	this->setVisible(show);
}

#pragma endregion SelectBox

IMPLEMENT_CLASS_GUI_INFO(UIImageViewWithTexture)

UIImageViewWithTexture::UIImageViewWithTexture() : ui::ImageView()
{

}

UIImageViewWithTexture::~UIImageViewWithTexture()
{

}

UIImageViewWithTexture* UIImageViewWithTexture::create(const std::string &imageFileName, TextureResType texType)
{
	UIImageViewWithTexture *widget = new (std::nothrow) UIImageViewWithTexture;
	if (widget && widget->init(imageFileName, texType))
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}

UIImageViewWithTexture* UIImageViewWithTexture::create()
{
	UIImageViewWithTexture* widget = new (std::nothrow) UIImageViewWithTexture();
	if (widget && widget->init())
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}


UIImageViewWithTexture* UIImageViewWithTexture::create(cocos2d::Texture2D *texture)
{
	UIImageViewWithTexture* widget = new (std::nothrow) UIImageViewWithTexture();
	if (widget && widget->init())
	{
		widget->loadTexture(texture);
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}

void UIImageViewWithTexture::loadTexture(cocos2d::Texture2D *texture)
{
	if (!texture)
	{
		return;
	}
	auto sprite = Sprite::createWithTexture(texture);
	_imageRenderer->init(sprite, Rect::ZERO, Rect::ZERO);
	_imageRenderer->setFlippedY(true);

	_imageTextureSize = _imageRenderer->getContentSize();

	this->updateChildrenDisplayedRGBA();

	updateContentSizeWithTextureSize(_imageTextureSize);
	_imageRendererAdaptDirty = true;
}

//
//  DLControl.cpp
//  xZone
//
//  Created by Sharezer on 15/6/9.
//
//

#include "Setting.h"
#include "LsTools.h"
#include "ZoneManager.h"

USING_NS_CC;
USING_NS_GUI;

Setting::Setting()
{
    
}

Setting::~Setting()
{
}

bool Setting::init()
{
    if (!BaseLayer::init()) {
        return false;
    }
    
    _type = LayerType::SECOND;
    return true;
}

#define STATE_CHANGE_ENABLE(img, name, child, px) img->loadTexture(name); \
	child->setPositionX(px);

#define STATE_CHANGE(b, img, child) if (b) \
	{ STATE_CHANGE_ENABLE(img, "set_switch01.png", child, 60.5f); } \
				else { STATE_CHANGE_ENABLE(img, "set_switch02.png", child, 30); }

void Setting::clickLeft()
{
	int maxD = DataManager::getInstance()->_sUserData.dlMaxCount;
	if (maxD > 1)
	{
		maxD--;
		setDLMaxCount(maxD);
		auto num = static_cast<ui::Text*>(LsTools::seekNodeByName(_focus, "num"));
		num->setString(int2str(maxD));
	}
}

void Setting::clickRight()
{
	int maxD = DataManager::getInstance()->_sUserData.dlMaxCount;
	maxD++;
	setDLMaxCount(maxD);
	auto num = static_cast<ui::Text*>(LsTools::seekNodeByName(_focus, "num"));
	num->setString(int2str(maxD));
}

bool Setting::initUI()
{
    bool bRet = false;
    do {
		_rootNode = CSLoader::createNode(s_SettingUI);
        this->addChild(_rootNode);
        
		//add event
		auto img1 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "image1"));
		img1->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		auto child1 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(img1, "Image_button_1"));
		child1->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		STATE_CHANGE(DataManager::getInstance()->_sUserData.isAutoInstall, img1, child1);

		auto img2 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "image2"));
		img2->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		auto child2 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(img2, "Image_button_2"));
		child2->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		STATE_CHANGE(DataManager::getInstance()->_sUserData.isInstallFinishDel, img2, child2);

		auto img3 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "image3"));
		img3->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));

		auto img4 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "image4"));
		img4->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		auto child4 = static_cast<ui::ImageView*>(LsTools::seekNodeByName(img4, "Image_button_4"));
		child4->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		STATE_CHANGE(DataManager::getInstance()->_sUserData.soundState, img4, child4);

		auto left = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "left"));
		left->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));

		auto right = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_rootNode, "right"));
		right->addTouchEventListener(CC_CALLBACK_2(Setting::clickOkItem, this));
		//update ui
		auto num = static_cast<ui::Text*>(LsTools::seekNodeByName(_rootNode, "num"));
		num->setString(int2str(DataManager::getInstance()->_sUserData.dlMaxCount));

        bRet = true;
    } while (0);
    return bRet;
}

bool Setting::initData()
{
    bool bRet = false;
    do {
        
        bRet = true;
    } while (0);
    return bRet;
}

#define NORMAL_NUM_FUCUS if (_focus) {\
	_focus->setScale(FOCUS_NORMAL_SCALE); \
	_focus->setLocalZOrder(_focus->getLocalZOrder() - FOCUS_Z_ORDER_ADD); \
	}

#define SCALE_NUM_FOCUS(node) if (node) { node->setScale(FOCUS_NORMAL_SCALE); \
	node->setLocalZOrder(node->getLocalZOrder() + FOCUS_Z_ORDER_ADD); \
	cocos2d::Rect rect = cocos2d::utils::getCascadeBoundingBox(_focus); \
	ZM->showSelect(true, \
	rect.size * FOCUS_SCALE_NUM, \
	_rootNode->convertToWorldSpace(rect.origin + cocos2d::Vec2(rect.size.width / 2.0f, rect.size.height / 2.0f)), \
	true); \
	}

void Setting::clickOkItem(cocos2d::Ref* sender, cocos2d::ui::Widget::TouchEventType type)
{
	if (type != ui::Widget::TouchEventType::ENDED)
		return;

	auto node = static_cast<Node*>(sender);
	if (!node) return;

	if (node->getName().substr(0, 5) == "image"){
		NORMAL_NUM_FUCUS;
		_focus = node->getParent();
		SCALE_NUM_FOCUS(_focus);
	}
	else if (node->getName() == "left"){
		NORMAL_NUM_FUCUS;
		_focus = LsTools::seekNodeByName(_rootNode, "5");
		clickLeft();
		SCALE_NUM_FOCUS(_focus);
		return;
	}
	else if (node->getName() == "right"){
		NORMAL_NUM_FUCUS;
		_focus = LsTools::seekNodeByName(_rootNode, "5");
		clickRight();
		SCALE_NUM_FOCUS(_focus);
		return;
	}
	else if (node->getName().substr(0, 13) == "Image_button_"){
		NORMAL_NUM_FUCUS;
		_focus = node->getParent()->getParent();
		SCALE_NUM_FOCUS(_focus);
	}
	else{
		_focus = node;
	}

	if (_focus->getName() == "1"){//自动安装
		auto child = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "Image_button_1"));
		auto img = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "image1"));
		setAutoInstall(!DataManager::getInstance()->_sUserData.isAutoInstall);
		STATE_CHANGE(DataManager::getInstance()->_sUserData.isAutoInstall, img, child);
	}
	else if (_focus->getName() == "2"){//安装完成删除软件包
		auto child = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "Image_button_2"));
		auto img = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "image2"));
		setInstallFinishDel(!DataManager::getInstance()->_sUserData.isInstallFinishDel);
		STATE_CHANGE(DataManager::getInstance()->_sUserData.isInstallFinishDel, img, child);
	}
	else if (_focus->getName() == "3"){//清理本地缓存
		auto child = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "Image_button_3"));
		auto img = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "image3"));

	}
	else if (_focus->getName() == "4"){//音效开关
		auto child = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "Image_button_4"));
		auto img = static_cast<ui::ImageView*>(LsTools::seekNodeByName(_focus, "image4"));
		setSoundEnable(!DataManager::getInstance()->_sUserData.soundState);
		STATE_CHANGE(DataManager::getInstance()->_sUserData.soundState, img, child);
	}
}

#undef STATE_CHANGE_ENABLE
#undef STATE_CHANGE

cocos2d::Node *Setting::getNextFocus(cocos2d::Vec2 dir)
{
	auto name = _focus->getName();
	int num = atoi(name.c_str());
	int upNum = (num > 0 ? (num - 1) : num);
	int doNum = (num < 5 ? (num + 1) : num);

	if (dir.equals(VEC2_UP))//up
		_focus = LsTools::seekNodeByName(_rootNode, int2str(upNum));
	else if (dir.equals(VEC2_DOWN))//down
		_focus = LsTools::seekNodeByName(_rootNode, int2str(doNum));

	return _focus;
}

void Setting::onClickOK()
{
	if (_focus && (_focus->getName() != "5"))
		clickOkItem(_focus, ui::Widget::TouchEventType::ENDED);
}

void Setting::onClickMenu()
{
}

void Setting::changeFocus(::EventKeyboard::KeyCode keyCode)
{
	Vec2Dir dir = LsTools::dirKeyCode2Vec2(keyCode);
	if (_focus && _focus->getName() == "5")
	{
		//left right
		if (dir.equals(VEC2_LEFT)){
			clickLeft();
			DataManager::getInstance()->updateDownloadStateByMaxCount();
			return;
		}
		else if (dir.equals(VEC2_RIGTH)){
			clickRight();
			DataManager::getInstance()->updateDownloadStateByMaxCount();
			return;
		}
	}

	NORMAL_NUM_FUCUS;

	if (!_focus){
		_focus = LsTools::seekNodeByName(_rootNode, "1");
	}
	else{
		getNextFocus(dir);
	}

	SCALE_NUM_FOCUS(_focus);
}

#undef NORMAL_NUM_FUCUS
#undef SCALE_NUM_FOCUS
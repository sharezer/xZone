//
//  ClippingNodeEx.cpp
//  xPressZone
//
//  Created by 苏明南 on 15/4/3.
//
//

#include "ClippingNodeEx.h"

USING_NS_CC;

ClippingNodeEx::ClippingNodeEx(std::string &imgName, std::string &stencilName) 
	: ClippingNode(), _imgName(imgName), _stencilName(stencilName)
{
	//create sprite name
	auto childSprite = Sprite::create(imgName);
	this->addChild(childSprite);
	//create stencil name
	auto sprite = Sprite::create(stencilName);
	this->setStencil(sprite);
	//
}

ClippingNodeEx::~ClippingNodeEx()
{
	//CCClippingNode::~ClippingNode();
}

ClippingNodeEx* ClippingNodeEx::create(std::string &imgName, std::string &stencilName)
{
	ClippingNodeEx *ret = new (std::nothrow) ClippingNodeEx(imgName, stencilName);
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////
ClippingNodeWithTex::ClippingNodeWithTex(std::string &imgName, std::string &stencilName, std::string tex, std::string texBg, float exten)
	: Node(), _imgName(imgName), _stencilName(stencilName), _texCont(tex),
	_texBg(texBg), _extenSize(exten)
{
	//create clipping node
	auto clipNode = ClippingNode::create();
	clipNode->setName("clipNode");
	this->addChild(clipNode);
	//create sprite name
	auto childSprite = Sprite::create(imgName);
	childSprite->setName("childSprite");
	clipNode->addChild(childSprite);
	//create stencil name
	auto sprite = Sprite::create(stencilName);
	clipNode->setStencil(sprite);
	//create texBg
	auto texBack = Sprite::create(_texBg);
	texBack->setName("texBack");
	this->addChild(texBack);
	//create texCon
	auto texCon = Label::createWithSystemFont(_texCont, "msyh.ttf", 24);
	texCon->setName("texCon");
	this->addChild(texCon);
}

ClippingNodeWithTex::~ClippingNodeWithTex()
{
	//Node::~Node();
}

ClippingNodeWithTex* ClippingNodeWithTex::create(std::string &imgName, std::string &stencilName, std::string tex, std::string texBg, float exten)
{
	ClippingNodeWithTex *ret = new (std::nothrow) ClippingNodeWithTex(imgName, stencilName, tex, texBg, exten);
	if (ret && ret->init())
	{
		ret->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ret);
	}

	return ret;
}


//
//  ClippingNodeEx.h
//  xPressZone
//
//  Created by 苏明南 on 15/4/3.
//
//

#ifndef __xPressZone__ClippingNodeEx__
#define __xPressZone__ClippingNodeEx__

#include <stdio.h>
#include "cocos2d.h"
#include "BaseLayer.h"

class ClippingNodeEx : public cocos2d::ClippingNode
{
public:
	/** Creates and initializes a clipping node with a stencilName and child image name.
	*/
	static ClippingNodeEx* create(std::string &imgName, std::string &stencilName);

CC_CONSTRUCTOR_ACCESS:
	ClippingNodeEx(std::string &imgName, std::string &stencilName);

	/**
	* @js NA
	* @lua NA
	*/
	virtual ~ClippingNodeEx();

private:
	std::string _imgName;
	std::string _stencilName;
};

class ClippingNodeWithTex : public cocos2d::Node
{
public:
	/** Creates and initializes a clipping node with a stencilName and child image name.
	*/
	static ClippingNodeWithTex* create(std::string &imgName, std::string &stencilName, std::string tex, std::string texBg = "Icon_GameDetail_txt.png", float extenSize = 34);

CC_CONSTRUCTOR_ACCESS:
	ClippingNodeWithTex(std::string &imgName, std::string &stencilName, std::string tex, std::string texBg, float exten);

	/**
	* @js NA
	* @lua NA
	*/
	virtual ~ClippingNodeWithTex();

private:
	std::string _imgName;
	std::string _stencilName;
	std::string _texCont;
	std::string _texBg;
	float _extenSize;
};

#endif /* defined(__xPressZone__ClippingNodeEx__) */

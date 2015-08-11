//
//  ContentTo.cpp
//  xZone
//
//  Created by Sharezer on 15/3/13.
//
//

#include "ContentTo.h"

USING_NS_CC;

ContentTo* ContentTo::create(float duration, cocos2d::Size size)
{
	ContentTo *ret = new (std::nothrow) ContentTo();
	if (ret)
	{
		if (ret->initWithDuration(duration, size))
			ret->autorelease();
		else
		{
			delete ret;
			ret = nullptr;
		}
	}
	return ret;
}

bool ContentTo::initWithDuration(float duration, cocos2d::Size size)
{
	if (ActionInterval::initWithDuration(duration))
	{
		_endSize = size;
		return true;
	}
	return false;
}

void ContentTo::startWithTarget(Node *target)
{
	ActionInterval::startWithTarget(target);
	_sizeDelta = _endSize - target->getContentSize();
}

ContentTo* ContentTo::clone() const
{
	// no copy constructor
	auto a = new (std::nothrow) ContentTo();
	a->initWithDuration(_duration, _sizeDelta);
	a->autorelease();
	return a;
}

void ContentTo::update(float t)
{
	if (_target)
	{
		Size curSize = _target->getContentSize();
		Size diff = curSize - _previousSize;
		_startSize = _startSize + diff;
		Size newSize = _startSize + (_sizeDelta * t);
		_target->setContentSize(newSize);
		_previousSize = newSize;
	}
}

ContentTo* ContentTo::reverse() const
{
	CCASSERT(false, "reverse() not supported in ContentTo");
	return nullptr;
}
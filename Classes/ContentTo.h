//
//  ContentTo.h
//  xZone
//
//  Created by Sharezer on 15/3/13.
//
//

#ifndef __xZone__ContentTo__
#define __xZone__ContentTo__

#include "cocos2d.h"

class ContentTo : public cocos2d::ActionInterval
{
public:

	static ContentTo* create(float duration, cocos2d::Size size);

	//
	// Overrides
	//
	virtual ContentTo* clone() const override;
	virtual ContentTo* reverse(void) const override;
	virtual void startWithTarget(cocos2d::Node *target) override;
	/**
	* @param time in seconds
	*/
	virtual void update(float t) override;
	ContentTo() {}
	virtual ~ContentTo() {}
	bool initWithDuration(float duration, cocos2d::Size size);

protected:
	cocos2d::Size _sizeDelta;
	cocos2d::Size _startSize;
	cocos2d::Size _previousSize;
	cocos2d::Size _endSize;
private:
	CC_DISALLOW_COPY_AND_ASSIGN(ContentTo);
};

#endif /* defined(__xZone__ContentTo__) */

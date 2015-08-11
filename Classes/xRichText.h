//
//  xRichText.h
//  xPressZone
//
//  Created by 苏明南 on 15/3/5.
//
//

#ifndef __xPressZone__xRichText__
#define __xPressZone__xRichText__

#include <stdio.h>
#include "cocos2d.h"
#include "ui/CocosGUI.h"

class xRichElement : public cocos2d::Ref
{
public:
	enum class Type
	{
		TEXT,
		IMAGE,
		CUSTOM
	};
	xRichElement(){};
	virtual ~xRichElement(){};
	bool init(int tag, const cocos2d::Color3B& color, GLubyte opacity);
protected:
	Type _type;
	int _tag;
	cocos2d::Color3B _color;
	GLubyte _opacity;
	friend class xRichText;
};

class xRichElementText : public xRichElement
{
public:
	xRichElementText(){ _type = Type::TEXT; };
	virtual ~xRichElementText(){};
	bool init(int tag, const cocos2d::Color3B& color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize);
	static xRichElementText* create(int tag, const cocos2d::Color3B& color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize);
protected:
	std::string _text;
	std::string _fontName;
	float _fontSize;
	friend class xRichText;

};

class xRichElementImage : public xRichElement
{
public:
	xRichElementImage(){ _type = Type::IMAGE; };
	virtual ~xRichElementImage(){};
	bool init(int tag, const cocos2d::Color3B& color, GLubyte opacity, const std::string& filePath);
	static xRichElementImage* create(int tag, const cocos2d::Color3B& color, GLubyte opacity, const std::string& filePath);
protected:
	std::string _filePath;
	cocos2d::Rect _textureRect;
	int _textureType;
	friend class xRichText;
};

class xRichElementCustomNode : public xRichElement
{
public:
	xRichElementCustomNode(){ _type = Type::CUSTOM; };
	virtual ~xRichElementCustomNode(){ CC_SAFE_RELEASE(_customNode); };
	bool init(int tag, const cocos2d::Color3B& color, GLubyte opacity, cocos2d::Node* customNode);
	static xRichElementCustomNode* create(int tag, const cocos2d::Color3B& color, GLubyte opacity, cocos2d::Node* customNode);
protected:
	cocos2d::Node* _customNode;
	friend class xRichText;
};

class xRichText : public cocos2d::ui::Widget
{
public:
	static xRichText* create();

	void setLeftSpace(cocos2d::Vec4 &size);
	cocos2d::Vec4 getLeftSpace() { return _sizeSpace; }

	void setBackColor(cocos2d::Color4B &color);
	cocos2d::Color4B getBackColor() { return _backColor; }

	void setIgnoreNullSize(bool ignore);
	bool getIgnoreNullSize() { return _isIgnoreNullSize; }

	void setMaxLineNum(int n) { _maxLineNum = n; }
	int getMaxLineNum() { return _maxLineNum; }

public:
	void insertElement(xRichElement* element, int index);
	void pushBackElement(xRichElement* element);
	void removeElement(int index);
	void removeElement(xRichElement* element);

	void setVerticalSpace(float space);
	virtual void setAnchorPoint(const cocos2d::Vec2 &pt);
	virtual cocos2d::Size getVirtualRendererSize() const override;
	void formatText();
	virtual void ignoreContentAdaptWithSize(bool ignore);
	virtual std::string getDescription() const override;

	virtual void setContentSize(const cocos2d::Size& contentSize) override;

protected:
	virtual void adaptRenderers();

	virtual void initRenderer();
	void pushToContainer(cocos2d::Node* renderer);
	void handleTextRenderer(const std::string& text, const std::string& fontName, float fontSize, const cocos2d::Color3B& color, GLubyte opacity);
	void handleImageRenderer(const std::string& fileParh, const cocos2d::Color3B& color, GLubyte opacity);
	void handleCustomRenderer(cocos2d::Node* renderer);
	void formarRenderers();
	void addNewLine();

CC_CONSTRUCTOR_ACCESS:
	xRichText();
	virtual ~xRichText();
	virtual bool init() override;

protected:
	cocos2d::Vec4 _sizeSpace;//x->leftSpace, y->rightSpace, z->topSpace, w->bottomSpace;
	cocos2d::Color4B _backColor;
	cocos2d::LayerColor *_backColorLayer;
	bool _isIgnoreNullSize;
	cocos2d::Size _originContentSize;
	int _maxLineNum;	//最大显示行数，如果为-1，表示不受限制，如果为0，说明不显示

	bool _formatTextDirty;
	cocos2d::Vector<xRichElement*> _richElements;
	std::vector<cocos2d::Vector<cocos2d::Node*>*> _elementRenders;
	float _leftSpaceWidth;
	float _verticalSpace;
	cocos2d::Node* _elementRenderersContainer;
};


#endif /* defined(__xPressZone__xRichText__) */

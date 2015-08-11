//
//  xRichText.cpp
//  xPressZone
//
//  Created by 苏明南 on 15/3/5.
//
//

#include "xRichText.h"

USING_NS_CC;

bool xRichElement::init(int tag, const Color3B &color, GLubyte opacity)
{
	_tag = tag;
	_color = color;
	_opacity = opacity;
	return true;
}


xRichElementText* xRichElementText::create(int tag, const Color3B &color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize)
{
	xRichElementText* element = new (std::nothrow) xRichElementText();
	if (element && element->init(tag, color, opacity, text, fontName, fontSize))
	{
		element->autorelease();
		return element;
	}
	CC_SAFE_DELETE(element);
	return nullptr;
}

bool xRichElementText::init(int tag, const Color3B &color, GLubyte opacity, const std::string& text, const std::string& fontName, float fontSize)
{
	if (xRichElement::init(tag, color, opacity))
	{
		_text = text;
		_fontName = fontName;
		_fontSize = fontSize;
		return true;
	}
	return false;
}

xRichElementImage* xRichElementImage::create(int tag, const Color3B &color, GLubyte opacity, const std::string& filePath)
{
	xRichElementImage* element = new (std::nothrow) xRichElementImage();
	if (element && element->init(tag, color, opacity, filePath))
	{
		element->autorelease();
		return element;
	}
	CC_SAFE_DELETE(element);
	return nullptr;
}

bool xRichElementImage::init(int tag, const Color3B &color, GLubyte opacity, const std::string& filePath)
{
	if (xRichElement::init(tag, color, opacity))
	{
		_filePath = filePath;
		return true;
	}
	return false;
}

xRichElementCustomNode* xRichElementCustomNode::create(int tag, const cocos2d::Color3B &color, GLubyte opacity, cocos2d::Node *customNode)
{
	xRichElementCustomNode* element = new (std::nothrow) xRichElementCustomNode();
	if (element && element->init(tag, color, opacity, customNode))
	{
		element->autorelease();
		return element;
	}
	CC_SAFE_DELETE(element);
	return nullptr;
}

bool xRichElementCustomNode::init(int tag, const cocos2d::Color3B &color, GLubyte opacity, cocos2d::Node *customNode)
{
	if (xRichElement::init(tag, color, opacity))
	{
		_customNode = customNode;
		_customNode->retain();
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
xRichText* xRichText::create()
{
	xRichText* widget = new (std::nothrow) xRichText();
	if (widget && widget->init())
	{
		widget->autorelease();
		return widget;
	}
	CC_SAFE_DELETE(widget);
	return nullptr;
}

xRichText::xRichText() :
_sizeSpace(10, 10, 10, 10), _backColorLayer(nullptr),
_isIgnoreNullSize(false),
_formatTextDirty(true),
_leftSpaceWidth(0.0f),
_verticalSpace(0.0f),
_elementRenderersContainer(nullptr),
_maxLineNum(-1)
{
//	setBackColor(Color4B(0, 0, 0, 50));
}

xRichText::~xRichText() 
{
	_richElements.clear();
}

bool xRichText::init()
{
	if (Widget::init())
	{
		return true;
	}
	return false;
}

void xRichText::setLeftSpace(cocos2d::Vec4 &size)
{
	_sizeSpace = size;
	_formatTextDirty = true;
}

void xRichText::setBackColor(cocos2d::Color4B &color)
{
	if (!_backColorLayer)
	{
		_backColorLayer = LayerColor::create();
		addProtectedChild(_backColorLayer, -2, -1);
	}
	_backColor = color;
	_backColorLayer->setColor(Color3B(color.r, color.g, color.b));
	_backColorLayer->setOpacity(color.a);
	_backColorLayer->setContentSize(_contentSize);
}

void xRichText::setIgnoreNullSize(bool ignore)
{
	_isIgnoreNullSize = ignore;
	if (_isIgnoreNullSize)
	{
		//用文本的大小来设置文本框
	}
	else
	{
		setContentSize(_originContentSize);
	}
	_formatTextDirty = true;
}

void xRichText::setContentSize(const cocos2d::Size& contentSize)
{
	_originContentSize = contentSize;
	ui::Widget::setContentSize(contentSize);
	if (_backColorLayer)
		_backColorLayer->setContentSize(contentSize);
} 

void xRichText::initRenderer()
{
	_elementRenderersContainer = Node::create();
	_elementRenderersContainer->setAnchorPoint(Vec2(0.5f, 0.5f));
	addProtectedChild(_elementRenderersContainer, 0, -1);
}

void xRichText::insertElement(xRichElement *element, int index)
{
	_richElements.insert(index, element);
	_formatTextDirty = true;
}

void xRichText::pushBackElement(xRichElement *element)
{
	_richElements.pushBack(element);
	_formatTextDirty = true;
}

void xRichText::removeElement(int index)
{
	_richElements.erase(index);
	_formatTextDirty = true;
}

void xRichText::removeElement(xRichElement *element)
{
	_richElements.eraseObject(element);
	_formatTextDirty = true;
}

void xRichText::formatText()
{
	if (_formatTextDirty)
	{
		_elementRenderersContainer->removeAllChildren();
		_elementRenders.clear();
		if (_ignoreSize)
		{
			addNewLine();
			for (ssize_t i = 0; i < _richElements.size(); i++)
			{
				xRichElement* element = _richElements.at(i);
				Node* elementRenderer = nullptr;
				switch (element->_type)
				{
				case xRichElement::Type::TEXT:
				{
					xRichElementText* elmtText = static_cast<xRichElementText*>(element);
					if (FileUtils::getInstance()->isFileExist(elmtText->_fontName))
					{
						elementRenderer = Label::createWithTTF(elmtText->_text.c_str(), elmtText->_fontName, elmtText->_fontSize);
					}
					else
					{
						elementRenderer = Label::createWithSystemFont(elmtText->_text.c_str(), elmtText->_fontName, elmtText->_fontSize);
					}
					break;
				}
				case xRichElement::Type::IMAGE:
				{
					xRichElementImage* elmtImage = static_cast<xRichElementImage*>(element);
					elementRenderer = Sprite::create(elmtImage->_filePath.c_str());
					break;
				}
				case xRichElement::Type::CUSTOM:
				{
					xRichElementCustomNode* elmtCustom = static_cast<xRichElementCustomNode*>(element);
					elementRenderer = elmtCustom->_customNode;
					break;
				}
				default:
					break;
				}
				elementRenderer->setColor(element->_color);
				elementRenderer->setOpacity(element->_opacity);
				pushToContainer(elementRenderer);
			}
		}
		else
		{
			addNewLine();
			for (ssize_t i = 0; i < _richElements.size(); i++)
			{

				xRichElement* element = static_cast<xRichElement*>(_richElements.at(i));
				switch (element->_type)
				{
				case xRichElement::Type::TEXT:
				{
					xRichElementText* elmtText = static_cast<xRichElementText*>(element);
					handleTextRenderer(elmtText->_text.c_str(), elmtText->_fontName.c_str(), elmtText->_fontSize, elmtText->_color, elmtText->_opacity);
					break;
				}
				case xRichElement::Type::IMAGE:
				{
					xRichElementImage* elmtImage = static_cast<xRichElementImage*>(element);
					handleImageRenderer(elmtImage->_filePath.c_str(), elmtImage->_color, elmtImage->_opacity);
					break;
				}
				case xRichElement::Type::CUSTOM:
				{
					xRichElementCustomNode* elmtCustom = static_cast<xRichElementCustomNode*>(element);
					handleCustomRenderer(elmtCustom->_customNode);
					break;
				}
				default:
					break;
				}
			}
		}
		formarRenderers();
		_formatTextDirty = false;
	}
}

void xRichText::handleTextRenderer(const std::string& text, const std::string& fontName, float fontSize, const Color3B &color, GLubyte opacity)
{
	auto fileExist = FileUtils::getInstance()->isFileExist(fontName);
	Label* textRenderer = nullptr;
	if (fileExist)
	{
		textRenderer = Label::createWithTTF(text, fontName, fontSize);
	}
	else
	{
		textRenderer = Label::createWithSystemFont(text, fontName, fontSize);
	}
	float textRendererWidth = textRenderer->getContentSize().width;
	_leftSpaceWidth -= textRendererWidth;
	if (_leftSpaceWidth < 0.0f)
	{
		float overstepPercent = (-_leftSpaceWidth) / textRendererWidth;
		std::string curText = text;
		size_t stringLength = StringUtils::getCharacterCountInUTF8String(text);
		int leftLength = stringLength * (1.0f - overstepPercent);
		std::string leftWords = ui::Helper::getSubStringOfUTF8String(curText, 0, leftLength);
		std::string cutWords = ui::Helper::getSubStringOfUTF8String(curText, leftLength, stringLength - leftLength);
		if (leftLength > 0)
		{
			Label* leftRenderer = nullptr;
			if (fileExist)
			{
				leftRenderer = Label::createWithTTF(ui::Helper::getSubStringOfUTF8String(leftWords, 0, leftLength), fontName, fontSize);
			}
			else
			{
				leftRenderer = Label::createWithSystemFont(ui::Helper::getSubStringOfUTF8String(leftWords, 0, leftLength), fontName, fontSize);
			}
			if (leftRenderer)
			{
				leftRenderer->setColor(color);
				leftRenderer->setOpacity(opacity);
				pushToContainer(leftRenderer);
			}
		}
		if (_maxLineNum > 0)
		{
			if (_elementRenders.size() >= _maxLineNum)
				return;
		}

		addNewLine();
		handleTextRenderer(cutWords.c_str(), fontName, fontSize, color, opacity);
	}
	else
	{
		textRenderer->setColor(color);
		textRenderer->setOpacity(opacity);
		pushToContainer(textRenderer);
	}
}

void xRichText::handleImageRenderer(const std::string& fileParh, const Color3B &color, GLubyte opacity)
{
	Sprite* imageRenderer = Sprite::create(fileParh);
	handleCustomRenderer(imageRenderer);
}

void xRichText::handleCustomRenderer(cocos2d::Node *renderer)
{
	Size imgSize = renderer->getContentSize();
	_leftSpaceWidth -= imgSize.width;
	if (_leftSpaceWidth < 0.0f)
	{
		addNewLine();
		pushToContainer(renderer);
		_leftSpaceWidth -= imgSize.width;
	}
	else
	{
		pushToContainer(renderer);
	}
}

void xRichText::addNewLine()
{
	_leftSpaceWidth = _customSize.width - _sizeSpace.x - _sizeSpace.y;//smn
	_elementRenders.push_back(new Vector<Node*>());
}

void xRichText::formarRenderers()
{
	if (_ignoreSize)
	{
		float newContentSizeWidth = 0.0f;
		float newContentSizeHeight = 0.0f;

		Vector<Node*>* row = (_elementRenders[0]);
		float nextPosX = 0.0f;
		for (ssize_t j = 0; j < row->size(); j++)
		{
			Node* l = row->at(j);
			l->setAnchorPoint(Vec2::ZERO);
			l->setPosition(nextPosX, 0.0f);
			_elementRenderersContainer->addChild(l, 1);
			Size iSize = l->getContentSize();
			newContentSizeWidth += iSize.width;
			newContentSizeHeight = MAX(newContentSizeHeight, iSize.height);
			nextPosX += iSize.width;
		}
		_elementRenderersContainer->setContentSize(Size(newContentSizeWidth, newContentSizeHeight));
	}
	else
	{
		float newContentSizeHeight = 0.0f;
		float *maxHeights = new float[_elementRenders.size()];

		for (size_t i = 0; i < _elementRenders.size(); i++)
		{
			Vector<Node*>* row = (_elementRenders[i]);
			float maxHeight = 0.0f;
			for (ssize_t j = 0; j < row->size(); j++)
			{
				Node* l = row->at(j);
				maxHeight = MAX(l->getContentSize().height, maxHeight);
			}
			maxHeights[i] = maxHeight;
			newContentSizeHeight += maxHeights[i];
		}


		float nextPosY = _customSize.height - _sizeSpace.z;//smn
		for (size_t i = 0; i < _elementRenders.size(); i++)
		{
			Vector<Node*>* row = (_elementRenders[i]);
			float nextPosX = 0.0f + _sizeSpace.x;//smn
			nextPosY -= (maxHeights[i] + _verticalSpace);
			//smn
			if (nextPosY <= _sizeSpace.w)
				break;
			//

			for (ssize_t j = 0; j < row->size(); j++)
			{
				Node* l = row->at(j);
				l->setAnchorPoint(Vec2::ZERO);
				l->setPosition(nextPosX, nextPosY);
				_elementRenderersContainer->addChild(l, 1);
				nextPosX += l->getContentSize().width;
			}
		}
		_elementRenderersContainer->setContentSize(_contentSize);
		delete[] maxHeights;
	}

	size_t length = _elementRenders.size();
	for (size_t i = 0; i < length; i++)
	{
		Vector<Node*>* l = _elementRenders[i];
		l->clear();
		delete l;
	}
	_elementRenders.clear();

	if (_ignoreSize)
	{
		Size s = getVirtualRendererSize();
		this->setContentSize(s);
	}
	else
	{
		this->setContentSize(_customSize);
	}
	updateContentSizeWithTextureSize(_contentSize);
	_elementRenderersContainer->setPosition(_contentSize.width / 2.0f, _contentSize.height / 2.0f);
}

void xRichText::adaptRenderers()
{
	this->formatText();
}

void xRichText::pushToContainer(cocos2d::Node *renderer)
{
	if (_elementRenders.size() <= 0)
	{
		return;
	}
	_elementRenders[_elementRenders.size() - 1]->pushBack(renderer);
}

void xRichText::setVerticalSpace(float space)
{
	_verticalSpace = space;
}

void xRichText::setAnchorPoint(const Vec2 &pt)
{
	Widget::setAnchorPoint(pt);
	_elementRenderersContainer->setAnchorPoint(pt);
}

Size xRichText::getVirtualRendererSize() const
{
	return _elementRenderersContainer->getContentSize();
}

void xRichText::ignoreContentAdaptWithSize(bool ignore)
{
	if (_ignoreSize != ignore)
	{
		_formatTextDirty = true;
		Widget::ignoreContentAdaptWithSize(ignore);
	}
}

std::string xRichText::getDescription() const
{
	return "xRichText";
}

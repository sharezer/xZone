//
//  Upgrade.h
//  xZone
//
//  Created by Sharezer on 15/3/3.
//
//

#ifndef __xZone__Upgrade__
#define __xZone__Upgrade__

#include "cocos2d.h"
#include "Global.h"
#include "extensions/cocos-ext.h"

class Upgrade : public cocos2d::Layer, public cocos2d::extension::AssetsManagerDelegateProtocol {
public:
	Upgrade();
	virtual ~Upgrade();
	CREATE_FUNC(Upgrade);
	virtual bool init();

	void check(cocos2d::Ref* sender);
	void reset(cocos2d::Ref* sender);

	void reCheck();

	virtual void onError(cocos2d::extension::AssetsManager::ErrorCode errorCode);
	virtual void onProgress(int percent);
	virtual void onSuccess();

	void start();

private:
	cocos2d::extension::AssetsManager* getAssetManager();
	void initDownloadDir();

	void loadGameJson(std::string version);
private:
	std::string _pathToSave;
	cocos2d::Label* _showDownloadInfo;

};

#endif /* defined(__xZone__Upgrade__) */

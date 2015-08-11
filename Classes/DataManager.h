//
//  DataManager.h
//  xZone
//
//  Created by Sharezer on 15/1/17.
//
//

#ifndef __xZone__DataManager__
#define __xZone__DataManager__

#include "cocos2d.h"
#include "Global.h"
#include "json/document.h"

struct GameDetailData {
	std::string _id;	//数据库的唯一标示的id
	std::string _type; //类型(0-5);可以属于多个分类(eg:03;表示属于分类0和分类3)
	GameBasicData _basic; //基础数据，名字和图标
	std::string _fileName;//截图文件夹名称
	std::string _summary; //游戏简介
	std::string _screenHot; //游戏截图：多个截图使用分号分割
	std::string _score; //游戏评分
	std::string _size; //游戏大小
	std::string _language; //游戏支持的语言
	std::string _starNum; //游戏星星数
	std::string _downloadNum; //下载数量
	std::string _fromAddress; //来源（百度91、碗豆浆、应用宝等）
	std::string _downloadURL; //下载地址
	std::string _network; //网络连接模式（强联网、弱联网和单机）
	std::string _model; //模式（单人、双人和多人）
	std::string _operModel; //操作模式（手柄、鼠标、手机、体感、遥控器等等）
	std::string _operExper; //操作体验
	std::string _information; //资讯数
	std::string _version; //版本号
	std::string _platform; //平台（android、iOS、wp、nokia）
	std::string _manufacturer; //制造商
	std::string _publisher; //发行商
	std::string _time; //发布时间,
	int _isCollect;	//是否收藏(0->未收藏, 1->已收藏)
	std::string _isNewAdd;	//是否新增(0->未新增, 1->是新增)
	std::string _isHot;	//是否热门游戏(0->不是热门, 1->热门)
	int _apkState;//0-未下载，1->下载ing，2->取消/暂停下载，3->下载成功，4->安装成功，5->下载失败
};

struct AppStruct
{
	std::string package;//包名
	std::string appIcon;//图标名称
	std::string name;//应用名称
	int state;
	std::string path;//apk路径
	std::string version;
	int customOrder;
};

class DataManager {
public:
    static DataManager* getInstance();
	static void destroyInstance();

    void initData();
    void flushUserData();

	void flushDetailData();
	void initDetailData();
	void initAppList();
	void flushAppList();

	void saveGameVersionForKey(const std::string& key, const std::string& version);
	std::string getGameVersionForKey(const std::string& key);

    const char* valueForKey(const std::string& key)
    {
		if (_languageDic)
			return  _languageDic->valueForKey(key)->getCString();
		return "error";
    };

	void initChinese(){
		CC_SAFE_RELEASE_NULL(_languageDic);
		_languageDic = cocos2d::Dictionary::createWithContentsOfFile("Chinese.xml");
		CC_SAFE_RETAIN(_languageDic);
	}

	std::string getApkDownloadURL(std::string packageName);
	std::string getApkDownloadURLNoVersion(std::string package);
	std::string isApkInstall(std::string package);
    //获取数据包下载地址
    std::string getApkDataPath(std::string package);
    
CC_CONSTRUCTOR_ACCESS:
    DataManager();
    ~DataManager();
    bool init();

public:
    std::vector<GameDetailData>& getDetailGameByType(std::string type);
    std::vector<GameBasicData>& getBasicGameByType(std::string type);
	std::vector<GameBasicData>& getBasicGameOfDownload();
	std::vector<GameDetailData*>& getDownloadingGame();
	void updateDownloadStateByMaxCount(std::string package = "");

	std::vector<std::string>& getGameFileListByType(std::string type);
	GameBasicData* getBasicGameByPackage(std::string package);
	GameDetailData* getDetailGameByPackage(std::string package);
	GameDetailData* getDetailGameByID(std::string id);
	int isDownloadFullGame(std::string package);
    /**
     *  用户数据保存的结构体
     */
    struct SUserData {
        bool musicState;
        bool soundState;
		bool netState;
		bool remoteState;
		bool joystickState;
		bool downloadState;//是否正下载
        bool isFirst;
		bool isAutoInstall;
		bool isInstallFinishDel;

		int dlMaxCount;//最大下载数
        int category; //用作选中的游戏类型(分类->分类详情)
		std::string gamePackageName;//游戏编号(分类详情->游戏详情)
        void reset()
        {
            category = 0;
			dlMaxCount = 3;
			gamePackageName = "";
            musicState = true;
            soundState = true;
			netState = false;
			remoteState = false;
			joystickState = false;
			downloadState = false;
			isAutoInstall = true;

            isFirst = true;
			isInstallFinishDel = true;
        }
    };
    SUserData _sUserData;
	
    std::vector<GameDetailData> _sGameDetailData;
	std::vector<AppStruct> _applist;
private:
    //语言字典
    cocos2d::__Dictionary* _languageDic;
	rapidjson::Document _appDoc;
	rapidjson::Document _localGameDoc;
	
};

#endif /* defined(__xZone__DataManager__) */

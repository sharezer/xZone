//
//  NetManager.h
//  xZone
//
//  Created by Sharezer on 15/3/6.
//
//

#ifndef __xZone__NetManager__
#define __xZone__NetManager__

#include "cocos2d.h"
#include "Global.h"
#include "LsTools.h"

enum ErrorCode
{
	// Error caused by creating a file to store downloaded data
	kCreateFile,
	/** Error caused by network
	-- network unavaivable
	-- timeout
	-- ...
	*/
	kNetwork,
	/** There is not a new version
	*/
	kNoNewVersion,
	/** Error caused in uncompressing stage
	-- can not open zip file
	-- can not read file global information
	-- can not read file information
	-- can not create a directory
	-- ...
	*/
	kUnCompress,
};
typedef std::list<std::string> SENDCOMMEND;
typedef std::list<std::string> SENDLOAD;

class NetManager : public cocos2d::Ref
{
public:
	static NetManager* getInstance();
	static void destroyInstance();

	/**
     *  用于下载资源文件
     *
     *  @param adds        文件下载地址
     *  @param savePath    保存的路径
	 *  @param saveName    文件名
	 *  @param succesCB    成功回调函数
	 *  @param failCB      失败回调函数
	 *  @param isAutoUnZip 保存的路径
     */
	void loadFile(std::string adds, std::string savePath, std::string saveName, std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip = false);
	//发送命令
	void sendCommend(CommendEnum commend, std::string data, std::function<void(const rapidjson::Value& value)> succesCB,
		std::function<void()> failCB = nullptr);
public:
	/**
	*  用于取得下载资源的大小
	*
	*  @param url        文件网络地址
	*/
	long getDownloadFileLength(std::string url);
	/**
	*  下载网络APK包
	*
	*  @param url        网络地址
	*
	*  @param path        文件完整路径地址
	*/
	bool downloadWithCurl(std::string url, std::string path, std::string package);
	/**
	*  更新下载信息的临时文件
	*
	*  @param package        安装包信息
	*
	*  @param totalSize/localSize        下载文件信息
	*/

	int downloadWithFile(std::string url, std::string fileName);// 启动线程的方法
	void* threadFunctionApk(std::string url, std::string fileName);// 被启动的线程函数，注意必须是静态方法
	void* threadFunctionData(std::string url, std::string fileName);// 被启动的线程函数，注意必须是静态方法

	void submitDownloadFail(std::string packageName);//通知服务端，游戏下载失败
CC_CONSTRUCTOR_ACCESS:
	NetManager();
	~NetManager();
    bool init();

private:
	void doloadFileHttp(std::string adds, std::string savePath, std::string saveName, std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip);
	void doLoadFileCURL(std::string adds, std::string savePath, std::string saveName, std::function<void()> succesCB, std::function<void()> failCB, bool isAutoUnZip);
	void doSendCommendHttp(CommendEnum commend, std::string data, std::function<void(const rapidjson::Value& value)> succesCB,
		std::function<void()> failCB = nullptr);
	void doSendCommendCURL(CommendEnum commend, std::string data, std::function<void(const rapidjson::Value& value)> succesCB,
		std::function<void()> failCB = nullptr);
	void safeRemoveLoadFile(std::string path);

	void loadEnd(std::string adds);
public:
	bool _isLoadFile;
	bool _isSendCommend;
	SENDCOMMEND _commendList;
	SENDLOAD _loadList;
};

#endif /* defined(__xZone__NetManager__) */

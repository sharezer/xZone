#include "AppDelegate.h"
#include "ZoneManager.h"
#include "LsTools.h"
#include "PlatformHelper.h"
#include "DataManager.h"
#include "NetManager.h"

USING_NS_CC;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
    NetManager::getInstance()->destroyInstance();
    DataManager::getInstance()->destroyInstance();
    ZM->destroyInstance();
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = { 8, 8, 8, 8, 24, 8 };

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if (!glview) {
        glview = GLViewImpl::create("xZone");
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        glview->setFrameSize(960, 540);
//		glview->setFrameSize(1536, 864);
//glview->setFrameSize(1920, 1080);
#endif
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(true);
    director->getOpenGLView()->setDesignResolutionSize(1920, 1080, ResolutionPolicy::SHOW_ALL);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    for (auto& folder : s_DataFolder) {
        if (!FileUtils::getInstance()->isDirectoryExist(LsTools::lsStandardPath(LsTools::getDataPath() + folder)))
            FileUtils::getInstance()->createDirectory(LsTools::lsStandardPath(LsTools::getDataPath() + folder));
    }
    PH::setDataPath(LsTools::getDataPath());

    //if (DM->_sUserData.isFirst)
    //{
    //	if (!FileUtils::getInstance()->isDirectoryExist(LsTools::getDataPath() + "/base/"))
    //		FileUtils::getInstance()->createDirectory(LsTools::getDataPath() + "/base/");
    //	//res文件
    //	LsTools::copyFileForPath("res.zip",
    //		LsTools::getDataPath() + "/base/res.zip");
    //	LsTools::unZip(LsTools::getDataPath() + "/base/", "res.zip", LsTools::getDataPath() + "/base/");
    //	LsTools::removeFileForPath(LsTools::getDataPath() + "/base/res.zip");
    //	//data文件
    //	LsTools::copyFileForPath("data.zip",
    //		LsTools::getDataPath() + "data.zip");
    //	LsTools::unZip(LsTools::getDataPath(), "data.zip", LsTools::getDataPath());
    //	LsTools::removeFileForPath(LsTools::getDataPath() + "data.zip");
    //}
    //FileUtils::getInstance()->addSearchPath(LsTools::getDataPath() + "/base/");

    FileUtils::getInstance()->addSearchPath(LsTools::getDataPath(), true);
    std::string path = LsTools::getDataPath() + DOWNLOAD_FIEL;
    FileUtils::getInstance()->addSearchPath(LsTools::lsStandardPath(path), true);
    //FileUtils::getInstance()->addSearchPath("res");

    rapidjson::Document server;
    std::string serverFile = "server.json";
    if (FileUtils::getInstance()->isFileExist("serverList.json")) {
        serverFile = "serverList.json";
    }

    LsTools::readJsonWithFile(serverFile.c_str(), server);
    //ZM->setServerAddress(server["serverAddress"].GetString());
//    ZM->setServerAddress("http://192.168.1.249/xZone/test.php");
    ZM->setServerAddress("http://sharezer.sinaapp.com/xZone/index.php");
    LS_LOG("server address:%s, server file:%s", ZM->getServerAddress().c_str(),
        cocos2d::FileUtils::getInstance()->fullPathForFilename(serverFile).c_str());

    ZM->appRun();
    if (DataManager::getInstance()->_sUserData.isFirst) {
        DataManager::getInstance()->_sUserData.isFirst = false;
        DataManager::getInstance()->flushUserData();
    }

    
    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground()
{
    LS_LOG("applicationDidEnterBackground");
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if (ZM->isVideoPlay())
        ZM->stopVideo();
#endif
    LS_LOG("applicationWillEnterForeground");

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

//  Global.h
//
//  Created by Sharezer on 14-2-28.
//
//

#ifndef _GlOBAL_H_
#define _GlOBAL_H_


#include "cocos2d.h"
#define MY_SCREEN_CENTER cocos2d::Vec2(MY_SCREEN.width / 2, MY_SCREEN.height / 2)
#define MY_SCREEN cocos2d::Director::getInstance()->getVisibleSize()

#define LS_SAFE_CALLBACK(p) \
do { \
	if(p) cocos2d::Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, p]{p();}); \
} while(0)

#define USING_NS_GUI            \
using namespace cocostudio; \
using namespace cocos2d::ui

#define USING_NS_NET using namespace cocos2d::network
#define USING_NS_CD using namespace  CocosDenshion

#define SAE                          CocosDenshion::SimpleAudioEngine::getInstance()
#define FU                           cocos2d::FileUtils::getInstance()
#define UD                           cocos2d::UserDefault::getInstance()

#define PI                           3.14159f
#define MIN_FOCUS_DS                 10.0f
#define FOCUS_Z_ORDER_ADD            99
#define FOCUS_SCALE_NUM              1.1f
#define FOCUS_NORMAL_SCALE           1.0f
#define SELECT_FOCUS_ACTION_TAG      101
#define SELECT_FOCUS_MOVE_TAG        102
#define SELECT_CAP_SIZE				 50.0f
#define CHANGE_STATE_DURATION		 0.25f

#define DOWNLOAD_FIEL				 "/res/"

static std::string s_TestDownloadURL = "http://192.168.1.200/xZone/1.apk";

#define USE_ANSY_CURLE
#define PAGEVIEW_USE_STENCIL	//Pageview子节点是否使用裁剪
#define USE_NODE_SAVE //是否使用节点保存重新绘制功能
//#define DEMO_XPRESS

typedef cocos2d::Vec2 Vec2Dir;
#define VEC2_LEFT					 cocos2d::Vec2(-1.0f, 0)
#define VEC2_RIGTH					 cocos2d::Vec2(1.0f, 0)
#define VEC2_UP						 cocos2d::Vec2(0, 1.0f)
#define VEC2_DOWN					 cocos2d::Vec2(0, -1.0f)
#define VEC2_CENTER					 cocos2d::Vec2(0, 0)

enum class Vec2DirEnum
{
	EDIR_CENTER = 0,
	EDIR_LEFT,
	EDIR_RIGHT,
	EDIR_UP,
	EDIR_DOWN
};

static const char s_GameDetailUI[] = "GameDetail.csb";
static const char s_HelpUI[] = "Help.json";
static const char s_GameCategoryDetail[] = "GCategoryDetail.csb";
static const char s_GameDetailIndroduce[] = "GameDetailIntroduce.csb";
static const char s_GameFileName[] = "game.json";//不直接操作，只操作备份文件
static const char s_LocalGameFileName[] = "LocalGame.json";
static const char s_MyGameUI[] = "MyGame.csb";
static const char s_MyGameAddUI[] = "MyGameAdd.csb";
static const char s_DownloadControl[] = "DLControl.csb";
static const char s_SettingUI[] = "setting.csb";
static const char s_AboutUI[] = "About.csb";

static const char s_GameIconPath[] = "data/game/icon/";
static const char s_GameScreenPath[] = "data/game/screenhot/";
static const char s_Star00[] = "Common_Star01.png";
static const char s_Star01[] = "Common_Star02.png";
static const char s_DefaultIcon[] = "com_idle01.png";

static const char s_customOrder[] = "customOrder";//用户自定义优先级
static const char s_collectKey[] = "collect";//收藏字段
static const char s_labelTTF[] = "msyh.ttf";

static bool s_isThreadEnd = false;
static bool s_isUserNet = true;

static float s_eventCD = 0.1f; //点击手柄遥控器，共用一个触发的CD;
enum class SecondState {
	NONE = 0,
	SETTING,
	CATEGORY_DETAIL,
	MY_GAME,
	COLLECT_GAME,
	HOT_GAME,
	NEW_GAME,
	ABOUNT,

	DOWNLOAD_CONTROL
};

enum class ThirdState {
	NONE = 0,
	GAME_DETAIL,
	GAME_ADD
};

enum class MusicType {
	NONE = 0,
	MAIN_BG,
};

enum class EffectType {
	NONE = 0,
	BUTTON
};

enum class LayerType {
	NONE = 0,
	MAIN,
	SECOND,
	THIRD,
	UPGRADE,
	DIALOG,
	SELECT,
	STATE,
	LOADING,
	VIDEO
};


enum class CornerType {
	NOON = 0,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

enum class DialogType {
	NONE = 0,
	AUTO_CLOSE,
	ONE_BTN,
	TWO_BTN
};

enum class VideoPlayerState{
    NONE = 0,
    PLAYING,
    PAUSE,
    STOP
};

enum DownloadState
{
	DOWNLOAD_UNLOAD = 0,//未下载
	DOWNLOAD_ING = 1,	//下载中
	DOWNLOAD_CANCEL = 2, //取消下载
	DOWNLOAD_STOP = 3, //暂停下载
	DOWNLOAD_SUCCESS = 4,//下载成功,
	DOWNLOAD_OPEN = 5,	//启动
	DOWNLOAD_UNDEFINE = 6,//未定义，说明下载出错
	DOWNLOAD_UNKNOW = 7, //未知大小
};

struct MyTime {
	int year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
	int wday;
};

//游戏数据结构
struct GameBasicData {
	std::string _name; //游戏名字
	std::string _icon; //游戏图标
	int _number; //编号，用于快速获取在数据库中的索引
	std::string packageName;
	bool _isSelect;
	bool _isStencil;//是否开启裁剪，默认为否（0）
};

enum class CommendEnum {
	UPGRADE = 1,
	GAME_LIST = 2,
	GAME_DETAIL = 3,
	DOWNLOAD_REMOTE = 4,
	DOWNLOAD_GAME = 5,              //下载游戏和配置文件
	DOWNLOAD_SCREEN_SHOT = 6,       //下载游戏截图
	DOWNLOAD_ICON_ALL = 7,			//下载icon.zip
	DOWNLOAD_GAME_JSON = 8,         //下载game.json弃用
	UPGRADE_RES = 9,                //热更新资源
	UPGRADE_GAME_JSON = 10,         //热更新game.json

	DOWNLOAD_FAIL = 15,             //下载失败统计接口
	DOWNLOAD_ICON = 16,             //下载单个图标
	DOWNLOAD_GAME_NO_VERSION = 17,  //模糊下载游戏
	GET_VIDEO = 20,                 //视频地址
    DOWNLOAD_GAME_DATA = 21,        //下载游戏数据包
};

enum class AppStateEnum {
	Noon = 0, //已安装
	MYGAME = 1, //Game.json中包含的
	PLUS = 2, //手动添加的
	DELETE = 3 //手动删除的
};

//外部文件缓存xZone目录
static const std::string s_DataFolder[]{
	"/dl/",
	"/temp/",
	"/temp/downInfor/",
	"/data/",
	"/data/game/",
	"/data/game/screenhot/",
	"/data/game/icon/",
	"/data/game/apk/",
	"/data/game/iconCopy/"
};

#pragma region keyCodeStr
static const char* s_JoystickStr[24]{
	"JOYSTICK_LEFT_X",
	"JOYSTICK_LEFT_Y",
	"JOYSTICK_RIGHT_X",
	"JOYSTICK_RIGHT_Y",

	"BUTTON_A",
	"BUTTON_B",
	"BUTTON_C",
	"BUTTON_X",
	"BUTTON_Y",
	"BUTTON_Z",

	"BUTTON_DPAD_UP",
	"BUTTON_DPAD_DOWN",
	"BUTTON_DPAD_LEFT",
	"BUTTON_DPAD_RIGHT",
	"BUTTON_DPAD_CENTER",

	"BUTTON_LEFT_SHOULDER",
	"BUTTON_RIGHT_SHOULDER",

	"AXIS_LEFT_TRIGGER",
	"AXIS_RIGHT_TRIGGER",

	"BUTTON_LEFT_THUMBSTICK",
	"BUTTON_RIGHT_THUMBSTICK",

	"BUTTON_START",
	"BUTTON_SELECT",

	"BUTTON_PAUSE"
};

static const char* s_KeyCodeStr[166]{
	"KEY_NONE",
	"KEY_PAUSE",
	"KEY_SCROLL_LOCK",
	"KEY_PRINT",
	"KEY_SYSREQ",
	"KEY_BREAK",
	"KEY_ESCAPE",
	"KEY_BACKSPACE",
	"KEY_TAB",
	"KEY_BACK_TAB",
	"KEY_RETURN",
	"KEY_CAPS_LOCK",
	"KEY_SHIFT",
	"KEY_RIGHT_SHIFT",
	"KEY_CTRL",
	"KEY_RIGHT_CTRL",
	"KEY_ALT",
	"KEY_RIGHT_ALT",
	"KEY_MENU",
	"KEY_HYPER",
	"KEY_INSERT",
	"KEY_HOME",
	"KEY_PG_UP",
	"KEY_DELETE",
	"KEY_END",
	"KEY_PG_DOWN",
	"KEY_LEFT_ARROW",
	"KEY_RIGHT_ARROW",
	"KEY_UP_ARROW",
	"KEY_DOWN_ARROW",
	"KEY_NUM_LOCK",
	"KEY_KP_PLUS",
	"KEY_KP_MINUS",
	"KEY_KP_MULTIPLY",
	"KEY_KP_DIVIDE",
	"KEY_KP_ENTER",
	"KEY_KP_HOME",
	"KEY_KP_UP",
	"KEY_KP_PG_UP",
	"KEY_KP_LEFT",
	"KEY_KP_FIVE",
	"KEY_KP_RIGHT",
	"KEY_KP_END",
	"KEY_KP_DOWN",
	"KEY_KP_PG_DOWN",
	"KEY_KP_INSERT",
	"KEY_KP_DELETE",
	"KEY_F1",
	"KEY_F2",
	"KEY_F3",
	"KEY_F4",
	"KEY_F5",
	"KEY_F6",
	"KEY_F7",
	"KEY_F8",
	"KEY_F9",
	"KEY_F10",
	"KEY_F11",
	"KEY_F12",
	"KEY_SPACE",
	"KEY_EXCLAM",
	"KEY_QUOTE",
	"KEY_NUMBER",
	"KEY_DOLLAR",
	"KEY_PERCENT",
	"KEY_CIRCUMFLEX",
	"KEY_AMPERSAND",
	"KEY_APOSTROPHE",
	"KEY_LEFT_PARENTHESIS",
	"KEY_RIGHT_PARENTHESIS",
	"KEY_ASTERISK",
	"KEY_PLUS",
	"KEY_COMMA",
	"KEY_MINUS",
	"KEY_PERIOD",
	"KEY_SLASH",
	"KEY_0",
	"KEY_1",
	"KEY_2",
	"KEY_3",
	"KEY_4",
	"KEY_5",
	"KEY_6",
	"KEY_7",
	"KEY_8",
	"KEY_9",
	"KEY_COLON",
	"KEY_SEMICOLON",
	"KEY_LESS_THAN",
	"KEY_EQUAL",
	"KEY_GREATER_THAN",
	"KEY_QUESTION",
	"KEY_AT",
	"KEY_CAPITAL_A",
	"KEY_CAPITAL_B",
	"KEY_CAPITAL_C",
	"KEY_CAPITAL_D",
	"KEY_CAPITAL_E",
	"KEY_CAPITAL_F",
	"KEY_CAPITAL_G",
	"KEY_CAPITAL_H",
	"KEY_CAPITAL_I",
	"KEY_CAPITAL_J",
	"KEY_CAPITAL_K",
	"KEY_CAPITAL_L",
	"KEY_CAPITAL_M",
	"KEY_CAPITAL_N",
	"KEY_CAPITAL_O",
	"KEY_CAPITAL_P",
	"KEY_CAPITAL_Q",
	"KEY_CAPITAL_R",
	"KEY_CAPITAL_S",
	"KEY_CAPITAL_T",
	"KEY_CAPITAL_U",
	"KEY_CAPITAL_V",
	"KEY_CAPITAL_W",
	"KEY_CAPITAL_X",
	"KEY_CAPITAL_Y",
	"KEY_CAPITAL_Z",
	"KEY_LEFT_BRACKET",
	"KEY_BACK_SLASH",
	"KEY_RIGHT_BRACKET",
	"KEY_UNDERSCORE",
	"KEY_GRAVE",
	"KEY_A",
	"KEY_B",
	"KEY_C",
	"KEY_D",
	"KEY_E",
	"KEY_F",
	"KEY_G",
	"KEY_H",
	"KEY_I",
	"KEY_J",
	"KEY_K",
	"KEY_L",
	"KEY_M",
	"KEY_N",
	"KEY_O",
	"KEY_P",
	"KEY_Q",
	"KEY_R",
	"KEY_S",
	"KEY_T",
	"KEY_U",
	"KEY_V",
	"KEY_W",
	"KEY_X",
	"KEY_Y",
	"KEY_Z",
	"KEY_LEFT_BRACE",
	"KEY_BAR",
	"KEY_RIGHT_BRACE",
	"KEY_TILDE",
	"KEY_EURO",
	"KEY_POUND",
	"KEY_YEN",
	"KEY_MIDDLE_DOT",
	"KEY_SEARCH",
	"KEY_DPAD_LEFT",
	"KEY_DPAD_RIGHT",
	"KEY_DPAD_UP",
	"KEY_DPAD_DOWN",
	"KEY_DPAD_CENTER",
	"KEY_ENTER",
	"KEY_PLAY"
};
#pragma endregion keyCodeStr

#endif /* defined(_GlOBAL_H_) */

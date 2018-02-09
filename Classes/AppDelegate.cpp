#include "AppDelegate.h"
#include "GameScene.h"

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(1280, 720);

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// If you want to use packages manager to install more packages, 
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching(char* cmdLine) {
    // initialize director
    auto director = Director::getInstance();
    auto glView = director->getOpenGLView();

	//  Init screen data
	bool fullscreen = (strcmp(cmdLine, "fullscreen") == 0);
    
    if(!glView) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	DEVMODE devmode;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);

    if (fullscreen)
    {
        glView = GLViewImpl::createWithRect("SuperBingo", Rect(0, 0, devmode.dmPelsWidth, devmode.dmPelsHeight));
    }
    else
#endif
    {
        glView = GLViewImpl::createWithRect("SuperBingo", Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
    }
#else
		glView = GLViewImpl::create("SuperBingo");
#endif

        director->setOpenGLView(glView);
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		if (fullscreen)
		{
			HWND hwnd = Director::getInstance()->getOpenGLView()->getWin32Window();
			HDC hdc = GetDC(hwnd);
			devmode.dmSize = sizeof(DEVMODE);
			EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
			SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, devmode.dmPelsWidth, devmode.dmPelsHeight, SWP_SHOWWINDOW);
			LONG fullscreen = ChangeDisplaySettings(&devmode, CDS_FULLSCREEN);
			ShowWindow(hwnd, SW_MAXIMIZE);
        }
#endif
    }

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60.0f);

    // Set the design resolution
	glView->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);

    register_all_packages();

    // create a scene. it's an autorelease object
    auto scene = GameScene::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

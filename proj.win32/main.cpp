#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"

USING_NS_CC;

void lptstr2str(LPTSTR tch, char* &pch) // or (TCHAR* tch, char* &pch)
{
#ifndef UNICODE
	std::memcpy(pch, tch, strlen(tch) + 1);
#else
	size_t n =
		sizeof(TCHAR) / sizeof(char)* wcsnlen(tch, std::string::npos);
	pch = new char[n + 1];
	std::memcpy(pch, tch, n + 1);
	int len = n - std::count(pch, pch + n, NULL);
	std::remove(pch, pch + n, NULL);
	pch[len] = NULL;
#endif
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance

	char* pCopy = NULL;
	lptstr2str(lpCmdLine, pCopy);

	AppDelegate app;
    return Application::getInstance()->run(pCopy);
}

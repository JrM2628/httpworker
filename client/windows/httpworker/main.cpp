#include "config.h"
#include "util.h"
#include "core.h"
#include "mainloop.h"


#ifdef _DEBUG // DEBUG
int main()
#else // RELEASE
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
#endif 
{
	mainloop();
}
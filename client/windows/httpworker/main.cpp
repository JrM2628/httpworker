#include "..\common\config.h"
#include "..\common\util.h"
#include "..\common\core.h"
#include "..\common\mainloop.h"


#ifdef _DEBUG // DEBUG
int main()
#else // RELEASE
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) 
#endif 
{
	mainloop();
}
#include "config.h"
#include "util.h"
#include "core.h"
#include "mainloop.h"


// Main is used for debug (prints output to stdout)
//int main(){
// WinMain is used for release (no gui)
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	mainloop();
}
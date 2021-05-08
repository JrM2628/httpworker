#include <wtypes.h>
#include "Gdiplus.h"
#include <vector>
#include <iostream>
#pragma comment(lib,"gdiplus.lib")

BITMAPINFOHEADER createBitmapHeader(int width, int height)
{
    BITMAPINFOHEADER  bi;
    // create a bitmap
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0; 
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    return bi;
}

HBITMAP GdiPlusScreenCapture(HWND hWnd)
{
    // get handles to a device context (DC)
    HDC hwindowDC = GetDC(hWnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // define scale, height and width
    int scale = 1;
    int screenx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screeny = GetSystemMetrics(SM_YVIRTUALSCREEN);
    DEVMODEA dm;
    EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &dm);
    int width = dm.dmPelsWidth;
    int height = dm.dmPelsHeight;
    //int width = GetSystemMetrics(SM_CXFULLSCREEN);
    //int height = GetSystemMetrics(SM_CYFULLSCREEN);
    std::cout << screenx << " " << screeny << " " << width << " " << height << std::endl;

    // create a bitmap
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    BITMAPINFOHEADER bi = createBitmapHeader(width, height);

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that call HeapAlloc using a handle to the process's default heap.
    // Therefore, GlobalAlloc and LocalAlloc have greater overhead than HeapAlloc.
    DWORD dwBmpSize = ((width * bi.biBitCount + 31) / 32) * 4 * height;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, screenx, screeny, width, height, SRCCOPY);   //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // avoid memory leak
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hWnd, hwindowDC);
    return hbwindow;
}

bool saveToMemory(HBITMAP* hbitmap, std::vector<BYTE>& data)
{
    Gdiplus::Bitmap bmp(*hbitmap, nullptr);
    // write to IStream
    IStream* istream = nullptr;
    CreateStreamOnHGlobal(NULL, TRUE, &istream);

    // define encoding
    CLSID clsid;
    CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &clsid); 
    Gdiplus::Status status = bmp.Save(istream, &clsid, NULL);
    if (status != Gdiplus::Status::Ok)
        return false;

    // get memory handle associated with istream
    HGLOBAL hg = NULL;
    GetHGlobalFromStream(istream, &hg);

    // copy IStream to buffer
    int bufsize = GlobalSize(hg);
    data.resize(bufsize);

    // lock & unlock memory
    LPVOID pimage = GlobalLock(hg);
    memcpy(&data[0], pimage, bufsize);
    GlobalUnlock(hg);
    istream->Release();
    return true;
}

std::vector<BYTE> doTheThing() {
    // Initialize GDI+.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // get the bitmap handle to the bitmap screenshot
    HWND hWnd = GetDesktopWindow();
    HBITMAP hBmp = GdiPlusScreenCapture(hWnd);
    std::vector<BYTE> data;
    if (saveToMemory(&hBmp, data)) {
        std::cout << "Screenshot saved to memory" << std::endl;
    }
    else {
        std::cout << "Error: Couldn't save screenshot to memory" << std::endl;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return data;
}
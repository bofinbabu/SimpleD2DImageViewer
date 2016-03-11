
typedef struct FileArray {
		LPWSTR *psFilePathArray;
		DWORD dwCount;
	}myFileArray;

#pragma once

#include "Resource.h"

const float DEFAULT_DPI = 96.f;   // Default DPI that maps image resolution directly to screen resoltuion




class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    HRESULT Initialize(HINSTANCE hInstance);

	

	

private:
	FileArray CreateD2DBitmapFromFile(HWND hWnd);
	FileArray LocateImageFile(HWND hWnd, LPWSTR pszFileName, DWORD cbFileName, WCHAR szFilename);
	HRESULT RenderImg(HWND hWnd, LPWSTR szFileName);
    HRESULT CreateDeviceResources(HWND hWnd);
	LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnPaint(HWND hWnd);

    static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    
    HINSTANCE               m_hInst;
    IWICImagingFactory     *m_pIWICFactory;

    ID2D1Factory           *m_pD2DFactory;
    ID2D1HwndRenderTarget  *m_pRT;
    ID2D1Bitmap            *m_pD2DBitmap;
    IWICFormatConverter    *m_pConvertedSourceBitmap;
};









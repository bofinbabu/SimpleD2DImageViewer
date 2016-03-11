#include <windows.h>
#include <WinBase.h>
#include <wincodec.h>
#include <commdlg.h>
#include <d2d1.h>
#include <Shobjidl.h>
#include "WICViewerD2D.h"

/* SafeRelease: https://msdn.microsoft.com/en-us/library/windows/desktop/dd940435(v=vs.85).aspx */


template <typename T>
inline void SafeRelease(T *&p) 
{
    if (nullptr != p)
    {
        p->Release();
        p = nullptr;
    }
}


int WINAPI wWinMain(
     HINSTANCE hInstance,
     HINSTANCE hPrevInstance,
     LPWSTR pszCmdLine,
     int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pszCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

	//MessageBox(NULL, L"Inside WinMain", L"Testing...", MB_OK);

    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    if (SUCCEEDED(hr))
    {
        {
            DemoApp app;
            hr = app.Initialize(hInstance);

            if (SUCCEEDED(hr))
            {
                BOOL fRet;
                MSG msg;

                // Main message loop:
                while ((fRet = GetMessage(&msg, nullptr, 0, 0)) != 0)
                {
                    if (fRet == -1)
                    {
                        break;
                    }
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
            }
        }
        CoUninitialize();
    }

    return 0;
}

//intialize member data

DemoApp::DemoApp()
    :
    m_pD2DBitmap(nullptr),
    m_pConvertedSourceBitmap(nullptr),
    m_pIWICFactory(nullptr),
    m_pD2DFactory(nullptr),
    m_pRT(nullptr)
{
	}


DemoApp::~DemoApp()
{
    SafeRelease(m_pD2DBitmap);
    SafeRelease(m_pConvertedSourceBitmap);
    SafeRelease(m_pIWICFactory);
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pRT);
}



HRESULT DemoApp::Initialize(HINSTANCE hInstance)
{
    
    HRESULT hr = S_OK;
	//MessageBox(NULL, L"Inside Initialize", L"Testing...", MB_OK);
    // Create WIC factory
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pIWICFactory)
        );

    if (SUCCEEDED(hr))
    {
        // Create D2D factory
        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    }

    if (SUCCEEDED(hr))
    {
        WNDCLASSEX wcex;

        // Register window class
        wcex.cbSize        = sizeof(WNDCLASSEX);
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = DemoApp::s_WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = hInstance;
        wcex.hIcon         = nullptr;
        wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
        wcex.lpszClassName = L"WICViewerD2D";
        wcex.hIconSm       = nullptr;

        m_hInst = hInstance;

        hr = RegisterClassEx(&wcex) ? S_OK : E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        // Create window
        HWND hWnd = CreateWindow(
            L"WICViewerD2D",
            L"Image Viewer",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            1280,
            1024,
            nullptr,
            nullptr,
            hInstance,
            this
            );

        hr = hWnd ? S_OK : E_FAIL;
    }

    return hr;
}

HRESULT DemoApp::RenderImg(HWND hWnd, LPWSTR szFileName) {

	// Create a decoder
	IWICBitmapDecoder *pDecoder = nullptr;

	HRESULT hr;

	hr = m_pIWICFactory->CreateDecoderFromFilename(	szFileName, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand,  &pDecoder );
	if (SUCCEEDED(hr)) {
		//MessageBox(NULL, L"Decoder created!!!", L"Testing...", MB_OK);
		//Sleep(1000);

	}

	// Retrieve the first frame of the image from the decoder

	IWICBitmapFrameDecode *pFrame = nullptr;



	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pFrame);
		if (SUCCEEDED(hr)) {
			//MessageBox(NULL, L"Got first frame...", L"Testing...", MB_OK);
		}
	}

	//Format convert the frame to 32bppPBGRA
	if (SUCCEEDED(hr))
	{
		SafeRelease(m_pConvertedSourceBitmap);
		hr = m_pIWICFactory->CreateFormatConverter(&m_pConvertedSourceBitmap);
		if (SUCCEEDED(hr)) {
			//MessageBox(NULL, L"Format convert the frame to 32bppPBGRA--1", L"Testing...", MB_OK);
		}
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pConvertedSourceBitmap->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,	nullptr,  0.f, WICBitmapPaletteTypeCustom );
		if (SUCCEEDED(hr)) {
			//MessageBox(NULL, L"Format convert the frame to 32bppPBGRA--2", L"Testing...", MB_OK);
		}
	}


	if (SUCCEEDED(hr))
	{
		// Need to release the previous D2DBitmap if there is one
		SafeRelease(m_pD2DBitmap);
		hr = m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, nullptr, &m_pD2DBitmap);
		if (SUCCEEDED(hr)) {
			//MessageBox(NULL, L"Success: CreateBitmapFromWicBitmap", L"Testing...", MB_OK);
		}
	}



	//InvalidateRect(hWnd, nullptr, TRUE);

	SafeRelease(pDecoder);
	SafeRelease(pFrame);

	//Sleep(2000);

	return hr;
}

LRESULT CALLBACK DemoApp::s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DemoApp *pThis;
	LRESULT lRet = 0;

	if (uMsg == WM_NCCREATE)
	{
		auto pcs = reinterpret_cast<LPCREATESTRUCT> (lParam);
		pThis = reinterpret_cast<DemoApp *> (pcs->lpCreateParams);

		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR> (pThis));
		lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		pThis = reinterpret_cast<DemoApp *> (GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (pThis)
		{
			lRet = pThis->WndProc(hWnd, uMsg, wParam, lParam);
		}
		else
		{
			lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
	return lRet;
}

FileArray DemoApp::CreateD2DBitmapFromFile(HWND hWnd)
{
    HRESULT hr = S_OK;

	//MessageBox(NULL, L"Inside CreateD2DBitMapFromFile", L"Testing...", MB_OK);

    WCHAR szFileName[1025];

    FileArray psFilePathArray;

	
	// Create the open dialog box and locate the image file
	psFilePathArray = LocateImageFile(hWnd, szFileName, ARRAYSIZE(szFileName), *szFileName);
  
	
    return psFilePathArray;
}


FileArray DemoApp::LocateImageFile(HWND hWnd, LPWSTR pszFileName, DWORD cchFileName, WCHAR szFileName)
{
	
	//MessageBox(NULL, L"Inside LocateImage", L"Testing...", MB_OK);

	HRESULT hr;

	IFileOpenDialog * pdf;
	IShellItemArray *psItemArray = nullptr;
	DWORD dCount;
	FileArray myFileArray;
	myFileArray.psFilePathArray = nullptr;
	hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pdf));
	if (SUCCEEDED(hr)) {
		DWORD dOptions;
		hr = pdf->GetOptions(&dOptions);
		if (SUCCEEDED(hr)) {
			hr = pdf->SetOptions(dOptions | FOS_ALLOWMULTISELECT);
		}
		if (SUCCEEDED(hr)) {
			//Show Open
			pdf->Show(NULL); //hWnd
			hr = pdf->GetResults(&psItemArray); // remember to release pdf
		}
		if (SUCCEEDED(hr)) {
			hr = psItemArray->GetCount(&dCount);
            myFileArray.dwCount = dCount;
			if (SUCCEEDED(hr)) {
				myFileArray.psFilePathArray = (LPWSTR *)malloc(dCount * sizeof(LPWSTR));
				for (DWORD dItem = 0; dItem < dCount; dItem++) {
					IShellItem *pItem;
					psItemArray->GetItemAt(dItem, &pItem);
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH , &(myFileArray.psFilePathArray[dItem]));

					if (SUCCEEDED(hr)) {
						//MessageBox(nullptr, psFilePathArray[dItem], L"Testing: File Path from LocatImg", MB_OK);
					}

				}

			}
		}
				
	}
	SafeRelease(pdf);

	return myFileArray;
	/*HRESULT hrl;
	
	pszFileName[0] = L'\0';

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = hWnd;
    ofn.lpstrFilter     = L"All Image Files\0"  L"*.bmp;*.dib;*.wdp;*.mdp;*.hdp;*.gif;*.png;*.jpg;*.jpeg;*.tif;*.ico\0"
                          L"All Files\0"        L"*.*\0"
                          L"\0";
    ofn.lpstrFile       = pszFileName;
    ofn.nMaxFile        = 1024; //The size, in characters, of the buffer pointed to by lpstrFile
    ofn.lpstrTitle      = L"Open Image (Multiple images can be opnened)";
	ofn.Flags           = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_NOCHANGEDIR;
	
    hrl = GetOpenFileName(&ofn); // Display the Open dialog box.

	
    return ofn; */
	
}




HRESULT DemoApp::CreateDeviceResources(HWND hWnd)
{
    HRESULT hr = S_OK;

	//MessageBox(NULL, L"Inside CreateDeviceResources", L"Testing...", MB_OK);

    if (!m_pRT) //m_pRT is of type ID2D1HwndRenderTarget
    {
        RECT rc;
        hr = GetClientRect(hWnd, &rc) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            auto renderTargetProperties = D2D1::RenderTargetProperties();

            // Set the DPI to be the default system DPI to allow direct mapping
            // between image pixels and desktop pixels in different system DPI settings
            renderTargetProperties.dpiX = DEFAULT_DPI;
            renderTargetProperties.dpiY = DEFAULT_DPI;

            auto size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

            hr = m_pD2DFactory->CreateHwndRenderTarget(
                renderTargetProperties,
                D2D1::HwndRenderTargetProperties(hWnd, size),
                &m_pRT
                );
        }
    }

    return hr;
}



LRESULT DemoApp::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
    switch (uMsg)
    {
		case WM_COMMAND:
        {
            // Parse the menu selections:
            switch (LOWORD(wParam))
            {
                case IDM_FILE:
                {
					//MessageBox(NULL, L"Inside WinProc->WM_COM->IDM_FILE",L"Testing...", MB_OK);

					FileArray psFilePathArray;

					psFilePathArray = CreateD2DBitmapFromFile(hWnd);

					HRESULT hr = S_OK;

					DWORD dCount = 0;
					DWORD dItem = 0;


					//MessageBox(NULL, psFilePathArray[0], L"Testing: File Path from WinProc", MB_OK);

					CreateDeviceResources(hWnd);
					//dCount = wcslen(* psFilePathArray);

					dCount = psFilePathArray.dwCount;
					while (dItem < dCount) {
						//MessageBox(NULL, psFilePathArray.psFilePathArray[dItem], L"Testing: File Path from WinProc", MB_OK);
						RenderImg(hWnd, psFilePathArray.psFilePathArray[dItem]);
						InvalidateRect(hWnd, nullptr, TRUE);
						Sleep(1000);
						dItem++;
					}

									
                    break;
                }
                case IDM_EXIT:
                {
					//MessageBox(NULL, L"Inside WinProc->WM_COM->IDM_EXIT", L"Testing...", MB_OK);
                    PostMessage(hWnd, WM_CLOSE, 0, 0);
                    break;
                }
            }
            break;
        }
        case WM_SIZE:
        {
			//MessageBox(NULL, L"Inside WinProc->WM_SIZE", L"Testing...", MB_OK);
            auto size = D2D1::SizeU(LOWORD(lParam), HIWORD(lParam));

            if (m_pRT)
            {
				
                // If we couldn't resize, release the device and we'll recreate it
                // during the next render pass.
                if (FAILED(m_pRT->Resize(size)))
                {
                    SafeRelease(m_pRT);
                    SafeRelease(m_pD2DBitmap);
                }
            }
            break;
        }
        case WM_PAINT:
        {
			//MessageBox(NULL, L"Inside WinProc->WM_PAINT", L"Testing...", MB_OK);
            return OnPaint(hWnd);
        }
        case WM_DESTROY:
        {
			//MessageBox(NULL, L"Inside WinProc->WM_DESTROY", L"Testing...", MB_OK);
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}


LRESULT DemoApp::OnPaint(HWND hWnd)
{
    HRESULT hr = S_OK;
    PAINTSTRUCT ps;
	//MessageBox(NULL, L"Inside Onpaint", L"Testing...", MB_OK);
    if (BeginPaint(hWnd, &ps))
    {
		//May be this causes to paint only the last img.
        // Create render target if not yet created
        hr = CreateDeviceResources(hWnd);

        if (SUCCEEDED(hr) && !(m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            m_pRT->BeginDraw();

            m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

            // Clear the background
            m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

            auto rtSize = m_pRT->GetSize();

            // Create a rectangle with size of current window
            auto rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

            // D2DBitmap may have been released due to device loss. 
            // If so, re-create it from the source bitmap
			
            if (m_pConvertedSourceBitmap && !m_pD2DBitmap)
            {
                m_pRT->CreateBitmapFromWicBitmap(m_pConvertedSourceBitmap, nullptr, &m_pD2DBitmap);
            }
			
            // Draws an image and scales it to the current window size
            if (m_pD2DBitmap)
            {
                m_pRT->DrawBitmap(m_pD2DBitmap, rectangle);
            }

            hr = m_pRT->EndDraw();

            // In case of device loss, discard D2D render target and D2DBitmap
            // They will be re-created in the next rendering pass

            if (hr == D2DERR_RECREATE_TARGET)
            {
                SafeRelease(m_pD2DBitmap);
                SafeRelease(m_pRT);
                // Force a re-render
                hr = InvalidateRect(hWnd, nullptr, TRUE)? S_OK : E_FAIL;
            }
        }

        EndPaint(hWnd, &ps);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}  

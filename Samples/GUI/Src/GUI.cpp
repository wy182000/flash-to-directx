//---------------------------------------------------------------------
// Copyright (c) 2009 Maksym Diachenko, Viktor Reutskyy, Anton Suchov.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//---------------------------------------------------------------------

#include "stdafx.h"
#include "GUI.h"

#pragma comment(lib, "FlashDX" CONFIGURATION_NAME "_" PLATFORM_NAME ".lib")

//---------------------------------------------------------------------
const int max_loadstring = 100;
const int window_width = 700;
const int window_height = 350;
const int num_textures_in_rotation = 2;
const wchar_t* movie_path = L"Data/VT.swf";
const IFlashDXPlayer::ETransparencyMode transparency_mode = IFlashDXPlayer::TMODE_FULL_ALPHA;

// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;
TCHAR szTitle[max_loadstring];					// The title bar text
TCHAR szWindowClass[max_loadstring];			// the main window class name

IFlashDX*			g_flashDX = NULL;
IFlashDXPlayer*		g_flashPlayer = NULL;

ASInterface*		g_playerASI;

IDirect3D9*			g_direct3D = NULL;
IDirect3DDevice9*	g_device = NULL;
D3DPRESENT_PARAMETERS g_params;

IDirect3DTexture9*	g_texturesRotation[num_textures_in_rotation] = { NULL };
int					g_currentTexture = 0;
IDirect3DTexture9*	g_textureGUI = NULL;

int					g_windowWidth = 0;
int					g_windowHeight = 0;

//---------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int iFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	iFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(iFlag);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, max_loadstring);
	LoadString(hInstance, IDC_GUI, szWindowClass, max_loadstring);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUI));

	// Main message loop:
	bool noQuit = true;
	while (noQuit)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				noQuit = false;
		}
		else
		{
			DrawFrame();
		}
	}

	ExitInstance();
	return (int) msg.wParam;
}

//---------------------------------------------------------------------
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GUI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GUI);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//---------------------------------------------------------------------
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	// Set current directory to module directory
	wchar_t imagePath[MAX_PATH];
	GetModuleFileName(NULL, imagePath, MAX_PATH);
	size_t pathLen = wcslen(imagePath);
	wchar_t* pathEnd = imagePath + pathLen - 1;
	while (pathEnd >= imagePath && (*pathEnd != L'\\' && *pathEnd != L'/'))
		--pathEnd;

	if (pathEnd >= imagePath)
		*pathEnd++ = 0;

	SetCurrentDirectory(imagePath);

	// Create window
	RECT rc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	//---------------------------------------------------------------------
	// Initialize Direct3D9
	g_direct3D = Direct3DCreate9(D3D_SDK_VERSION);

	g_params.Windowed					= TRUE;
	g_params.BackBufferWidth			= window_width;
	g_params.BackBufferHeight			= window_height;
	g_params.BackBufferFormat			= D3DFMT_A8R8G8B8;
	g_params.BackBufferCount			= 0;
	g_params.MultiSampleType			= D3DMULTISAMPLE_NONE;
	g_params.MultiSampleQuality			= 0;
	g_params.SwapEffect					= D3DSWAPEFFECT_DISCARD;
	g_params.hDeviceWindow				= hWnd;
	g_params.EnableAutoDepthStencil		= FALSE;
	g_params.AutoDepthStencilFormat		= D3DFMT_D24S8;
	g_params.Flags						= 0; //D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	g_params.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;
	g_params.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;

	// Create device
	DWORD Flags = D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_FPU_PRESERVE;
	if (FAILED(g_direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
										Flags, &g_params, &g_device)))
	{
		return FALSE;
	}

	//---------------------------------------------------------------------
	// Create render targets
	if (!RecreateTargets(window_width, window_height))
		return FALSE;

	//---------------------------------------------------------------------
	// Initialize Flash-to-DirectX
	//---------------------------------------------------------------------
	g_flashDX = GetFlashToDirectXInstance();
	double flashVersion = g_flashDX->GetFlashVersion();
	g_flashPlayer = g_flashDX->CreatePlayer(window_width, window_height);

	if (!g_flashPlayer)
	{
		MessageBox(NULL, L"Flash Player failed to initialize.", L"Error", MB_OK);
		return FALSE;
	}

	g_playerASI = new ASInterface(g_flashPlayer);

	//---------------------------------------------------------------------
	// Function callbacks example
	//---------------------------------------------------------------------
	{
		struct TestCallbacks
		{
			static void test1(bool yes, ASValue::Array arr)
			{
			}
			int test2()
			{
				return 0;
			}
			void command1(const wchar_t*)
			{
			}
			static void fsCommandDef(const wchar_t*, const wchar_t*)
			{
			}
		};

		TestCallbacks s_testCallbacks;

		g_playerASI->AddCallback(L"test1", &TestCallbacks::test1);
		g_playerASI->AddCallback(L"test2", s_testCallbacks, &TestCallbacks::test2);

		g_playerASI->AddFSCCallback(L"command1", s_testCallbacks, &TestCallbacks::command1);
		g_playerASI->SetDefaultFSCCallback(&TestCallbacks::fsCommandDef);
	}

	//---------------------------------------------------------------------
	// Load and play movie
	//---------------------------------------------------------------------
	g_flashPlayer->LoadMovie(movie_path);

	g_flashPlayer->SetTransparencyMode(transparency_mode);
	g_flashPlayer->SetBackgroundColor(RGB(0, 0, 0));

	//---------------------------------------------------------------------
	// Function call example
	//---------------------------------------------------------------------
	/*{
		bool boolResult = g_playerASI->Call(L"test", true);
		int numberResult = g_playerASI->Call(L"test1", 22);
		std::wstring stringResult = g_playerASI->Call(L"test2", 123.456);
		ASValue::Array arrayResult = g_playerASI->Call(L"test3", stringResult);
		ASValue::Object objectResult = g_playerASI->Call(L"test4", arrayResult);
	}*/

	// Show window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//---------------------------------------------------------------------
void ExitInstance()
{
	delete g_playerASI;

	g_flashDX->DestroyPlayer(g_flashPlayer);

	for (int i = 0; i < num_textures_in_rotation; ++i)
		SAFE_RELEASE(g_texturesRotation[i]);
	SAFE_RELEASE(g_textureGUI);

	SAFE_RELEASE(g_device);
	SAFE_RELEASE(g_direct3D);
}

//---------------------------------------------------------------------
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEMOVE:
		if (g_flashPlayer)
			g_flashPlayer->SetMousePos(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONDOWN:
		if (g_flashPlayer)
			g_flashPlayer->SetMouseButtonState(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), IFlashDXPlayer::eMouse1, true);
		return 0;
	case WM_LBUTTONUP:
		if (g_flashPlayer)
			g_flashPlayer->SetMouseButtonState(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), IFlashDXPlayer::eMouse1, false);
		return 0;
	case WM_KEYDOWN:
		if (g_flashPlayer)
			g_flashPlayer->SendKey(true, wParam, lParam);
		return 0;
	case WM_KEYUP:
		if (g_flashPlayer)
			g_flashPlayer->SendKey(false, wParam, lParam);
		return 0;
	case WM_CHAR:
		if (g_flashPlayer)
			g_flashPlayer->SendChar(wParam, lParam);
		return 0;
	case WM_SIZE:
		if (g_flashPlayer)
		{
			SAFE_RELEASE(g_textureGUI);
			g_params.BackBufferWidth = GET_X_LPARAM(lParam);
			g_params.BackBufferHeight = GET_Y_LPARAM(lParam);
			g_device->Reset(&g_params);
			RecreateTargets(g_params.BackBufferWidth, g_params.BackBufferHeight);
		}
		return 0;
	case WM_SETCURSOR:
		if (g_flashPlayer)
		{
			static bool restoreCursor = true;
			if (LOWORD(lParam) != HTCLIENT)
				restoreCursor = true;

			if (restoreCursor)
			{
				restoreCursor = false;
				break; // DefWindowProc will set the cursor
			}
			return 1;
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
void DrawFrame()
{
	HRESULT hr;

	// Update flash movie if necessarily
	unsigned int numDirtyRects; const RECT* dirtyRects;
	if (g_flashPlayer->IsNeedUpdate(NULL, &dirtyRects, &numDirtyRects))
	{
		IDirect3DTexture9* pTexToUpdate = g_texturesRotation[g_currentTexture];
		if (++g_currentTexture == num_textures_in_rotation)
			g_currentTexture = 0;

		IDirect3DSurface9* pSrcSurface;
		hr = pTexToUpdate->GetSurfaceLevel(0, &pSrcSurface);
		assert(SUCCEEDED(hr));

		HDC surfaceDC;
		hr = pSrcSurface->GetDC(&surfaceDC);
		assert(SUCCEEDED(hr));

		// Draw flash frame
		g_flashPlayer->DrawFrame(surfaceDC);

		hr = pSrcSurface->ReleaseDC(surfaceDC);
		assert(SUCCEEDED(hr));

		// Update our GUI texture
		IDirect3DSurface9* pDestSurface;
		hr = g_textureGUI->GetSurfaceLevel(0, &pDestSurface);
		assert(SUCCEEDED(hr));

		for (unsigned int i = 0; i < numDirtyRects; ++i)
		{
			POINT destPoint = { dirtyRects[i].left, dirtyRects[i].top };
			hr = g_device->UpdateSurface(pSrcSurface, dirtyRects + i, pDestSurface, &destPoint);
			assert(SUCCEEDED(hr));
		}

		pDestSurface->Release();
		pSrcSurface->Release();
	}

	//---------------------------------------------------------------------
	struct TLVERTEX
	{
		float x;
		float y;
		float z;
		float rhw;
		D3DCOLOR color;
		float u;
		float v;
	};

	const DWORD D3DFVF_TLVERTEX = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

	float halfInvWindowWidth = 1.0f / g_windowWidth * 0.5f;
	float halfInvWindowHeight = 1.0f / g_windowHeight * 0.5f;

	// Create quad vertices
	TLVERTEX vertices[4];
	vertices[0].x = 0; vertices[0].y = 0;
	vertices[0].z = 0.0f; vertices[0].rhw = 1.0f;
	vertices[0].color = 0xFFFFFFFF;
	vertices[0].u = halfInvWindowWidth; vertices[0].v = halfInvWindowHeight;

	vertices[1].x = (float)g_windowWidth; vertices[1].y = 0;
	vertices[1].z = 0.0f; vertices[1].rhw = 1.0f;
	vertices[1].color = 0xFFFFFFFF;
	vertices[1].u = 1.0f + halfInvWindowWidth; vertices[1].v = halfInvWindowHeight;

	vertices[2].x = (float)g_windowWidth; vertices[2].y = (float)g_windowHeight;
	vertices[2].z = 0.0f; vertices[2].rhw = 1.0f;
	vertices[2].color = 0xFFFFFFFF;
	vertices[2].u = 1.0f + halfInvWindowWidth; vertices[2].v = 1.0f + halfInvWindowHeight;

	vertices[3].x = 0; vertices[3].y = (float)g_windowHeight;
	vertices[3].z = 0.0f; vertices[3].rhw = 1.0f;
	vertices[3].color = 0xFFFFFFFF;
	vertices[3].u = halfInvWindowWidth; vertices[3].v = 1.0f + halfInvWindowHeight;

	// Begin frame
	hr = g_device->BeginScene();
	assert(SUCCEEDED(hr));
	hr = g_device->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
	assert(SUCCEEDED(hr));

	// Draw the quad
	hr = g_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	assert(SUCCEEDED(hr));

	hr = g_device->SetRenderState(D3DRS_FOGENABLE, false);
	assert(SUCCEEDED(hr));
	hr = g_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	assert(SUCCEEDED(hr));

	// Use alpha channel in texture for alpha
	hr = g_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	assert(SUCCEEDED(hr));
	hr = g_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	assert(SUCCEEDED(hr));

	hr = g_device->SetTexture(0, g_textureGUI);
	assert(SUCCEEDED(hr));

	hr = g_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	assert(SUCCEEDED(hr));
	hr = g_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	assert(SUCCEEDED(hr));

	hr = g_device->SetFVF(D3DFVF_TLVERTEX);
	assert(SUCCEEDED(hr));
	hr = g_device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(TLVERTEX));
	assert(SUCCEEDED(hr));

	hr = g_device->EndScene();
	assert(SUCCEEDED(hr));
	hr = g_device->Present(NULL, NULL, NULL, NULL);
	assert(SUCCEEDED(hr));
}

//---------------------------------------------------------------------
bool RecreateTargets(unsigned int newWidth, unsigned int newHeight)
{
	HRESULT hr;

	if (g_flashPlayer)
		g_flashPlayer->ResizePlayer(newWidth, newHeight);

	for (int i = 0; i < num_textures_in_rotation; ++i)
	{
		SAFE_RELEASE(g_texturesRotation[i]);

		hr = g_device->CreateTexture(newWidth, newHeight, 1, 0,
			transparency_mode ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &g_texturesRotation[i], NULL);
		if (FAILED(hr))
			return false;
	}

	SAFE_RELEASE(g_textureGUI);
	hr = g_device->CreateTexture(newWidth, newHeight, 1, 0,
		transparency_mode ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &g_textureGUI, NULL);
	if (FAILED(hr))
		return false;

	g_windowWidth = newWidth;
	g_windowHeight = newHeight;

	return true;
}
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
#include "FlashDXPlayer.h"
#include "shlwapi.h"
#include "algorithm"
#include "sstream"

using namespace ShockwaveFlashObjects;

//---------------------------------------------------------------------
CFlashDXPlayer::CFlashDXPlayer(HMODULE flashDLL, unsigned int width, unsigned int height)
{
	m_userData = NULL;
	m_flashInterface = NULL;
	m_oleObject = NULL;
	m_windowlessObject = NULL;
	m_lastMouseX = 0;
	m_lastMouseY = 0;
	m_lastMouseButtons = 0;

	m_dirtyFlag = false;

	m_width = width;
	m_height = height;

	m_controlSite.Init(this);
	m_controlSite.AddRef();

	m_alphaBlackDC = NULL;
	m_alphaBlackBitmap = NULL;
	m_alphaBlackBuffer = NULL;
	m_alphaWhiteDC = NULL;
	m_alphaWhiteBitmap = NULL;
	m_alphaWhiteBuffer = NULL;

	HRESULT hr;

	typedef HRESULT (__stdcall *DllGetClassObjectFunc)(REFCLSID rclsid, REFIID riid, LPVOID * ppv);

	if (flashDLL != NULL)
	{
		IClassFactory* aClassFactory = NULL;
		DllGetClassObjectFunc aDllGetClassObjectFunc = (DllGetClassObjectFunc) GetProcAddress(flashDLL, "DllGetClassObject");
		hr = aDllGetClassObjectFunc(CLSID_ShockwaveFlash, IID_IClassFactory, (void**)&aClassFactory);

		if (FAILED(hr))
			return;

		aClassFactory->CreateInstance(NULL, IID_IOleObject, (void**)&m_oleObject);
		aClassFactory->Release();	
	}
	else
	{
		hr = CoCreateInstance(CLSID_ShockwaveFlash, NULL, CLSCTX_INPROC_SERVER, IID_IOleObject, (void**)&m_oleObject);

		if (FAILED(hr))
			return;
	}

	IOleClientSite* pClientSite = NULL;
	hr = m_controlSite.QueryInterface(__uuidof(IOleClientSite), (void**)&pClientSite);
	if (FAILED(hr))
		return;

	hr = m_oleObject->SetClientSite(pClientSite);
	if (FAILED(hr))
		return;

	hr = m_oleObject->QueryInterface(__uuidof(IShockwaveFlash), (void**)&m_flashInterface);
	if (FAILED(hr))
		return;

	m_flashInterface->DisableLocalSecurity();
	m_flashInterface->PutEmbedMovie(FALSE);
	m_flashInterface->PutAllowScriptAccess(L"always");
	SetTransparencyMode(IFlashDXPlayer::TMODE_OPAQUE);
	SetQuality(IFlashDXPlayer::QUALITY_HIGH);

	hr = m_oleObject->DoVerb(OLEIVERB_INPLACEACTIVATE, NULL, pClientSite, 0, NULL, NULL);
	assert(SUCCEEDED(hr));

	pClientSite->Release();

	hr = m_oleObject->QueryInterface(__uuidof(IOleInPlaceObjectWindowless), (void**)&m_windowlessObject);
	assert(SUCCEEDED(hr));

	m_flashSink.Init(this);
	m_flashSink.AddRef();

	// Resize player
	ResizePlayer(width, height);
}

//---------------------------------------------------------------------
CFlashDXPlayer::~CFlashDXPlayer()
{
	if (m_alphaBlackDC)
	{
		DeleteDC(m_alphaBlackDC);
		DeleteObject(m_alphaBlackBitmap);
		DeleteDC(m_alphaWhiteDC);
		DeleteObject(m_alphaWhiteBitmap);
	}

	SAFE_RELEASE(m_windowlessObject);
	SAFE_RELEASE(m_flashInterface);

	m_flashSink.Shutdown();
	m_flashSink.Release();

	if (m_oleObject)
	{
		m_oleObject->Close(OLECLOSE_NOSAVE);
		m_oleObject->Release();	
	}

	m_controlSite.Release();
}

//---------------------------------------------------------------------
void CFlashDXPlayer::AddDirtyRect(const RECT* pRect)
{
	m_dirtyFlag = true;
	
	if (pRect == NULL)
	{
		RECT rect = { 0, 0, m_width, m_height };
		m_dirtyUnionRect = rect;
		m_dirtyRects.clear();
		m_dirtyRects.push_back(m_dirtyUnionRect);
	}
	else
	{
		RECT screenRect = {0, 0, m_width, m_height};

		RECT newRect = {0};
		IntersectRect(&newRect, pRect, &screenRect);

		if (IsRectEmpty(&newRect))
			return;

		#define MIN_MACRO(x, y) ((x) < (y) ? (x) : (y))
		#define MAX_MACRO(x, y) ((x) > (y) ? (x) : (y))

		for (std::vector<RECT>::iterator it = m_dirtyRects.begin(); it != m_dirtyRects.end(); ++it)
			if (it->left <= newRect.left && it->top <= newRect.top &&
				it->right >= newRect.right && it->bottom >= newRect.bottom)
			{
				return; // already included
			}

		RECT rect = { newRect.left, newRect.top, newRect.right, newRect.bottom };
		rect.left = MAX_MACRO(rect.left, 0);
		rect.top = MAX_MACRO(rect.top, 0);
		rect.right = MIN_MACRO(rect.right, (LONG)m_width);
		rect.bottom = MIN_MACRO(rect.bottom, (LONG)m_height);

		m_dirtyUnionRect.left = MIN_MACRO(m_dirtyUnionRect.left, newRect.left);
		m_dirtyUnionRect.top = MIN_MACRO(m_dirtyUnionRect.top, newRect.top);
		m_dirtyUnionRect.right = MAX_MACRO(m_dirtyUnionRect.right, newRect.right);
		m_dirtyUnionRect.bottom = MAX_MACRO(m_dirtyUnionRect.bottom, newRect.bottom);

		m_dirtyRects.push_back(rect);
	}
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetUserData(intptr_t data)
{
	m_userData = data;
}

//---------------------------------------------------------------------
intptr_t CFlashDXPlayer::GetUserData() const
{
	return m_userData;
}

//---------------------------------------------------------------------
IFlashDXPlayer::EState CFlashDXPlayer::GetState() const
{
	if (!m_flashInterface)
		return IFlashDXPlayer::STATE_IDLE;

	if (m_flashInterface->IsPlaying())
		return IFlashDXPlayer::STATE_PLAYING;
	return IFlashDXPlayer::STATE_STOPPED;
}

static wchar_t* aQualityNames[6] = { L"Low", L"Medium", L"High", L"Best", L"AutoLow", L"AutoHigh" };

//---------------------------------------------------------------------
IFlashDXPlayer::EQuality CFlashDXPlayer::GetQuality() const
{
	if (m_flashInterface)
	{
		for (int i = 0; i < 6; ++i)
			if (_wcsicmp(aQualityNames[i], m_flashInterface->GetQuality2()) == 0)
				return (IFlashDXPlayer::EQuality)(IFlashDXPlayer::QUALITY_LOW + i);
	}
	return IFlashDXPlayer::QUALITY_HIGH;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetQuality(EQuality quality)
{
	if (m_flashInterface)
	{

		_bstr_t newStr(aQualityNames[quality]);
		m_flashInterface->PutQuality2(newStr);
	}
}

//---------------------------------------------------------------------
IFlashDXPlayer::ETransparencyMode CFlashDXPlayer::GetTransparencyMode() const
{
	return m_transpMode;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetTransparencyMode(ETransparencyMode mode)
{
	if (m_flashInterface)
	{
		switch (mode)
		{
		case IFlashDXPlayer::TMODE_OPAQUE:
			m_flashInterface->PutWMode(_bstr_t("opaque"));
			break;
		case IFlashDXPlayer::TMODE_TRANSPARENT:
		case IFlashDXPlayer::TMODE_FULL_ALPHA:
			m_flashInterface->PutWMode(_bstr_t("transparent"));
			break;
		default:
			break;
		}
	}

	m_transpMode = mode;
}

//---------------------------------------------------------------------
bool CFlashDXPlayer::LoadMovie(const wchar_t* movie)
{
	if (m_flashInterface)
	{
		wchar_t fullpath[MAX_PATH];

		if(!_wfullpath(fullpath, movie, MAX_PATH))
			return false;

		if (!PathFileExists(fullpath))
			return false;

		HRESULT hr = m_flashInterface->put_Movie(_bstr_t(fullpath));
		return SUCCEEDED(hr);
	}

	return false;
}

//---------------------------------------------------------------------
COLORREF CFlashDXPlayer::GetBackgroundColor()
{
	long color = 0;
	if (m_flashInterface)
		color = m_flashInterface->GetBackgroundColor();
	return (COLORREF)color;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetBackgroundColor(COLORREF color)
{
	if (m_flashInterface)
		m_flashInterface->PutBackgroundColor((long)(color & 0x00FFFFFF));
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StartPlaying()
{
	if (m_flashInterface)
		m_flashInterface->Play();
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StartPlaying(const wchar_t* timelineTarget)
{
	if (m_flashInterface)
		m_flashInterface->TPlay(_bstr_t(timelineTarget));
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StopPlaying()
{
	if (m_flashInterface)
		m_flashInterface->StopPlay();
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StopPlaying(const wchar_t* timelineTarget)
{
	if (m_flashInterface)
		m_flashInterface->TStopPlay(_bstr_t(timelineTarget));
}

//---------------------------------------------------------------------
void CFlashDXPlayer::Rewind()
{
	if (m_flashInterface)
		m_flashInterface->Rewind();
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StepForward()
{
	if (m_flashInterface)
		m_flashInterface->Forward();
}

//---------------------------------------------------------------------
void CFlashDXPlayer::StepBack()
{
	if (m_flashInterface)
		m_flashInterface->Back();
}

//---------------------------------------------------------------------
int CFlashDXPlayer::GetCurrentFrame()
{
	if (m_flashInterface)
		return m_flashInterface->CurrentFrame();
	return -1;
}

//---------------------------------------------------------------------
int CFlashDXPlayer::GetCurrentFrame(const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		return m_flashInterface->TCurrentFrame(_bstr_t(timelineTarget));
	return -1;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::GotoFrame(int frame)
{
	if (m_flashInterface)
		m_flashInterface->GotoFrame((long)frame);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::GotoFrame(int frame, const wchar_t* timelineTarget)
{
	if (m_flashInterface)
		m_flashInterface->TGotoFrame(_bstr_t(timelineTarget), (long)frame);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::CallFrame(int frame, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		m_flashInterface->TCallFrame(_bstr_t(timelineTarget), frame);
}

//---------------------------------------------------------------------
const wchar_t* CFlashDXPlayer::GetCurrentLabel(const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
	{
		m_tempStorage = m_flashInterface->TCurrentLabel(_bstr_t(timelineTarget));
		return m_tempStorage.c_str();
	}
	return NULL;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::GotoLabel(const wchar_t* label, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		m_flashInterface->TGotoLabel(_bstr_t(timelineTarget), _bstr_t(label));
}

//---------------------------------------------------------------------
void CFlashDXPlayer::CallLabel(const wchar_t* label, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		m_flashInterface->TCallLabel(_bstr_t(timelineTarget), _bstr_t(label));
}

//---------------------------------------------------------------------
const wchar_t* CFlashDXPlayer::GetVariable(const wchar_t* name)
{
	if (m_flashInterface)
	{
		m_tempStorage = m_flashInterface->GetVariable(_bstr_t(name));
		return m_tempStorage.c_str();
	}
	return NULL;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetVariable(const wchar_t* name, const wchar_t* value)
{
	if (m_flashInterface)
		m_flashInterface->SetVariable(_bstr_t(name), _bstr_t(value));
}

//---------------------------------------------------------------------
const wchar_t* CFlashDXPlayer::GetProperty(int iProperty, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
	{
		m_tempStorage = m_flashInterface->TGetProperty(_bstr_t(timelineTarget), iProperty);
		return m_tempStorage.c_str();
	}
	return NULL;
}

//---------------------------------------------------------------------
double CFlashDXPlayer::GetPropertyAsNumber(int iProperty, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		return m_flashInterface->TGetPropertyNum(_bstr_t(timelineTarget), iProperty);
	return 0;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetProperty(int iProperty, const wchar_t* value, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		m_flashInterface->TSetProperty(_bstr_t(timelineTarget), iProperty, _bstr_t(value));
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetProperty(int iProperty, double value, const wchar_t* timelineTarget /*= L"/"*/)
{
	if (m_flashInterface)
		m_flashInterface->TSetPropertyNum(_bstr_t(timelineTarget), iProperty, value);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::ResizePlayer(unsigned int newWidth, unsigned int newHeight)
{
	IOleInPlaceObject* pInPlaceObject = NULL;
	m_oleObject->QueryInterface(__uuidof(IOleInPlaceObject), (LPVOID*) &pInPlaceObject);

	if (pInPlaceObject != NULL)
	{
		RECT rect = { 0, 0, newWidth, newHeight };
		pInPlaceObject->SetObjectRects(&rect, &rect);
		pInPlaceObject->Release();

		m_width = newWidth;
		m_height = newHeight;

		AddDirtyRect(NULL);
	}

	if (m_alphaBlackDC)
	{
		DeleteDC(m_alphaBlackDC);
		DeleteObject(m_alphaBlackBitmap);
		DeleteDC(m_alphaWhiteDC);
		DeleteObject(m_alphaWhiteBitmap);

		m_alphaBlackDC = NULL;
		m_alphaBlackBitmap = NULL;
		m_alphaBlackBuffer = NULL;
		m_alphaWhiteDC = NULL;
		m_alphaWhiteBitmap = NULL;
		m_alphaWhiteBuffer = NULL;
	}
}

//---------------------------------------------------------------------
bool CFlashDXPlayer::IsNeedUpdate(const RECT** unitedDirtyRect, const RECT** dirtyRects, unsigned int* numDirtyRects)
{
	if (m_dirtyFlag)
	{
		while (ReduceNumDirtyRects());

		m_savedUnionRect = m_dirtyUnionRect;
		m_savedDirtyRects.assign(m_dirtyRects.begin(), m_dirtyRects.end());

		if (unitedDirtyRect)
			*unitedDirtyRect = &m_savedUnionRect;
		if (dirtyRects)
			*dirtyRects = m_savedDirtyRects.size() ? &m_savedDirtyRects.front() : NULL;
		if (numDirtyRects)
			*numDirtyRects = (unsigned int)m_savedDirtyRects.size();
	}

	return m_dirtyFlag;
}

//---------------------------------------------------------------------
bool CFlashDXPlayer::ReduceNumDirtyRects()
{
	for (std::vector<RECT>::iterator firstIt = m_dirtyRects.begin(); firstIt != m_dirtyRects.end(); ++firstIt)
	{
		for (std::vector<RECT>::iterator secondIt = m_dirtyRects.begin(); secondIt != m_dirtyRects.end(); ++secondIt)
		{
			if (firstIt == secondIt)
				continue;

			RECT unionRect;
			if (UnionRect(&unionRect, &(*firstIt), &(*secondIt)))
			{
				if (EqualRect(&unionRect, &(*firstIt)))
				{
					m_dirtyRects.erase(secondIt);
					return true;
				}
				if (EqualRect(&unionRect, &(*secondIt)))
				{
					m_dirtyRects.erase(firstIt);
					return true;
				}
			}
		}
	}

	return false;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::DrawFrame(HDC dc)
{
	if (m_dirtyFlag)
	{
		IViewObject* pViewObject = NULL;
		m_flashInterface->QueryInterface(IID_IViewObject, (LPVOID*) &pViewObject);
		if (pViewObject != NULL)
		{
			// Combine regions
			HRGN unionRgn, first, second = NULL;
			unionRgn = CreateRectRgnIndirect(&m_dirtyRects[0]);
			if (m_dirtyRects.size() >= 2)
				second = CreateRectRgn(0, 0, 1, 1);

			for (std::vector<RECT>::iterator it = m_dirtyRects.begin() + 1; it != m_dirtyRects.end(); ++it)
			{
				// Fill combined region
				first = unionRgn;
				SetRectRgn(second, it->left, it->top, it->right, it->bottom);
				unionRgn = CreateRectRgn(0, 0, 1, 1);

				CombineRgn(unionRgn, first, second, RGN_OR);
				DeleteObject(first);
			}

			if (second)
				DeleteObject(second);

			RECT clipRgnRect; GetRgnBox(unionRgn, &clipRgnRect);
			RECTL clipRect = { 0, 0, m_width, m_height };

			// Fill background
			if (m_transpMode != TMODE_FULL_ALPHA)
			{
				// Set clip region
				SelectClipRgn(dc, unionRgn);

				COLORREF fillColor = GetBackgroundColor();
				HBRUSH fillColorBrush = CreateSolidBrush(fillColor);
				FillRgn(dc, unionRgn, fillColorBrush);
				DeleteObject(fillColorBrush);

				// Draw to main buffer
				HRESULT hr = pViewObject->Draw(DVASPECT_TRANSPARENT, 1, NULL, NULL, NULL, dc, &clipRect, &clipRect, NULL, 0);
				assert(SUCCEEDED(hr));
			}
			else
			{
				if (m_alphaBlackDC == NULL)
				{
					// Create memory buffers
					BITMAPINFOHEADER bih = {0};
					bih.biSize = sizeof(BITMAPINFOHEADER);
					bih.biBitCount = 32;
					bih.biCompression = BI_RGB;
					bih.biPlanes = 1;
					bih.biWidth = LONG(m_width);
					bih.biHeight = -LONG(m_height);

					m_alphaBlackDC = CreateCompatibleDC(dc);
					m_alphaBlackBitmap = CreateDIBSection(m_alphaBlackDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&m_alphaBlackBuffer, 0, 0);
					SelectObject(m_alphaBlackDC, m_alphaBlackBitmap);

					m_alphaWhiteDC = CreateCompatibleDC(dc);
					m_alphaWhiteBitmap = CreateDIBSection(m_alphaWhiteDC, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (void**)&m_alphaWhiteBuffer, 0, 0);
					SelectObject(m_alphaWhiteDC, m_alphaWhiteBitmap);
				}

				HRESULT hr;
				HBRUSH fillColorBrush;

				// Render frame twice - against white and against black background to calculate alpha
				SelectClipRgn(m_alphaBlackDC, unionRgn);

				COLORREF blackColor = 0x00000000;
				fillColorBrush = CreateSolidBrush(blackColor);
				FillRgn(m_alphaBlackDC, unionRgn, fillColorBrush);
				DeleteObject(fillColorBrush);

				hr = pViewObject->Draw(DVASPECT_TRANSPARENT, 1, NULL, NULL, NULL, m_alphaBlackDC, &clipRect, &clipRect, NULL, 0);
				assert(SUCCEEDED(hr));

				// White background
				SelectClipRgn(m_alphaWhiteDC, unionRgn);

				COLORREF whiteColor = 0x00FFFFFF;
				fillColorBrush = CreateSolidBrush(whiteColor);
				FillRgn(m_alphaWhiteDC, unionRgn, fillColorBrush);
				DeleteObject(fillColorBrush);

				hr = pViewObject->Draw(DVASPECT_TRANSPARENT, 1, NULL, NULL, NULL, m_alphaWhiteDC, &clipRect, &clipRect, NULL, 0);
				assert(SUCCEEDED(hr));

				// Combine alpha
				for (LONG y = clipRgnRect.top; y < clipRgnRect.bottom; ++y)
				{
					int offset = y * m_width * 4 + clipRgnRect.left * 4;
					for (LONG x = clipRgnRect.left; x < clipRgnRect.right; ++x)
					{
						BYTE blackRed = m_alphaBlackBuffer[offset];
						BYTE whiteRed = m_alphaWhiteBuffer[offset];
						m_alphaBlackBuffer[offset + 3] = 255 - (whiteRed - blackRed);
						offset += 4;
					}
				}

				// Blit result to target DC
				BitBlt(dc, clipRgnRect.left, clipRgnRect.top,
					   clipRgnRect.right - clipRgnRect.left,
					   clipRgnRect.bottom - clipRgnRect.top,
					   m_alphaBlackDC, clipRgnRect.left, clipRgnRect.top, SRCCOPY);
			}

			DeleteObject(unionRgn);
			pViewObject->Release();
		}

		m_dirtyFlag = false;
		m_dirtyRects.clear();
		m_dirtyUnionRect.left = m_dirtyUnionRect.top = LONG_MAX;
		m_dirtyUnionRect.right = m_dirtyUnionRect.bottom = -LONG_MAX;
	}
}

//---------------------------------------------------------------------
WPARAM CFlashDXPlayer::CreateMouseWParam(WPARAM highWord)
{
	WPARAM result = highWord;
	result |= GetAsyncKeyState(VK_CONTROL) ? MK_CONTROL : 0;
	result |= GetAsyncKeyState(VK_SHIFT) ? MK_SHIFT : 0;
	result |= m_lastMouseButtons;
	return result;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetMousePos(unsigned int x, unsigned int y)
{
	LRESULT lr;
	m_windowlessObject->OnWindowMessage(WM_MOUSEMOVE, CreateMouseWParam(0), MAKELPARAM(x, y), &lr);
	m_lastMouseX = x;
	m_lastMouseY = y;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetMouseButtonState(unsigned int x, unsigned int y, EMouseButton button, bool pressed)
{
	m_lastMouseX = x;
	m_lastMouseY = y;

	LRESULT lr;
	switch (button)
	{
	case IFlashDXPlayer::eMouse1:
		if (pressed)
		{
			m_lastMouseButtons |= MK_LBUTTON;
			m_windowlessObject->OnWindowMessage(WM_LBUTTONDOWN, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		else
		{
			m_lastMouseButtons &= ~MK_LBUTTON;
			m_windowlessObject->OnWindowMessage(WM_LBUTTONUP, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		break;
	case IFlashDXPlayer::eMouse2:
		if (pressed)
		{
			m_lastMouseButtons |= MK_RBUTTON;
			m_windowlessObject->OnWindowMessage(WM_RBUTTONDOWN, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		else
		{
			m_lastMouseButtons &= ~MK_RBUTTON;
			m_windowlessObject->OnWindowMessage(WM_RBUTTONUP, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		break;
	case IFlashDXPlayer::eMouse3:
		if (pressed)
		{
			m_lastMouseButtons |= MK_MBUTTON;
			m_windowlessObject->OnWindowMessage(WM_MBUTTONDOWN, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		else
		{
			m_lastMouseButtons &= ~MK_MBUTTON;
			m_windowlessObject->OnWindowMessage(WM_MBUTTONUP, CreateMouseWParam(0), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
		}
		break;
	default:
		break;
	}
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SendMouseWheel(int delta)
{
	LRESULT lr;
	m_windowlessObject->OnWindowMessage(WM_MOUSEWHEEL, CreateMouseWParam(MAKEWPARAM(0, delta)), MAKELPARAM(m_lastMouseX, m_lastMouseY), &lr);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SendKey(bool pressed, UINT_PTR virtualKey, LONG_PTR extended)
{
	LRESULT lr;
	if (pressed)
		m_windowlessObject->OnWindowMessage(WM_KEYDOWN, (WPARAM)virtualKey, (LPARAM)extended, &lr);
	else
		m_windowlessObject->OnWindowMessage(WM_KEYUP, (WPARAM)virtualKey, (LPARAM)extended, &lr);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SendChar(UINT_PTR character, LONG_PTR extended)
{
	LRESULT lr;
	m_windowlessObject->OnWindowMessage(WM_CHAR, (WPARAM)character, (LPARAM)extended, &lr);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::EnableSound(bool enable)
{
	// NOT YET IMPLEMENTED
}

//---------------------------------------------------------------------
const wchar_t* CFlashDXPlayer::CallFunction(const wchar_t* request)
{
	BSTR response = NULL;
	HRESULT hr = m_flashInterface->raw_CallFunction(_bstr_t(request), &response);

	if (response)
	{
		m_tempStorage = response;
		return m_tempStorage.c_str();
	}

	return NULL;
}

//---------------------------------------------------------------------
void CFlashDXPlayer::SetReturnValue(const wchar_t* returnValue)
{
	m_flashInterface->SetReturnValue(returnValue);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::AddEventHandler(struct IFlashDXEventHandler* pHandler)
{
	m_eventHandlers.push_back(pHandler);
}

//---------------------------------------------------------------------
void CFlashDXPlayer::RemoveEventHandler(struct IFlashDXEventHandler* pHandler)
{
	std::vector<IFlashDXEventHandler*>::iterator it = std::find(m_eventHandlers.begin(), m_eventHandlers.end(), pHandler);
	if (it != m_eventHandlers.end())
		m_eventHandlers.erase(it);
}

//---------------------------------------------------------------------
struct IFlashDXEventHandler* CFlashDXPlayer::GetEventHandlerByIndex(unsigned int index)
{
	if (index < (unsigned int)m_eventHandlers.size())
		return m_eventHandlers[index];
	return NULL;
}

//---------------------------------------------------------------------
unsigned int CFlashDXPlayer::GetNumEventHandlers() const
{
	return (unsigned int)m_eventHandlers.size();
}

//---------------------------------------------------------------------
HRESULT CFlashDXPlayer::FlashCall(const wchar_t* request)
{
	for (unsigned int i = 0; i < m_eventHandlers.size(); ++i)
	{
		HRESULT result = m_eventHandlers[i]->FlashCall(request);
		if (result != E_NOTIMPL) return result;
	}
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT CFlashDXPlayer::FSCommand(const wchar_t* command, const wchar_t* args)
{
	for (unsigned int i = 0; i < m_eventHandlers.size(); ++i)
	{
		HRESULT result = m_eventHandlers[i]->FSCommand(command, args);
		if (result != E_NOTIMPL) return result;
	}
	return E_NOTIMPL;
}
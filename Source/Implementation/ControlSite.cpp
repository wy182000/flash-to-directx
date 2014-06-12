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
#include "ControlSite.h"
#include "FlashDXPlayer.h"

//---------------------------------------------------------------------
CControlSite::CControlSite()
{
	m_flashPlayer = NULL;
	m_refs = 0;
}

//---------------------------------------------------------------------
CControlSite::~CControlSite()
{
	assert(m_refs == 0);
}

//---------------------------------------------------------------------
void CControlSite::Init(CFlashDXPlayer* flashPlayer)
{
	m_flashPlayer = flashPlayer;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::QueryInterface(REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;

	if (riid == IID_IUnknown)
	{
		*ppv = (IUnknown*) (IOleWindow*) this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IOleWindow)
	{
		*ppv = (IOleWindow*)this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IOleInPlaceSite)
	{
		*ppv = (IOleInPlaceSite*)this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IOleInPlaceSiteEx)
	{
		*ppv = (IOleInPlaceSiteEx*)this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IOleInPlaceSiteWindowless)
	{
		*ppv = (IOleInPlaceSiteWindowless*)this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IOleClientSite)
	{
		*ppv = (IOleClientSite*)this;
		AddRef();
		return S_OK;
	}
	else if (riid == __uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents))
	{
		*ppv = (ShockwaveFlashObjects::_IShockwaveFlashEvents*) this;
		AddRef();
		return S_OK;
	}
	else
	{   
		return E_NOTIMPL;
	}
}

//---------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CControlSite::AddRef()
{
	return ++m_refs;
}

//---------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CControlSite::Release()
{
	return --m_refs;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::SaveObject()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk)
{
	*ppmk = NULL;
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetContainer(IOleContainer ** theContainerP)
{
	//return QueryInterface(__uuidof(IOleContainer), (void**) theContainerP);				
	return E_NOINTERFACE;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::ShowObject()
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnShowWindow(BOOL)
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::RequestNewObjectLayout()
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::ContextSensitiveHelp(/* [in] */ BOOL fEnterMode)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetWindow(/* [out] */ HWND __RPC_FAR* theWnndow)
{
	return E_FAIL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::CanInPlaceActivate()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceActivate()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnUIActivate()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetWindowContext(/* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
														 /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
														 /* [out] */ LPRECT lprcPosRect,
														 /* [out] */ LPRECT lprcClipRect,
														 /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	RECT rect = { 0, 0, m_flashPlayer->m_width, m_flashPlayer->m_height };

	*lprcPosRect = rect;
	*lprcClipRect = rect;

	*ppFrame = NULL;
	QueryInterface(__uuidof(IOleInPlaceFrame), (void**) ppFrame);		
	*ppDoc = NULL;

	lpFrameInfo->fMDIApp = FALSE;
	lpFrameInfo->hwndFrame = NULL;
	lpFrameInfo->haccel = NULL;
	lpFrameInfo->cAccelEntries = 0;

	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::Scroll(/* [in] */ SIZE scrollExtant)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnUIDeactivate(/* [in] */ BOOL fUndoable)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceDeactivate()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::DiscardUndoState()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::DeactivateAndUndo()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnPosRectChange(/* [in] */ LPCRECT lprcPosRect)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceActivateEx(/* [out] */ BOOL __RPC_FAR *pfNoRedraw, /* [in] */ DWORD dwFlags)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnInPlaceDeactivateEx(/* [in] */ BOOL fNoRedraw)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::RequestUIActivate()
{
	return S_FALSE;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::CanWindowlessActivate()
{
	// Allow windowless activation?
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetCapture()
{
	// TODO capture the mouse for the object
	return S_FALSE;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::SetCapture(/* [in] */ BOOL fCapture)
{
	// TODO capture the mouse for the object
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetFocus()
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::SetFocus(/* [in] */ BOOL fFocus)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::GetDC(/* [in] */ LPCRECT pRect, /* [in] */ DWORD grfFlags, /* [out] */ HDC __RPC_FAR *phDC)
{
	return E_INVALIDARG;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::ReleaseDC(/* [in] */ HDC hDC)
{
	return E_INVALIDARG;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::InvalidateRect(/* [in] */ LPCRECT pRect, /* [in] */ BOOL fErase)
{
	m_flashPlayer->AddDirtyRect(pRect);

	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::InvalidateRgn(/* [in] */ HRGN hRGN, /* [in] */ BOOL fErase)
{
	m_flashPlayer->AddDirtyRect(NULL);

	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::ScrollRect(/* [in] */ INT dx, /* [in] */ INT dy, /* [in] */ LPCRECT pRectScroll, /* [in] */ LPCRECT pRectClip)
{
	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::AdjustRect(/* [out][in] */ LPRECT prc)
{
	if (prc == NULL)
	{
		return E_INVALIDARG;
	}

	return S_OK;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CControlSite::OnDefWindowMessage(/* [in] */ UINT msg, /* [in] */ WPARAM wParam, /* [in] */ LPARAM lParam, /* [out] */ LRESULT __RPC_FAR *plResult)
{
	return S_FALSE;
}
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

#pragma once

//---------------------------------------------------------------------
class CControlSite : public IOleInPlaceSiteWindowless, public IOleClientSite	
{
protected:
	class CFlashDXPlayer*	m_flashPlayer;
	ULONG					m_refs;

public:
	//---------------------------------------------------------------------
	CControlSite();	

	//---------------------------------------------------------------------
	virtual ~CControlSite();

	//---------------------------------------------------------------------
	void Init(CFlashDXPlayer* flashPlayer);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv);

	//---------------------------------------------------------------------
	ULONG STDMETHODCALLTYPE AddRef();

	//---------------------------------------------------------------------
	ULONG STDMETHODCALLTYPE Release();

	//---------------------------------------------------------------------
	virtual HRESULT  STDMETHODCALLTYPE SaveObject();

	//---------------------------------------------------------------------
	virtual HRESULT  STDMETHODCALLTYPE GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);

	//---------------------------------------------------------------------
	virtual HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer ** theContainerP);

	//---------------------------------------------------------------------
	virtual HRESULT  STDMETHODCALLTYPE ShowObject();

	//---------------------------------------------------------------------
	virtual HRESULT  STDMETHODCALLTYPE OnShowWindow(BOOL);

	//---------------------------------------------------------------------
	virtual HRESULT  STDMETHODCALLTYPE RequestNewObjectLayout();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(/* [in] */ BOOL fEnterMode);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE GetWindow(/* [out] */ HWND __RPC_FAR* theWnndow);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE CanInPlaceActivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnInPlaceActivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnUIActivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE GetWindowContext(/* [out] */ IOleInPlaceFrame __RPC_FAR *__RPC_FAR *ppFrame,
											   /* [out] */ IOleInPlaceUIWindow __RPC_FAR *__RPC_FAR *ppDoc,
											   /* [out] */ LPRECT lprcPosRect,
											   /* [out] */ LPRECT lprcClipRect,
											   /* [out][in] */ LPOLEINPLACEFRAMEINFO lpFrameInfo);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE Scroll(/* [in] */ SIZE scrollExtant);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnUIDeactivate(/* [in] */ BOOL fUndoable);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE DiscardUndoState();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE DeactivateAndUndo();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnPosRectChange(/* [in] */ LPCRECT lprcPosRect);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnInPlaceActivateEx(/* [out] */ BOOL __RPC_FAR *pfNoRedraw, /* [in] */ DWORD dwFlags);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnInPlaceDeactivateEx(/* [in] */ BOOL fNoRedraw);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE RequestUIActivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE CanWindowlessActivate();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE GetCapture();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE SetCapture(/* [in] */ BOOL fCapture);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE GetFocus();

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE SetFocus(/* [in] */ BOOL fFocus);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE GetDC(/* [in] */ LPCRECT pRect, /* [in] */ DWORD grfFlags, /* [out] */ HDC __RPC_FAR *phDC);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE ReleaseDC(/* [in] */ HDC hDC);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE InvalidateRect(/* [in] */ LPCRECT pRect, /* [in] */ BOOL fErase);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE InvalidateRgn(/* [in] */ HRGN hRGN, /* [in] */ BOOL fErase);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE ScrollRect(/* [in] */ INT dx, /* [in] */ INT dy, /* [in] */ LPCRECT pRectScroll, /* [in] */ LPCRECT pRectClip);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE AdjustRect(/* [out][in] */ LPRECT prc);

	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE OnDefWindowMessage(/* [in] */ UINT msg, /* [in] */ WPARAM wParam, /* [in] */ LPARAM lParam, /* [out] */ LRESULT __RPC_FAR *plResult);
};
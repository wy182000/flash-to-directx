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
#include "FlashSink.h"
#include "FlashDXPlayer.h"

//---------------------------------------------------------------------
CFlashSink::CFlashSink()
{
	m_cookie = 0;
	m_connectionPoint = NULL;
	m_refs = 0;
	m_flashPlayer = NULL;
}

//---------------------------------------------------------------------
CFlashSink::~CFlashSink()
{
}

//---------------------------------------------------------------------
HRESULT CFlashSink::Init(CFlashDXPlayer* flashPlayer)
{
	m_flashPlayer = flashPlayer;

	HRESULT hr = NOERROR;
	LPCONNECTIONPOINTCONTAINER pConnectionPoint = NULL;

	if ((m_flashPlayer->m_flashInterface->QueryInterface(IID_IConnectionPointContainer, (void**) &pConnectionPoint) == S_OK) &&
		(pConnectionPoint->FindConnectionPoint(__uuidof(ShockwaveFlashObjects::_IShockwaveFlashEvents), &m_connectionPoint) == S_OK))			
	{
		IDispatch* pDispatch = NULL;
		QueryInterface(__uuidof(IDispatch), (void**) &pDispatch);
		if (pDispatch != NULL)
		{
			hr = m_connectionPoint->Advise((LPUNKNOWN)pDispatch, &m_cookie);
			pDispatch->Release();
		}
	}

	if (pConnectionPoint != NULL) 
		pConnectionPoint->Release();

	return hr;
}

//---------------------------------------------------------------------
HRESULT CFlashSink::Shutdown()
{
	HRESULT aResult = S_OK;

	if (m_connectionPoint)
	{
		if (m_cookie)
		{
			aResult = m_connectionPoint->Unadvise(m_cookie);
			m_cookie = 0;
		}

		m_connectionPoint->Release();
		m_connectionPoint = NULL;
	}

	return aResult;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CFlashSink::QueryInterface(REFIID riid, LPVOID* ppv)
{
	*ppv = NULL;

	if (riid == IID_IUnknown)
	{
		*ppv = (LPUNKNOWN)this;
		AddRef();
		return S_OK;
	}
	else if (riid == IID_IDispatch)
	{
		*ppv = (IDispatch*)this;
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
ULONG STDMETHODCALLTYPE CFlashSink::AddRef()
{
	return ++m_refs;
}

//---------------------------------------------------------------------
ULONG STDMETHODCALLTYPE CFlashSink::Release()
{
	return --m_refs;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CFlashSink::GetTypeInfoCount(UINT* pctinfo)
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CFlashSink::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CFlashSink::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId)
{
	return E_NOTIMPL;
}

//---------------------------------------------------------------------
HRESULT STDMETHODCALLTYPE CFlashSink::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, ::DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult, ::EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr)
{
	switch (dispIdMember)
	{
	case 0xc5: // FlashCall
		if (pDispParams->cArgs != 1 || pDispParams->rgvarg[0].vt != VT_BSTR) return E_INVALIDARG;
		return m_flashPlayer->FlashCall(pDispParams->rgvarg[0].bstrVal);
	case 0x96: // FSCommand
		if (pDispParams->cArgs != 2 || pDispParams->rgvarg[0].vt != VT_BSTR || pDispParams->rgvarg[1].vt != VT_BSTR) return E_INVALIDARG;
		return m_flashPlayer->FSCommand(pDispParams->rgvarg[1].bstrVal, pDispParams->rgvarg[0].bstrVal);
	case 0x7a6: // OnProgress
		return E_NOTIMPL;
	case DISPID_READYSTATECHANGE:
		return E_NOTIMPL;
	}

	return DISP_E_MEMBERNOTFOUND;
}
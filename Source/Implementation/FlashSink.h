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
class CFlashSink : public ShockwaveFlashObjects::_IShockwaveFlashEvents
{
public:
	LPCONNECTIONPOINT		m_connectionPoint;	
	DWORD					m_cookie;
	ULONG					m_refs;
	class CFlashDXPlayer*	m_flashPlayer;

public:
	//---------------------------------------------------------------------
	CFlashSink();

	//---------------------------------------------------------------------
	virtual ~CFlashSink();

	//---------------------------------------------------------------------
	HRESULT Init(CFlashDXPlayer* flashPlayer);

	//---------------------------------------------------------------------
	HRESULT Shutdown();
 
	//---------------------------------------------------------------------
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv);

	//---------------------------------------------------------------------
	ULONG STDMETHODCALLTYPE AddRef();

	//---------------------------------------------------------------------
	ULONG STDMETHODCALLTYPE Release();

	//---------------------------------------------------------------------
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* pctinfo);

	//---------------------------------------------------------------------
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);

	//---------------------------------------------------------------------
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
		UINT cNames, LCID lcid,DISPID* rgDispId);

	//---------------------------------------------------------------------
	virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
		WORD wFlags, ::DISPPARAMS __RPC_FAR *pDispParams, VARIANT __RPC_FAR *pVarResult,
		::EXCEPINFO __RPC_FAR *pExcepInfo, UINT __RPC_FAR *puArgErr);
};
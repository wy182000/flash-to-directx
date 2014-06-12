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
#include "FlashDX.h"
#include "FlashDXPlayer.h"

#pragma comment(lib, "comsuppw.lib")
using namespace ShockwaveFlashObjects;

//---------------------------------------------------------------------
CFlashDX g_instance;

//---------------------------------------------------------------------
IFlashDX* GetFlashToDirectXInstance()
{
	return &g_instance;
}

//---------------------------------------------------------------------
CFlashDX::CFlashDX()
{
	CoInitialize(NULL);
	m_flashLibHandle = LoadLibrary(L"flash10c.ocx");
}

//---------------------------------------------------------------------
CFlashDX::~CFlashDX()
{
	FreeLibrary(m_flashLibHandle);	
	CoUninitialize();
}

//---------------------------------------------------------------------
double CFlashDX::GetFlashVersion()
{
	IOleObject* pOleObject = NULL;
	if (FAILED(CoCreateInstance(CLSID_ShockwaveFlash, NULL, CLSCTX_INPROC_SERVER, IID_IOleObject, (void**) &pOleObject)))
		return 0.0;	

	IShockwaveFlash* pFlashInterface = NULL;
	if (FAILED(pOleObject->QueryInterface(__uuidof(IShockwaveFlash), (LPVOID*) &pFlashInterface)))
		return 0.0;

	long version = pFlashInterface->FlashVersion();

	pFlashInterface->Release();
	pOleObject->Release();

	return version / 65536.0;
}

//---------------------------------------------------------------------
struct IFlashDXPlayer* CFlashDX::CreatePlayer(unsigned int width, unsigned int height)
{
	CFlashDXPlayer* player = new CFlashDXPlayer(m_flashLibHandle, width, height);
	if (player->m_flashInterface == NULL)
	{
		delete player;
		return NULL;
	}
	return player;
}

//---------------------------------------------------------------------
void CFlashDX::DestroyPlayer(IFlashDXPlayer* pPlayer)
{
	delete (CFlashDXPlayer*)pPlayer;
}

//---------------------------------------------------------------------
bool CFlashDX::GetMovieProperties(const wchar_t* movie, SMovieProperties& props)
{
	return false;
}

//---------------------------------------------------------------------
bool CFlashDX::GetMovieProperties(const void* movieData, const unsigned int movieDataSize, SMovieProperties& props)
{
	return false;
}
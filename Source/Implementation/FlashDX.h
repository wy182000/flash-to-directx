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

#include "IFlashDX.h"

//---------------------------------------------------------------------
/// Implementation of IFlashDX interface.
//---------------------------------------------------------------------
class CFlashDX : public IFlashDX
{
public:
	//---------------------------------------------------------------------
	/// Constructor.
	CFlashDX();

	//---------------------------------------------------------------------
	/// Destructor.
	~CFlashDX();

	//---------------------------------------------------------------------
	// IFlashDX implementations.
	//---------------------------------------------------------------------
	virtual double GetFlashVersion();
	virtual struct IFlashDXPlayer* CreatePlayer(unsigned int width, unsigned int height);
	virtual void DestroyPlayer(IFlashDXPlayer* pPlayer);
	virtual bool GetMovieProperties(const wchar_t* movie, SMovieProperties& props);
	virtual bool GetMovieProperties(const void* movieData, const unsigned int movieDataSize, SMovieProperties& props);

protected:
	HMODULE					m_flashLibHandle;		
};
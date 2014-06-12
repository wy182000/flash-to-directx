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

// NOTE: this implementation based on Popcap Framework Flash Widget implementation

#pragma once

//---------------------------------------------------------------------
/// @brief				Returns pointer on instance of Flash-to-DirectX.
extern struct IFlashDX* GetFlashToDirectXInstance();

//---------------------------------------------------------------------
/// Flash-to-DirectX interface.
//---------------------------------------------------------------------
struct IFlashDX
{
	//---------------------------------------------------------------------
	/// @brief				Returns version of the Flash ActiveX.
	/// @return				Flash version number, eg 10.0.
	virtual double GetFlashVersion() = 0;

	//---------------------------------------------------------------------
	/// @brief				Creates flash player. Can be called multiple times to create different players.
	/// @param width		Width of rendering surface of the player.
	/// @param height		Width of rendering surface of the player.
	/// @return				Player interface.
	virtual struct IFlashDXPlayer* CreatePlayer(unsigned int width, unsigned int height) = 0;

	//---------------------------------------------------------------------
	/// @brief				Destroys player by it's pointer.
	/// @param pPlayer		Player to destroy.
	virtual void DestroyPlayer(IFlashDXPlayer* pPlayer) = 0;

	//---------------------------------------------------------------------
	/// Movie properties description.
	struct SMovieProperties
	{
		unsigned int	m_width;
		unsigned int	m_height;
		unsigned int	m_fps;
		unsigned int	m_numFrames;
	};

	//---------------------------------------------------------------------
	/// @brief				Returns movie properties by path to movie file.
	/// @param movie		Path to movie file.
	/// @param props		Returned properties in case of success.
	/// @return				Success flag.
	virtual bool GetMovieProperties(const wchar_t* movie, SMovieProperties& props) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns movie properties by movie data.
	/// @param movieData	Pointer on movie data.
	/// @param movieDataSize Movie data size.
	/// @param props		Returned properties in case of success.
	/// @return				Success flag.
	virtual bool GetMovieProperties(const void* movieData, const unsigned int movieDataSize, SMovieProperties& props) = 0;
};


//---------------------------------------------------------------------
/// Flash player interface.
//---------------------------------------------------------------------
struct IFlashDXPlayer
{
	//---------------------------------------------------------------------
	/// @brief				Attaches user data to this player.
	/// @param data			User data.
	virtual void SetUserData(intptr_t data) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns user data attached to this player.
	/// @return				intptr_t
	virtual intptr_t GetUserData() const = 0;

	//---------------------------------------------------------------------
	/// State of the player.
	enum EState
	{
		STATE_IDLE = 0,		///< no movie loaded
		STATE_PLAYING,
		STATE_STOPPED
	};

	//---------------------------------------------------------------------
	/// @brief				Returns current state of the player.
	/// @return				IFlashDXPlayer::EState
	virtual EState GetState() const = 0;

	//---------------------------------------------------------------------
	/// Quality settings for the player.
	enum EQuality
	{
		QUALITY_LOW = 0,
		QUALITY_MEDIUM,
		QUALITY_HIGH,
		QUALITY_BEST,
		QUALITY_AUTOLOW,
		QUALITY_AUTOHIGH
	};

	//---------------------------------------------------------------------
	/// @brief				Gets quality of the player's output.
	/// @return				Quality settings.
	virtual EQuality GetQuality() const = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets quality of the player's output.
	/// @param quality		Quality settings.
	virtual void SetQuality(EQuality quality) = 0;

	//---------------------------------------------------------------------
	/// Transparency mode for the player.
	///
	/// NOTE: there is known bug with transparency of the text that uses device fonts.
	/// For some reason (probably DrawText()) alpha for this pixels is set to zero, but color channels are ok.
	/// And even setting transparency mode to opaque don't fix the problem if you use A8R8G8B8 texture or equivalent.
	/// Just don't use such fonts (use anti-aliased font or set 'cacheAsBitmap' property of the object to true)
	/// if you want proper transparency. Otherwise use X8R8G8B8 texture or any equivalent with disabled alpha channel.
	enum ETransparencyMode
	{
		TMODE_OPAQUE = 0,		///< Alpha is disabled. Use texture surface format with disabled alpha (such as X8R8G8B8).
		TMODE_TRANSPARENT = 1,	///< Basic alpha, no semi-transparency.
		TMODE_FULL_ALPHA = 2,	///< Alpha fully supported. A bit slower (not much, though) than TMODE_TRANSPARENT.
	};

	//---------------------------------------------------------------------
	/// @brief				Gets transparency mode for the player.
	/// @return				Transparency mode.
	virtual ETransparencyMode GetTransparencyMode() const = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets transparency mode for the player.
	/// @param mode			Transparency mode.
	virtual void SetTransparencyMode(ETransparencyMode mode) = 0;

	//---------------------------------------------------------------------
	/// @brief				Loads and starts playing the movie.
	/// @param movie		Movie to play. You can use absolute or relative to GetCurrentDirectory() path.
	/// @return				Success flag. False means movie was not found.
	virtual bool LoadMovie(const wchar_t* movie) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns current background color.
	/// @return				Background color.
	///
	/// Background color setting works only in OPAQUE rendering mode.
	virtual COLORREF GetBackgroundColor() = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets current background color.
	/// @param color		Background color.
	///
	/// Background color setting works only in OPAQUE rendering mode.
	virtual void SetBackgroundColor(COLORREF color) = 0;

	//---------------------------------------------------------------------
	/// @brief				Starts playing of the movie.
	virtual void StartPlaying() = 0;

	//---------------------------------------------------------------------
	/// @brief				Starts playing of the movie.
	/// @param timelineTarget Time line of the movie.
	virtual void StartPlaying(const wchar_t* timelineTarget) = 0;

	//---------------------------------------------------------------------
	/// @brief				Stops playing of the movie.
	virtual void StopPlaying() = 0;

	//---------------------------------------------------------------------
	/// @brief				Stops playing of the movie.
	/// @param timelineTarget Time line of the movie.
	virtual void StopPlaying(const wchar_t* timelineTarget) = 0;

	//---------------------------------------------------------------------
	/// @brief				Rewind movie to the beginning.
	virtual void Rewind() = 0;

	//---------------------------------------------------------------------
	/// @brief				Steps movie forward for one frame and enters stopped mode.
	virtual void StepForward() = 0;

	//---------------------------------------------------------------------
	/// @brief				Steps movie backward for one frame and enters stopped mode.
	virtual void StepBack() = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns current frame.
	/// @return				Frame number.
	virtual int GetCurrentFrame() = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns current frame.
	/// @param timelineTarget Time line of the movie.
	/// @return				Frame number.
	virtual int GetCurrentFrame(const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Moves current frame marker to specified frame.
	/// @param frame		Target frame.
	virtual void GotoFrame(int frame) = 0;

	//---------------------------------------------------------------------
	/// @brief				Moves current frame marker to specified frame.
	/// @param timelineTarget Time line of the movie.
	/// @param frame		Target frame.
	virtual void GotoFrame(int frame, const wchar_t* timelineTarget) = 0;

	//---------------------------------------------------------------------
	/// @brief				Calls specified frame.
	/// @param frame		Target frame.
	/// @param timelineTarget Time line of the movie.
	virtual void CallFrame(int frame, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns current label.
	/// @param timelineTarget Time line of the movie.
	/// @return				Label name in temporarily storage. Copy it if you wish to keep it.
	virtual const wchar_t* GetCurrentLabel(const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Moves current frame marker to specified label.
	/// @param label		Target label.
	/// @param timelineTarget Time line of the movie.
	virtual void GotoLabel(const wchar_t* label, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Calls specified label.
	/// @param label		Target label.
	/// @param timelineTarget Time line of the movie.
	virtual void CallLabel(const wchar_t* label, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns contents of the variable with specified name.
	/// @param name			Name of the variable.
	/// @return				Content of the variable.
	virtual const wchar_t* GetVariable(const wchar_t* name) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets contents of the variable with specified name.
	/// @param name			Name of the variable.
	/// @param value		New value for the variable.
	virtual void SetVariable(const wchar_t* name, const wchar_t* value) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns contents of the property with specified index.
	/// @param iProperty	Property index. See Flash API for possible values. TODO: add enumeration.
	/// @param timelineTarget Time line of the movie.
	/// @return				Property value in temporarily storage. Copy it if you wish to keep it.
	virtual const wchar_t* GetProperty(int iProperty, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns contents of the property with specified index as number.
	/// @param iProperty	Property index. See Flash API for possible values. TODO: add enumeration.
	/// @param timelineTarget Time line of the movie.
	/// @return				Property value.
	virtual double GetPropertyAsNumber(int iProperty, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets property with specified index.
	/// @param iProperty	Property index. See Flash API for possible values. TODO: add enumeration.
	/// @param value		New value for the property.
	/// @param timelineTarget Time line of the movie.
	virtual void SetProperty(int iProperty, const wchar_t* value, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets property with specified index.
	/// @param iProperty	Property index. See Flash API for possible values. TODO: add enumeration.
	/// @param value		New value for the property.
	/// @param timelineTarget Time line of the movie.
	virtual void SetProperty(int iProperty, double value, const wchar_t* timelineTarget = L"/") = 0;

	//---------------------------------------------------------------------
	/// @brief				Resizes player.
	/// @param newWidth		New width for the player.
	/// @param newHeight	New height for the player.
	virtual void ResizePlayer(unsigned int newWidth, unsigned int newHeight) = 0;

	//---------------------------------------------------------------------
	/// @brief				Checks if player wants update target surface.
	/// @param unionDirtyRect Pointer on pointer that will receive pointer on united dirty rect. Can be NULL.
	/// @param dirtyRects	Pointer on pointer that will receive address of the dirty rectangles array. Can be NULL.
	/// @param numDirtyRects Pointer on variable that will receive size of the dirty rectangles array. Can be NULL.
	/// @return				Dirty flag.
	virtual bool IsNeedUpdate(const RECT** unitedDirtyRect = NULL, const RECT** dirtyRects = NULL, unsigned int* numDirtyRects = NULL) = 0;

	//---------------------------------------------------------------------
	/// @brief				Draws flash frame to provided DC.
	/// @param dc			Target DC.
	///
	/// To use this method for updating DirectX texture use IDirect3DSurface9::GetDC() method.
	virtual void DrawFrame(HDC dc) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets mouse cursor position for the movie.
	/// @param x			Target mouse X coordinate.
	/// @param y			Target mouse Y coordinate.
	///
	/// It doesn't affect system cursor position.
	virtual void SetMousePos(unsigned int x, unsigned int y) = 0;

	//---------------------------------------------------------------------
	/// Mouse buttons enumeration.
	enum EMouseButton
	{
		eMouse1 = 0,	// left mouse button
		eMouse2,		// right mouse button
		eMouse3,		// middle (wheel) mouse button
		eMouse4,
		eMouse5,
	};

	//---------------------------------------------------------------------
	/// @brief				Sets mouse button state.
	/// @param x			Target mouse X coordinate.
	/// @param y			Target mouse Y coordinate.
	/// @param button		Button to update.
	/// @param pressed		Button state (is pressed).
	virtual void SetMouseButtonState(unsigned int x, unsigned int y, EMouseButton button, bool pressed) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sends mouse wheel delta to flash control.
	/// @param delta		Mouse wheel delta to send.
	virtual void SendMouseWheel(int delta) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sends key state.
	/// @param pressed		Key state (is pressed).
	/// @param virtualKey	Virtual key (see WM_KEYUP/WM_KEYDOWN event).
	/// @param extended		Extended data (see WM_KEYUP/WM_KEYDOWN event).
	/// @return				void
	virtual void SendKey(bool pressed, UINT_PTR virtualKey, LONG_PTR extended) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sends character to flash control.
	/// @param character	Character to send.
	/// @param extended		Extended data (see WM_CHAR event).
	virtual void SendChar(UINT_PTR character, LONG_PTR extended) = 0;

	//---------------------------------------------------------------------
	/// @brief				Enables/disables sound for flash control.
	/// @param enable		New status.
	virtual void EnableSound(bool enable) = 0;

	//---------------------------------------------------------------------
	/// @brief				Calls Action Script function.
	/// @param request		Function call XML.
	///
	/// Please use provided ASInterface helper to ease the task of calling and handling Flash events.
	///
	/// or
	///
	/// See "The external API's XML format".
	/// http://livedocs.adobe.com/flash/9.0/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00000344.html
	virtual const wchar_t* CallFunction(const wchar_t* request) = 0;

	//---------------------------------------------------------------------
	/// @brief				Sets return value for Action Script call.
	/// @param returnValue	Return value XML.
	///
	/// Please use provided ASInterface helper to ease the task of calling and handling Flash events.
	///
	/// or
	///
	/// See "The external API's XML format".
	/// http://livedocs.adobe.com/flash/9.0/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00000344.html
	/// Should be called only from IFlashDXEventHandler::FlashCall().
	virtual void SetReturnValue(const wchar_t* returnValue) = 0;

	//---------------------------------------------------------------------
	/// @brief				Adds event handler.
	/// @param pHandler		Handler interface.
	virtual void AddEventHandler(struct IFlashDXEventHandler* pHandler) = 0;

	//---------------------------------------------------------------------
	/// @brief				Removes event handler.
	/// @param pHandler		Handler interface.
	virtual void RemoveEventHandler(struct IFlashDXEventHandler* pHandler) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns event handler by index.
	/// @param index		Index of event handler.
	/// @return				Handler interface.
	virtual struct IFlashDXEventHandler* GetEventHandlerByIndex(unsigned int index) = 0;

	//---------------------------------------------------------------------
	/// @brief				Returns number of registered event handlers.
	/// @return				Number of event handlers.
	virtual unsigned int GetNumEventHandlers() const = 0;
};


//---------------------------------------------------------------------
/// Flash event handler interface.
//---------------------------------------------------------------------
struct IFlashDXEventHandler
{
	//---------------------------------------------------------------------
	/// @brief				Called when Flash Action Script uses ExternalInterface.call().
	/// @param request		Call XML data.
	/// @return				Should be NOERROR if call was processed successfully or E_NOTIMPL if request is not
	///						recognized. Or any other valid COM error.
	///
	/// Please use provided ASInterface helper to ease the task of calling and handling Flash events.
	///
	/// or
	///
	/// See "The external API's XML format".
	/// http://livedocs.adobe.com/flash/9.0/main/wwhelp/wwhimpl/common/html/wwhelp.htm?context=LiveDocs_Parts&file=00000344.html
	virtual HRESULT FlashCall(const wchar_t* request) = 0;

	//---------------------------------------------------------------------
	/// @brief				Called when Flash Action Script uses fscommand().
	/// @param command		Command string.
	/// @param args			Arguments string.
	/// @return				Should be NOERROR if call was processed successfully or E_NOTIMPL if request is not
	///						recognized. Or any other valid COM error.
	///
	/// Please use provided ASInterface helper to ease the task of calling and handling Flash events.
	virtual HRESULT FSCommand(const wchar_t* command, const wchar_t* args) = 0;
};

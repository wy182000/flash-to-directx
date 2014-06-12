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

#include <string>
#include <vector>
#include <map>

//---------------------------------------------------------------------
/// Action Script value. Used to exchange data with Action Script.
//---------------------------------------------------------------------
struct ASValue
{
	//---------------------------------------------------------------------
	/// @brief				Constructor.
	inline ASValue();

	//---------------------------------------------------------------------
	/// @brief				Copy constructor.
	/// @param value		Initializing value.
	inline ASValue(const ASValue &value);

	//---------------------------------------------------------------------
	/// @brief				Initializing constructor template.
	/// @param value		Initializing value.
	template<typename _Type> inline ASValue(const _Type &value);

	//---------------------------------------------------------------------
	/// @brief				Assignment operator template.
	/// @param value		Assignment value.
	/// @return				Reference on this value.
	template<typename _Type> inline ASValue& operator = (const _Type &value);

	//---------------------------------------------------------------------
	/// @brief				Cast operator template.
	/// @return				Casted value.
	template<typename _Type> inline operator _Type() const;

	//---------------------------------------------------------------------
	/// @brief				Destructor.
	inline ~ASValue();

	//---------------------------------------------------------------------
	/// @brief				Checks if value is empty.
	/// @return				Empty flag.
	inline bool IsEmpty() const;

	//---------------------------------------------------------------------
	/// @brief				Checks if value is boolean.
	/// @return				Boolean flag.
	inline bool IsBoolean() const;

	//---------------------------------------------------------------------
	/// @brief				Checks if value is number.
	/// @return				Number flag.
	inline bool IsNumber() const;
	//---------------------------------------------------------------------
	/// @brief				Checks if value is string.
	/// @return				String flag.
	inline bool IsString() const;

	//---------------------------------------------------------------------
	/// @brief				Checks if value is array.
	/// @return				Array flag.
	inline bool IsArray() const;

	//---------------------------------------------------------------------
	/// @brief				Checks if value is object.
	/// @return				Object flag.
	inline bool IsObject() const;

	//---------------------------------------------------------------------
	/// @brief				Converts value to Action Script XML representation.
	/// @return				Result string.
	inline std::wstring ToXML() const;

	//---------------------------------------------------------------------
	/// @brief				Converts Action Script XML representation to value.
	/// @param xml			XML string.
	inline void FromXML(const std::wstring &xml);

	//---------------------------------------------------------------------
	// AS data types
	//---------------------------------------------------------------------
	typedef bool Boolean;
	typedef float Number;
	typedef std::wstring String;
	typedef std::vector<ASValue> Array;
	typedef std::map<std::wstring, ASValue>	Object;

private:
	struct _Data; _Data &m_data;
};

//---------------------------------------------------------------------
/// Action Script interface.
/// Allows to call AS function and register callbacks called from AS.
//---------------------------------------------------------------------
struct ASInterface
{
	//---------------------------------------------------------------------
	/// @brief				Constructor.
	/// @param pPlayer		Player to use with this helper instance.
	inline ASInterface(IFlashDXPlayer *pPlayer);

	//---------------------------------------------------------------------
	/// @brief				Destructor.
	inline ~ASInterface();

	//---------------------------------------------------------------------
	/// @brief				Calls and Action Script function with up to ten arguments.
	/// @param functionName	Function name.
	/// @param arg0-9		Optional function arguments.
	/// @return				Function call result.
	inline ASValue Call(const std::wstring &functionName,
						const ASValue &arg0 = ASValue(), const ASValue &arg1 = ASValue(),
						const ASValue &arg2 = ASValue(), const ASValue &arg3 = ASValue(),
						const ASValue &arg4 = ASValue(), const ASValue &arg5 = ASValue(),
						const ASValue &arg6 = ASValue(), const ASValue &arg7 = ASValue(),
						const ASValue &arg8 = ASValue(), const ASValue &arg9 = ASValue());

	//---------------------------------------------------------------------
	/// @brief				Registers a function as an Action Script callback.
	/// @param functionName	Function name.
	/// @param function		Function to register.
	template<typename _Function>
	inline void AddCallback(const std::wstring &functionName, _Function function);

	//---------------------------------------------------------------------
	/// @brief				Registers a function as an Action Script callback.
	/// @param functionName	Function name.
	/// @param object		Object instance which owns the method.
	/// @param method		Method to register.
	template<typename _Object, typename _Method>
	inline void AddCallback(const std::wstring &functionName, _Object &object, _Method method);

	//---------------------------------------------------------------------
	/// @brief				Registers a function as a fscommand() callback.
	/// @param command		Command to watch.
	/// @param function		Function to register.
	inline void AddFSCCallback(const std::wstring &command, void (*function)(const wchar_t* args));

	//---------------------------------------------------------------------
	/// @brief				Registers a method as a fscommand() callback.
	/// @param command		Command to watch.
	/// @param object		Object instance which owns the method.
	/// @param method		Method to register.
	template<typename _Object>
	inline void AddFSCCallback(const std::wstring &command, _Object &object, void (_Object::*method)(const wchar_t* args));

	//---------------------------------------------------------------------
	/// @brief				Registers a function as the default fscommand() callback.
	/// @param function		Function to register.
	///
	/// Function will be called on any fscommand() that was not processed by dedicated FSC callback.
	inline void SetDefaultFSCCallback(void (*function)(const wchar_t* command, const wchar_t* args));

	//---------------------------------------------------------------------
	/// @brief				Registers a method as the default fscommand() callback.
	/// @param object		Object instance which owns the method.
	/// @param method		Method to register.
	///
	/// Function will be called on any fscommand() that was not processed by dedicated FSC callback.
	template<typename _Object>
	inline void SetDefaultFSCCallback(_Object &object, void (_Object::*method)(const wchar_t* command, const wchar_t* args));

private:
	struct _Data; _Data &m_data;
};

#include "ASInterface.inl"
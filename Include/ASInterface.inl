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

#include <sstream>
#include <limits>
#include <float.h>

//---------------------------------------------------------------------
// ASValue::_Data
//---------------------------------------------------------------------
struct ASValue::_Data
{
	int type;
	void* data;

	inline _Data()
	:
		type(-1), data(0)
	{}
	inline ~_Data()
	{
		Destruct();
	}

	inline void Construct(int newtype, const void* value)
	{
		Destruct();
		switch (newtype)
		{
		case 0: data = new Boolean(*reinterpret_cast<const Boolean*>(value)); break;
		case 1: data = new Number(*reinterpret_cast<const Number*>(value)); break;
		case 2: data = new String(*reinterpret_cast<const String*>(value)); break;
		case 3: data = new Array(*reinterpret_cast<const Array*>(value)); break;
		case 4: data = new Object(*reinterpret_cast<const Object*>(value)); break;
		}
		type = newtype;
	}
	inline void Destruct()
	{
		switch (type)
		{
		case 0: delete reinterpret_cast<Boolean*>(data); break;
		case 1: delete reinterpret_cast<Number*>(data); break;
		case 2: delete reinterpret_cast<String*>(data); break;
		case 3: delete reinterpret_cast<Array*>(data); break;
		case 4: delete reinterpret_cast<Object*>(data); break;
		}
		type = -1;
	}

	inline _Data& operator = (Boolean value)
	{
		Construct(0, &value);
		return *this;
	}
	inline _Data& operator = (Number value)
	{
		Construct(1, &value);
		return *this;
	}
	inline _Data& operator = (double value)
	{
		return *this = (Number)value;
	}
	inline _Data& operator = (int value)
	{
		return *this = (Number)value;
	}
	inline _Data& operator = (unsigned int value)
	{
		return *this = (Number)value;
	}
	inline _Data& operator = (const String &value)
	{
		Construct(2, &value);
		return *this;
	}
	inline _Data& operator = (const std::string &value)
	{
		std::wstringstream s; s << value.c_str();
		return *this = s.str();
	}
	inline _Data& operator = (const wchar_t* value)
	{
		std::wstringstream s; s << value;
		return *this = s.str();
	}
	inline _Data& operator = (const char* value)
	{
		std::wstringstream s; s << value;
		return *this = s.str();
	}
	inline _Data& operator = (const Array &value)
	{
		Construct(3, &value);
		return *this;
	}
	inline _Data& operator = (const Object &value)
	{
		Construct(4, &value);
		return *this;
	}
	inline _Data& operator = (const ASValue &value)
	{
		Construct(value.m_data.type, value.m_data.data);
		return *this;
	}

	inline operator Boolean() const
	{
		Boolean result;
		if (type == -1) result = false;
		else
		if (type == 0) result = *(Boolean*)data;
		else
		if (type == 1) result = (*(Number*)data != 0) && !_isnan(*(Number*)data);
		else
		if (type == 2) result = (*(String*)data).empty();
		else result = true;
		return result;
	}
	inline operator Number() const
	{
		Number result;
		if (type == -1) result = std::numeric_limits<float>::quiet_NaN();
		else
		if (type == 0) result = (*(Boolean*)data) ? 1.f : 0.f;
		else
		if (type == 1) result = *(Number*)data;
		else
		if (type == 2)
		{
			const String &str = *(String*)data;
			if (swscanf_s(str.c_str(), L"%f", &result) != 1)
			{
				result = std::numeric_limits<float>::quiet_NaN();
			}
		}
		else result = 0;
		return result;
	}
	inline operator int() const
	{
		Number result = Number(*this);
		return _isnan(result) ? 0 : (int)result;
	}
	inline operator unsigned int() const
	{
		Number result = Number(*this);
		return _isnan(result) ? 0 : (unsigned int)result;
	}
	inline operator String() const
	{
		std::wstringstream s;
		if (type == -1) s << L"null";
		else
		if (type == 0) s << *(Boolean*)data;
		else
		if (type == 1) s << *(Number*)data;
		else
		if (type == 3)
		{
			Array &a = *(Array*)data;
			for (size_t i = 0, e = a.size(); i < e; ++i)
			{
				if (i > 0) s << L",";
				s << (String)a[i];
			}
		}
		else
		if (type == 4) s << L"[object Object]";
		return s.str();
	}
	inline operator const wchar_t*() const
	{
		assert(type == 3);
		return (*(String*)data).c_str();
	}
	inline operator Array() const
	{
		assert(type == 3);
		if (type == 3) return *(Array*)data;
		return Array();
	}
	inline operator Object() const
	{
		assert(type == 4);
		if (type == 4) return *(Object*)data;
		return Object();
	}
};


//---------------------------------------------------------------------
// ASValue
//---------------------------------------------------------------------
inline ASValue::ASValue()
:
	m_data(* new _Data)
{}
inline ASValue::ASValue(const ASValue &value)
:
	m_data(* new _Data)
{
	m_data = value;
}
template<typename _Type>
inline ASValue::ASValue(const _Type &value)
:
	m_data(* new _Data)
{
	m_data = value;
}
template<typename _Type>
inline ASValue& ASValue::operator = (const _Type &value)
{
	m_data = value;
	return *this;
}
template<typename _Type>
inline ASValue::operator _Type() const
{
	return m_data;
}
inline ASValue::~ASValue()
{
	delete &m_data;
}

inline bool ASValue::IsEmpty() const
{
	return m_data.type == -1;
}
inline bool ASValue::IsBoolean() const
{
	return m_data.type == 0;
}
inline bool ASValue::IsNumber() const
{
	return m_data.type == 1;
}
inline bool ASValue::IsString() const
{
	return m_data.type == 2;
}
inline bool ASValue::IsArray() const
{
	return m_data.type == 3;
}
inline bool ASValue::IsObject() const
{
	return m_data.type == 4;
}

inline std::wstring ASValue::ToXML() const
{
	struct _Array { static std::wstring ToXML(const Array &a)
	{
		std::wstringstream s;
		s << "<array>";
		for (size_t i = 0, e = (unsigned int)a.size(); i < e; ++i)
		{
			s << L"<property id='" << i << L"'>" << a[i].ToXML() << L"</property>";
		}
		s << "</array>";
		return s.str();
	}};

	struct _Object { static std::wstring ToXML(const Object &o)
	{
		std::wstringstream s;
		s << "<object>";
		for (Object::const_iterator i = o.begin(), e = o.end(); i != e; ++i)
		{
			s << "<property id='" << i->first << "'>" << i->second.ToXML() << "</property>";
		}
		s << "</object>";
		return s.str();
	}};

	std::wstringstream s;

	switch (m_data.type)
	{
	case 0: s << (*(Boolean*)m_data.data ? L"<true/>" : L"<false/>"); break;
	case 1: s << L"<number>" << *(Number*)m_data.data << L"</number>"; break;
	case 2: s << L"<string>" << *(String*)m_data.data << L"</string>"; break;
	case 3: s << _Array::ToXML(*(Array*)m_data.data); break;
	case 4: s << _Object::ToXML(*(Object*)m_data.data); break;
	default: s << L"<null/>"; break;
	}

	return s.str();
}
inline void ASValue::FromXML(const std::wstring &xml)
{
	struct _Props { static void parseFirst(const std::wstring &xml, std::wstring &name, std::wstring &content, size_t &length)
	{
		std::wstring propOpen = xml.substr(0, xml.find(L">") + 1);
		name = propOpen.substr(14, propOpen.length() - 14 - 2);
		const wchar_t* propClose = xml.c_str() + propOpen.length();
		int nesting = 0;
		while (propClose - xml.c_str() < (int)xml.length())
		{
			if (wcsncmp(propClose, L"<property id=", 13) == 0) ++nesting;
			else
			if (wcsncmp(propClose, L"</property>", 10) == 0)
			{
				if (nesting != 0) --nesting;
				else break;
			}
			++propClose;
		}
		content = xml.substr(propOpen.length(), propClose - xml.c_str() - propOpen.length());
		length = propOpen.length() + content.length() + 11;
	}};

	if (xml.find(L"<true/>") == 0) m_data = true;
	else
	if (xml.find(L"<false/>") == 0) m_data = false;
	else
	if (xml.find(L"<number>") == 0)
	{
		std::wstringstream s(xml.substr(8));
		Number value; s >> value;
		m_data = value;
	}
	else
	if (xml.find(L"<string>") == 0)
	{
		String value = xml.substr(8, xml.rfind(L"</string>") - 8);
		m_data = value;
	}
	else
	if (xml.find(L"<array>") == 0)
	{
		Array value;
		std::wstring propsXML = xml.substr(7, xml.rfind(L"</array>") - 7);
		while (!propsXML.empty())
		{
			std::wstring propName, propContent; size_t propLength;
			_Props::parseFirst(propsXML, propName, propContent, propLength);
			size_t propIndex; swscanf_s(propName.c_str(), L"%d", &propIndex);
			ASValue prop; prop.FromXML(propContent);
			if (propIndex >= value.size()) value.resize(propIndex + 1);
			value[propIndex] = prop;
			propsXML = propsXML.substr(propLength);
		}
		m_data = value;
	}
	else
	if (xml.find(L"<object>") == 0)
	{
		Object value;
		std::wstring propsXML = xml.substr(8, xml.rfind(L"</object>") - 8);
		while (!propsXML.empty())
		{
			std::wstring propName, propContent; size_t propLength;
			_Props::parseFirst(propsXML, propName, propContent, propLength);
			ASValue prop; prop.FromXML(propContent);
			value[propName] = prop;
			propsXML = propsXML.substr(propLength);
		}
		m_data = value;
	}
}


//---------------------------------------------------------------------
// ASInterface::_Data
//---------------------------------------------------------------------
struct ASInterface::_Data : IFlashDXEventHandler
{
	struct Callback
	{
		template<typename _Type> struct CallHlp;

		//
		template<typename _R>
		struct CallHlp<_R (*)()> {
			typedef _R (*_F)();
			static const size_t argNum = 0;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f();
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f();
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0>
		struct CallHlp<_R (*)(_A0)> {
			typedef _R (*_F)(_A0);
			static const size_t argNum = 1;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1>
		struct CallHlp<_R (*)(_A0, _A1)> {
			typedef _R (*_F)(_A0, _A1);
			static const size_t argNum = 2;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2>
		struct CallHlp<_R (*)(_A0, _A1, _A2)> {
			typedef _R (*_F)(_A0, _A1, _A2);
			static const size_t argNum = 3;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3);
			static const size_t argNum = 4;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4);
			static const size_t argNum = 5;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4, _A5)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4, _A5);
			static const size_t argNum = 6;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4], a[5]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4], a[5]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4, _A5, _A6);
			static const size_t argNum = 7;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7);
			static const size_t argNum = 8;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7, typename _A8>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8);
			static const size_t argNum = 9;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};
		template<typename _R, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7, typename _A8, typename _A9>
		struct CallHlp<_R (*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8, _A9)> {
			typedef _R (*_F)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8, _A9);
			static const size_t argNum = 10;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _F f, const ASValue::Array &a) {
				f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
			}};
			static inline void Call(ASValue &r, _F f, const ASValue::Array &a) { Ret<_R>::Call(r, f, a); }
		};

		//
		template<typename _R, typename _O>
		struct CallHlp<_R (_O::*)()> {
			typedef _R (_O::*_M)();
			static const size_t argNum = 0;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)();
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)();
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0>
		struct CallHlp<_R (_O::*)(_A0)> {
			typedef _R (_O::*_M)(_A0);
			static const size_t argNum = 1;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1>
		struct CallHlp<_R (_O::*)(_A0, _A1)> {
			typedef _R (_O::*_M)(_A0, _A1);
			static const size_t argNum = 2;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2);
			static const size_t argNum = 3;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3);
			static const size_t argNum = 4;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4);
			static const size_t argNum = 5;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4, _A5)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4, _A5);
			static const size_t argNum = 6;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4], a[5]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4], a[5]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4, _A5, _A6);
			static const size_t argNum = 7;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7);
			static const size_t argNum = 8;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7, typename _A8>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8);
			static const size_t argNum = 9;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};
		template<typename _R, typename _O, typename _A0, typename _A1, typename _A2, typename _A3, typename _A4, typename _A5, typename _A6, typename _A7, typename _A8, typename _A9>
		struct CallHlp<_R (_O::*)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8, _A9)> {
			typedef _R (_O::*_M)(_A0, _A1, _A2, _A3, _A4, _A5, _A6, _A7, _A8, _A9);
			static const size_t argNum = 10;
			template<typename _R> struct Ret { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				r = (o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
			}};
			template<> struct Ret<void> { static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) {
				(o.*m)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], a[9]);
			}};
			static inline void Call(ASValue &r, _O &o, _M m, const ASValue::Array &a) { Ret<_R>::Call(r, o, m, a); }
		};

		struct BaseCaller
		{
			virtual size_t ArgNum() = 0;
			virtual void Call(const ASValue::Array &arguments, ASValue &returnValue) = 0;
			virtual BaseCaller& Clone() = 0;
		};

		template<typename _F> struct Caller : BaseCaller
		{
			_F fn; inline Caller(_F _fn) : fn(_fn) {}
			size_t ArgNum() { return CallHlp<_F>::argNum; }
			void Call(const ASValue::Array &a, ASValue &r) { CallHlp<_F>::Call(r, fn, a); }
			Caller& Clone() { return * new Caller(fn); }
		};
		template<typename _R, typename _O> struct Caller<_R _O::*> : BaseCaller
		{
			typedef _R _O::*_M; _O &ob; _M mt; inline Caller(_O &_ob, _M _mt) : ob(_ob), mt(_mt) {}
			size_t ArgNum() { return CallHlp<_M>::argNum; }
			void Call(const ASValue::Array &a, ASValue &r) { CallHlp<_M>::Call(r, ob, mt, a); }
			Caller& Clone() { return * new Caller(ob, mt); }
		};

		BaseCaller &caller;

		inline Callback()
		:
			caller(*(BaseCaller*)0)
		{}
		template<typename _Function> inline Callback(_Function function)
		:
			caller(* new Caller<_Function>(function))
		{}
		template<typename _Object, typename _Method> inline Callback(_Object &object, _Method method)
		:
			caller(* new Caller<_Method>(object, method))
		{}
		inline Callback(const Callback &value)
		:
			caller(&value.caller ? value.caller.Clone() : *(BaseCaller*)0)
		{}
		inline ~Callback()
		{
			delete &caller;
		}
		inline Callback& operator = (const Callback &value)
		{
			this->~Callback(); new (this) Callback(value);
			return *this;
		}

		HRESULT Call(const ASValue::Array &arguments, ASValue &returnValue) const
		{
			if (&caller == 0) return E_NOTIMPL;
			if (arguments.size() < caller.ArgNum()) return E_INVALIDARG;

			caller.Call(arguments, returnValue);

			return NOERROR;
		}
	};

	IFlashDXPlayer &player;
	typedef std::map<std::wstring, Callback> Callbacks;
	Callbacks callbacks;
	Callbacks fsCallbacks;
	Callback fsDefCallback;

	inline _Data(IFlashDXPlayer *pPlayer)
	:
		player(*pPlayer)
	{
		player.AddEventHandler(this);
	}
	inline ~_Data()
	{
		player.RemoveEventHandler(this);
	}
	HRESULT FlashCall(const wchar_t* request)
	{
		struct _Args { static void split(const std::wstring &xml, std::vector<std::wstring> &args)
		{
			const wchar_t* argStart = xml.c_str();
			const wchar_t* argEnd = xml.c_str();
			int nesting = 0;
			bool closetag = false;
			while (argEnd - xml.c_str() < (int)xml.length())
			{
				if (wcsncmp(argEnd, L"</arguments>", 12) == 0) break;
				else
				if (wcsncmp(argEnd, L"<arguments>", 11) == 0)
				{
					nesting = 0;
					argStart += 11;
					argEnd += 10;
				}
				else
				if (wcsncmp(argEnd, L"</", 2) == 0) closetag = true;
				else
				if (wcsncmp(argEnd, L"<", 1) == 0) ++nesting;
				else
				if (wcsncmp(argEnd, L"/>", 2) == 0) closetag = true;
				else
				if (wcsncmp(argEnd, L">", 1) == 0)
				{
					if (closetag)
					{
						closetag = false;
						if (--nesting == 0)
						{
							size_t start = argStart - xml.c_str();
							size_t size = argEnd - argStart + 1;
							args.push_back(xml.substr(start, size));
							argStart = argEnd + 1;
						}
					}
				}
				++argEnd;
			}
		}};

		std::wstring xml = request;
		std::wstring functionName = xml.substr(14, xml.find(L">") - 14 - 18);
		Callbacks::iterator itCallback = callbacks.find(functionName);
		if (itCallback != callbacks.end())
		{
			std::vector<std::wstring> args;
			_Args::split(xml.substr(xml.find(L">") + 1), args);
			ASValue::Array arguments;
			for (size_t i = 0, s = args.size(); i < s; ++i)
			{
				ASValue arg; arg.FromXML(args[i]);
				arguments.push_back(arg);
			}
			ASValue returnValue;
			HRESULT result = itCallback->second.Call(arguments, returnValue);
			if (result == NOERROR && !returnValue.IsEmpty()) player.SetReturnValue(returnValue.ToXML().c_str());
			return result;
		}

		return E_NOTIMPL;
	}
	HRESULT FSCommand(const wchar_t* command, const wchar_t* args)
	{
		Callbacks::iterator itCallback = fsCallbacks.find(command);
		if (itCallback != fsCallbacks.end())
		{
			ASValue::Array arguments;
			arguments.push_back(args);
			ASValue returnValue;
			return itCallback->second.Call(arguments, returnValue);
		}
		ASValue::Array arguments;
		arguments.push_back(command);
		arguments.push_back(args);
		ASValue returnValue;
		return fsDefCallback.Call(arguments, returnValue);
	}
};

//---------------------------------------------------------------------
// ASInterface
//---------------------------------------------------------------------
inline ASInterface::ASInterface(IFlashDXPlayer *pPlayer)
:
	m_data(* new _Data(pPlayer))
{}

inline ASInterface::~ASInterface()
{
	delete &m_data;
}

inline ASValue ASInterface::Call(const std::wstring &functionName, const ASValue &arg0, const ASValue &arg1, const ASValue &arg2, const ASValue &arg3, const ASValue &arg4, const ASValue &arg5, const ASValue &arg6, const ASValue &arg7, const ASValue &arg8, const ASValue &arg9)
{
	struct _Args { static void ToXML(std::wstring &arguments, const ASValue &arg0 = ASValue(), const ASValue &arg1 = ASValue(), const ASValue &arg2 = ASValue(), const ASValue &arg3 = ASValue(), const ASValue &arg4 = ASValue(), const ASValue &arg5 = ASValue(), const ASValue &arg6 = ASValue(), const ASValue &arg7 = ASValue(), const ASValue &arg8 = ASValue(), const ASValue &arg9 = ASValue())
	{
		if (arg0.IsEmpty()) return;

		arguments += arg0.ToXML();

		ToXML(arguments, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	}};

	std::wstring arguments(L"<arguments>");
	_Args::ToXML(arguments, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
	if (arguments.length() > 11) arguments += std::wstring(L"</arguments>");
	else arguments = L"";

	std::wstring request = L"<invoke name='" + functionName + L"' returntype='xml'>" + arguments + L"</invoke>";
	const wchar_t* result = m_data.player.CallFunction(request.c_str());

	if (result == NULL) return ASValue();

	ASValue value; value.FromXML(result);

	return value;
}

template<typename _Function>
inline void ASInterface::AddCallback(const std::wstring &functionName, _Function function)
{
	m_data.callbacks[functionName] = _Data::Callback(function);
}
template<typename _Object, typename _Method>
inline void ASInterface::AddCallback(const std::wstring &functionName, _Object &object, _Method method)
{
	m_data.callbacks[functionName] = _Data::Callback(object, method);
}
inline void ASInterface::AddFSCCallback(const std::wstring &command, void (*function)(const wchar_t* args))
{
	m_data.fsCallbacks[command] = _Data::Callback(function);
}
template<typename _Object>
inline void ASInterface::AddFSCCallback(const std::wstring &command, _Object &object, void (_Object::*method)(const wchar_t* args))
{
	m_data.fsCallbacks[command] = _Data::Callback(object, method);
}
inline void ASInterface::SetDefaultFSCCallback(void (*function)(const wchar_t* command, const wchar_t* args))
{
	m_data.fsDefCallback = _Data::Callback(function);
}
template<typename _Object>
inline void ASInterface::SetDefaultFSCCallback(_Object &object, void (_Object::*method)(const wchar_t* command, const wchar_t* args))
{
	m_data.fsDefCallback = _Data::Callback(object, method);
}

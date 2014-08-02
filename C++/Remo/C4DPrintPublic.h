// ------------------------------------------------------------------------------------------------
// C4DPrintPublic.h
// Printing For C4D
// Copyright (c) 2005-2014 Remotion(Igor Schulz)  http://www.remotion4d.net
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, 
// including commercial applications, and to alter it and redistribute it freely, 
// subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. 
// If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// ------------------------------------------------------------------------------------------------
/// Printing For C4D like this:
/// print("test-print",23, true,false,NULL ,1.2,5.6f,Vector(1),Filename("fname"),BaseTime(55), Matrix()  );
// ------------------------------------------------------------------------------------------------
// print(" variadig"," print ","test ",1,2,3,4,5,6,7,8,9," h"," b"," v",5.5,6.8,7.8,9.0,10.21,NULL,
// "\n test "," resat ",9,8,7,6,5,4,3,2,1,0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.9,
// "\n",Vector(),Matrix(),op,(BaseObject*)NULL);
// ------------------------------------------------------------------------------------------------
#pragma once
#ifndef _REMO_C4D_PRINT_H 
#define _REMO_C4D_PRINT_H

#include "c4d_general.h"
#include "c4d_baseobject.h"
#include "c4d_graphview_enum.h"
#include "c4d_raytrace.h"
#include "c4d_tools.h"
#include "c4d_basetag.h"
#include "c4d_string.h"

//#include "un_legacy.h"

#ifndef __LEGACY_API
inline String LongToString(Int32 l)		{ return C4DOS.St->LongToString(l); }
inline String LLongToString(Int64 l)	{ return C4DOS.St->LLongToString(l); }
inline String RealToString(Float v, Int32 vvk=-1, Int32 nnk=-1, Bool e=false, UInt16 xchar='0') { return C4DOS.St->RealToString(v,vvk,nnk,e,xchar); }
inline String PtrToString(const void *hex)	{ return String::HexToString((UInt)hex); }
inline String MemoryToString(Int64 mem)		{ return String::MemoryToString(mem); }
#endif

// convert to c4d String
inline String to_c4d_string(const String &val) 	{ return val; }
inline String to_c4d_string(const char *str) 	{ return String(str); }

inline String to_c4d_string(void* hex)			{ return String::HexToString((UInt)hex); }
inline String to_c4d_string(void** hex)			{ return String::HexToString((UInt)(const void*)hex); }

inline String to_c4d_string(const bool val) 	{ return (val)?"true":"false"; }

inline String to_c4d_string(const Char val) 	{ return LongToString(val); }
inline String to_c4d_string(const UChar val) 	{ return LongToString(val); }

inline String to_c4d_string(const Int16 val) 	{ return LongToString(val); }
inline String to_c4d_string(const UInt16 val) 	{ return LongToString(val); }

inline String to_c4d_string(const Int32 val) 	{ return LongToString(val); }
inline String to_c4d_string(const UInt32 val) 	{ return LLongToString(val); }

inline String to_c4d_string(const Int64 val)	{ return LLongToString(val); }
inline String to_c4d_string(const UInt64 val)	{ return LLongToString(val); }

inline String to_c4d_string(const Float32 val) 	{ return RealToString(val/*,-1,6*/); }
inline String to_c4d_string(const Float64 val) 	{ return RealToString(val/*,-1,9*/); }

inline String to_c4d_string(const Vector32 &val)	{ return "("+RealToString(val.x/*,-1,6*/)+", "+RealToString(val.y/*,-1,6*/)+", "+RealToString(val.z/*,-1,6*/)+")"; }
inline String to_c4d_string(const Vector64 &val)	{ return "("+RealToString(val.x/*,-1,9*/)+", "+RealToString(val.y/*,-1,9*/)+", "+RealToString(val.z/*,-1,9*/)+")"; }

inline String to_c4d_string(const Matrix32 &val)	{ return "\n off"+to_c4d_string(val.off)+" \n v1"+to_c4d_string(val.v1)+" \n v2"+to_c4d_string(val.v2)+" \n v3"+to_c4d_string(val.v3)+"\n"; }
inline String to_c4d_string(const Matrix64 &val)	{ return "\n off"+to_c4d_string(val.off)+" \n v1"+to_c4d_string(val.v1)+" \n v2"+to_c4d_string(val.v2)+" \n v3"+to_c4d_string(val.v3)+"\n"; }

inline String to_c4d_string(const UVWStruct &val)	{ return to_c4d_string(val.a)+" \n"+to_c4d_string(val.b)+" \n"+to_c4d_string(val.c)+" \n"+to_c4d_string(val.d); }
inline String to_c4d_string(const CPolygon &val)	{ return "["+to_c4d_string(val.a)+" "+to_c4d_string(val.b)+" "+to_c4d_string(val.c)+" "+to_c4d_string(val.d)+"]"; }

inline String to_c4d_string(const BaseObject *op) 	{ return (op==NULL)?"null":op->GetName(); }
inline String to_c4d_string(const Filename &fname) 	{ return fname.GetString(); }

inline String to_c4d_string(const BaseTime &time) 	{ return to_c4d_string(time.Get())+"("+to_c4d_string(time.GetNumerator())+"/"+to_c4d_string(time.GetDenominator())+")"; }

inline String to_c4d_string(const GeData &data) {
	switch( data.GetType() ) {
	case DA_NIL : return String("DA_NIL"); break;
	case DA_LONG: return to_c4d_string( data.GetInt32() ); break;
	case DA_REAL: return to_c4d_string( data.GetFloat() ); break;
	case DA_TIME: return to_c4d_string( data.GetTime() ); break;
	case DA_VECTOR: return to_c4d_string( data.GetVector() ); break;
	case DA_MATRIX: return to_c4d_string( data.GetMatrix() ); break;
	case DA_BYTEARRAY: return String("DA_BYTEARRAY");break;
	case DA_STRING: return to_c4d_string( data.GetString() ); break;
	case DA_FILENAME: return to_c4d_string( data.GetFilename() ); break;
	case DA_CONTAINER: return String("DA_CONTAINER");break;
	case DA_ALIASLINK: return String("DA_ALIASLINK");break;
	case DA_MARKER: return String("DA_MARKER");break;
	case DA_MISSINGPLUG: return String("DA_MISSINGPLUG");break;
#ifdef _LIB_DESCRIPTION_H_
	case CUSTOMDATATYPE_DESCID:
		{
			DescID *res = (DescID*)data.GetCustomDataType(CUSTOMDATATYPE_DESCID);
			if(res)  return  String("DescID ")+to_c4d_string( (*res)[0].id ); 
			else return  String("CUSTOMDATATYPE_DESCID ");  
		}break;
#endif
	default: return String("Unknown "+LongToString(data.GetType()));
	}
}

//------------------------------------------------------------------------------
inline String to_c4d_string(const BaseContainer &bc) { //Remo: 15.01.2012
	if(bc==NULL) return "BaseContainer nullptr";
	String result = "BaseContainer {\n";
	Int32 index = 0; //number
	Int32 bid = 0; //ID
	do {
		bid = bc.GetIndexId(index); if (bid==NOTOK){ break; }  
		const GeData *gd = bc.GetIndexData(index);
		if(gd) {
			result += "   Id "+ ::to_c4d_string(bid) + ":  " + ::to_c4d_string(*gd) + " \n";
		}else {	//should not be called
			//GetData() does not work if date has the same id
			result += "   Id "+ ::to_c4d_string(bid) + ":  " + ::to_c4d_string(bc.GetData(bid)) + " \n"; 
		}
		++index;
	}while(bid != NOTOK); 
	result += "}\n";
	return result;
}
inline String to_c4d_string(const BaseContainer *bc) {//Remo: 21.03.2011
	if(bc==NULL) return "BaseContainer nullptr";
	return to_c4d_string(*bc);
}

// ------------------------------------------------------------------------------
void GePrintNoCR(const String &str);//from C4D SDK  c4d_general.cpp

#ifdef _HAS_VARIADIC_TEMPLATES //__ICL //only if Compiler support Variadic templates >>>
// ------------------------------------------------------------------------------
template<typename T>
inline void print(const T &val)
{
	GePrint(to_c4d_string(val) );
}
// ------------------------------------------------------------------------------
template<typename First,typename ... Rest>
inline void print(const First &first,const Rest& ... rest)
{
	GePrintNoCR(to_c4d_string(first)+" "); 
	print(rest...); 
}
#else //_HAS_VARIADIC_TEMPLATES
// ------------------------------------------------------------------------------
// Printing for up to 11 parameters.
// One could use Variadic templates here once all used compilers have support for them. 
template<typename T>
inline void print(const T &val) { GePrint(to_c4d_string(val) ); }

template<typename T1, typename T2>
inline void print(const T1 &v1, const T2 &v2) { GePrint(to_c4d_string(v1) +" "+  to_c4d_string(v2) ); }

template<typename T1, typename T2, typename T3>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3) { GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3) ); }

template<typename T1, typename T2, typename T3, typename T4>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7)+" "+to_c4d_string(v8) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8, const T9 &v9) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7)+" "+to_c4d_string(v8)+" "+to_c4d_string(v9) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8, const T9 &v9, const T10 &v10) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7)+" "+to_c4d_string(v8)+" "+to_c4d_string(v9)+" "+to_c4d_string(v10) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8, const T9 &v9, const T10 &v10, const T11 &v11) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7)+" "+to_c4d_string(v8)+" "+to_c4d_string(v9)+" "+to_c4d_string(v10)+" "+to_c4d_string(v11) ); }

template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
inline void print(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8, const T9 &v9, const T10 &v10, const T11 &v11, const T12 &v12) 
{ GePrint(to_c4d_string(v1) +" "+to_c4d_string(v2) +" "+to_c4d_string(v3)+" "+to_c4d_string(v4)+" "+to_c4d_string(v5)+" "+to_c4d_string(v6)+" "+to_c4d_string(v7)+" "+to_c4d_string(v8)+" "+to_c4d_string(v9)+" "+to_c4d_string(v10)+" "+to_c4d_string(v11)+" "+to_c4d_string(v12) ); }

#endif //_HAS_VARIADIC_TEMPLATES


#endif//_REMO_C4D_PRINT_H

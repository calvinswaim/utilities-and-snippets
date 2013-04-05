// ------------------------------------------------------------------------------------------------
// C4DPrintPublic.h
// Printing For C4D
// Copyright (c) 2005-2011 Remotion(Igor Schulz)  http://www.remotion4d.net
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

#ifndef _REMO_C4D_PRINT_H 
#define _REMO_C4D_PRINT_H

#include "c4d_general.h"
#include "c4d_baseobject.h"
#include "c4d_graphview_enum.h"
#include "c4d_raytrace.h"
#include "c4d_tools.h"
#include "c4d_basetag.h"
#include "c4d_string.h"

// convert to c4d String
inline String to_c4d_string(const String &val)   { return val; }
inline String to_c4d_string(const char *str) 	{ return String(str); }

inline String to_c4d_string(const bool val) 	{ return (val)?"true":"false"; }

inline String to_c4d_string(const CHAR val) 	{ return LongToString(val); }
inline String to_c4d_string(const UCHAR val) 	{ return LongToString(val); }

inline String to_c4d_string(const SWORD val) 	{ return LongToString(val); }
inline String to_c4d_string(const UWORD val) 	{ return LongToString(val); }

inline String to_c4d_string(const LONG val) 	{ return LongToString(val); }
inline String to_c4d_string(const ULONG val) 	{ return LLongToString(val); }

#if !defined(__GNUC__) // on Mac OS X INT is the same as LONG
inline String to_c4d_string(const INT val) 		{ return LongToString(val); }
inline String to_c4d_string(const UINT val) 	{ return LLongToString(val); }
#endif

inline String to_c4d_string(const SReal val) 	{ return RealToString(val,-1,6); }
inline String to_c4d_string(const LReal val) 	{ return RealToString(val,-1,9); }

inline String to_c4d_string(const LLONG val)	{ return LLongToString(val); }
inline String to_c4d_string(const LULONG val)	{ return LLongToString(val); }

inline String to_c4d_string(const SVector &val)		{ return "("+RealToString(val.x,-1,6)+", "+RealToString(val.y,-1,6)+", "+RealToString(val.z,-1,6)+")"; }
inline String to_c4d_string(const LVector &val)		{ return "("+RealToString(val.x,-1,9)+", "+RealToString(val.y,-1,9)+", "+RealToString(val.z,-1,9)+")"; }

inline String to_c4d_string(const SMatrix &val)		{ return "\n off"+to_c4d_string(val.off)+" \n v1"+to_c4d_string(val.v1)+" \n v2"+to_c4d_string(val.v2)+" \n v3"+to_c4d_string(val.v3)+"\n"; }
inline String to_c4d_string(const LMatrix &val)		{ return "\n off"+to_c4d_string(val.off)+" \n v1"+to_c4d_string(val.v1)+" \n v2"+to_c4d_string(val.v2)+" \n v3"+to_c4d_string(val.v3)+"\n"; }

inline String to_c4d_string(const UVWStruct &val)	{ return to_c4d_string(val.a)+" \n"+to_c4d_string(val.b)+" \n"+to_c4d_string(val.c)+" \n"+to_c4d_string(val.d); }
inline String to_c4d_string(const CPolygon &val)	{ return "["+to_c4d_string(val.a)+" "+to_c4d_string(val.b)+" "+to_c4d_string(val.c)+" "+to_c4d_string(val.d)+"]"; }

inline String to_c4d_string(const BaseObject *op) 	{ return (op==NULL)?"null":op->GetName(); }
inline String to_c4d_string(const Filename &fname) 	{ return fname.GetString(); }

inline String to_c4d_string(const BaseTime &time) 	{ return to_c4d_string(time.Get()); }


// ------------------------------------------------------------------------------
void GePrintNoCR(const String &str);//from C4D SDK  c4d_general.cpp

#ifdef __ICL //only if Compiler support Variadic templates >>>
// ------------------------------------------------------------------------------
template<typename T>
void print(const T &val)
{
	GePrint(to_c4d_string(val) );
}
// ------------------------------------------------------------------------------
template<typename First,typename ... Rest>
void print(const First &first,const Rest& ... rest)
{
	GePrintNoCR(to_c4d_string(first)+" "); 
	print(rest...); 
}
#else
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

#endif


#endif//_REMO_C4D_PRINT_H

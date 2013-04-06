// ------------------------------------------------------------------------------------------------
// AutoPointerPublic.h
// Printing For C4D
// Copyright (c) 2008-2013 Remotion(Igor Schulz)  http://www.remotion4d.net
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
// Some additional smart (not very smart :) pointers. 
// ------------------------------------------------------------------------------------------------
#pragma once
#ifndef _RE_AUTO_POINTER_H_
#define _RE_AUTO_POINTER_H_


//=====================================================================================================================
template <class TYPE, class FunctorT>
class AutoDo
{
  TYPE *ptr;

	FunctorT f;
private:
	TYPE *operator = (TYPE *p);
public:
	AutoDo(TYPE *p, FunctorT &f_) : f(f_)	{ ptr = p; }
	~AutoDo()							{ Free(); }
	void Free()							{ f(ptr); }
	operator TYPE* () const				{ return  ptr; }
	operator TYPE& () const				{ return *ptr; }
	TYPE *operator -> () const			{ return  ptr; }
	TYPE *const *operator & () const	{ return &ptr; }
	TYPE *Release()						{ TYPE *tmp=ptr; ptr=NULL; return tmp; }
	void Assign(TYPE *p)				{ ptr=p; }
	TYPE* Get()							{ return ptr; }
};

//=====================================================================================================================
template <class TYPE> 
class AutoDelete 
{
	TYPE *ptr;
private: //delete >>>
	TYPE *operator = (TYPE *p);
	TYPE *operator = (const TYPE *p);

	AutoDelete(const AutoDelete&);	// not defined
	AutoDelete& operator=(const AutoDelete&);	// not defined
public:
	AutoDelete()						{ ptr = nullptr; }
	AutoDelete(AutoDelete&& other)		{ Free(); ptr = other.ptr;  other.ptr = nullptr; } // move
	explicit AutoDelete(TYPE *p)		{ ptr = p; }
	~AutoDelete()						{ Free(); }

	AutoDelete& operator=(const AutoDelete&& other) { if (this != &other){ Reset(other.Release()); } return (*this); } // assign by moving

	template<class ITYPE>
	void Create()						{ Reset( new ITYPE() ); }

	template<class ITYPE, typename ... RestT>
	void Create(RestT&& ...rest)		{ Reset( new ITYPE(rest...) ); }

	void Free()							{ if(ptr) delete ptr;  ptr = nullptr; }

	operator TYPE* () const				{ return  ptr; }
	operator TYPE& () const				{ return *ptr; }

	TYPE *operator -> () const			{ return  ptr; }
	TYPE *const *operator & () const	{ return &ptr; }

	// returns a pointer to the managed object and releases the ownership 
	TYPE *Release()						{ TYPE *tmp = ptr; ptr = nullptr; return tmp; }

	// replaces the managed object,  use Reset(); as Free();   and Reset(new_ptr);  as Free(); Assign(new_ptr);
	void Reset(TYPE *p = nullptr)		{ if(ptr != p){ Free(); ptr=p; } }

	// returns a pointer to the managed object 
	TYPE* Get()							{ return ptr; }

	template<class ITYPE>
	ITYPE* Get()						{ return static_cast<ITYPE*>(ptr); }

	//! dangerous >>>
	void Assign(TYPE *p)				{ ptr=p; } 
};

//=====================================================================================================================
template <class TYPE> 
class AutoDeleteArray
{
	TYPE *ptr;
private:
	TYPE *operator = (TYPE *p);
public:
	AutoDeleteArray(TYPE *p)			{ ptr = p; }
	~AutoDeleteArray()					{ Free(); }
	void Free()							{ if(ptr) delete[] ptr;  ptr = NULL; /*gDelete(ptr);*/  }
	operator TYPE* () const				{ return  ptr; }
	operator TYPE& () const				{ return *ptr; }
	TYPE *operator -> () const			{ return  ptr; }
	TYPE *const *operator & () const	{ return &ptr; }
	TYPE *Release()						{ TYPE *tmp=ptr; ptr=NULL; return tmp; }
	void Assign(TYPE *p)				{ ptr=p; }
	TYPE* Get()							{ return ptr; }
};

//=====================================================================================================================
template <class TYPE> 
class AutoRemoveFree
{
	TYPE *ptr;
private:
	TYPE *operator = (TYPE *p);
public:
	AutoRemoveFree()					{ ptr = NULL; }
	AutoRemoveFree(TYPE* p)				{ ptr = p; }
	~AutoRemoveFree()					{ if(ptr){ ptr->Remove(); TYPE::Free(ptr); } ptr = NULL; }
	void Free()							{ if(ptr){ ptr->Remove(); TYPE::Free(ptr); } ptr=NULL; }
	void Set(TYPE* p)					{ ptr = p; }
	operator TYPE* () const				{ return  ptr; }
	operator TYPE& () const				{ return *ptr; }
	TYPE *operator -> () const			{ return  ptr; }
	TYPE *const *operator & () const	{ return &ptr; }
	TYPE *Release()						{ TYPE *tmp=ptr; ptr=NULL; return tmp; }
	void Assign(TYPE *p)				{ ptr=p; }
};

#endif

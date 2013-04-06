#pragma once
// Remo: 09.09.2012
//
// GeColliderHelper.h
// GeColliderHelper For C4D
// Copyright (c) 2012 Remotion(Igor Schulz)  http://www.remotion4d.net
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, 
// including commercial applications, and to alter it and redistribute it freely, 
// subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#include "lib_collider.h"
#include "C4DPrintPublic.h"

//==============================================================================
class GeColliderHelper
//==============================================================================
{
protected:
  AutoAlloc<GeColliderEngine> m_colle;
	AutoAlloc<GeColliderCache> m_cache[2];

	bool FillColliderCache(GeColliderCache& c, BaseObject& obj);
public:
	GeColliderHelper() {}
	GeColliderHelper(BaseObject *obj1, BaseObject *obj2) { SetObj1(obj1); SetObj2(obj2); }

	bool SetObj1(BaseObject *obj1);
	bool SetObj2(BaseObject *obj2);

	/// Collide object-1 and object-2.
	//input: mg1 and mg2 are global matrices of object-1 and object-2.
	//output: poly_id1 - polygon id of object-1.
	//output: poly_id2 - polygon id of object-2.
	bool Collide(const Matrix& mg1, const Matrix& mg2, LONG &poly_id1, LONG &poly_id2);


	/// Calculate distance between object-1 and object-2.
	//input: mg1 and mg2 are global matrices of object-1 and object-2.
	//output:  dist- distance between object-1 and object-2.
	//closestPoint1 and closestPoint2 closest points on object-1 and object-2.
	bool CalcDistance(const Matrix& mg1, const Matrix& mg2, Real &dist, Vector &closestPoint1, Vector &closestPoint2);

};
// ----------------------------------------------------------------------------------------------------
bool GeColliderHelper::SetObj1(BaseObject *obj1)
{
	if(obj1==nullptr) return false;
	return FillColliderCache(*m_cache[0],*obj1);
}
// ----------------------------------------------------------------------------------------------------
bool GeColliderHelper::SetObj2(BaseObject *obj2)
{
	if(obj2==nullptr) return false;
	return FillColliderCache(*m_cache[1],*obj2);
}
// ----------------------------------------------------------------------------------------------------
inline bool GeColliderHelper::Collide( const Matrix& mg1, const Matrix& mg2, LONG &poly_id1, LONG &poly_id2 )
{
	//const LONG res = m_colle->DoPolyPairs(mg1,m_cache[0],mg2,m_cache[1],0.01);
	const LONG res = m_colle->DoCollide(mg1,m_cache[0],mg2,m_cache[1],COL_FIRST_CONTACT);
	if(res != COL_OK) return false;

	const LONG pcnt = m_colle->GetNumPairs();
	if(pcnt>0){
		LONG id = 0;
		poly_id1 = m_colle->GetId1(id);
		poly_id2 = m_colle->GetId2(id);
	}
	return true;
}
// ----------------------------------------------------------------------------------------------------
inline bool GeColliderHelper::CalcDistance( const Matrix& mg1, const Matrix& mg2, Real &dist, Vector &closestPoint1, Vector &closestPoint2)
{
	const LONG res = m_colle->DoDistance(mg1,m_cache[0],mg2,m_cache[1],0.01,0.01);
	if(res != COL_OK) return false;

	dist = m_colle->GetDistance();
	closestPoint1  = m_colle->GetP1();
	closestPoint2  = m_colle->GetP2();
	return true;
}
// ----------------------------------------------------------------------------------------------------
inline bool GeColliderHelper::FillColliderCache(GeColliderCache& c, BaseObject& obj)
{
	// Get polygon object
	ModelingCommandData md1; md1.op = &obj; md1.doc = obj.GetDocument();
	if (!SendModelingCommand(MCOMMAND_CURRENTSTATETOOBJECT, md1)) return FALSE;

	// Triangulate it
	ModelingCommandData md2; md2.op = static_cast<BaseObject*>(md1.result->GetIndex(0)); 
	if (!SendModelingCommand(MCOMMAND_TRIANGULATE, md2)) return FALSE;
	AutoAlloc<PolygonObject> poly(static_cast<PolygonObject*>(md2.op)); if (!poly) return FALSE;

	// Get the polygon data
	const CPolygon* tris = poly->GetPolygonR();
	const Vector* points = poly->GetPointR();

	// Fill the cache
	if (c.BeginInput(poly->GetPolygonCount()) != COL_OK) return FALSE;
	for (int i = 0; i < poly->GetPolygonCount(); ++i) {
		if (c.AddTriangle(points[tris[i].a], points[tris[i].b], points[tris[i].c], i) != COL_OK) return FALSE;
	}
	if (c.EndInput() != COL_OK) return FALSE;

	return TRUE;
}


#if 1
// ----------------------------------------------------------------------------------------------------
inline bool GeColliderHelperTest(BaseDocument *doc)
{
	BaseObject* obj1 = doc->GetFirstObject(); if (!obj1) return false;
	BaseObject* obj2 = obj1->GetNext();		  if (!obj2) return false;

	GeColliderHelper ch(obj1,obj2);

	Real dist(MAXREALl);
	Vector closestPoint1;
	Vector closestPoint2;
	if( ch.CalcDistance(obj1->GetMg(), obj2->GetMg(), dist, closestPoint1,closestPoint2)  ){
		print("Distance ",obj1,obj2,dist,closestPoint1,closestPoint2);
	}

	LONG poly_id1(-1);
	LONG poly_id2(-1);
	if( ch.Collide(obj1->GetMg(), obj2->GetMg(), poly_id1,poly_id2)  ){
		print("Collide ",obj1,obj2,poly_id1,poly_id2);
	}

	return true;
}
#endif

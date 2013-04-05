// ------------------------------------------------------------------------------------------------
// AutoPointerPublic.h
// Printing For C4D
// Copyright (c) 2003 - 2012 Remotion(Igor Schulz)  http://www.remotion4d.net
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

#ifndef _SAMPLER_REMO_H_
#define _SAMPLER_REMO_H_
//=================================================================================================
//  Remotion (Igor Schulz) 2003 - 2012
//	BaseMaterial/Shader Sampler for C4D
//	First Created:: 17.12.03
//=================================================================================================
#include "c4d_raytrace.h"
//=================================================================================================
class Sampler
//=================================================================================================
{
public:
	 Sampler();
	~Sampler();
 
	LONG Init(BaseMaterial *mat ,LONG chnr,Real time,BaseDocument *doc,BaseObject *op=NULL);
	LONG Init(TextureTag *textag,LONG chnr,Real time,BaseDocument *doc,BaseObject *op=NULL);
 
	inline Bool IsInit(){ return TexInit; };
	Vector	SampleUV(const Vector &uv, Real time=0.0);
	Vector	Sample3D(const Vector &p = 0.0, const Vector &uv = 0.0, Real time=0.0);
 
	void Free();
private:
	BaseShader			*texShader;
	TexData				*tex;
	RayObject			*rop;
	InitRenderStruct	*irs;
	ChannelData			cd;
	Matrix		omg;
	Real		offsetX;
	Real		offsetY;
	Real		lenX;
	Real		lenY;
	Bool		TexInit;
};
//-------------------------------------------------------------------------------------------------
inline Vector Sampler::SampleUV(const Vector &uv, Real time)
{
	cd.t		= time;
	cd.p		= uv;
	cd.p.x		= (uv.x-offsetX) / lenX; 
	cd.p.y		= (uv.y-offsetY) / lenY;
	return texShader->Sample(&cd);	
}
//-------------------------------------------------------------------------------------------------
inline Vector Sampler::Sample3D(const Vector &p, const Vector &uv, Real time)
{
	cd.t			= time;
	cd.vd->p		= p;
	cd.vd->back_p	= p;
	cd.p			= uv;
	return texShader->Sample(&cd);
}
//-------------------------------------------------------------------------------------------------
inline LONG Sampler::Init(TextureTag *textag,LONG chnr,Real time,BaseDocument *doc,BaseObject *op)
{
	if(!textag) return 1;
	BaseMaterial *mat = textag->GetMaterial();if(!mat) return 2;//no Material
	return Init(mat,chnr,time,doc,op);
}
//-------------------------------------------------------------------------------------------------
LONG Sampler::Init(BaseMaterial *mat,LONG chnr, Real time,BaseDocument *doc, BaseObject *op)
{
	if (!doc || !mat || chnr < 0 || chnr > 12)	return 1;
	if (!cd.vd	 || !tex || !irs  || !rop)		return 2;
	TexInit = FALSE;
	LONG			err = 0;
	BaseContainer	chandata;
	LONG			fps		= doc->GetFps();
	Filename		dpath	= doc->GetDocumentPath();
	BaseChannel *texChan1   = mat->GetChannel(chnr);	if (!texChan1) return 3;//no Channel
	texShader = texChan1->GetShader(); if(!texShader) return 4;
	if (op){ //Object
		rop->link		= op;
		rop->mg			= op->GetMg();
		rop->mp			= op->GetMp()*op->GetMg();
		rop->rad		= op->GetRad();
		if (op->IsInstanceOf(Opolygon)){
			rop->pcnt	= ToPoly(op)->GetPointCount();
			rop->padr	= GetPointW(ToPoly(op));
			rop->vcnt	= ToPoly(op)->GetPolygonCount();
			rop->vadr	= (RayPolygon*)GetPolygonW(ToPoly(op));
			rop->type   = O_POLYGON;
		}
	}
	//----------------- TexData -------------------
	tex->mp			= mat;// Set Material
	//------------ InitRenderStruct ----------------
	irs->fps		= fps;
	irs->time		= BaseTime(time);
	irs->doc		= doc;
	irs->docpath	= dpath;
	irs->vd			= cd.vd;
	//---------------- Sampling ---------------------
	chandata	= texChan1->GetData();
	cd.off		= chandata.GetReal(BASECHANNEL_BLUR_OFFSET,0.0);
	cd.scale	= 0.0;
	cd.d		= 0.0;
	cd.n		= Vector(0.0,1.0,0.0);
	cd.texflag	= TEX_TILE;
	if (texShader->InitRender(*irs)==LOAD_OK) { TexInit = TRUE; return 0;} 
	else err = 5;
Error:
	texShader->FreeRender();
	return err;
}
//-------------------------------------------------------------------------------------------------
Sampler::Sampler()
{
	texShader		= NULL;
	TexInit			= FALSE;
	offsetX			= 1.0;
	offsetY			= 1.0;
	lenX			= 1.0;
	lenY			= 1.0;
	//--------------- VolumeData -----------------
	cd.vd =	 VolumeData::Alloc(); if (!cd.vd) {VolumeData::Free(cd.vd);  return;}
	cd.vd->version		= 1;  //LONG
	cd.vd->fps			= 25; //LONG
	cd.vd->ambient		= Vector(0.5);//0.0;
	cd.vd->time			= 0;  //Real
	cd.vd->bumpn		= Vector(0.0,1.0,0.0);
	cd.vd->back_delta	= Vector(0.0);
	cd.vd->global_mip	= 1.0; //Real
	cd.vd->orign		= Vector(0.0,1.0,0.0);
	cd.vd->n			= Vector(0.0,1.0,0.0);
	cd.vd->dispn		= Vector(0.0,1.0,0.0);
	cd.vd->delta		= Vector(0.0);
	cd.vd->dist			= 0.0; //Real
	cd.vd->lhit			= RayHitID();
	cd.vd->cosc			= 1.0; //Real Angle
	cd.vd->ddu			= Vector(1.0,0.0,0.0);
	cd.vd->ddv			= Vector(0.0,1.0,0.0);
	cd.vd->tray			= NULL; //TRay
	cd.vd->rray			= NULL; //RRay
	cd.vd->ray			= NULL; //Ray
	cd.vd->xlight		= NULL; //RayLight
	//---------------- TexData ------------------
	tex	= TexData::Alloc(); if (!tex) {VolumeData::Free(cd.vd);TexData::Free(tex);  return;}
	tex->Init();	
	tex->proj		= P_UVW;
	tex->texflag	= TEX_TILE;
	tex->lenx		= lenX;
	tex->leny		= lenY;
	tex->ox			= offsetX;
	tex->oy			= offsetY;
	cd.vd->tex		= tex;
	//---------------- RayObject -----------------
	rop = AllocRayObject(0); if (!rop) return; 
	rop->link		  = NULL;
	rop->texture_link = NULL;
	rop->visibility	= 1.0;
	rop->pcnt		= 0;
	rop->padr		= NULL;
	rop->vcnt		= 0;
	rop->vadr		= NULL;
	rop->texcnt		= 0;
	rop->texadr		= NULL;
	rop->uvwadr		= NULL;
	rop->rsadr		= NULL;
	cd.vd->op		= rop;
	//---------------- RenderStruct -----------------
	irs = gNew(InitRenderStruct);
}
//-------------------------------------------------------------------------------------------------
void Sampler::Free()
{
	if (texShader){  if (TexInit) texShader->FreeRender();  }
	TexInit = FALSE;
}
//-------------------------------------------------------------------------------------------------
Sampler::~Sampler(void)
{
	Free();
	if (tex) TexData::Free(tex);
	if (cd.vd)  VolumeData::Free(cd.vd);
	FreeRayObject(rop);
	gDelete(irs);
}
#endif//_SAMPLER_REMO_H_

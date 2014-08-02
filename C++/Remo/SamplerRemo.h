// ------------------------------------------------------------------------------------------------
// SamplerRemo.h
// ShaderSmapler For C4D
// Copyright (c) 2003 - 2014 Remotion(Igor Schulz)  http://www.remotion4d.net
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
#pragma once
#ifndef _SAMPLER_REMO_H_
#define _SAMPLER_REMO_H_
//=================================================================================================
//  Remotion (Igor Schulz) 2003 - 2013
//	BaseMaterial/Shader Sampler for C4D
//	Created:: 17.12.03 
//  Please note this code was created 10 years ago it could be improved a lot.
//=================================================================================================
//   BaseDocument *doc = GetActiveDocument();
//   BaseObject *obj = doc->GetActiveObject();  if (!obj) return FALSE;
//   BaseMaterial *mat = doc->GetFirstMaterial(); if (!mat) return FALSE;  //The material to be sampled
//   Sampler smpl;
//   const Int32 init_res = smpl.Init(mat,CHANNEL_COLOR, 0.0, doc); if(init_res != 0) return FALSE;
//   Vector pos3d; //3d coordinates for 3D shader.
//   Vector uv; //UVW coordinates for all other shaders.
//   ... //set here pos3d and uv !!!
//   const Vector color = smpl.Sample3D(pos3d, uv);                      
//=================================================================================================
#include "c4d_raytrace.h"
#include "ge_sys_math.h"

enum INIT_SAMPLER_RESULT
{
	INIT_SAMPLER_RESULT_OK = 0,
	INIT_SAMPLER_RESULT_WRONG_PARAM = -1,
	INIT_SAMPLER_RESULT_NO_MATERIAL = -2,
	INIT_SAMPLER_RESULT_NO_CHANNEL  = -2,
	INIT_SAMPLER_RESULT_NO_SHADER   = -3,
	INIT_SAMPLER_RESULT_NO_INITRENDER  = -5,

} ENUM_END_LIST(INIT_SAMPLER_RESULT);

//=================================================================================================
class Sampler
//=================================================================================================
{
public:
	 Sampler();
	~Sampler();
 
	//init this sampler, always call one of this before everything else.
	INIT_SAMPLER_RESULT Init(BaseObject *obj,Int32 chnr=CHANNEL_COLOR,Float time=0.0);
	INIT_SAMPLER_RESULT Init(BaseMaterial *mat ,Int32 chnr,Float time,BaseDocument *doc,BaseObject *op=nullptr);
	INIT_SAMPLER_RESULT Init(TextureTag *textag,Int32 chnr,Float time,BaseDocument *doc,BaseObject *op=nullptr);

	//return true if Init was called before.
	inline Bool IsInit(){ return TexInit; };
	
	//return color at UVW coordinates.
	Vector	SampleUV(const Vector &uv, Float time=0.0);
	
	//return color at 3D coordinates p, if shader is not 3D then uv will be used.
	Vector	Sample3D(const Vector &pos3d, const Vector &uv = Vector(0.0), Float time=0.0);
 
	//return average color of the shader
	Vector AverageColor(Int32 num_samples = 128);
 
	void Free();
private:
	BaseShader			 *texShader; //NON owning ptr
	AutoAlloc<VolumeData> vd;  //Owning ptr
	AutoAlloc<TexData>	  tex; //Owning ptr
	RayObject			 *rop; //Owning ptr

	InitRenderStruct	irs;
	ChannelData			cd;

	Matrix			omg;
	Float			offsetX;
	Float			offsetY;
	Float			lenX;
	Float			lenY;
	Bool			TexInit;
};
//-------------------------------------------------------------------------------------------------
inline Vector Sampler::SampleUV(const Vector &uv, Float time)
{
	cd.t		= time;
	cd.p		= uv;
	cd.p.x		= (uv.x-offsetX) / lenX; 
	cd.p.y		= (uv.y-offsetY) / lenY;
	return texShader->Sample(&cd);	
}
//-------------------------------------------------------------------------------------------------
inline Vector Sampler::Sample3D(const Vector &p, const Vector &uv, Float time)
{
	cd.t			= time;
	cd.vd->p		= p;
	cd.vd->back_p	= p;
	cd.p			= uv;
	return texShader->Sample(&cd);
}
//-------------------------------------------------------------------------------------------------
inline INIT_SAMPLER_RESULT Sampler::Init(BaseObject *obj,Int32 chnr,Float time)
{
	if(!obj) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseDocument *doc = obj->GetDocument(); if (!doc) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	TextureTag* textag = (TextureTag*)obj->GetTag(Ttexture);  if (!textag) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseMaterial *mat = textag->GetMaterial(); 	if(!mat) return INIT_SAMPLER_RESULT_NO_MATERIAL;//no Material
	return Init(mat,chnr,time,doc,obj);
}
//-------------------------------------------------------------------------------------------------
inline INIT_SAMPLER_RESULT Sampler::Init(TextureTag *textag,Int32 chnr,Float time,BaseDocument *doc,BaseObject *op)
{
	if(!textag) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseMaterial *mat = textag->GetMaterial(); 	if(!mat) return INIT_SAMPLER_RESULT_NO_MATERIAL;//no Material
	return Init(mat,chnr,time,doc,op);
}
//-------------------------------------------------------------------------------------------------
inline INIT_SAMPLER_RESULT Sampler::Init(BaseMaterial *mat,Int32 chnr, Float time,BaseDocument *doc, BaseObject *op)
{
	if (!mat || !doc || chnr < 0 || chnr > 12)	return INIT_SAMPLER_RESULT_WRONG_PARAM;
	if (!cd.vd	|| !tex || !rop) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	TexInit = FALSE;
	INIT_SAMPLER_RESULT	 err = INIT_SAMPLER_RESULT_OK;
	BaseContainer	chandata;
	Int32			fps		= doc->GetFps();
	Filename		dpath	= doc->GetDocumentPath();
	BaseChannel	*texChan1   = mat->GetChannel(chnr); if (!texChan1) return INIT_SAMPLER_RESULT_NO_CHANNEL;//no Channel
	texShader = texChan1->GetShader(); 	if(!texShader) return INIT_SAMPLER_RESULT_NO_SHADER;
	if (op){ //Object
		rop->link		= op;
		rop->mg			= op->GetMg();
		rop->mp			= op->GetMg() * op->GetMp();
		rop->rad		= op->GetRad();
		if (op->IsInstanceOf(Opolygon)){
			rop->pcnt	= ToPoly(op)->GetPointCount();
			rop->padr	= ToPoly(op)->GetPointW();
			rop->vcnt	= ToPoly(op)->GetPolygonCount();
			rop->vadr	= (RayPolygon*)ToPoly(op)->GetPolygonW();
			rop->type   = O_POLYGON;
		}
	}
	//----------------- TexData -------------------
	tex->mp			= mat;// Set Material
	//------------ InitRenderStruct ----------------
	//irs.fps		= fps;
	//irs.time		= BaseTime(time);
	//irs.doc		= doc;
	//irs.docpath	= dpath;
	irs.Init(doc); //R15 only
	irs.vd		= cd.vd;
	//---------------- Sampling ---------------------
	chandata	= texChan1->GetData();
	cd.off		= chandata.GetFloat(BASECHANNEL_BLUR_OFFSET,0.0);
	cd.scale	= 0.0;
	cd.d		= Vector(0.0);
	cd.n		= Vector(0.0,1.0,0.0);
	cd.texflag	= TEX_TILE;
	if (texShader->InitRender(irs)==INITRENDERRESULT_OK) { 
		TexInit = TRUE;
		return INIT_SAMPLER_RESULT_OK; //OK
	}else{ 
		err = INIT_SAMPLER_RESULT_NO_INITRENDER;
	}
	texShader->FreeRender();
	return err;
}
//-------------------------------------------------------------------------------------------------
inline Sampler::Sampler()
{
	texShader		= nullptr;
	TexInit			= FALSE;
	offsetX			= 1.0;
	offsetY			= 1.0;
	lenX			= 1.0;
	lenY			= 1.0;
	//--------------- VolumeData -----------------
	//vd.Assign()
	//cd.vd =	 VolumeData::Alloc(); 
	if (!vd) { return; } //! VolumeData is null !
	cd.vd = vd;
	cd.vd->version		= 1;  //Int32
	cd.vd->fps			= 25; //Int32
	cd.vd->ambient		= Vector(0.5);//0.0;
	cd.vd->time			= 0;  //Float
	cd.vd->bumpn		= Vector(0.0,1.0,0.0);
	cd.vd->back_delta	= Vector(0.0);
	cd.vd->global_mip	= 1.0; //Float
	cd.vd->orign		= Vector(0.0,1.0,0.0);
	cd.vd->n			= Vector(0.0,1.0,0.0);
	cd.vd->dispn		= Vector(0.0,1.0,0.0);
	cd.vd->delta		= Vector(0.0);
	cd.vd->dist			= 0.0; //Float
	cd.vd->lhit			= RayHitID();
	cd.vd->cosc			= 1.0; //Float Angle
	cd.vd->ddu			= Vector(1.0,0.0,0.0);
	cd.vd->ddv			= Vector(0.0,1.0,0.0);
	cd.vd->tray			= nullptr; //TRay
	cd.vd->rray			= nullptr; //RRay
	cd.vd->ray			= nullptr; //Ray
	cd.vd->xlight		= nullptr; //RayLight
	//---------------- TexData ------------------
	//tex.Assign( TexData::Alloc() ); //! Assign() is a dangerous method please use Reset() instate !
	if (!tex) { return; } //! TexData is null !
	tex->Init();	
	tex->proj		= P_UVW;
	tex->texflag	= TEX_TILE;
	tex->lenx		= lenX;
	tex->leny		= lenY;
	tex->ox			= offsetX;
	tex->oy			= offsetY;
	cd.vd->tex		= tex;
	//---------------- RayObject -----------------
	rop = AllocRayObject(0); 
	if (!rop) { return; } //! RayObject is null !
	rop->link		  = nullptr;
	rop->texture_link = nullptr;
	rop->visibility	= 1.0;
	rop->pcnt		= 0;
	rop->padr		= nullptr;
	rop->vcnt		= 0;
	rop->vadr		= nullptr;
	rop->texcnt		= 0;
	rop->texadr		= nullptr;
	rop->uvwadr		= nullptr;
	rop->rsadr		= nullptr;
	cd.vd->op		= rop;
	//---------------- RenderStruct -----------------
	//irs = gNew(InitRenderStruct);
}
//-------------------------------------------------------------------------------------------------
inline void Sampler::Free()
{
	if (texShader){ 
		if (TexInit){ texShader->FreeRender(); }
	}
	TexInit = FALSE;
}
//-------------------------------------------------------------------------------------------------
inline Sampler::~Sampler(void)
{
	Free();
	//if (tex) TexData::Free(tex);
	if (cd.vd){  VolumeData::Free(cd.vd); }
	FreeRayObject(rop);
	//gDelete(irs);
}
//-------------------------------------------------------------------------------------------------
inline Vector Sampler::AverageColor(Int32 num_samples)
{
	const Int32 random_seed = 43;
	const Float scale3d = 50.0;
	
	Random rnd; rnd.Init(random_seed);
	Vector ave_color = Vector(0.0);
	Float cnt = 0.0;
	
	Vector pos3d; //3d coordinates for 3D shader.
	Vector uv; //UVW coordinates for all other shaders.
	for(Int32 i=0; i < num_samples; ++i){
		uv = Vector(rnd.Get01(),rnd.Get01(),0.0);
		pos3d = Vector(rnd.Get11(),rnd.Get11(),rnd.Get11()) * scale3d;
		const Vector color = Sample3D(pos3d, uv);  
		ave_color += (color - ave_color) / cnt;
		cnt += 1.0;
	}
	
	return ave_color;
}

//#####################################################################################################
///					Examples
//#####################################################################################################

//#include "C4DPrintPublic.h"
//#include "c4d_misc.h"
// ----------------------------------------------------------------------------------------------------
Bool SampleColorAtVertices(BaseObject *obj) //Remo: 02.08.2014
{
	if(! obj->IsInstanceOf(Opolygon)) if (!obj) return FALSE; //Not a polygon Object 
	PolygonObject *polyo = ToPoly(obj);

	const Int32   pcnt = polyo->GetPointCount();
	const Vector *padr = polyo->GetPointR();
	const Int32	  vcnt = polyo->GetPolygonCount();
	const CPolygon *vadr = polyo->GetPolygonR();

	UVWTag *uvw_tag = (UVWTag*)polyo->GetTag(Tuvw); if (!uvw_tag) return FALSE;
	const Int32 uv_cnt    = uvw_tag->GetDataCount();
	ConstUVWHandle uv_handle = uvw_tag->GetDataAddressR();
	if(uv_cnt != vcnt)  return FALSE; //Wrong UVS count !


	Sampler smpl;
	const INIT_SAMPLER_RESULT init_res = smpl.Init(polyo,CHANNEL_COLOR);
	if(init_res != INIT_SAMPLER_RESULT_OK) return FALSE;

	struct Colors {
		Colors() : sampled(false) {}
		Vector col;
		Bool   sampled;
	};
	maxon::BaseArray<Colors>	colors;
	colors.Resize(pcnt);

	/// Sample using vertex position and uv.
	UVWStruct uvw;
	for(Int32 c=0; c<vcnt; ++c) {
		const CPolygon &cp = vadr[c];
		uvw_tag->Get(uv_handle,c,uvw);
		{ //a
			Colors &ca = colors[cp.a];
			if(!ca.sampled){ ca.col = smpl.Sample3D(padr[cp.a],uvw.a); ca.sampled = true;  }
		}
		{ //b
			Colors &cb = colors[cp.b];
			if(!cb.sampled){ cb.col = smpl.Sample3D(padr[cp.b],uvw.b); cb.sampled = true; 	}
		}
		{ //c
			Colors &cc = colors[cp.c];
			if(!cc.sampled){ cc.col = smpl.Sample3D(padr[cp.c],uvw.c); cc.sampled = true; 	}
		}
		if(cp.c!=cp.d) { //d
			Colors &cd = colors[cp.d];
			if(!cd.sampled){ cd.col = smpl.Sample3D(padr[cp.d],uvw.d); cd.sampled = true; 	}
		}
	}

	//print result 
	for(Int32 i=0; i<pcnt; ++i)	{
		print(i,colors[i].col,colors[i].sampled);
	}
}

#endif//_SAMPLER_REMO_H_

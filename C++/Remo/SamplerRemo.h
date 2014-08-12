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

	Bool ProjectPoint(const Vector &p, const Vector &n, Vector *uv);

	VolumeData* GetVolumeData() { return vd; }
	TexData*    GetTexData() { return tex; }
 
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
// ----------------------------------------------------------------------------------------------------
inline Bool ReadTextureTag(TextureTag *textag, TexData *tex)
{	
	if(textag==nullptr) return FALSE;
	if(tex==nullptr) return FALSE;

	//Get Texture Data
	BaseContainer*		texdata = textag->GetDataInstance();
	const Bool			tex_tile = texdata->GetBool(TEXTURETAG_TILE);
	const Bool			tex_seam = texdata->GetBool(TEXTURETAG_SEAMLESS);

	tex->Init();
	//tex->mp		= mat;//<<POINTER>> ???
	tex->m			= textag->GetMl();
	tex->im			= !tex->m; //matrix inverse
	tex->proj		= texdata->GetInt32(TEXTURETAG_PROJECTION); 
	tex->texflag	= (tex_tile)?TEX_TILE:0;tex->texflag += (tex_seam)?TEX_MIRROR:0;
	tex->side		= texdata->GetInt32(TEXTURETAG_SIDE);
	tex->lenx		= texdata->GetFloat(TEXTURETAG_LENGTHX);
	tex->leny		= texdata->GetFloat(TEXTURETAG_LENGTHY);
	tex->ox			= texdata->GetFloat(TEXTURETAG_OFFSETX)+EPSILON;
	tex->oy			= texdata->GetFloat(TEXTURETAG_OFFSETY)+EPSILON;

	return TRUE;
}
//-------------------------------------------------------------------------------------------------
inline INIT_SAMPLER_RESULT Sampler::Init(BaseObject *obj,Int32 chnr,Float time)
{
	if(!obj) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseDocument *doc = obj->GetDocument(); if (!doc) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	TextureTag* textag = (TextureTag*)obj->GetTag(Ttexture);  if (!textag) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseMaterial *mat = textag->GetMaterial(); 	if(!mat) return INIT_SAMPLER_RESULT_NO_MATERIAL;//no Material
	ReadTextureTag(textag,tex);
	lenX     = tex->lenx;
	lenY     = tex->leny;
	offsetX  = tex->ox	;
	offsetY  = tex->oy	;
	return Init(mat,chnr,time,doc,obj);
}
//-------------------------------------------------------------------------------------------------
inline INIT_SAMPLER_RESULT Sampler::Init(TextureTag *textag,Int32 chnr,Float time,BaseDocument *doc,BaseObject *op)
{
	if(!textag) return INIT_SAMPLER_RESULT_WRONG_PARAM;
	BaseMaterial *mat = textag->GetMaterial(); 	if(!mat) return INIT_SAMPLER_RESULT_NO_MATERIAL;//no Material
	ReadTextureTag(textag,tex);
	lenX     = tex->lenx;
	lenY     = tex->leny;
	offsetX  = tex->ox	;
	offsetY  = tex->oy	;
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
	//tex.Free(); //we do not need this
	//vd.Free(); //we do not need this
	FreeRayObject(rop);
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
// ----------------------------------------------------------------------------------------------------
Bool Sampler::ProjectPoint(const Vector &p, const Vector &n, Vector *uv)
{
	Float lenxinv=0.0,lenyinv=0.0;
	if (tex->lenx!=0.0) lenxinv = 1.0/tex->lenx;
	if (tex->leny!=0.0) lenyinv = 1.0/tex->leny;

	switch (tex->proj)
	{
	case P_VOLUMESHADER:
		{
			*uv = p * tex->im;
			return true;
		}

	case P_SPHERICAL: default:
		{
			Vector d = p * tex->im;
			Float sq = Sqrt(d.x*d.x + d.z*d.z);
			if (sq==0.0)
			{
				uv->x = 0.0;
				if (d.y>0.0)
					uv->y = +0.5;
				else
					uv->y = -0.5;
			}
			else
			{
				uv->x = ACos(d.x/sq)/PI2;
				if (d.z<0.0) uv->x = 1.0-uv->x;

				uv->x -= tex->ox;
				if (tex->lenx>0.0 && uv->x<0.0)
					uv->x += 1.0;
				else if (tex->lenx<0.0 && uv->x>0.0)
					uv->x -= 1.0;
				uv->x *= lenxinv;
				uv->y = ATan(d.y/sq)/PI;
			}
			uv->y = -(uv->y+tex->oy)*lenyinv;
			break;
		}

	case P_SHRINKWRAP:
		{
			Vector d = p * tex->im;
			Float   sn,cs,sq = Sqrt(d.x*d.x + d.z*d.z);

			if (sq==0.0)
			{
				uv->x = 0.0;
				if (d.y>0.0)
					uv->y = 0.0;
				else
					uv->y = 1.0;
			}
			else
			{
				uv->x = ACos(d.x/sq)/PI2;
				if (d.z<0.0) uv->x = 1.0-uv->x;
				uv->y = 0.5-ATan(d.y/sq)/PI;
			}

			SinCos(uv->x*PI2,sn,cs);

			uv->x = (0.5 + 0.5*cs*uv->y - tex->ox)*lenxinv;
			uv->y = (0.5 + 0.5*sn*uv->y - tex->oy)*lenyinv;
			break;
		}

	case P_CYLINDRICAL:
		{
			Vector d = p * tex->im;
			Float sq = Sqrt(d.x*d.x + d.z*d.z);
			if (sq==0.0)
				uv->x = 0.0;
			else
			{
				uv->x = ACos(d.x/sq)/PI2;
				if (d.z<0.0) uv->x = 1.0-uv->x;

				uv->x -= tex->ox;
				if (tex->lenx>0.0 && uv->x<0.0)
					uv->x += 1.0;
				else if (tex->lenx<0.0 && uv->x>0.0)
					uv->x -= 1.0;
				uv->x *= lenxinv;
			}
			uv->y = -(d.y*0.5+tex->oy)*lenyinv;
			break;
		}

	case P_FLAT: case P_SPATIAL:
		{
			Vector d = p * tex->im;
			uv->x =  (d.x*0.5-tex->ox)*lenxinv;
			uv->y = -(d.y*0.5+tex->oy)*lenyinv;
			break;
		}

	case P_CUBIC:
		{
			Vector d = p * tex->im;
			Vector v = n ^ tex->im; 
			Int32   dir;

			if (Abs(v.x)>Abs(v.y))
			{
				if (Abs(v.x)>Abs(v.z))
					dir = 0; 
				else
					dir = 2; 
			}
			else
			{
				if (Abs(v.y)>Abs(v.z))
					dir = 1; 
				else
					dir = 2; 
			}

			switch (dir)
			{
			case 0: // x axis
				{
					if (v.x<0.0)
						uv->x = (-d.z*0.5-tex->ox)*lenxinv;
					else
						uv->x = ( d.z*0.5-tex->ox)*lenxinv;

					uv->y = -(d.y*0.5+tex->oy)*lenyinv;
					break;
				}

			case 1:  // y axis
				{
					if (v.y<0.0)
						uv->y = ( d.z*0.5-tex->oy)*lenyinv;
					else
						uv->y = (-d.z*0.5-tex->oy)*lenyinv;

					uv->x = (d.x*0.5-tex->ox)*lenxinv;
					break;
				}

			case 2: // z axis
				{
					if (v.z<0.0)
						uv->x = ( d.x*0.5-tex->ox)*lenxinv;
					else
						uv->x = (-d.x*0.5-tex->ox)*lenxinv;

					uv->y = -(d.y*0.5+tex->oy)*lenyinv;
					break;
				}
			}

			break;
		}

	case P_FRONTAL:
		{
			RayParameter *param=vd->GetRayParameter();

			Float ox=0.0,oy=0.0,ax=param->xres,ay=param->yres;
			Int32 curr_x,curr_y,scl;
			vd->GetXY(&curr_x,&curr_y,&scl);
			uv->x = ((Float(curr_x)/Float(scl)-ox)/ax - tex->ox)*lenxinv;
			uv->y = ((Float(curr_y)/Float(scl)-ox)/ay - tex->oy)*lenyinv;
			break;
		}

	case P_UVW:
		{
			/*const RayHitID& lhit,*/ /*Int32 lhit,*/ 
			/*
			RayObject *op = lhit.GetObject(vd); //RayObject *op = vd->ID_to_Obj(lhit,nullptr);
			if (op && tex->uvwind<op->uvwcnt && op->uvwadr[tex->uvwind])
				*uv=vd->GetPointUVW(tex,lhit,p);
			else
				uv->x = uv->y = 0.0;
			*/
			break;
		}
	}//switch

	if (tex->texflag&TEX_TILE)
		return true;
	else
		return uv->x>=0.0 && uv->x<=1.0 && uv->y>=0.0 && uv->y<=1.0;
}

//#####################################################################################################
///					Examples
//#####################################################################################################
//#include "C4DPrintPublic.h"
//#include "c4d_misc.h"
// ----------------------------------------------------------------------------------------------------
Int32 SampleColorAtVertices(BaseObject *obj) //Remo: 02.08.2014
{
	if(! obj->IsInstanceOf(Opolygon)) if (!obj) return -11; //Not a polygon Object 
	PolygonObject *polyo = ToPoly(obj);

	const Int32   pcnt = polyo->GetPointCount();
	const Vector *padr = polyo->GetPointR();
	const Int32	  vcnt = polyo->GetPolygonCount();
	const CPolygon *vadr = polyo->GetPolygonR();

	UVWTag *uvw_tag = (UVWTag*)polyo->GetTag(Tuvw); //if (!uvw_tag) return -12;
	Int32 uv_cnt  = 0;
	ConstUVWHandle uv_handle = nullptr;
	if(uvw_tag!=nullptr) {
		uv_cnt    = uvw_tag->GetDataCount();
		uv_handle = uvw_tag->GetDataAddressR();
		if(uv_cnt != vcnt)  return -13; //Wrong UVS count !
	}

	Vector n = Vector(0.0,1.0,0.0); //normal
	Sampler smpl;
	const INIT_SAMPLER_RESULT init_res = smpl.Init(polyo,CHANNEL_COLOR);
	if(init_res != INIT_SAMPLER_RESULT_OK) return init_res;

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
		if(uv_handle!=nullptr){ 
			uvw_tag->Get(uv_handle,c,uvw); }
		else{ 
			//Not really tested code, Please contribute fixes !
			smpl.ProjectPoint(padr[cp.a],n, &uvw.a); //print(uvw.a);
			smpl.ProjectPoint(padr[cp.b],n, &uvw.b); //print(uvw.b);
			smpl.ProjectPoint(padr[cp.c],n, &uvw.c); //print(uvw.c);
			if(cp.c!=cp.d){ smpl.ProjectPoint(padr[cp.d],n, &uvw.d); }
		}
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

	return INIT_SAMPLER_RESULT_OK; // OK
}

#endif//_SAMPLER_REMO_H_

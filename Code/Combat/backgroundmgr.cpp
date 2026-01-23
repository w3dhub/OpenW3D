/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/backgroundmgr.cpp                     $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/19/02 3:45p                                               $*
 *                                                                                             *
 *                    $Revision:: 43                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "backgroundmgr.h"
#include "apppackettypes.h"
#include "assetmgr.h"
#include "AudibleSound.h"
#include "camera.h"
#include "dazzle.h"
#include "dx8wrapper.h"
#include "gameobjmanager.h"
#include "gametype.h"
#include "matrix3d.h"
#include "phys.h"
#include "pscene.h"
#include "random.h"
#include "rinfo.h"
#include "scene.h"
#include "SoundEnvironment.h"
#include "texture.h"
#include "WWAudio.h"
#include "wwmemlog.h"
#include "seglinerenderer.h"
#include "textureloader.h"


// Singletons.
BackgroundMgrClass _TheBackgroundMgr;


// Static data.
unsigned											  BackgroundMgrClass::_Hours;
unsigned											  BackgroundMgrClass::_Minutes;
BackgroundMgrClass::LightSourceTypeEnum  BackgroundMgrClass::_LightSourceType;
SkyClass::MoonTypeEnum						  BackgroundMgrClass::_MoonType;
BackgroundParameterClass					  BackgroundMgrClass::_Parameters [PARAMETER_COUNT];
Vector3											  BackgroundMgrClass::_LightVector;
Vector3											  BackgroundMgrClass::_UnitLightVector;
unsigned											  BackgroundMgrClass::_CloudOverrideCount;
unsigned											  BackgroundMgrClass::_LightningOverrideCount;
unsigned											  BackgroundMgrClass::_SkyTintOverrideCount;
bool												  BackgroundMgrClass::_Dirty;

SkyClass											 *BackgroundMgrClass::_Sky		= NULL;
DazzleRenderObjClass							 *BackgroundMgrClass::_Dazzle	= NULL;

static Random2Class _RandomNumber (0x1f855092);


/***********************************************************************************************
 * HazeClass::HazeClass -- Constructor																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
HazeClass::HazeClass (float radius)
	: RowCount (ROW_COUNT)
{
	const Vector3	white	(1.0f, 1.0f, 1.0f);
	const float		longitude [ROW_COUNT + 1] = {1.22f, 1.55f, 1.57f, 1.92f};
	const unsigned	segmentcount				  = 16;
	const float		twopioosegmentcount		  = (1.0f / segmentcount) * 2.0f * WWMATH_PI;

	unsigned			row, segment;
	unsigned			v;
	float				latitude;
	float				x, y, z;
	unsigned short i;

	VertexCount	  = (RowCount + 1) * segmentcount;
	TriangleCount = segmentcount * (RowCount * 2);

	// Define vertices.
	VertexArray = new Vector3 [VertexCount];
	WWASSERT (VertexArray != NULL);
	v = 0;
	for (segment = 0; segment < segmentcount; segment++) {
		latitude = segment * twopioosegmentcount;
		for (row = 0; row < RowCount + 1; row++) {
			x = radius * sinf (longitude [row]) * cosf (latitude);
			y = radius * sinf (longitude [row]) * sinf (latitude);
			z = radius * cosf (longitude [row]);
			VertexArray [v].Set (x, y, z);
			v++;
		}
	}

	// Define triangles.
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (TriangleCount * VERTICES_PER_TRIANGLE));
	{
		DX8IndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		i = 0;
		for (row = 0; row < RowCount; row++) {
			v = row;
			for (segment = 0; segment < segmentcount; segment++) {

				// Is this not the last segment?
				if (segment < segmentcount - 1) {
					indices [i + 0] = v;
					indices [i + 1] = v + RowCount + 1;
					indices [i + 2] = v + RowCount + 2;
					indices [i + 3] = v + RowCount + 2;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				} else {
					indices [i + 0] = v;
					indices [i + 1] = row;
					indices [i + 2] = row + 1;
					indices [i + 3] = row + 1;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				}
				v += RowCount + 1;
				i += 6;
			}
		}
	}

	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	Shader = ShaderClass::_PresetOpaque2DShader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Texturing (ShaderClass::TEXTURING_DISABLE);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);

	DiffuseArray = new unsigned [VertexCount];
	WWASSERT (DiffuseArray != NULL);

	Configure (white, white, 1.0f);
}


/***********************************************************************************************
 * HazeClass::~HazeClass -- Destructor																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
HazeClass::~HazeClass()
{
	// Clean-up.
	delete [] DiffuseArray;
	REF_PTR_RELEASE (Material);
	REF_PTR_RELEASE (IndexBuffer);
	delete [] VertexArray;
}


/***********************************************************************************************
 * HazeClass::Configure --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Configure (const Vector3 &blendcolor, const Vector3 &horizoncolor, float intensity)
{
	BlendColor	 = blendcolor;
	HorizonColor = horizoncolor;
	Intensity	 = intensity;
	Configure();
}


/***********************************************************************************************
 * HazeClass::Configure --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Configure (const Vector3 &blendcolor, const Vector3 &horizoncolor)
{
	BlendColor	 = blendcolor;
	HorizonColor = horizoncolor;
	Configure();
}


/***********************************************************************************************
 * HazeClass::Configure --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Configure (const Vector3 &blendcolor, float intensity)
{
	BlendColor = blendcolor;
	Intensity  = intensity;
	Configure();
}


/***********************************************************************************************
 * HazeClass::Configure --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Configure (const Vector3 &blendcolor)
{
	BlendColor = blendcolor;
	Configure();
}


/***********************************************************************************************
 * HazeClass::Configure --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Configure()
{
	const unsigned	d3dblendcolor	 =	DX8Wrapper::Convert_Color (BlendColor, 1.0f);
	const unsigned	d3dhorizoncolor = DX8Wrapper::Convert_Color (HorizonColor * Intensity, 1.0f);

	for (unsigned v = 0; v < VertexCount; v++) {

		switch (v % (RowCount + 1)) {

			case 0:
				DiffuseArray [v] = d3dblendcolor;
				break;

			default:
				DiffuseArray [v] = d3dhorizoncolor;
				break;
		}
	}

	// Haze is always visible.
	Set_Visibility (true);
}


/***********************************************************************************************
 * HazeClass::Render --																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void HazeClass::Render()
{
	if (Is_Visible()) {

		// Copy the vertices into a dynamic vertex buffer.
		// NOTE: Vertex normals and UV's are uninitialized.
		DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, VertexCount);
		{
			DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);
			VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

			for (unsigned v = 0; v < VertexCount; v++) {
				vertex->x		 = VertexArray [v].X;
				vertex->y		 = VertexArray [v].Y;
				vertex->z		 = VertexArray [v].Z;
				vertex->diffuse = DiffuseArray [v];
				vertex++;
			}
		}

		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);
		DX8Wrapper::Set_Vertex_Buffer (dynamicvb);
		DX8Wrapper::Draw_Triangles (0, TriangleCount, 0, VertexCount);
	}
}


/***********************************************************************************************
 * StarfieldClass::StarfieldClass --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
StarfieldClass::StarfieldClass (float /* extent */, unsigned starcount)
{
	const Vector3 white (1.0f, 1.0f, 1.0f);

	VertexCount	  = VERTICES_PER_TRIANGLE * starcount;
	TriangleCount = starcount;

	VertexArray = new Vector3 [VertexCount];
	WWASSERT (VertexArray != NULL);

	// Define triangles.
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (VertexCount));
	{
		DX8IndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		for (unsigned short i = 0; i < VertexCount; i++) {
			indices [i] = i;
		}
	}

	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	Shader = ShaderClass::_PresetAlpha2DShader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);

	Texture = WW3DAssetManager::Get_Instance()->Get_Texture ("Star.tga");
	Texture->Set_U_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
	Texture->Set_V_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);

	#ifdef WW3D_DX8
	Set_Texture_Hint (Texture, srTextureIFace::HINT_ALPHA_ONLY);
	#endif

	DiffuseArray = new unsigned [VertexCount];
	WWASSERT (DiffuseArray != NULL);

	Configure (Vector3 (1.0f, 0.0f, 0.0f), 1.0f, 1.0f, white, white, 0.0f);
}


/***********************************************************************************************
 * StarfieldClass::~StarfieldClass -- Destructor															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
StarfieldClass::~StarfieldClass()
{
	// Clean-up.
	delete [] DiffuseArray;
	REF_PTR_RELEASE (Texture);
	REF_PTR_RELEASE (Material);
	REF_PTR_RELEASE (IndexBuffer);
	delete [] VertexArray;
}


/***********************************************************************************************
 * StarfieldClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
void StarfieldClass::Configure (const Vector3 &orientation, float length, float radius, const Vector3 &color0, const Vector3 &color1, float alpha)
{
	Orientation = orientation;
	Length		= length;
	Radius		= radius;
	Color0		= color0;
	Color1		= color1;
	Alpha			= alpha;
	Configure();
}


/***********************************************************************************************
 * StarfieldClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
void StarfieldClass::Configure (const Vector3 &orientation, float length, float radius)
{
	Orientation = orientation;
	Length		= length;
	Radius		= radius;
	Configure();
}


/***********************************************************************************************
 * StarfieldClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
void StarfieldClass::Configure (const Vector3 &color0, const Vector3 &color1, float alpha)
{
	Color0		= color0;
	Color1		= color1;
	Alpha			= alpha;
	Configure();
}


/***********************************************************************************************
 * StarfieldClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
void StarfieldClass::Configure()
{
	const unsigned positionrandomness		= 8192;
	const float		oopositionrandomnesstwo = 2.0f / positionrandomness;
	const unsigned radiusrandomness			= 16;
	const float		ooradiusrandomness		= 1.0f / radiusrandomness;
	const float		minradius					= 0.65f;
	const float		maxradius					= 0.90f;
	const unsigned	intensityrandomness		= 16;
	const float		oointensityrandomness	= 1.0f / intensityrandomness;
	const float		theta							= sinf (WWMATH_PI / 36);

	Matrix3D		 m0;
	unsigned		 t;
	Vector3		 colordifference;
	float			 maxdp;

	colordifference = Color1 - Color0;
	m0.Look_At (Vector3 (0.0f, 0.0f, 0.0f), Orientation, 0.0f);
	maxdp = Length / sqrtf ((Length * Length) + (Radius * Radius));

	ActiveVertexCount	  = 0;
	ActiveTriangleCount = 0;
	for (t = 0; t < TriangleCount; t++) {

		float	   x, y, z;
		Vector3  d;
		float		r, intensity;

		// NOTE: Generate all random numbers regardless of orientation to ensure a consistent starfield.
		x = (_RandomNumber (0, positionrandomness) * oopositionrandomnesstwo) - 1.0f;
		y = (_RandomNumber (0, positionrandomness) * oopositionrandomnesstwo) - 1.0f;
		z = (_RandomNumber (0, positionrandomness) * oopositionrandomnesstwo) - 1.0f;
		r			 = _RandomNumber (1, radiusrandomness) * ooradiusrandomness;
		intensity = _RandomNumber (1, intensityrandomness) * oointensityrandomness;
		d.Set (x, y, z);
		d.Normalize();
		d = m0 * d;

		// Optimization: If the star is located an arbitrary angle above the horizon deem it visible.
		if (d.Z >= theta) {

			// Ensure that the star does not lie within the radius of the orientation vector
			// (this area is reserved for another object).
			if (Vector3::Dot_Product (d, Orientation) < maxdp) {

				Matrix3D m1;
				Vector3	color;
				unsigned	d3dcolor;

				m1.Look_At (d * Length, d * Length * 2.0f, 0.0f);

				r = pow (r, 10) * maxradius;
				r = MAX (r, minradius);
				VertexArray [ActiveVertexCount]		= m1 * Vector3 (-r, -r, 0.0f);
				VertexArray [ActiveVertexCount + 1] = m1 * Vector3 (-r, +r, 0.0f);
				VertexArray [ActiveVertexCount + 2] = m1 * Vector3 (+r, -r, 0.0f);

				color		= Color0 + (colordifference * intensity);
				d3dcolor = DX8Wrapper::Convert_Color (color, Alpha);

				DiffuseArray [ActiveVertexCount + 0] = d3dcolor;
				DiffuseArray [ActiveVertexCount + 1] = d3dcolor;
				DiffuseArray [ActiveVertexCount + 2] = d3dcolor;

				ActiveTriangleCount++;
				ActiveVertexCount += VERTICES_PER_TRIANGLE;
			}
		}
	}

	// Optimization: If alpha is zero then stars are invisible.
	Set_Visibility (Alpha > 0.0f);
}


/***********************************************************************************************
 * StarfieldClass::Render --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/00    IML : Created.                                                                *
 *=============================================================================================*/
void StarfieldClass::Render()
{
	if (Is_Visible()) {

		const unsigned flickercount = 5;
		const float		flickeralpha = 0.6f;

		unsigned	activeflickercount;
		float		alpha, f, frac;
		unsigned	triangleindices [flickercount];
		unsigned	i, v;

		// If time has gone by this frame...
		if (WW3D::Get_Frame_Time() > 0) {

			// Emulate flickering stars by attenuating some randomly selected diffuse alphas.
			alpha	= Alpha * flickeralpha;
			f	   = flickercount * (((float) ActiveTriangleCount) / TriangleCount);
			frac  = f - floorf (f);
			activeflickercount = (frac < 0.5f) ? floorf (f) : floorf (f) + 1;
			for (i = 0; i < activeflickercount; i++) {
				triangleindices [i] = _RandomNumber (0, ActiveTriangleCount - 1);
				v = triangleindices [i] * VERTICES_PER_TRIANGLE;
				DX8Wrapper::Set_Alpha (alpha, DiffuseArray [v + 0]);
				DX8Wrapper::Set_Alpha (alpha, DiffuseArray [v + 1]);
				DX8Wrapper::Set_Alpha (alpha, DiffuseArray [v + 2]);
			}

		} else {
			activeflickercount = 0;
		}

		// Copy the vertices into a dynamic vertex buffer.
		// NOTE: Vertex normals and stage 1 UV's are uninitialized.
		DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, ActiveVertexCount);
		{
			const float	texcoordarray [VERTICES_PER_TRIANGLE][2] = {{0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}};

			DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);
			VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

			for (unsigned v = 0; v < ActiveVertexCount; v += VERTICES_PER_TRIANGLE) {
				for (unsigned t = 0; t < VERTICES_PER_TRIANGLE; t++) {

					unsigned i = v + t;

					vertex->x		 = VertexArray [i].X;
					vertex->y		 = VertexArray [i].Y;
					vertex->z		 = VertexArray [i].Z;
					vertex->diffuse = DiffuseArray [i];
					vertex->u1		 = texcoordarray [t][0];
					vertex->v1		 = texcoordarray [t][1];
					vertex++;
				}
			}
		}

		DX8Wrapper::Set_Texture (0, Texture);
		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);
		DX8Wrapper::Set_Vertex_Buffer (dynamicvb);
		DX8Wrapper::Draw_Triangles (0, ActiveTriangleCount, 0, ActiveVertexCount);

		// Restore alphas for those stars that were modified prior to rendering.
		for (i = 0; i < activeflickercount; i++) {
			v = triangleindices [i] * VERTICES_PER_TRIANGLE;
			DX8Wrapper::Set_Alpha (Alpha, DiffuseArray [v + 0]);
			DX8Wrapper::Set_Alpha (Alpha, DiffuseArray [v + 1]);
			DX8Wrapper::Set_Alpha (Alpha, DiffuseArray [v + 2]);
		}
	}
}


/***********************************************************************************************
 * SkyObjectClass::SkyObjectClass -- Constructor															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyObjectClass::SkyObjectClass (ShaderClass shader)
 :	VertexCount (8),
	TriangleCount (6)
{
	const Vector3 white (1.0f, 1.0f, 1.0f);

	VertexArray = new Vector3 [VertexCount];
	WWASSERT (VertexArray != NULL);

	// Define triangles.
	// NOTE: For simplicity, assume that there are exactly 8 vertices and 6 triangles.
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (TriangleCount * VERTICES_PER_TRIANGLE));
	{
		static const unsigned short _indices [] = {0, 4, 5, 5, 1, 0, 1, 5, 6, 6, 2, 1, 2, 6, 7, 7, 3, 2};

		DX8IndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		for (unsigned short i = 0; i < TriangleCount * VERTICES_PER_TRIANGLE; i++) {
			indices [i] = _indices [i];
		}
	}

	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	Shader = shader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Src_Blend_Func (ShaderClass::SRCBLEND_SRC_ALPHA);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);

	Texture = NULL;

	// Define texture coordinates.
	// NOTE: Must be in clockwise order.
	TexCoordArray = new Vector2 [VertexCount];
	WWASSERT (TexCoordArray != NULL);

	DiffuseArray = new unsigned [VertexCount];
	WWASSERT (DiffuseArray != NULL);

	Configure (Vector3 (1.0f, 0.0f, 0.0f), 1.0f, 1.0f, white);
}


/***********************************************************************************************
 * SkyObjectClass::~SkyObjectClass -- Destructor															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyObjectClass::~SkyObjectClass()
{
	// Clean-up.
	delete [] DiffuseArray;
	delete [] TexCoordArray;
	REF_PTR_RELEASE (Texture);
	REF_PTR_RELEASE (Material);
	REF_PTR_RELEASE (IndexBuffer);
	delete [] VertexArray;
}


/***********************************************************************************************
 * SkyObjectClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Configure (const Vector3 &direction, float length, float radius, const Vector3 &color)
{
	Direction = direction;
	Length	 = length;
	Width		 = radius;
	Height	 = radius;
	Color		 = color;
	Configure();
}


/***********************************************************************************************
 * SkyObjectClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Configure (const Vector3 &direction, float length, float width, float height)
{
	Direction = direction;
	Length	 = length;
	Width		 = width;
	Height	 = height;
	Configure();
}


/***********************************************************************************************
 * SkyObjectClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Configure (const Vector3 &direction, float length, float radius)
{
	Direction = direction;
	Length	 = length;
	Width		 = radius;
	Height	 = radius;
	Configure();
}


/***********************************************************************************************
 * SkyObjectClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Configure (const Vector3 &color)
{
	Color	= color;
	Configure();
}


/***********************************************************************************************
 * SkyObjectClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Configure()
{
	const Vector3 black				  = Vector3 (0.0f, 0.0f, 0.0f);
	const float	  defaultlambda [2] = {0.0f,  1.0f};
	const float	  zblend [2]		  = {0.0f, 20.0f};

	Matrix3D m;
	float		z0, z1;
	bool		visible;
	float		lambda [2];

	m.Look_At (Direction * Length, Direction * (Length * 2.0f), 0.0f);

	// Set the corner vertices.
	// NOTE: For simplicity, assume that there are exactly 8 vertices.
	VertexArray	  [0] = m * Vector3 (-Width, -Height, 0.0f);
	TexCoordArray [0].Set (0.0f, 0.0f);
	VertexArray	  [4] = m * Vector3 (+Width, -Height, 0.0f);
	TexCoordArray [4].Set (1.0f, 0.0f);
	VertexArray	  [7] = m * Vector3 (+Width, +Height, 0.0f);
	TexCoordArray [7].Set (1.0f, 1.0f);
	VertexArray	  [3] = m * Vector3 (-Width, +Height, 0.0f);
	TexCoordArray [3].Set (0.0f, 1.0f);

	z0 = VertexArray [0].Z;
	z1 = VertexArray [3].Z;

	for (unsigned r = 0; r < 2; r++) {

		if (((z0 >= zblend [r]) && (z1 >= zblend [r])) || ((z0 <= zblend [r]) && (z1 <= zblend [r]))) {
			lambda [r] = defaultlambda [r];
		} else {
			lambda [r] = (zblend [r] - z0) / (z1 - z0);
		}

		VertexArray	  [1 + r] = VertexArray	  [0] + lambda [r] * (VertexArray	[3] - VertexArray	  [0]);
		TexCoordArray [1 + r] = TexCoordArray [0] + lambda [r] * (TexCoordArray [3] - TexCoordArray [0]);
		VertexArray	  [5 + r] = VertexArray	  [4] + lambda [r] * (VertexArray	[7] - VertexArray	  [4]);
		TexCoordArray [5 + r] = TexCoordArray [4] + lambda [r] * (TexCoordArray [7] - TexCoordArray [4]);
	}

	visible = false;
	for (unsigned v = 0; v < VertexCount; v++) {

		float alpha;

		if (VertexArray [v].Z <= zblend [0]) {
			alpha = 0.0f;
		} else {
			if (VertexArray [v].Z >= zblend [1]) {
				alpha = 1.0f;
			} else {
				alpha = (VertexArray [v].Z - zblend [0]) / (zblend [1] - zblend [0]);
			}
		}
		visible |= (alpha > 0.0f);
		DiffuseArray [v] = DX8Wrapper::Convert_Color (Color, alpha);
	}

	// Optimization: If all vertex alphas are zero then the object is invisible.
	Set_Visibility (visible);
}


/***********************************************************************************************
 * SkyObjectClass::Set_Texture --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Set_Texture (const char *texturename)
{
	REF_PTR_RELEASE (Texture);
	Texture = WW3DAssetManager::Get_Instance()->Get_Texture (texturename);
}


/***********************************************************************************************
 * SkyObjectClass::Render --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyObjectClass::Render()
{
	if (Is_Visible()) {

		// Copy the vertices into the dynamic vertex buffer.
		// NOTE: Vertex normals and stage 1 UV's are uninitialized.
		DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, VertexCount);
		{
			DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);
			VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

			for (unsigned v = 0; v < VertexCount; v++) {
				vertex->x		 = VertexArray [v].X;
				vertex->y		 = VertexArray [v].Y;
				vertex->z		 = VertexArray [v].Z;
				vertex->diffuse = DiffuseArray [v];
				vertex->u1		 = TexCoordArray [v].X;
				vertex->v1		 = TexCoordArray [v].Y;
				vertex++;
			}
		}

		DX8Wrapper::Set_Texture (0, Texture);
		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);
		DX8Wrapper::Set_Vertex_Buffer (dynamicvb);
		DX8Wrapper::Draw_Triangles (0, TriangleCount, 0, VertexCount);
	}
}


/***********************************************************************************************
 * CloudLayerClass::CloudLayerClass --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
CloudLayerClass::CloudLayerClass (float maxdistance, const char *texturename, const Vector2 &velocity, float tilefactor, bool rotate)
	: RowCount (4)
{
	const Vector3  white (1.0f, 1.0f, 1.0f);
	const Vector3  black (0.0f, 0.0f, 0.0f);

	const float		radius				  = maxdistance;
	const float		maxlongitude		  = WWMATH_PI / 15;
	const float		oosinmaxlongitude	  = 1.0f / sinf (maxlongitude);
	const float		oorowcount			  = 1.0f / RowCount;
	const unsigned	segmentcount		  = 16;
	const float		twopioosegmentcount = (1.0f / segmentcount) * 2.0f * WWMATH_PI;

	unsigned			row, segment;
	unsigned			v;
	unsigned short i;
	float				longitude, latitude;
	float				x, y, z;
	float				scale;

	VertexCount	  = (RowCount + 1) * segmentcount;
	TriangleCount = segmentcount * (RowCount * 2);

	// Define vertices.
	VertexArray = new Vector3 [VertexCount];
	WWASSERT (VertexArray != NULL);
	v = 0;
	for (segment = 0; segment < segmentcount; segment++) {
		latitude = segment * twopioosegmentcount;
		for (row = 0; row < RowCount + 1; row++) {
			longitude = (sinf (row * oorowcount * maxlongitude) * oosinmaxlongitude) * row * oorowcount * maxlongitude;
			x = radius * sinf (longitude) * cosf (latitude);
			y = radius * sinf (longitude) * sinf (latitude);
			z = radius * (cosf (longitude) - cosf (maxlongitude));
			VertexArray [v].Set (x, y, z);
			v++;
		}
	}

	// Define triangles.
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (TriangleCount * VERTICES_PER_TRIANGLE));
	{
		DX8IndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		i = 0;
		for (row = 0; row < RowCount; row++) {
			v = row;
			for (segment = 0; segment < segmentcount; segment++) {

				// Is this not the last segment?
				if (segment < segmentcount - 1) {
					indices [i + 0] = v;
					indices [i + 1] = v + RowCount + 1;
					indices [i + 2] = v + RowCount + 2;
					indices [i + 3] = v + RowCount + 2;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				} else {
					indices [i + 0] = v;
					indices [i + 1] = row;
					indices [i + 2] = row + 1;
					indices [i + 3] = row + 1;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				}
				v += RowCount + 1;
				i += 6;
			}
		}
	}

	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	Shader = ShaderClass::_PresetAlpha2DShader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);

	Texture = WW3DAssetManager::Get_Instance()->Get_Texture (texturename, TextureClass::MIP_LEVELS_ALL, WW3D_FORMAT_UNKNOWN);
	Texture->Set_U_Addr_Mode (TextureClass::TEXTURE_ADDRESS_REPEAT);
	Texture->Set_V_Addr_Mode (TextureClass::TEXTURE_ADDRESS_REPEAT);

	#if WW3D_DX8
	Set_Texture_Hint (Texture, srTextureIFace::HINT_ALPHA_ONLY);
	#endif

	// Define texture UV's.
	TexCoordArray = new Vector2 [VertexCount];
	WWASSERT (TexCoordArray != NULL);

	scale = tilefactor / (radius * sinf (maxlongitude));
	for (v = 0; v < VertexCount; v++) {
		if (rotate) {
			TexCoordArray [v] = Vector2 (VertexArray [v].Y, VertexArray [v].X) * scale;
		} else {
			TexCoordArray [v] = Vector2 (VertexArray [v].X, VertexArray [v].Y) * scale;
		}
	}

	if (rotate) {
		Velocity.X = velocity.Y;
		Velocity.Y = velocity.X;
  	} else {
		Velocity = velocity;
	}
	Velocity	*= tilefactor;

	DiffuseArray = new unsigned [VertexCount];
	WWASSERT (DiffuseArray != NULL);

	WarmColor		  = white;
	ColdColor		  = white;
	WarmDirection	  = Vector3 (1.0f, 0.0f, 0.0f);
	Alpha				  = 1.0f;
	CloudIntensity	  = 1.0f;
	HorizonIntensity = 1.0f;
	Configure();
}


/***********************************************************************************************
 * CloudLayerClass::~CloudLayerClass --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
CloudLayerClass::~CloudLayerClass()
{
	delete [] DiffuseArray;
	delete [] TexCoordArray;
	REF_PTR_RELEASE (Texture);
	REF_PTR_RELEASE (Material);
	REF_PTR_RELEASE (IndexBuffer);
	delete [] VertexArray;
}


/***********************************************************************************************
 * CloudLayerClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Configure (const Vector3 &warmcolor, const Vector3 &coldcolor)
{
	WarmColor = warmcolor;
	ColdColor = coldcolor;
	Configure();
}


/***********************************************************************************************
 * CloudLayerClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Configure (const Vector3 &warmdirection)
{
	WarmDirection = warmdirection;
	Configure();
}


/***********************************************************************************************
 * CloudLayerClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Configure (const Vector3 &warmcolor, const Vector3 &coldcolor, const Vector3 &warmdirection)
{
	WarmColor	  = warmcolor;
	ColdColor	  = coldcolor;
	WarmDirection = warmdirection;
	Configure();
}


/***********************************************************************************************
 * CloudLayerClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Configure (float alpha, float cloudintensity, float horizonintensity)
{
	Alpha				  = alpha;
	CloudIntensity	  = cloudintensity;
	HorizonIntensity = horizonintensity;
	Configure();
}


/***********************************************************************************************
 * CloudLayerClass::Configure --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Configure()
{
	const Vector3 cap (1.0f, 1.0f, 1.0f);

	Vector3 warmcloudcolor, coldcloudcolor, warmhorizoncolor, coldhorizoncolor;

	warmcloudcolor	  = WarmColor * CloudIntensity;
	coldcloudcolor	  = ColdColor * CloudIntensity;
	warmhorizoncolor = WarmColor * HorizonIntensity;
	coldhorizoncolor = ColdColor * HorizonIntensity;

	for (unsigned v = 0; v < VertexCount; v++) {

		float	  interpolant, alpha;
		Vector3 n, color;

		n = Normalize (VertexArray [v]);
		interpolant = Vector3::Dot_Product (WarmDirection, n);
		if (interpolant < 0.0f) interpolant = 0.0f;
		if (v % (RowCount + 1) < RowCount) {
			Vector3::Lerp (coldcloudcolor, warmcloudcolor, interpolant, &color);
			alpha = Alpha;
		} else {
			Vector3::Lerp (coldhorizoncolor, warmhorizoncolor, interpolant, &color);
			alpha = 0.0f;
		}
		DiffuseArray [v] = DX8Wrapper::Convert_Color (color, alpha);
	}

	// Optimization: If alpha is zero then cloud layer is invisible.
	Set_Visibility (Alpha > 0.0f);
}


/***********************************************************************************************
 * CloudLayerClass::Render --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void CloudLayerClass::Render()
{
	const float wraplimit = 8.0f;

	Vector2 offset;

	offset = (WW3D::Get_Frame_Time() * 0.001f) * Velocity;

	// Keep the texture coordinates within certain bounds to avoid floating point overflow
	// and to avoid potential hardware problems associated with large +ve/-ve coordinates.
	// NOTE: Wrap limit has been chosen arbitrarily to allow for enough texture wrapping.
	if (TexCoordArray [0].X < -wraplimit) {
		offset.X += wraplimit;
	} else {
		if (TexCoordArray [0].X > wraplimit) {
			offset.X -= wraplimit;
		}
	}
	if (TexCoordArray [0].Y < -wraplimit) {
		offset.Y += wraplimit;
	} else {
		if (TexCoordArray [0].Y > wraplimit) {
			offset.Y -= wraplimit;
		}
	}

	for (unsigned v = 0; v < VertexCount; v++) {
		TexCoordArray [v] += offset;
	}

	if (Is_Visible()) {

		// Copy the vertices into the dynamic vertex buffer.
		// NOTE: Vertex normals and stage 1 UV's are uninitialized.
		DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, VertexCount);
		{
			DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);
			VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

			for (unsigned v = 0; v < VertexCount; v++) {
				vertex->x = VertexArray [v].X;
				vertex->y = VertexArray [v].Y;
				vertex->z = VertexArray [v].Z;
				vertex->diffuse = DiffuseArray [v];
				vertex->u1 = TexCoordArray [v].X;
				vertex->v1 = TexCoordArray [v].Y;
				vertex++;
			}
		}

		DX8Wrapper::Set_Texture (0, Texture);
		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);
		DX8Wrapper::Set_Vertex_Buffer (dynamicvb);
		DX8Wrapper::Draw_Triangles (0, TriangleCount, 0, VertexCount);
	}
}


/***********************************************************************************************
 * SkyGlowClass::SkyGlowClass -- Constructor																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyGlowClass::SkyGlowClass (float radius)
	: RowCount (ROW_COUNT),
	  Radius (radius)
{
	const Vector3	white	(1.0f, 1.0f, 1.0f);
	const float		longitude [ROW_COUNT + 1] = {1.37f, 1.42f, 1.47f, 1.52f, 1.57f, 1.92f};
	const unsigned	segmentcount				  = 32;
	const float		twopioosegmentcount		  = (1.0f / segmentcount) * 2.0f * WWMATH_PI;

	unsigned			row, segment;
	unsigned			v;
	float				latitude;
	float				x, y, z;
	unsigned short i;

	VertexCount	  = (RowCount + 1) * segmentcount;
	TriangleCount = segmentcount * (RowCount * 2);

	MinZ = 0.0f;
	MaxZ = radius * cosf (longitude [0]);

	// Define vertices.
	VertexArray = new Vector3 [VertexCount];
	WWASSERT (VertexArray != NULL);
	v = 0;
	for (segment = 0; segment < segmentcount; segment++) {
		latitude = segment * twopioosegmentcount;
		for (row = 0; row < RowCount + 1; row++) {
			x = radius * sinf (longitude [row]) * cosf (latitude);
			y = radius * sinf (longitude [row]) * sinf (latitude);
			z = radius * cosf (longitude [row]);
			VertexArray [v].Set (x, y, z);
			v++;
		}
	}

	// Define triangles.
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (TriangleCount * VERTICES_PER_TRIANGLE));
	{
		DX8IndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		i = 0;
		for (row = 0; row < RowCount; row++) {
			v = row;
			for (segment = 0; segment < segmentcount; segment++) {

				// Is this not the last segment?
				if (segment < segmentcount - 1) {
					indices [i + 0] = v;
					indices [i + 1] = v + RowCount + 1;
					indices [i + 2] = v + RowCount + 2;
					indices [i + 3] = v + RowCount + 2;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				} else {
					indices [i + 0] = v;
					indices [i + 1] = row;
					indices [i + 2] = row + 1;
					indices [i + 3] = row + 1;
					indices [i + 4] = v + 1;
					indices [i + 5] = v;
				}
				v += RowCount + 1;
				i += 6;
			}
		}
	}

	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	Shader = ShaderClass::_PresetAdditive2DShader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Texturing (ShaderClass::TEXTURING_DISABLE);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);

	DiffuseArray = new unsigned [VertexCount];
	WWASSERT (DiffuseArray != NULL);

	Configure (Vector2 (1.0f, 0.0f), white, 1.0f);
}


/***********************************************************************************************
 * SkyGlowClass::~SkyGlowClass -- Destructor																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyGlowClass::~SkyGlowClass()
{
	// Clean-up.
	delete [] DiffuseArray;
	REF_PTR_RELEASE (Material);
	REF_PTR_RELEASE (IndexBuffer);
	delete [] VertexArray;
}


/***********************************************************************************************
 * SkyGlowClass::Configure --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyGlowClass::Configure (const Vector2 &hotdirection, const Vector3 &horizoncolor, float coldintensity)
{
	HotDirection  = Normalize (hotdirection);
	HorizonColor  = horizoncolor;
	ColdIntensity = coldintensity;
	Configure();
}


/***********************************************************************************************
 * SkyGlowClass::Configure --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyGlowClass::Configure()
{
	const float	  ooradius = 1.0f / Radius;
	const float	  exponent = 7.0f;
	const float	  oomaxz	  = 1.0f / MaxZ;
	const Vector3 hotdirection (HotDirection.X, HotDirection.Y, 0.0f);

	for (unsigned v = 0; v < VertexCount; v++) {

		float	dotproduct, gradient, z;

		dotproduct = Vector3::Dot_Product (hotdirection, VertexArray [v] * ooradius);
		dotproduct = pow (dotproduct, exponent);
		if (dotproduct < ColdIntensity) dotproduct = ColdIntensity;

		z = VertexArray [v].Z;
		if (z  < MinZ) {
			gradient = 1.0f;
		} else {
			gradient = 1.0f - (z * oomaxz);
		}

		DiffuseArray [v] = DX8Wrapper::Convert_Color (HorizonColor * (dotproduct * gradient), 1.0f);
	}

	// Sky glow is always visible.
	Set_Visibility (true);
}


/***********************************************************************************************
 * SkyGlowClass::Render --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyGlowClass::Render()
{
	if (Is_Visible()) {

		// Copy the vertices into a dynamic vertex buffer.
		// NOTE: Vertex normals and UV's are uninitialized.
		DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, VertexCount);
		{
			DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);
			VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

			for (unsigned v = 0; v < VertexCount; v++) {
				vertex->x		 = VertexArray [v].X;
				vertex->y		 = VertexArray [v].Y;
				vertex->z		 = VertexArray [v].Z;
				vertex->diffuse = DiffuseArray [v];
				vertex++;
			}
		}

		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);
		DX8Wrapper::Set_Vertex_Buffer (dynamicvb);
		DX8Wrapper::Draw_Triangles (0, TriangleCount, 0, VertexCount);
	}
}


/***********************************************************************************************
 * LightningBoltClass::LightningBoltClass -- Constructor													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
LightningBoltClass::LightningBoltClass (int branchcount, Matrix3D &m, float length, float childlength, float width, float amplitude)
	: Branches (NULL)
{
	const int		 randomness					  = 100;
	const float		 oorandomness				  = 1.0f / randomness;
	const unsigned  maxvertexcount			  = MAX (2, length * 0.5f);
	const float		 oomaxvertexcountminusone = 1.0f / (maxvertexcount - 1);
	const char		*texturename				  = "LightningBolt.tga";

	Vector3		  white (1.0f, 1.0f, 1.0f);
	unsigned		  vertexcount;
	float			  x;
	Vector3		 *localvertex, *worldvertex;
	int			  branchrandomness;
	TextureClass *texture;

 	localvertex = new Vector3 [maxvertexcount];
	worldvertex = new Vector3 [maxvertexcount];

	vertexcount = 0;
	x = 0;
	for (unsigned v = 0; v < maxvertexcount; v++) {
		vertexcount++;
		localvertex [v] = Vector3 (x, -oomaxvertexcountminusone * v * length, 0.0f);
		worldvertex [v] = m * localvertex [v];
		x = x + _RandomNumber (-randomness, +randomness) * 0.5f * oorandomness * amplitude;
	}

	Set_Points (vertexcount, worldvertex);
	texture = WW3DAssetManager::Get_Instance()->Get_Texture (texturename, TextureClass::MIP_LEVELS_3);
	Set_Texture (texture);
	REF_PTR_RELEASE (texture);

	Set_Shader (ShaderClass::_PresetAdditive2DShader);
	Set_Width (width);
	Set_Texture_Mapping_Mode (SegLineRendererClass::UNIFORM_WIDTH_TEXTURE_MAP);

	// Optimization: Merging intersections yields no visual improvement.
	Set_Merge_Intersections (false);

	// Optimization: Sorting is not required because all of the line segments are coplanar.
	Set_Disable_Sorting (true);

	BranchCount = (vertexcount == maxvertexcount) ? branchcount : 0;

	// Create the child lightning bolts.
	if (BranchCount > 0) {

		const float	minbranchangle  = WWMATH_PI * 0.167f;
		const float	maxbranchangle  = WWMATH_PI * 0.223f;
		const float	branchfactor	 = 0.50f;
		const	float	minlengthfactor = 0.20f;
		const float maxlengthfactor = 0.45f;
		const	float	widthfactor		 = 0.30f;
		const float	minwidth			 = 0.13f;
		const float amplitudefactor = 0.85f;

		float	oobranchcount;
		int	lbbranchcount;
		float	minlength, maxlength, w, a;

		Branches = new BranchStruct [BranchCount];
		WWASSERT (Branches != NULL);
		oobranchcount = 1.0f / BranchCount;
		branchrandomness = vertexcount / (BranchCount * 2);
		lbbranchcount = BranchCount * branchfactor;
		minlength = childlength * minlengthfactor;
		maxlength = childlength * maxlengthfactor;
		w = MAX (minwidth, width * widthfactor);
		a = amplitude * amplitudefactor;
		for (int b = 0; b < BranchCount; b++) {

			float		angle, l;
			unsigned v;
			Matrix3D m0 (m);

			angle = WWMath::Lerp (minbranchangle, maxbranchangle, _RandomNumber (0, randomness) * oorandomness);
			if ((b & 0x1) == 0) angle = -angle;
			l = WWMath::Lerp (minlength, maxlength, _RandomNumber (0, randomness) * oorandomness);
			v = MIN (((int) vertexcount) - 1, b * oobranchcount * (((int) vertexcount) - 1) + _RandomNumber (0, +branchrandomness));
			m0.Translate (localvertex [v] - localvertex [0]);
			m0.Rotate_Z (angle);

			// If the lightning bolt originates below the horizon don't bother generating it.
			if (m0.Get_Translation().Z > 0.0f) {
				Branches [b].LightningBolt = NEW_REF (LightningBoltClass, (lbbranchcount, m0, l, l, w, a));
			} else {
				Branches [b].LightningBolt = NULL;
			}

			WWASSERT (Branches != NULL);
		}
	}

	Configure (white);

	// Clean-up.
	delete [] localvertex;
	delete [] worldvertex;
}


/***********************************************************************************************
 * LightningBoltClass::~LightningBoltClass -- Destructor													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
LightningBoltClass::~LightningBoltClass()
{
	for (int b = 0; b < BranchCount; b++) {
		REF_PTR_RELEASE (Branches [b].LightningBolt);
	}

	if (Branches != NULL) {
		delete Branches;
	}
}


/***********************************************************************************************
 * LightningBoltClass::Set_Visibility --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightningBoltClass::Set_Visibility (bool visible, bool recurse)
{
	VisibilityClass::Set_Visibility (visible);
	if (recurse) {
		for (int b = 0; b < BranchCount; b++) {
			if (Branches [b].LightningBolt != NULL) {
				Branches [b].LightningBolt->Set_Visibility (visible, recurse);
			}
		}
	}
}


/***********************************************************************************************
 * LightningBoltClass::Configure --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightningBoltClass::Configure (Vector3 &color)
{
	Set_Color (color);
	for (int b = 0; b < BranchCount; b++) {
		if (Branches [b].LightningBolt != NULL) {
			Branches [b].LightningBolt->Configure (color);
		}
	}
}


/***********************************************************************************************
 * LightningBoltClass::Set_Transform --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightningBoltClass::Set_Transform (Matrix3D &t)
{
	((SegmentedLineClass*) this)->Set_Transform (t);
	for (int b = 0; b < BranchCount; b++) {
		if (Branches [b].LightningBolt != NULL) {
			Branches [b].LightningBolt->Set_Transform (t);
		}
	}
}


/***********************************************************************************************
 * LightningBoltClass::Render --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightningBoltClass::Render (RenderInfoClass &rinfo)
{
	if (VisibilityClass::Is_Visible()) {
		if (!rinfo.Camera.Cull_Sphere (Get_Bounding_Sphere())) {

			Matrix4 t;

			// NOTE: Copy and restore the object to world space transform because Render_Seg_Line()
			// will modify it (because it is a render-object function).
			DX8Wrapper::Get_Transform (D3DTS_WORLD, t);
			SegmentedLineClass::Render_Seg_Line (rinfo);
			DX8Wrapper::Set_Transform (D3DTS_WORLD, t);
		}
		for (int b = 0; b < BranchCount; b++) {
			if (Branches [b].LightningBolt != NULL) {
				Branches [b].LightningBolt->Render (rinfo);
			}
		}
	}
}


/***********************************************************************************************
 * LightningClass::LightningClass -- Constructor															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
LightningClass::LightningClass (float extent, float startdistance, float enddistance, float heading, float distribution)
	: Time (0),
	  PlayedThunder (false)
{
	const Vector3  white (1.0f, 1.0f, 1.0f);

	const unsigned randomness						 = 100;
	const float		oorandomness					 = 1.0f / randomness;
	const unsigned minthunderdelaytime			 = 0;
	const unsigned maxthunderdelaytime			 = 1000;
	const float		minlongitude					 = WWMATH_PI * 0.340f;
	const float		maxlongitude					 = WWMATH_PI * 0.472f;
	const float		bufferangle						 = 0.35f;
	const int	   minbranchcount					 = 5;
	const int	   maxbranchcount					 = 8;
	const float		minwidth							 = 0.5f;
	const float	   maxwidth							 = 1.1f;
	const float		amplitude						 = 1.5f;
	const char	  *lightningsourcetexturename  = "LightningSource.tga";
	const float	   minlightningsourcewidth		 = 1.00f;
	const float	   maxlightningsourcewidth		 = 3.00f;
	const float	   minlightningsourceheight	 = 0.35f;
	const float	   maxlightningsourceheight	 = 1.05f;
	const unsigned majorsamplecount				 = 2;
	const unsigned minorsamplecount				 = 3;

	static const char	*_thundersamplename [majorsamplecount][minorsamplecount] = {
		{"Thunder01", "Thunder02", "Thunder03"},
		{"Thunder04", "Thunder05", "Thunder06"}
	};

	unsigned	branchcount;
	float		latitude, longitude, theta, x, y, z, length, childlength, width;
	Vector3	d, o;
	Matrix3D	m;
	float		lightningsourcewidth, lightningsourceheight;
	unsigned	majorsampleindex, minorsampleindex;

	Distance			  = WWMath::Lerp (startdistance, enddistance, _RandomNumber (0, randomness) * oorandomness);
	ThunderDelayTime = WWMath::Lerp ((float) minthunderdelaytime, (float) maxthunderdelaytime, Distance);

	LightningGlow = new SkyGlowClass (extent);
	WWASSERT (LightningGlow != NULL);

	branchcount = WWMath::Lerp ((float) maxbranchcount, (float) minbranchcount, Distance);
	latitude = heading + (0.5f * WWMATH_PI) + (_RandomNumber (- ((int) randomness), ((int) randomness)) * oorandomness * WWMATH_PI * distribution);
	x = cosf (latitude);
	y = sinf (latitude);
	Direction.Set (x, y);
	d.Set (x, y, 0.0f);
	longitude = WWMath::Lerp (minlongitude, maxlongitude, Distance);
	x = sinf (longitude) * cosf (latitude);
	y = sinf (longitude) * sinf (latitude);
	z = cosf (longitude);
	o.Set (x, y, z);
	m.Look_At (o * extent, (o + d) * extent, 0.0f);
	theta   = (WWMATH_PI * 0.5f) - longitude;
	childlength = extent * sinf (theta);
	length = childlength + extent * cosf (theta) * tanf (bufferangle);
	width	 = WWMath::Lerp (maxwidth, minwidth, Distance);
	LightningBolt = NEW_REF (LightningBoltClass, (branchcount, m, length, childlength, width, amplitude));
	WWASSERT (LightningBolt != NULL);

	LightningSource = new SkyObjectClass (ShaderClass::_PresetAdditive2DShader);
	WWASSERT (LightningSource != NULL);
	lightningsourcewidth  = WWMath::Lerp (maxlightningsourcewidth,  minlightningsourcewidth,  Distance);
	lightningsourceheight = WWMath::Lerp (maxlightningsourceheight, minlightningsourceheight, Distance);
	LightningSource->Configure (o, extent, lightningsourcewidth, lightningsourceheight);
	LightningSource->Set_Texture (lightningsourcetexturename);

	ThunderPosition	= o * (extent * Distance);
	majorsampleindex	= MIN ((unsigned) (Distance * majorsamplecount), majorsamplecount - 1);
	minorsampleindex	= _RandomNumber (0, minorsamplecount - 1);
	ThunderSampleName = _thundersamplename [majorsampleindex][minorsampleindex];
}


/***********************************************************************************************
 * LightningClass::~LightningClass -- Destructor															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
LightningClass::~LightningClass()
{
	delete LightningSource;
	REF_PTR_RELEASE (LightningBolt);
	delete LightningGlow;
}


/***********************************************************************************************
 * LightningClass::Update --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool LightningClass::Update (Matrix3D &t, Vector3 &additivecolor, SoundEnvironmentClass *soundenvironment)
{
	const Vector3  black (0.000f, 0.000f, 0.000f);
	const Vector3  blue  (0.663f, 0.750f, 0.969f);
	const Vector3  gray  (0.350f, 0.350f, 0.350f);
	const Vector3  white (1.000f, 1.000f, 1.000f);
	const unsigned lightningtime	  = 550;
	const float		minglowintensity = 0.10f;
	const float		coldintensity	  = 0.25f;

	bool notfinished = true;

	// Set the lightning bolt transform so that render-object culling will work correctly.
	LightningBolt->Set_Transform (t);

	Time += WW3D::Get_Frame_Time();
	if (Time < lightningtime) {

  		const unsigned phasecount = 7;

  		const static float _intensities [phasecount]		= {1.00f, 0.75f, 0.50f, 0.50f, 1.00f, 0.50f, 0.10f};
  		const static bool  _renderbranches [phasecount] = {true,  true,  true,  true,  false, false, false};

  		unsigned phase;
  		Vector3	color;

  		phase = (((float) Time) / lightningtime) * (phasecount + 1);
  		phase = MIN (phase, phasecount);

  		additivecolor = blue * _intensities [phase] * MAX (1.0f - Distance, minglowintensity);
		LightningGlow->Configure (Direction, additivecolor, coldintensity);
		additivecolor *= coldintensity;

  		color = gray * _intensities [phase];
  		LightningBolt->Configure (color);
  		if (_renderbranches [phase]) {
  			LightningBolt->Set_Visibility (true, true);
  		} else {
  			LightningBolt->Set_Visibility (false, true);
  			LightningBolt->Set_Visibility (true, false);
  		}

  		color = white * _intensities [phase];
  		LightningSource->Configure (color);
  		LightningSource->Set_Visibility (true);

  	} else {

  		additivecolor = black;
  		if (Time < lightningtime + ThunderDelayTime) {

  			LightningGlow->Set_Visibility (false);
  			LightningBolt->Set_Visibility (false, false);
  			LightningSource->Set_Visibility (false);

  		} else {

  			LightningGlow->Set_Visibility (false);
  			LightningBolt->Set_Visibility (false, false);
  			LightningSource->Set_Visibility (false);
  			if (!PlayedThunder) {

  				AudibleSoundClass *sound;

  				sound = WWAudioClass::Get_Instance()->Create_Sound (ThunderSampleName, NULL, 0, CLASSID_PSEUDO3D);
  				if (sound != NULL) {

  					Matrix3D	m (t);

  					m.Adjust_Translation (ThunderPosition);

  					sound->Set_Transform (m);
  					sound->Add_To_Scene();
  					sound->Set_Volume (soundenvironment->Get_Amplitude() * 0.5f);
  					sound->Play();
  					sound->Release_Ref();
  				}
  				PlayedThunder = true;
  			}
  			notfinished = false;
  		}
  	}

	return (notfinished);
}


/***********************************************************************************************
 * LightningClass::Render --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void LightningClass::Render (RenderInfoClass &rinfo)
{
	LightningGlow->Render();
	LightningBolt->Render (rinfo);
	LightningSource->Render();
}


/***********************************************************************************************
 * WarBlitzClass::WarBlitzClass -- Constructor																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/26/01    IML : Created.                                                                *
 *=============================================================================================*/
WarBlitzClass::WarBlitzClass (float extent, float startdistance, float enddistance, float heading, float distribution)
	: Time (0),
	  PlayedSample (false)
{
	const unsigned randomness			 = 100;
	const float		oorandomness		 = 1.0f / randomness;
	const unsigned minsampledelaytime = 25;
	const unsigned maxsampledelaytime = 400;

	float	  latitude, x, y;
	Vector2 position;

	Distance			 = WWMath::Lerp (startdistance, enddistance, _RandomNumber (0, randomness) * oorandomness);
	SampleDelayTime = WWMath::Lerp ((float) minsampledelaytime, (float) maxsampledelaytime, Distance);

	latitude = heading + (0.5f * WWMATH_PI) + (_RandomNumber (- ((int) randomness), ((int) randomness)) * oorandomness * WWMATH_PI * distribution);
	x = cosf (latitude);
	y = sinf (latitude);
	Direction.Set (x, y);

	position = Direction * (extent * Distance);
	SamplePosition.Set (position.X, position.Y, 0.0f);

	WarBlitzGlow = new SkyGlowClass (extent);
	WWASSERT (WarBlitzGlow != NULL);
}


/***********************************************************************************************
 * WarBlitzClass::~WarBlitzClass -- Destructor																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/26/01    IML : Created.                                                                *
 *=============================================================================================*/
WarBlitzClass::~WarBlitzClass()
{
	delete WarBlitzGlow;
}


/***********************************************************************************************
 * WarBlitzClass::Update --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/26/01    IML : Created.                                                                *
 *=============================================================================================*/
bool WarBlitzClass::Update (Matrix3D &t, Vector3 &additivecolor)
{
	const Vector3  red   (0.950f, 0.250f, 0.250f);
	const Vector3  black (0.000f, 0.000f, 0.000f);
	const unsigned warblitztime = 550;
	const float		minglowintensity = 0.10f;
	const float		coldintensity	  = 0.25f;

	bool notfinished = true;

	Time += WW3D::Get_Frame_Time();
	if (Time < warblitztime) {

  		const unsigned		  phasecount = 7;
		const static float _intensities [phasecount]	= {0.50f, 1.00f, 0.80f, 0.60f, 0.40f, 0.20f, 0.10f};

  		unsigned phase;

  		phase = (((float) Time) / warblitztime) * (phasecount + 1);
  		phase = MIN (phase, phasecount);
  		additivecolor = red * _intensities [phase] * MAX (1.0f - Distance, minglowintensity);
		WarBlitzGlow->Configure (Direction, additivecolor, coldintensity);

  	} else {

		additivecolor = black;
		WarBlitzGlow->Set_Visibility (false);
		if (Time >= warblitztime + SampleDelayTime) {
			if (!PlayedSample) {

				const char *samplename = "SFX.Ambient_Explosion_01";

			 	AudibleSoundClass *sound;

  				sound = WWAudioClass::Get_Instance()->Create_Sound (samplename, NULL, 0, CLASSID_PSEUDO3D);
  				if (sound != NULL) {

  					Matrix3D	m (t);

  					m.Adjust_Translation (SamplePosition);
  					sound->Set_Transform (m);
  					sound->Add_To_Scene();
  					sound->Play();
  					sound->Release_Ref();
  				}
  				PlayedSample = true;
			}
	  		notfinished = false;
		}
	}

	additivecolor *= coldintensity;
	return (notfinished);
}


/***********************************************************************************************
 * WarBlitzClass::Render --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/26/01    IML : Created.                                                                *
 *=============================================================================================*/
void WarBlitzClass::Render (RenderInfoClass &/* rinfo */)
{
	WarBlitzGlow->Render();
}


/***********************************************************************************************
 * SkyClass::SkyClass -- Constructor																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyClass::SkyClass (SoundEnvironmentClass *soundenvironment)
	: Extent (100.0f),
	  Color (0.0f, 0.0f, 0.0f),
	  Hours (0),
	  Minutes (0),
	  Gloominess (0.0f),
	  TintFactor (0.0f),
	  LightningIntensity (0.0f),
	  LightningStartDistance (0.0f),
	  LightningEndDistance (1.0f),
	  LightningHeading (0.0f),
	  LightningDistribution (0.5f),
	  SoundEnvironment (NULL),
	  WarBlitzIntensity (0.0f),
	  WarBlitzStartDistance (0.0f),
	  WarBlitzEndDistance (1.0f),
	  WarBlitzHeading (0.0f),
	  WarBlitzDistribution (0.5f)
{
	const unsigned starcount = 200;

	WWASSERT (soundenvironment != NULL);
	REF_PTR_SET (SoundEnvironment, soundenvironment);
	SoundEnvironment->Add_User();

	Haze = new HazeClass (Extent);
	WWASSERT (Haze != NULL);

	Starfield = new StarfieldClass (Extent, starcount);
	WWASSERT (Starfield != NULL);

	Sun = new SkyObjectClass (ShaderClass::_PresetAdditive2DShader);
	WWASSERT (Sun != NULL);
	Sun->Set_Texture ("Sun.tga");

	SunHalo = new SkyObjectClass (ShaderClass::_PresetAdditive2DShader);
	WWASSERT (SunHalo != NULL);
	SunHalo->Set_Texture ("SunHalo.tga");

	Moon = new SkyObjectClass (ShaderClass::_PresetAlpha2DShader);
	WWASSERT (Moon != NULL);
	Set_Moon_Type (MOON_TYPE_FULL);

	MoonHalo = new SkyObjectClass (ShaderClass::_PresetAdditive2DShader);
	WWASSERT (MoonHalo != NULL);
	MoonHalo->Set_Texture ("MoonHalo.tga");

	CloudLayer0 = new CloudLayerClass (Extent, "CloudLayer.tga", Vector2 (0.0030f, 0.0006f), 1.2f, false);
	WWASSERT (CloudLayer0 != NULL);

	CloudLayer1 = new CloudLayerClass (Extent, "CloudLayer.tga", Vector2 (0.0050f, 0.0010f), 0.8f, true);
	WWASSERT (CloudLayer1 != NULL);

	for (unsigned l = 0; l < LIGHTNING_COUNT; l++) {
		Lightning [l]			  = NULL;
		LightningCountdown [l] = Lightning_Delay();
	}

	WarBlitz				= NULL;
	WarBlitzCountdown = War_Blitz_Delay();
}


/***********************************************************************************************
 * SkyClass::~SkyClass -- Destructor																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
SkyClass::~SkyClass()
{
	// Clean-up.
	if (WarBlitz != NULL) delete WarBlitz;
	for (unsigned l = 0; l < LIGHTNING_COUNT; l++) {
		if (Lightning [l] != NULL) delete Lightning [l];
	}
	delete CloudLayer1;
	delete CloudLayer0;
	delete MoonHalo;
	delete Moon;
	delete SunHalo;
	delete Sun;
	delete Starfield;
	delete Haze;

	SoundEnvironment->Remove_User();
	REF_PTR_RELEASE (SoundEnvironment);
}


/***********************************************************************************************
 * SkyClass::Set_Light_Direction --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Light_Direction (const Vector3 &sundirection, const Vector3 &moondirection)
{
	const float	sunradius		= 30.00f;
	const float	sunhaloradius	= 85.00f;
	const float	moonradius		=  5.00f;
	const float	moonhaloradius	= 20.00f;

	Starfield->Configure (moondirection, Extent, moonradius);
	Sun->Configure (sundirection, Extent, sunradius);
	SunHalo->Configure (sundirection, Extent, sunhaloradius);
	Moon->Configure (moondirection, Extent, moonradius);
	MoonHalo->Configure (moondirection, Extent, moonhaloradius);
	CloudLayer0->Configure (sundirection);
	CloudLayer1->Configure (sundirection);
}


/***********************************************************************************************
 * SkyClass::Set_Time_Of_Day --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Time_Of_Day (unsigned hours, unsigned minutes)
{
	const Vector3 white			  (1.00f, 1.00f, 1.00f);
	const Vector3 starfieldcolor (0.27f, 0.31f, 0.37f);

	const unsigned colorcount = 24;
	const unsigned alphacount = 24;

	static const unsigned char _warmskycolors [colorcount][3] = {

	{ 36,  36,  40},	// 0000 hrs
	{ 36,  36,  40},	// 0100 hrs
	{ 36,  36,  40},	//	0200 hrs
	{ 36,  36,  40},	//	0300 hrs
	{ 51,  83, 100},	//	0400 hrs
	{168, 160, 216},	//	0500 hrs
	{240, 212, 216},	//	0600 hrs
	{255, 255, 192},	//	0700 hrs
	{255, 255, 248},	//	0800 hrs
	{255, 255, 248},	//	0900 hrs
	{255, 255, 248},	//	1000 hrs
	{255, 255, 248},	//	1100 hrs
	{255, 255, 248},	//	1200 hrs
	{255, 255, 248},	//	1300 hrs
	{255, 255, 248},	//	1400 hrs
	{255, 255, 248},	//	1500 hrs
	{245, 243, 200},	//	1600 hrs
	{245, 209, 121},	//	1700 hrs
	{229, 188,  88},	//	1800 hrs
	{201, 103,  60},	// 1900 hrs
	{ 72,  72,  80},	//	2000 hrs
	{ 36,  36,  40},	//	2100 hrs
	{ 36,  36,  40},	//	2200 hrs
	{ 36,  36,  40}};	//	2300 hrs

	static const unsigned char _coldskycolors [colorcount][3] = {

	{ 36,  36,  40},	// 0000 hrs
	{ 36,  36,  40},	// 0100 hrs
	{ 36,  36,  40},	//	0200 hrs
	{ 36,  36,  40},	//	0300 hrs
	{ 51,  83, 100},	//	0400 hrs
	{170, 164, 205},	//	0500 hrs
	{200, 179, 182},	//	0600 hrs
	{215, 215, 181},	//	0700 hrs
	{215, 215, 209},	//	0800 hrs
	{215, 215, 209},	//	0900 hrs
	{215, 215, 209},	//	1000 hrs
	{215, 215, 209},	//	1100 hrs
	{215, 215, 209},	//	1200 hrs
	{215, 215, 209},	//	1300 hrs
	{215, 215, 209},	//	1400 hrs
	{215, 215, 209},	//	1500 hrs
	{210, 209, 176},	//	1600 hrs
	{213, 190, 135},	//	1700 hrs
	{214, 187, 123},	//	1800 hrs
	{179, 123,  98},	//	1900 hrs
	{ 72,  72,  80},	//	2000 hrs
	{ 36,  36,  40},	//	2100 hrs
	{ 36,  36,  40},	//	2200 hrs
	{ 36,  36,  40}};	//	2300 hrs

	static const unsigned char _suncolors [colorcount][3] = {

	{  0,   0,   0},	// 0000 hrs
	{  0,   0,   0},	// 0100 hrs
	{  0,   0,   0},	//	0200 hrs
	{  0,   0,   0},	//	0300 hrs
	{  0,   0,   0},	//	0400 hrs
	{127, 127,   0},	//	0500 hrs
	{127, 127,   0},	//	0600 hrs
	{110, 110,   0},	//	0700 hrs
	{ 96,  96,  32},	//	0800 hrs
	{ 76,  76,  76},	//	0900 hrs
	{ 76,  76,  76},	//	1000 hrs
	{ 76,  76,  76},	//	1100 hrs
	{ 76,  76,  76},	//	1200 hrs
	{ 76,  76,  76},	//	1300 hrs
	{ 76,  76,  76},	//	1400 hrs
	{ 76,  76,  76},	//	1500 hrs
	{ 96,  81,  48},	//	1600 hrs
	{123,  81,  21},	//	1700 hrs
	{127,  32,  32},	//	1800 hrs
	{127,  32,  32},	//	1900 hrs
	{  0,   0,   0},	//	2000 hrs
	{  0,   0,   0},	//	2100 hrs
	{  0,   0,   0},	//	2200 hrs
	{  0,   0,   0}};	//	2300 hrs

	static const unsigned char _sunhalocolors [colorcount][3] = {

	{  0,   0,   0},	// 0000 hrs
	{  0,   0,   0},	// 0100 hrs
	{  0,   0,   0},	//	0200 hrs
	{  0,   0,   0},	//	0300 hrs
	{  0,   0,   0},	//	0400 hrs
	{192, 192, 144},	//	0500 hrs
	{192, 192, 144},	//	0600 hrs
	{192, 192, 144},	//	0700 hrs
	{ 96,  96,  96},	//	0800 hrs
	{ 96,  96,  96},	//	0900 hrs
	{ 96,  96,  96},	//	1000 hrs
	{ 96,  96,  96},	//	1100 hrs
	{ 96,  96,  96},	//	1200 hrs
	{ 96,  96,  96},	//	1300 hrs
	{ 96,  96,  96},	//	1400 hrs
	{ 96,  96,  96},	//	1500 hrs
	{ 96,  96,  96},	//	1600 hrs
	{245, 223, 185},	//	1700 hrs
	{239, 179, 107},	//	1800 hrs
	{239, 179, 107},	//	1900 hrs
	{  0,   0,   0},	//	2000 hrs
	{  0,   0,   0},	//	2100 hrs
	{  0,   0,   0},	//	2200 hrs
	{  0,   0,   0}};	//	2300 hrs

	static const unsigned char _moonhalocolors [colorcount][3] = {

	{192, 192, 192},	// 0000 hrs
	{192, 192, 192},	// 0100 hrs
	{192, 192, 192},	//	0200 hrs
	{192, 192, 192},	//	0300 hrs
	{160, 160, 160},	//	0400 hrs
	{128, 128, 128},	//	0500 hrs
	{ 64,  64,  64},	//	0600 hrs
	{  0,   0,   0},	//	0700 hrs
	{  0,   0,   0},	//	0800 hrs
	{  0,   0,   0},	//	0900 hrs
	{  0,   0,   0},	//	1000 hrs
	{  0,   0,   0},	//	1100 hrs
	{  0,   0,   0},	//	1200 hrs
	{  0,   0,   0},	//	1300 hrs
	{  0,   0,   0},	//	1400 hrs
	{  0,   0,   0},	//	1500 hrs
	{  0,   0,   0},	//	1600 hrs
	{  0,   0,   0},	//	1700 hrs
	{ 64,  64,  64},	//	1800 hrs
	{128, 128, 128},	//	1900 hrs
	{160, 160, 160},	//	2000 hrs
	{192, 192, 192},	//	2100 hrs
	{192, 192, 192},	//	2200 hrs
	{192, 192, 192}};	//	2300 hrs

	static const unsigned char _starfieldalphas [alphacount] = {

	255,	// 0000 hrs
	255,	// 0100 hrs
	255,	//	0200 hrs
	255,	//	0300 hrs
	255,	//	0400 hrs
	  0,	//	0500 hrs
	  0,	//	0600 hrs
	  0,	//	0700 hrs
	  0,	//	0800 hrs
	  0,	//	0900 hrs
	  0,	//	1000 hrs
	  0,	//	1100 hrs
	  0,	//	1200 hrs
	  0,	//	1300 hrs
	  0,	//	1400 hrs
	  0,	//	1500 hrs
	  0,	//	1600 hrs
	  0,	//	1700 hrs
	  0,	//	1800 hrs
	  0,	//	1900 hrs
	255,	//	2000 hrs
	255,	//	2100 hrs
	255,	//	2200 hrs
	255};	//	2300 hrs

	float	  timeofday;
	Vector3 warmskycolor, coldskycolor, sunhalocolor, moonhalocolor, cloudcolor, suncolor;
	float	  starfieldalpha;

	Hours	  = hours;
	Minutes = minutes;
	Set_Color();

	// Calculate haze, sun, moon & cloud colors for time of day.
	timeofday		= hours + (minutes / 60.0f);
	starfieldalpha = Interpolate_Scalar (_starfieldalphas, alphacount, timeofday);
	warmskycolor	= Interpolate_Color (_warmskycolors, colorcount, timeofday);
	coldskycolor	= Interpolate_Color (_coldskycolors, colorcount, timeofday);
	sunhalocolor	= Interpolate_Color (_sunhalocolors, colorcount, timeofday);
	moonhalocolor  = Interpolate_Color (_moonhalocolors, colorcount, timeofday);
	suncolor		   = Interpolate_Color (_suncolors, colorcount, timeofday);

	Haze->Configure (Get_Color(), coldskycolor);
	Starfield->Configure (starfieldcolor, white, starfieldalpha);
	Sun->Configure (suncolor);
	SunHalo->Configure (sunhalocolor);
	Moon->Configure (white);
	MoonHalo->Configure (moonhalocolor);
	CloudLayer0->Configure (warmskycolor, coldskycolor);
	CloudLayer1->Configure (warmskycolor, coldskycolor);
}


/***********************************************************************************************
 * SkyClass::Set_Clouds --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Clouds (float cloudcover, float gloominess)
{
	const float layer0minintensity = 0.60f;
	const float layer0maxintensity = 0.95f;
	const float layer1minintensity = 0.20f;
	const float layer1maxintensity = 0.90f;

	float layer0alpha, layer1alpha, layer0intensity, layer1intensity;

	WWASSERT ((cloudcover >= 0.0f) && (cloudcover <= 1.0f));
	WWASSERT ((gloominess >= 0.0f) && (gloominess <= 1.0f));

	Gloominess = gloominess;
	Set_Color();

	if (cloudcover <= 0.5f) {
		layer0alpha = cloudcover * 2.0f;
		layer1alpha = 0.0f;
	} else {
		layer0alpha = 1.0f;
		layer1alpha = (cloudcover - 0.5f) * 2.0f;
	}

	layer0alpha		 = log10 (cloudcover * 9.0f + 1.0f);
	layer1alpha		 = cloudcover * cloudcover * cloudcover;
	layer0intensity = layer0maxintensity - gloominess * (layer0maxintensity - layer0minintensity);
	layer1intensity = layer1maxintensity - gloominess * (layer1maxintensity - layer1minintensity);

	Haze->Configure (Get_Color(), layer0intensity);
	CloudLayer0->Configure (layer0alpha, layer0intensity, layer0intensity);
	CloudLayer1->Configure (layer1alpha, layer1intensity, layer0intensity);
}


/***********************************************************************************************
 * SkyClass::Set_Tint_Factor --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Tint_Factor (float tintfactor)
{
	TintFactor = tintfactor;
	Set_Color();
	Haze->Configure (Get_Color());
}


/***********************************************************************************************
 * SkyClass::Set_Color --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Color()
{
	const Vector3	tintcolor (1.0f, 0.0f, 0.0f);
	const unsigned colorcount = 24;

	static const unsigned char _skycolors [colorcount][3] = {

	{  4,  12,  18},	// 0000 hrs
	{	4,  12,  18},	// 0100 hrs
	{	4,  12,  18},	//	0200 hrs
	{	4,  12,  18},	//	0300 hrs
	{  8,  24,  40},	//	0400 hrs
	{ 72,  92, 136},	//	0500 hrs
	{120, 120, 128},	//	0600 hrs
	{128, 152, 168},	//	0700 hrs
	{112, 148, 168},	//	0800 hrs
	{112, 148, 176},	//	0900 hrs
	{112, 148, 176},	//	1000 hrs
	{112, 148, 176},	//	1100 hrs
	{112, 148, 176},	//	1200 hrs
	{112, 148, 176},	//	1300 hrs
	{112, 148, 176},	//	1400 hrs
	{112, 146, 176},	//	1500 hrs
	{128, 160, 176},	//	1600 hrs
	{120, 148, 152},	//	1700 hrs
	{160, 132, 112},	//	1800 hrs
	{ 72,  88,  88},	//	1900 hrs
	{  8,  24,  36},	//	2000 hrs
	{	4,  12,  18},	//	2100 hrs
	{	4,  12,  18},	//	2200 hrs
	{	4,  12,  18}};	//	2300 hrs

	static const unsigned char _gloominesscolors [colorcount][3] = {

	{  4,  12,  18},	// 0000 hrs
	{	4,  12,  18},	// 0100 hrs
	{	4,  12,  18},	//	0200 hrs
	{	4,  12,  18},	//	0300 hrs
	{  8,  24,  40},	//	0400 hrs
	{ 72,  92, 136},	//	0500 hrs
	{192, 192, 192},	//	0600 hrs
	{192, 192, 192},	//	0700 hrs
	{192, 192, 192},	//	0800 hrs
	{192, 192, 192},	//	0900 hrs
	{192, 192, 192},	//	1000 hrs
	{192, 192, 192},	//	1100 hrs
	{192, 192, 192},	//	1200 hrs
	{192, 192, 192},	//	1300 hrs
	{192, 192, 192},	//	1400 hrs
	{192, 192, 192},	//	1500 hrs
	{192, 192, 192},	//	1600 hrs
	{192, 192, 192},	//	1700 hrs
	{192, 192, 192},	//	1800 hrs
	{ 72,  88,  88},	//	1900 hrs
	{  8,  24,  36},	//	2000 hrs
	{	4,  12,  18},	//	2100 hrs
	{	4,  12,  18},	//	2200 hrs
	{	4,  12,  18}};	//	2300 hrs

	float	  timeofday, interpolant;
	Vector3 color;

	timeofday	= Hours + (Minutes / 60.0f);
	interpolant = log10 (Gloominess * 9.0f + 1.0f);
	Vector3::Lerp (Interpolate_Color (_skycolors, colorcount, timeofday), Interpolate_Color (_gloominesscolors, colorcount, timeofday), interpolant, &color);
	Vector3::Lerp (color, tintcolor, TintFactor, &Color);
}


/***********************************************************************************************
 * SkyClass::Set_Moon_Type --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Set_Moon_Type (MoonTypeEnum moontype)
{
	const char *moontextures [MOON_TYPE_COUNT] = {"FullMoon.tga",
																 "PartMoon.tga"};

	Moon->Set_Texture (moontextures [moontype]);
}


/***********************************************************************************************
 * SkyClass::Interpolate_Color --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
Vector3 SkyClass::Interpolate_Color (const unsigned char colortable [][3], unsigned colorcount, float interpolant)
{
	const float ooucharmax = 1.0f / UCHAR_MAX;

	unsigned lowerindex, upperindex;
	Vector3  lowercolor, uppercolor;
	float		alpha;
	Vector3  color;

	// Clamp the interpolant to the index range of the color table.
	interpolant = MIN (MAX (0.0f, interpolant), colorcount);
	if (interpolant == colorcount) interpolant = 0.0f;

	lowerindex = floorf (interpolant);
	upperindex = (lowerindex + 1) % colorcount;

	lowercolor = Vector3 (colortable [lowerindex][0] * ooucharmax, colortable [lowerindex][1] * ooucharmax, colortable [lowerindex][2] * ooucharmax);
	uppercolor = Vector3 (colortable [upperindex][0] * ooucharmax, colortable [upperindex][1] * ooucharmax, colortable [upperindex][2] * ooucharmax);
	alpha		  = interpolant - lowerindex;

	Vector3::Lerp (lowercolor, uppercolor, alpha, &color);
	return (color);
}


/***********************************************************************************************
 * SkyClass::Interpolate_Scalar --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
float SkyClass::Interpolate_Scalar (const unsigned char scalartable [], unsigned scalarcount, float interpolant)
{
	const float ooucharmax = 1.0f / UCHAR_MAX;

	unsigned lowerindex, upperindex;
	float		lowervalue, uppervalue, alpha;

	// Clamp the interpolant to the index range of the color table.
	interpolant = MIN (MAX (0.0f, interpolant), scalarcount);
	if (interpolant == scalarcount) interpolant = 0.0f;

	lowerindex = floorf (interpolant);
	upperindex = (lowerindex + 1) % scalarcount;

	lowervalue = scalartable [lowerindex] * ooucharmax;
	uppervalue = scalartable [upperindex] * ooucharmax;
	alpha		  = interpolant - lowerindex;

	return (lowervalue + ((uppervalue - lowervalue) * alpha));
}


/***********************************************************************************************
 * SkyClass::Lightning_Delay --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
unsigned SkyClass::Lightning_Delay()
{
	const unsigned maxdelay = 240000; // Maximum delay for lightning (in milliseconds).

	return (_RandomNumber (1, maxdelay));
}


/***********************************************************************************************
 * SkyClass::War_Blitz_Delay --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/26/01    IML : Created.                                                                *
 *=============================================================================================*/
unsigned SkyClass::War_Blitz_Delay()
{
	const unsigned maxdelay = 240000; // Maximum delay for war blitz (in milliseconds).

	return (_RandomNumber (1, maxdelay));
}


/***********************************************************************************************
 * SkyClass::Update --																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Update (SceneClass *mainscene, const Vector3 &cameraposition)
{
	const unsigned timescale = 40;
	const Vector3  white (1.0f, 1.0f, 1.0f);

	unsigned	time;
	Vector3	colorsum, fogcolor;
	int		ticks;

	Matrix3D t = Matrix3D (cameraposition);
	Set_Transform (t);

	// Iterate over lightning and create, update or destroy them as necessary.
	time = WW3D::Get_Frame_Time();
	colorsum.Set (0.0f, 0.0f, 0.0f);
	for (unsigned l = 0; l < LIGHTNING_COUNT; l++) {

		// Is it time to create some more lightning?
		// NOTE: If lightning intensity is zero then no more lightning will be created.
		ticks = ((int) (LightningIntensity * timescale)) * time;
		LightningCountdown [l] -= ticks;
		if (LightningCountdown [l] <= 0) {

			LightningCountdown [l] = Lightning_Delay();
			if (Lightning [l] == NULL) {

				// Is lightning enabled?
				if (LightningIntensity > 0.0f) {
					Lightning [l] = new LightningClass (Extent, LightningStartDistance, LightningEndDistance, LightningHeading, LightningDistribution);
					WWASSERT (Lightning [l] != NULL);
				}
			}
		}

		if (Lightning [l] != NULL) {

			Vector3 lightningcolor;

			if (!Lightning [l]->Update (t, lightningcolor, SoundEnvironment)) {

				// Lightning has played out - so remove it.
				delete Lightning [l];
				Lightning [l] = NULL;

			} else {
				colorsum += lightningcolor;
			}
		}
	}

	// Is it time to create a war blitz?
	// NOTE: If war blitz intensity is zero then no more will be created.
	ticks = ((int) (WarBlitzIntensity * timescale)) * time;
	WarBlitzCountdown -= ticks;
	if (WarBlitzCountdown <= 0) {

		WarBlitzCountdown = War_Blitz_Delay();
		if (WarBlitz == NULL) {

			// Is war blitz enabled?
			if (WarBlitzIntensity > 0.0f) {
				WarBlitz = new WarBlitzClass (Extent, WarBlitzStartDistance, WarBlitzEndDistance, WarBlitzHeading, WarBlitzDistribution);
				WWASSERT (WarBlitz != NULL);
			}
		}
	}

	if (WarBlitz != NULL) {

		Vector3 warblitzcolor;

		if (!WarBlitz->Update (t, warblitzcolor)) {

			// War blitz has played out - so remove it.
			delete WarBlitz;
			WarBlitz = NULL;

		} else {
			colorsum += warblitzcolor;
		}
	}

	// Set the main scene fog color to color at horizon plus any lightning/war blitz color.
	fogcolor = Haze->Get_Horizon_Color() + colorsum;
	fogcolor.Update_Min (white);
	mainscene->Set_Fog_Color (fogcolor);
}


/***********************************************************************************************
 * SkyClass::Render --																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Render (RenderInfoClass &rinfo)
{
	DX8Wrapper::Set_Transform (D3DTS_WORLD, Transform);

	// Render in predetermined order.
	Haze->Render();
	Starfield->Render();
	CloudLayer0->Render();
	MoonHalo->Render();
	Moon->Render();
	SunHalo->Render();
	CloudLayer1->Render();
	Sun->Render();
	for (unsigned l = 0; l < LIGHTNING_COUNT; l++) {
		if (Lightning [l] != NULL) Lightning [l]->Render (rinfo);
	}
	if (WarBlitz != NULL) WarBlitz->Render (rinfo);
}


/***********************************************************************************************
 * SkyClass::Get_Obj_Space_Bounding_Sphere --																  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const
{
	const Vector3 position (0.0f, 0.0f, 0.0f);
	const Vector3 extent (Extent, Extent, Extent);

	sphere.Init (position, extent.Length());
}


/***********************************************************************************************
 * SkyClass::Get_Obj_Space_Bounding_Box --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void SkyClass::Get_Obj_Space_Bounding_Box (AABoxClass &box) const
{
	const Vector3 position (0.0f, 0.0f, 0.0f);
	const Vector3 extent	(Extent, Extent, Extent);

	box.Init (position, extent);
}


/***********************************************************************************************
 * BackgroundParameterClass::Initialize --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/01    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundParameterClass::Initialize()
{
	CurrentValue	  = 0.0f;
	NormalTarget	  = 0.0f;
	NormalDuration	  = 0.0f;
	OverrideTarget	  = 0.0f;
	OverrideDuration = 0.0f;
}


/***********************************************************************************************
 * BackgroundParameterClass::Set --																				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/01    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundParameterClass::Set (float target, float ramptime, bool override)
{
	if (!override) {
		NormalTarget	  = target;
		NormalDuration	  = ramptime;
	} else {
		OverrideTarget	  = target;
		OverrideDuration = ramptime;
	}
}


/***********************************************************************************************
 * BackgroundParameterClass::Update --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/01    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundParameterClass::Update (float time, bool override)
{
	const float previouscurrentvalue = CurrentValue;

	// Update the normal parameters.
	Update (NormalValue, NormalTarget, NormalDuration, time);

	// Update the override parameters?
	if (override) {
		Update (CurrentValue, OverrideTarget, OverrideDuration, time);
	} else {
		if (OverrideDuration > 0.0f) {
			Update (CurrentValue, NormalValue, OverrideDuration, time);
		} else {
			CurrentValue = NormalValue;
		}
	}

	return (CurrentValue != previouscurrentvalue);
}


/***********************************************************************************************
 * BackgroundParameterClass::Update --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/01    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundParameterClass::Update (float &value, float &target, float &duration, float time)
{
	if (value == target) {
		duration = 0.0f;
	} else {
		duration -= time;
		if (duration > 0.0f) {

			bool sign0, sign1;

			sign0 = value < target;
			value += ((target - value) * (time / duration));
			if (value == target) {
				duration = 0.0f;
			} else {
				sign1 = value < target;

				// If the value has 'blown past' the target value...
				if (sign0 ^ sign1) {
					duration = 0.0f;
					value		= target;
				}
			}
		} else {
			duration = 0.0f;
			value	   = target;
		}
	}
}


/***********************************************************************************************
 * BackgroundMgrClass::BackgroundMgrClass --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/06/01    IML : Created.                                                                *
 *=============================================================================================*/
BackgroundMgrClass::BackgroundMgrClass()
{
	Set_Network_ID (NETID_SERVER_BACKGROUND);
	Set_App_Packet_Type (APPPACKETTYPE_NETBACKGROUND);
}


/***********************************************************************************************
 * BackgroundMgrClass::Init --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Init (SimpleSceneClass *renderscene, SoundEnvironmentClass *soundenvironment, bool render_available)
{
	WWASSERT (renderscene != NULL);
	WWASSERT (_Sky == NULL);
	WWASSERT (_Dazzle == NULL);

	if (render_available) {
		_Sky = NEW_REF (SkyClass, (soundenvironment));
		renderscene->Add_Render_Object (_Sky);

		_Dazzle = NEW_REF (DazzleRenderObjClass, ("SUN"));
		renderscene->Add_Render_Object (_Dazzle);
	}
	Reset();
}


/***********************************************************************************************
 * BackgroundMgrClass::Reset --																					  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Reset()
{
	// Iterate and initialize the parameters.
	for (unsigned p = 0; p < PARAMETER_COUNT; p++) {
		_Parameters [p].Initialize();
	}

	// Set the background components to sensible defaults.
	Set_Time_Of_Day (15, 0);
	_LightSourceType = LIGHT_SOURCE_TYPE_SUN;
	Set_Moon_Type (SkyClass::MOON_TYPE_FULL);
	Set_Clouds (0.0f, 0.0f, 0.0f, false);
	Set_Sky_Tint (0.0f, 0.0f, false);
	Set_Lightning (0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, false);
	Set_War_Blitz (0.0f, 0.0f, 1.0f, 0.0f, 0.5f);

	_LightVector.Set (1.0f, 0.0f, 0.0f);
	_UnitLightVector.Set (1.0f, 0.0f, 0.0f);
	_CloudOverrideCount = 0;
	_LightningOverrideCount = 0;
	_SkyTintOverrideCount = 0;

	Set_Dirty();
}


/***********************************************************************************************
 * BackgroundMgrClass::Shutdown -- Deallocate resources.													  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Shutdown()
{
	//WWASSERT (_Dazzle != NULL);
	//WWASSERT (_Sky != NULL);
	if (_Dazzle) {
		_Dazzle->Remove();
		REF_PTR_RELEASE (_Dazzle);
	}
	if (_Sky) {
		_Sky->Remove();
		REF_PTR_RELEASE (_Sky);
	}
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Clouds --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Clouds (float cloudcover, float cloudgloominess, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Clouds (cloudcover, cloudgloominess, ramptime, false)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Override_Clouds --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Override_Clouds (float cloudcover, float cloudgloominess, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_CloudOverrideCount++;
		if (Set_Clouds (cloudcover, cloudgloominess, ramptime, true)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Get_Clouds --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Get_Clouds (float &cloudcover, float &gloominess)
{
	cloudcover = _Parameters [PARAMETER_CLOUD_COVER].Value();
	gloominess = _Parameters [PARAMETER_CLOUD_GLOOMINESS].Value();
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Clouds --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Clouds (float cloudcover, float cloudgloominess, float ramptime, bool override)
{
	if ((cloudcover >= 0.0f) && (cloudcover <= 1.0f) && (cloudgloominess >= 0.0f) && (cloudgloominess <= 1.0f) && (ramptime >= 0.0f)) {

		const bool o = override && (_CloudOverrideCount > 0);

		_Parameters [PARAMETER_CLOUD_COVER].Set (cloudcover, ramptime, o);
		_Parameters [PARAMETER_CLOUD_GLOOMINESS].Set (cloudgloominess, ramptime, o);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Restore_Clouds --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Restore_Clouds (float ramptime)
{
	if (CombatManager::I_Am_Server()) {

		WWASSERT (_CloudOverrideCount > 0);

		_Parameters [PARAMETER_CLOUD_COVER].Set (ramptime);
		_Parameters [PARAMETER_CLOUD_GLOOMINESS].Set (ramptime);
		_CloudOverrideCount--;
		_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Sky_Tint --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Sky_Tint (float skytintfactor, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Sky_Tint (skytintfactor, ramptime, false)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Override_Sky_Tint --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Override_Sky_Tint (float skytintfactor, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_SkyTintOverrideCount++;
		if (Set_Sky_Tint (skytintfactor, ramptime, true)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Restore_Sky_Tint --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Restore_Sky_Tint (float ramptime)
{
	if (CombatManager::I_Am_Server()) {

		WWASSERT (_SkyTintOverrideCount > 0);

		_Parameters [PARAMETER_SKY_TINT_FACTOR].Set (ramptime);
		_SkyTintOverrideCount--;
		_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Sky_Tint --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Sky_Tint (float skytintfactor, float ramptime, bool override)
{
	if ((skytintfactor >= 0.0f) && (skytintfactor <= 1.0f) && (ramptime >= 0.0f)) {

		const bool o = override && (_SkyTintOverrideCount > 0);

		_Parameters [PARAMETER_SKY_TINT_FACTOR].Set (skytintfactor, ramptime, o);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Lightning_Intensity --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Lightning_Intensity (float intensity, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Lightning_Intensity (intensity, ramptime, false)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Lightning --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Lightning (intensity, startdistance, enddistance, heading, distribution, ramptime, false)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Override_Lightning --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Override_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_LightningOverrideCount++;
		if (Set_Lightning (intensity, startdistance, enddistance, heading, distribution, ramptime, true)) {
			_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Lightning_Intensity --															  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Lightning_Intensity (float intensity, float ramptime, bool override)
{
	if ((intensity >= 0.0f) && (intensity <= 1.0f) && (ramptime >= 0.0f)) {

		const bool o = (_LightningOverrideCount > 0) && override;

		_Parameters [PARAMETER_LIGHTNING_INTENSITY].Set (intensity, ramptime, o);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_Lightning --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime, bool override)
{
	if ((intensity >= 0.0f) && (intensity <= 1.0f) && (startdistance >= 0.0f) && (startdistance <= 1.0f) &&
		 (enddistance >= 0.0f) && (enddistance <= 1.0f) && (heading >= 0.0f) && (heading <= 360.0f) &&
		 (distribution	>= 0.0f) && (distribution <= 1.0f) && (ramptime >= 0.0f)) {

		const bool o = (_LightningOverrideCount > 0) && override;

		_Parameters [PARAMETER_LIGHTNING_INTENSITY].Set (intensity, ramptime, o);
		_Parameters [PARAMETER_LIGHTNING_START_DISTANCE].Set (startdistance, ramptime, o);
		_Parameters [PARAMETER_LIGHTNING_END_DISTANCE].Set (enddistance, ramptime, o);
		_Parameters [PARAMETER_LIGHTNING_HEADING].Set (heading, ramptime, o);
		_Parameters [PARAMETER_LIGHTNING_DISTRIBUTION].Set (distribution, ramptime, o);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Get_Lightning --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Get_Lightning (float &intensity, float &startdistance, float &enddistance, float &heading, float &distribution)
{
	intensity	  = _Parameters [PARAMETER_LIGHTNING_INTENSITY].Value();
	startdistance = _Parameters [PARAMETER_LIGHTNING_START_DISTANCE].Value();
	enddistance	  = _Parameters [PARAMETER_LIGHTNING_END_DISTANCE].Value();
	heading		  = _Parameters [PARAMETER_LIGHTNING_HEADING].Value();
	distribution  = _Parameters [PARAMETER_LIGHTNING_DISTRIBUTION].Value();
}


/***********************************************************************************************
 * BackgroundMgrClass::Restore_Lightning --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Restore_Lightning (float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_Parameters [PARAMETER_LIGHTNING_INTENSITY].Set (ramptime);
		_Parameters [PARAMETER_LIGHTNING_START_DISTANCE].Set (ramptime);
		_Parameters [PARAMETER_LIGHTNING_END_DISTANCE].Set (ramptime);
		_Parameters [PARAMETER_LIGHTNING_HEADING].Set (ramptime);
		_Parameters [PARAMETER_LIGHTNING_DISTRIBUTION].Set (ramptime);
		_LightningOverrideCount--;
		_TheBackgroundMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_War_Blitz --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_War_Blitz (float intensity, float ramptime)
{
	if ((intensity >= 0.0f) && (intensity <= 1.0f) && (ramptime >= 0.0f)) {
		_Parameters [PARAMETER_WAR_BLITZ_INTENSITY].Set (intensity, ramptime, false);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Set_War_Blitz --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Set_War_Blitz (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime, bool /* override */)
{
	if ((intensity >= 0.0f) && (intensity <= 1.0f) && (startdistance >= 0.0f) && (startdistance <= 1.0f) &&
		 (enddistance >= 0.0f) && (enddistance <= 1.0f) && (heading >= 0.0f) && (heading <= 360.0f) &&
		 (distribution	>= 0.0f) && (distribution <= 1.0f) && (ramptime >= 0.0f)) {

		_Parameters [PARAMETER_WAR_BLITZ_INTENSITY].Set (intensity, ramptime, false);
		_Parameters [PARAMETER_WAR_BLITZ_START_DISTANCE].Set (startdistance, ramptime, false);
		_Parameters [PARAMETER_WAR_BLITZ_END_DISTANCE].Set (enddistance, ramptime, false);
		_Parameters [PARAMETER_WAR_BLITZ_HEADING].Set (heading, ramptime, false);
		_Parameters [PARAMETER_WAR_BLITZ_DISTRIBUTION].Set (distribution, ramptime, false);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Get_War_Blitz --																		  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Get_War_Blitz (float &intensity, float &startdistance, float &enddistance, float &heading, float &distribution)
{
	intensity	  = _Parameters [PARAMETER_WAR_BLITZ_INTENSITY].Value();
	startdistance = _Parameters [PARAMETER_WAR_BLITZ_START_DISTANCE].Value();
	enddistance	  = _Parameters [PARAMETER_WAR_BLITZ_END_DISTANCE].Value();
	heading		  = _Parameters [PARAMETER_WAR_BLITZ_HEADING].Value();
	distribution  = _Parameters [PARAMETER_WAR_BLITZ_DISTRIBUTION].Value();
}


/***********************************************************************************************
 * BackgroundMgrClass::Update --								                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
void BackgroundMgrClass::Update (PhysicsSceneClass *mainscene, CameraClass *camera)
{
	const float minlensflareintensity = 0.10f;
	const float maxlensflareintensity = 0.50f;

	float	  time;
	bool	  cloudsmodified, skytintmodified, lightningmodified, warblitzmodified;
	Vector3 lightvector;
	float	  znear, zfar;
	Vector3 dazzlecolor;
	float	  cloudcover, gloominess;
	float	  intensity, dazzleintensity, lensflareintensity;

	if (_Sky == NULL) return;

	WWASSERT (mainscene != NULL);
	WWASSERT (camera != NULL);
	WWASSERT (_Sky != NULL);
	WWASSERT (_Dazzle != NULL);

	if (Is_Dirty()) {
		_Sky->Set_Time_Of_Day (_Hours, _Minutes);
		_Sky->Set_Moon_Type (_MoonType);
	}

	// Process parameters.
	time = WW3D::Get_Frame_Time() * 0.001f;

	cloudsmodified	 = _Parameters [PARAMETER_CLOUD_COVER].Update (time, _CloudOverrideCount > 0);
	cloudsmodified	|= _Parameters [PARAMETER_CLOUD_GLOOMINESS].Update (time, _CloudOverrideCount > 0);
  	if (Is_Dirty() || cloudsmodified) {
  		_Sky->Set_Clouds (_Parameters [PARAMETER_CLOUD_COVER].Value(), _Parameters [PARAMETER_CLOUD_GLOOMINESS].Value());
	}

	skytintmodified = _Parameters [PARAMETER_SKY_TINT_FACTOR].Update (time, _SkyTintOverrideCount > 0);
	if (Is_Dirty() || skytintmodified) {
		_Sky->Set_Tint_Factor (_Parameters [PARAMETER_SKY_TINT_FACTOR].Value());
	}

  	lightningmodified  = _Parameters [PARAMETER_LIGHTNING_INTENSITY].Update (time, _LightningOverrideCount > 0);
	lightningmodified |= _Parameters [PARAMETER_LIGHTNING_START_DISTANCE].Update (time, _LightningOverrideCount > 0);
	lightningmodified |= _Parameters [PARAMETER_LIGHTNING_END_DISTANCE].Update (time, _LightningOverrideCount > 0);
	lightningmodified |= _Parameters [PARAMETER_LIGHTNING_HEADING].Update (time, _LightningOverrideCount > 0);
	lightningmodified |= _Parameters [PARAMETER_LIGHTNING_DISTRIBUTION].Update (time, _LightningOverrideCount > 0);
	if (Is_Dirty() || lightningmodified) {
 		_Sky->Set_Lightning (_Parameters [PARAMETER_LIGHTNING_INTENSITY].Value(), _Parameters [PARAMETER_LIGHTNING_START_DISTANCE].Value(), _Parameters [PARAMETER_LIGHTNING_END_DISTANCE].Value(), _Parameters [PARAMETER_LIGHTNING_HEADING].Value(), _Parameters [PARAMETER_LIGHTNING_DISTRIBUTION].Value());
  	}

	warblitzmodified  = _Parameters [PARAMETER_WAR_BLITZ_INTENSITY].Update (time, false);
	warblitzmodified |= _Parameters [PARAMETER_WAR_BLITZ_START_DISTANCE].Update (time, false);
	warblitzmodified |= _Parameters [PARAMETER_WAR_BLITZ_END_DISTANCE].Update (time, false);
	warblitzmodified |= _Parameters [PARAMETER_WAR_BLITZ_HEADING].Update (time, false);
	warblitzmodified |= _Parameters [PARAMETER_WAR_BLITZ_DISTRIBUTION].Update (time, false);
	if (Is_Dirty() || warblitzmodified) {
 		_Sky->Set_War_Blitz (_Parameters [PARAMETER_WAR_BLITZ_INTENSITY].Value(), _Parameters [PARAMETER_WAR_BLITZ_START_DISTANCE].Value(), _Parameters [PARAMETER_WAR_BLITZ_END_DISTANCE].Value(), _Parameters [PARAMETER_WAR_BLITZ_HEADING].Value(), _Parameters [PARAMETER_WAR_BLITZ_DISTRIBUTION].Value());
	}

  	// Has the sunlight direction changed this frame?
  	mainscene->Get_Sun_Light_Vector (&lightvector);
  	if (Is_Dirty() || (_LightVector != lightvector)) {

  	  	// NOTE 0: Direction must be normalized.
  		// NOTE 1: For simplicity, the moon is always directly opposite the sun.
  		_LightVector	  = lightvector;
  		_UnitLightVector = Normalize (-lightvector);

  		// Invert the direction if the light source is the moon (rather than the sun).
  		if (_LightSourceType == LIGHT_SOURCE_TYPE_MOON) {
  			_UnitLightVector = -_UnitLightVector;
  		}

  		_Sky->Set_Light_Direction (_UnitLightVector, -_UnitLightVector);
  	}

  	_Sky->Update (mainscene, camera->Get_Position());

  	// Locate the sun dazzle just inside the far clip plane.
  	camera->Get_Clip_Planes (znear, zfar);
  	_Dazzle->Set_Transform (Matrix3D (camera->Get_Position() + _UnitLightVector * (zfar - 1.0f)));

  	// Set the dazzle color to that of sun.
	// Attenuate dazzle and lensflare as function of cloud cover and gloominess.
	Get_Clouds (cloudcover, gloominess);
	intensity = 1.0f - (0.5f * (cloudcover + gloominess));
	dazzleintensity = (0.75f * intensity) + 0.25f;
	dazzlecolor = _Sky->Get_Sun()->Get_Color() * dazzleintensity;
	if (intensity < minlensflareintensity) {
		lensflareintensity = 0.0f;
	} else {
	  	lensflareintensity = MIN (maxlensflareintensity, intensity);
	}
	_Dazzle->Set_Dazzle_Color (dazzlecolor);
	_Dazzle->Set_Lensflare_Intensity (lensflareintensity);

	// Everything necessary has been updated. Clear the dirty flag.
	Set_Dirty (false);
}


/***********************************************************************************************
 * BackgroundMgrClass::Save --								                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
#define WRITE_PARAMETER(varname) \
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _CURRENT_VALUE, _Parameters [PARAMETER_ ## varname].CurrentValue); \
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_VALUE, _Parameters [PARAMETER_ ## varname].NormalValue);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_TARGET, _Parameters [PARAMETER_ ## varname].NormalTarget);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_DURATION, _Parameters [PARAMETER_ ## varname].NormalDuration);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _OVERRIDE_TARGET, _Parameters [PARAMETER_ ## varname].OverrideTarget);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _OVERRIDE_DURATION, _Parameters [PARAMETER_ ## varname].OverrideDuration)

bool BackgroundMgrClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_MICRO_CHUNKS);
	WRITE_MICRO_CHUNK (csave, VARID_TIME_HOURS, _Hours);
	WRITE_MICRO_CHUNK (csave, VARID_TIME_MINUTES, _Minutes);
	WRITE_MICRO_CHUNK (csave, VARID_LIGHT_SOURCE_TYPE, _LightSourceType);
	WRITE_MICRO_CHUNK (csave, VARID_MOON_TYPE, _MoonType);
	csave.End_Chunk ();

	Save_Dynamic (csave);
	return (true);
}


/***********************************************************************************************
 * BackgroundMgrClass::Save Dynamic --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Save_Dynamic (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DYNAMIC_MICRO_CHUNKS);
	WRITE_PARAMETER (SKY_TINT_FACTOR);
	WRITE_PARAMETER (CLOUD_COVER);
	WRITE_PARAMETER (CLOUD_GLOOMINESS);
	WRITE_PARAMETER (LIGHTNING_INTENSITY);
	WRITE_PARAMETER (LIGHTNING_START_DISTANCE);
	WRITE_PARAMETER (LIGHTNING_END_DISTANCE);
	WRITE_PARAMETER (LIGHTNING_HEADING);
	WRITE_PARAMETER (LIGHTNING_DISTRIBUTION);
	WRITE_MICRO_CHUNK (csave, VARID_CLOUD_OVERRIDE_COUNT, _CloudOverrideCount);
	WRITE_MICRO_CHUNK (csave, VARID_LIGHTNING_OVERRIDE_COUNT, _LightningOverrideCount);
	WRITE_PARAMETER (WAR_BLITZ_INTENSITY);
	WRITE_PARAMETER (WAR_BLITZ_START_DISTANCE);
	WRITE_PARAMETER (WAR_BLITZ_END_DISTANCE);
	WRITE_PARAMETER (WAR_BLITZ_HEADING);
	WRITE_PARAMETER (WAR_BLITZ_DISTRIBUTION);
	WRITE_MICRO_CHUNK (csave, VARID_SKY_TINT_OVERRIDE_COUNT, _SkyTintOverrideCount);
	csave.End_Chunk ();
	return (true);
}


/***********************************************************************************************
 * BackgroundMgrClass::Load --								                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Load (ChunkLoadClass &cload)
{
	WWMEMLOG (MEM_GAMEDATA);

	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_MICRO_CHUNKS:
				retval &= Load_Micro_Chunks (cload);
				break;

			case CHUNKID_DYNAMIC_MICRO_CHUNKS:
				retval &= Load_Dynamic_Micro_Chunks (cload);
				break;

		}
		cload.Close_Chunk ();
	}

	return (retval);
}


/***********************************************************************************************
 * BackgroundMgrClass::Load_Micro_Chunks --																	  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
#define READ_PARAMETER(varname) \
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _CURRENT_VALUE, _Parameters [PARAMETER_ ## varname].CurrentValue); \
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_VALUE, _Parameters [PARAMETER_ ## varname].NormalValue);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_TARGET, _Parameters [PARAMETER_ ## varname].NormalTarget);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_DURATION, _Parameters [PARAMETER_ ## varname].NormalDuration);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _OVERRIDE_TARGET, _Parameters [PARAMETER_ ## varname].OverrideTarget);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _OVERRIDE_DURATION, _Parameters [PARAMETER_ ## varname             \
].OverrideDuration)

bool BackgroundMgrClass::Load_Micro_Chunks (ChunkLoadClass &cload)
{
	// Read background micro chunk.
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_MICRO_CHUNK (cload, VARID_TIME_HOURS, _Hours);
			READ_MICRO_CHUNK (cload, VARID_TIME_MINUTES, _Minutes);
			READ_MICRO_CHUNK (cload, VARID_LIGHT_SOURCE_TYPE, _LightSourceType);
			READ_MICRO_CHUNK (cload, VARID_MOON_TYPE, _MoonType);
		}
		cload.Close_Micro_Chunk ();
	}

	return (true);
}


/***********************************************************************************************
 * BackgroundMgrClass::Load Dynamic --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Load_Dynamic (ChunkLoadClass &cload)
{
	WWMEMLOG (MEM_GAMEDATA);

	bool retval = true;

	while (cload.Open_Chunk ()) {
		switch (cload.Cur_Chunk_ID ()) {

			case CHUNKID_DYNAMIC_MICRO_CHUNKS:
				retval &= Load_Dynamic_Micro_Chunks (cload);
				break;
		}
		cload.Close_Chunk ();
	}
	return (retval);
}


/***********************************************************************************************
 * BackgroundMgrClass::Load_Dynamic_Micro_Chunks --														  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/15/00    IML : Created.                                                                *
 *=============================================================================================*/
bool BackgroundMgrClass::Load_Dynamic_Micro_Chunks (ChunkLoadClass &cload)
{
	// Read background micro chunk.
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_PARAMETER (SKY_TINT_FACTOR);
			READ_PARAMETER (CLOUD_COVER);
			READ_PARAMETER (CLOUD_GLOOMINESS);
			READ_PARAMETER (LIGHTNING_INTENSITY);
			READ_PARAMETER (LIGHTNING_START_DISTANCE);
			READ_PARAMETER (LIGHTNING_END_DISTANCE);
			READ_PARAMETER (LIGHTNING_HEADING);
			READ_PARAMETER (LIGHTNING_DISTRIBUTION);
			READ_MICRO_CHUNK (cload, VARID_CLOUD_OVERRIDE_COUNT, _CloudOverrideCount);
			READ_MICRO_CHUNK (cload, VARID_LIGHTNING_OVERRIDE_COUNT, _LightningOverrideCount);
			READ_PARAMETER (WAR_BLITZ_INTENSITY);
			READ_PARAMETER (WAR_BLITZ_START_DISTANCE);
			READ_PARAMETER (WAR_BLITZ_END_DISTANCE);
			READ_PARAMETER (WAR_BLITZ_HEADING);
			READ_PARAMETER (WAR_BLITZ_DISTRIBUTION);
			READ_MICRO_CHUNK (cload, VARID_SKY_TINT_OVERRIDE_COUNT, _SkyTintOverrideCount);
		}
		cload.Close_Micro_Chunk ();
	}

	return (true);
}


/***********************************************************************************************
 * BackgroundMgrClass::Export_Rare --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/02/01    IML : Created.                                                                *
 *=============================================================================================*/
#define EXPORT_PARAMETER(object, varname) \
object.Add (_Parameters [PARAMETER_ ## varname].NormalTarget); \
object.Add (_Parameters [PARAMETER_ ## varname].NormalDuration); \
object.Add (_Parameters [PARAMETER_ ## varname].OverrideTarget); \
object.Add (_Parameters [PARAMETER_ ## varname].OverrideDuration)

void BackgroundMgrClass::Export_Rare (BitStreamClass &packet)
{
	WWASSERT (CombatManager::I_Am_Server());

	EXPORT_PARAMETER (packet, SKY_TINT_FACTOR);
	EXPORT_PARAMETER (packet, CLOUD_COVER);
	EXPORT_PARAMETER (packet, CLOUD_GLOOMINESS);
	EXPORT_PARAMETER (packet, LIGHTNING_INTENSITY);
	EXPORT_PARAMETER (packet, LIGHTNING_START_DISTANCE);
	EXPORT_PARAMETER (packet, LIGHTNING_END_DISTANCE);
	EXPORT_PARAMETER (packet, LIGHTNING_HEADING);
	EXPORT_PARAMETER (packet, LIGHTNING_DISTRIBUTION);
	packet.Add (_CloudOverrideCount);
	packet.Add (_LightningOverrideCount);
	packet.Add (_SkyTintOverrideCount);
}


/***********************************************************************************************
 * BackgroundMgrClass::Import_Rare --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/02/01    IML : Created.                                                                *
 *=============================================================================================*/
#define IMPORT_PARAMETER(object, varname) \
object.Get (_Parameters [PARAMETER_ ## varname].NormalTarget); \
object.Get (_Parameters [PARAMETER_ ## varname].NormalDuration); \
object.Get (_Parameters [PARAMETER_ ## varname].OverrideTarget); \
object.Get (_Parameters [PARAMETER_ ## varname].OverrideDuration)

void BackgroundMgrClass::Import_Rare (BitStreamClass &packet)
{
	WWASSERT (CombatManager::I_Am_Client());

	IMPORT_PARAMETER (packet, SKY_TINT_FACTOR);
	IMPORT_PARAMETER (packet, CLOUD_COVER);
	IMPORT_PARAMETER (packet, CLOUD_GLOOMINESS);
	IMPORT_PARAMETER (packet, LIGHTNING_INTENSITY);
	IMPORT_PARAMETER (packet, LIGHTNING_START_DISTANCE);
	IMPORT_PARAMETER (packet, LIGHTNING_END_DISTANCE);
	IMPORT_PARAMETER (packet, LIGHTNING_HEADING);
	IMPORT_PARAMETER (packet, LIGHTNING_DISTRIBUTION);
	packet.Get (_CloudOverrideCount);
	packet.Get (_LightningOverrideCount);
	packet.Get (_SkyTintOverrideCount);
}


#undef EXPORT_PARAMETER
#undef IMPORT_PARAMETER
#undef READ_PARAMETER
#undef WRITE_PARAMETER
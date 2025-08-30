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
 *                     $Archive:: /Commando/Code/Combat/backgroundmgr.h                       $*
 *                                                                                             *
 *                       Author:: Patrick Smith                                                *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 7:26p                                              $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef __BACKGROUNDMGR_H
#define __BACKGROUNDMGR_H


// Includes.
#include "combat.h"
#include "combatchunkid.h"
#include "dx8indexbuffer.h"
#include "networkobject.h"
#include	"rendobj.h"
#include "saveloadsubsystem.h"
#include "segline.h"
#include "shader.h"
#include "vector3.h"
#include	"vector3i.h"
#include "vertmaterial.h"
#include "wwdebug.h"


// Forward declarations.
class CameraClass;
class DazzleRenderObjClass;
class SimpleSceneClass;
class TextureClass;
class SoundEnvironmentClass;

// Classes.
class VisibilityClass {

	public:
		VisibilityClass()							{Visible = true;}
		bool Is_Visible()							{return (Visible);}
		void Set_Visibility (bool visible)	{Visible = visible;}

	private :
		bool Visible;
};


class HazeClass : public VisibilityClass
{
	public:

		 HazeClass (float radius);
		~HazeClass();

		void Configure (const Vector3 &blendcolor, const Vector3 &horizoncolor, float intensity);
		void Configure (const Vector3 &blendcolor, const Vector3 &horizoncolor);
		void Configure (const Vector3 &blendcolor, float intensity);
		void Configure (const Vector3 &blendcolor);

		Vector3 Get_Horizon_Color() {return (HorizonColor * Intensity);}

		void Render();

	protected:

		enum {
			ROW_COUNT = 3,
			VERTICES_PER_TRIANGLE = 3
		};

		void Configure();

		Vector3					 BlendColor;
		Vector3					 HorizonColor;
		float						 Intensity;
		unsigned					 RowCount;
		unsigned					 VertexCount;
		Vector3				   *VertexArray;
		unsigned					 TriangleCount;
		DX8IndexBufferClass  *IndexBuffer;
		unsigned					*DiffuseArray;
		VertexMaterialClass	*Material;
		ShaderClass			    Shader;
};


class StarfieldClass : public VisibilityClass
{
	public:

		StarfieldClass (float radius, unsigned starcount);
	  ~StarfieldClass();

	  void Configure (const Vector3 &orientation, float length, float radius, const Vector3 &color0, const Vector3 &color1, float alpha);
	  void Configure (const Vector3 &orientation, float length, float radius);
	  void Configure (const Vector3 &color0, const Vector3 &color1, float alpha);

	  void Render();

	protected:

		void Configure();

		enum {
			VERTICES_PER_TRIANGLE = 3
		};

		Vector3					Orientation;
		float						Length;
		float						Radius;
		Vector3					Color0, Color1;
		float					   Alpha;
		unsigned					VertexCount;
		Vector3				  *VertexArray;
		unsigned					TriangleCount;
		DX8IndexBufferClass *IndexBuffer;
		VertexMaterialClass *Material;
		ShaderClass			   Shader;
		TextureClass		  *Texture;
		unsigned				  *DiffuseArray;
		unsigned					ActiveVertexCount;
		unsigned					ActiveTriangleCount;
};


class SkyObjectClass	: public VisibilityClass
{
	public:

		SkyObjectClass (ShaderClass shader);
	  ~SkyObjectClass();

		void Configure (const Vector3 &direction, float length, float radius, const Vector3 &color);
		void Configure (const Vector3 &direction, float length, float width, float height);
		void Configure (const Vector3 &direction, float length, float radius);
		void Configure (const Vector3 &color);

		void	  Set_Texture (const char *texturename);
		Vector3 Get_Color() {return (Color);}
		void	  Render();

	protected:

		enum {
			VERTICES_PER_TRIANGLE = 3
		};

		void Configure();

		Vector3					Direction;
		float						Length;
		float						Width;
		float						Height;
		Vector3					Color;
		unsigned					VertexCount;
		Vector3				  *VertexArray;
		unsigned					TriangleCount;
		DX8IndexBufferClass *IndexBuffer;
		VertexMaterialClass *Material;
		ShaderClass			   Shader;
		TextureClass		  *Texture;
		Vector2				  *TexCoordArray;
		unsigned				  *DiffuseArray;
};


class CloudLayerClass : public VisibilityClass
{
	public:

  		 CloudLayerClass (float maxdistance, const char *texturename, const Vector2 &velocity, float tilefactor, bool rotate);
		~CloudLayerClass();

		void Configure (const Vector3 &warmcolor, const Vector3 &coldcolor);
		void Configure (const Vector3 &warmdirection);
		void Configure (const Vector3 &warmcolor, const Vector3 &coldcolor, const Vector3 &warmdirection);
		void Configure (float alpha, float cloudintensity, float horizonintensity);

		void Render();

	protected:

		enum {
			VERTICES_PER_TRIANGLE = 3
		};

		void Configure();

		Vector3					WarmColor;
		Vector3					ColdColor;
		Vector3					WarmDirection;
		float						Alpha;
		float						CloudIntensity;
		float						HorizonIntensity;
		unsigned					RowCount;
		unsigned					VertexCount;
		Vector3				  *VertexArray;
		unsigned					TriangleCount;
		DX8IndexBufferClass *IndexBuffer;
		VertexMaterialClass *Material;
		ShaderClass			   Shader;
		TextureClass		  *Texture;
		Vector2				  *TexCoordArray;
		Vector2					Velocity;
		unsigned				  *DiffuseArray;
};


class SkyGlowClass : public VisibilityClass
{
	public:

		 SkyGlowClass (float radius);
		~SkyGlowClass();

		void Configure (const Vector2 &hotdirection, const Vector3 &horizoncolor, float coldintensity);
		void Render();

	protected:

		enum {
			ROW_COUNT = 5,
			VERTICES_PER_TRIANGLE = 3
		};

		void Configure();

		Vector2					 HotDirection;
		Vector3					 HorizonColor;
		float						 ColdIntensity;
		unsigned					 RowCount;
		float						 Radius;
		float						 MinZ, MaxZ;
		unsigned					 VertexCount;
		Vector3				   *VertexArray;
		unsigned					 TriangleCount;
		DX8IndexBufferClass  *IndexBuffer;
		unsigned					*DiffuseArray;
		VertexMaterialClass	*Material;
		ShaderClass			    Shader;
};


class LightningBoltClass : public VisibilityClass, public SegmentedLineClass
{
	public:
		 LightningBoltClass (int branchcount, Matrix3D &m, float length, float childlength, float width, float amplitude);
		~LightningBoltClass ();

				  void Configure (Vector3 &color);
		virtual void Set_Transform (Matrix3D &t);
		virtual void Render (RenderInfoClass &rinfo) override;

		void Set_Visibility (bool visible, bool recurse);

	private:

		struct BranchStruct {
			LightningBoltClass *LightningBolt;
		};

		BranchStruct *Branches;
		int			  BranchCount;
};


class LightningClass
{
	public:
		 LightningClass (float extent, float startdistance, float enddistance, float heading, float distribution);
		~LightningClass();

		bool Update (Matrix3D &t, Vector3 &additivecolor, SoundEnvironmentClass *soundenvironment);
		void Render (RenderInfoClass &rinfo);

	private:

		SkyGlowClass		 *LightningGlow;
		LightningBoltClass *LightningBolt;
		SkyObjectClass		 *LightningSource;
		Vector2				  Direction;
		float					  Distance;
		unsigned				  Time;
		unsigned				  ThunderDelayTime;
		Vector3				  ThunderPosition;
		const char			 *ThunderSampleName;
		bool					  PlayedThunder;
};


class WarBlitzClass
{
	public:
		WarBlitzClass (float extent, float startdistance, float enddistance, float heading, float distribution);
	  ~WarBlitzClass();

	  bool Update (Matrix3D &t, Vector3 &additivecolor);
	  void Render (RenderInfoClass &rinfo);

	private:

		SkyGlowClass   *WarBlitzGlow;
		Vector2			 Direction;
		float				 Distance;
		unsigned			 Time;
		unsigned			 SampleDelayTime;
		Vector3			 SamplePosition;
		bool				 PlayedSample;
};


class	SkyClass : public RenderObjClass
{
	public:

		enum MoonTypeEnum {
			MOON_TYPE_FULL,
			MOON_TYPE_PART,
			MOON_TYPE_COUNT
		};

		 SkyClass (SoundEnvironmentClass *soundenvironment);
		~SkyClass();

		RenderObjClass *Clone() const override
		{
			WWASSERT (false);
			return (0);
		}

		void				 Set_Color();
		Vector3			 Get_Color()  {return (Color);}
		SkyObjectClass *Get_Sun()	  {return (Sun);}

		void Set_Light_Direction (const Vector3 &sundirection, const Vector3 &moondirection);
		void Set_Time_Of_Day (unsigned hours, unsigned minutes);
		void Set_Moon_Type (MoonTypeEnum moontype);
		void Set_Clouds (float cloudcover, float gloominess);
		void Set_Tint_Factor (float tintfactor);

		void Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution)
		{
			LightningIntensity	  = intensity;
			LightningStartDistance = startdistance;
			LightningEndDistance	  = enddistance;
			LightningHeading		  = DEG_TO_RADF (heading);
			LightningDistribution  = distribution;
		}

		void Set_War_Blitz (float intensity, float startdistance, float enddistance, float heading, float distribution)
		{
			WarBlitzIntensity		 = intensity;
			WarBlitzStartDistance = startdistance;
			WarBlitzEndDistance	 = enddistance;
			WarBlitzHeading		 = DEG_TO_RADF (heading);
			WarBlitzDistribution  = distribution;
		}

		void Update (SceneClass *mainscene, const Vector3 &cameraposition);
		void Render (RenderInfoClass &rinfo) override;

		void Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const override;
		void Get_Obj_Space_Bounding_Box (AABoxClass &box) const override;

	protected:

		enum {
			LIGHTNING_COUNT = 2
		};

		Vector3  Interpolate_Color (const unsigned char colortable [][3], unsigned colorcount, float interpolant);
		float    Interpolate_Scalar (const unsigned char scalartable [], unsigned scalarcount, float interpolant);
		unsigned	Lightning_Delay();
		unsigned War_Blitz_Delay();

		float								Extent;							// Radius of sphere (and half cube width) which contains all sky objects (for clipping purposes).
		Vector3							Color;
		unsigned							Hours, Minutes;
		float								Gloominess;
		float								TintFactor;
		HazeClass					  *Haze;
		StarfieldClass				  *Starfield;
		SkyObjectClass				  *Sun;
		SkyObjectClass				  *SunHalo;
		SkyObjectClass				  *Moon;
		SkyObjectClass				  *MoonHalo;
		CloudLayerClass			  *CloudLayer0;
		CloudLayerClass			  *CloudLayer1;
		LightningClass				  *Lightning [LIGHTNING_COUNT];
		int								LightningCountdown [LIGHTNING_COUNT];
		float								LightningIntensity;
		float								LightningStartDistance;
		float								LightningEndDistance;
		float								LightningHeading;
		float								LightningDistribution;
		SoundEnvironmentClass	  *SoundEnvironment;
		WarBlitzClass				  *WarBlitz;
		int								WarBlitzCountdown;
		float								WarBlitzIntensity;
		float								WarBlitzStartDistance;
		float								WarBlitzEndDistance;
		float								WarBlitzHeading;
		float								WarBlitzDistribution;
};


class BackgroundParameterClass
{
	public:

		void Initialize();
		void Set (float targetvalue, float ramptime, bool override);
		void Set (float overrideramptime) {OverrideDuration = overrideramptime;}

		float Value() {return (CurrentValue);}

		bool Update (float time, bool override);
		void Update (float &value, float &target, float &duration, float time);

	private:

		float CurrentValue;
		float NormalValue;
		float NormalTarget;
		float	NormalDuration;
		float OverrideTarget;
		float OverrideDuration;

		friend class BackgroundMgrClass;
};


class	BackgroundMgrClass : public SaveLoadSubSystemClass, public NetworkObjectClass
{
	public:

		enum LightSourceTypeEnum {
			LIGHT_SOURCE_TYPE_SUN,
			LIGHT_SOURCE_TYPE_MOON
		};

		 BackgroundMgrClass();
		~BackgroundMgrClass() {}

		uint32		Chunk_ID() const override	{return (CHUNKID_BACKGROUND_MGR);}
		const char *Name() const override		{return ("BackgroundMgrClass");}
		void			Delete (void) override		{}
		virtual void Set_Delete_Pending (void) override {};

		bool Save (ChunkSaveClass &csave) override;
		bool Load (ChunkLoadClass &cload) override;
		bool Load_Micro_Chunks (ChunkLoadClass &cload);
		void Export_Rare (BitStreamClass &packet) override;
		void Import_Rare (BitStreamClass &packet) override;

		static void Init (SimpleSceneClass *renderscene, SoundEnvironmentClass *soundenvironment, bool render_available = true);
		static void Reset();
		static void Shutdown();

		static bool Save_Dynamic (ChunkSaveClass &csave);
		static bool Load_Dynamic (ChunkLoadClass &cload);
		static bool Load_Dynamic_Micro_Chunks (ChunkLoadClass &cload);

		static Vector3 Get_Clear_Color()
		{
			if (_Sky != NULL) {
				return (_Sky->Get_Color());
			} else {
				return (Vector3 (0.0f, 0.0f, 0.0f));
			}
		}

		static bool Set_Time_Of_Day (unsigned hours, unsigned minutes)
		{
			if ((hours >= 0) && (hours <= 23) && (minutes >= 0) && (minutes <= 59)) {
				_Hours	= hours;
				_Minutes = minutes;
				Set_Dirty();
				return (true);
			} else {
				return (false);
			}
		}

		static void Get_Time_Of_Day (unsigned &hours, unsigned &minutes)
		{
			hours	  = _Hours;
			minutes = _Minutes;
		}

		static void	Set_Light_Source_Type (LightSourceTypeEnum lightsourcetype)
		{
			_LightSourceType = lightsourcetype;
			Set_Dirty();
		}

		static LightSourceTypeEnum Get_Light_Source_Type() {return (_LightSourceType);}

		static void	Set_Moon_Type (SkyClass::MoonTypeEnum moontype)
		{
			_MoonType = moontype;
			Set_Dirty();
		}

		static SkyClass::MoonTypeEnum Get_Moon_Type()
		{
			return (_MoonType);
		}

		static bool Set_Clouds (float cloudcover, float cloudgloominess, float ramptime = 0.0f);
		static bool	Override_Clouds (float cloudcover, float cloudgloominess, float ramptime = 0.0f);
		static void Get_Clouds (float &cloudcover, float &gloominess);
		static void Restore_Clouds (float ramptime);

		static bool Set_Sky_Tint (float skytintfactor, float ramptime = 0.0f);
		static bool Override_Sky_Tint (float skytintfactor, float ramptime = 0.0f);
		static void Restore_Sky_Tint (float ramptime);

		static bool Set_Lightning_Intensity (float intensity, float ramptime = 0.0f);
		static bool Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime = 0.0f);
		static bool Override_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime = 0.0f);
		static void Get_Lightning (float &intensity, float &startdistance, float &enddistance, float &heading, float &distribution);
		static void Restore_Lightning (float ramptime);

		static bool Set_War_Blitz (float intensity, float ramptime = 0.0f);
		static bool Set_War_Blitz (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime = 0.0f, bool override = false);
		static void Get_War_Blitz (float &intensity, float &startdistance, float &enddistance, float &heading, float &distribution);

		static void Update (PhysicsSceneClass *mainscene, CameraClass *camera);

	private:

		#define VARID_PARAMETER(varname) \
			VARID_ ## varname ## _CURRENT_VALUE, \
			VARID_ ## varname ## _NORMAL_VALUE,	\
			VARID_ ## varname ## _NORMAL_TARGET, \
			VARID_ ## varname ## _NORMAL_DURATION, \
			VARID_ ## varname ## _OVERRIDE_TARGET,	\
			VARID_ ## varname ## _OVERRIDE_DURATION

		// Constants.
		enum
		{
			CHUNKID_MICRO_CHUNKS			  = 0x11080732,
			CHUNKID_DYNAMIC_MICRO_CHUNKS = 0x11020216
		};

		enum {
			VARID_TIME_HOURS = 0x14,
			VARID_TIME_MINUTES,
			VARID_LIGHT_SOURCE_TYPE,
			VARID_MOON_TYPE,

			VARID_PARAMETER (CLOUD_COVER),
			VARID_PARAMETER (CLOUD_GLOOMINESS),
			VARID_PARAMETER (SKY_TINT_FACTOR),
			VARID_PARAMETER (LIGHTNING_INTENSITY),
			VARID_PARAMETER (LIGHTNING_START_DISTANCE),
			VARID_PARAMETER (LIGHTNING_END_DISTANCE),
			VARID_PARAMETER (LIGHTNING_HEADING),
			VARID_PARAMETER (LIGHTNING_DISTRIBUTION),

			VARID_CLOUD_OVERRIDE_COUNT,
			VARID_LIGHTNING_OVERRIDE_COUNT,

			VARID_PARAMETER (WAR_BLITZ_INTENSITY),
			VARID_PARAMETER (WAR_BLITZ_START_DISTANCE),
			VARID_PARAMETER (WAR_BLITZ_END_DISTANCE),
			VARID_PARAMETER (WAR_BLITZ_HEADING),
			VARID_PARAMETER (WAR_BLITZ_DISTRIBUTION),

			VARID_SKY_TINT_OVERRIDE_COUNT
   	};

		#undef VARID_PARAMETER

		enum {
			PARAMETER_CLOUD_COVER,
			PARAMETER_CLOUD_GLOOMINESS,
			PARAMETER_SKY_TINT_FACTOR,
			PARAMETER_LIGHTNING_INTENSITY,
			PARAMETER_LIGHTNING_START_DISTANCE,
			PARAMETER_LIGHTNING_END_DISTANCE,
			PARAMETER_LIGHTNING_HEADING,
			PARAMETER_LIGHTNING_DISTRIBUTION,
			PARAMETER_WAR_BLITZ_INTENSITY,
			PARAMETER_WAR_BLITZ_START_DISTANCE,
			PARAMETER_WAR_BLITZ_END_DISTANCE,
			PARAMETER_WAR_BLITZ_HEADING,
			PARAMETER_WAR_BLITZ_DISTRIBUTION,
			PARAMETER_COUNT
		};

		static bool Set_Clouds (float cloudcover, float cloudgloominess, float ramptime, bool override);
		static bool Set_Sky_Tint (float skytintfactor, float ramptime, bool override);
		static bool Set_Lightning_Intensity (float intensity, float ramptime, bool override);
		static bool Set_Lightning (float intensity, float startdistance, float enddistance, float heading, float distribution, float ramptime, bool override);

		static bool Is_Dirty()							{return (_Dirty);}
		static void Set_Dirty (bool dirty = true)	{_Dirty = dirty;}

		static unsigned							_Hours;
		static unsigned							_Minutes;
		static LightSourceTypeEnum				_LightSourceType;
  		static SkyClass::MoonTypeEnum			_MoonType;
		static BackgroundParameterClass		_Parameters [PARAMETER_COUNT];
		static Vector3								_LightVector;
		static Vector3								_UnitLightVector;
		static unsigned							_CloudOverrideCount;
		static unsigned							_LightningOverrideCount;
		static unsigned							_SkyTintOverrideCount;
		static bool									_Dirty;
		static bool									_FogEnabled;

		static SkyClass						  *_Sky;
		static DazzleRenderObjClass		  *_Dazzle;
};


// Externals.
extern BackgroundMgrClass _TheBackgroundMgr;

#endif // BACKGROUNDMGR_H

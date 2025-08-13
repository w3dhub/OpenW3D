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
 *                     $Archive:: /Commando/Code/Combat/WeatherMgr.h          $*
 *                                                                                             *
 *                       Author:: Ian Leslie																	  *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 7:26p       $*
 *                                                                                             *
 *                    $Revision:: 16                  $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WEATHERMGR_H
#define WEATHERMGR_H


// Code controlling defines.
#define WEATHER_PARTICLE_SORT 0

// Includes.
#include "combatchunkid.h"
#include "pointgr.h"
#include "mempool.h"
#include "networkobject.h"
#include "random.h"
#include	"rendobj.h"
#include "saveloadsubsystem.h"
#include "soundenvironment.h"
#include "texture.h"
#include "vector.h"
#include "vector2.h"
#include "vector3.h"
#include "wwdebug.h"


// Class declarations.
class ChunkLoadClass;
class ChunkSaveClass;
class PhysicsSceneClass;

#if WEATHER_PARTICLE_SORT
class SortingIndexBufferClass;
#else
class	DX8IndexBufferClass;
#endif

class AudibleSoundClass;


class WindClass
{
	public:

		 WindClass (float heading, float speed, float variability, SoundEnvironmentClass *soundenvironment);
		~WindClass();

		Vector2 Get_Velocity()	{return (Velocity);}
		void	  Set (float heading, float speed, float variability);
		bool	  Update();

	protected:

		enum {
			OCTAVE_COUNT = 2
		};

		SoundEnvironmentClass *SoundEnvironment;
		float						  Heading;
		float						  Speed;
		float						  Variability;
		double					  Theta [OCTAVE_COUNT];
		Vector2					  Velocity;
		AudibleSoundClass		 *Sound;
};


class WeatherSystemClass : public RenderObjClass
{
	public:

		enum {
			GROWTH_STEP = 256
		};

		enum RenderModeEnum {
			RENDER_MODE_AXIS_ALIGNED,		// Render particle oriented about the particle's velocity vector (suitable for rain, for example).
			RENDER_MODE_CAMERA_ALIGNED,	// Render particle aligned with the camera direction (suitable for snow, for example).
			RENDER_MODE_SURFACE_ALIGNED	// Render particle such that it looks attached to the surface of an object.
		};

		 WeatherSystemClass (PhysicsSceneClass *scene,
									float emittersize,
								   float emitterheight,
								   float particledensity,
									float particlesperunitlength,
								   float particlewidth,
								   float particleheight,
								   float particlespeed,
								   const Vector2 &pageoffset,
									const Vector2 &pagesize,
								   unsigned pagecount,
									bool staticpageexists,
									float minstatictime,
									float maxstatictime,
									RenderModeEnum rendermode,
									bool decayaftercollision,
									bool prime);

		~WeatherSystemClass();
		RenderObjClass *Clone() const
		{
			WWASSERT (false);
			return (0);
		}

		void Set_Density (float density);
		void Render (RenderInfoClass &rinfo);
		void Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const;
		void Get_Obj_Space_Bounding_Box (AABoxClass &box) const;

		virtual bool Update (WindClass *wind, const Vector3 &cameraposition);

	protected:

		static constexpr int VERTICES_PER_TRIANGLE = 3;
		static constexpr int MAX_IB_PARTICLE_COUNT = 2048;
		static constexpr int MAX_AGE					 = 1000000;

		struct RayStruct : public AutoPoolClass <RayStruct, GROWTH_STEP>
		{
			public:

				RayStruct		*Next;				  	// Next ray in list.
				bool				 Initialized;			// Has the ray been defined?
				bool				 RayCast;				// Does this ray need to be relocated in the emitter (and therefore needs to be raycast).
				Vector3			 ParticleVelocity;	// Velocity of all particles spawned by this ray.
				Vector2			 StartPosition;		// Start position of ray inside emitter.
				Vector3			 EndPosition;			// Point of collision of ray with environment.
				bool				 ValidSurfaceNormal;	// Does the ray intersect a phyical object?
				Vector3			 SurfaceNormal;		// Normal of surface ray intersects (if any).
		};

		struct ParticleStruct : public AutoPoolClass <ParticleStruct, GROWTH_STEP>
		{
			public:

				ParticleStruct *Prev;					// Previous particle in list.
				ParticleStruct *Next;					// Next particle in list.
				float				 CollisionTime;		// Time when particle hits something (in seconds).
				float				 LifeTime;				// Lifetime of particle (in seconds).
				float				 ElapsedTime;			// Time elapsed since birth of particle (in seconds).
				Vector3			 Velocity;				// Velocity of particle (in metres per second).
				Vector2			 UnitZVelocity;		// Velocity / Velocity.Z (precomputed for optimization purposes).
				Vector3			 CollisionPosition;	// Position of collision.
				Vector3			 CurrentPosition;		// Current position of particle (in world space).
				Vector3			 SurfaceNormal;		// Normal of surface particle collides with (if any).

				unsigned char	 Page;					// Texture page for this particle.
				unsigned char	 RenderMode;
				unsigned char	 Pad [2];				// Pad structure to 4-byte multiple.
		};

		// Utility functions.
		float Spawn_Count (float time) {return (ParticleDensity * EmitterSize * EmitterSize * time);}
		bool	Can_Spawn (const RayStruct *rayptr) {return (rayptr->EndPosition.Z < EmitterPosition.Z);}

		bool Spawn (RayStruct *suppliedrayptr = NULL);
		void Kill (ParticleStruct *particleptr);

		PhysicsSceneClass						*Scene;						// The scene that contains the weather system.
		float										 Age;							// Age of this weather system (in seconds).
		float										 EmitterSize;				// Size of square emitter (area of emitter = size * size).
		float										 EmitterHeight;			// Height of emitter (relative to camera).
		Vector3									 EmitterPosition;			// Centroid of emitter (in world space).
		float										 ParticleDensity;			// Density of particles (over area of emitter).
		float										 ParticlesPerUnitLength;
		float										 ParticleSpeed;
		Vector3									 ParticleVelocity;		// Dominant velocity vector of particles.
		float										 HalfParticleWidth;
		float										 HalfParticleHeight;
	   RayStruct								*RayHead;
		unsigned									 RayCount;
	   RayStruct								*RaySpawnPtr;
		RayStruct								*RayUpdatePtr;
		float										 MinRayEndZ;				// Current lowest Z-value of end of any ray (used to determine a bounding box around this render object).
		float										 SpawnCountFraction;		// Cumulative fractional spawn counts (for improved accuracy).
		Vector3									 SceneMin, SceneMax;		// Bounding box around the scene that the particle system lives in.
		Vector3									 ObjectMin, ObjectMax;	// Bounding box around this render object (for culling purposes).
		ParticleStruct							*ParticleHead;				// Head of list of particles in system.
		unsigned									 ParticleCount;			// No. of particles in system.

		#if WEATHER_PARTICLE_SORT
		SortingIndexBufferClass				*IndexBuffer;
		#else
		DX8IndexBufferClass					*IndexBuffer;
		#endif

		VertexMaterialClass					*Material;
		ShaderClass								 Shader;
		TextureClass							*Texture;
		Vector2									*TextureArray;
		RenderModeEnum							 RenderMode;
		bool										 DecayAfterCollision;
		unsigned									 PageCount;
		bool										 StaticPageExists;
		float										 MinStaticTime;
		float										 MaxStaticTime;
		Vector3									 CameraPosition;
		bool										 CameraPositionValid;

		static Random2Class					_RandomNumber;				// Random no. generator.
		static unsigned						_GlobalParticleCount;	// Total no. of particles over all weather systems.
};


class RainSystemClass : public WeatherSystemClass
{
	public:

					RainSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass *wind, SoundEnvironmentClass *soundenvironment, bool prime);
		virtual ~RainSystemClass();

		bool Update (WindClass *wind, const Vector3 &cameraposition);

	protected:

		enum {
			PAGE_COUNT = 4
		};

		AudibleSoundClass		 *Sound;					// Sound effect for rainfall.
		SoundEnvironmentClass *SoundEnvironment;
};


class SnowSystemClass : public WeatherSystemClass
{
	public:

		SnowSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass *wind, bool prime);

		bool Update (WindClass *wind, const Vector3 &cameraposition);

	protected:

		enum {
			PAGE_COUNT = 4
		};
};


class AshSystemClass : public WeatherSystemClass
{
	public:

		AshSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass *wind, bool prime);

		bool Update (WindClass *wind, const Vector3 &cameraposition);

	protected:

		enum {
			PAGE_COUNT = 4
		};
};


class WeatherParameterClass
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

		friend class WeatherMgrClass;
};



class	WeatherMgrClass : public SaveLoadSubSystemClass, public NetworkObjectClass
{
	public:

		enum PrecipitationEnum {
			PRECIPITATION_FIRST,
			PRECIPITATION_RAIN = PRECIPITATION_FIRST,
			PRECIPITATION_SNOW,
			PRECIPITATION_ASH,
			PRECIPITATION_COUNT
		};

		 WeatherMgrClass();
		~WeatherMgrClass() {}

		uint32		Chunk_ID() const	{return (CHUNKID_WEATHER_MGR);}
		const char *Name() const		{return ("WeatherMgrClass");}
		void Delete (void)				{}
		virtual void Set_Delete_Pending (void) {};

		bool Save (ChunkSaveClass &csave);
		bool Load (ChunkLoadClass &cload);
		bool Load_Micro_Chunks (ChunkLoadClass &cload);
		void Export_Rare (BitStreamClass &packet);
		void Import_Rare (BitStreamClass &packet);

		static void Init (SoundEnvironmentClass *soundenvironment);
		static void	Reset();
		static void Shutdown();

		static bool Save_Dynamic (ChunkSaveClass &csave);
		static bool Load_Dynamic (ChunkLoadClass &cload);
		static bool Load_Dynamic_Micro_Chunks (ChunkLoadClass &cload);

		static bool Set_Wind (float heading, float speed, float variability, float ramptime = 0.0f);
		static bool Override_Wind (float heading, float speed, float variability, float ramptime = 0.0f);
		static void Get_Wind (float &heading, float &speed, float &variability);
		static void Restore_Wind (float ramptime);

		static bool Set_Precipitation (PrecipitationEnum precipitation, float density, float ramptime = 0.0f);
		static bool Override_Precipitation (PrecipitationEnum precipitation, float density, float ramptime = 0.0f);
		static void Get_Precipitation (PrecipitationEnum precipitation, float &density);
		static void Restore_Precipitation (float ramptime);

		static void Set_Fog_Enable (bool enabled)
		{
			_FogEnabled = enabled;
			Set_Dirty();
		}
		static bool Get_Fog_Enable()
		{
			return (_FogEnabled);
		}
		static bool Set_Fog_Range (float startdistance, float enddistance, float ramptime = 0.0f);
		static void Get_Fog_Range (float &startdistance, float &enddistance);

		static void Update (PhysicsSceneClass *scene, CameraClass *camera);
		static void Render (const CameraClass *camera);

	private:

		#define VARID_PARAMETER(varname) \
			VARID_ ## varname ## _CURRENT_VALUE, \
			VARID_ ## varname ## _NORMAL_VALUE,	\
			VARID_ ## varname ## _NORMAL_TARGET, \
			VARID_ ## varname ## _NORMAL_DURATION, \
			VARID_ ## varname ## _OVERRIDE_TARGET,	\
			VARID_ ## varname ## _OVERRIDE_DURATION

		// Constants.
		enum {
			CHUNKID_MICRO_CHUNKS			  = 0x03020113,
			CHUNKID_DYNAMIC_MICRO_CHUNKS = 0x11020245
		};

		enum {
			VARID_DUMMY = 0x09,

			VARID_PARAMETER (WIND_HEADING),
			VARID_PARAMETER (WIND_SPEED),
			VARID_PARAMETER (WIND_VARIABILITY),
			VARID_PARAMETER (RAIN_DENSITY),
			VARID_PARAMETER (SNOW_DENSITY),
			VARID_PARAMETER (ASH_DENSITY),

			VARID_WIND_OVERRIDE_COUNT,
			VARID_PRECIPITATION_OVERRIDE_COUNT,

			VARID_FOG_ENABLED,
			VARID_PARAMETER (FOG_START_DISTANCE),
			VARID_PARAMETER (FOG_END_DISTANCE)
		};

		#undef VARID_PARAMETER

		enum {
			PARAMETER_WIND_HEADING,
			PARAMETER_WIND_SPEED,
			PARAMETER_WIND_VARIABILITY,
			PARAMETER_RAIN_DENSITY,
			PARAMETER_SNOW_DENSITY,
			PARAMETER_ASH_DENSITY,
			PARAMETER_FOG_START_DISTANCE,
			PARAMETER_FOG_END_DISTANCE,
			PARAMETER_COUNT
		};

		static bool Set_Wind (float heading, float speed, float variability, float ramptime, bool override);
		static bool Set_Precipitation (PrecipitationEnum precipitation, float density, float ramptime, bool override);

		static bool Is_Dirty()							{return (_Dirty);}
		static void Set_Dirty (bool dirty = true)	{_Dirty = dirty;}

		static SoundEnvironmentClass *_SoundEnvironment;
		static WeatherParameterClass	_Parameters [PARAMETER_COUNT];
		static bool							_Prime;
		static bool							_Imported;
		static unsigned					_WindOverrideCount;
		static unsigned					_PrecipitationOverrideCount;

		static WindClass				  *_Wind;
		static WeatherSystemClass	  *_Precipitation [PRECIPITATION_COUNT];
		static bool							_FogEnabled;
		static bool							_Dirty;
};


// Externals.
extern WeatherMgrClass _TheWeatherMgr;


#endif // WEATHERMGR_H
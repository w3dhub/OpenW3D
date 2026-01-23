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
 *                     $Archive:: /Commando/Code/Combat/WeatherMgr.cpp        $*
 *                                                                                             *
 *                       Author:: Ian Leslie		                                               *
 *                                                                                             *
 *                     $Modtime:: 1/02/02 1:26p                                     $*
 *                                                                                             *
 *                    $Revision:: 27                                                        $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Includes.
#include "WeatherMgr.h"
#include "apppackettypes.h"
#include "assetmgr.h"
#include "AudibleSound.h"
#include "camera.h"
#include "chunkio.h"
#include "combat.h"
#include "gameobjmanager.h"
#include "gametype.h"
#include "light.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "phys.h"
#include "physcoltest.h"
#include "pscene.h"
#include "rinfo.h"
#include "scene.h"
#include "sortingrenderer.h"
#include "SoundEnvironment.h"
#include "WWAudio.h"
#include "wwmemlog.h"


// Singletons.
WeatherMgrClass _TheWeatherMgr;


// Static data.
DEFINE_AUTO_POOL(WeatherSystemClass::RayStruct, WeatherSystemClass::GROWTH_STEP);
DEFINE_AUTO_POOL(WeatherSystemClass::ParticleStruct, WeatherSystemClass::GROWTH_STEP);

Random2Class									 WeatherSystemClass::_RandomNumber (0x60486223);
unsigned											 WeatherSystemClass::_GlobalParticleCount = 0;

SoundEnvironmentClass						*WeatherMgrClass::_SoundEnvironment;
WeatherParameterClass						 WeatherMgrClass::_Parameters [PARAMETER_COUNT];
bool												 WeatherMgrClass::_Prime;
bool												 WeatherMgrClass::_Imported;
unsigned											 WeatherMgrClass::_WindOverrideCount;
unsigned											 WeatherMgrClass::_PrecipitationOverrideCount;

WindClass										*WeatherMgrClass::_Wind;
WeatherSystemClass							*WeatherMgrClass::_Precipitation [PRECIPITATION_COUNT];
bool												 WeatherMgrClass::_FogEnabled;
bool												 WeatherMgrClass::_Dirty;


/***********************************************************************************************
 * WindClass::WindClass --																							  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/09/01    IML : Created.                                                                *
 *=============================================================================================*/
WindClass::WindClass (float heading, float speed, float variability, SoundEnvironmentClass *soundenvironment)
	: Velocity (0.0f, 0.0f),
	  SoundEnvironment (soundenvironment)
{
	const char *windsamplename	= "Wind01";

	Set (heading, speed, variability);

	for (unsigned octave = 0; octave < OCTAVE_COUNT; octave++) {
		Theta [octave] = 0.0l;
	}

	// Create the wind sound effect.
	Sound = WWAudioClass::Get_Instance()->Create_Sound (windsamplename, NULL, 0, CLASSID_2D);
	if (Sound != NULL) {
		SoundEnvironment->Add_User();
		Sound->Set_Volume (0.0f);
		Sound->Play();
	}
}


/***********************************************************************************************
 * WindClass::~WindClass --																						  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/09/01    IML : Created.                                                                *
 *=============================================================================================*/
WindClass::~WindClass()
{
	// Remove wind sound effect.
	if (Sound != NULL) {
		Sound->Stop();
		REF_PTR_RELEASE (Sound);
		SoundEnvironment->Remove_User();
	}
}


/***********************************************************************************************
 * WindClass::Set --																									  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/09/01    IML : Created.                                                                *
 *=============================================================================================*/
void WindClass::Set (float heading, float speed, float variability)
{
	WWASSERT (speed >= 0.0f);

	Heading		= DEG_TO_RADF (heading);
	Speed			= speed;
	Variability = variability;
}


/***********************************************************************************************
 * WindClass::Update --																								  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/09/01    IML : Created.                                                                *
 *=============================================================================================*/
bool WindClass::Update()
{
	const double frequency [OCTAVE_COUNT] = {0.5l, 0.2l};
	const double twopi						  = WWMATH_PI * 2.0l;

	float	h, speed;

	if (Variability > 0.0f) {

		float f = 0.0f;
		int	d;

		for (unsigned octave = 0; octave < OCTAVE_COUNT; octave++) {

			Theta [octave] += WW3D::Get_Frame_Time() * 0.001l * frequency [octave];
			d = floorf (Theta [octave] / twopi);
			if (d >= 1) Theta [octave] -= d * twopi;
			f += sinf (Theta [octave]);
		}
		speed = Speed - (Speed * ((f + 1.0f) * 0.5f) * Variability);
	} else {
		speed = Speed;
	}

	h = Heading + (0.5f * WWMATH_PI);
	Velocity.Set (cosf (h) * speed, sinf (h) * speed);

	// Update wind sound effect.
	if (Sound != NULL) {

		const float maxvolumespeed	= 10.0f;

		float attenuation;

		// Precalculate volume attenuation based on speed.
		attenuation = (0.5f * (MIN (Speed, maxvolumespeed) + MIN (speed, maxvolumespeed))) / maxvolumespeed;
		Sound->Set_Volume (SoundEnvironment->Get_Amplitude() * attenuation);
	}

	// Is the wind essentially idle (ie. it is not contributing to the scene visually or audibly)?
	return (Speed > 0.0f);
}


/***********************************************************************************************
 * WeatherSystemClass::WeatherSystemClass --																	  *
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
WeatherSystemClass::WeatherSystemClass	(PhysicsSceneClass *scene,
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
													 bool	staticpageexists,
													 float minstatictime,
													 float maxstatictime,
													 RenderModeEnum rendermode,
													 bool decayaftercollision,
													 bool prime)
	: Scene (scene),
	  EmitterSize (emittersize),
	  EmitterHeight (emitterheight),
	  EmitterPosition (Vector3 (0.0f, 0.0f, 0.0f)),
	  ParticlesPerUnitLength (particlesperunitlength),
	  ParticleSpeed (particlespeed),
	  HalfParticleWidth (particlewidth * 0.5f),
	  HalfParticleHeight (particleheight * 0.5f),
	  RayHead (NULL),
	  RayCount (0),
	  RaySpawnPtr (NULL),
	  RayUpdatePtr (NULL),
	  ParticleHead (NULL),
	  ParticleCount (0),
	  MinRayEndZ (FLT_MAX),
	  SpawnCountFraction (0.0f),
	  PageCount (pagecount),
	  StaticPageExists (staticpageexists),
	  MinStaticTime (minstatictime),
	  MaxStaticTime (maxstatictime),
	  RenderMode (rendermode),
	  DecayAfterCollision (decayaftercollision),
	  CameraPositionValid (false)
{
	const char								*texturename = "WeatherParticles.tga";
	const TextureClass::MipCountType  mipcount	 = TextureClass::MIP_LEVELS_5;
	const float								 oopagecount = 1.0f / pagecount;

	WWASSERT (particlespeed >= 0.0f);

	// How old is the weather system?
	Age = prime ? MAX_AGE : 0.0f;

	// Set density of system.
	Set_Density (particledensity);

	// Get the scene's bounding box.
	scene->Get_Level_Extents (SceneMin, SceneMax);

	// Expand the bounding box by a small amount so that we can distinguish between collisions with geometry and
	// collisions with the bounding box.
	SceneMin.Z -= 1.0f;
	SceneMax.Z += 1.0f;

	// Initialize an index buffer.
	#if WEATHER_PARTICLE_SORT
	IndexBuffer = NEW_REF (SortingIndexBufferClass, (MAX_IB_PARTICLE_COUNT * VERTICES_PER_TRIANGLE));
	#else
	IndexBuffer = NEW_REF (DX8IndexBufferClass, (MAX_IB_PARTICLE_COUNT * VERTICES_PER_TRIANGLE));
	#endif
	{
		SortingIndexBufferClass::WriteLockClass lock (IndexBuffer);
		unsigned short *indices = lock.Get_Index_Array();

		for (unsigned i = 0; i < MAX_IB_PARTICLE_COUNT * VERTICES_PER_TRIANGLE; i++) {
			*indices++ = i;
		}
	}

	// Configure material.
	Material = VertexMaterialClass::Get_Preset (VertexMaterialClass::PRELIT_NODIFFUSE);

	// Configure shader.
	Shader = ShaderClass::_PresetAlphaShader;
	Shader.Set_Primary_Gradient (ShaderClass::GRADIENT_MODULATE);
	Shader.Set_Cull_Mode (ShaderClass::CULL_MODE_DISABLE);
	Shader.Enable_Fog ("WeatherSystemClass");

	// Configure texture.
	Texture = WW3DAssetManager::Get_Instance()->Get_Texture (texturename, mipcount);
	Texture->Set_U_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
	Texture->Set_V_Addr_Mode (TextureClass::TEXTURE_ADDRESS_CLAMP);
	Set_Translucent (true);

	// Configure texture coordinates according to given page count.
	// NOTE: Split the texture into vertical pages.
	WWASSERT (pagecount > 0);
	TextureArray = new Vector2 [pagecount * VERTICES_PER_TRIANGLE];
	WWASSERT (TextureArray != NULL);
	for (unsigned page = 0; page < pagecount; page++) {
		TextureArray [page * VERTICES_PER_TRIANGLE + 0].Set (pageoffset.U + (((page + 0.5f) * oopagecount) * pagesize.U), pageoffset.V + 0.0f);
		TextureArray [page * VERTICES_PER_TRIANGLE + 1].Set (pageoffset.U + (((page + 1.0f) * oopagecount) * pagesize.U), pageoffset.V + pagesize.V);
		TextureArray [page * VERTICES_PER_TRIANGLE + 2].Set (pageoffset.U + (((page + 0.0f) * oopagecount) * pagesize.U), pageoffset.V + pagesize.V);
	}
}


/***********************************************************************************************
 * WeatherSystemClass::WeatherSystemClass --																	  *
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
WeatherSystemClass::~WeatherSystemClass()
{
	RayStruct		*rayptr;
	ParticleStruct *particleptr;

	// Clean-up rays.
	rayptr = RayHead;
	while (rayptr != NULL) {

		RayStruct *nextrayptr;

		nextrayptr = rayptr->Next;
		delete rayptr;
		rayptr = nextrayptr;
		RayCount--;
	}

	WWASSERT (RayCount == 0);

	// Clean-up particles.
	particleptr = ParticleHead;
	while (particleptr != NULL) {

		ParticleStruct *nextparticleptr = particleptr->Next;

		Kill (particleptr);
		particleptr = nextparticleptr;
	}

	WWASSERT (ParticleCount == 0);

	REF_PTR_RELEASE (Material);
	delete [] TextureArray;
	REF_PTR_RELEASE (Texture);
	REF_PTR_RELEASE (IndexBuffer);
}


/***********************************************************************************************
 * WeatherSystemClass::Set_Density --																			  *
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
void WeatherSystemClass::Set_Density (float density)
{
	unsigned   raycount;
	int		  signedcount, r;
	RayStruct *rayptr;

	// Calculate no. of rays required.
	ParticleDensity = density;
	raycount			 = Spawn_Count (1.0f) / (ParticlesPerUnitLength * ParticleSpeed);

	// Is the ray count increasing or decreasing in size?
	signedcount = ((int) RayCount) - ((int) raycount);
	if (signedcount < 0) {

		// Add new, uninitialized rays to head of list.
		for (r = signedcount; r < 0; r++) {

			rayptr = new RayStruct;
			WWASSERT (rayptr != NULL);

			rayptr->Next = RayHead;
			rayptr->Initialized = false;
			RayHead = rayptr;
		}

		// If necessary, initialize the spawn and update pointers.
		if (RaySpawnPtr  == NULL) RaySpawnPtr  = RayHead;
		if (RayUpdatePtr == NULL) RayUpdatePtr = RayHead;

	} else {

		// Remove rays from head of list.
		rayptr = RayHead;
		for (r = 0; r < signedcount; r++) {

			RayStruct *nextrayptr;

			if (rayptr == NULL) break;

			nextrayptr = rayptr->Next;

			// If this ray is referenced by the spawn or update pointers then change them.
			if (RaySpawnPtr  == rayptr) RaySpawnPtr  = nextrayptr;
			if (RayUpdatePtr == rayptr) RayUpdatePtr = nextrayptr;

			delete rayptr;
			rayptr = nextrayptr;
		}
		RayHead = rayptr;
	}

	RayCount = raycount;
}


/***********************************************************************************************
 * WeatherSystemClass::Update --																					  *
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
bool WeatherSystemClass::Update (WindClass *wind, const Vector3 &cameraposition)
{
	struct BoundingBoxStruct {
		Vector2 Min, Max;
	};

	const unsigned randomness		= 10000;
	const float		oorandomness	= 1.0f / randomness;
	const float		overlapdelta	= EmitterSize * 2.0f;
	const unsigned rayupdatecount = MAX (RayCount * 0.018f, 1);

	Vector3				oldemitterposition;
	float					ooz;
	Vector3				emitterdirection, emitteroffset;
	Vector3				projectedemitterposition;
	BoundingBoxStruct emitterbounds;
	float					deltax, deltay;
	BoundingBoxStruct nonoverlapregions [2];
	float					regionthreshold;
	Vector3				sceneminoffset, scenemaxoffset;
	Vector2				raystartoffset;
	RayStruct		  *rayptr;
	float					spawncountfraction;
	float					alpha, beta;
	Vector2				range;
	Vector3				minrayendposition;
	float					l, boxoffset;
	ParticleStruct	  *particleptr;
	float					s;
	unsigned				spawncount;
	float					time;

	oldemitterposition = EmitterPosition;

	// Calculate particle velocity based on wind.
	if (wind != NULL) {
		ParticleVelocity.Set (wind->Get_Velocity().X, wind->Get_Velocity().Y, -ParticleSpeed);
	} else {
		ParticleVelocity.Set (0.0f, 0.0f, -ParticleSpeed);
	}

	// Calculate new emitter position.
	ooz = 1.0f / ParticleVelocity.Z;
	emitterdirection.Set (ParticleVelocity.X * ooz, ParticleVelocity.Y * ooz, 1.0f);
	emitteroffset = EmitterHeight * emitterdirection;
	EmitterPosition =	cameraposition + emitteroffset;

	// Define bounding box for new emitter area.
	emitterbounds.Min.Set (EmitterPosition.X - EmitterSize, EmitterPosition.Y - EmitterSize);
	emitterbounds.Max.Set (EmitterPosition.X + EmitterSize, EmitterPosition.Y + EmitterSize);

	// Calculate new regions that define the areas of non-overlap between the old and new emitter positions.

	// Project the old emitter onto the plane of the new emitter.
	projectedemitterposition = oldemitterposition + ((EmitterPosition.Z - oldemitterposition.Z) * emitterdirection);

	// If the old and new emitters partially overlap...
	deltax = WWMath::Fabs (EmitterPosition.X - projectedemitterposition.X);
	deltay = WWMath::Fabs (EmitterPosition.Y - projectedemitterposition.Y);
	if ((deltax < overlapdelta) && (deltay < overlapdelta) && (EmitterPosition != projectedemitterposition)) {

		float area0, area1;

		// Compare X-extents.
		if (EmitterPosition.X < projectedemitterposition.X) {
			nonoverlapregions [0].Min.X = emitterbounds.Min.X;
			nonoverlapregions [0].Max.X = projectedemitterposition.X - EmitterSize;
			nonoverlapregions [1].Min.X = nonoverlapregions [0].Max.X;
			nonoverlapregions [1].Max.X = emitterbounds.Max.X;
		} else {
			nonoverlapregions [0].Min.X = projectedemitterposition.X + EmitterSize;
			nonoverlapregions [0].Max.X = emitterbounds.Max.X;
			nonoverlapregions [1].Min.X = emitterbounds.Min.X;
			nonoverlapregions [1].Max.X = nonoverlapregions [0].Min.X;
		}
		nonoverlapregions [0].Min.Y = emitterbounds.Min.Y;
		nonoverlapregions [0].Max.Y = emitterbounds.Max.Y;

		// Compare Y-extents.
		if (EmitterPosition.Y < projectedemitterposition.Y) {
			nonoverlapregions [1].Min.Y = emitterbounds.Min.Y;
			nonoverlapregions [1].Max.Y = projectedemitterposition.Y - EmitterSize;
		} else {
			nonoverlapregions [1].Min.Y = projectedemitterposition.Y + EmitterSize;
			nonoverlapregions [1].Max.Y = emitterbounds.Max.Y;
		}

		area0 = (nonoverlapregions [0].Max.X - nonoverlapregions [0].Min.X) * (nonoverlapregions [0].Max.Y - nonoverlapregions [0].Min.Y);
		area1 = (nonoverlapregions [1].Max.X - nonoverlapregions [1].Min.X) * (nonoverlapregions [1].Max.Y - nonoverlapregions [1].Min.Y);
		if (area0 == 0.0f) {

			// Always select region 1.
			regionthreshold = -FLT_MAX;

		} else {
			if (area1 == 0.0f) {

				// Always select region 0.
				regionthreshold = +FLT_MAX;

			} else {

				// Select either region 0 or region 1 weighted by area.
				regionthreshold = area0 / (area0 + area1);
			}
		}

	} else {

		// No overlap or complete overlap: region 0 is emitter area, region 1 is not used.
		nonoverlapregions [0] = emitterbounds;
		regionthreshold = +FLT_MAX;
	}

	// Calculate offset of projection of ray start from old to new emitter.
	raystartoffset	= (EmitterPosition.Z - oldemitterposition.Z) * Vector2 (emitterdirection.X, emitterdirection.Y);

	// Precalculate offsets to intersection with upper and lower planes of scene bounds from emitter position.
	sceneminoffset = (SceneMin.Z - EmitterPosition.Z) * emitterdirection;
	scenemaxoffset = (SceneMax.Z - EmitterPosition.Z) * emitterdirection;

	// Iterate over all rays...
	spawncountfraction = 0.0f;
	rayptr = RayHead;
	while (rayptr != NULL) {

	  	Vector3 raystartposition;

		// Does this ray need to be initialized?
		if (!rayptr->Initialized) {

			// Randomly allocate a new ray start position from the entire emitter region.
			alpha = _RandomNumber (0, randomness) * oorandomness;
			beta  = _RandomNumber (0, randomness) * oorandomness;
			range = emitterbounds.Max - emitterbounds.Min;
			range.Scale (alpha, beta);
			rayptr->StartPosition = emitterbounds.Min + range;
			rayptr->Initialized	 = true;
			rayptr->RayCast		 = true;

		} else {

		  	// Project the ray's emitter position onto the plane of the new emitter.
			rayptr->StartPosition += raystartoffset;

			// Does the ray fall outside the emitter (and therefore needs to be raycast)?
			rayptr->RayCast = (rayptr->StartPosition.X < emitterbounds.Min.X) ||
									(rayptr->StartPosition.X > emitterbounds.Max.X) ||
									(rayptr->StartPosition.Y < emitterbounds.Min.Y) ||
									(rayptr->StartPosition.Y > emitterbounds.Max.Y);

			if (rayptr->RayCast) {

				unsigned regionindex;

				// Randomly allocate a new ray start position from one of the non-overlap regions.
				if ((_RandomNumber (0, randomness) * oorandomness) < regionthreshold) {
					regionindex = 0;
				} else {
					regionindex = 1;
				}
				alpha = _RandomNumber (0, randomness) * oorandomness;
				beta  = _RandomNumber (0, randomness) * oorandomness;
				range = nonoverlapregions [regionindex].Max - nonoverlapregions [regionindex].Min;
				range.Scale (alpha, beta);
				rayptr->StartPosition = nonoverlapregions [regionindex].Min + range;

			} else {

				// Next ray.
				rayptr = rayptr->Next;
				continue;
			}
		}

		raystartposition.Set (rayptr->StartPosition.X, rayptr->StartPosition.Y, EmitterPosition.Z);

		// Raycast to find the ray's collision point with the environment.
		{
			Vector3						  raycaststartpoint (raystartposition + scenemaxoffset);
			Vector3						  raycastendpoint (raystartposition + sceneminoffset);
			LineSegClass				  raycast (raycaststartpoint, raycastendpoint);
			CastResultStruct			  rayresult;
			PhysRayCollisionTestClass raytest (raycast, &rayresult, TERRAIN_ONLY_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);

			Scene->Cast_Ray (raytest);
			raycast.Compute_Point (raytest.Result->Fraction, &(rayptr->EndPosition));
			if (raytest.Result->Fraction < 1.0f) {
				rayptr->ValidSurfaceNormal = true;
				rayptr->SurfaceNormal = raytest.Result->Normal;
			} else {
				rayptr->ValidSurfaceNormal = false;
			}
		}

		rayptr->ParticleVelocity = ParticleVelocity;

		if ((Age > 0.0f) && (Can_Spawn (rayptr))) {

			float		s;
			unsigned	spawncount;

			// Spawn some particles along the ray.
			// NOTE: For accuracy, accumulate fractional spawncounts so that they can be used on a later ray.
			s = ParticlesPerUnitLength * (rayptr->EndPosition - raystartposition).Quick_Length();
			spawncount = floor (s);
			spawncountfraction += s - spawncount;
			if (spawncountfraction >= 1.0f) {
				spawncountfraction -= 1.0f;
				spawncount++;
			}
			for (unsigned p = 0; p < spawncount; p++) {
				Spawn (rayptr);
			}
		}

		// Update minimum ray end Z.
		if (rayptr->EndPosition.Z < MinRayEndZ) {
			MinRayEndZ = rayptr->EndPosition.Z;
		}

		// Next ray.
		rayptr = rayptr->Next;
	}

	// Now iterate over rayupdatecount rays and randomize them so that the ray 'pattern' is
	// not discernable to the user (because it is constantly changing) and also to take
	// account of the new particle velocity.
	if (RayUpdatePtr != NULL) {

		for (unsigned r = 0; r < rayupdatecount; r++) {

			// NOTE: Only need to randomize those rays that have not just been relocated inside the emitter.
			if (!RayUpdatePtr->RayCast) {

				// Randomly allocate a new ray start position from the entire emitter region.
				alpha = _RandomNumber (0, randomness) * oorandomness;
				beta  = _RandomNumber (0, randomness) * oorandomness;
				range = emitterbounds.Max - emitterbounds.Min;
				range.Scale (alpha, beta);
				RayUpdatePtr->StartPosition = emitterbounds.Min + range;

				// Raycast to find the ray's collision point with the environment.
				{
					Vector3						  raystartposition (RayUpdatePtr->StartPosition.X, RayUpdatePtr->StartPosition.Y, EmitterPosition.Z);
					Vector3						  raycaststartpoint (raystartposition + scenemaxoffset);
					Vector3						  raycastendpoint (raystartposition + sceneminoffset);
					LineSegClass				  raycast (raycaststartpoint, raycastendpoint);
					CastResultStruct			  rayresult;
					PhysRayCollisionTestClass raytest (raycast, &rayresult, TERRAIN_ONLY_COLLISION_GROUP, COLLISION_TYPE_PROJECTILE);

					Scene->Cast_Ray (raytest);
					raycast.Compute_Point (raytest.Result->Fraction, &(RayUpdatePtr->EndPosition));
					if (raytest.Result->Fraction < 1.0f) {
						RayUpdatePtr->ValidSurfaceNormal = true;
						RayUpdatePtr->SurfaceNormal = raytest.Result->Normal;
					} else {
						RayUpdatePtr->ValidSurfaceNormal = false;
					}
				}

				RayUpdatePtr->ParticleVelocity = ParticleVelocity;

				// Update minimum ray end Z.
				if (RayUpdatePtr->EndPosition.Z < MinRayEndZ) {
					MinRayEndZ = RayUpdatePtr->EndPosition.Z;
				}
			}

			// Next ray. If necessary, wrap around to head of list.
			RayUpdatePtr = RayUpdatePtr->Next;
			if (RayUpdatePtr == NULL) RayUpdatePtr = RayHead;
		}
	}

	// Calculate a bounding box for the render object that encompasses the rays.
	// NOTE 0: For efficiency, calculate the parallelepiped that is generated by projecting
	//			  the emitter area onto the plane that contains the lowest ray end point and put
	//			  a bounding box around this.
	// NOTE 1: Make the bounding box a little bigger to account for the particle point size.
	minrayendposition = EmitterPosition + ((MinRayEndZ - EmitterPosition.Z) * emitterdirection);
	l = MAX (HalfParticleWidth, HalfParticleHeight);
	boxoffset = EmitterSize + l;
	ObjectMax.Set (MAX (EmitterPosition.X, minrayendposition.X) + boxoffset, MAX (EmitterPosition.Y, minrayendposition.Y) + boxoffset, MAX (EmitterPosition.Z, minrayendposition.Z) + l);
	ObjectMin.Set (MIN (EmitterPosition.X, minrayendposition.X) - boxoffset, MIN (EmitterPosition.Y, minrayendposition.Y) - boxoffset, MIN (EmitterPosition.Z, minrayendposition.Z) - l);

	// Flag that the object bounding box has been modified.
	Invalidate_Cached_Bounding_Volumes();

	// Iterate over all particles...
	time = WW3D::Get_Frame_Time() * 0.001f;
	particleptr = ParticleHead;
	while (particleptr != NULL) {

		Vector2 emitterposition;
		bool	  outside;

		ParticleStruct *nextparticleptr = particleptr->Next;

		// Advance time.
		particleptr->ElapsedTime += time;

		// Has it expired?
		if (particleptr->ElapsedTime >= particleptr->LifeTime) {

			Kill (particleptr);
			particleptr = nextparticleptr;
			continue;
		}

		// Has it just collided?
		if (particleptr->ElapsedTime >= particleptr->CollisionTime) {
			if (particleptr->Velocity.Z != 0.0f) {

				// Place the particle at the collision point.
				particleptr->Velocity.Set (0.0f, 0.0f, 0.0f);
				particleptr->CurrentPosition = particleptr->CollisionPosition;
				if (StaticPageExists) particleptr->Page = PageCount - 1;
				particleptr->RenderMode = RENDER_MODE_SURFACE_ALIGNED;
			}

		} else {

			// Advance position.
			particleptr->CurrentPosition += particleptr->Velocity * time;
		}

		// Project the particle's position onto the plane of the new emitter.
		emitterposition = Vector2 (particleptr->CurrentPosition.X, particleptr->CurrentPosition.Y) + ((EmitterPosition.Z - particleptr->CurrentPosition.Z) * particleptr->UnitZVelocity);

		// Does the particle fall outside the emitter?
		outside = (emitterposition.X < emitterbounds.Min.X) ||
					 (emitterposition.X > emitterbounds.Max.X) ||
					 (emitterposition.Y < emitterbounds.Min.Y) ||
					 (emitterposition.Y > emitterbounds.Max.Y);

		if (outside) {
			Kill (particleptr);
		}

		particleptr = nextparticleptr;
	}


	// Spawn any new particles that need to be spawned on this update.
	// NOTE: For accuracy, accumulate fractional spawncounts so that they can be used on a later iteration.
	s = Spawn_Count (time);
	spawncount = floor (s);
	SpawnCountFraction += s - spawncount;
	if (SpawnCountFraction >= 1.0f) {
		SpawnCountFraction -= 1.0f;
		spawncount++;
	}
	for (unsigned p = 0; p < spawncount; p++) {
		Spawn();
	}

	// Advance weather system age.
	// NOTE: To prevent floating point overflow, don't advance the age past some maximum.
	if (Age < MAX_AGE) Age += time;

	// Is the weather still active (ie. it is contributing to the scene visually or audibly)?
	return ((ParticleDensity > 0.0f) || (ParticleCount > 0));
}


/***********************************************************************************************
 * WeatherSystemClass::Spawn --																					  *
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
bool WeatherSystemClass::Spawn (RayStruct *suppliedrayptr)
{
	const unsigned maxparticlecount = USHRT_MAX / (2 * VERTICES_PER_TRIANGLE);

	// Due to a limitation in the underlying rendering API, there cannot be more than USHRT_MAX
	// total vertices in the system. Conservatively restrict this to half of this to compensate
	// for other primitives in the system.
	if (_GlobalParticleCount < maxparticlecount) {

		RayStruct *rayptr;

		if (suppliedrayptr == NULL) {

			// Assign a ray to this particle.
			// If there are no rays to assign then return failure to spawn.
			if (RaySpawnPtr != NULL) {
				rayptr = RaySpawnPtr;
				RaySpawnPtr = RaySpawnPtr->Next;
				if (RaySpawnPtr == NULL) RaySpawnPtr = RayHead;
			} else {
				return (false);
			}

		} else {
			rayptr = suppliedrayptr;
		}

		// Ensure that this ray's endpoint lies below the emitter (otherwise there is an obstruction
		// between the ray's source and the emitter).
		if (Can_Spawn (rayptr)) {

			const unsigned randomness	  = 1000;
			const float		oorandomness  = 1.0f / randomness;
			const float		ooz			  = 1.0f / rayptr->ParticleVelocity.Z;
			const float		collisiontime = (rayptr->EndPosition.Z - EmitterPosition.Z) * ooz;

			ParticleStruct *particleptr;

			particleptr = new ParticleStruct;
			WWASSERT (particleptr != NULL);

			// Add this particle to the head of the list.
			if (ParticleHead != NULL) {
				ParticleHead->Prev = particleptr;
			}
			particleptr->Prev = NULL;
			particleptr->Next = ParticleHead;
			ParticleHead		= particleptr;

			// Increment the no. of particles.
			ParticleCount++;

			// Increment total no. of global weather particles.
			_GlobalParticleCount++;

			particleptr->UnitZVelocity.Set (rayptr->ParticleVelocity.X * ooz, rayptr->ParticleVelocity.Y * ooz);
			particleptr->CollisionTime		 = collisiontime;
			particleptr->CollisionPosition = rayptr->EndPosition;

			// If the ray does not have a valid surface normal then the ray's end position intersects the level's bounding box.
			// In this case it is not necessary to sustain the particle past the collision time.
			if (rayptr->ValidSurfaceNormal) {
				particleptr->LifeTime = collisiontime + WWMath::Lerp (MinStaticTime, MaxStaticTime, _RandomNumber (0, randomness) * oorandomness);
				particleptr->SurfaceNormal = rayptr->SurfaceNormal;
			} else {
				particleptr->LifeTime = collisiontime;
			}

			if (suppliedrayptr == NULL) {

				// Start particle at emitter.
				particleptr->ElapsedTime	  = 0.0f;
				particleptr->Velocity		  = rayptr->ParticleVelocity;
				particleptr->CurrentPosition = Vector3 (rayptr->StartPosition.X, rayptr->StartPosition.Y, EmitterPosition.Z);
				particleptr->Page				  = _RandomNumber (0, PageCount - (StaticPageExists) ? 2 : 1);
				particleptr->RenderMode		  = RenderMode;

			} else {

				float t;

				// Advance the particle some random amount in time.
				// NOTE: The particle cannot have existed longer than the weather system itself.
				t = _RandomNumber (0, randomness) * oorandomness * particleptr->LifeTime;
				particleptr->ElapsedTime = MIN (t, Age);
				if (particleptr->ElapsedTime >= particleptr->CollisionTime) {
					particleptr->Velocity.Set (0.0f, 0.0f, 0.0f);
					particleptr->CurrentPosition = rayptr->EndPosition;
					if (StaticPageExists) {
						particleptr->Page	= PageCount - 1;
					} else {
						particleptr->Page	= _RandomNumber (0, PageCount - 1);
					}
					particleptr->RenderMode = RENDER_MODE_SURFACE_ALIGNED;

				} else {
					particleptr->Velocity		  = rayptr->ParticleVelocity;
					particleptr->CurrentPosition = Vector3 (rayptr->StartPosition.X, rayptr->StartPosition.Y, EmitterPosition.Z) + (particleptr->Velocity * particleptr->ElapsedTime);
					particleptr->Page				  = _RandomNumber (0, PageCount - (StaticPageExists) ? 2 : 1);
					particleptr->RenderMode		  = RenderMode;
				}
			}

			return (true);
		}
	}

	return (false);
}


/***********************************************************************************************
 * WeatherSystemClass::Kill --																					  *
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
void WeatherSystemClass::Kill (ParticleStruct *particleptr)
{
	WWASSERT (ParticleCount > 0);

	// Remove this particle from the list.
	if (particleptr->Prev != NULL) {
		particleptr->Prev->Next = particleptr->Next;
	} else {
		ParticleHead = particleptr->Next;
	}
	if (particleptr->Next != NULL) {
		particleptr->Next->Prev = particleptr->Prev;
	}

	delete particleptr;

	// Decrement the no. of particles.
	ParticleCount--;

	// Decrement the no. of global weather particles.
	_GlobalParticleCount--;
}


/***********************************************************************************************
 * WeatherSystemClass::Render --																					  *
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
void WeatherSystemClass::Render (RenderInfoClass &rinfo)
{
	if (WW3D::Are_Static_Sort_Lists_Enabled()) {

		const unsigned sortlevel = 31;

		WW3D::Add_To_Static_Sort_List (this, sortlevel);

	} else {

		const Vector3 zero  (0.0f, 0.0f, 0.0f);
		const Vector3 white (1.0f, 1.0f, 1.0f);

		Vector3			 cameravelocity;
		Vector3			 color;
		unsigned			 dxcolor;
		Matrix4			 viewmatrix (true), identitymatrix (true);
		float				 maxalphaheight, oomaxalphaheight, deltaheight;
		Vector3			 x, y;
		float				 w, h;
		Vector3			 offset [VERTICES_PER_TRIANGLE];
		Vector3			 camerafocus;
		ParticleStruct *particleptr;
		unsigned			 processedparticlecount, bufferparticlecount;

		if (CameraPositionValid) {
			cameravelocity = ((rinfo.Camera.Get_Position() - CameraPosition) / (WW3D::Get_Frame_Time() * 0.001f));
		} else {
			cameravelocity.Set (0.0f, 0.0f, 0.0f);
			CameraPositionValid = true;
		}

		CameraPosition = rinfo.Camera.Get_Position();

		dxcolor = DX8Wrapper::Convert_Color (Vector3 (1.0f, 1.0f, 1.0f), 0.0f);

		// Precalculate alpha values.
		maxalphaheight	  = EmitterHeight * 0.2f;
		oomaxalphaheight = 1.0f / maxalphaheight;
		deltaheight		  = rinfo.Camera.Get_Position().Z + (EmitterHeight - maxalphaheight);

		// NOTE: All particle positions are already in world space.
		DX8Wrapper::Set_Transform (D3DTS_WORLD, identitymatrix);

		DX8Wrapper::Set_Material (Material);
		DX8Wrapper::Set_Shader (Shader);
		DX8Wrapper::Set_Texture (0, Texture);

		DX8Wrapper::Set_Index_Buffer (IndexBuffer, 0);

		#if WEATHER_PARTICLE_SORT
		#else
		float depthbias = -0.02f;
		DX8Wrapper::Set_DX8_Render_State(D3DRS_DEPTHBIAS,*reinterpret_cast<unsigned*>(&depthbias));
		#endif

 		camerafocus = rinfo.Camera.Get_Transform().Get_Z_Vector();
		particleptr = ParticleHead;
		processedparticlecount = 0;
		bufferparticlecount = MIN (MAX_IB_PARTICLE_COUNT, ParticleCount);
		while (processedparticlecount < ParticleCount) {

			unsigned particlecount, submittedparticlecount;

			#if WEATHER_PARTICLE_SORT
			DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_SORTING, dynamic_fvf_type, bufferparticlecount * VERTICES_PER_TRIANGLE);
			#else
			DynamicVBAccessClass dynamicvb (BUFFER_TYPE_DYNAMIC_DX8, dynamic_fvf_type, bufferparticlecount * VERTICES_PER_TRIANGLE);
			#endif

			// Copy the data into the sorting vertex buffer.
			particlecount = MIN (ParticleCount - processedparticlecount, MAX_IB_PARTICLE_COUNT);
			submittedparticlecount = 0;
			{
				DynamicVBAccessClass::WriteLockClass lock (&dynamicvb);

				VertexFormatXYZNDUV2 *vertex = lock.Get_Formatted_Vertex_Array();

				for (unsigned p = 0; p < particlecount; p++)	{

					Vector3	position;

					WWASSERT (particleptr != NULL);
					position = particleptr->CurrentPosition;

					// Optimization: only submit this particle for rendering if it is in the view frustum.
					if (CollisionMath::Overlap_Test (rinfo.Camera.Get_Frustum(), position) != CollisionMath::OUTSIDE) {

						unsigned base;
						float		alphaheight, alpha;

						switch (particleptr->RenderMode) {

							case RENDER_MODE_AXIS_ALIGNED:
								y = particleptr->Velocity - cameravelocity;
								y /= y.Quick_Length();
								x = Vector3::Cross_Product (camerafocus, y);
								x /= x.Quick_Length();
								w = HalfParticleWidth;
								h = HalfParticleHeight;
								break;

							case RENDER_MODE_CAMERA_ALIGNED:
								x = rinfo.Camera.Get_Transform().Get_X_Vector();
								y = rinfo.Camera.Get_Transform().Get_Y_Vector();
								w = HalfParticleWidth;
								h = HalfParticleHeight;
								break;

							case RENDER_MODE_SURFACE_ALIGNED:

								// Particle has an orientation so back-face cull it.
								if (Vector3::Dot_Product (camerafocus, particleptr->SurfaceNormal) > 0.0f) {

									x = Vector3::Cross_Product (camerafocus, particleptr->SurfaceNormal);
									x /= x.Quick_Length();
									y = Vector3::Cross_Product (x, particleptr->SurfaceNormal);

									if (DecayAfterCollision) {

										float decaytime, totaldecaytime, s;

										decaytime		= particleptr->ElapsedTime - particleptr->CollisionTime;
										totaldecaytime = particleptr->LifeTime - particleptr->CollisionTime;
										if ((decaytime >= 0.0f) && (totaldecaytime > 0.0f)) {
											s = 1.0f - (decaytime / totaldecaytime);
											w = HalfParticleWidth  * s;
											h = HalfParticleHeight * s;
										} else {
											w = h = 0.0f;
										}

									} else {
										w = HalfParticleWidth;
										h = HalfParticleHeight;
									}
									break;

								} else {

				  					// Advance to next particle.
									particleptr = particleptr->Next;

									continue;
								}

 							default:
								WWASSERT (false);
								w = h = 0.0f;
								break;
						}

						offset [0] = -h * y;
						offset [1] =  h * y + w * x;
						offset [2] =  h * y - w * x;

						base = particleptr->Page * VERTICES_PER_TRIANGLE;

						alphaheight = position.Z - deltaheight;
						if (alphaheight > 0.0f) {
							alpha = 1.0f - (alphaheight * oomaxalphaheight);
						} else {
							alpha = 1.0f;
						}
						DX8Wrapper::Set_Alpha (alpha, dxcolor);

						// Vertex 0 of triangle.
						vertex->x		 = position.X + offset [0].X;
						vertex->y		 = position.Y + offset [0].Y;
						vertex->z		 = position.Z + offset [0].Z;
						vertex->diffuse = dxcolor;
						vertex->u1		 = TextureArray [base].U;
						vertex->v1		 = TextureArray [base].V;
						vertex++;

						// Vertex 1 of triangle.
						vertex->x		 = position.X + offset [1].X;
						vertex->y		 = position.Y + offset [1].Y;
						vertex->z		 = position.Z + offset [1].Z;
						vertex->diffuse = dxcolor;
						vertex->u1		 = TextureArray [base + 1].U;
						vertex->v1		 = TextureArray [base + 1].V;
						vertex++;

						// Vertex 2 of triangle.
						vertex->x		 = position.X + offset [2].X;
						vertex->y		 = position.Y + offset [2].Y;
						vertex->z		 = position.Z + offset [2].Z;
						vertex->diffuse = dxcolor;
						vertex->u1		 = TextureArray [base + 2].U;
						vertex->v1		 = TextureArray [base + 2].V;
						vertex++;

						submittedparticlecount++;
					}

					// Advance to next particle.
					particleptr = particleptr->Next;
				}
			}

			if (submittedparticlecount > 0) {

				DX8Wrapper::Set_Vertex_Buffer (dynamicvb);

				#if WEATHER_PARTICLE_SORT
				SortingRendererClass::Insert_Triangles (0, submittedparticlecount, 0, submittedparticlecount * VERTICES_PER_TRIANGLE);
				#else
				DX8Wrapper::Draw_Triangles (0, submittedparticlecount, 0, submittedparticlecount * VERTICES_PER_TRIANGLE);
				#endif
			}

			processedparticlecount += particlecount;
		}

		WWASSERT (particleptr == NULL);

		#if WEATHER_PARTICLE_SORT
		#else
		DX8Wrapper::Set_DX8_Render_State (D3DRS_DEPTHBIAS, 0);
		#endif
	}
}


/***********************************************************************************************
 * WeatherSystemClass::Get_Obj_Space_Bounding_Sphere --													  *
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
void WeatherSystemClass::Get_Obj_Space_Bounding_Sphere (SphereClass &sphere) const
{
	sphere.Init ((ObjectMin + ObjectMax) * 0.5f, ((ObjectMax - ObjectMin) * 0.5f).Length());
}


/***********************************************************************************************
 * WeatherSystemClass::Get_Obj_Space_Bounding_Box --														  *
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
void WeatherSystemClass::Get_Obj_Space_Bounding_Box (AABoxClass &box) const
{
	box.Init ((ObjectMin + ObjectMax) * 0.5f, (ObjectMax - ObjectMin) * 0.5f);
}


/***********************************************************************************************
 * RainSystemClass::RainSystemClass --																			  *
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
RainSystemClass::RainSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass * /* wind */, SoundEnvironmentClass *soundenvironment, bool prime)
	: WeatherSystemClass (scene, 20.0f, 20.0f, particledensity, 0.2f, 0.15f, 0.45f, 15.0f, Vector2 (0.0f, 0.0f), Vector2 (1.0f, 0.5f), PAGE_COUNT, true, 0.1f, 0.2f, WeatherSystemClass::RENDER_MODE_AXIS_ALIGNED, false, prime),
	  SoundEnvironment (soundenvironment)
{
	const char *rainsamplename	= "Rainfall01";

	// Create the rain sound effect.
	// Optimization: Only add the sound effect if wind speed is non-zero.
	Sound = WWAudioClass::Get_Instance()->Create_Sound (rainsamplename, NULL, 0, CLASSID_2D);
	if (Sound != NULL) {
		SoundEnvironment->Add_User();
		Sound->Set_Volume (0.0f);
		Sound->Play();
 	}
}


/***********************************************************************************************
 * RainSystemClass::~RainSystemClass --																		  *
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
RainSystemClass::~RainSystemClass()
{
	// Remove rain sound effect.
	if (Sound != NULL) {
		Sound->Stop();
		REF_PTR_RELEASE (Sound);
		SoundEnvironment->Remove_User();
	}
}


/***********************************************************************************************
 * RainSystemClass::Update --																						  *
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
bool RainSystemClass::Update (WindClass *wind, const Vector3 &cameraposition)
{
	if (Sound != NULL) {

		const float maxvolume			= 4.0f;
		const float volumeperparticle	= 0.0025f;

		float attenuation;

		// Calculate sound attenuation based on no. of particles in system.
		attenuation = MIN (ParticleCount * volumeperparticle, maxvolume) / maxvolume;
		Sound->Set_Volume (SoundEnvironment->Get_Amplitude() * attenuation);
	}

	// Base class update.
	return (WeatherSystemClass::Update (wind, cameraposition));
}


/***********************************************************************************************
 * SnowSystemClass::SnowSystemClass --																			  *
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
SnowSystemClass::SnowSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass * /* wind */, bool prime)
	: WeatherSystemClass (scene, 40.0f, 20.0f, particledensity, 0.1f, 0.32f, 0.32f, 3.5f, Vector2 (0.0f, 0.5f), Vector2 (1.0f, 0.25f), PAGE_COUNT, false, 1.0f, 2.0f, WeatherSystemClass::RENDER_MODE_CAMERA_ALIGNED, true, prime)
{
}


/***********************************************************************************************
 * SnowSystemClass::Update --																						  *
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
bool SnowSystemClass::Update (WindClass *wind, const Vector3 &cameraposition)
{
	// Base class update.
	return (WeatherSystemClass::Update (wind, cameraposition));
}


/***********************************************************************************************
 * AshSystemClass::AshSystemClass --																			  *
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
AshSystemClass::AshSystemClass (PhysicsSceneClass *scene, float particledensity, WindClass * /* wind */, bool prime)
	: WeatherSystemClass (scene, 40.0f, 20.0f, particledensity, 0.1f, 0.32f, 0.32f, 3.0f, Vector2 (0.0f, 0.75f), Vector2 (1.0f, 0.25f), PAGE_COUNT, false, 1.0f, 2.0f, WeatherSystemClass::RENDER_MODE_CAMERA_ALIGNED, true, prime)
{
}


/***********************************************************************************************
 * AshSystemClass::Update --																						  *
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
bool AshSystemClass::Update (WindClass *wind, const Vector3 &cameraposition)
{
	// Base class update.
	return (WeatherSystemClass::Update (wind, cameraposition));
}


/***********************************************************************************************
 * WeatherParameterClass::Initialize --																		  *
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
void WeatherParameterClass::Initialize()
{
	CurrentValue	  = 0.0f;
	NormalTarget	  = 0.0f;
	NormalDuration	  = 0.0f;
	OverrideTarget	  = 0.0f;
	OverrideDuration = 0.0f;
}


/***********************************************************************************************
 * WeatherParameterClass::Set --																					  *
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
void WeatherParameterClass::Set (float target, float ramptime, bool override)
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
 * WeatherParameterClass::Update --																				  *
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
bool WeatherParameterClass::Update (float time, bool override)
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
 * WeatherParameterClass::Update --																				  *
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
void WeatherParameterClass::Update (float &value, float &target, float &duration, float time)
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
 * WeatherMgrClass::WeatherMgrClass --																			  *
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
WeatherMgrClass::WeatherMgrClass()
{
	Set_Network_ID (NETID_SERVER_WEATHER);
	Set_App_Packet_Type (APPPACKETTYPE_NETWEATHER);
}


/***********************************************************************************************
 * WeatherMgrClass::Init --																						  *
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
void WeatherMgrClass::Init (SoundEnvironmentClass *soundenvironment)
{
	WWASSERT (soundenvironment != NULL);

	REF_PTR_SET (_SoundEnvironment, soundenvironment);

	_Wind = NULL;
	for (int p = PRECIPITATION_FIRST; p < PRECIPITATION_COUNT; p++) {
		_Precipitation [p] = NULL;
	}

	Reset();
}


/***********************************************************************************************
 * WeatherMgrClass::Reset --																						  *
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
void WeatherMgrClass::Reset()
{
	int p;

	// Iterate and initialize the parameters.
	for (p = 0; p < PARAMETER_COUNT; p++) {
		_Parameters [p].Initialize();
	}

	if (_Wind != NULL) {
		delete _Wind;
		_Wind = NULL;
	}

	//	Restore the settings to default.
	Set_Wind (0.0f, 0.0f, 0.0f, 0.0f, false);
	for (p = PRECIPITATION_FIRST; p < PRECIPITATION_COUNT; p++) {
		if (_Precipitation [p] != NULL) {
			_Precipitation [p]->Remove();
  			REF_PTR_RELEASE (_Precipitation [p]);
			Set_Precipitation ((PrecipitationEnum) p, 0.0f, 0.0f, false);
		}
	}
	Set_Fog_Enable (false);
	Set_Fog_Range (200.0f, 300.0f);

	_Prime = true;
	_Imported = false;
	_WindOverrideCount = 0;
	_PrecipitationOverrideCount = 0;

	Set_Dirty();
}


/***********************************************************************************************
 * WeatherMgrClass::Shutdown --																					  *
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
void WeatherMgrClass::Shutdown()
{
	Reset();
	REF_PTR_RELEASE (_SoundEnvironment);
}


/***********************************************************************************************
 * WeatherMgrClass::Set_Wind --																					  *
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
bool WeatherMgrClass::Set_Wind (float heading, float speed, float variability, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Wind (heading, speed, variability, ramptime, false)) {
			_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Override_Wind --																			  *
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
bool WeatherMgrClass::Override_Wind (float heading, float speed, float variability, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_WindOverrideCount++;
		if (Set_Wind (heading, speed, variability, ramptime, true)) {
			_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Set_Wind --																					  *
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
bool WeatherMgrClass::Set_Wind (float heading, float speed, float variability, float ramptime, bool override)
{
	if ((heading >= 0.0f) && (heading <= 360.0f) && (speed >= 0.0f) && (variability >= 0.0f) &&
		(variability <= 1.0f) && (ramptime >= 0.0f)) {

		bool o;

		o = (_WindOverrideCount > 0) && override;
		_Parameters [PARAMETER_WIND_HEADING].Set (heading, ramptime, o);
		_Parameters [PARAMETER_WIND_SPEED].Set (speed, ramptime, o);
		_Parameters [PARAMETER_WIND_VARIABILITY].Set (variability, ramptime, o);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Get_Wind --																					  *
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
void WeatherMgrClass::Get_Wind (float &heading, float &speed, float &variability)
{
	heading		= _Parameters [PARAMETER_WIND_HEADING].Value();
	speed			= _Parameters [PARAMETER_WIND_SPEED].Value();
	variability = _Parameters [PARAMETER_WIND_VARIABILITY].Value();
}


/***********************************************************************************************
 * WeatherMgrClass::Restore_Wind --																				  *
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
void WeatherMgrClass::Restore_Wind (float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_Parameters [PARAMETER_WIND_HEADING].Set (ramptime);
		_Parameters [PARAMETER_WIND_SPEED].Set (ramptime);
		_Parameters [PARAMETER_WIND_VARIABILITY].Set (ramptime);
		_WindOverrideCount--;
		_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/***********************************************************************************************
 * WeatherMgrClass::Set_Precipitation --																		  *
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
bool WeatherMgrClass::Set_Precipitation (PrecipitationEnum precipitation, float density, float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		if (Set_Precipitation (precipitation, density, ramptime, false)) {
			_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
			return (true);
		}
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Override_Precipitation --																  *
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
bool WeatherMgrClass::Override_Precipitation (PrecipitationEnum precipitation, float density, float ramptime)
{
	if (CombatManager::I_Am_Server()) {

		bool success;

		_PrecipitationOverrideCount++;

		// Override requested precipitation but also override and ramp down any other types of precipitation that may exist.
		success = true;
		for (int p = PRECIPITATION_FIRST; p < PRECIPITATION_COUNT; p++) {
			if (p != precipitation) {
				success &= Set_Precipitation ((PrecipitationEnum) p, 0.0f, ramptime, true);
			} else {
				success &= Set_Precipitation ((PrecipitationEnum) p, density, ramptime, true);
			}
		}
		_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
		return (success);
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Set_Precipitation --																		  *
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
bool WeatherMgrClass::Set_Precipitation (PrecipitationEnum precipitation, float density, float ramptime, bool override)
{
	if ((density >= 0.0f) && (ramptime >= 0.0f)) {

		const bool o = (_PrecipitationOverrideCount > 0) && override;

		switch (precipitation) {

			case PRECIPITATION_RAIN:
				_Parameters [PARAMETER_RAIN_DENSITY].Set (density, ramptime, o);
				break;

			case PRECIPITATION_SNOW:
				_Parameters [PARAMETER_SNOW_DENSITY].Set (density, ramptime, o);
				break;

			case PRECIPITATION_ASH:
				_Parameters [PARAMETER_ASH_DENSITY].Set (density, ramptime, o);
				break;

			default:
				WWASSERT (false);
				break;
		}
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Get_Precipitation --																		  *
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
void WeatherMgrClass::Get_Precipitation (PrecipitationEnum precipitation, float &density)
{
	switch (precipitation) {

		case PRECIPITATION_RAIN:
			density = _Parameters [PARAMETER_RAIN_DENSITY].Value();
			break;

		case PRECIPITATION_SNOW:
			density = _Parameters [PARAMETER_SNOW_DENSITY].Value();
			break;

		case PRECIPITATION_ASH:
			density = _Parameters [PARAMETER_ASH_DENSITY].Value();
			break;

		default:
			WWASSERT (false);
			break;
	}
}


/***********************************************************************************************
 * WeatherMgrClass::Restore_Precipitation --																	  *
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
void WeatherMgrClass::Restore_Precipitation (float ramptime)
{
	if (CombatManager::I_Am_Server()) {
		_Parameters [PARAMETER_RAIN_DENSITY].Set (ramptime);
		_Parameters [PARAMETER_SNOW_DENSITY].Set (ramptime);
		_Parameters [PARAMETER_ASH_DENSITY].Set (ramptime);
		_PrecipitationOverrideCount--;
		_TheWeatherMgr.Set_Object_Dirty_Bit (NetworkObjectClass::BIT_RARE, true);
	}
}


/***********************************************************************************************
 * WeatherMgrClass::Set_Fog_Range --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/12/01    IML : Created.                                                                *
 *=============================================================================================*/
bool WeatherMgrClass::Set_Fog_Range (float startdistance, float enddistance, float ramptime)
{
	if ((startdistance >= 0.0f) && (enddistance >= startdistance)) {
		_Parameters [PARAMETER_FOG_START_DISTANCE].Set (startdistance, ramptime, false);
		_Parameters [PARAMETER_FOG_END_DISTANCE].Set (enddistance, ramptime, false);
		return (true);
	}
	return (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Get_Fog_Range --																			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/12/01    IML : Created.                                                                *
 *=============================================================================================*/
void WeatherMgrClass::Get_Fog_Range (float &startdistance, float &enddistance)
{
	startdistance = _Parameters [PARAMETER_FOG_START_DISTANCE].Value();
	enddistance	  = _Parameters [PARAMETER_FOG_END_DISTANCE].Value();
}


/***********************************************************************************************
 * WeatherMgrClass::Update --																						  *
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
void WeatherMgrClass::Update (PhysicsSceneClass *scene, CameraClass *camera)
{
	float	time;
	bool	windmodified, fogmodified;

	time = WW3D::Get_Frame_Time() * 0.001f;

	// Update wind.
	windmodified  = _Parameters [PARAMETER_WIND_HEADING].Update (time, _WindOverrideCount > 0);
	windmodified |= _Parameters [PARAMETER_WIND_SPEED].Update (time, _WindOverrideCount > 0);
	windmodified |= _Parameters [PARAMETER_WIND_VARIABILITY].Update (time, _WindOverrideCount > 0);
	if (_Wind != NULL) {
		if (windmodified) {
			_Wind->Set (_Parameters [PARAMETER_WIND_HEADING].Value(), _Parameters [PARAMETER_WIND_SPEED].Value(), _Parameters [PARAMETER_WIND_VARIABILITY].Value());
		}

		// Optimization: if there is nothing to update, can safely remove the wind.
		if (!_Wind->Update()) {
			delete _Wind;
			_Wind = NULL;
		}

	} else {

	  	// Optimization: only create wind if speed is non-zero.
		if (_Parameters [PARAMETER_WIND_SPEED].Value() > 0.0f) {
			_Wind = new WindClass (_Parameters [PARAMETER_WIND_HEADING].Value(), _Parameters [PARAMETER_WIND_SPEED].Value(), _Parameters [PARAMETER_WIND_VARIABILITY].Value(), _SoundEnvironment);
		}
	}

	for (int p = PRECIPITATION_FIRST; p < PRECIPITATION_COUNT; p++) {

		WeatherParameterClass *parameterptr = NULL;
		bool						  modified;

		switch (p) {

		  	case PRECIPITATION_RAIN:
				parameterptr = &_Parameters [PARAMETER_RAIN_DENSITY];
				break;

		  	case PRECIPITATION_SNOW:
				parameterptr = &_Parameters [PARAMETER_SNOW_DENSITY];
				break;

		  	case PRECIPITATION_ASH:
				parameterptr = &_Parameters [PARAMETER_ASH_DENSITY];
				break;

			default:
				WWASSERT (false);
				break;
		}

		modified = parameterptr->Update (time, _PrecipitationOverrideCount > 0);
	  	if (_Precipitation [p] != NULL) {
	  		if (modified) {
	  			_Precipitation [p]->Set_Density (parameterptr->Value());
	  		}

	  		// Optimization: if there is nothing to update, can safely remove the weather system.
	  		if (!_Precipitation [p]->Update (_Wind, camera->Get_Position())) {
				_Precipitation [p]->Remove();
	  			REF_PTR_RELEASE (_Precipitation [p]);
	  		}

	  	} else {

			// Optimization: only create weather if density is non-zero.
	  	  	if (parameterptr->Value() > 0.0f) {

				// Don't bother on dedicated servers.
				if (!CombatManager::I_Am_Only_Server()) {

					switch (p) {

	  					case PRECIPITATION_RAIN:
	  						_Precipitation [p] = NEW_REF (RainSystemClass, (scene, parameterptr->Value(), _Wind, _SoundEnvironment, _Prime));
	  						break;

	  					case PRECIPITATION_SNOW:
	  						_Precipitation [p] = NEW_REF (SnowSystemClass, (scene, parameterptr->Value(), _Wind, _Prime));
	  						break;

	  					case PRECIPITATION_ASH:
	  						_Precipitation [p] = NEW_REF (AshSystemClass, (scene, parameterptr->Value(), _Wind, _Prime));
	  						break;

						default:
							WWASSERT (false);
							break;
					}
					scene->Add_Render_Object (_Precipitation [p]);
				}
			}
		}
	}

	fogmodified  = _Parameters [PARAMETER_FOG_START_DISTANCE].Update (time, false);
	fogmodified |= _Parameters [PARAMETER_FOG_END_DISTANCE].Update (time, false);
	if (Is_Dirty() || fogmodified) {
		scene->Set_Fog_Enable (_FogEnabled);
		scene->Set_Fog_Range (_Parameters [PARAMETER_FOG_START_DISTANCE].Value(), _Parameters [PARAMETER_FOG_END_DISTANCE].Value());
	}

	if (CombatManager::I_Am_Server()) {

		// Server precipitation can only be primed on the first update iteration of a level.
		_Prime = false;

	} else {

		// Clients can be primed right up until the first import from the server.
		if (_Imported) _Prime = false;
	}

	// Everything necessary has been updated. Clear the dirty flag.
	Set_Dirty (false);
}


/***********************************************************************************************
 * WeatherMgrClass::Save --																						  *
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
 #define WRITE_PARAMETER(varname) \
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _CURRENT_VALUE, _Parameters [PARAMETER_ ## varname].CurrentValue); \
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_VALUE, _Parameters [PARAMETER_ ## varname].NormalValue);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_TARGET, _Parameters [PARAMETER_ ## varname].NormalTarget);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _NORMAL_DURATION, _Parameters [PARAMETER_ ## varname].NormalDuration);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _OVERRIDE_TARGET, _Parameters [PARAMETER_ ## varname].OverrideTarget);	\
WRITE_MICRO_CHUNK (csave, VARID_ ## varname ## _OVERRIDE_DURATION, _Parameters [PARAMETER_ ## varname].OverrideDuration)

bool WeatherMgrClass::Save (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_MICRO_CHUNKS);
	csave.End_Chunk ();

	Save_Dynamic (csave);
	return (true);
}


/***********************************************************************************************
 * WeatherMgrClass::Save_Dynamic --																						  *
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
bool WeatherMgrClass::Save_Dynamic (ChunkSaveClass &csave)
{
	csave.Begin_Chunk (CHUNKID_DYNAMIC_MICRO_CHUNKS);
	WRITE_PARAMETER (WIND_HEADING);
	WRITE_PARAMETER (WIND_SPEED);
	WRITE_PARAMETER (WIND_VARIABILITY);
	WRITE_PARAMETER (RAIN_DENSITY);
	WRITE_PARAMETER (SNOW_DENSITY);
	WRITE_PARAMETER (ASH_DENSITY);
	WRITE_MICRO_CHUNK (csave, VARID_WIND_OVERRIDE_COUNT, _WindOverrideCount);
	WRITE_MICRO_CHUNK (csave, VARID_PRECIPITATION_OVERRIDE_COUNT, _PrecipitationOverrideCount);
	WRITE_MICRO_CHUNK (csave, VARID_FOG_ENABLED, _FogEnabled);
	WRITE_PARAMETER (FOG_START_DISTANCE);
	WRITE_PARAMETER (FOG_END_DISTANCE);
	csave.End_Chunk ();
	return (true);
}


/***********************************************************************************************
 * WeatherMgrClass::Load --																						  *
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
bool WeatherMgrClass::Load (ChunkLoadClass &cload)
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
 * WeatherMgrClass::Load_Micro_Chunks --																		  *
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
#define READ_PARAMETER(varname) \
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _CURRENT_VALUE, _Parameters [PARAMETER_ ## varname].CurrentValue); \
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_VALUE, _Parameters [PARAMETER_ ## varname].NormalValue);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_TARGET, _Parameters [PARAMETER_ ## varname].NormalTarget);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _NORMAL_DURATION, _Parameters [PARAMETER_ ## varname].NormalDuration);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _OVERRIDE_TARGET, _Parameters [PARAMETER_ ## varname].OverrideTarget);	\
READ_MICRO_CHUNK (cload, VARID_ ## varname ## _OVERRIDE_DURATION, _Parameters [PARAMETER_ ## varname].OverrideDuration)

bool WeatherMgrClass::Load_Micro_Chunks (ChunkLoadClass &cload)
{
	// Read weather micro chunk.
	while (cload.Open_Micro_Chunk ()) {
		cload.Close_Micro_Chunk ();
	}

	return (true);
}


/***********************************************************************************************
 * WeatherMgrClass::Load_Dynamic --																						  *
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
bool WeatherMgrClass::Load_Dynamic (ChunkLoadClass &cload)
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
 * WeatherMgrClass::Load_Dynamic_Micro_Chunks --																		  *
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
bool WeatherMgrClass::Load_Dynamic_Micro_Chunks (ChunkLoadClass &cload)
{
	// Read weather micro chunk.
	while (cload.Open_Micro_Chunk ()) {
		switch (cload.Cur_Micro_Chunk_ID ()) {
			READ_PARAMETER (WIND_HEADING);
			READ_PARAMETER (WIND_SPEED);
			READ_PARAMETER (WIND_VARIABILITY);
			READ_PARAMETER (RAIN_DENSITY);
			READ_PARAMETER (SNOW_DENSITY);
			READ_PARAMETER (ASH_DENSITY);
			READ_MICRO_CHUNK (cload, VARID_WIND_OVERRIDE_COUNT, _WindOverrideCount);
			READ_MICRO_CHUNK (cload, VARID_PRECIPITATION_OVERRIDE_COUNT, _PrecipitationOverrideCount);
			READ_MICRO_CHUNK (cload, VARID_FOG_ENABLED, _FogEnabled);
			READ_PARAMETER (FOG_START_DISTANCE);
			READ_PARAMETER (FOG_END_DISTANCE);
		}
		cload.Close_Micro_Chunk ();
	}

	return (true);
}


/***********************************************************************************************
 * WeatherMgrClass::Export_Rare --																				  *
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

void WeatherMgrClass::Export_Rare (BitStreamClass &packet)
{
	WWASSERT (CombatManager::I_Am_Server());

	EXPORT_PARAMETER (packet, WIND_HEADING);
	EXPORT_PARAMETER (packet, WIND_SPEED);
	EXPORT_PARAMETER (packet, WIND_VARIABILITY);
	EXPORT_PARAMETER (packet, RAIN_DENSITY);
	EXPORT_PARAMETER (packet, SNOW_DENSITY);
	EXPORT_PARAMETER (packet, ASH_DENSITY);
	packet.Add (_WindOverrideCount);
	packet.Add (_PrecipitationOverrideCount);
}


/***********************************************************************************************
 * WeatherMgrClass::Import_Rare --																				  *
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

void WeatherMgrClass::Import_Rare (BitStreamClass &packet)
{
	WWASSERT (CombatManager::I_Am_Client());

	IMPORT_PARAMETER (packet, WIND_HEADING);
	IMPORT_PARAMETER (packet, WIND_SPEED);
	IMPORT_PARAMETER (packet, WIND_VARIABILITY);
	IMPORT_PARAMETER (packet, RAIN_DENSITY);
	IMPORT_PARAMETER (packet, SNOW_DENSITY);
	IMPORT_PARAMETER (packet, ASH_DENSITY);
	packet.Get (_WindOverrideCount);
	packet.Get (_PrecipitationOverrideCount);

	// Flag that weather data has been imported.
	_Imported = true;
}


#undef EXPORT_PARAMETER
#undef IMPORT_PARAMETER
#undef READ_PARAMETER
#undef WRITE_PARAMETER

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

/***************************************************************************************
**
**  OBSOLETE CODE!!!!!
**
**  THIS CODE IS JUST HERE IN CASE THERE IS SOME USEFUL IDEA IN IT THAT I MAY HAVE
**  FORGOTTEN.  PROBABLY NOT!
**
***************************************************************************************/

/*
** This is a timestep function for RigidBodyClass that tried to do orientation and
** position updates independently when a collision occured
*/
#if 0
void RigidBodyClass::Timestep(float dt)
{
	const int MAX_BUMPS = 5;
	Assert_State_Valid();
	LastTimestep = dt;

	/*
	** Repeat until we eat all of the time
	*/
	float remaining_time = dt;
	float timestep;
	int bumps = 0;

	Inc_Ignore_Counter();

	while ((remaining_time > 0.0f) && (bumps < MAX_BUMPS)) {

		timestep = remaining_time;
		bumps++;

		CastResultStruct result;
		OBBoxClass obbox;

		/*
		** Integrate the state of the object
		*/
		Assert_State_Valid();
		RigidBodyStateStruct oldstate = State;
		Integrate(timestep);
		Assert_State_Valid();

		/*
		** sample the start and end positions and orientations
		*/
		Vector3 p0 = oldstate.Position;
		Vector3 p1 = State.Position;
		Quaternion q0 = oldstate.Orientation;
		Quaternion q1 = State.Orientation;

		/*
		** rotate in place
		*/
		Matrix3D rotate_in_place(q1,p0);
		Model->Set_Transform(rotate_in_place);
		obbox = Box->Get_Box();

		result.Reset();
		PhysOBBoxCollisionTestClass boxrotate(	obbox,
															Vector3(0,0,0),
															&result,
															Get_Collision_Group(),
															COLLISION_TYPE_PHYSICAL);

		PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxrotate);
		if (result.StartBad) {
			q1 = q0;
			State.Orientation = q0;
			State.AMomentum /= 4.0f;
			StickCount++;
		} else {
			StickCount = 0;
		}

		/*
		** now translate
		*/
		Matrix3D initial_position(q1,p0);
		Vector3 move(p1 - p0);
		Model->Set_Transform(initial_position);
		obbox = Box->Get_Box();

		result.Reset();
		result.ComputePoint = true;
		PhysOBBoxCollisionTestClass boxmove(	obbox,
															move,
															&result,
															Get_Collision_Group(),
															COLLISION_TYPE_PHYSICAL);
		PhysicsSceneClass::Get_Instance()->Cast_OBBox(boxmove);

		if (result.StartBad) {
			remaining_time = 0;
			State = oldstate;
			//goto do_it_again;

		} else {

			if (result.Fraction == 1.0f) {

				// accepting the entire move
				remaining_time -= timestep;
				State.Position = p0 + 0.99f * move;

			} else {

				// IMPACT!
				// only take the fraction of time
				remaining_time -= result.Fraction * timestep;

				// only move up to the collision point, minus a padding amount
#if 0
				float len = (result.Fraction * move).Length();
				len -= MOVE_BUFFER;
				if (len > 0.0f) {
					move.Normalize();
					move *= len;
					State.Position = p0 + move;
				} else {
					State.Position = p0;
				}
#else
				State.Position = p0 + 0.99f * result.Fraction * move;
#endif

				// interpolate the momenta to approximate their state at time of impact
				Assert_State_Valid();
				State.LMomentum = oldstate.LMomentum + (State.LMomentum - oldstate.LMomentum) * result.Fraction;
				State.AMomentum = oldstate.AMomentum + (State.AMomentum - oldstate.AMomentum) * result.Fraction;
				Assert_State_Valid();

				// TODO: support multiple contact points!
				ContactCount = 1;
				ContactPoint[0] = result.Point;

				Vector3 impulse;
				Compute_Impact(result,&impulse);

				Vector3 padot0,padot1;
				Compute_Point_Velocity(result.Point,&padot0);

				Assert_State_Valid();
				Apply_Impulse(impulse,result.Point);
				Assert_State_Valid();

Assert_State_Valid();
// E3Demo!!!
if (StickCount > 3) {
	State.AMomentum *= 0.5f;
	State.Position.Z += 0.1f;
	Update_Auxiliary_State();
	Apply_Impulse(result.Normal * 0.25f * Get_Weight());
}
Assert_State_Valid();


			}
		}

		Update_Auxiliary_State();
	}

	Dec_Ignore_Counter();
	WWASSERT(Model);
	Model->Set_Transform(Matrix3D(Rotation,State.Position));
	Update_Cull_Box();

	Assert_State_Valid();


	if (ContactBox != NULL) {
		ContactBox->Compute_Forces();
	}

#ifdef WWDEBUG
	if(Is_Debug_Display_Enabled()) {
		Add_Debug_Vector(State.Position,Velocity,LMOMENTUM_COLOR);
		Add_Debug_Vector(State.Position,AngularVelocity,AMOMENTUM_COLOR);
	}
#endif

}
#endif


/*
** This was a version of Timestep which used a binary search through time to try
** and find the "exact" time of collision.  What a collosal waste of time!
*/
#if 0
void RigidBodyClass::Timestep(float dt)
{

	if ( Is_Immovable() ) {
		return;
	}

	WWASSERT(Model);
	WWASSERT(ContactBox);

	Inc_Ignore_Counter();

#ifdef RBODY_DEBUGGING
	WWDEBUG_SAY(("------------------------------\r\n"));
	WWDEBUG_SAY(("RigidBody Timestep Begin.\r\n"));
	WWASSERT(!ContactBox->Is_Intersecting());
#endif

	/*
	** Repeat until we eat all of the time
	*/
	const int MAX_COLLISIONS = 100;
	int collisions = 0;
	float remaining_time = dt;
	float timestep;

#ifdef RBODY_DEBUGGING
	WWDEBUG_SAY(("clock counter begin:\r\n"));
#endif
	while ((remaining_time > 0.0f) && (collisions < MAX_COLLISIONS)) {

		timestep = remaining_time;

		/*
		** Integrate the state of the object
		*/
#ifdef RBODY_DEBUGGING
		WWDEBUG_SAY(("Computing new state, delta_t = %f:\r\n",timestep));
#endif
		RigidBodyStateStruct oldstate = State;
		Integrate(timestep);

#ifdef RBODY_DEBUGGING
		WWDEBUG_SAY(("Done.\r\n\r\n"));
#endif
		/*
		** Check the final state of the object for collision.
		*/
		if (!ContactBox->Is_Intersecting()) {

			/*
			** Not intersecting so we're accepting the entire move
			*/
			remaining_time -= timestep;
			StickCount = 0;
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("No Collision\r\n"));
#endif

		} else {
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("Collision!  Searching for TOC.\r\n"));
#endif
			/*
			** Sample the start and end positions and orientations.
			*/
			Vector3 p0 = oldstate.Position;
			Vector3 p1 = State.Position;
			Quaternion q0 = oldstate.Orientation;
			Quaternion q1 = State.Orientation;

			/*
			** Assert that our start point was not intersecting.
			*/
#ifdef WWDEBUG
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("Prev Position: %f %f %f\r\n",oldstate.Position.X,oldstate.Position.Y,oldstate.Position.Z));
			WWDEBUG_SAY(("New Position: %f %f %f\r\n",State.Position.X,State.Position.Y,State.Position.Z));
			WWDEBUG_SAY(("Prev Orientation: %f %f %f %f\r\n",oldstate.Orientation.X,oldstate.Orientation.Y,oldstate.Orientation.Z,oldstate.Orientation.W));
			WWDEBUG_SAY(("New Orientation: %f %f %f %f\r\n",State.Orientation.X,State.Orientation.Y,State.Orientation.Z,State.Orientation.W));

			ContactBox->Set_Transform(q0,p0);
			WWASSERT(!ContactBox->Is_Intersecting());
#endif
#endif

			/*
			** Binary search to find the time of collision
			*/
			float t = 0.0f;
			float t0 = 0.0f;
			float t1 = 1.0f;
			const float COLLISION_EPSILON = 0.01f;//(float)WWMATH_EPSILON;
			Vector3 p;
			Quaternion q;

			SlerpInfoStruct slerpinfo;
			Slerp_Setup(q0,q1,&slerpinfo);

			while (t1 - t0 > COLLISION_EPSILON) {

				t = (t1 + t0) / 2.0f;

				/*
				** Compute interpolated orientation and position
				*/
				Lerp(p0,p1,t,&p);
				Cached_Slerp(q0,q1,t,&slerpinfo,&q);
				ContactBox->Set_Transform(q,p);

				/*
				** Test for collision here
				*/
#ifdef RBODY_DEBUGGING
				bool intersecting = ContactBox->Is_Intersecting();
				WWDEBUG_SAY(("   t0 = %f  t1 = %f  t=%f   %s\r\n",t0,t1,t,(intersecting ? "colliding" : "not-colliding")));
#endif
				/*
				** Zero in on the TOC
				*/
				if (intersecting) {
					t1 = t;
				} else {
					t0 = t;
				}
			}

			/*
			** t0 is guaranteed to be non-intersecting so use it for TOC
			*/
			t = t0;
			remaining_time -= t * timestep;
			if (t == 0.0f) {
				remaining_time -= 0.001f;
			}
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("Found collision at fraction: %f\r\n",t));
#endif
			/*
			** Compute the state right before the collision
			*/
			Cached_Slerp(q0,q1,t,&slerpinfo,&State.Orientation);
			Lerp(p0,p1,t,&State.Position);
			Lerp(oldstate.LMomentum,State.LMomentum,t,&State.LMomentum);
			Lerp(oldstate.AMomentum,State.AMomentum,t,&State.AMomentum);
			Update_Auxiliary_State();

			//ContactBox->Set_Transform(Matrix3D(State.Orientation,State.Position));

#ifdef RBODY_DEBUGGING
//			WWDEBUG_SAY(("Position: %f %f %f\r\n",State.Position.X,State.Position.Y,State.Position.Z));
//			WWDEBUG_SAY(("Orientation: %f %f %f %f\r\n",State.Orientation.X,State.Orientation.Y,State.Orientation.Z,State.Orientation.W));
			WWASSERT(!ContactBox->Is_Intersecting());
#endif

			/*
			** Find the collision point and normal and apply an impulse to the object.
			*/
			ContactBox->Compute_Contacts();


#if 0   // Compute an impulse for the worst contact

			Vector3 point;
			Vector3 normal;

			for (int i=0; i<ContactBox->Get_Contact_Count(); i++) {
				if (ContactBox->Get_Contact(i).Fraction < frac) {
					point = ContactBox->Get_Contact(i).Point;
					normal = ContactBox->Get_Contact(i).Normal;
				};
			}

			Vector3 impulse;

//			Compute_Impact(point,normal,&impulse);
// DEBUG DEBUG apply impulse only at CM!
			Compute_Impact(State.Position,normal,&impulse);
			Apply_Impulse(impulse,point);
#ifdef RBODY_DEBUGGING
			DEBUG_RENDER_POINT(point,Vector3(1,0,0));
			WWDEBUG_SAY(("Applied Impulse: %f %f %f\r\n",impulse.X,impulse.Y,impulse.Z));
#endif
#endif
#if 1		// Simultaneously compute impulses for all contacts

			if (ContactBox->Get_Contact_Count() > 0) {

				int i;
				Vector3 * impulses = new Vector3[ContactBox->Get_Contact_Count()];
				for (i=0; i<ContactBox->Get_Contact_Count(); i++) {

					Vector3 point;
					Vector3 normal;
					point = ContactBox->Get_Contact(i).Point;
					normal = ContactBox->Get_Contact(i).Normal;
					Compute_Impact(point,normal,&(impulses[i]));

				}

				for (i=0; i<ContactBox->Get_Contact_Count(); i++) {

					Vector3 point;
					point = ContactBox->Get_Contact(i).Point;
					impulses[i] /= ContactBox->Get_Contact_Count();
					Apply_Impulse(impulses[i],point);
					Assert_State_Valid();
				}
			}

#endif
#if 0		// Repeatedly compute impulses until the object is happy.

			int cur_contact = 0;
			int impact_count = 0;
			bool done = false;
			while (!done && (impact_count < 100)) {

				Vector3 impulse;
				const Vector3 & point = ContactBox->Get_Contact(cur_contact).Point;
				const Vector3 & normal = ContactBox->Get_Contact(cur_contact).Normal;

				if (Is_Colliding(point,normal)) {
					Compute_Impact(point,normal,&impulse);
					Apply_Impulse(impulse,point);
				}

				done = true;
				for (int i=0; i<ContactBox->Get_Contact_Count(); i++) {
					if (Is_Colliding(ContactBox->Get_Contact(i).Point,ContactBox->Get_Contact(i).Normal)) {
						done = false;
					}
				}
				cur_contact = (cur_contact + 1) % ContactBox->Get_Contact_Count();
				impact_count++;
			}
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("Number of impacts to resolve collision: %d\r\n",impact_count));
#endif
#endif
#ifdef RBODY_DEBUGGING
			WWDEBUG_SAY(("\r\n"));
			WWDEBUG_SAY(("Position: %f %f %f\r\n",State.Position.X,State.Position.Y,State.Position.Z));
			WWDEBUG_SAY(("Orientation: %f %f %f %f\r\n",State.Orientation.X,State.Orientation.Y,State.Orientation.Z,State.Orientation.W));
#endif
		}
		collisions++;
	}

#ifdef RBODY_DEBUGGING
	WWASSERT(!ContactBox->Is_Intersecting());
	WWDEBUG_SAY(("RigidBody Timestep End.\r\n"));
	WWDEBUG_SAY(("\r\n"));
#endif

#ifdef WWDEBUG
	if(Is_Debug_Display_Enabled()) {
		Add_Debug_Vector(State.Position,Velocity,LMOMENTUM_COLOR);
		Add_Debug_Vector(State.Position,AngularVelocity,AMOMENTUM_COLOR);
	}
#endif

	Dec_Ignore_Counter();
	Model->Set_Transform(Matrix3D(Rotation,State.Position));
	Update_Cull_Box();
}
#endif //0

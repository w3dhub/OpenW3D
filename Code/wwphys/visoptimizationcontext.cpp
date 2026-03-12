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
 *                 Project Name : WWPhys                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwphys/visoptimizationcontext.cpp            $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Patrick                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/16/00 7:31p                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "visoptimizationcontext.h"
#include "visoptprogress.h"
#include "pscene.h"
#include "vistablemgr.h"
#include "vistable.h"
#include "dynamicaabtreecull.h"
#include "wwdebug.h"


const float MIN_OBJECT_MATCH_FRACTION = 0.99f;
const float MIN_SECTOR_MATCH_FRACTION = 0.99f;
const float MIN_PRUNE_MATCH_FRACTION = 0.90f;


/***************************************************************************************************
**
** PVSInfoStruct Implementation
**
***************************************************************************************************/

VisOptimizationContextClass::PVSInfoStruct::PVSInfoStruct(void) :
	Table(NULL),
	UnUsed(false)
{
}

VisOptimizationContextClass::PVSInfoStruct::~PVSInfoStruct(void)
{
	REF_PTR_RELEASE(Table);
}

const VisOptimizationContextClass::PVSInfoStruct &
VisOptimizationContextClass::PVSInfoStruct::operator = (const PVSInfoStruct & that)
{
	REF_PTR_SET(Table,that.Table);
	UnUsed = that.UnUsed;
	return *this;
}

/***************************************************************************************************
**
** VisOptimizationContextClass Implementation
**
***************************************************************************************************/

VisOptimizationContextClass::VisOptimizationContextClass
(
	PhysicsSceneClass * scene,
	VisOptProgressClass & stats
) :
	MinDynCellPruneMatchFraction(MIN_PRUNE_MATCH_FRACTION),
	MinVisObjectMatchFraction(MIN_OBJECT_MATCH_FRACTION),
	MinVisSectorMatchFraction(MIN_SECTOR_MATCH_FRACTION),
	Scene(scene),
	Stats(stats)
{
	WWASSERT(Scene != NULL);
}

VisOptimizationContextClass::~VisOptimizationContextClass(void)
{
}

float VisOptimizationContextClass::Compute_Sector_Table_Match_Fraction(int sector_id_0,int sector_id_1)
{
	if (SectorTables[sector_id_0].UnUsed || SectorTables[sector_id_1].UnUsed) {
		return 0.0f;
	}
	return SectorTables[sector_id_0].Table->Match_Fraction(*(SectorTables[sector_id_1].Table));
}

float VisOptimizationContextClass::Compute_Object_Table_Match_Fraction(int object_id_0,int object_id_1)
{
	return ObjectTables[object_id_0].Table->Match_Fraction(*(ObjectTables[object_id_1].Table));
}

void VisOptimizationContextClass::Optimize
(
	VisTableMgrClass * vis_mgr,
	DynamicAABTreeCullClass * dyn_obj_tree
)
{
	/*
	** Generate the object tables
	*/
	Build_Object_Tables(vis_mgr);

	/*
	** Prune redundant leaves of the dynamic object culling tree.  This operation actually
	** deletes leaf nodes from the tree.
	*/
	dyn_obj_tree->Prune_Redundant_Leaf_Nodes(*this);

	/*
	** Combine the redundant objects.  This operation combines objects by assigning the
	** same vis ID to both of them.
	*/
	Combine_Redundant_Objects();

	/*
	** Now that we're done combining objects, build new sector tables so that we
	** can remove the redundant vis sectors.  Each table will have a bitcount
	** equal to the number of object tables.
	*/
	Build_Sector_Tables_From_Object_Tables(vis_mgr);

	/*
	** Now combine the redundant sectors
	*/
	Combine_Redundant_Sectors();

	/*
	** All done, so install the results
	*/
	Install_Results(vis_mgr);
}

void VisOptimizationContextClass::Build_Object_Tables(VisTableMgrClass * vis_mgr)
{
	/*
	** Generate the object tables (what sectors can see each object).  Consider the sector
	** tables columns and the object tables rows of the same 2D grid of visibility bits.
	*/
	int i,j;
	int sector_count = vis_mgr->Get_Vis_Table_Count();
	int object_count = vis_mgr->Get_Vis_Table_Size();

	ObjectTables.Resize(object_count);
	for (i=0; i<object_count; i++) {
		PVSInfoStruct objinfo;
		objinfo.Table = NEW_REF(VisTableClass,(sector_count,0));
		ObjectTables.Add(objinfo);
	}

	for (i=0; i<sector_count; i++) {

		/*
		** Get each sector table
		*/
		VisTableClass * sector_table = vis_mgr->Get_Vis_Table(i);
		if (sector_table == NULL) {
			sector_table = NEW_REF(VisTableClass,(object_count,0));
			sector_table->Reset_All();
		}

		/*
		** Copy its bits into the object tables
		*/
		for (j=0; j<object_count; j++) {
			ObjectTables[j].Table->Set_Bit(i,sector_table->Get_Bit(j) != 0);
		}

		/*
		** Release it
		*/
		REF_PTR_RELEASE(sector_table);
	}
}

void VisOptimizationContextClass::Combine_Redundant_Objects(void)
{
	int i,j;
	for (i=0; i<ObjectTables.Count(); i++) {

		/*
		** Take a copy of object table 'i' for comparisons
		*/
		VisTableClass *table_i = NEW_REF (VisTableClass, (*(ObjectTables[i].Table)));

		for (j=i+1; j<ObjectTables.Count(); j++) {

			/*
			** Compare table 'j' with the original copy of table 'i'
			*/
			float frac = table_i->Match_Fraction(*(ObjectTables[j].Table));
			if (frac > MinVisObjectMatchFraction) {
				Combine_Object_Tables(i,j);
				Stats.Increment_Objects_Merged();
				j--;
			}
		}

		Stats.Increment_Completed_Operations();
		REF_PTR_RELEASE (table_i);
	}
}


void VisOptimizationContextClass::Build_Sector_Tables_From_Object_Tables(VisTableMgrClass * vis_mgr)
{
	int i,j;
	for (i=0; i<vis_mgr->Get_Vis_Table_Count(); i++) {

		PVSInfoStruct sectorinfo;
		sectorinfo.Table = NEW_REF(VisTableClass,(ObjectTables.Count(),0));
		sectorinfo.UnUsed = (vis_mgr->Has_Vis_Table(i) == false);

		for (j=0; j<ObjectTables.Count(); j++) {
			sectorinfo.Table->Set_Bit(j,(ObjectTables[j].Table->Get_Bit(i) != 0));
		}
		SectorTables.Add(sectorinfo);
	}
}

void VisOptimizationContextClass::Combine_Redundant_Sectors(void)
{
	int i,j;
	for (i=0; i<SectorTables.Count(); i++) {

		/*
		** Take a copy of object table 'i' for comparisons
		*/
		VisTableClass *table_i = NEW_REF (VisTableClass, (*(SectorTables[i].Table)));

		for (j=i+1; j<SectorTables.Count(); j++) {

			/*
			** Compare table 'j' with the original copy of table 'i'
			*/
			float frac = table_i->Match_Fraction(*(SectorTables[j].Table));
			if (frac > MinVisSectorMatchFraction) {
				Combine_Sector_Tables(i,j);
				Stats.Increment_Sectors_Merged();
				j--;
			}
		}
		Stats.Increment_Completed_Operations(1);
		REF_PTR_RELEASE (table_i);
	}
}


void VisOptimizationContextClass::Combine_Sector_Tables(int sector_id_0,int sector_id_1)
{
	/*
	** Sort the given id's into ascending order
	*/
	int id0 = sector_id_0;
	int id1 = sector_id_1;
	if (id0 > id1) {
		int tmp = id0;
		id0 = id1;
		id1 = tmp;
	}

	/*
	** Merge the second table into the first.
	*/
	SectorTables[id0].Table->Merge(*(SectorTables[id1].Table));
	SectorTables.Delete(id1);

#if 0
	/*
	** Remove the id1'th bit from each object_table
	*/
	for (int i=0; i<ObjectTables.Count(); i++) {
		ObjectTables[i].Table->Delete_Bit(id1);
	}
#endif

	/*
	** Tell the physics scene to merge any sector with id1 into id0
	*/
	Scene->Merge_Vis_Sector_IDs(id0,id1);
}

void VisOptimizationContextClass::Combine_Object_Tables(int object_id_0,int object_id_1)
{
	/*
	** Sort the given id's into ascending order
	*/
	int id0 = object_id_0;
	int id1 = object_id_1;
	if (id0 > id1) {
		int tmp = id0;
		id0 = id1;
		id1 = tmp;
	}

	/*
	** Merge the second table into the first
	*/
	ObjectTables[id0].Table->Merge(*(ObjectTables[id1].Table));
	ObjectTables.Delete(id1);

#if 0
	/*
	** Remove the id1'th bit from each sector table
	*/
	for (int i=0; i<SectorTables.Count(); i++) {
		SectorTables[i].Table->Delete_Bit(id1);
	}
#endif

	/*
	** Tell the physics scene to merge any object with id1 into id0
	*/
	Scene->Merge_Vis_Object_IDs(id0,id1);
}


void VisOptimizationContextClass::Install_Results(VisTableMgrClass * vismgr)
{
	if (vismgr == NULL) {
		WWDEBUG_SAY(("Error! NULL VisTableMgrClass passed into Install_Results\n"));
		return;
	}

	/*
	** First, reset the contents of the vis manager
	*/
	vismgr->Reset();

	/*
	** Allocate the ID's needed (which also sets the number of tables and the size of each table)
	*/
	vismgr->Set_Optimized_Vis_Object_Count(ObjectTables.Count());
	vismgr->Allocate_Vis_Sector_ID(SectorTables.Count());

	/*
	** Install the Vis Tables.
	*/
	for (int i=0; i<SectorTables.Count(); i++) {
		if (SectorTables[i].UnUsed != true) {

			vismgr->Update_Vis_Table(i,SectorTables[i].Table);
			REF_PTR_RELEASE(SectorTables[i].Table);

		}
	}
}

int VisOptimizationContextClass::Get_Vis_Sector_Count(void)
{
	return SectorTables.Count();
}

int VisOptimizationContextClass::Get_Vis_Object_Count(void)
{
	return ObjectTables.Count();
}

VisOptProgressClass & VisOptimizationContextClass::Get_Progress_Object(void)
{
	return Stats;
}


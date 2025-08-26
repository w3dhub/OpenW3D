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

/******************************************************************************
*
* FILE
*
* DESCRIPTION
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* VERSION INFO
*     $Author: Denzil_l $
*     $Revision: 2 $
*     $Modtime: 3/29/00 2:20p $
*     $Archive: /Commando/Code/Scripts/ScriptNode.h $
*
******************************************************************************/

#ifndef _SCRIPTNODE_H_
#define _SCRIPTNODE_H_

#include "DPrint.h"

class ScriptNode
{
	public:
		virtual ~ScriptNode() {}

	protected:
		friend class ScriptList;
		friend class ScriptListIter;

		// Retrieve next node
		inline ScriptNode* GetNext(void) const
		{
			return mNext;
		}

		// Set next node
		inline void SetNext(ScriptNode* link)
		{
			if (mNext != NULL) {
				assert(link != NULL);
				link->SetNext(mNext);
			}

			mNext = link;
		}

		// Can only be derived
		ScriptNode() : mNext(NULL) {}

	private:
		ScriptNode* mNext;
};


class ScriptList
{
	public:
		ScriptList() : mHead(NULL) {DebugPrint("ScriptList\n");}

		virtual ~ScriptList() {}

		// Add a node to this list
		void AddNode(ScriptNode* node)
		{
			if ((node != NULL) && !HasNode(node)) {
				node->SetNext(mHead);
				mHead = node;
			}
		}

		// Remove a node from the list
		bool RemoveNode(ScriptNode* node)
		{
			ScriptNode* previous = NULL;
			ScriptNode* current = mHead;

			while (current != NULL) {
				ScriptNode* next = current->GetNext();

				if (current == node) {
					// Handle head of list condition
					if (previous == NULL) {
						mHead = next;
					} else {
						previous->SetNext(next);
					}

					return true;
				}

				// Advance to next node
				previous = current;
				current = next;
			}

			return false;
		}

		// Count the number of node in the list.
		int CountNodes(void) const
		{
			ScriptNode* node = mHead;
			int count = 0;

			while (node != NULL) {
				count++;
				node = node->GetNext();
			}

			return count;
		}

		// Check if a node exists in the list.
		bool HasNode(ScriptNode* node) const
		{
			ScriptNode* current = mHead;

			while (current != NULL) {
				if (current == node) {
					return true;
				}

				current = current->GetNext();
			}

			return false;
		}

		// Clear the list
		void Clear(void)
		{
			mHead = NULL;
		}

	private:
		friend class ScriptListIter;
		ScriptNode* mHead;
};


// Script list iterator: This class is used to traverse or iterate a list.
class ScriptListIter
{
	public:
		ScriptListIter(const ScriptList& list)
			: mList(&list), mCurrent(list.mHead)
			{}

		// Set iterator to first node
		inline ScriptNode* First(void)
		{
			mCurrent = mList->mHead;
			return mCurrent;
		}

		// Retrieve current node
		inline ScriptNode* Current(void)
		{
			return mCurrent;
		}

		// Advance to next node
		inline void Next(void)
		{
			mCurrent = mCurrent->GetNext();
		}

		// Test if at end of list
		inline bool AtEnd(void)
		{
			return (mCurrent->GetNext() == NULL);
		}

	private:
		const ScriptList* mList;
		ScriptNode* mCurrent;
};

#endif // _SCRIPTNODE_H_

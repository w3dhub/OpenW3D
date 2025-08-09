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

#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "always.h"
#include "texture.h"

class StringClass;
struct IDirect3DTexture8;
class TextureLoadTaskClass;

class TextureLoader
{
public:
	static void Init(void);
	static void Deinit(void);

	// Modify given texture size to nearest valid size on current hardware.
	static void Validate_Texture_Size(unsigned& width, unsigned& height);

	static IDirect3DTexture8 * Load_Thumbnail(
		const StringClass& filename);
//		WW3DFormat texture_format);	// Pass WW3D_FORMAT_UNKNOWN if you don't care

	static IDirect3DSurface8 *		Load_Surface_Immediate(
		const StringClass& filename,
		WW3DFormat surface_format,		// Pass WW3D_FORMAT_UNKNOWN if you don't care
		bool allow_compression);

	static void	Request_Thumbnail(TextureClass* tc);

	// Adds a loading task to the system. The task if processed in a separate
	// thread as soon as possible. The task will appear in finished tasks list
	// when it's been completed. The texture will be refreshed on the next
	// update call after appearing to the finished tasks list.
	static void Request_Background_Loading(TextureClass* tc);

	// Textures can only be created and locked by the main thread so this function sends a request to the texture
	// handling system to load the texture immediatelly next time it enters the main thread. If this function
	// is called from the main thread the texture is loaded immediatelly.
	static void Request_Foreground_Loading(TextureClass* tc);

	static void	Flush_Pending_Load_Tasks(void);
	static void Update(void(*network_callback)(void) = NULL);

	// returns true if current thread of execution is allowed to make DX8 calls.
	static bool Is_DX8_Thread(void);

	static void Suspend_Texture_Load();
	static void Continue_Texture_Load();

private:
	static void Process_Foreground_Load			(TextureLoadTaskClass *task);
	static void Process_Foreground_Thumbnail	(TextureLoadTaskClass *task);

	static void Begin_Load_And_Queue				(TextureLoadTaskClass *task);
	static void Load_Thumbnail						(TextureClass *tc);

	static bool TextureLoadSuspended;
};

class TextureLoadTaskListNodeClass
{
	friend class TextureLoadTaskListClass;

	public:
		TextureLoadTaskListNodeClass(void) : Next(0), Prev(0) { }

		TextureLoadTaskListClass *Get_List(void)		{ return List; }

		TextureLoadTaskListNodeClass *Next;
		TextureLoadTaskListNodeClass *Prev;
		TextureLoadTaskListClass *		List;
};


class TextureLoadTaskListClass
{
	// This class implements an unsynchronized, double-linked list of TextureLoadTaskClass 
	// objects, using an embedded list node.

	public:
		TextureLoadTaskListClass(void);

		// Returns true if list is empty, false otherwise.
		bool									Is_Empty		(void) const		{ return (Root.Next == &Root); }

		// Add a task to beginning of list
		void									Push_Front	(TextureLoadTaskClass *task);

		// Add a task to end of list
		void									Push_Back	(TextureLoadTaskClass *task);

		// Remove and return a task from beginning of list, or NULL if list is empty.
		TextureLoadTaskClass *			Pop_Front	(void);

		// Remove and return a task from end of list, or NULL if list is empty
		TextureLoadTaskClass *			Pop_Back		(void);

		// Remove specified task from list, if present
		void									Remove		(TextureLoadTaskClass *task);

	private:
		// This list is implemented using a sentinel node.
		TextureLoadTaskListNodeClass	Root;
};


class SynchronizedTextureLoadTaskListClass : public TextureLoadTaskListClass
{
	// This class added thread-safety to the basic TextureLoadTaskListClass.

	public:
		SynchronizedTextureLoadTaskListClass(void);

		// See comments above for description of member functions.
		void									Push_Front	(TextureLoadTaskClass *task);
		void									Push_Back	(TextureLoadTaskClass *task);
		TextureLoadTaskClass *			Pop_Front	(void);
		TextureLoadTaskClass *			Pop_Back		(void);
		void									Remove		(TextureLoadTaskClass *task);

	private:
		CriticalSectionClass		CriticalSection;
};


class TextureLoadTaskClass : public TextureLoadTaskListNodeClass
{
	public:
		enum TaskType {
			TASK_NONE,
			TASK_THUMBNAIL,
			TASK_LOAD,
		};

		enum PriorityType {
			PRIORITY_LOW,
			PRIORITY_HIGH,
		};

		enum StateType {
			STATE_NONE,

			STATE_LOAD_BEGUN,
			STATE_LOAD_MIPMAP,
			STATE_LOAD_COMPLETE,

			STATE_COMPLETE,
		};


		TextureLoadTaskClass(void);
		~TextureLoadTaskClass(void);

		static TextureLoadTaskClass *	Create			(TextureClass *tc, TaskType type, PriorityType priority);
		void						Destroy						(void);
		static void				Delete_Free_Pool			(void);

		void						Init							(TextureClass *tc, TaskType type, PriorityType priority);
		void						Deinit						(void);

		TaskType					Get_Type						(void) const		{ return Type;				}
		PriorityType			Get_Priority				(void) const		{ return Priority;		}
		StateType				Get_State					(void) const		{ return State;			}

		WW3DFormat				Get_Format					(void) const		{ return Format;			}
		unsigned int			Get_Width					(void) const		{ return Width;			}
		unsigned int			Get_Height					(void) const		{ return Height;			}
		unsigned int			Get_Mip_Level_Count		(void) const		{ return MipLevelCount; }
		unsigned int			Get_Reduction				(void) const		{ return Reduction;		}

		unsigned char *		Get_Locked_Surface_Ptr	(unsigned int level);
		unsigned int			Get_Locked_Surface_Pitch(unsigned int level) const;

		TextureClass *			Peek_Texture				(void)				{ return Texture;			}
		IDirect3DTexture8	*	Peek_D3D_Texture			(void)				{ return D3DTexture;		}

		void						Set_Type						(TaskType t)		{ Type		= t;			}
		void						Set_Priority				(PriorityType p)	{ Priority	= p;			}
		void						Set_State					(StateType s)		{ State		= s;			}

		bool						Begin_Load					(void);
		bool						Load							(void);
		void						End_Load						(void);
		void						Finish_Load					(void);
		void						Apply_Missing_Texture	(void);						

	private:
		bool						Begin_Compressed_Load	(void);
		bool						Begin_Uncompressed_Load	(void);

		bool						Load_Compressed_Mipmap	(void);
		bool						Load_Uncompressed_Mipmap(void);

		void						Lock_Surfaces				(void);
		void						Unlock_Surfaces			(void);

		void						Apply							(bool initialize);
		
		TextureClass*			Texture;
		IDirect3DTexture8 *	D3DTexture;
		WW3DFormat				Format;

		unsigned int			Width;
		unsigned	int			Height;
		unsigned	int			MipLevelCount;
		unsigned	int			Reduction;

		unsigned char *		LockedSurfacePtr[TextureClass::MIP_LEVELS_MAX];
		unsigned	int			LockedSurfacePitch[TextureClass::MIP_LEVELS_MAX];

		TaskType					Type;
		PriorityType			Priority;
		StateType				State;
};


#endif

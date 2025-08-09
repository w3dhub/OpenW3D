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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Combat/debug.h                               $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 12/18/01 10:36a                                             $*
 *                                                                                             *
 *                    $Revision:: 27                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	DEBUG_H
#define	DEBUG_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	WWDEBUG_H
	#include "wwdebug.h"
#endif

#ifndef	BITTYPE_H
	#include	"bittype.h"
#endif

#ifndef	__WIDESTRING_H
	#include	"widestring.h"
#endif

#ifndef	VECTOR3_H
	#include	"vector3.h"
#endif

#ifndef	VECTOR4_H
	#include	"vector4.h"
#endif

//#ifndef _CONTEXT
//struct CONTEXT;
//#define TEMP_CONTEXT_DEFINE
//#endif
//int Stack_Walk(unsigned long *return_addresses, int num_addresses, CONTEXT *context = NULL);
//#ifdef TEMP_CONTEXT_DEFINE
//#undef CONTEXT
//#endif

/*
**
*/
class	DebugDisplayHandlerClass {
public:
	virtual	void	Display_Text( const char * string, const Vector4 & color = Vector4( 1,1,1,1 ) ) = 0;
	virtual	void	Display_Text( const WideStringClass & string, const Vector4 & color = Vector4( 1,1,1,1 ) ) = 0;
};


/*
**
*/
class	DebugManager {

public:
	static	void	Init( void );
	static	void	Shutdown( void );

	static	void	Update( void );

	static	void	Load_Registry_Settings( const char * sub_key );
	static	void	Save_Registry_Settings( const char * sub_key );

	//
	//	Version number support.  The major revision number is stored in the
	// hiword of the version, the minor revision number is stored in the loword.
	//
	static	int	Get_Version_Number (void)			{ return VersionNumber; }
	static	void	Set_Version_Number (int number)	{ VersionNumber = number; }

	/*
	** Debug Displaying
	*/
	enum DebugDevice {
		DEBUG_DEVICE_SCREEN		= 1<<0,
		DEBUG_DEVICE_MONO			= 1<<1,
		DEBUG_DEVICE_SRDEBUG		= 1<<2,		// OBSOLETE!
		DEBUG_DEVICE_DBWIN32		= 1<<3,
		DEBUG_DEVICE_LOG			= 1<<4,
		DEBUG_DEVICE_WINDOWS		= 1<<5,
	};
	static	void	Enable_Device( DebugDevice device )				{ EnabledDevices |= device;	}
	static	void	Disable_Device( DebugDevice device )			{ EnabledDevices &= ~device;	}
	static	void	Toggle_Device_Enabled( DebugDevice device ) 	{ EnabledDevices ^= device; }
	static	bool	Is_Device_Enabled( DebugDevice device )		{ return (EnabledDevices & device) != 0; }


	enum DebugType {
		DEBUG_TYPE_INFORMATION		   = 1<<WWDEBUG_TYPE_INFORMATION,
		DEBUG_TYPE_WARNING			   = 1<<WWDEBUG_TYPE_WARNING,
		DEBUG_TYPE_ERROR				   = 1<<WWDEBUG_TYPE_ERROR,
		DEBUG_TYPE_SCRIPT				   = 1<<(WWDEBUG_TYPE_USER+1),
		DEBUG_TYPE_NETWORK_ADMIN      = 1<<(WWDEBUG_TYPE_USER+2),
		DEBUG_TYPE_NETWORK_BASIC      = 1<<(WWDEBUG_TYPE_USER+3),
		DEBUG_TYPE_NETWORK_PROLIFIC   = 1<<(WWDEBUG_TYPE_USER+4),
	};
	static	void	Enable_Type( DebugType type )				{ EnabledTypes |= type;	}
	static	void	Disable_Type( DebugType type )			{ EnabledTypes &= ~type;	}
	static	void	Toggle_Type_Enabled( DebugType type ) 	{ EnabledTypes ^= type; }
	static	bool	Is_Type_Enabled( DebugType type )		{ return (EnabledTypes & type) != 0; }


	static	void	Display( char const *buffer );
	static	void	Display_Script( char const *text, ... );
	static	void	Display_Network_Admin( char const *text, ... );
	static	void	Display_Network_Basic( char const *text, ... );
	static	void	Display_Network_Prolific( char const *text, ... );

	/*
	** Debug Options
	** (gth) Enable and disable debug options and triggers.  Now that we've
	** moved more functions to the console (rather than 10 billion key combinations)
	** we need an interface for turning on and off some of the debug triggers.
	*/
	enum DebugOption {
		DEBUG_INVERT_VIS				= 1<<0,
		DEBUG_DISABLE_VIS				= 1<<1,
		DEBUG_COLLISION_MESSAGES	= 1<<2,
		DEBUG_COLLISION_DISPLAY		= 1<<3,
		DEBUG_DISABLE_LIGHTS			= 1<<4,
		DEBUG_PROCESS_STATS			= 1<<5,
		DEBUG_SURFACE_CACHE			= 1<<6,
	};
	static	void	Option_Enable( DebugOption option, bool onoff )
		{ if ( onoff ) EnabledOptions |= option; else EnabledOptions &= ~option; }
	static	bool	Option_Is_Enabled( DebugOption option )	{ return (EnabledOptions & option) != 0; }
	static	void	Option_Toggle( DebugOption option )		{ EnabledOptions ^= option; }

	// Misc Debug Functions
	static	void		Measure_Frame_Textures( void );

	// Outside handler display
	static	void	Set_Display_Handler( DebugDisplayHandlerClass * handler )	{ DisplayHandler = handler; }

	static	void	Display_Text(const char * string, const Vector4 & color );
	static	void	Display_Text(const char * string, const Vector3 & color = Vector3( 1,1,1 ) );
	static	void	Display_Text(const WideStringClass & string, const Vector4 & color );
	static	void	Display_Text(const WideStringClass & string, const Vector3 & color = Vector3( 1,1,1 ) );

   static	void	Init_Logfile(void);
	static	void	Write_To_File(LPCSTR str);
	static	LPCSTR Logfile_Name(void) {return LOGFILE;}

	static	void	Enable_Memory_Logging( bool enable );

	static	bool	Is_File_Logging_Enabled( void )		{ return EnableFileLogging; }
	static	bool	Is_Diag_Logging_Enabled( void )		{ return EnableDiagLogging; }
	static	bool	Load_Debug_Scripts( void )				{ return LoadDebugScripts; }

	static	void	Set_Is_Slave(bool isslave) {IsSlave = isslave;}

	static	bool	Allow_Cinematic_Keys( void )	{ return AllowCinematicKeys; };

private:
	static	int	EnabledDevices;
	static	int	EnabledTypes;
	static	int	EnabledOptions;
	static	bool	EnableFileLogging;
	static	bool	EnableDiagLogging;
	static	bool	LoadDebugScripts;
	static	int	VersionNumber;
	static	bool	AllowCinematicKeys;

	static	DebugDisplayHandlerClass * DisplayHandler;
   static	const char *LOGFILE;
	static	char	LogfileNameBuffer[256];
	static	bool	IsSlave;

	/*
	** Thread safety
	*/
	static	CriticalSectionClass CriticalSection;


};

// macro to print
#define	Debug_Say			         WWDEBUG_SAY
#define	Debug_Warning		         WWDEBUG_WARNING
#define	Debug_Error			         WWDEBUG_ERROR
#define	Debug_Script(x)	         DebugManager::Display_Script x
#define	Debug_Network_Basic(x)	   DebugManager::Display_Network_Basic x
#define	Debug_Network_Prolific(x)	DebugManager::Display_Network_Prolific x

#endif	// DEBUG_H







//#define	Debug_Network_Admin(x)	   DebugManager::Display_Network_Admin x
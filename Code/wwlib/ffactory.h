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
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/ffactory.h                     $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 9/07/01 5:30p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef FFACTORY_H
#define FFACTORY_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "mutex.h"
#include "vector.h"
#include "wwstring.h"

/*
**
*/
class	FileClass;

/*
** FileFactoryClass is a pure virtual class used to
** create FileClasses.
*/
class	FileFactoryClass {

public:
	virtual ~FileFactoryClass(void){};
	virtual FileClass * Get_File( char const *filename ) = 0;
	virtual void Return_File( FileClass *file ) = 0;
};




//
// Handy auto pointer class.  Prevents you from having to call Return_File manually
//
class file_auto_ptr
{
public:
	explicit	file_auto_ptr(FileFactoryClass *fac, const char *filename);
				~file_auto_ptr();

	operator FileClass*(void) const
		{return (get()); }

	FileClass& operator*() const
		{return (*get()); }

	FileClass *operator->() const
		{return (get()); }

	FileClass *get() const
		{return (_Ptr); }

private:
	// prevent these from getting auto-generated or used
						file_auto_ptr(const file_auto_ptr &other);
	file_auto_ptr	&operator=(const file_auto_ptr &other);


	FileClass			*_Ptr;
	FileFactoryClass	*_Fac;
};







/*
** RawFileFactoryClass is a derived FileFactoryClass which
** gives RawFileClass objects
*/
class	RawFileFactoryClass : public FileFactoryClass {
public:
	virtual FileClass * Get_File( char const *filename ) override;
	virtual void Return_File( FileClass *file ) override;
};


/*
** SimpleFileFactoryClass is a slightly more capable derivative of
** FileFactoryClass which adds some simple extra functionality. It supports a
** current subdirectory, and also adds some logging capabilities. Note that
** it currently creates BufferedFileClass objects instead of RawFileClass
** objects.
*/
class	SimpleFileFactoryClass : public FileFactoryClass {

public:
	SimpleFileFactoryClass( void );
	~SimpleFileFactoryClass( void )	{}

	virtual FileClass *	Get_File( char const *filename ) override;
	virtual void			Return_File( FileClass *file ) override;

	// sub_directory may be a semicolon seperated search path.  New files will always
	//   go in the last dir in the path.
	void						Get_Sub_Directory( StringClass& new_dir ) const;
	void						Set_Sub_Directory( const char * sub_directory );
	void						Prepend_Sub_Directory( const char * sub_directory );
	void						Append_Sub_Directory( const char * sub_directory );
	bool						Get_Strip_Path( void ) const								{ return IsStripPath; }
	void						Set_Strip_Path( bool set )									{ IsStripPath = set; }

protected:
	StringClass				SubDirectory;
	bool						IsStripPath;

	// Mutex must be mutable because const functions lock on it.
	mutable CriticalSectionClass	Mutex;
};


/*
** TheFileFactory is a static pointer to the single file factory (only one
** should be inexistence at any one time). TheSimpleFileFactory is a
** differently typed pointer to the same entity for those cases where the
** extra interface functions are needed (TheFileFactory is created as an
** object of type SimpleFileFactoryClass by default. If this changes to some
** other class which is not a derivative of SimpleFileFactoryClass then
** TheSimpleFileFactory should be set to NULL).
*/
extern FileFactoryClass	*	_TheFileFactory;
extern FileFactoryClass	*	_TheWritingFileFactory;
extern SimpleFileFactoryClass	*	_TheSimpleFileFactory;

#endif
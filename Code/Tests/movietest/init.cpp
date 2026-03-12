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

/* $Header: /Commando/Code/Tests/movietest/init.cpp 4     5/06/98 3:10p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tests/movietest/init.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/24/98 10:43a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "init.h"
#include <sr.hpp>
#include "_viewpt.h"
#include "_world.h"
#include "winmain.h"
#include "ww3d.h"

bool Init(void)
{

	ScreenResolution.X = 0;
	ScreenResolution.Y = 0;
	ScreenResolution.Width = 640;
	ScreenResolution.Height = 480;

	MainViewport.X = 16;
	MainViewport.Y = 16;
	MainViewport.Width = 640-32;
	MainViewport.Height = 480-32;

	WW3D::Init(hWndMain);
	WW3D::Set_Resolution(ScreenResolution.Width,ScreenResolution.Height,16,false);

	const DynamicVectorClass<RenderDeviceDescClass> & RDDescriptions = WW3D::Enumerate_Render_Devices();

	char buf[1024];
	char tmp[64];
	char title[64];

	for (int ri=0; ri<RDDescriptions.Count(); ri++) {

		sprintf(title,"Render Device:       %d",ri);
		buf[0] = 0;
		sprintf(tmp,"Device Name:           %s\n",RDDescriptions[ri].Get_Device_Name());
		strcat(buf,tmp);
		sprintf(tmp,"Device Vendor:         %s\n",RDDescriptions[ri].Get_Device_Vendor());
		strcat(buf,tmp);
		sprintf(tmp,"Device Platform:       %s\n",RDDescriptions[ri].Get_Device_Platform());
		strcat(buf,tmp);

		sprintf(tmp,"Driver Name:           %s\n",RDDescriptions[ri].Get_Driver_Name());
		strcat(buf,tmp);
		sprintf(tmp,"Driver Vendor:         %s\n",RDDescriptions[ri].Get_Driver_Vendor());
		strcat(buf,tmp);
		sprintf(tmp,"Driver Version:        %s\n",RDDescriptions[ri].Get_Driver_Version());
		strcat(buf,tmp);

		sprintf(tmp,"Hardware Name:         %s\n",RDDescriptions[ri].Get_Hardware_Name());
		strcat(buf,tmp);
		sprintf(tmp,"Hardware Vendor:       %s\n",RDDescriptions[ri].Get_Hardware_Vendor());
		strcat(buf,tmp);
		sprintf(tmp,"Hardware Chipset:      %s\n",RDDescriptions[ri].Get_Hardware_Chipset());
		strcat(buf,tmp);

		MessageBox(NULL,buf,title,MB_OK);
	}

	return true;
}


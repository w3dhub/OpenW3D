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

/**********************************************************************
 *<
	FILE: MaxFly.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#include "MaxFly.h"

#define MAXFLY_CLASS_ID	Class_ID(0xf1077c8, 0x387a6b66)

const float PITCH_SPEED = 8.0f * 3.1415f / 180.0f;
const float YAW_SPEED = 8.0f * 3.1415f / 180.0f;
const float FLY_SPEED = 5.0f;


class MaxFly : public UtilityObj 
{
public:
	MaxFly();
	~MaxFly();		

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	
	void DeleteThis() { }		

protected:

	void	Select_Camera(void);
	void	Pitch_Up(void);
	void	Pitch_Down(void);
	void	Yaw_Left(void);
	void	Yaw_Right(void);
	void	Fly_Forward(void);
	void	Fly_Backward(void);
		
	HWND			hPanel;
	IUtil *		iu;
	Interface *	ip;

	friend static BOOL CALLBACK MaxFlyDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

static MaxFly theMaxFly;

class MaxFlyClassDesc:public ClassDesc2 
{
	public:
	int 				IsPublic() {return 1;}
	void *			Create(BOOL loading = false) {return &theMaxFly;}
	const TCHAR *	ClassName() {return GetString(IDS_CLASS_NAME);}
	SClass_ID		SuperClassID() {return UTILITY_CLASS_ID;}
	Class_ID			ClassID() {return MAXFLY_CLASS_ID;}
	const TCHAR* 	Category() {return GetString(IDS_CATEGORY);}
	const TCHAR*	InternalName() { return _T("MaxFly"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }				// returns owning module handle
};

static MaxFlyClassDesc MaxFlyDesc;
ClassDesc2* GetMaxFlyDesc() {return &MaxFlyDesc;}

static BOOL CALLBACK MaxFlyDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
		case WM_INITDIALOG:
			theMaxFly.Init(hWnd);
			break;

		case WM_DESTROY:
			theMaxFly.Destroy(hWnd);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_PITCH_UP_BUTTON:			theMaxFly.Pitch_Up();			break;
				case IDC_PITCH_DOWN_BUTTON:		theMaxFly.Pitch_Down();			break;
				case IDC_YAW_LEFT_BUTTON:			theMaxFly.Yaw_Left();			break;
				case IDC_YAW_RIGHT_BUTTON:			theMaxFly.Yaw_Right();			break;
				case IDC_FLY_FORWARD_BUTTON:		theMaxFly.Fly_Forward();		break;
				case IDC_FLY_BACKWARD_BUTTON:		theMaxFly.Fly_Backward();		break;
				case IDC_SELECT_CAMERA_BUTTON:	theMaxFly.Select_Camera();		break;
			};

			break;


		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			theMaxFly.ip->RollupMouseMessage(hWnd,msg,wParam,lParam); 
			break;

		default:
			return false;
	}
	return true;
}



//--- MaxFly -------------------------------------------------------


MaxFly::MaxFly()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

MaxFly::~MaxFly()
{

}

void MaxFly::BeginEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = iu;
	this->ip = ip;
	hPanel = ip->AddRollupPage(
		hInstance,
		MAKEINTRESOURCE(IDD_PANEL),
		MaxFlyDlgProc,
		GetString(IDS_PARAMS),
		0);
}
	
void MaxFly::EndEditParams(Interface *ip,IUtil *iu) 
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void MaxFly::Init(HWND hWnd)
{

}

void MaxFly::Destroy(HWND hWnd)
{

}

void MaxFly::Select_Camera(void)
{
}

void MaxFly::Pitch_Up(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Rotate(	ip->GetTime(),									// Time
									object_tm,										// tmAxis
									AngAxis(Point3(1,0,0),-PITCH_SPEED),	// const AngAxis& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}

		ip->RedrawViews(ip->GetTime());
	}
}

void MaxFly::Pitch_Down(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Rotate(	ip->GetTime(),									// Time
									object_tm,										// tmAxis
									AngAxis(Point3(1,0,0),PITCH_SPEED),		// const AngAxis& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}
		ip->RedrawViews(ip->GetTime());
	}
}

void MaxFly::Yaw_Left(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Rotate(	ip->GetTime(),									// Time
									object_tm,										// tmAxis
									AngAxis(Point3(0,1,0),-YAW_SPEED),		// const AngAxis& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}
		ip->RedrawViews(ip->GetTime());
	}
}

void MaxFly::Yaw_Right(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Rotate(	ip->GetTime(),									// Time
									object_tm,										// tmAxis
									AngAxis(Point3(0,1,0),+YAW_SPEED),		// const AngAxis& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}
		ip->RedrawViews(ip->GetTime());
	}
}

void MaxFly::Fly_Forward(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Move(		ip->GetTime(),									// TimeValue t, 
									object_tm,										// const Matrix3& tmAxis, 
									Point3(0.0f,0.0f,-FLY_SPEED),				// const Point3& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}
		ip->RedrawViews(ip->GetTime());
	}
}

void MaxFly::Fly_Backward(void)
{
	if (ip->GetSelNodeCount() > 0) {
		for (int i=0; i<ip->GetSelNodeCount(); i++) {
			INode * node = ip->GetSelNode(i);
			if (node) {
				Matrix3 object_tm = node->GetObjectTM(ip->GetTime());
				node->Move(		ip->GetTime(),									// TimeValue t, 
									object_tm,										// const Matrix3& tmAxis, 
									Point3(0.0f,0.0f,FLY_SPEED),				// const Point3& val, 
									true,												// BOOL localOrigin=false, 
									true,												// BOOL affectKids=true, 
									PIV_NONE,										// int pivMode=PIV_NONE, 
									true												// BOOL ignoreLocks=false
								);
			}
		}
		ip->RedrawViews(ip->GetTime());
	}
}

		

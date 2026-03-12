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

// RenegadeGR.cpp : Defines the entry point for the DLL application.
//

#include <malloc.h>
#include <string.h>
#include "jni.h"
#include "RenegadeGR.h"
#include "RenegadeNet.h"
#include "rengameres.h"
#include "tcpmgr.h"
#include "tcpcon.h"




JNIEXPORT jint JNICALL Java_RenegadeNet__1nativeSendGR
	(JNIEnv* env, jclass cls, jobjectArray loginArray, jdoubleArray scoreArray)
{
	int		i = 0;
	char**	packetLogins = NULL;
	double*	packetScores = NULL;


	// Build packetLogins array
	jsize loginlen = env->GetArrayLength(loginArray);
	packetLogins = new char*[loginlen];
    for(i = 0; i < loginlen; i++)
	{
		jstring jstr = (jstring)env->GetObjectArrayElement(loginArray, i);
		const char *str = env->GetStringUTFChars(jstr, 0);
		packetLogins[i] = new char[strlen(str)+1];
		strcpy(packetLogins[i], str);
		env->ReleaseStringUTFChars(jstr, str);
    }


	// Build packetScores array
	jsize scorelen = env->GetArrayLength(scoreArray);
	jdouble *jscores = env->GetDoubleArrayElements(scoreArray, 0);
	packetScores = new double[scorelen];
    for (i = 0; i < scorelen; i++)
		packetScores[i] = jscores[i];
	env->ReleaseDoubleArrayElements(scoreArray, jscores, 0);


	RenegadeGameRes renegadeGameRes("10.2.20.28", 4850);
	//RenegadeGameRes renegadeGameRes;
	renegadeGameRes.setGameID(21450402);
	renegadeGameRes.setPlayerCount((unsigned char)loginlen);
	renegadeGameRes.setClanGame(0);
	renegadeGameRes.setDuration(100);
	renegadeGameRes.setMapName("Renegade Map");
	renegadeGameRes.setSKU(GR_ENGLISH);
	renegadeGameRes.setStyle(GR_DEATHMATCH);
	renegadeGameRes.setNumClans(0);
	renegadeGameRes.setStartTime(100);
	renegadeGameRes.setTournament(1);
	for(i = 0; i < loginlen; i++)
	{
		renegadeGameRes.addPlayer(packetLogins[i], packetScores[i], 0, 245, 0, 5, 5, 3, 244);
	}
	int sendlen = renegadeGameRes.sendResults();



	delete[] packetScores;
	for(i = 0; i < loginlen; i++)
		delete[] (packetLogins[i]);
	delete[] packetLogins;


	return sendlen;
}


JNIEXPORT void JNICALL Java_RenegadeNet_startWinSock(JNIEnv *, jclass)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err = 1;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup( wVersionRequested, &wsaData);
	/*if (err != 0)
	{
		LOG_END("failed");
		//assert(false);
	}
	else
	{
		LOG_END("ok");
	}*/
}


JNIEXPORT void JNICALL Java_RenegadeNet_stopWinSock(JNIEnv *, jclass)
{
	WSACleanup();
}





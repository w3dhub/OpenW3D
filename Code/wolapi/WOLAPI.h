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

#ifndef __WOLAPI_h__
#define __WOLAPI_h__

#ifdef _WIN32
#include <windows.h>
#include <ole2.h>
#endif

namespace WOL {

#define WOLAPI_STDMETHOD(NAME) WOL::WOLAPI_RESULT WOLAPI_CALLTYPE NAME
#define WOLAPI_STDMETHODIMP WOL::WOLAPI_RESULT WOLAPI_CALLTYPE
typedef int time_t;
typedef int WOLAPI_RESULT;
typedef int WOLAPI_BOOL;

#ifdef _WIN32
#define WOLAPI_CALLTYPE __stdcall
#define RefCountType ULONG
#ifdef __MINGW32__
#define WOLAPI_INTERFACE(UUID) struct
#else
#define WOLAPI_INTERFACE(UUID) struct __declspec(uuid(UUID)) __declspec(novtable)
#endif
#else
#define WOLAPI_CALLTYPE
#define WOLAPI_INTERFACE(UUID) struct

struct IUnknown {
	using RefCountType = unsigned int;

	virtual RefCountType WOLAPI_CALLTYPE Release() = 0;
	virtual RefCountType WOLAPI_CALLTYPE AddRef() = 0;
};
#endif

/* Forward Declarations */

struct IRTPatcher;
struct IRTPatcherEvent;
struct IChat;
struct IChatEvent;
struct IDownload;
struct IDownloadEvent;
struct INetUtil;
struct INetUtilEvent;
struct IChat2;
struct IChat2Event;
struct IIGROptions;

#ifdef _WIN32
extern "C" {
extern const IID IID_IRTPatcher;
extern const IID IID_IRTPatcherEvent;
extern const IID IID_IChat;
extern const IID IID_IChatEvent;
extern const IID IID_IDownload;
extern const IID IID_IDownloadEvent;
extern const IID IID_INetUtil;
extern const IID IID_INetUtilEvent;
extern const IID IID_IChat2;
extern const IID IID_IChat2Event;
extern const IID IID_IIGROptions;
extern const IID LIBID_WOLAPILib;
extern const CLSID CLSID_RTPatcher;
extern const CLSID CLSID_Chat;
extern const CLSID CLSID_Download;
extern const CLSID CLSID_IGROptions;
extern const CLSID CLSID_NetUtil;
extern const CLSID CLSID_Chat2;
}
#endif


    WOLAPI_INTERFACE("925CDEDE-71B9-11D1-B1C5-006097176556")
    IRTPatcher : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE ApplyPatch(
            /* [string][in] */ const char *destpath,
            /* [string][in] */ const char *filename) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE PumpMessages( void) = 0;

    };

    WOLAPI_INTERFACE("925CDEE3-71B9-11D1-B1C5-006097176556")
    IRTPatcherEvent : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnProgress(
            /* [in] */ const char *filename,
            /* [in] */ int progress) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnTermination(
            /* [in] */ WOLAPI_BOOL success) = 0;

    };

typedef
enum Locale
    {	LOC_UNKNOWN	= 0,
	LOC_OTHER	= LOC_UNKNOWN + 1,
	LOC_USA	= LOC_OTHER + 1,
	LOC_CANADA	= LOC_USA + 1,
	LOC_UK	= LOC_CANADA + 1,
	LOC_GERMANY	= LOC_UK + 1,
	LOC_FRANCE	= LOC_GERMANY + 1,
	LOC_SPAIN	= LOC_FRANCE + 1,
	LOC_NETHERLANDS	= LOC_SPAIN + 1,
	LOC_BELGIUM	= LOC_NETHERLANDS + 1,
	LOC_AUSTRIA	= LOC_BELGIUM + 1,
	LOC_SWITZERLAND	= LOC_AUSTRIA + 1,
	LOC_ITALY	= LOC_SWITZERLAND + 1,
	LOC_DENMARK	= LOC_ITALY + 1,
	LOC_SWEDEN	= LOC_DENMARK + 1,
	LOC_NORWAY	= LOC_SWEDEN + 1,
	LOC_FINLAND	= LOC_NORWAY + 1,
	LOC_ISRAEL	= LOC_FINLAND + 1,
	LOC_SOUTH_AFRICA	= LOC_ISRAEL + 1,
	LOC_JAPAN	= LOC_SOUTH_AFRICA + 1,
	LOC_SOUTH_KOREA	= LOC_JAPAN + 1,
	LOC_CHINA	= LOC_SOUTH_KOREA + 1,
	LOC_SINGAPORE	= LOC_CHINA + 1,
	LOC_TAIWAN	= LOC_SINGAPORE + 1,
	LOC_MALAYSIA	= LOC_TAIWAN + 1,
	LOC_AUSTRALIA	= LOC_MALAYSIA + 1,
	LOC_NEW_ZEALAND	= LOC_AUSTRALIA + 1,
	LOC_BRAZIL	= LOC_NEW_ZEALAND + 1,
	LOC_THAILAND	= LOC_BRAZIL + 1,
	LOC_ARGENTINA	= LOC_THAILAND + 1,
	LOC_PHILIPPINES	= LOC_ARGENTINA + 1,
	LOC_GREECE	= LOC_PHILIPPINES + 1,
	LOC_IRELAND	= LOC_GREECE + 1,
	LOC_POLAND	= LOC_IRELAND + 1,
	LOC_PORTUGAL	= LOC_POLAND + 1,
	LOC_MEXICO	= LOC_PORTUGAL + 1,
	LOC_RUSSIA	= LOC_MEXICO + 1,
	LOC_TURKEY	= LOC_RUSSIA + 1
    }	Locale;

struct  Highscore
    {
    unsigned int sku;
    unsigned int wins;
    unsigned int losses;
    unsigned int points;
    unsigned int rank;
    unsigned int accomplishments;
    struct Highscore *next;
    unsigned char login_name[ 40 ];
    };
struct  Ladder
    {
    unsigned int sku;
    unsigned int team_no;
    unsigned int wins;
    unsigned int losses;
    unsigned int points;
    unsigned int kills;
    unsigned int rank;
    unsigned int rung;
    unsigned int disconnects;
    unsigned int team_rung;
    unsigned int provisional;
    unsigned int last_game_date;
    unsigned int win_streak;
    unsigned int reserved1;
    unsigned int reserved2;
    struct Ladder *next;
    unsigned char login_name[ 40 ];
    Locale locale;
    };
typedef int GroupID;

struct  Server
    {
    int gametype;
    int chattype;
    int timezone;
    float longitude;
    float lattitude;
    struct Server *next;
    unsigned char name[ 71 ];
    unsigned char connlabel[ 5 ];
    unsigned char conndata[ 128 ];
    unsigned char login[ 10 ];
    unsigned char password[ 10 ];
    };
struct  Channel
    {
    int type;
    unsigned int minUsers;
    unsigned int maxUsers;
    unsigned int currentUsers;
    unsigned int official;
    unsigned int tournament;
    unsigned int ingame;
    unsigned int flags;
    unsigned int reserved;
    unsigned int ipaddr;
    int latency;
    int hidden;
    struct Channel *next;
    unsigned char name[ 17 ];
    unsigned char topic[ 81 ];
    unsigned char location[ 65 ];
    unsigned char key[ 9 ];
    unsigned char exInfo[ 41 ];
    };
struct  User
    {
    unsigned int flags;
    GroupID group;
    unsigned int reserved;
    unsigned int reserved2;
    unsigned int reserved3;
    unsigned int squadID;
    unsigned int ipaddr;
    unsigned int squad_icon;
    struct User *next;
    unsigned char name[ 10 ];
    unsigned char squadname[ 41 ];
    unsigned char squadabbrev[ 10 ];
    Locale locale;
    int team;
    };
struct  Group
    {
    GroupID ident;
    int type;
    unsigned int members;
    struct Group *next;
    unsigned char name[ 65 ];
    };
struct  Squad
    {
    unsigned int id;
    int sku;
    int members;
    int color1;
    int color2;
    int color3;
    int icon1;
    int icon2;
    int icon3;
    struct Squad *next;
    int rank;
    int team;
    int status;
    unsigned char email[ 81 ];
    unsigned char icq[ 17 ];
    unsigned char motto[ 81 ];
    unsigned char url[ 129 ];
    unsigned char name[ 41 ];
    unsigned char abbreviation[ 41 ];
    };
struct  Update
    {
    unsigned int SKU;
    unsigned int version;
    int required;
    struct Update *next;
    unsigned char server[ 65 ];
    unsigned char patchpath[ 256 ];
    unsigned char patchfile[ 33 ];
    unsigned char login[ 33 ];
    unsigned char password[ 65 ];
    unsigned char localpath[ 256 ];
    };
typedef struct Server Server;

typedef struct Channel Channel;

typedef struct User User;

typedef struct Group Group;

typedef struct Update Update;

typedef struct Ladder Ladder;

typedef struct Highscore Highscore;

typedef struct Squad Squad;

    WOLAPI_INTERFACE("4DD3BAF4-7579-11D1-B1C6-006097176556")
    IChat : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE PumpMessages( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestServerList(
            /* [in] */ unsigned int SKU,
            /* [in] */ unsigned int current_version,
            /* [in] */ const char *loginname,
            /* [in] */ const char *password,
            /* [in] */ int timeout) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestConnection(
            /* [in] */ Server *server,
            /* [in] */ int timeout,
            int domangle) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelList(
            /* [in] */ int channelType,
            /* [in] */ int autoping) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelCreate(
            /* [in] */ Channel *channel) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelJoin(
            /* [in] */ Channel *channel) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelLeave( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserList( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPublicMessage(
            /* [in] */ const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPrivateMessage(
            /* [in] */ User *users,
            /* [in] */ const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLogout( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPrivateGameOptions(
            /* [in] */ User *users,
            /* [in] */ const char *options) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPublicGameOptions(
            /* [in] */ const char *options) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPublicAction(
            /* [in] */ const char *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPrivateAction(
            /* [in] */ User *users,
            /* [in] */ const char *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestGameStart(
            /* [in] */ User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelTopic(
            /* [in] */ const char *topic) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetVersion(
            /* [in] */ unsigned int *version) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserKick(
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserIP(
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetGametypeInfo(
            unsigned int gtype,
            int icon_size,
            unsigned char * *bitmap,
            int *bmp_bytes,
            const char **name,
            const char **URL) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestFind(
            User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPage(
            User *user,
            const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetFindPage(
            int findOn,
            int pageOn) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetSquelch(
            User *user,
            int squelch) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetSquelch(
            User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetChannelFilter(
            int channelType) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestGameEnd( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetLangFilter(
            int onoff) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelBan(
            const char *name,
            int ban) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetGametypeList(
            const char **list) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetHelpURL(
            const char **url) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetProductSKU(
            unsigned int SKU) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetNick(
            int num,
            const char **nick,
            const char **pass) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetNick(
            int num,
            const char *nick,
            const char *pass,
            int domangle) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetLobbyCount(
            int *count) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestRawMessage(
            const char *ircmsg) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetAttributeValue(
            const char *attrib,
            const char **value) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetAttributeValue(
            const char *attrib,
            const char *value) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetChannelExInfo(
            const char *info) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE StopAutoping( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSquadInfo(
            unsigned int id) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSetTeam(
            int team) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSetLocale(
            Locale locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserLocale(
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserTeam(
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetNickLocale(
            int nicknum,
            Locale *locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetNickLocale(
            int nicknum,
            Locale locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetLocaleString(
            const char **loc_string,
            Locale locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetLocaleCount(
            int *num) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetClientVersion(
            unsigned int version) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetCodepageFilter(
            int filter) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestBuddyList( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestBuddyAdd(
            User *newbuddy) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestBuddyDelete(
            User *buddy) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPublicUnicodeMessage(
            /* [in] */ const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPrivateUnicodeMessage(
            /* [in] */ User *users,
            /* [in] */ const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPublicUnicodeAction(
            /* [in] */ const unsigned short *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPrivateUnicodeAction(
            /* [in] */ User *users,
            /* [in] */ const unsigned short *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUnicodePage(
            User *user,
            const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSetPlayerCount(
            unsigned int currentPlayers,
            unsigned int maxPlayers) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestServerTime( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestInsiderStatus(
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSetLocalIP( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestSquadByName(
            const char *name) = 0;

    };

    WOLAPI_INTERFACE("4DD3BAF6-7579-11D1-B1C6-006097176556")
    IChatEvent : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnServerList(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Server *servers) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUpdateList(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Update *updates) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnServerError(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ const char *ircmsg) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnConnection(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ const char *motd) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnMessageOfTheDay(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ const char *motd) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelList(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channels) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelCreate(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelJoin(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelLeave(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelTopic(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ const char *topic) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPrivateAction(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user,
            /* [in] */ const char *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPublicAction(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            User *user,
            /* [in] */ const char *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserList(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPublicMessage(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *user,
            /* [in] */ const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPrivateMessage(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user,
            /* [in] */ const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnSystemMessage(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnNetStatus(
            /* [in] */ WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnLogout(
            /* [in] */ WOLAPI_RESULT status,
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPrivateGameOptions(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user,
            /* [in] */ const char *options) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPublicGameOptions(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *user,
            /* [in] */ const char *options) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnGameStart(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *users,
            /* [in] */ int gameid) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserKick(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *kicked,
            /* [in] */ User *kicker) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserIP(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnFind(
            WOLAPI_RESULT res,
            Channel *chan) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPageSend(
            WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPaged(
            WOLAPI_RESULT res,
            User *user,
            const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnServerBannedYou(
            WOLAPI_RESULT res,
            time_t bannedTill) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserFlags(
            WOLAPI_RESULT res,
            const char *name,
            unsigned int flags,
            unsigned int mask) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelBan(
            WOLAPI_RESULT res,
            const char *name,
            int banned) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnSquadInfo(
            WOLAPI_RESULT res,
            unsigned int id,
            Squad *squad) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserLocale(
            WOLAPI_RESULT res,
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserTeam(
            WOLAPI_RESULT res,
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnSetLocale(
            WOLAPI_RESULT res,
            Locale newlocale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnSetTeam(
            WOLAPI_RESULT res,
            int newteam) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnBuddyList(
            WOLAPI_RESULT res,
            User *buddy_list) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnBuddyAdd(
            WOLAPI_RESULT res,
            User *buddy_added) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnBuddyDelete(
            WOLAPI_RESULT res,
            User *buddy_deleted) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPublicUnicodeMessage(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            /* [in] */ User *user,
            /* [in] */ const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPrivateUnicodeMessage(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user,
            /* [in] */ const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPrivateUnicodeAction(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ User *user,
            /* [in] */ const unsigned short *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPublicUnicodeAction(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel,
            User *user,
            /* [in] */ const unsigned short *action) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPagedUnicode(
            WOLAPI_RESULT res,
            User *user,
            const unsigned short *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnServerTime(
            WOLAPI_RESULT res,
            time_t stime) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnInsiderStatus(
            WOLAPI_RESULT res,
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnSetLocalIP(
            WOLAPI_RESULT res,
            const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelListBegin(
            /* [in] */ WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelListEntry(
            /* [in] */ WOLAPI_RESULT res,
            /* [in] */ Channel *channel) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelListEnd(
            /* [in] */ WOLAPI_RESULT res) = 0;

    };

    WOLAPI_INTERFACE("0BF5FCEB-9F03-11D1-9DC7-006097C54321")
    IDownload : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE DownloadFile(
            const char *server,
            const char *login,
            const char *password,
            const char *file,
            const char *localfile,
            const char *regkey) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Abort( void) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE PumpMessages( void) = 0;

    };


    WOLAPI_INTERFACE("6869E99D-9FB4-11D1-9DC8-006097C54321")
    IDownloadEvent : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnEnd( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnError(
            int error) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnProgressUpdate(
            int bytesread,
            int totalsize,
            int timetaken,
            int timeleft) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnQueryResume( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnStatusUpdate(
            int status) = 0;

    };


    WOLAPI_INTERFACE("B832B0AA-A7D3-11D1-97C3-00609706FA0C")
    INetUtil : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestGameresSend(
            const char *host,
            int port,
            unsigned char *data,
            int length) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLadderSearch(
            const char *host,
            int port,
            const char *key,
            unsigned int SKU,
            int team,
            int cond,
            int sort,
            int number,
            int leading) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLadderList(
            const char *host,
            int port,
            const char *keys,
            unsigned int SKU,
            int team,
            int cond,
            int sort) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestPing(
            const char *host,
            int timeout,
            int *handle) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE PumpMessages( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetAvgPing(
            unsigned int ip,
            int *avg) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestNewNick(
            const char *nick,
            const char *pass,
            const char *email,
            const char *parentEmail,
            int newsletter,
            int shareinfo) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestAgeCheck(
            int month,
            int day,
            int year,
            const char *email) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestWDTState(
            const char *host,
            int port,
            unsigned char request) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLocaleLadderList(
            const char *host,
            int port,
            const char *keys,
            unsigned int SKU,
            int team,
            int cond,
            int sort,
            Locale locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLocaleLadderSearch(
            const char *host,
            int port,
            const char *key,
            unsigned int sku,
            int team,
            int cond,
            int sort,
            int number,
            int leading,
            Locale locale) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestHighscore(
            const char *host,
            int port,
            const char *keys,
            unsigned int SKU) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE SetGameResMD5(
            int flag) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLargeGameresSend(
            const char *host,
            int port,
            unsigned char *data,
            unsigned int length) = 0;

    };


    WOLAPI_INTERFACE("B832B0AC-A7D3-11D1-97C3-00609706FA0C")
    INetUtilEvent : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnPing(
            WOLAPI_RESULT res,
            int time,
            unsigned int ip,
            int handle) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnLadderList(
            WOLAPI_RESULT res,
            /* [in] */ Ladder *list,
            int totalCount,
            int timeStamp,
            int keyRung) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnGameresSent(
            WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnNewNick(
            WOLAPI_RESULT res,
            const char *message,
            const char *nick,
            const char *pass) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnAgeCheck(
            WOLAPI_RESULT res,
            int years,
            int consent) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnWDTState(
            WOLAPI_RESULT res,
            unsigned char *state,
            int length) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnHighscore(
            WOLAPI_RESULT res,
            /* [in] */ Highscore *list,
            int totalCount,
            int timeStamp,
            int keyRung) = 0;

    };

typedef unsigned int GID;


enum GTYPE_
    {	SERVER	= 0,
	CHANNEL	= 1,
	CLIENT	= 2
    };
typedef enum GTYPE_ GTYPE;


enum CHAN_CTYPE_
    {	ALLEXIT	= 0,
	CREATOREXIT	= 1,
	CLOSEC	= 2
    };
typedef enum CHAN_CTYPE_ CHAN_CTYPE;


    WOLAPI_INTERFACE("8B938190-EF3F-11D1-9808-00609706FA0C")
    IChat2 : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE PumpMessages( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestConnection(
            Server *server,
            int timeout) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestMessage(
            GID who,
            const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE GetTypeFromGID(
            GID id,
            GTYPE *type) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelList( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelJoin(
            const char *name) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelLeave(
            Channel *chan) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestUserList(
            Channel *chan) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestLogout( void) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestChannelCreate(
            Channel *chan) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE RequestRawCmd(
            const char *cmd) = 0;

    };


    WOLAPI_INTERFACE("8B938192-EF3F-11D1-9808-00609706FA0C")
    IChat2Event : public IUnknown
    {
    public:
        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnNetStatus(
            WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnMessage(
            WOLAPI_RESULT res,
            User *user,
            const char *message) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelList(
            WOLAPI_RESULT res,
            Channel *list) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelJoin(
            WOLAPI_RESULT res,
            Channel *chan,
            User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnLogin(
            WOLAPI_RESULT res) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUserList(
            WOLAPI_RESULT res,
            Channel *chan,
            User *users) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelLeave(
            WOLAPI_RESULT res,
            Channel *chan,
            User *user) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnChannelCreate(
            WOLAPI_RESULT res,
            Channel *chan) = 0;

        virtual /* [helpstring] */ WOLAPI_RESULT WOLAPI_CALLTYPE OnUnknownLine(
            WOLAPI_RESULT res,
            const char *line) = 0;

    };


    WOLAPI_INTERFACE("89DD1ECD-0DCA-49d8-8EF3-3375E6D6EE9D")
    IIGROptions : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Init( void) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Is_Auto_Login_Allowed( void) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Is_Storing_Nicks_Allowed( void) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Is_Running_Reg_App_Allowed( void) = 0;

        virtual /* [helpstring][id] */ WOLAPI_RESULT WOLAPI_CALLTYPE Set_Options(
            unsigned int options) = 0;

    };

}

#endif

// $Id: clif.c 164 2004-10-01 16:46:58Z $

#define DUMP_UNKNOWN_PACKET	1

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "socket.h"
#include "timer.h"
#include "malloc.h"
#include "version.h"
#include "nullpo.h"

#include "map.h"
#include "chrif.h"
#include "clif.h"
#include "pc.h"
#include "npc.h"
#include "itemdb.h"
#include "chat.h"
#include "trade.h"
#include "storage.h"
#include "script.h"
#include "skill.h"
#include "atcommand.h"
#include "intif.h"
#include "battle.h"
#include "mob.h"
#include "party.h"
#include "guild.h"
#include "vending.h"
#include "pet.h"
#include "mmo.h"

#ifdef MEMWATCH
#include "memwatch.h"
#endif

#define STATE_BLIND 0x10

/* Packet Database */
struct packet_db packet_db[MAX_PACKET_DB];

// local define
enum {
	ALL_CLIENT,
	ALL_SAMEMAP,
	AREA,
	AREA_WOS,
	AREA_WOC,
	AREA_WOSC,
	AREA_CHAT_WOC,
	CHAT,
	CHAT_WOS,
	PARTY,
	PARTY_WOS,
	PARTY_SAMEMAP,
	PARTY_SAMEMAP_WOS,
	PARTY_AREA,
	PARTY_AREA_WOS,
	GUILD,
	GUILD_WOS,
	GUILD_SAMEMAP,	// [Valaris]
	GUILD_SAMEMAP_WOS,
	GUILD_AREA,
	GUILD_AREA_WOS,	// end additions [Valaris]
	SELF
};

#define WBUFPOS(p,pos,x,y) { unsigned char *__p = (p); __p+=(pos); __p[0] = (x)>>2; __p[1] = ((x)<<6) | (((y)>>4)&0x3f); __p[2] = (y)<<4; }
#define WBUFPOS2(p,pos,x0,y0,x1,y1) { unsigned char *__p = (p); __p+=(pos); __p[0] = (x0)>>2; __p[1] = ((x0)<<6) | (((y0)>>4)&0x3f); __p[2] = ((y0)<<4) | (((x1)>>6)&0x0f); __p[3]=((x1)<<2) | (((y1)>>8)&0x03); __p[4]=(y1); }

#define WFIFOPOS(fd,pos,x,y) { WBUFPOS (WFIFOP(fd,pos),0,x,y); }
#define WFIFOPOS2(fd,pos,x0,y0,x1,y1) { WBUFPOS2(WFIFOP(fd,pos),0,x0,y0,x1,y1); }

static char map_ip_str[16];
static in_addr_t map_ip;
static int map_port = 5121;
int map_fd;
char talkie_mes[80];

/*==========================================
 * Clif Parsing Functions
 *------------------------------------------
 */
void clif_parse_WantToConnection(int fd,struct map_session_data *sd, int cmd);
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TickSend(int fd,struct map_session_data *sd, int cmd);
void clif_parse_WalkToXY(int fd,struct map_session_data *sd, int cmd);
void clif_parse_QuitGame(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GetCharNameRequest(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GlobalMessage(int fd,struct map_session_data *sd, int cmd);
void clif_parse_MapMove(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChangeDir(int fd,struct map_session_data *sd, int cmd);
void clif_parse_Emotion(int fd,struct map_session_data *sd, int cmd);
void clif_parse_HowManyConnections(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ActionRequest(int fd,struct map_session_data *sd, int cmd);
void clif_parse_Restart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_Wis(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMmessage(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TakeItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_DropItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UseItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_EquipItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UnequipItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcClicked(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcBuySellSelected(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcBuyListSend(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcSellListSend(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CreateChatRoom(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChatAddMember(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChatRoomStatusChange(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChangeChatOwner(int fd,struct map_session_data *sd, int cmd);
void clif_parse_KickFromChat(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChatLeave(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeRequest(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeAck(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeAddItem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeOk(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeCancel(int fd,struct map_session_data *sd, int cmd);
void clif_parse_TradeCommit(int fd,struct map_session_data *sd, int cmd);
void clif_parse_StopAttack(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PutItemToCart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GetItemFromCart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_RemoveOption(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChangeCart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_StatusUp(int fd,struct map_session_data *sd, int cmd);
void clif_parse_SkillUp(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UseSkillToId(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UseSkillToPos(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UseSkillMap(int fd,struct map_session_data *sd, int cmd);
void clif_parse_RequestMemo(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ProduceMix(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcSelectMenu(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcNextClicked(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcAmountInput(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcStringInput(int fd,struct map_session_data *sd, int cmd);
void clif_parse_NpcCloseClicked(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ItemIdentify(int fd,struct map_session_data *sd, int cmd);
void clif_parse_SelectArrow(int fd,struct map_session_data *sd, int cmd);
void clif_parse_AutoSpell(int fd,struct map_session_data *sd, int cmd);
void clif_parse_UseCard(int fd,struct map_session_data *sd, int cmd);
void clif_parse_InsertCard(int fd,struct map_session_data *sd, int cmd);
void clif_parse_SolveCharName(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ResetChar(int fd,struct map_session_data *sd, int cmd);
void clif_parse_LGMmessage(int fd,struct map_session_data *sd, int cmd);
void clif_parse_MoveToKafra(int fd,struct map_session_data *sd, int cmd);
void clif_parse_MoveFromKafra(int fd,struct map_session_data *sd, int cmd);
void clif_parse_MoveToKafraFromCart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_MoveFromKafraToCart(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CloseKafra(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CreateParty(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CreateParty2(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PartyInvite(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ReplyPartyInvite(int fd,struct map_session_data *sd, int cmd);
void clif_parse_LeaveParty(int fd,struct map_session_data *sd, int cmd);
void clif_parse_RemovePartyMember(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PartyChangeOption(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PartyMessage(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CloseVending(int fd,struct map_session_data *sd, int cmd);
void clif_parse_VendingListReq(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PurchaseReq(int fd,struct map_session_data *sd, int cmd);
void clif_parse_OpenVending(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CreateGuild(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildCheckMaster(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildRequestInfo(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildChangePositionInfo(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildChangeMemberPosition(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildRequestEmblem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildChangeEmblem(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildChangeNotice(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildInvite(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildReplyInvite(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildLeave(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildExplusion(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildMessage(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildRequestAlliance(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildReplyAlliance(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildDelAlliance(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildOpposition(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GuildBreak(int fd,struct map_session_data *sd, int cmd);
void clif_parse_PetMenu(int fd,struct map_session_data *sd, int cmd);
void clif_parse_CatchPet(int fd,struct map_session_data *sd, int cmd);
void clif_parse_SelectEgg(int fd,struct map_session_data *sd, int cmd);
void clif_parse_SendEmotion(int fd,struct map_session_data *sd, int cmd);
void clif_parse_ChangePetName(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMKick(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMHide(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMReqNoChat(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMReqNoChatCount(int fd,struct map_session_data *sd, int cmd);
void clif_parse_sn_doridori(int fd,struct map_session_data *sd, int cmd);
void clif_parse_sn_explosionspirits(int fd,struct map_session_data *sd, int cmd);
void clif_parse_wisexin(int fd,struct map_session_data *sd, int cmd);
void clif_parse_wisall(int fd,struct map_session_data *sd, int cmd);
void clif_parse_wisexlist(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMkillall(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMsummon(int fd,struct map_session_data *sd, int cmd);
void clif_parse_GMshift(int fd,struct map_session_data *sd, int cmd);
void clif_parse_debug(int fd,struct map_session_data *sd, int cmd);

struct {
	void (*func)();
	char *name;
} clif_parse_func[]={
	{clif_parse_WantToConnection,"wanttoconnection"},
	{clif_parse_LoadEndAck,"loadendack"},
	{clif_parse_TickSend,"ticksend"},
	{clif_parse_WalkToXY,"walktoxy"},
	{clif_parse_QuitGame,"quitgame"},
	{clif_parse_GetCharNameRequest,"getcharnamerequest"},
	{clif_parse_GlobalMessage,"globalmessage"},
	{clif_parse_MapMove,"mapmove"},
	{clif_parse_ChangeDir,"changedir"},
	{clif_parse_Emotion,"emotion"},
	{clif_parse_HowManyConnections,"howmanyconnections"},
	{clif_parse_ActionRequest,"actionrequest"},
	{clif_parse_Restart,"restart"},
	{clif_parse_Wis,"wis"},
	{clif_parse_GMmessage,"gmmessage"},
	{clif_parse_TakeItem,"takeitem"},
	{clif_parse_DropItem,"dropitem"},
	{clif_parse_UseItem,"useitem"},
	{clif_parse_EquipItem,"equipitem"},
	{clif_parse_UnequipItem,"unequipitem"},
	{clif_parse_NpcClicked,"npcclicked"},
	{clif_parse_NpcBuySellSelected,"npcbuysellselected"},
	{clif_parse_NpcBuyListSend,"npcbuylistsend"},
	{clif_parse_NpcSellListSend,"npcselllistsend"},
	{clif_parse_CreateChatRoom,"createchatroom"},
	{clif_parse_ChatAddMember,"chataddmember"},
	{clif_parse_ChatRoomStatusChange,"chatroomstatuschange"},
	{clif_parse_ChangeChatOwner,"changechatowner"},
	{clif_parse_KickFromChat,"kickfromchat"},
	{clif_parse_ChatLeave,"chatleave"},
	{clif_parse_TradeRequest,"traderequest"},
	{clif_parse_TradeAck,"tradeack"},
	{clif_parse_TradeAddItem,"tradeadditem"},
	{clif_parse_TradeOk,"tradeok"},
	{clif_parse_TradeCancel,"tradecancel"},
	{clif_parse_TradeCommit,"tradecommit"},
	{clif_parse_StopAttack,"stopattack"},
	{clif_parse_PutItemToCart,"putitemtocart"},
	{clif_parse_GetItemFromCart,"getitemfromcart"},
	{clif_parse_RemoveOption,"removeoption"},
	{clif_parse_ChangeCart,"changecart"},
	{clif_parse_StatusUp,"statusup"},
	{clif_parse_SkillUp,"skillup"},
	{clif_parse_UseSkillToId,"useskilltoid"},
	{clif_parse_UseSkillToPos,"useskilltopos"},
	{clif_parse_UseSkillMap,"useskillmap"},
	{clif_parse_RequestMemo,"requestmemo"},
	{clif_parse_ProduceMix,"producemix"},
	{clif_parse_NpcSelectMenu,"npcselectmenu"},
	{clif_parse_NpcNextClicked,"npcnextclicked"},
	{clif_parse_NpcAmountInput,"npcamountinput"},
	{clif_parse_NpcStringInput,"npcstringinput"},
	{clif_parse_NpcCloseClicked,"npccloseclicked"},
	{clif_parse_ItemIdentify,"itemidentify"},
	{clif_parse_SelectArrow,"selectarrow"},
	{clif_parse_AutoSpell,"autospell"},
	{clif_parse_UseCard,"usecard"},
	{clif_parse_InsertCard,"insertcard"},
	{clif_parse_SolveCharName,"solvecharname"},
	{clif_parse_ResetChar,"resetchar"},
	{clif_parse_LGMmessage,"lgmmessage"},
	{clif_parse_MoveToKafra,"movetokafra"},
	{clif_parse_MoveFromKafra,"movefromkafra"},
	{clif_parse_MoveToKafraFromCart,"movetokafrafromcart"},
	{clif_parse_MoveFromKafraToCart,"movefromkafratocart"},
	{clif_parse_CloseKafra,"closekafra"},
	{clif_parse_CreateParty,"createparty"},
	{clif_parse_CreateParty2,"createparty2"},
	{clif_parse_PartyInvite,"partyinvite"},
	{clif_parse_ReplyPartyInvite,"replypartyinvite"},
	{clif_parse_LeaveParty,"leaveparty"},
	{clif_parse_RemovePartyMember,"removepartymember"},
	{clif_parse_PartyChangeOption,"partychangeoption"},
	{clif_parse_PartyMessage,"partymessage"},
	{clif_parse_CloseVending,"closevending"},
	{clif_parse_VendingListReq,"vendinglistreq"},
	{clif_parse_PurchaseReq,"purchasereq"},
	{clif_parse_OpenVending,"openvending"},
	{clif_parse_CreateGuild,"createguild"},
	{clif_parse_GuildCheckMaster,"guildcheckmaster"},
	{clif_parse_GuildRequestInfo,"guildrequestinfo"},
	{clif_parse_GuildChangePositionInfo,"guildchangepositioninfo"},
	{clif_parse_GuildChangeMemberPosition,"guildchangememberposition"},
	{clif_parse_GuildRequestEmblem,"guildrequestemblem"},
	{clif_parse_GuildChangeEmblem,"guildchangeemblem"},
	{clif_parse_GuildChangeNotice,"guildchangenotice"},
	{clif_parse_GuildInvite,"guildinvite"},
	{clif_parse_GuildReplyInvite,"guildreplyinvite"},
	{clif_parse_GuildLeave,"guildleave"},
	{clif_parse_GuildExplusion,"guildexplusion"},
	{clif_parse_GuildMessage,"guildmessage"},
	{clif_parse_GuildRequestAlliance,"guildrequestalliance"},
	{clif_parse_GuildReplyAlliance,"guildreplyalliance"},
	{clif_parse_GuildDelAlliance,"guilddelalliance"},
	{clif_parse_GuildOpposition,"guildopposition"},
	{clif_parse_GuildBreak,"guildbreak"},
	{clif_parse_PetMenu,"petmenu"},
	{clif_parse_CatchPet,"catchpet"},
	{clif_parse_SelectEgg,"selectegg"},
	{clif_parse_SendEmotion,"sendemotion"},
	{clif_parse_ChangePetName,"changepetname"},
	{clif_parse_GMKick,"gmkick"},
	{clif_parse_GMHide,"gmhide"},
	{clif_parse_GMReqNoChat,"gmreqnochat"},
	{clif_parse_GMReqNoChatCount,"gmreqnochatcount"},
	{clif_parse_sn_doridori,"sndoridori"},
	{clif_parse_sn_explosionspirits,"snexplosionspirits"},
	{clif_parse_wisexin,"wisexin"},
	{clif_parse_wisexlist,"wisexlist"},
	{clif_parse_wisall,"wisall"},
	{clif_parse_GMkillall,"killall"},
	{clif_parse_GMsummon,"summon"},
	{clif_parse_GMshift,"shift"},
	{clif_parse_debug,"debug"},

	{NULL,NULL}
};

/*==========================================
 * map鯖のip設定
 *------------------------------------------
 */
void clif_setip(char *ip)
{
	memcpy(map_ip_str,ip,16);
	map_ip=inet_addr(map_ip_str);
}

/*==========================================
 * map鯖のport設定
 *------------------------------------------
 */
void clif_setport(int port)
{
	map_port=port;
}

/*==========================================
 * map鯖のip読み出し
 *------------------------------------------
 */
in_addr_t clif_getip(void)
{
	return map_ip;
}

/*==========================================
 * map鯖のport読み出し
 *------------------------------------------
 */
int clif_getport(void)
{
	return map_port;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_countusers(void)
{
	int users=0,i;
	struct map_session_data *sd;

	for(i=0;i<fd_max;i++){
		if(session[i] && (sd=session[i]->session_data) && sd->state.auth
			&& !(battle_config.hide_GM_session && pc_isGM(sd)) )
			users++;
	}
	return users;
}

/*==========================================
 * 全てのclientに対してfunc()実行
 *------------------------------------------
 */
int clif_foreachclient(int (*func)(struct map_session_data*,va_list),...)
{
	int i;
	va_list ap;
	struct map_session_data *sd;

	va_start(ap,func);
	for(i=0;i<fd_max;i++){
		if(session[i] && (sd=session[i]->session_data) && sd->state.auth)
			func(sd,ap);
	}
	va_end(ap);
	return 0;
}

/*==========================================
 * clif_sendでAREA*指定時用
 *------------------------------------------
 */
int clif_send_sub(struct block_list *bl,va_list ap)
{
	unsigned char *buf;
	int len;
	struct block_list *src_bl;
	int type;
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, sd=(struct map_session_data *)bl);

	buf=va_arg(ap,unsigned char*);
	len=va_arg(ap,int);
	nullpo_retr(0, src_bl=va_arg(ap,struct block_list*));
	type=va_arg(ap,int);

	switch(type){
	case AREA_WOS:
		if(bl && bl==src_bl)
			return 0;
		break;
	case AREA_WOC:
		if(sd->chatID || bl==src_bl)
			return 0;
		break;
	case AREA_WOSC:
		if ((sd) && sd->chatID && sd->chatID == ((struct map_session_data*)src_bl)->chatID)
			return 0;
		break;
	}

	if (sd) {
		if (WFIFOP(sd->fd,0) == buf) {
			printf("WARNING: Invalid use of clif_send function\n");
			printf("         Packet x%4x use a WFIFO of a player instead of to use a buffer.\n", WBUFW(buf,0));
			printf("         Please correct your code.\n");
			// don't send to not move the pointer of the packet for next sessions in the loop
		} else {
			memcpy(WFIFOP(sd->fd,0), buf, len);
			WFIFOSET(sd->fd,len);
		}
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_send(unsigned char *buf,int len,struct block_list *bl,int type)
{
	int i;
	struct map_session_data *sd;
	struct chat_data *cd;
	struct party *p=NULL;
	struct guild *g=NULL;
	int x0=0,x1=0,y0=0,y1=0;

	if( type != ALL_CLIENT ){
		nullpo_retr(0, bl);
	}

	switch(type){
	case ALL_CLIENT:	// 全クライアントに送信
		for(i=0;i<fd_max;i++){
			if(session[i] && (sd=session[i]->session_data) && sd->state.auth){
				memcpy(WFIFOP(i,0),buf,len);
				WFIFOSET(i,len);
			}
		}
		break;
	case ALL_SAMEMAP:	// 同じマップの全クライアントに送信
		for(i=0;i<fd_max;i++){
			if(session[i] && (sd=session[i]->session_data) && sd->state.auth &&
			   sd->bl.m == bl->m){
				memcpy(WFIFOP(i,0),buf,len);
				WFIFOSET(i,len);
			}
		}
		break;
	case AREA:
	case AREA_WOS:
	case AREA_WOC:
	case AREA_WOSC:
		map_foreachinarea(clif_send_sub,bl->m,bl->x-AREA_SIZE,bl->y-AREA_SIZE,bl->x+AREA_SIZE,bl->y+AREA_SIZE,BL_PC,buf,len,bl,type);
		break;
	case AREA_CHAT_WOC:
		map_foreachinarea(clif_send_sub,bl->m,bl->x-(AREA_SIZE-5),bl->y-(AREA_SIZE-5),bl->x+(AREA_SIZE-5),bl->y+(AREA_SIZE-5),BL_PC,buf,len,bl,AREA_WOC);
		break;
	case CHAT:
	case CHAT_WOS:
		cd=(struct chat_data*)bl;
		if(bl->type==BL_PC){
			sd=(struct map_session_data*)bl;
			cd=(struct chat_data*)map_id2bl(sd->chatID);
		} else if(bl->type!=BL_CHAT)
			break;
		if(cd==NULL)
			break;
		for(i=0;i<cd->users;i++){
			if(type==CHAT_WOS && cd->usersd[i]==(struct map_session_data*)bl)
				continue;
			memcpy(WFIFOP(cd->usersd[i]->fd,0),buf,len);
			WFIFOSET(cd->usersd[i]->fd,len);
		}
		break;

	case PARTY_AREA:		// 同じ画面内の全パーティーメンバに送信
	case PARTY_AREA_WOS:	// 自分以外の同じ画面内の全パーティーメンバに送信
		x0=bl->x-AREA_SIZE;
		y0=bl->y-AREA_SIZE;
		x1=bl->x+AREA_SIZE;
		y1=bl->y+AREA_SIZE;
	case PARTY:				// 全パーティーメンバに送信
	case PARTY_WOS:			// 自分以外の全パーティーメンバに送信
	case PARTY_SAMEMAP:		// 同じマップの全パーティーメンバに送信
	case PARTY_SAMEMAP_WOS:	// 自分以外の同じマップの全パーティーメンバに送信
		if (bl->type == BL_PC) {
			sd=(struct map_session_data *)bl;
			if (sd->partyspy > 0) {
				p = party_search(sd->partyspy);
			} else {
				if (sd->status.party_id > 0)
					p = party_search(sd->status.party_id);
			}
		}
		if (p) {
			for(i=0;i<MAX_PARTY;i++){
				if ((sd = p->member[i].sd) != NULL) {
					if (sd->bl.id == bl->id && (type == PARTY_WOS ||
					    type == PARTY_SAMEMAP_WOS || type == PARTY_AREA_WOS))
						continue;
					if (type != PARTY && type != PARTY_WOS && bl->m != sd->bl.m) // マップチェック
						continue;
					if ((type == PARTY_AREA || type == PARTY_AREA_WOS) &&
					    (sd->bl.x < x0 || sd->bl.y < y0 ||
					     sd->bl.x > x1 || sd->bl.y > y1))
						continue;
					memcpy(WFIFOP(sd->fd,0), buf, len);
					WFIFOSET(sd->fd,len);
//					if(battle_config.etc_log)
//						printf("send party %d %d %d\n",p->party_id,i,flag)

				}
			}
			for (i = 0; i < fd_max; i++){
				if (session[i] && (sd = session[i]->session_data) != NULL && sd->state.auth) {
					if (sd->partyspy == p->party_id) {
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
		}
		break;
	case SELF:
		sd=(struct map_session_data *)bl;
		memcpy(WFIFOP(sd->fd,0),buf,len);
		WFIFOSET(sd->fd,len);
		break;

/* New definitions for guilds [Valaris]	*/

	case GUILD_AREA:
	case GUILD_AREA_WOS:
		x0 = bl->x - AREA_SIZE;
		y0 = bl->y - AREA_SIZE;
		x1 = bl->x + AREA_SIZE;
		y1 = bl->y + AREA_SIZE;
	case GUILD:
	case GUILD_WOS:
		if (bl && bl->type == BL_PC) { // guildspy [Syrus22]
			sd=(struct map_session_data *)bl;
			if (sd->guildspy > 0) {
				g = guild_search(sd->guildspy);
			} else {
				if (sd->status.guild_id > 0)
					g = guild_search(sd->status.guild_id);
			}
		}
		if(g){
			for(i=0;i<g->max_member;i++){
				if((sd=g->member[i].sd)!=NULL){
					if(type==GUILD_WOS && sd->bl.id==bl->id)
						continue;
					memcpy(WFIFOP(sd->fd,0),buf,len);
					WFIFOSET(sd->fd,len);
				}
			}
			for (i = 0; i < fd_max; i++){
				if (session[i] && (sd = session[i]->session_data) != NULL && sd->state.auth) {
					if (sd->guildspy == g->guild_id) {
						memcpy(WFIFOP(sd->fd,0), buf, len);
						WFIFOSET(sd->fd,len);
					}
				}
			}
		}
		break;
	case GUILD_SAMEMAP:
	case GUILD_SAMEMAP_WOS:
		if (bl->type == BL_PC) {
			sd = (struct map_session_data *)bl;
			if (sd->status.guild_id > 0)
				g = guild_search(sd->status.guild_id);
		}
		if (g) {
			for(i = 0; i < g->max_member; i++) {
				if ((sd = g->member[i].sd) != NULL) {
					if (sd->bl.id == bl->id && (type == GUILD_WOS ||
					    type == GUILD_SAMEMAP_WOS || type == GUILD_AREA_WOS))
						continue;
					if (type != GUILD && type != GUILD_WOS && bl->m != sd->bl.m) // マップチェック
						continue;
					if ((type == GUILD_AREA || type == GUILD_AREA_WOS) &&
					    (sd->bl.x < x0 || sd->bl.y < y0 ||
					     sd->bl.x > x1 || sd->bl.y > y1))
						continue;
					memcpy(WFIFOP(sd->fd,0), buf, len);
					WFIFOSET(sd->fd,len);
				}
			}
		}
		break;
/*				End [Valaris]			*/

	default:
		if(battle_config.error_log)
			printf("clif_send まだ作ってないよー\n");
		return -1;
	}

	return 0;
}

//
// パケット作って送信
//
/*==========================================
 *
 *------------------------------------------
 */
int clif_authok(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	if (!sd)
		return 0;

	if (!sd->fd)
		return 0;

	fd = sd->fd;

	WFIFOW(fd, 0) = 0x73;
	WFIFOL(fd, 2) = gettick();
	WFIFOPOS(fd, 6, sd->bl.x, sd->bl.y);
	WFIFOB(fd, 9) = 5;
	WFIFOB(fd,10) = 5;
	WFIFOSET(fd,packet_db[0x73].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_authfail_fd(int fd, int type) {
	if (!fd || !session[fd])
		return 0;

	WFIFOW(fd,0) = 0x81;
	WFIFOL(fd,2) = type;
	WFIFOSET(fd,packet_db[0x81].len);

	clif_setwaitclose(fd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_charselectok(int id)
{
	struct map_session_data *sd;
	int fd;

	if((sd=map_id2sd(id))==NULL)
		return 1;

	if (!sd->fd)
		return 1;

	fd = sd->fd;
	WFIFOW(fd,0) = 0xb3;
	WFIFOB(fd,2) = 1;
	WFIFOSET(fd,packet_db[0xb3].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_set009e(struct flooritem_data *fitem,unsigned char *buf)
{
	int view;

	nullpo_retr(0, fitem);

	//009e <ID>.l <name ID>.w <identify flag>.B <X>.w <Y>.w <subX>.B <subY>.B <amount>.w
	WBUFW(buf,0)=0x9e;
	WBUFL(buf,2)=fitem->bl.id;
	if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
		WBUFW(buf,6)=view;
	else
		WBUFW(buf,6)=fitem->item_data.nameid;
	WBUFB(buf,8)=fitem->item_data.identify;
	WBUFW(buf,9)=fitem->bl.x;
	WBUFW(buf,11)=fitem->bl.y;
	WBUFB(buf,13)=fitem->subx;
	WBUFB(buf,14)=fitem->suby;
	WBUFW(buf,15)=fitem->item_data.amount;

	return packet_db[0x9e].len;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_dropflooritem(struct flooritem_data *fitem)
{
	char buf[64];

	nullpo_retr(0, fitem);

	if(fitem->item_data.nameid <= 0)
		return 0;
	clif_set009e(fitem,buf);
	clif_send(buf,packet_db[0x9e].len,&fitem->bl,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearflooritem(struct flooritem_data *fitem,int fd)
{
	unsigned char buf[16];

	nullpo_retr(0, fitem);

	WBUFW(buf,0) = 0xa1;
	WBUFL(buf,2) = fitem->bl.id;

	if(fd==0){
		clif_send(buf,packet_db[0xa1].len,&fitem->bl,AREA);
	} else {
		memcpy(WFIFOP(fd,0),buf,6);
		WFIFOSET(fd,packet_db[0xa1].len);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchar(struct block_list *bl,int type)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = bl->id;
	if (type == 9) {
		WBUFB(buf,6) = 0;
		clif_send(buf, packet_db[0x80].len, bl, AREA);
	} else {
		WBUFB(buf,6) = type;
		clif_send(buf, packet_db[0x80].len, bl, type == 1 ? AREA : AREA_WOS);
	}

	return 0;
}

static int clif_clearchar_delay_sub(int tid,unsigned int tick,int id,int data)
{
	struct block_list *bl = (struct block_list *)id;

	clif_clearchar(bl,data);
	map_freeblock(bl);

	return 0;
}

int clif_clearchar_delay(unsigned int tick, struct block_list *bl, int type) {
	struct block_list *tmpbl=(struct block_list *)aCalloc(1,sizeof(struct block_list));
	if (tmpbl == NULL) {
		printf("clif_clearchar_delay: out of memory !\n");
		exit(1);
	}
	memcpy(tmpbl, bl, sizeof(struct block_list));
	add_timer(tick, clif_clearchar_delay_sub, (int)tmpbl, type);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchar_id(int id,int type,int fd)
{
	unsigned char buf[16];

	WBUFW(buf,0) = 0x80;
	WBUFL(buf,2) = id;
	WBUFB(buf,6) = type;
	memcpy(WFIFOP(fd,0),buf,7);
	WFIFOSET(fd,packet_db[0x80].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_set0078(struct map_session_data *sd, unsigned char *buf) {
	int level=0;

	nullpo_retr(0, sd);

	if (sd->disguise > 23 && sd->disguise < 4001) { // mob disguises [Valaris]
		WBUFW(buf,0) = 0x78;
		WBUFL(buf,2) = sd->bl.id;
		WBUFW(buf,6) = battle_get_speed(&sd->bl);
		WBUFW(buf,8) = sd->opt1;
		WBUFW(buf,10) = sd->opt2;
		WBUFW(buf,12) = sd->status.option;
		WBUFW(buf,14) = sd->disguise;
		WBUFW(buf,42) = 0;
		WBUFB(buf,44) = 0;
		WBUFPOS(buf, 46, sd->bl.x, sd->bl.y);
		WBUFB(buf,48) |= sd->dir & 0x0f;
		WBUFB(buf,49) = 5;
		WBUFB(buf,50) = 5;
		WBUFB(buf,51) = 0;
		WBUFW(buf,52) = ((level = battle_get_lv(&sd->bl)) > battle_config.max_lv) ? battle_config.max_lv : level;

		return packet_db[0x78].len;
	}

#if PACKETVER < 4
	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->speed;
	WBUFW(buf,8)=sd->opt1;
	WBUFW(buf,10)=sd->opt2;
	WBUFW(buf,12)=sd->status.option;
	WBUFW(buf,14)=sd->view_class;
	WBUFW(buf,16)=sd->status.hair;
	if(sd->view_class != 22)
		WBUFW(buf,18)=sd->status.weapon;
	else
		WBUFW(buf,18)=0;
	WBUFW(buf,20)=sd->status.head_bottom;
	WBUFW(buf,22)=sd->status.shield;
	WBUFW(buf,24)=sd->status.head_top;
	WBUFW(buf,26)=sd->status.head_mid;
	WBUFW(buf,28)=sd->status.hair_color;
	WBUFW(buf,30)=sd->status.clothes_color;
	WBUFW(buf,32)=sd->head_dir;
	WBUFL(buf,34)=sd->status.guild_id;
	WBUFL(buf,38)=sd->guild_emblem_id;
	WBUFW(buf,42)=sd->status.manner;
	WBUFB(buf,44)=sd->status.karma;
	WBUFB(buf,45)=sd->sex;
	WBUFPOS(buf,46,sd->bl.x,sd->bl.y);
	WBUFB(buf,48)|=sd->dir&0x0f;
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFB(buf,51)=sd->state.dead_sit;
	WBUFW(buf,52)=(sd->status.base_level>battle_config.max_lv)?battle_config.max_lv:sd->status.base_level;

	return packet_db[0x78].len;
#else
	WBUFW(buf,0)=0x1d8;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->speed;
	WBUFW(buf,8)=sd->opt1;
	WBUFW(buf,10)=sd->opt2;
	WBUFW(buf,12)=sd->status.option;
	WBUFW(buf,14)=sd->view_class;
	WBUFW(buf,16)=sd->status.hair;
	if(sd->equip_index[9] >= 0 && sd->inventory_data[sd->equip_index[9]] && sd->view_class != 22) {
		if(sd->inventory_data[sd->equip_index[9]]->view_id > 0)
			WBUFW(buf,18)=sd->inventory_data[sd->equip_index[9]]->view_id;
		else
			WBUFW(buf,18)=sd->status.inventory[sd->equip_index[9]].nameid;
	} else
		WBUFW(buf,18) = 0;
	if (sd->equip_index[8] >= 0 && sd->equip_index[8] != sd->equip_index[9] && sd->inventory_data[sd->equip_index[8]] && sd->view_class != 22) {
		if (sd->inventory_data[sd->equip_index[8]]->view_id > 0)
			WBUFW(buf,20) = sd->inventory_data[sd->equip_index[8]]->view_id;
		else
			WBUFW(buf,20)=sd->status.inventory[sd->equip_index[8]].nameid;
	} else
		WBUFW(buf,20) = 0;
	WBUFW(buf,22)=sd->status.head_bottom;
	WBUFW(buf,24)=sd->status.head_top;
	WBUFW(buf,26)=sd->status.head_mid;
	WBUFW(buf,28)=sd->status.hair_color;
	WBUFW(buf,30)=sd->status.clothes_color;
	WBUFW(buf,32)=sd->head_dir;
	WBUFL(buf,34)=sd->status.guild_id;
	WBUFW(buf,38)=sd->guild_emblem_id;
	WBUFW(buf,40)=sd->status.manner;
	WBUFW(buf,42)=sd->opt3;
	WBUFB(buf,44)=sd->status.karma;
	WBUFB(buf,45)=sd->sex;
	WBUFPOS(buf,46,sd->bl.x,sd->bl.y);
	WBUFB(buf,48)|=sd->dir&0x0f;
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFB(buf,51)=sd->state.dead_sit;
	WBUFW(buf,52)=((level = battle_get_lv(&sd->bl)) > battle_config.max_lv) ? battle_config.max_lv : level;

	return packet_db[0x1d8].len;
#endif
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_set007b(struct map_session_data *sd,unsigned char *buf) {
	int level=0;

	nullpo_retr(0, sd);

	if (sd->disguise > 23 && sd->disguise < 4001) { // mob disguises [Valaris]
		WBUFW(buf,0)=0x7b;
		WBUFL(buf,2)=sd->bl.id;
		WBUFW(buf,6)=battle_get_speed(&sd->bl);
		WBUFW(buf,8)=sd->opt1;
		WBUFW(buf,10)=sd->opt2;
		WBUFW(buf,12)=sd->status.option;
		WBUFW(buf,14)=sd->disguise;
		WBUFL(buf,22)=gettick();
		WBUFW(buf,46)=0;
		WBUFB(buf,48)=0;
		WBUFPOS2(buf,50,sd->bl.x,sd->bl.y,sd->to_x,sd->to_y);
		WBUFB(buf,55)=0;
		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		WBUFW(buf,58)=((level = battle_get_lv(&sd->bl))>battle_config.max_lv)? battle_config.max_lv:level;

		return packet_db[0x7b].len;
	}

#if PACKETVER < 4
	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->speed;
	WBUFW(buf,8)=sd->opt1;
	WBUFW(buf,10)=sd->opt2;
	WBUFW(buf,12)=sd->status.option;
	WBUFW(buf,14)=sd->view_class;
	WBUFW(buf,16)=sd->status.hair;
	if(sd->view_class != 22)
		WBUFW(buf,18)=sd->status.weapon;
	else
		WBUFW(buf,18)=0;
	WBUFW(buf,20)=sd->status.head_bottom;
	WBUFL(buf,22)=gettick();
	WBUFW(buf,26)=sd->status.shield;
	WBUFW(buf,28)=sd->status.head_top;
	WBUFW(buf,30)=sd->status.head_mid;
	WBUFW(buf,32)=sd->status.hair_color;
	WBUFW(buf,34)=sd->status.clothes_color;
	WBUFW(buf,36)=sd->head_dir;
	WBUFL(buf,38)=sd->status.guild_id;
	WBUFL(buf,42)=sd->guild_emblem_id;
	WBUFW(buf,46)=sd->status.manner;
	WBUFB(buf,48)=sd->status.karma;
	WBUFB(buf,49)=sd->sex;
	WBUFPOS2(buf,50,sd->bl.x,sd->bl.y,sd->to_x,sd->to_y);
	WBUFB(buf,55)=0;
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd->status.base_level>battle_config.max_lv)?battle_config.max_lv:sd->status.base_level;

	return packet_db[0x7b].len;
#else
	WBUFW(buf,0)=0x1da;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->speed;
	WBUFW(buf,8)=sd->opt1;
	WBUFW(buf,10)=sd->opt2;
	WBUFW(buf,12)=sd->status.option;
	WBUFW(buf,14)=sd->view_class;
	WBUFW(buf,16)=sd->status.hair;
	if(sd->equip_index[9] >= 0 && sd->inventory_data[sd->equip_index[9]] && sd->view_class != 22) {
		if(sd->inventory_data[sd->equip_index[9]]->view_id > 0)
			WBUFW(buf,18)=sd->inventory_data[sd->equip_index[9]]->view_id;
		else
			WBUFW(buf,18)=sd->status.inventory[sd->equip_index[9]].nameid;
	}
	else
		WBUFW(buf,18)=0;
	if(sd->equip_index[8] >= 0 && sd->equip_index[8] != sd->equip_index[9] && sd->inventory_data[sd->equip_index[8]] && sd->view_class != 22) {
		if(sd->inventory_data[sd->equip_index[8]]->view_id > 0)
			WBUFW(buf,20)=sd->inventory_data[sd->equip_index[8]]->view_id;
		else
			WBUFW(buf,20)=sd->status.inventory[sd->equip_index[8]].nameid;
	}
	else
		WBUFW(buf,20)=0;
	WBUFW(buf,22)=sd->status.head_bottom;
	WBUFL(buf,24)=gettick();
	WBUFW(buf,28)=sd->status.head_top;
	WBUFW(buf,30)=sd->status.head_mid;
	WBUFW(buf,32)=sd->status.hair_color;
	WBUFW(buf,34)=sd->status.clothes_color;
	WBUFW(buf,36)=sd->head_dir;
	WBUFL(buf,38)=sd->status.guild_id;
	WBUFW(buf,42)=sd->guild_emblem_id;
	WBUFW(buf,44)=sd->status.manner;
	WBUFW(buf,46)=sd->opt3;
	WBUFB(buf,48)=sd->status.karma;
	WBUFB(buf,49)=sd->sex;
	WBUFPOS2(buf,50,sd->bl.x,sd->bl.y,sd->to_x,sd->to_y);
	WBUFB(buf,55)=0;
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;
	WBUFW(buf,58)=(sd->status.base_level>battle_config.max_lv)?battle_config.max_lv:sd->status.base_level;

	return packet_db[0x1da].len;
#endif
}

/*==========================================
 * クラスチェンジ typeはMobの場合は1で他は0？
 *------------------------------------------
 */
int clif_class_change(struct block_list *bl,int class,int type)
{
	char buf[16];

	nullpo_retr(0, bl);

	if(class >= MAX_PC_CLASS) {
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=bl->id;
		WBUFB(buf,6)=type;
		WBUFL(buf,7)=class;

		clif_send(buf,packet_db[0x1b0].len,bl,AREA);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_mob_class_change(struct mob_data *md, int class) {
	char buf[16];
	int view = mob_get_viewclass(class);

	nullpo_retr(0, md);

	if(view >= MAX_PC_CLASS) {
		WBUFW(buf,0)=0x1b0;
		WBUFL(buf,2)=md->bl.id;
		WBUFB(buf,6)=1;
		WBUFL(buf,7)=view;

		clif_send(buf,packet_db[0x1b0].len,&md->bl,AREA);
	}
	return 0;
}
// mob equipment [Valaris]

int clif_mob_equip(struct mob_data *md, int nameid) {
	unsigned char buf[16];

	nullpo_retr(0, md);

	memset(buf,0,packet_db[0x1a4].len);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=3;
	WBUFL(buf,3)=md->bl.id;
	WBUFL(buf,7)=nameid;

	clif_send(buf,packet_db[0x1a4].len,&md->bl,AREA);

	return 0;
}

/*==========================================
 * MOB表示1
 *------------------------------------------
 */
static int clif_mob0078(struct mob_data *md, unsigned char *buf) 
{
	int level;

	memset(buf,0,packet_db[0x78].len);

	nullpo_retr(0, md);

	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=md->bl.id;
	WBUFW(buf,6)=battle_get_speed(&md->bl);
	WBUFW(buf,8)=md->opt1;
	WBUFW(buf,10)=md->opt2;
	WBUFW(buf,12)=md->option;
	WBUFW(buf,14)=mob_get_viewclass(md->class);
	if((mob_get_viewclass(md->class) <= 23) || (mob_get_viewclass(md->class) == 812) || (mob_get_viewclass(md->class) >= 4001)) {
		WBUFW(buf,12)|=mob_db[md->class].option;
		WBUFW(buf,16)=mob_get_hair(md->class);
		WBUFW(buf,18)=mob_get_weapon(md->class);
		WBUFW(buf,20)=mob_get_head_buttom(md->class);
		WBUFW(buf,22)=mob_get_shield(md->class);
		WBUFW(buf,24)=mob_get_head_top(md->class);
		WBUFW(buf,26)=mob_get_head_mid(md->class);
		WBUFW(buf,28)=mob_get_hair_color(md->class);
		WBUFW(buf,30)=mob_get_clothes_color(md->class);	//Add for player monster dye - Valaris
		WBUFB(buf,45)=mob_get_sex(md->class);
	}

	if (md->class >= 1285 && md->class <= 1287 && md->guild_id) {	// Added guardian emblems [Valaris]
		struct guild *g;
		struct guild_castle *gc=guild_mapname2gc(map[md->bl.m].name);
		if (gc && gc->guild_id > 0) {
			g=guild_search(gc->guild_id);
			if (g) {
				WBUFL(buf,22)=g->emblem_id;
				WBUFL(buf,26)=gc->guild_id;
			}
		}
	}	// End addition

	WBUFPOS(buf,46,md->bl.x,md->bl.y);
	WBUFB(buf,48)|=md->dir&0x0f;
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;
	WBUFW(buf,52)=((level = battle_get_lv(&md->bl))>battle_config.max_lv)? battle_config.max_lv:level;

	return packet_db[0x78].len;
}

/*==========================================
 * MOB表示2
 *------------------------------------------
 */
static int clif_mob007b(struct mob_data *md,unsigned char *buf)
{
	int level;

	memset(buf,0,packet_db[0x7b].len);

	nullpo_retr(0, md);

	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=md->bl.id;
	WBUFW(buf,6)=battle_get_speed(&md->bl);
	WBUFW(buf,8)=md->opt1;
	WBUFW(buf,10)=md->opt2;
	WBUFW(buf,12)=md->option;
	WBUFW(buf,14)=mob_get_viewclass(md->class);
	if ((mob_get_viewclass(md->class) < 24) || (mob_get_viewclass(md->class) > 4000)) {
		WBUFW(buf,12)|=mob_db[md->class].option;
		WBUFW(buf,16)=mob_get_hair(md->class);
		WBUFW(buf,18)=mob_get_weapon(md->class);
		WBUFW(buf,20)=mob_get_head_buttom(md->class);
		WBUFL(buf,22)=gettick();
		WBUFW(buf,26)=mob_get_shield(md->class);
		WBUFW(buf,28)=mob_get_head_top(md->class);
		WBUFW(buf,30)=mob_get_head_mid(md->class);
		WBUFW(buf,32)=mob_get_hair_color(md->class);
		WBUFW(buf,34)=mob_get_clothes_color(md->class);	//Add for player monster dye - Valaris
		WBUFB(buf,49)=mob_get_sex(md->class);
	} else

		WBUFL(buf,22)=gettick();


		if(md->class >= 1285 && md->class <= 1287 && md->guild_id)	{	// Added guardian emblems [Valaris]
			struct guild *g;
			struct guild_castle *gc=guild_mapname2gc(map[md->bl.m].name);
			if(gc && gc->guild_id > 0){
				g=guild_search(gc->guild_id);
					if(g) {
					WBUFL(buf,28)=gc->guild_id;
					WBUFL(buf,24)=g->emblem_id;
					}

			}

		}					// End addition

		WBUFB(buf,56)=5;
		WBUFB(buf,57)=5;
		WBUFW(buf,58)=((level = battle_get_lv(&md->bl))>battle_config.max_lv)? battle_config.max_lv:level;
	return packet_db[0x7b].len;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_npc0078(struct npc_data *nd,unsigned char *buf)
{
	struct guild *g;

	nullpo_retr(0, nd);

	memset(buf,0,packet_db[0x78].len);

	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=nd->bl.id;
	WBUFW(buf,6)=nd->speed;
	WBUFW(buf,12)=nd->option;
	WBUFW(buf,14)=nd->class;
	if( (nd->bl.subtype!=WARP) && 
		(nd->class == 722) && 
		(nd->u.scr.guild_id > 0) && 
		((g=guild_search(nd->u.scr.guild_id))) )
	{
		WBUFL(buf,22)=g->emblem_id;
		WBUFL(buf,26)=g->guild_id;
	}
	WBUFPOS(buf,46,nd->bl.x,nd->bl.y);
	WBUFB(buf,48)|=nd->dir&0x0f;
	WBUFB(buf,49)=5;
	WBUFB(buf,50)=5;

	return packet_db[0x78].len;
}

// NPC Walking [Valaris]
static int clif_npc007b(struct npc_data *nd, unsigned char *buf) {
	struct guild *g;

	nullpo_retr(0, nd);

	memset(buf,0,packet_db[0x7b].len);

	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=nd->bl.id;
	WBUFW(buf,6)=nd->speed;
	WBUFW(buf,12)=nd->option;
	WBUFW(buf,14)=nd->class;
	//if ((nd->class == 722) && (nd->u.scr.guild_id > 0) && ((g=guild_search(nd->u.scr.guild_id)) != NULL))
	if((nd->bl.subtype!=WARP) && (nd->class == 722) && (nd->u.scr.guild_id > 0) && 
((g=guild_search(nd->u.scr.guild_id))))
	{
		WBUFL(buf,22)=g->emblem_id;
		WBUFL(buf,26)=g->guild_id;
	}

	WBUFL(buf,22)=gettick();
	WBUFPOS2(buf,50,nd->bl.x,nd->bl.y,nd->to_x,nd->to_y);
	WBUFB(buf,56)=5;
	WBUFB(buf,57)=5;

	return packet_db[0x7b].len;
}


/*==========================================
 *
 *------------------------------------------
 */
static int clif_pet0078(struct pet_data *pd,unsigned char *buf)
{
	int view,level;

	nullpo_retr(0, pd);

	memset(buf,0,packet_db[0x78].len);

	WBUFW(buf,0)=0x78;
	WBUFL(buf,2)=pd->bl.id;
	WBUFW(buf,6)=pd->speed;
	WBUFW(buf,14)=mob_get_viewclass(pd->class);
	if((mob_get_viewclass(pd->class) < 24) || (mob_get_viewclass(pd->class) > 4000)) {
		WBUFW(buf,12)=mob_db[pd->class].option;
		WBUFW(buf,16)=mob_get_hair(pd->class);
		WBUFW(buf,18)=mob_get_weapon(pd->class);
		WBUFW(buf,20)=mob_get_head_buttom(pd->class);
		WBUFW(buf,22)=mob_get_shield(pd->class);
		WBUFW(buf,24)=mob_get_head_top(pd->class);
		WBUFW(buf,26)=mob_get_head_mid(pd->class);
		WBUFW(buf,28)=mob_get_hair_color(pd->class);
		WBUFW(buf,30)=mob_get_clothes_color(pd->class);	//Add for player pet dye - Valaris
		WBUFB(buf,45)=mob_get_sex(pd->class);
	} else {
		WBUFW(buf,16)=0x14;
		if((view = itemdb_viewid(pd->equip)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd->equip;
	}
	WBUFPOS(buf,46,pd->bl.x,pd->bl.y);
	WBUFB(buf,48)|=pd->dir&0x0f;
	WBUFB(buf,49)=0;
	WBUFB(buf,50)=0;
	WBUFW(buf,52)=((level = battle_get_lv(&pd->bl))>battle_config.max_lv)? battle_config.max_lv:level;

	return packet_db[0x78].len;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_pet007b(struct pet_data *pd,unsigned char *buf)
{
	int view,level;

	nullpo_retr(0, pd);

	memset(buf,0,packet_db[0x7b].len);

	WBUFW(buf,0)=0x7b;
	WBUFL(buf,2)=pd->bl.id;
	WBUFW(buf,6)=pd->speed;
	WBUFW(buf,14)=mob_get_viewclass(pd->class);
	if((mob_get_viewclass(pd->class) < 24) || (mob_get_viewclass(pd->class) > 4000)) {
		WBUFW(buf,12)=mob_db[pd->class].option;
		WBUFW(buf,16)=mob_get_hair(pd->class);
		WBUFW(buf,18)=mob_get_weapon(pd->class);
		WBUFW(buf,20)=mob_get_head_buttom(pd->class);
		WBUFL(buf,22)=gettick();
		WBUFW(buf,26)=mob_get_shield(pd->class);
		WBUFW(buf,28)=mob_get_head_top(pd->class);
		WBUFW(buf,30)=mob_get_head_mid(pd->class);
		WBUFW(buf,32)=mob_get_hair_color(pd->class);
		WBUFW(buf,34)=mob_get_clothes_color(pd->class);	//Add for player pet dye - Valaris
		WBUFB(buf,49)=mob_get_sex(pd->class);
	} else {
		WBUFW(buf,16)=0x14;
		if((view = itemdb_viewid(pd->equip)) > 0)
			WBUFW(buf,20)=view;
		else
			WBUFW(buf,20)=pd->equip;
		WBUFL(buf,22)=gettick();
	}
	WBUFPOS2(buf,50,pd->bl.x,pd->bl.y,pd->to_x,pd->to_y);
	WBUFB(buf,56)=0;
	WBUFB(buf,57)=0;
	WBUFW(buf,58)=((level = battle_get_lv(&pd->bl))>battle_config.max_lv)? battle_config.max_lv:level;

	return packet_db[0x7b].len;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_set01e1(struct map_session_data *sd,unsigned char *buf)
{
	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x1e1;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->spiritball;

	return packet_db[0x1e1].len;
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_set0192(int fd,int m,int x,int y,int type)
{
	WFIFOW(fd,0) = 0x192;
	WFIFOW(fd,2) = x;
	WFIFOW(fd,4) = y;
	WFIFOW(fd,6) = type;
	memcpy(WFIFOP(fd,8),map[m].name,16);
	WFIFOSET(fd,packet_db[0x192].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnpc(struct map_session_data *sd) {
	unsigned char buf[128];

	nullpo_retr(0, sd);

	if (sd->disguise > 23 && sd->disguise < 4001) { // mob disguises [Valaris]
		clif_clearchar(&sd->bl, 9);

		memset(buf, 0, packet_db[0x119].len);

		WBUFW(buf, 0) = 0x119;
		WBUFL(buf, 2) = sd->bl.id;
		WBUFW(buf, 6) = 0;
		WBUFW(buf, 8) = 0;
		WBUFW(buf,10) = 0x40;
		WBUFB(buf,12) = 0;

		clif_send(buf, packet_db[0x119].len, &sd->bl, SELF);

		memset(buf, 0, packet_db[0x7c].len);

		WBUFW(buf, 0) = 0x7c;
		WBUFL(buf, 2) = sd->bl.id;
		WBUFW(buf, 6) = sd->speed;
		WBUFW(buf, 8) = sd->opt1;
		WBUFW(buf,10) = sd->opt2;
		WBUFW(buf,12) = sd->status.option;
		WBUFW(buf,20) = sd->disguise;
		WBUFPOS(buf, 36, sd->bl.x, sd->bl.y);
		clif_send(buf, packet_db[0x7c].len, &sd->bl, AREA);
	}

	clif_set0078(sd, buf);

#if PACKETVER < 4
	WBUFW(buf, 0) = 0x79;
	WBUFW(buf,51) = (sd->status.base_level > battle_config.max_lv) ? battle_config.max_lv : sd->status.base_level;
	clif_send(buf, packet_db[0x79].len, &sd->bl, AREA_WOS);
#else
	WBUFW(buf, 0) = 0x1d9;
	WBUFW(buf,51) = (sd->status.base_level > battle_config.max_lv) ? battle_config.max_lv : sd->status.base_level;
	clif_send(buf, packet_db[0x1d9].len, &sd->bl, AREA_WOS);
#endif


	if (sd->spiritball > 0)
		clif_spiritball(sd);

	if (sd->status.guild_id > 0) { // force display of guild emblem [Valaris]
		struct guild *g = guild_search(sd->status.guild_id);
		if (g)
			clif_guild_emblem(sd,g);
	}	// end addition [Valaris]

	if (sd->status.class==13 || sd->status.class==21 || sd->status.class==4014 || sd->status.class==4022)
		pc_setoption(sd,sd->status.option|0x0020); // [Valaris]

	if ((pc_isriding(sd) && pc_checkskill(sd,KN_RIDING)>0) && (sd->status.class==7 ||
	    sd->status.class==14 || sd->status.class==4008 || sd->status.class==4015))
		pc_setriding(sd); // update peco riders for people upgrading athena [Valaris]


	if (map[sd->bl.m].flag.snow)
		clif_specialeffect(&sd->bl, 162, 1);
	if (map[sd->bl.m].flag.fog)
		clif_specialeffect(&sd->bl, 233, 1);
	if (map[sd->bl.m].flag.sakura)
		clif_specialeffect(&sd->bl, 163, 1);
	if (map[sd->bl.m].flag.leaves)
		clif_specialeffect(&sd->bl, 333, 1);
	if (map[sd->bl.m].flag.rain)
		clif_specialeffect(&sd->bl, 161, 1);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnnpc(struct npc_data *nd)
{
	unsigned char buf[64];
	int len;

	nullpo_retr(0, nd);

	if(nd->class < 0 || (nd->flag&1 && nd->option != 0x0002) || nd->class == INVISIBLE_CLASS)
		return 0;

	memset(buf,0,packet_db[0x7c].len);

	WBUFW(buf,0)=0x7c;
	WBUFL(buf,2)=nd->bl.id;
	WBUFW(buf,6)=nd->speed;
	WBUFW(buf,12)=nd->option;
	WBUFW(buf,20)=nd->class;
	WBUFPOS(buf,36,nd->bl.x,nd->bl.y);

	clif_send(buf,packet_db[0x7c].len,&nd->bl,AREA);

	len = clif_npc0078(nd,buf);
	clif_send(buf,len,&nd->bl,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnmob(struct mob_data *md)
{
	unsigned char buf[64];
	int len;

	nullpo_retr(0, md);

	if (mob_get_viewclass(md->class) > 23 ) {
		memset(buf,0,packet_db[0x7c].len);

		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=md->bl.id;
		WBUFW(buf,6)=md->speed;
		WBUFW(buf,8)=md->opt1;
		WBUFW(buf,10)=md->opt2;
		WBUFW(buf,12)=md->option;
		WBUFW(buf,20)=mob_get_viewclass(md->class);
		WBUFPOS(buf,36,md->bl.x,md->bl.y);
		clif_send(buf,packet_db[0x7c].len,&md->bl,AREA);
	}

	len = clif_mob0078(md,buf);
	clif_send(buf,len,&md->bl,AREA);

	if (mob_get_equip(md->class) > 0) // mob equipment [Valaris]
		clif_mob_equip(md,mob_get_equip(md->class));

	return 0;
}

// pet

/*==========================================
 *
 *------------------------------------------
 */
int clif_spawnpet(struct pet_data *pd)
{
	unsigned char buf[64];
	int len;

	nullpo_retr(0, pd);

	if(mob_get_viewclass(pd->class) >= MAX_PC_CLASS) {
		memset(buf,0,packet_db[0x7c].len);

		WBUFW(buf,0)=0x7c;
		WBUFL(buf,2)=pd->bl.id;
		WBUFW(buf,6)=pd->speed;
		WBUFW(buf,20)=mob_get_viewclass(pd->class);
		WBUFPOS(buf,36,pd->bl.x,pd->bl.y);

		clif_send(buf,packet_db[0x7c].len,&pd->bl,AREA);
	}

	len = clif_pet0078(pd,buf);
	clif_send(buf,len,&pd->bl,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_movepet(struct pet_data *pd)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, pd);

	len = clif_pet007b(pd,buf);
	clif_send(buf,len,&pd->bl,AREA);

	return 0;
}

/*==========================================
 * npc walking [Valaris]
 *------------------------------------------
 */
int clif_movenpc(struct npc_data *nd) {
	unsigned char buf[256];
	int len;

	nullpo_retr(0, nd);

	len = clif_npc007b(nd,buf);
	clif_send(buf,len,&nd->bl,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_servertick(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x7f;
	WFIFOL(fd,2)=sd->server_tick;
	WFIFOSET(fd,packet_db[0x7f].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_walkok(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x87;
	WFIFOL(fd,2)=gettick();;
	WFIFOPOS2(fd,6,sd->bl.x,sd->bl.y,sd->to_x,sd->to_y);
	WFIFOB(fd,11)=0;
	WFIFOSET(fd,packet_db[0x87].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_movechar(struct map_session_data *sd)
{
	int fd;
	int len;
	unsigned char buf[256];

	nullpo_retr(0, sd);

	fd = sd->fd;

	len = clif_set007b(sd, buf);

	if (sd->disguise > 23 && sd->disguise < 4001) {
		clif_send(buf, len, &sd->bl, AREA);
		return 0;
	} else
		clif_send(buf, len, &sd->bl, AREA_WOS);

	if (battle_config.save_clothcolor == 1 && sd->status.clothes_color > 0)
		clif_changelook(&sd->bl, LOOK_CLOTHES_COLOR, sd->status.clothes_color);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_quitsave(int fd,struct map_session_data *sd)
{
	map_quit(sd);
	chrif_chardisconnect(sd);
}

/*==========================================
 *
 *------------------------------------------
 */
static int clif_waitclose(int tid, unsigned int tick, int id, int data) {
	if (session[id])
		session[id]->eof = 1;
	close(id);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_setwaitclose(int fd) {
	add_timer(gettick() + 5000, clif_waitclose, fd, 0);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemap(struct map_session_data *sd,char *mapname,int x,int y)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0x91;
	memcpy(WFIFOP(fd,2),mapname,16);
	WFIFOW(fd,18)=x;
	WFIFOW(fd,20)=y;
	WFIFOSET(fd,packet_db[0x91].len);

	if (sd->disguise > 23 && sd->disguise < 4001) // mob disguises [Valaris]
		clif_spawnpc(sd);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapserver(struct map_session_data *sd,char *mapname,int x,int y,int ip,int port)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x92;
	memcpy(WFIFOP(fd,2),mapname,16);
	WFIFOW(fd,18)=x;
	WFIFOW(fd,20)=y;
	WFIFOL(fd,22)=ip;
	WFIFOW(fd,26)=port;
	WFIFOSET(fd,packet_db[0x92].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_fixpos(struct block_list *bl)
{
	char buf[16];

	nullpo_retr(0, bl);

	WBUFW(buf,0)=0x88;
	WBUFL(buf,2)=bl->id;
	WBUFW(buf,6)=bl->x;
	WBUFW(buf,8)=bl->y;

	clif_send(buf,packet_db[0x88].len,bl,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_npcbuysell(struct map_session_data* sd,int id)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xc4;
	WFIFOL(fd,2)=id;
	WFIFOSET(fd,packet_db[0xc4].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_buylist(struct map_session_data *sd,struct npc_data *nd)
{
	struct item_data *id;
	int fd,i,val;

	nullpo_retr(0, sd);
	nullpo_retr(0, nd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xc6;
	for(i=0;nd->u.shop_item[i].nameid > 0;i++){
		id = itemdb_search(nd->u.shop_item[i].nameid);
		val=nd->u.shop_item[i].value;
		WFIFOL(fd,4+i*11)=val;
		if ( ! id->flag.value_notdc)
			val=pc_modifybuyvalue(sd,val);
		WFIFOL(fd,8+i*11)=val;
		WFIFOB(fd,12+i*11)=id->type;
		if(id->view_id > 0)
			WFIFOW(fd,13+i*11)=id->view_id;
		else
			WFIFOW(fd,13+i*11)=nd->u.shop_item[i].nameid;
	}
	WFIFOW(fd,2)=i*11+4;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_selllist(struct map_session_data *sd)
{
	int fd,i,c=0,val;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xc7;
	for(i=0;i<MAX_INVENTORY;i++) {
		if(sd->status.inventory[i].nameid > 0 && sd->inventory_data[i]) {
			val=sd->inventory_data[i]->value_sell;
			if(val < 0)
				continue;
			WFIFOW(fd,4+c*10)=i+2;
			WFIFOL(fd,6+c*10)=val;
			if ( !sd->inventory_data[i]->flag.value_notoc)
				val=pc_modifysellvalue(sd,val);
			WFIFOL(fd,10+c*10)=val;
			c++;
		}
	}
	WFIFOW(fd,2)=c*10+4;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptmes(struct map_session_data *sd,int npcid,char *mes)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xb4;
	WFIFOW(fd,2)=strlen(mes)+9;
	WFIFOL(fd,4)=npcid;
	strncpy(WFIFOP(fd,8),mes,strlen(mes)+1);
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptnext(struct map_session_data *sd,int npcid)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xb5;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_db[0xb5].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptclose(struct map_session_data *sd,int npcid)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xb6;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_db[0xb6].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptmenu(struct map_session_data *sd,int npcid,char *mes)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xb7;
	WFIFOW(fd,2)=strlen(mes)+9;
	WFIFOL(fd,4)=npcid;
	strncpy(WFIFOP(fd,8),mes,strlen(mes)+1);
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptinput(struct map_session_data *sd,int npcid)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x142;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_db[0x142].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_scriptinputstr(struct map_session_data *sd,int npcid)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1d4;
	WFIFOL(fd,2)=npcid;
	WFIFOSET(fd,packet_db[0x1d4].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_viewpoint(struct map_session_data *sd,int npc_id,int type,int x,int y,int id,int color)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x144;
	WFIFOL(fd,2)=npc_id;
	WFIFOL(fd,6)=type;
	WFIFOL(fd,10)=x;
	WFIFOL(fd,14)=y;
	WFIFOB(fd,18)=id;
	WFIFOL(fd,19)=color;
	WFIFOSET(fd,packet_db[0x144].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_cutin(struct map_session_data *sd,char *image,int type)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1b3;
	memcpy(WFIFOP(fd,2),image,64);
	WFIFOB(fd,66)=type;
	WFIFOSET(fd,packet_db[0x1b3].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_additem(struct map_session_data *sd,int n,int amount,int fail)
{
	int fd,j;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf=WFIFOP(fd,0);
	if(fail) {
		WBUFW(buf,0)=0xa0;
		WBUFW(buf,2)=n+2;
		WBUFW(buf,4)=amount;
		WBUFW(buf,6)=0;
		WBUFB(buf,8)=0;
		WBUFB(buf,9)=0;
		WBUFB(buf,10)=0;
		WBUFW(buf,11)=0;
		WBUFW(buf,13)=0;
		WBUFW(buf,15)=0;
		WBUFW(buf,17)=0;
		WBUFW(buf,19)=0;
		WBUFB(buf,21)=0;
		WBUFB(buf,22)=fail;
	} else {
		if (n<0 || n>=MAX_INVENTORY || sd->status.inventory[n].nameid <=0 || sd->inventory_data[n] == NULL)
			return 1;

		WBUFW(buf,0)=0xa0;
		WBUFW(buf,2)=n+2;
		WBUFW(buf,4)=amount;
		if(sd->inventory_data[n]->view_id > 0)
			WBUFW(buf,6)=sd->inventory_data[n]->view_id;
		else
			WBUFW(buf,6)=sd->status.inventory[n].nameid;
		WBUFB(buf,8)=sd->status.inventory[n].identify;
		WBUFB(buf,9)=sd->status.inventory[n].attribute;
		WBUFB(buf,10)=sd->status.inventory[n].refine;
		if(sd->status.inventory[n].card[0]==0x00ff || sd->status.inventory[n].card[0]==0x00fe || sd->status.inventory[n].card[0]==(short)0xff00) {
			WBUFW(buf,11)=sd->status.inventory[n].card[0];
			WBUFW(buf,13)=sd->status.inventory[n].card[1];
			WBUFW(buf,15)=sd->status.inventory[n].card[2];
			WBUFW(buf,17)=sd->status.inventory[n].card[3];
		} else {
			if (sd->status.inventory[n].card[0] > 0 && (j=itemdb_viewid(sd->status.inventory[n].card[0])) > 0)
				WBUFW(buf,11)=j;
			else
				WBUFW(buf,11)=sd->status.inventory[n].card[0];
			if(sd->status.inventory[n].card[1] > 0 && (j=itemdb_viewid(sd->status.inventory[n].card[1])) > 0)
				WBUFW(buf,13)=j;
			else
				WBUFW(buf,13)=sd->status.inventory[n].card[1];
			if(sd->status.inventory[n].card[2] > 0 && (j=itemdb_viewid(sd->status.inventory[n].card[2])) > 0)
				WBUFW(buf,15)=j;
			else
				WBUFW(buf,15)=sd->status.inventory[n].card[2];
			if(sd->status.inventory[n].card[3] > 0 && (j=itemdb_viewid(sd->status.inventory[n].card[3])) > 0)
				WBUFW(buf,17)=j;
			else
				WBUFW(buf,17)=sd->status.inventory[n].card[3];
		}
		WBUFW(buf,19)=pc_equippoint(sd,n);
		WBUFB(buf,21)=(sd->inventory_data[n]->type == 7)? 4:sd->inventory_data[n]->type;
		WBUFB(buf,22)=fail;
	}

	WFIFOSET(fd,packet_db[0xa0].len);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_delitem(struct map_session_data *sd,int n,int amount)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xaf;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=amount;

	WFIFOSET(fd,packet_db[0xaf].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_itemlist(struct map_session_data *sd)
{
	int i,n,fd,arrow=-1;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0xa3;
	for(i=0,n=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid <=0 || sd->inventory_data[i] == NULL || itemdb_isequip2(sd->inventory_data[i]))
			continue;
		WBUFW(buf,n*10+4)=i+2;
		if(sd->inventory_data[i]->view_id > 0)
			WBUFW(buf,n*10+6)=sd->inventory_data[i]->view_id;
		else
			WBUFW(buf,n*10+6)=sd->status.inventory[i].nameid;
		WBUFB(buf,n*10+8)=sd->inventory_data[i]->type;
		WBUFB(buf,n*10+9)=sd->status.inventory[i].identify;
		WBUFW(buf,n*10+10)=sd->status.inventory[i].amount;
		if(sd->inventory_data[i]->equip == 0x8000){
			WBUFW(buf,n*10+12)=0x8000;
			if (sd->status.inventory[i].equip)
				arrow=i;	// ついでに矢装備チェック
		} else
			WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1ee;
	for(i=0,n=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid <=0 || sd->inventory_data[i] == NULL || itemdb_isequip2(sd->inventory_data[i]))
			continue;
		WBUFW(buf,n*18+4)=i+2;
		if(sd->inventory_data[i]->view_id > 0)
			WBUFW(buf,n*18+6)=sd->inventory_data[i]->view_id;
		else
			WBUFW(buf,n*18+6)=sd->status.inventory[i].nameid;
		WBUFB(buf,n*18+8)=sd->inventory_data[i]->type;
		WBUFB(buf,n*18+9)=sd->status.inventory[i].identify;
		WBUFW(buf,n*18+10)=sd->status.inventory[i].amount;
		if(sd->inventory_data[i]->equip == 0x8000){
			WBUFW(buf,n*18+12)=0x8000;
			if(sd->status.inventory[i].equip)
				arrow=i;	// ついでに矢装備チェック
		} else
			WBUFW(buf,n*18+12)=0;
		WBUFW(buf,n*18+14)=sd->status.inventory[i].card[0];
		WBUFW(buf,n*18+16)=sd->status.inventory[i].card[1];
		WBUFW(buf,n*18+18)=sd->status.inventory[i].card[2];
		WBUFW(buf,n*18+20)=sd->status.inventory[i].card[3];
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	if(arrow >= 0)
		clif_arrowequip(sd,arrow);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_equiplist(struct map_session_data *sd)
{
	int i,j,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0xa4;
	for(i=0,n=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid<=0 || sd->inventory_data[i] == NULL || !itemdb_isequip2(sd->inventory_data[i]))
			continue;
		WBUFW(buf,n*20+4)=i+2;
		if(sd->inventory_data[i]->view_id > 0)
			WBUFW(buf,n*20+6)=sd->inventory_data[i]->view_id;
		else
			WBUFW(buf,n*20+6)=sd->status.inventory[i].nameid;
		WBUFB(buf,n*20+8)=(sd->inventory_data[i]->type == 7)? 4:sd->inventory_data[i]->type;
		WBUFB(buf,n*20+9)=sd->status.inventory[i].identify;
		WBUFW(buf,n*20+10)=pc_equippoint(sd,i);
		WBUFW(buf,n*20+12)=sd->status.inventory[i].equip;
		WBUFB(buf,n*20+14)=sd->status.inventory[i].attribute;
		WBUFB(buf,n*20+15)=sd->status.inventory[i].refine;
		if(sd->status.inventory[i].card[0]==0x00ff || sd->status.inventory[i].card[0]==0x00fe || sd->status.inventory[i].card[0]==(short)0xff00) {
			WBUFW(buf,n*20+16)=sd->status.inventory[i].card[0];
			WBUFW(buf,n*20+18)=sd->status.inventory[i].card[1];
			WBUFW(buf,n*20+20)=sd->status.inventory[i].card[2];
			WBUFW(buf,n*20+22)=sd->status.inventory[i].card[3];
		} else {
			if(sd->status.inventory[i].card[0] > 0 && (j=itemdb_viewid(sd->status.inventory[i].card[0])) > 0)
				WBUFW(buf,n*20+16)=j;
			else
				WBUFW(buf,n*20+16)=sd->status.inventory[i].card[0];
			if(sd->status.inventory[i].card[1] > 0 && (j=itemdb_viewid(sd->status.inventory[i].card[1])) > 0)
				WBUFW(buf,n*20+18)=j;
			else
				WBUFW(buf,n*20+18)=sd->status.inventory[i].card[1];
			if(sd->status.inventory[i].card[2] > 0 && (j=itemdb_viewid(sd->status.inventory[i].card[2])) > 0)
				WBUFW(buf,n*20+20)=j;
			else
				WBUFW(buf,n*20+20)=sd->status.inventory[i].card[2];
			if(sd->status.inventory[i].card[3] > 0 && (j=itemdb_viewid(sd->status.inventory[i].card[3])) > 0)
				WBUFW(buf,n*20+22)=j;
			else
				WBUFW(buf,n*20+22)=sd->status.inventory[i].card[3];
		}
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * カプラさんに預けてある消耗品&収集品リスト
 *------------------------------------------
 */
int clif_storageitemlist(struct map_session_data *sd,struct storage *stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0xa5;
	for(i=0,n=0;i<MAX_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(itemdb_isequip2(id))
			continue;

		WBUFW(buf,n*10+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=stor->storage[i].nameid;
		WBUFB(buf,n*10+8)=id->type;;
		WBUFB(buf,n*10+9)=stor->storage[i].identify;
		WBUFW(buf,n*10+10)=stor->storage[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1f0;
	for(i=0,n=0;i<MAX_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(itemdb_isequip2(id))
			continue;

		WBUFW(buf,n*18+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=stor->storage[i].nameid;
		WBUFB(buf,n*18+8)=id->type;;
		WBUFB(buf,n*18+9)=stor->storage[i].identify;
		WBUFW(buf,n*18+10)=stor->storage[i].amount;
		WBUFW(buf,n*18+12)=0;
		WBUFW(buf,n*18+14)=stor->storage[i].card[0];
		WBUFW(buf,n*18+16)=stor->storage[i].card[1];
		WBUFW(buf,n*18+18)=stor->storage[i].card[2];
		WBUFW(buf,n*18+20)=stor->storage[i].card[3];
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 * カプラさんに預けてある装備リスト
 *------------------------------------------
 */
int clif_storageequiplist(struct map_session_data *sd,struct storage *stor)
{
	struct item_data *id;
	int i,j,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0xa6;
	for(i=0,n=0;i<MAX_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(!itemdb_isequip2(id))
			continue;
		WBUFW(buf,n*20+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=stor->storage[i].nameid;
		WBUFB(buf,n*20+8)=id->type;
		WBUFB(buf,n*20+9)=stor->storage[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=stor->storage[i].equip;
		WBUFB(buf,n*20+14)=stor->storage[i].attribute;
		WBUFB(buf,n*20+15)=stor->storage[i].refine;
		if(stor->storage[i].card[0]==0x00ff || stor->storage[i].card[0]==0x00fe || stor->storage[i].card[0]==(short)0xff00) {
			WBUFW(buf,n*20+16)=stor->storage[i].card[0];
			WBUFW(buf,n*20+18)=stor->storage[i].card[1];
			WBUFW(buf,n*20+20)=stor->storage[i].card[2];
			WBUFW(buf,n*20+22)=stor->storage[i].card[3];
		} else {
			if(stor->storage[i].card[0] > 0 && (j=itemdb_viewid(stor->storage[i].card[0])) > 0)
				WBUFW(buf,n*20+16)=j;
			else
				WBUFW(buf,n*20+16)=stor->storage[i].card[0];
			if(stor->storage[i].card[1] > 0 && (j=itemdb_viewid(stor->storage[i].card[1])) > 0)
				WBUFW(buf,n*20+18)=j;
			else
				WBUFW(buf,n*20+18)=stor->storage[i].card[1];
			if(stor->storage[i].card[2] > 0 && (j=itemdb_viewid(stor->storage[i].card[2])) > 0)
				WBUFW(buf,n*20+20)=j;
			else
				WBUFW(buf,n*20+20)=stor->storage[i].card[2];
			if(stor->storage[i].card[3] > 0 && (j=itemdb_viewid(stor->storage[i].card[3])) > 0)
				WBUFW(buf,n*20+22)=j;
			else
				WBUFW(buf,n*20+22)=stor->storage[i].card[3];
		}
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageitemlist(struct map_session_data *sd,struct guild_storage *stor)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	buf=WFIFOP(fd,0);

#if PACKETVER < 5
	WBUFW(buf,0)=0xa5;
	for(i=0,n=0;i<MAX_GUILD_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(itemdb_isequip2(id))
			continue;

		WBUFW(buf,n*10+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=stor->storage[i].nameid;
		WBUFB(buf,n*10+8)=id->type;;
		WBUFB(buf,n*10+9)=stor->storage[i].identify;
		WBUFW(buf,n*10+10)=stor->storage[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1f0;
	for(i=0,n=0;i<MAX_GUILD_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(itemdb_isequip2(id))
			continue;

		WBUFW(buf,n*18+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=stor->storage[i].nameid;
		WBUFB(buf,n*18+8)=id->type;;
		WBUFB(buf,n*18+9)=stor->storage[i].identify;
		WBUFW(buf,n*18+10)=stor->storage[i].amount;
		WBUFW(buf,n*18+12)=0;
		WBUFW(buf,n*18+14)=stor->storage[i].card[0];
		WBUFW(buf,n*18+16)=stor->storage[i].card[1];
		WBUFW(buf,n*18+18)=stor->storage[i].card[2];
		WBUFW(buf,n*18+20)=stor->storage[i].card[3];
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageequiplist(struct map_session_data *sd,struct guild_storage *stor)
{
	struct item_data *id;
	int i,j,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf=WFIFOP(fd,0);

	WBUFW(buf,0)=0xa6;
	for(i=0,n=0;i<MAX_GUILD_STORAGE;i++){
		if(stor->storage[i].nameid<=0)
			continue;
		nullpo_retr(0, id = itemdb_search(stor->storage[i].nameid));
		if(!itemdb_isequip2(id))
			continue;
		WBUFW(buf,n*20+4)=i+1;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=stor->storage[i].nameid;
		WBUFB(buf,n*20+8)=id->type;
		WBUFB(buf,n*20+9)=stor->storage[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=stor->storage[i].equip;
		WBUFB(buf,n*20+14)=stor->storage[i].attribute;
		WBUFB(buf,n*20+15)=stor->storage[i].refine;
		if(stor->storage[i].card[0]==0x00ff || stor->storage[i].card[0]==0x00fe || stor->storage[i].card[0]==(short)0xff00) {
			WBUFW(buf,n*20+16)=stor->storage[i].card[0];
			WBUFW(buf,n*20+18)=stor->storage[i].card[1];
			WBUFW(buf,n*20+20)=stor->storage[i].card[2];
			WBUFW(buf,n*20+22)=stor->storage[i].card[3];
		} else {
			if(stor->storage[i].card[0] > 0 && (j=itemdb_viewid(stor->storage[i].card[0])) > 0)
				WBUFW(buf,n*20+16)=j;
			else
				WBUFW(buf,n*20+16)=stor->storage[i].card[0];
			if(stor->storage[i].card[1] > 0 && (j=itemdb_viewid(stor->storage[i].card[1])) > 0)
				WBUFW(buf,n*20+18)=j;
			else
				WBUFW(buf,n*20+18)=stor->storage[i].card[1];
			if(stor->storage[i].card[2] > 0 && (j=itemdb_viewid(stor->storage[i].card[2])) > 0)
				WBUFW(buf,n*20+20)=j;
			else
				WBUFW(buf,n*20+20)=stor->storage[i].card[2];
			if(stor->storage[i].card[3] > 0 && (j=itemdb_viewid(stor->storage[i].card[3])) > 0)
				WBUFW(buf,n*20+22)=j;
			else
				WBUFW(buf,n*20+22)=stor->storage[i].card[3];
		}
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * ステータスを送りつける
 * 表示専用数字はこの中で計算して送る
 *------------------------------------------
 */
int clif_updatestatus(struct map_session_data *sd,int type)
{
	int fd,len=8;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0xb0;
	WFIFOW(fd,2)=type;
	switch(type){
		// 00b0
	case SP_WEIGHT:
		pc_checkweighticon(sd);
		WFIFOW(fd,0)=0xb0;
		WFIFOW(fd,2)=type;
		WFIFOL(fd,4)=sd->weight;
		break;
	case SP_MAXWEIGHT:
		WFIFOL(fd,4)=sd->max_weight;
		break;
	case SP_SPEED:
		WFIFOL(fd,4)=sd->speed;
		break;
	case SP_BASELEVEL:
		WFIFOL(fd,4)=sd->status.base_level;
		break;
	case SP_JOBLEVEL:
		WFIFOL(fd,4)=sd->status.job_level;
		break;
	case SP_MANNER:
		WFIFOL(fd,4)=sd->status.manner;
		clif_changestatus(&sd->bl,SP_MANNER,sd->status.manner);
		break;
	case SP_STATUSPOINT:
		WFIFOL(fd,4)=sd->status.status_point;
		break;
	case SP_SKILLPOINT:
		WFIFOL(fd,4)=sd->status.skill_point;
		break;
	case SP_HIT:
		WFIFOL(fd,4)=sd->hit;
		break;
	case SP_FLEE1:
		WFIFOL(fd,4)=sd->flee;
		break;
	case SP_FLEE2:
		WFIFOL(fd,4)=sd->flee2/10;
		break;
	case SP_MAXHP:
		WFIFOL(fd,4)=sd->status.max_hp;
		break;
	case SP_MAXSP:
		WFIFOL(fd,4)=sd->status.max_sp;
		break;
	case SP_HP:
		WFIFOL(fd,4)=sd->status.hp;
		break;
	case SP_SP:
		WFIFOL(fd,4)=sd->status.sp;
		break;
	case SP_ASPD:
		WFIFOL(fd,4)=sd->aspd;
		break;
	case SP_ATK1:
		WFIFOL(fd,4)=sd->base_atk+sd->watk;
		break;
	case SP_DEF1:
		WFIFOL(fd,4)=sd->def;
		break;
	case SP_MDEF1:
		WFIFOL(fd,4)=sd->mdef;
		break;
	case SP_ATK2:
		WFIFOL(fd,4)=sd->watk2;
		break;
	case SP_DEF2:
		WFIFOL(fd,4)=sd->def2;
		break;
	case SP_MDEF2:
		WFIFOL(fd,4)=sd->mdef2;
		break;
	case SP_CRITICAL:
		WFIFOL(fd,4)=sd->critical/10;
		break;
	case SP_MATK1:
		WFIFOL(fd,4)=sd->matk1;
		break;
	case SP_MATK2:
		WFIFOL(fd,4)=sd->matk2;
		break;

		// 00b1 終了
	case SP_ZENY:
		WFIFOW(fd,0)=0xb1;
		if(sd->status.zeny < 0)
			sd->status.zeny = 0;
		WFIFOL(fd,4)=sd->status.zeny;
		break;
	case SP_BASEEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=sd->status.base_exp;
		break;
	case SP_JOBEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=sd->status.job_exp;
		break;
	case SP_NEXTBASEEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=pc_nextbaseexp(sd);
		break;
	case SP_NEXTJOBEXP:
		WFIFOW(fd,0)=0xb1;
		WFIFOL(fd,4)=pc_nextjobexp(sd);
		break;

		// 00be 終了
	case SP_USTR:
	case SP_UAGI:
	case SP_UVIT:
	case SP_UINT:
	case SP_UDEX:
	case SP_ULUK:
		WFIFOW(fd,0)=0xbe;
		WFIFOB(fd,4)=pc_need_status_point(sd,type-SP_USTR+SP_STR);
		len=5;
		break;

		// 013a 終了
	case SP_ATTACKRANGE:
		WFIFOW(fd,0)=0x13a;
		WFIFOW(fd,2)=sd->attackrange;
		len=4;
		break;

		// 0141 終了
	case SP_STR:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.str;
		WFIFOL(fd,10)=sd->paramb[0] + sd->parame[0];
		len=14;
		break;
	case SP_AGI:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.agi;
		WFIFOL(fd,10)=sd->paramb[1] + sd->parame[1];
		len=14;
		break;
	case SP_VIT:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.vit;
		WFIFOL(fd,10)=sd->paramb[2] + sd->parame[2];
		len=14;
		break;
	case SP_INT:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.int_;
		WFIFOL(fd,10)=sd->paramb[3] + sd->parame[3];
		len=14;
		break;
	case SP_DEX:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.dex;
		WFIFOL(fd,10)=sd->paramb[4] + sd->parame[4];
		len=14;
		break;
	case SP_LUK:
		WFIFOW(fd,0)=0x141;
		WFIFOL(fd,2)=type;
		WFIFOL(fd,6)=sd->status.luk;
		WFIFOL(fd,10)=sd->paramb[5] + sd->parame[5];
		len=14;
		break;

	case SP_CARTINFO:
		WFIFOW(fd,0)=0x121;
		WFIFOW(fd,2)=sd->cart_num;
		WFIFOW(fd,4)=sd->cart_max_num;
		WFIFOL(fd,6)=sd->cart_weight;
		WFIFOL(fd,10)=sd->cart_max_weight;
		len=14;
		break;

	default:
		if(battle_config.error_log)
			printf("clif_updatestatus : make %d routine\n",type);
		return 1;
	}
	WFIFOSET(fd,len);

	return 0;
}
int clif_changestatus(struct block_list *bl,int type,int val)
{
	unsigned char buf[12];
	struct map_session_data *sd = NULL;

	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		sd = (struct map_session_data *)bl;

//printf("clif_changestatus id:%d type:%d val:%d\n",bl->id,type,val);
	if(sd){
		WBUFW(buf,0)=0x1ab;
		WBUFL(buf,2)=bl->id;
		WBUFW(buf,6)=type;
		switch(type){
		case SP_MANNER:
			WBUFL(buf,8)=val;
			break;
		default:
			if(battle_config.error_log)
				printf("clif_changestatus : make %d routine\n",type);
			return 1;
		}
		clif_send(buf,packet_db[0x1ab].len,bl,AREA_WOS);
	}
	return 0;
}
/*==========================================
 *
 *------------------------------------------
 */
int clif_changelook(struct block_list *bl,int type,int val)
{

	unsigned char buf[32];
	struct map_session_data *sd = NULL;

	nullpo_retr(0, bl);

	if(bl->type == BL_PC)
		sd = (struct map_session_data *)bl;

	if(sd && sd->disguise > 23 && sd->disguise < 4001) // mob disguises [Valaris]
		return 0;

#if PACKETVER < 4
	if(sd && (type == LOOK_WEAPON || type == LOOK_SHIELD) && sd->view_class == 22)
		val =0;
	WBUFW(buf,0)=0xc3;
	WBUFL(buf,2)=bl->id;
	WBUFB(buf,6)=type;
	WBUFB(buf,7)=val;
	clif_send(buf,packet_db[0xc3].len,bl,AREA);
#else
	if(sd && (type == LOOK_WEAPON || type == LOOK_SHIELD || type == LOOK_SHOES)) {
		WBUFW(buf,0)=0x1d7;
		WBUFL(buf,2)=bl->id;
		if(type == LOOK_SHOES) {
			WBUFB(buf,6)=9;
			if(sd->equip_index[2] >= 0 && sd->inventory_data[sd->equip_index[2]]) {
				if(sd->inventory_data[sd->equip_index[2]]->view_id > 0)
					WBUFW(buf,7)=sd->inventory_data[sd->equip_index[2]]->view_id;
				else
					WBUFW(buf,7)=sd->status.inventory[sd->equip_index[2]].nameid;
			} else
				WBUFW(buf,7)=0;
			WBUFW(buf,9)=0;
		}
		else {
			WBUFB(buf,6)=2;
			if(sd->equip_index[9] >= 0 && sd->inventory_data[sd->equip_index[9]] && sd->view_class != 22) {
				if(sd->inventory_data[sd->equip_index[9]]->view_id > 0)
					WBUFW(buf,7)=sd->inventory_data[sd->equip_index[9]]->view_id;
				else
					WBUFW(buf,7)=sd->status.inventory[sd->equip_index[9]].nameid;
			} else
				WBUFW(buf,7)=0;
			if(sd->equip_index[8] >= 0 && sd->equip_index[8] != sd->equip_index[9] && sd->inventory_data[sd->equip_index[8]] &&
				sd->view_class != 22) {
				if(sd->inventory_data[sd->equip_index[8]]->view_id > 0)
					WBUFW(buf,9)=sd->inventory_data[sd->equip_index[8]]->view_id;
				else
					WBUFW(buf,9)=sd->status.inventory[sd->equip_index[8]].nameid;
			} else
				WBUFW(buf,9)=0;
		}
		clif_send(buf,packet_db[0x1d7].len,bl,AREA);
	}
	else if(sd && (type == LOOK_BASE) && (val > 255))
		{
		WBUFW(buf,0)=0x1d7;
		WBUFL(buf,2)=bl->id;
		WBUFB(buf,6)=type;
		WBUFW(buf,7)=val;
		WBUFW(buf,9)=0;
		clif_send(buf,packet_db[0x1d7].len,bl,AREA);
	} else {
		WBUFW(buf,0)=0xc3;
		WBUFL(buf,2)=bl->id;
		WBUFB(buf,6)=type;
		WBUFB(buf,7)=val;
		clif_send(buf,packet_db[0xc3].len,bl,AREA);
	}
#endif
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_initialstatus(struct map_session_data *sd)
{
	int fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf=WFIFOP(fd,0);

	WBUFW(buf,0)=0xbd;
	WBUFW(buf,2)=sd->status.status_point;
	WBUFB(buf,4)=(sd->status.str > 255)? 255:sd->status.str;
	WBUFB(buf,5)=pc_need_status_point(sd,SP_STR);
	WBUFB(buf,6)=(sd->status.agi > 255)? 255:sd->status.agi;
	WBUFB(buf,7)=pc_need_status_point(sd,SP_AGI);
	WBUFB(buf,8)=(sd->status.vit > 255)? 255:sd->status.vit;
	WBUFB(buf,9)=pc_need_status_point(sd,SP_VIT);
	WBUFB(buf,10)=(sd->status.int_ > 255)? 255:sd->status.int_;
	WBUFB(buf,11)=pc_need_status_point(sd,SP_INT);
	WBUFB(buf,12)=(sd->status.dex > 255)? 255:sd->status.dex;
	WBUFB(buf,13)=pc_need_status_point(sd,SP_DEX);
	WBUFB(buf,14)=(sd->status.luk > 255)? 255:sd->status.luk;
	WBUFB(buf,15)=pc_need_status_point(sd,SP_LUK);

	WBUFW(buf,16) = sd->base_atk + sd->watk;
	WBUFW(buf,18) = sd->watk2; //atk bonus
	WBUFW(buf,20) = sd->matk1;
	WBUFW(buf,22) = sd->matk2;
	WBUFW(buf,24) = sd->def; // def
	WBUFW(buf,26) = sd->def2;
	WBUFW(buf,28) = sd->mdef; // mdef
	WBUFW(buf,30) = sd->mdef2;
	WBUFW(buf,32) = sd->hit;
	WBUFW(buf,34) = sd->flee;
	WBUFW(buf,36) = sd->flee2/10;
	WBUFW(buf,38) = sd->critical/10;
	WBUFW(buf,40) = sd->status.karma;
	WBUFW(buf,42) = sd->status.manner;

	WFIFOSET(fd,packet_db[0xbd].len);

	clif_updatestatus(sd,SP_STR);
	clif_updatestatus(sd,SP_AGI);
	clif_updatestatus(sd,SP_VIT);
	clif_updatestatus(sd,SP_INT);
	clif_updatestatus(sd,SP_DEX);
	clif_updatestatus(sd,SP_LUK);

	clif_updatestatus(sd,SP_ATTACKRANGE);
	clif_updatestatus(sd,SP_ASPD);

	return 0;
}

/*==========================================
 *矢装備
 *------------------------------------------
 */
int clif_arrowequip(struct map_session_data *sd,int val)
{
	int fd;

	nullpo_retr(0, sd);

	if(sd->attacktarget && sd->attacktarget > 0) // [Valaris]
		sd->attacktarget = 0;

	fd=sd->fd;
	WFIFOW(fd,0)=0x013c;
	WFIFOW(fd,2)=val+2;//矢のアイテムID

	WFIFOSET(fd,packet_db[0x013c].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_arrow_fail(struct map_session_data *sd,int type)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x013b;
	WFIFOW(fd,2)=type;

	WFIFOSET(fd,packet_db[0x013b].len);

	return 0;
}

/*==========================================
 * 作成可能 矢リスト送信
 *------------------------------------------
 */
int clif_arrow_create_list(struct map_session_data *sd)
{
	int i,c,view;
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1ad;

	for(i=0,c=0;i<MAX_SKILL_ARROW_DB;i++){
		if(skill_arrow_db[i].nameid > 0 && pc_search_inventory(sd,skill_arrow_db[i].nameid)>=0){
			if((view = itemdb_viewid(skill_arrow_db[i].nameid)) > 0)
				WFIFOW(fd,c*2+4) = view;
			else
				WFIFOW(fd,c*2+4) = skill_arrow_db[i].nameid;
			c++;
		}
	}
	WFIFOW(fd,2)=c*2+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	if(c > 0) sd->state.make_arrow_flag = 1;

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_statusupack(struct map_session_data *sd,int type,int ok,int val)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xbc;
	WFIFOW(fd,2)=type;
	WFIFOB(fd,4)=ok;
	WFIFOB(fd,5)=val;
	WFIFOSET(fd,packet_db[0xbc].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_equipitemack(struct map_session_data *sd,int n,int pos,int ok)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xaa;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet_db[0xaa].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_unequipitemack(struct map_session_data *sd,int n,int pos,int ok)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xac;
	WFIFOW(fd,2)=n+2;
	WFIFOW(fd,4)=pos;
	WFIFOB(fd,6)=ok;
	WFIFOSET(fd,packet_db[0xac].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_misceffect(struct block_list* bl,int type)
{
	char buf[32];

	nullpo_retr(0, bl);

	WBUFW(buf,0) = 0x19b;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	clif_send(buf,packet_db[0x19b].len,bl,AREA);

	return 0;
}
int clif_misceffect2(struct block_list *bl, int type) {
	char buf[32];

	nullpo_retr(0, bl);

	//memset(buf, 0, packet_db[0x1f3].len);

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	clif_send(buf, packet_db[0x1f3].len, bl, AREA);

	return 0;

}
/*==========================================
 * 表示オプション変更
 *------------------------------------------
 */
int clif_changeoption(struct block_list* bl)
{
	char buf[32];
	short option;
	struct status_change *sc_data;
	struct map_session_data *sd;
	static const int omask[]={ 0x10,0x20 };
	static const int scnum[]={ SC_FALCON, SC_RIDING };
	int i;

	nullpo_retr(0, bl);

	option = *battle_get_option(bl);
	sc_data = battle_get_sc_data(bl);

	WBUFW(buf,0) = 0x119;
	WBUFL(buf,2) = bl->id;
	WBUFW(buf,6) = *battle_get_opt1(bl);
	WBUFW(buf,8) = *battle_get_opt2(bl);
	WBUFW(buf,10) = option;
	WBUFB(buf,12) = 0;	// ??

	if(bl->type==BL_PC) { // disguises [Valaris]
		struct map_session_data *sd=((struct map_session_data *)bl);
		if(sd && sd->disguise > 23 && sd->disguise < 4001) {
			clif_send(buf,packet_db[0x119].len,bl,AREA_WOS);
			clif_spawnpc(sd);
		} else
			clif_send(buf,packet_db[0x119].len,bl,AREA);
	} else
		clif_send(buf,packet_db[0x119].len,bl,AREA);

	// アイコンの表示
	if(sc_data)
	{
		for(i=0;i<sizeof(omask)/sizeof(omask[0]);i++){
			if( option&omask[i] ){
				if( sc_data[scnum[i]].timer==-1)
					skill_status_change_start(bl,scnum[i],0,0,0,0,0,0);
			}else{
				skill_status_change_end(bl,scnum[i],-1);
			}
		}
	}

	if(bl->type==BL_PC && (sd=(struct map_session_data *)bl))
		clif_changelook(bl,LOOK_BASE,sd->view_class);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_useitemack(struct map_session_data *sd,int index,int amount,int ok)
{
	nullpo_retr(0, sd);

	if(!ok) {
		int fd=sd->fd;
		WFIFOW(fd,0)=0xa8;
		WFIFOW(fd,2)=index+2;
		WFIFOW(fd,4)=amount;
		WFIFOB(fd,6)=ok;
		WFIFOSET(fd,packet_db[0xa8].len);
	}
	else {
#if PACKETVER < 3
		int fd=sd->fd;
		WFIFOW(fd,0)=0xa8;
		WFIFOW(fd,2)=index+2;
		WFIFOW(fd,4)=amount;
		WFIFOB(fd,6)=ok;
		WFIFOSET(fd,packet_db[0xa8].len);
#else
		char buf[32];

		WBUFW(buf,0)=0x1c8;
		WBUFW(buf,2)=index+2;
		if(sd->inventory_data[index] && sd->inventory_data[index]->view_id > 0)
			WBUFW(buf,4)=sd->inventory_data[index]->view_id;
		else
			WBUFW(buf,4)=sd->status.inventory[index].nameid;
		WBUFL(buf,6)=sd->bl.id;
		WBUFW(buf,10)=amount;
		WBUFB(buf,12)=ok;
		clif_send(buf,packet_db[0x1c8].len,&sd->bl,AREA);
#endif
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_createchat(struct map_session_data *sd,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xd6;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_db[0xd6].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_dispchat(struct chat_data *cd,int fd)
{
	char buf[128];	// 最大title(60バイト)+17

	if(cd==NULL || *cd->owner==NULL)
		return 1;

	WBUFW(buf,0)=0xd7;
	WBUFW(buf,2)=strlen(cd->title)+18;
	WBUFL(buf,4)=(*cd->owner)->id;
	WBUFL(buf,8)=cd->bl.id;
	WBUFW(buf,12)=cd->limit;
	WBUFW(buf,14)=cd->users;
	WBUFB(buf,16)=cd->pub;
	strncpy(WBUFP(buf,17),cd->title,strlen(cd->title)+1);
	if(fd){
		memcpy(WFIFOP(fd,0),buf,WBUFW(buf,2));
		WFIFOSET(fd,WBUFW(buf,2));
	} else {
		clif_send(buf,WBUFW(buf,2),*cd->owner,AREA_WOSC);
	}

	return 0;
}

/*==========================================
 * chatの状態変更成功
 * 外部の人用と命令コード(d7->df)が違うだけ
 *------------------------------------------
 */
int clif_changechatstatus(struct chat_data *cd)
{
	char buf[128];	// 最大title(60バイト)+17

	if(cd==NULL || cd->usersd[0]==NULL)
		return 1;

	WBUFW(buf,0)=0xdf;
	WBUFW(buf,2)=strlen(cd->title)+18;
	WBUFL(buf,4)=cd->usersd[0]->bl.id;
	WBUFL(buf,8)=cd->bl.id;
	WBUFW(buf,12)=cd->limit;
	WBUFW(buf,14)=cd->users;
	WBUFB(buf,16)=cd->pub;
	strncpy(WBUFP(buf,17),cd->title,strlen(cd->title)+1);
	clif_send(buf,WBUFW(buf,2),&cd->usersd[0]->bl,CHAT);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_clearchat(struct chat_data *cd,int fd)
{
	char buf[32];

	nullpo_retr(0, cd);

	WBUFW(buf,0)=0xd8;
	WBUFL(buf,2)=cd->bl.id;
	if(fd){
		memcpy(WFIFOP(fd,0),buf,packet_db[0xd8].len);
		WFIFOSET(fd,packet_db[0xd8].len);
	} else {
		clif_send(buf,packet_db[0xd8].len,*cd->owner,AREA_WOSC);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_joinchatfail(struct map_session_data *sd,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0xda;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_db[0xda].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_joinchatok(struct map_session_data *sd,struct chat_data* cd)
{
	int fd;
	int i;

	nullpo_retr(0, sd);
	nullpo_retr(0, cd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xdb;
	WFIFOW(fd,2)=8+(28*cd->users);
	WFIFOL(fd,4)=cd->bl.id;
	for(i = 0;i < cd->users;i++){
		WFIFOL(fd,8+i*28) = (i!=0)||((*cd->owner)->type==BL_NPC);
		memcpy(WFIFOP(fd,8+i*28+4),cd->usersd[i]->status.name,24);
	}
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_addchat(struct chat_data* cd,struct map_session_data *sd)
{
	char buf[32];

	nullpo_retr(0, sd);
	nullpo_retr(0, cd);

	WBUFW(buf, 0) = 0x0dc;
	WBUFW(buf, 2) = cd->users;
	memcpy(WBUFP(buf, 4),sd->status.name,24);
	clif_send(buf,packet_db[0xdc].len,&sd->bl,CHAT_WOS);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changechatowner(struct chat_data* cd,struct map_session_data *sd)
{
	char buf[64];

	nullpo_retr(0, sd);
	nullpo_retr(0, cd);

	WBUFW(buf, 0) = 0xe1;
	WBUFL(buf, 2) = 1;
	memcpy(WBUFP(buf,6),cd->usersd[0]->status.name,24);
	WBUFW(buf,30) = 0xe1;
	WBUFL(buf,32) = 0;
	memcpy(WBUFP(buf,36),sd->status.name,24);

	clif_send(buf,packet_db[0xe1].len*2,&sd->bl,CHAT);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_leavechat(struct chat_data* cd,struct map_session_data *sd)
{
	char buf[32];

	nullpo_retr(0, sd);
	nullpo_retr(0, cd);

	WBUFW(buf, 0) = 0xdd;
	WBUFW(buf, 2) = cd->users-1;
	memcpy(WBUFP(buf,4),sd->status.name,24);
	WBUFB(buf,28) = 0;

	clif_send(buf,packet_db[0xdd].len,&sd->bl,CHAT);

	return 0;
}

/*==========================================
 * 取り引き要請受け
 *------------------------------------------
 */
int clif_traderequest(struct map_session_data *sd,char *name)
{
	int fd;
	struct map_session_data *target_sd;

	nullpo_retr(0, sd);
	nullpo_retr(0, (target_sd=map_id2sd(sd->trade_partner)));

	fd=sd->fd;
#if PACKETVER < 6
	WFIFOW(fd,0)=0xe5;
	strncpy(WFIFOP(fd,2),name,24);
	WFIFOSET(fd,packet_db[0xe5].len);
#else
	WFIFOW(fd,0)=0x1f4;
	strncpy(WFIFOP(fd,2),name,24);
	WFIFOL(fd,26)=target_sd->status.char_id;	//良く分からないからとりあえずchar_id
	WFIFOW(fd,30)=target_sd->status.base_level;
	WFIFOSET(fd,packet_db[0x1f4].len);
#endif

	return 0;
}

/*==========================================
 * 取り引き要求応答
 *------------------------------------------
 */
int clif_tradestart(struct map_session_data *sd,int type)
{
	int fd;
	struct map_session_data *target_sd;

	nullpo_retr(0, sd);

	if((target_sd=map_id2sd(sd->trade_partner)) == NULL)
		return -1;

	fd=sd->fd;
#if PACKETVER < 6
	WFIFOW(fd,0)=0xe7;
	WFIFOB(fd,2)=type;
	WFIFOSET(fd,packet_db[0xe7].len);
#else
	WFIFOW(fd,0)=0x1f5;
	WFIFOB(fd,2)=type;
	WFIFOL(fd,3)=target_sd->status.char_id;	//良く分からないからとりあえずchar_id
	WFIFOW(fd,7)=target_sd->status.base_level;
	WFIFOSET(fd,packet_db[0x1f5].len);
#endif

	return 0;
}

/*==========================================
 * 相手方からのアイテム追加
 *------------------------------------------
 */
int clif_tradeadditem(struct map_session_data *sd,struct map_session_data *tsd,int index,int amount)
{
	int fd,j;

	nullpo_retr(0, sd);
	nullpo_retr(0, tsd);

	fd=tsd->fd;
	WFIFOW(fd,0)=0xe9;
	WFIFOL(fd,2)=amount;
	if(index==0){
		WFIFOW(fd,6) = 0; // type id
		WFIFOB(fd,8) = 0; //identify flag
		WFIFOB(fd,9) = 0; // attribute
		WFIFOB(fd,10)= 0; //refine
		WFIFOW(fd,11)= 0; //card (4w)
		WFIFOW(fd,13)= 0; //card (4w)
		WFIFOW(fd,15)= 0; //card (4w)
		WFIFOW(fd,17)= 0; //card (4w)
	}
	else{
		index -= 2;
		if(sd->inventory_data[index] && sd->inventory_data[index]->view_id > 0)
			WFIFOW(fd,6) = sd->inventory_data[index]->view_id;
		else
			WFIFOW(fd,6) = sd->status.inventory[index].nameid; // type id
		WFIFOB(fd,8) = sd->status.inventory[index].identify; //identify flag
		WFIFOB(fd,9) = sd->status.inventory[index].attribute; // attribute
		WFIFOB(fd,10)= sd->status.inventory[index].refine; //refine
		if(sd->status.inventory[index].card[0]==0x00ff || sd->status.inventory[index].card[0]==0x00fe || sd->status.inventory[index].card[0]==(short)0xff00) {
			WFIFOW(fd,11)= sd->status.inventory[index].card[0]; //card (4w)
			WFIFOW(fd,13)= sd->status.inventory[index].card[1]; //card (4w)
			WFIFOW(fd,15)= sd->status.inventory[index].card[2]; //card (4w)
			WFIFOW(fd,17)= sd->status.inventory[index].card[3]; //card (4w)
		} else {
			if(sd->status.inventory[index].card[0] > 0 && (j=itemdb_viewid(sd->status.inventory[index].card[0])) > 0)
				WFIFOW(fd,11)= j;
			else
				WFIFOW(fd,11)= sd->status.inventory[index].card[0];
			if(sd->status.inventory[index].card[1] > 0 && (j=itemdb_viewid(sd->status.inventory[index].card[1])) > 0)
				WFIFOW(fd,13)= j;
			else
				WFIFOW(fd,13)= sd->status.inventory[index].card[1];
			if(sd->status.inventory[index].card[2] > 0 && (j=itemdb_viewid(sd->status.inventory[index].card[2])) > 0)
				WFIFOW(fd,15)= j;
			else
				WFIFOW(fd,15)= sd->status.inventory[index].card[2];
			if(sd->status.inventory[index].card[3] > 0 && (j=itemdb_viewid(sd->status.inventory[index].card[3])) > 0)
				WFIFOW(fd,17)= j;
			else
				WFIFOW(fd,17)= sd->status.inventory[index].card[3];
		}
	}
	WFIFOSET(fd,packet_db[0xe9].len);

	return 0;
}

/*==========================================
 * アイテム追加成功/失敗
 *------------------------------------------
 */
int clif_tradeitemok(struct map_session_data *sd,int index,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xea;
	WFIFOW(fd,2)=index;
	WFIFOB(fd,4)=fail;
	WFIFOSET(fd,packet_db[0xea].len);

	return 0;
}

/*==========================================
 * 取り引きok押し
 *------------------------------------------
 */
int clif_tradedeal_lock(struct map_session_data *sd,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xec;
	WFIFOB(fd,2)=fail; // 0=you 1=the other person
	WFIFOSET(fd,packet_db[0xec].len);

	return 0;
}

/*==========================================
 * 取り引きがキャンセルされました
 *------------------------------------------
 */
int clif_tradecancelled(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xee;
	WFIFOSET(fd,packet_db[0xee].len);

	return 0;
}

/*==========================================
 * 取り引き完了
 *------------------------------------------
 */
int clif_tradecompleted(struct map_session_data *sd,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xf0;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_db[0xf0].len);

	return 0;
}

/*==========================================
 * カプラ倉庫のアイテム数を更新
 *------------------------------------------
 */
int clif_updatestorageamount(struct map_session_data *sd,struct storage *stor)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor->storage_amount;  //items
	WFIFOW(fd,4) = MAX_STORAGE; //items max
	WFIFOSET(fd,packet_db[0xf2].len);

	return 0;
}

/*==========================================
 * カプラ倉庫にアイテムを追加する
 *------------------------------------------
 */
int clif_storageitemadded(struct map_session_data *sd,struct storage *stor,int index,int amount)
{
	int view,fd,j;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	WFIFOW(fd,0) =0xf4; // Storage item added
	WFIFOW(fd,2) =index+1; // index
	WFIFOL(fd,4) =amount; // amount
	if((view = itemdb_viewid(stor->storage[index].nameid)) > 0)
		WFIFOW(fd,8) =view;
	else
		WFIFOW(fd,8) =stor->storage[index].nameid; // id
	WFIFOB(fd,10)=stor->storage[index].identify; //identify flag
	WFIFOB(fd,11)=stor->storage[index].attribute; // attribute
	WFIFOB(fd,12)=stor->storage[index].refine; //refine
	if(stor->storage[index].card[0]==0x00ff || stor->storage[index].card[0]==0x00fe || stor->storage[index].card[0]==(short)0xff00) {
		WFIFOW(fd,13)=stor->storage[index].card[0]; //card (4w)
		WFIFOW(fd,15)=stor->storage[index].card[1]; //card (4w)
		WFIFOW(fd,17)=stor->storage[index].card[2]; //card (4w)
		WFIFOW(fd,19)=stor->storage[index].card[3]; //card (4w)
	} else {
		if(stor->storage[index].card[0] > 0 && (j=itemdb_viewid(stor->storage[index].card[0])) > 0)
			WFIFOW(fd,13)= j;
		else
			WFIFOW(fd,13)= stor->storage[index].card[0];
		if(stor->storage[index].card[1] > 0 && (j=itemdb_viewid(stor->storage[index].card[1])) > 0)
			WFIFOW(fd,15)= j;
		else
			WFIFOW(fd,15)= stor->storage[index].card[1];
		if(stor->storage[index].card[2] > 0 && (j=itemdb_viewid(stor->storage[index].card[2])) > 0)
			WFIFOW(fd,17)= j;
		else
			WFIFOW(fd,17)= stor->storage[index].card[2];
		if(stor->storage[index].card[3] > 0 && (j=itemdb_viewid(stor->storage[index].card[3])) > 0)
			WFIFOW(fd,19)= j;
		else
			WFIFOW(fd,19)= stor->storage[index].card[3];
	}
	WFIFOSET(fd,packet_db[0xf4].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_updateguildstorageamount(struct map_session_data *sd,struct guild_storage *stor)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	WFIFOW(fd,0) = 0xf2;  // update storage amount
	WFIFOW(fd,2) = stor->storage_amount;  //items
	WFIFOW(fd,4) = MAX_GUILD_STORAGE; //items max
	WFIFOSET(fd,packet_db[0xf2].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_guildstorageitemadded(struct map_session_data *sd,struct guild_storage *stor,int index,int amount)
{
	int view,fd,j;

	nullpo_retr(0, sd);
	nullpo_retr(0, stor);

	fd=sd->fd;
	WFIFOW(fd,0) =0xf4; // Storage item added
	WFIFOW(fd,2) =index+1; // index
	WFIFOL(fd,4) =amount; // amount
	if((view = itemdb_viewid(stor->storage[index].nameid)) > 0)
		WFIFOW(fd,8) =view;
	else
		WFIFOW(fd,8) =stor->storage[index].nameid; // id
	WFIFOB(fd,10)=stor->storage[index].identify; //identify flag
	WFIFOB(fd,11)=stor->storage[index].attribute; // attribute
	WFIFOB(fd,12)=stor->storage[index].refine; //refine
	if(stor->storage[index].card[0]==0x00ff || stor->storage[index].card[0]==0x00fe || stor->storage[index].card[0]==(short)0xff00) {
		WFIFOW(fd,13)=stor->storage[index].card[0]; //card (4w)
		WFIFOW(fd,15)=stor->storage[index].card[1]; //card (4w)
		WFIFOW(fd,17)=stor->storage[index].card[2]; //card (4w)
		WFIFOW(fd,19)=stor->storage[index].card[3]; //card (4w)
	} else {
		if(stor->storage[index].card[0] > 0 && (j=itemdb_viewid(stor->storage[index].card[0])) > 0)
			WFIFOW(fd,13)= j;
		else
			WFIFOW(fd,13)= stor->storage[index].card[0];
		if(stor->storage[index].card[1] > 0 && (j=itemdb_viewid(stor->storage[index].card[1])) > 0)
			WFIFOW(fd,15)= j;
		else
			WFIFOW(fd,15)= stor->storage[index].card[1];
		if(stor->storage[index].card[2] > 0 && (j=itemdb_viewid(stor->storage[index].card[2])) > 0)
			WFIFOW(fd,17)= j;
		else
			WFIFOW(fd,17)= stor->storage[index].card[2];
		if(stor->storage[index].card[3] > 0 && (j=itemdb_viewid(stor->storage[index].card[3])) > 0)
			WFIFOW(fd,19)= j;
		else
			WFIFOW(fd,19)= stor->storage[index].card[3];
	}
	WFIFOSET(fd,packet_db[0xf4].len);

	return 0;
}

/*==========================================
 * カプラ倉庫からアイテムを取り去る
 *------------------------------------------
 */
int clif_storageitemremoved(struct map_session_data *sd,int index,int amount)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xf6; // Storage item removed
	WFIFOW(fd,2)=index+1;
	WFIFOL(fd,4)=amount;
	WFIFOSET(fd,packet_db[0xf6].len);

	return 0;
}

/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
int clif_storageclose(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xf8; // Storage Closed
	WFIFOSET(fd,packet_db[0xf8].len);

	return 0;
}

//
// callback系 ?
//
/*==========================================
 * PC表示
 *------------------------------------------
 */
void clif_getareachar_pc(struct map_session_data* sd,struct map_session_data* dstsd)
{
	int len;

	nullpo_retv(sd);
	nullpo_retv(dstsd);

	if(dstsd->walktimer != -1){
		len = clif_set007b(dstsd,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	} else {
		len = clif_set0078(dstsd,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	}

	if(dstsd->chatID){
		struct chat_data *cd;
		cd=(struct chat_data*)map_id2bl(dstsd->chatID);
		if(cd->usersd[0]==dstsd)
			clif_dispchat(cd,sd->fd);
	}
	if(dstsd->vender_id){
		clif_showvendingboard(&dstsd->bl,dstsd->message,sd->fd);
	}
	if(dstsd->spiritball > 0) {
		clif_set01e1(dstsd,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,packet_db[0x1e1].len);
	}
	if(battle_config.save_clothcolor==1 && dstsd->status.clothes_color > 0)
		clif_changelook(&dstsd->bl,LOOK_CLOTHES_COLOR,dstsd->status.clothes_color);

	if(sd->status.manner < 0)
		clif_changestatus(&sd->bl,SP_MANNER,sd->status.manner);

}

/*==========================================
 * NPC表示
 *------------------------------------------
 */
void clif_getareachar_npc(struct map_session_data* sd,struct npc_data* nd)
{
	int len;

	nullpo_retv(sd);
	nullpo_retv(nd);

	if(nd->class < 0 ||(nd->flag&1 && nd->option != 0x0002) || nd->class == INVISIBLE_CLASS)
		return;

	len = clif_npc0078(nd,WFIFOP(sd->fd,0));
	WFIFOSET(sd->fd,len);

	if(nd->chat_id){
		clif_dispchat((struct chat_data*)map_id2bl(nd->chat_id),sd->fd);
	}

}

/*==========================================
 * 移動停止
 *------------------------------------------
 */
int clif_movemob(struct mob_data *md)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, md);

	len = clif_mob007b(md,buf);
	clif_send(buf,len,&md->bl,AREA);

	if(mob_get_equip(md->class) > 0) // mob equipment [Valaris]
		clif_mob_equip(md,mob_get_equip(md->class));

	return 0;
}

/*==========================================
 * モンスターの位置修正
 *------------------------------------------
 */
int clif_fixmobpos(struct mob_data *md)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, md);

	if(md->state.state == MS_WALK){
		len = clif_mob007b(md,buf);
		clif_send(buf,len,&md->bl,AREA);
	} else {
		len = clif_mob0078(md,buf);
		clif_send(buf,len,&md->bl,AREA);
	}

	return 0;
}

/*==========================================
 * PCの位置修正
 *------------------------------------------
 */
int clif_fixpcpos(struct map_session_data *sd)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, sd);

	if(sd->walktimer != -1){
		len = clif_set007b(sd,buf);
		clif_send(buf,len,&sd->bl,AREA);
	} else {
		len = clif_set0078(sd,buf);
		clif_send(buf,len,&sd->bl,AREA);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_fixpetpos(struct pet_data *pd)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, pd);

	if(pd->state.state == MS_WALK){
		len = clif_pet007b(pd,buf);
		clif_send(buf,len,&pd->bl,AREA);
	} else {
		len = clif_pet0078(pd,buf);
		clif_send(buf,len,&pd->bl,AREA);
	}

	return 0;
}

// npc walking [Valaris]
int clif_fixnpcpos(struct npc_data *nd)
{
	unsigned char buf[256];
	int len;

	nullpo_retr(0, nd);

	if(nd->state.state == MS_WALK){
		len = clif_npc007b(nd,buf);
		clif_send(buf,len,&nd->bl,AREA);
	} else {
		len = clif_npc0078(nd,buf);
		clif_send(buf,len,&nd->bl,AREA);
	}

	return 0;
}

/*==========================================
 * 通常攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_damage(struct block_list *src,struct block_list *dst,unsigned int tick,int sdelay,int ddelay,int damage,int div,int type,int damage2)
{
	unsigned char buf[256];
	struct status_change *sc_data;

	nullpo_retr(0, src);
	nullpo_retr(0, dst);

	sc_data = battle_get_sc_data(dst);

	if(type != 4 && dst->type == BL_PC && ((struct map_session_data *)dst)->special_state.infinite_endure)
		type = 9;
	if(sc_data) {
		if(type != 4 && sc_data[SC_ENDURE].timer != -1)
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1) {
			if(damage > 0)
				damage = damage*(5+sc_data[SC_HALLUCINATION].val1) + rand()%100;
			if(damage2 > 0)
				damage2 = damage2*(5+sc_data[SC_HALLUCINATION].val1) + rand()%100;
		}
	}

	WBUFW(buf,0)=0x8a;
	WBUFL(buf,2)=src->id;
	WBUFL(buf,6)=dst->id;
	WBUFL(buf,10)=tick;
	WBUFL(buf,14)=sdelay;
	WBUFL(buf,18)=ddelay;
	WBUFW(buf,22)=(damage > 0x7fff)? 0x7fff:damage;
	WBUFW(buf,24)=div;
	WBUFB(buf,26)=type;
	WBUFW(buf,27)=damage2;
	clif_send(buf,packet_db[0x8a].len,src,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_getareachar_mob(struct map_session_data* sd,struct mob_data* md)
{
	int len;
	nullpo_retv(sd);
	nullpo_retv(md);

	if(md->state.state == MS_WALK){
		len = clif_mob007b(md,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	} else {
		len = clif_mob0078(md,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	}

	if(mob_get_equip(md->class) > 0) // mob equipment [Valaris]
		clif_mob_equip(md,mob_get_equip(md->class));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_getareachar_pet(struct map_session_data* sd,struct pet_data* pd)
{
	int len;

	nullpo_retv(sd);
	nullpo_retv(pd);

	if(pd->state.state == MS_WALK){
		len = clif_pet007b(pd,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	} else {
		len = clif_pet0078(pd,WFIFOP(sd->fd,0));
		WFIFOSET(sd->fd,len);
	}
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_getareachar_item(struct map_session_data* sd,struct flooritem_data* fitem)
{
	int view,fd;

	nullpo_retv(sd);
	nullpo_retv(fitem);

	fd=sd->fd;
	//009d <ID>.l <item ID>.w <identify flag>.B <X>.w <Y>.w <amount>.w <subX>.B <subY>.B
	WFIFOW(fd,0)=0x9d;
	WFIFOL(fd,2)=fitem->bl.id;
	if((view = itemdb_viewid(fitem->item_data.nameid)) > 0)
		WFIFOW(fd,6)=view;
	else
		WFIFOW(fd,6)=fitem->item_data.nameid;
	WFIFOB(fd,8)=fitem->item_data.identify;
	WFIFOW(fd,9)=fitem->bl.x;
	WFIFOW(fd,11)=fitem->bl.y;
	WFIFOW(fd,13)=fitem->item_data.amount;
	WFIFOB(fd,15)=fitem->subx;
	WFIFOB(fd,16)=fitem->suby;

	WFIFOSET(fd,packet_db[0x9d].len);
}
/*==========================================
 * 場所スキルエフェクトが視界に入る
 *------------------------------------------
 */
int clif_getareachar_skillunit(struct map_session_data *sd,struct skill_unit *unit)
{
	int fd;
	struct block_list *bl;

	nullpo_retr(0, unit);

	fd=sd->fd;
	bl=map_id2bl(unit->group->src_id);
#if PACKETVER < 3
	memset(WFIFOP(fd,0),0,packet_db[0x11f].len);
	WFIFOW(fd, 0)=0x11f;
	WFIFOL(fd, 2)=unit->bl.id;
	WFIFOL(fd, 6)=unit->group->src_id;
	WFIFOW(fd,10)=unit->bl.x;
	WFIFOW(fd,12)=unit->bl.y;
	WFIFOB(fd,14)=unit->group->unit_id;
	WFIFOB(fd,15)=0;
	WFIFOSET(fd,packet_db[0x11f].len);
#else
	memset(WFIFOP(fd,0),0,packet_db[0x1c9].len);
	WFIFOW(fd, 0)=0x1c9;
	WFIFOL(fd, 2)=unit->bl.id;
	WFIFOL(fd, 6)=unit->group->src_id;
	WFIFOW(fd,10)=unit->bl.x;
	WFIFOW(fd,12)=unit->bl.y;
	WFIFOB(fd,14)=unit->group->unit_id;
	WFIFOB(fd,15)=1;
	if(unit->group->unit_id==0xb0){	//グラフィティ
		WFIFOB(fd,15+1)=1;
		memcpy(WFIFOP(fd,15+2),unit->group->valstr,80);
	}else{
		WFIFOL(fd,15+1)=0;						//1-4調べた限り固定
		WFIFOL(fd,15+5)=0;						//5-8調べた限り固定
												//9-12マップごとで一定の77-80とはまた違う4バイトのかなり大きな数字
		WFIFOL(fd,15+13)=unit->bl.y - 0x12;		//13-16ユニットのY座標-18っぽい(Y:17でFF FF FF FF)
		WFIFOL(fd,15+17)=0x004f37dd;			//17-20調べた限り固定
		WFIFOL(fd,15+21)=0x0012f674;			//21-24調べた限り固定
		WFIFOL(fd,15+25)=0x0012f664;			//25-28調べた限り固定
		WFIFOL(fd,15+29)=0x0012f654;			//29-32調べた限り固定
		WFIFOL(fd,15+33)=0x77527bbc;			//33-36調べた限り固定
												//37-39
		WFIFOB(fd,15+40)=0x2d;					//40調べた限り固定
		WFIFOL(fd,15+41)=0;						//41-44調べた限り0固定
		WFIFOL(fd,15+45)=0;						//45-48調べた限り0固定
		WFIFOL(fd,15+49)=0;						//49-52調べた限り0固定
		WFIFOL(fd,15+53)=0x0048d919;			//53-56調べた限り固定
		WFIFOL(fd,15+57)=0x0000003e;			//57-60調べた限り固定
		WFIFOL(fd,15+61)=0x0012f66c;			//61-64調べた限り固定
												//65-68
												//69-72
		if(bl) WFIFOL(fd,15+73)=bl->y;			//73-76術者のY座標
		WFIFOL(fd,15+77)=unit->bl.m;			//77-80マップIDかなぁ？かなり2バイトで足りそうな数字
		WFIFOB(fd,15+81)=0xaa;					//81終端文字0xaa
	}

		/*	Graffiti [Valaris]	*/
		if(unit->group->unit_id==0xb0)	{
			WFIFOL(fd,15)=1;
			WFIFOL(fd,16)=1;
			memcpy(WFIFOP(fd,17),unit->group->valstr,80);
		}

	WFIFOSET(fd,packet_db[0x1c9].len);
#endif
	if(unit->group->skill_id == WZ_ICEWALL)
		clif_set0192(fd,unit->bl.m,unit->bl.x,unit->bl.y,5);

	return 0;
}
/*==========================================
 * 場所スキルエフェクトが視界から消える
 *------------------------------------------
 */
int clif_clearchar_skillunit(struct skill_unit *unit,int fd)
{
	nullpo_retr(0, unit);

	WFIFOW(fd, 0)=0x120;
	WFIFOL(fd, 2)=unit->bl.id;
	WFIFOSET(fd,packet_db[0x120].len);
	if(unit->group->skill_id == WZ_ICEWALL)
		clif_set0192(fd,unit->bl.m,unit->bl.x,unit->bl.y,unit->val2);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_01ac(struct block_list *bl)
{
	char buf[32];

	nullpo_retr(0, bl);

	WBUFW(buf, 0) = 0x1ac;
	WBUFL(buf, 2) = bl->id;

	clif_send(buf,packet_db[0x1ac].len,bl,AREA);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
 int clif_getareachar(struct block_list* bl,va_list ap)
{
	struct map_session_data *sd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	sd=va_arg(ap,struct map_session_data*);

	switch(bl->type){
	case BL_PC:
		if(sd==(struct map_session_data*)bl)
			break;
		clif_getareachar_pc(sd,(struct map_session_data*) bl);
		break;
	case BL_NPC:
		clif_getareachar_npc(sd,(struct npc_data*) bl);
		break;
	case BL_MOB:
		clif_getareachar_mob(sd,(struct mob_data*) bl);
		break;
	case BL_PET:
		clif_getareachar_pet(sd,(struct pet_data*) bl);
		break;
	case BL_ITEM:
		clif_getareachar_item(sd,(struct flooritem_data*) bl);
		break;
	case BL_SKILL:
		clif_getareachar_skillunit(sd,(struct skill_unit *)bl);
		break;
	default:
		if(battle_config.error_log)
			printf("get area char ??? %d\n",bl->type);
		break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pcoutsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd,*dstsd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, sd=va_arg(ap,struct map_session_data*));

	switch(bl->type){
	case BL_PC:
		dstsd = (struct map_session_data*) bl;
		if(sd != dstsd) {
			clif_clearchar_id(dstsd->bl.id,0,sd->fd);
			clif_clearchar_id(sd->bl.id,0,dstsd->fd);
			if(dstsd->chatID){
				struct chat_data *cd;
				cd=(struct chat_data*)map_id2bl(dstsd->chatID);
				if(cd->usersd[0]==dstsd)
					clif_dispchat(cd,sd->fd);
			}
			if(dstsd->vender_id){
				clif_closevendingboard(&dstsd->bl,sd->fd);
			}
		}
		break;
	case BL_NPC:
		if( ((struct npc_data *)bl)->class != INVISIBLE_CLASS )
			clif_clearchar_id(bl->id,0,sd->fd);
		break;
	case BL_MOB:
	case BL_PET:
		clif_clearchar_id(bl->id,0,sd->fd);
		break;
	case BL_ITEM:
		clif_clearflooritem((struct flooritem_data*)bl,sd->fd);
		break;
	case BL_SKILL:
		clif_clearchar_skillunit((struct skill_unit *)bl,sd->fd);
		break;
	}
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pcinsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd,*dstsd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, sd=va_arg(ap,struct map_session_data*));

	switch(bl->type){
	case BL_PC:
		dstsd = (struct map_session_data *)bl;
		if(sd != dstsd) {
			clif_getareachar_pc(sd,dstsd);
			clif_getareachar_pc(dstsd,sd);
		}
		break;
	case BL_NPC:
		clif_getareachar_npc(sd,(struct npc_data*)bl);
		break;
	case BL_MOB:
		clif_getareachar_mob(sd,(struct mob_data*)bl);
		break;
	case BL_PET:
		clif_getareachar_pet(sd,(struct pet_data*)bl);
		break;
	case BL_ITEM:
		clif_getareachar_item(sd,(struct flooritem_data*)bl);
		break;
	case BL_SKILL:
		clif_getareachar_skillunit(sd,(struct skill_unit *)bl);
		break;
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_moboutsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct mob_data *md;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, md=va_arg(ap,struct mob_data*));

	if(bl->type==BL_PC && (sd = (struct map_session_data*) bl)){
		clif_clearchar_id(md->bl.id,0,sd->fd);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_mobinsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct mob_data *md;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	md=va_arg(ap,struct mob_data*);
	if(bl->type==BL_PC && (sd = (struct map_session_data *)bl)){
		clif_getareachar_mob(sd,md);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_petoutsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct pet_data *pd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, pd=va_arg(ap,struct pet_data*));

	if(bl->type==BL_PC && (sd = (struct map_session_data*) bl)){
		clif_clearchar_id(pd->bl.id,0,sd->fd);
	}

	return 0;
}
// npc walking [Valaris]
int clif_npcoutsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct npc_data *nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);
	nullpo_retr(0, nd=va_arg(ap,struct npc_data*));

	if(bl->type==BL_PC && (sd = (struct map_session_data*) bl)){
		clif_clearchar_id(nd->bl.id,0,sd->fd);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_petinsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct pet_data *pd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	pd=va_arg(ap,struct pet_data*);
	if(bl->type==BL_PC && (sd = (struct map_session_data *)bl)){
		clif_getareachar_pet(sd,pd);
	}

	return 0;
}
// npc walking [Valaris]
int clif_npcinsight(struct block_list *bl,va_list ap)
{
	struct map_session_data *sd;
	struct npc_data *nd;

	nullpo_retr(0, bl);
	nullpo_retr(0, ap);

	nd=va_arg(ap,struct npc_data*);
	if(bl->type==BL_PC && (sd = (struct map_session_data *)bl)){
		clif_getareachar_npc(sd,nd);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_skillinfo(struct map_session_data *sd,int skillid,int type,int range)
{
	int fd,id;

	nullpo_retr(0, sd);

	fd=sd->fd;
	if( (id=sd->status.skill[skillid].id) <= 0 )
		return 0;
	WFIFOW(fd,0)=0x147;
	WFIFOW(fd,2) = id;
	if(type < 0)
		WFIFOW(fd,4) = skill_get_inf(id);
	else
		WFIFOW(fd,4) = type;
	WFIFOW(fd,6) = 0;
	WFIFOW(fd,8) = sd->status.skill[skillid].lv;
	WFIFOW(fd,10) = skill_get_sp(id,sd->status.skill[skillid].lv);
	if(range < 0) {
		range = skill_get_range(id,sd->status.skill[skillid].lv);
		if(range < 0)
			range = battle_get_range(&sd->bl) - (range + 1);
		WFIFOW(fd,12)= range;
	} else
		WFIFOW(fd,12)= range;
	memset(WFIFOP(fd,14),0,24);
	if(!(skill_get_inf2(id)&0x01) || battle_config.quest_skill_learn == 1 || (battle_config.gm_allskill > 0 && pc_isGM(sd) >= battle_config.gm_allskill) )
		WFIFOB(fd,38)= (sd->status.skill[skillid].lv < skill_get_max(id) && sd->status.skill[skillid].flag ==0 )? 1:0;
	else
		WFIFOB(fd,38) = 0;
	WFIFOSET(fd,packet_db[0x147].len);

	return 0;
}

/*==========================================
 * スキルリストを送信する
 *------------------------------------------
 */
int clif_skillinfoblock(struct map_session_data *sd)
{
	int fd;
	int i,c,len=4,id,range;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x10f;
	for ( i = c = 0; i < MAX_SKILL; i++){
		if( (id=sd->status.skill[i].id)!=0 ){
			WFIFOW(fd,len  ) = id;
			WFIFOW(fd,len+2) = skill_get_inf(id);
			WFIFOW(fd,len+4) = 0;
			WFIFOW(fd,len+6) = sd->status.skill[i].lv;
			WFIFOW(fd,len+8) = skill_get_sp(id,sd->status.skill[i].lv);
			range = skill_get_range(id,sd->status.skill[i].lv);
			if(range < 0)
				range = battle_get_range(&sd->bl) - (range + 1);
			WFIFOW(fd,len+10)= range;
			memset(WFIFOP(fd,len+12),0,24);
			if(!(skill_get_inf2(id)&0x01) || battle_config.quest_skill_learn == 1 || (battle_config.gm_allskill > 0 && pc_isGM(sd) >= battle_config.gm_allskill) )
				WFIFOB(fd,len+36)= (sd->status.skill[i].lv < skill_get_max(id) && sd->status.skill[i].flag ==0 )? 1:0;
			else
				WFIFOB(fd,len+36) = 0;
			len+=37;
			c++;
		}
	}
	WFIFOW(fd,2)=len;
	WFIFOSET(fd,len);

	return 0;
}

/*==========================================
 * スキル割り振り通知
 *------------------------------------------
 */
int clif_skillup(struct map_session_data *sd,int skill_num)
{
	int range,fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skill_num;
	WFIFOW(fd,4) = sd->status.skill[skill_num].lv;
	WFIFOW(fd,6) = skill_get_sp(skill_num,sd->status.skill[skill_num].lv);
	range = skill_get_range(skill_num,sd->status.skill[skill_num].lv);
	if(range < 0)
		range = battle_get_range(&sd->bl) - (range + 1);
	WFIFOW(fd,8) = range;
	WFIFOB(fd,10) = (sd->status.skill[skill_num].lv < skill_get_max(sd->status.skill[skill_num].id)) ? 1 : 0;
	WFIFOSET(fd,packet_db[0x10e].len);

	return 0;
}

/*==========================================
 * スキル詠唱エフェクトを送信する
 *------------------------------------------
 */
int clif_skillcasting(struct block_list* bl,
	int src_id,int dst_id,int dst_x,int dst_y,int skill_num,int casttime)
{
	unsigned char buf[32];
	WBUFW(buf,0) = 0x13e;
	WBUFL(buf,2) = src_id;
	WBUFL(buf,6) = dst_id;
	WBUFW(buf,10) = dst_x;
	WBUFW(buf,12) = dst_y;
	WBUFW(buf,14) = skill_num;//魔法詠唱スキル
	WBUFL(buf,16) = skill_get_pl(skill_num);//属性
	WBUFL(buf,20) = casttime;//skill詠唱時間
	clif_send(buf,packet_db[0x13e].len, bl, AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_skillcastcancel(struct block_list* bl)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	WBUFW(buf,0) = 0x1b9;
	WBUFL(buf,2) = bl->id;
	clif_send(buf,packet_db[0x1b9].len, bl, AREA);

	return 0;
}

/*==========================================
 * スキル詠唱失敗
 *------------------------------------------
 */
int clif_skill_fail(struct map_session_data *sd,int skill_id,int type,int btype)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	if(type==0x4 && battle_config.display_delay_skill_fail==0){
		return 0;
	}

	WFIFOW(fd,0) = 0x110;
	WFIFOW(fd,2) = skill_id;
	WFIFOW(fd,4) = btype;
	WFIFOW(fd,6) = 0;
	WFIFOB(fd,8) = 0;
	WFIFOB(fd,9) = type;
	WFIFOSET(fd,packet_db[0x110].len);

	return 0;
}

/*==========================================
 * スキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage(struct block_list *src,struct block_list *dst,
	unsigned int tick,int sdelay,int ddelay,int damage,int div,int skill_id,int skill_lv,int type)
{
	unsigned char buf[64];
	struct status_change *sc_data;

	nullpo_retr(0, src);
	nullpo_retr(0, dst);

	sc_data = battle_get_sc_data(dst);

	if(type != 5 && dst->type == BL_PC && ((struct map_session_data *)dst)->special_state.infinite_endure)
		type = 9;
	if(sc_data) {
		if(type != 5 && sc_data[SC_ENDURE].timer != -1)
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1 && damage > 0)
			damage = damage*(5+sc_data[SC_HALLUCINATION].val1) + rand()%100;
	}

#if PACKETVER < 3
	WBUFW(buf,0)=0x114;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFW(buf,24)=damage;
	WBUFW(buf,26)=skill_lv;
	WBUFW(buf,28)=div;
	WBUFB(buf,30)=(type>0)?type:skill_get_hit(skill_id);
	clif_send(buf,packet_db[0x114].len,src,AREA);
#else
	WBUFW(buf,0)=0x1de;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFL(buf,24)=damage;
	WBUFW(buf,28)=skill_lv;
	WBUFW(buf,30)=div;
	WBUFB(buf,32)=(type>0)?type:skill_get_hit(skill_id);
	clif_send(buf,packet_db[0x1de].len,src,AREA);
#endif

	return 0;
}

/*==========================================
 * 吹き飛ばしスキル攻撃エフェクト＆ダメージ
 *------------------------------------------
 */
int clif_skill_damage2(struct block_list *src,struct block_list *dst,
	unsigned int tick,int sdelay,int ddelay,int damage,int div,int skill_id,int skill_lv,int type)
{
	unsigned char buf[64];
	struct status_change *sc_data;

	nullpo_retr(0, src);
	nullpo_retr(0, dst);

	sc_data = battle_get_sc_data(dst);

	if(type != 5 && dst->type == BL_PC && ((struct map_session_data *)dst)->special_state.infinite_endure)
		type = 9;
	if(sc_data) {
		if(type != 5 && sc_data[SC_ENDURE].timer != -1)
			type = 9;
		if(sc_data[SC_HALLUCINATION].timer != -1 && damage > 0)
			damage = damage*(5+sc_data[SC_HALLUCINATION].val1) + rand()%100;
	}

	WBUFW(buf,0)=0x115;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFL(buf,8)=dst->id;
	WBUFL(buf,12)=tick;
	WBUFL(buf,16)=sdelay;
	WBUFL(buf,20)=ddelay;
	WBUFW(buf,24)=dst->x;
	WBUFW(buf,26)=dst->y;
	WBUFW(buf,28)=damage;
	WBUFW(buf,30)=skill_lv;
	WBUFW(buf,32)=div;
	WBUFB(buf,34)=(type>0)?type:skill_get_hit(skill_id);
	clif_send(buf,packet_db[0x115].len,src,AREA);

	return 0;
}

/*==========================================
 * 支援/回復スキルエフェクト
 *------------------------------------------
 */
int clif_skill_nodamage(struct block_list *src,struct block_list *dst,
	int skill_id,int heal,int fail)
{
	unsigned char buf[32];

	nullpo_retr(0, src);
	nullpo_retr(0, dst);

	if(heal>0x7fff && skill_id != (NPC_SELFDESTRUCTION || NPC_SELFDESTRUCTION2))
		heal=0x7fff;

	WBUFW(buf,0)=0x11a;
	WBUFW(buf,2)=skill_id;
	WBUFW(buf,4)=heal;
	WBUFL(buf,6)=dst->id;
	WBUFL(buf,10)=src->id;
	WBUFB(buf,14)=fail;
	clif_send(buf,packet_db[0x11a].len,src,AREA);

	return 0;
}

/*==========================================
 * 場所スキルエフェクト
 *------------------------------------------
 */
int clif_skill_poseffect(struct block_list *src,int skill_id,int val,int x,int y,int tick)
{
	unsigned char buf[32];

	nullpo_retr(0, src);

	WBUFW(buf,0)=0x117;
	WBUFW(buf,2)=skill_id;
	WBUFL(buf,4)=src->id;
	WBUFW(buf,8)=val;
	WBUFW(buf,10)=x;
	WBUFW(buf,12)=y;
	WBUFL(buf,14)=tick;
	clif_send(buf,packet_db[0x117].len,src,AREA);

	return 0;
}

/*==========================================
 * 場所スキルエフェクト表示
 *------------------------------------------
 */
int clif_skill_setunit(struct skill_unit *unit)
{
	unsigned char buf[128];
	struct block_list *bl;

	nullpo_retr(0, unit);

	bl=map_id2bl(unit->group->src_id);

#if PACKETVER < 3
	memset(WBUFP(buf, 0),0,packet_db[0x11f].len);
	WBUFW(buf, 0)=0x11f;
	WBUFL(buf, 2)=unit->bl.id;
	WBUFL(buf, 6)=unit->group->src_id;
	WBUFW(buf,10)=unit->bl.x;
	WBUFW(buf,12)=unit->bl.y;
	WBUFB(buf,14)=unit->group->unit_id;
	WBUFB(buf,15)=0;
	clif_send(buf,packet_db[0x11f].len,&unit->bl,AREA);
#else
	memset(WBUFP(buf, 0),0,packet_db[0x1c9].len);
	WBUFW(buf, 0)=0x1c9;
	WBUFL(buf, 2)=unit->bl.id;
	WBUFL(buf, 6)=unit->group->src_id;
	WBUFW(buf,10)=unit->bl.x;
	WBUFW(buf,12)=unit->bl.y;
	WBUFB(buf,14)=unit->group->unit_id;
	WBUFB(buf,15)=1;

	if(unit->group->unit_id==0xb0){	//グラフィティ
		WBUFB(buf,15+1)=1;
		memcpy(WBUFP(buf,15+2),unit->group->valstr,80);
	}else{
		WBUFL(buf,15+1)=0;						//1-4調べた限り固定
		WBUFL(buf,15+5)=0;						//5-8調べた限り固定
												//9-12マップごとで一定の77-80とはまた違う4バイトのかなり大きな数字
		WBUFL(buf,15+13)=unit->bl.y - 0x12;		//13-16ユニットのY座標-18っぽい(Y:17でFF FF FF FF)
		WBUFL(buf,15+17)=0x004f37dd;			//17-20調べた限り固定(0x1b2で0x004fdbddだった)
		WBUFL(buf,15+21)=0x0012f674;			//21-24調べた限り固定
		WBUFL(buf,15+25)=0x0012f664;			//25-28調べた限り固定
		WBUFL(buf,15+29)=0x0012f654;			//29-32調べた限り固定
		WBUFL(buf,15+33)=0x77527bbc;			//33-36調べた限り固定
												//37-39
		WBUFB(buf,15+40)=0x2d;					//40調べた限り固定
		WBUFL(buf,15+41)=0;						//41-44調べた限り0固定
		WBUFL(buf,15+45)=0;						//45-48調べた限り0固定
		WBUFL(buf,15+49)=0;						//49-52調べた限り0固定
		WBUFL(buf,15+53)=0x0048d919;			//53-56調べた限り固定(0x01b2で0x00495119だった)
		WBUFL(buf,15+57)=0x0000003e;			//57-60調べた限り固定
		WBUFL(buf,15+61)=0x0012f66c;			//61-64調べた限り固定
												//65-68
												//69-72
		if(bl) WBUFL(buf,15+73)=bl->y;			//73-76術者のY座標
		WBUFL(buf,15+77)=unit->bl.m;			//77-80マップIDかなぁ？かなり2バイトで足りそうな数字
		WBUFB(buf,15+81)=0xaa;					//81終端文字0xaa
	}

	/*		Graffiti [Valaris]		*/
	if(unit->group->unit_id==0xb0)	{
		WBUFL(buf,15)=1;
		WBUFL(buf,16)=1;
		memcpy(WBUFP(buf,17),unit->group->valstr,80);
	}

		clif_send(buf,packet_db[0x1c9].len,&unit->bl,AREA);
#endif
	return 0;
}
/*==========================================
 * 場所スキルエフェクト削除
 *------------------------------------------
 */
int clif_skill_delunit(struct skill_unit *unit)
{
	unsigned char buf[16];

	nullpo_retr(0, unit);

	WBUFW(buf, 0)=0x120;
	WBUFL(buf, 2)=unit->bl.id;
	clif_send(buf,packet_db[0x120].len,&unit->bl,AREA);
	return 0;
}
/*==========================================
 * ワープ場所選択
 *------------------------------------------
 */
int clif_skill_warppoint(struct map_session_data *sd,int skill_num,
	const char *map1,const char *map2,const char *map3,const char *map4)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x11c;
	WFIFOW(fd,2)=skill_num;
	memcpy(WFIFOP(fd, 4),map1,16);
	memcpy(WFIFOP(fd,20),map2,16);
	memcpy(WFIFOP(fd,36),map3,16);
	memcpy(WFIFOP(fd,52),map4,16);
	WFIFOSET(fd,packet_db[0x11c].len);
	return 0;
}
/*==========================================
 * メモ応答
 *------------------------------------------
 */
int clif_skill_memo(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0x11e;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x11e].len);
	return 0;
}
int clif_skill_teleportmessage(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x189;
	WFIFOW(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x189].len);
	return 0;
}

/*==========================================
 * モンスター情報
 *------------------------------------------
 */
int clif_skill_estimation(struct map_session_data *sd,struct block_list *dst)
{
	struct mob_data *md;
	unsigned char buf[64];
	int i;

	nullpo_retr(0, sd);
	nullpo_retr(0, dst);

	if(dst->type!=BL_MOB )
		return 0;
	if((md=(struct mob_data *)dst) == NULL)
		return 0;

	WBUFW(buf, 0)=0x18c;
	WBUFW(buf, 2)=mob_get_viewclass(md->class);
	WBUFW(buf, 4)=mob_db[md->class].lv;
	WBUFW(buf, 6)=mob_db[md->class].size;
	WBUFL(buf, 8)=md->hp;
	WBUFW(buf,12)=battle_get_def2(&md->bl);
	WBUFW(buf,14)=mob_db[md->class].race;
	WBUFW(buf,16)=battle_get_mdef2(&md->bl) - (mob_db[md->class].vit>>1);
	WBUFW(buf,18)=battle_get_elem_type(&md->bl);
	for(i=0;i<9;i++)
		WBUFB(buf,20+i)= battle_attr_fix(100,i+1,md->def_ele);

	if(sd->status.party_id>0)
		clif_send(buf,packet_db[0x18c].len,&sd->bl,PARTY_AREA);
	else{
		memcpy(WFIFOP(sd->fd,0),buf,packet_db[0x18c].len);
		WFIFOSET(sd->fd,packet_db[0x18c].len);
	}
	return 0;
}
/*==========================================
 * アイテム合成可能リスト
 *------------------------------------------
 */
int clif_skill_produce_mix_list(struct map_session_data *sd,int trigger)
{
	int i,c,view,fd;
	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd, 0)=0x18d;

	for(i=0,c=0;i<MAX_SKILL_PRODUCE_DB;i++){
		if( skill_can_produce_mix(sd,skill_produce_db[i].nameid,trigger) ){
			if((view = itemdb_viewid(skill_produce_db[i].nameid)) > 0)
				WFIFOW(fd,c*8+ 4)= view;
			else
				WFIFOW(fd,c*8+ 4)= skill_produce_db[i].nameid;
			WFIFOW(fd,c*8+ 6)= 0x0012;
			WFIFOL(fd,c*8+ 8)= sd->status.char_id;
			c++;
		}
	}
	WFIFOW(fd, 2)=c*8+8;
	WFIFOSET(fd,WFIFOW(fd,2));
	if(c > 0) sd->state.produce_flag = 1;
	return 0;
}

/*==========================================
 * 状態異常アイコン/メッセージ表示
 *------------------------------------------
 */
int clif_status_change(struct block_list *bl,int type,int flag)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	WBUFW(buf,0)=0x0196;
	WBUFW(buf,2)=type;
	WBUFL(buf,4)=bl->id;
	WBUFB(buf,8)=flag;
	clif_send(buf,packet_db[0x196].len,bl,AREA);
	return 0;
}

/*==========================================
 * Send message (modified by [Yor])
 *------------------------------------------
 */
int clif_displaymessage(const int fd, char* mes) 
{
	//Console [Wizputer]
	if ( fd == 0 )
	    printf("console: %s\n",mes);
    else {
	int len_mes = strlen(mes);

	if (len_mes > 0) { // don't send a void message (it's not displaying on the client chat). @help can send void line.
		WFIFOW(fd,0) = 0x8e;
		WFIFOW(fd,2) = 5 + len_mes; // 4 + len + NULL teminate
		memcpy(WFIFOP(fd,4), mes, len_mes + 1);
		WFIFOSET(fd, 5 + len_mes);
	}
    }    

	return 0;
}

/*==========================================
 * 天の声を送信する
 *------------------------------------------
 */
int clif_GMmessage(struct block_list *bl,char* mes,int len,int flag)
{
	unsigned char buf[len+16];
	//unsigned char *buf = ((len + 16) >= sizeof(lbuf)) ? malloc(len+16) : lbuf;
	int lp = (flag&0x10) ? 8 : 4;

	WBUFW(buf,0) = 0x9a;
	WBUFW(buf,2) = len+lp;
	WBUFL(buf,4) = 0x65756c62;
	memcpy(WBUFP(buf,lp), mes, len);
	flag&=0x07;
	clif_send(buf, WBUFW(buf,2), bl,
		(flag==1)? ALL_SAMEMAP:
		(flag==2)? AREA:
		(flag==3)? SELF:
		ALL_CLIENT);
        //if (buf != lbuf)
        //    free(buf);
	return 0;
}

/*==========================================
 * HPSP回復エフェクトを送信する
 *------------------------------------------
 */
int clif_heal(int fd,int type,int val)
{
	WFIFOW(fd,0)=0x13d;
	WFIFOW(fd,2)=type;
	WFIFOW(fd,4)=val;
	WFIFOSET(fd,packet_db[0x13d].len);

	return 0;
}

/*==========================================
 * 復活する
 *------------------------------------------
 */
int clif_resurrection(struct block_list *bl,int type)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	if(bl->type==BL_PC) { // disguises [Valaris]
		struct map_session_data *sd=((struct map_session_data *)bl);
		if(sd && sd->disguise > 23 && sd->disguise < 4001)
			clif_spawnpc(sd);
	}

	WBUFW(buf,0)=0x148;
	WBUFL(buf,2)=bl->id;
	WBUFW(buf,6)=type;

	clif_send(buf,packet_db[0x148].len,bl,type==1 ? AREA : AREA_WOS);

	return 0;
}

/*==========================================
 * PVP実装？（仮）
 *------------------------------------------
 */
int clif_set0199(int fd,int type)
{
	WFIFOW(fd,0)=0x199;
	WFIFOW(fd,2)=type;
	WFIFOSET(fd,packet_db[0x199].len);

	return 0;
}

/*==========================================
 * PVP実装？(仮)
 *------------------------------------------
 */
int clif_pvpset(struct map_session_data *sd,int pvprank,int pvpnum,int type)
{
	nullpo_retr(0, sd);

	if(map[sd->bl.m].flag.nopvp)
		return 0;

	if(type == 2) {
		WFIFOW(sd->fd,0) = 0x19a;
		WFIFOL(sd->fd,2) = sd->bl.id;
		if(pvprank<=0)
			pc_calc_pvprank(sd);
		WFIFOL(sd->fd,6) = pvprank;
		WFIFOL(sd->fd,10) = pvpnum;
		WFIFOSET(sd->fd,packet_db[0x19a].len);
	} else {
		char buf[32];

		WBUFW(buf,0) = 0x19a;
		WBUFL(buf,2) = sd->bl.id;
		if(sd->status.option&0x46)
			WBUFL(buf,6) = -1;
		else
			if(pvprank<=0)
				pc_calc_pvprank(sd);
			WBUFL(buf,6) = pvprank;
		WBUFL(buf,10) = pvpnum;
		if(!type)
			clif_send(buf,packet_db[0x19a].len,&sd->bl,AREA);
		else
			clif_send(buf,packet_db[0x19a].len,&sd->bl,ALL_SAMEMAP);
	}

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_send0199(int map,int type)
{
	struct block_list bl;
	char buf[16];

	bl.m = map;
	WBUFW(buf,0)=0x199;
	WBUFW(buf,2)=type;
	clif_send(buf,packet_db[0x199].len,&bl,ALL_SAMEMAP);

	return 0;
}

/*==========================================
 * 精錬エフェクトを送信する
 *------------------------------------------
 */
int clif_refine(int fd,struct map_session_data *sd,int fail,int index,int val)
{
	WFIFOW(fd,0)=0x188;
	WFIFOW(fd,2)=fail;
	WFIFOW(fd,4)=index+2;
	WFIFOW(fd,6)=val;
	WFIFOSET(fd,packet_db[0x188].len);

	return 0;
}

/*==========================================
 * Wisp/page is transmitted to the destination player
 *------------------------------------------
 */
int clif_wis_message(int fd, char *nick, char *mes, int mes_len) // R 0097 <len>.w <nick>.24B <message>.?B
{
	WFIFOW(fd,0)=0x97;
	WFIFOW(fd,2)=mes_len +24+ 4;
	memcpy(WFIFOP(fd,4),nick,24);
	memcpy(WFIFOP(fd,28),mes,mes_len);
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

/*==========================================
 * The transmission result of Wisp/page is transmitted to the source player
 *------------------------------------------
 */
int clif_wis_end(int fd, int flag) // R 0098 <type>.B: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
{ 
	WFIFOW(fd,0)=0x98;
	WFIFOW(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x98].len);
	return 0;
}

/*==========================================
 * キャラID名前引き結果を送信する
 *------------------------------------------
 */
int clif_solved_charname(struct map_session_data *sd,int char_id)
{
	char *p= map_charid2nick(char_id);
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	if(p!=NULL){
		WFIFOW(fd,0)=0x194;
		WFIFOL(fd,2)=char_id;
		memcpy(WFIFOP(fd,6), p,24 );
		WFIFOSET(fd,packet_db[0x194].len);
	}else{
		map_reqchariddb(sd,char_id);
		chrif_searchcharid(char_id);
	}
	return 0;
}

/*==========================================
 * カードの挿入可能リストを返す
 *------------------------------------------
 */
int clif_use_card(struct map_session_data *sd,int idx)
{
	nullpo_retr(0, sd);

	if(sd->inventory_data[idx]) {
		int i,c;
		int ep=sd->inventory_data[idx]->equip;
		int fd=sd->fd;
		WFIFOW(fd,0)=0x017b;

		for(i=c=0;i<MAX_INVENTORY;i++){
			int j;

			if(sd->inventory_data[i] == NULL)
				continue;
			if(sd->inventory_data[i]->type!=4 && sd->inventory_data[i]->type!=5)	// 武器防具じゃない
				continue;
			if(sd->status.inventory[i].card[0]==0x00ff)	// 製造武器
				continue;
			if(sd->status.inventory[i].card[0]==(short)0xff00 || sd->status.inventory[i].card[0]==0x00fe)
				continue;
			if(sd->status.inventory[i].identify==0 )	// 未鑑定
				continue;

			if((sd->inventory_data[i]->equip&ep)==0)	// 装備個所が違う
				continue;
			if(sd->inventory_data[i]->type==4 && ep==32)	// 盾カードと両手武器
				continue;

			for(j=0;j<sd->inventory_data[i]->slot;j++){
				if( sd->status.inventory[i].card[j]==0 )
					break;
			}
			if(j==sd->inventory_data[i]->slot)	// すでにカードが一杯
				continue;

			WFIFOW(fd,4+c*2)=i+2;
			c++;
		}
		WFIFOW(fd,2)=4+c*2;
		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return 0;
}
/*==========================================
 * カードの挿入終了
 *------------------------------------------
 */
int clif_insert_card(struct map_session_data *sd,int idx_equip,int idx_card,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x17d;
	WFIFOW(fd,2)=idx_equip+2;
	WFIFOW(fd,4)=idx_card+2;
	WFIFOB(fd,6)=flag;
	WFIFOSET(fd,packet_db[0x17d].len);
	return 0;
}

/*==========================================
 * 鑑定可能アイテムリスト送信
 *------------------------------------------
 */
int clif_item_identify_list(struct map_session_data *sd)
{
	int i,c;
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0x177;
	for(i=c=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].identify!=1){
			WFIFOW(fd,c*2+4)=i+2;
			c++;
		}
	}
	if(c > 0) {
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * 鑑定結果
 *------------------------------------------
 */
int clif_item_identified(struct map_session_data *sd,int idx,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd, 0)=0x179;
	WFIFOW(fd, 2)=idx+2;
	WFIFOB(fd, 4)=flag;
	WFIFOSET(fd,packet_db[0x179].len);
	return 0;
}

/*==========================================
 * 修理可能アイテムリスト送信
 * ※実際のパケットがわからないので動作しません
 *------------------------------------------
 */
int clif_item_repair_list(struct map_session_data *sd)
{
	int i,c;
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0x0;
	for(i=c=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].nameid > 0 && sd->status.inventory[i].attribute!=0){
			WFIFOW(fd,c*2+4)=i+2;
			c++;
		}
	}
	if(c > 0) {
		WFIFOW(fd,2)=c*2+4;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * アイテムによる一時的なスキル効果
 *------------------------------------------
 */
int clif_item_skill(struct map_session_data *sd,int skillid,int skilllv,const char *name)
{
	int range,fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd, 0)=0x147;
	WFIFOW(fd, 2)=skillid;
	WFIFOW(fd, 4)=skill_get_inf(skillid);
	WFIFOW(fd, 6)=0;
	WFIFOW(fd, 8)=skilllv;
	WFIFOW(fd,10)=skill_get_sp(skillid,skilllv);
	range = skill_get_range(skillid,skilllv);
	if(range < 0)
		range = battle_get_range(&sd->bl) - (range + 1);
	WFIFOW(fd,12)=range;
	memcpy(WFIFOP(fd,14),name,24);
	WFIFOB(fd,38)=0;
	WFIFOSET(fd,packet_db[0x147].len);
	return 0;
}

/*==========================================
 * カートにアイテム追加
 *------------------------------------------
 */
int clif_cart_additem(struct map_session_data *sd,int n,int amount,int fail)
{
	int view,j,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf=WFIFOP(fd,0);
	if(n<0 || n>=MAX_CART || sd->status.cart[n].nameid<=0)
		return 1;

	WBUFW(buf,0)=0x124;
	WBUFW(buf,2)=n+2;
	WBUFL(buf,4)=amount;
	if((view = itemdb_viewid(sd->status.cart[n].nameid)) > 0)
		WBUFW(buf,8)=view;
	else
		WBUFW(buf,8)=sd->status.cart[n].nameid;
	WBUFB(buf,10)=sd->status.cart[n].identify;
	WBUFB(buf,11)=sd->status.cart[n].attribute;
	WBUFB(buf,12)=sd->status.cart[n].refine;
	if(sd->status.cart[n].card[0]==0x00ff || sd->status.cart[n].card[0]==0x00fe || sd->status.cart[n].card[0]==(short)0xff00) {
		WBUFW(buf,13)=sd->status.cart[n].card[0];
		WBUFW(buf,15)=sd->status.cart[n].card[1];
		WBUFW(buf,17)=sd->status.cart[n].card[2];
		WBUFW(buf,19)=sd->status.cart[n].card[3];
	} else {
		if(sd->status.cart[n].card[0] > 0 && (j=itemdb_viewid(sd->status.cart[n].card[0])) > 0)
			WBUFW(buf,13)= j;
		else
			WBUFW(buf,13)= sd->status.cart[n].card[0];
		if(sd->status.cart[n].card[1] > 0 && (j=itemdb_viewid(sd->status.cart[n].card[1])) > 0)
			WBUFW(buf,15)= j;
		else
			WBUFW(buf,15)= sd->status.cart[n].card[1];
		if(sd->status.cart[n].card[2] > 0 && (j=itemdb_viewid(sd->status.cart[n].card[2])) > 0)
			WBUFW(buf,17)= j;
		else
			WBUFW(buf,17)= sd->status.cart[n].card[2];
		if(sd->status.cart[n].card[3] > 0 && (j=itemdb_viewid(sd->status.cart[n].card[3])) > 0)
			WBUFW(buf,19)= j;
		else
			WBUFW(buf,19)= sd->status.cart[n].card[3];
	}
	WFIFOSET(fd,packet_db[0x124].len);
	return 0;
}

/*==========================================
 * カートからアイテム削除
 *------------------------------------------
 */
int clif_cart_delitem(struct map_session_data *sd,int n,int amount)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;

	WFIFOW(fd,0)=0x125;
	WFIFOW(fd,2)=n+2;
	WFIFOL(fd,4)=amount;

	WFIFOSET(fd,packet_db[0x125].len);

	return 0;
}

/*==========================================
 * カートのアイテムリスト
 *------------------------------------------
 */
int clif_cart_itemlist(struct map_session_data *sd)
{
	struct item_data *id;
	int i,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf = WFIFOP(fd,0);
#if PACKETVER < 5
	WBUFW(buf,0)=0x123;
	for(i=0,n=0;i<MAX_CART;i++){
		if(sd->status.cart[i].nameid<=0)
			continue;
		id = itemdb_search(sd->status.cart[i].nameid);
		if(itemdb_isequip2(id))
			continue;
		WBUFW(buf,n*10+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*10+6)=id->view_id;
		else
			WBUFW(buf,n*10+6)=sd->status.cart[i].nameid;
		WBUFB(buf,n*10+8)=id->type;
		WBUFB(buf,n*10+9)=sd->status.cart[i].identify;
		WBUFW(buf,n*10+10)=sd->status.cart[i].amount;
		WBUFW(buf,n*10+12)=0;
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*10;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#else
	WBUFW(buf,0)=0x1ef;
	for(i=0,n=0;i<MAX_CART;i++){
		if(sd->status.cart[i].nameid<=0)
			continue;
		id = itemdb_search(sd->status.cart[i].nameid);
		if(itemdb_isequip2(id))
			continue;
		WBUFW(buf,n*18+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*18+6)=id->view_id;
		else
			WBUFW(buf,n*18+6)=sd->status.cart[i].nameid;
		WBUFB(buf,n*18+8)=id->type;
		WBUFB(buf,n*18+9)=sd->status.cart[i].identify;
		WBUFW(buf,n*18+10)=sd->status.cart[i].amount;
		WBUFW(buf,n*18+12)=0;
		WBUFW(buf,n*18+14)=sd->status.cart[i].card[0];
		WBUFW(buf,n*18+16)=sd->status.cart[i].card[1];
		WBUFW(buf,n*18+18)=sd->status.cart[i].card[2];
		WBUFW(buf,n*18+20)=sd->status.cart[i].card[3];
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*18;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
#endif
	return 0;
}

/*==========================================
 * カートの装備品リスト
 *------------------------------------------
 */
int clif_cart_equiplist(struct map_session_data *sd)
{
	struct item_data *id;
	int i,j,n,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf = WFIFOP(fd,0);

	WBUFW(buf,0)=0x122;
	for(i=0,n=0;i<MAX_INVENTORY;i++){
		if(sd->status.cart[i].nameid<=0)
			continue;
		id = itemdb_search(sd->status.cart[i].nameid);
		if(!itemdb_isequip2(id))
			continue;
		WBUFW(buf,n*20+4)=i+2;
		if(id->view_id > 0)
			WBUFW(buf,n*20+6)=id->view_id;
		else
			WBUFW(buf,n*20+6)=sd->status.cart[i].nameid;
		WBUFB(buf,n*20+8)=id->type;
		WBUFB(buf,n*20+9)=sd->status.cart[i].identify;
		WBUFW(buf,n*20+10)=id->equip;
		WBUFW(buf,n*20+12)=sd->status.cart[i].equip;
		WBUFB(buf,n*20+14)=sd->status.cart[i].attribute;
		WBUFB(buf,n*20+15)=sd->status.cart[i].refine;
		if(sd->status.cart[i].card[0]==0x00ff || sd->status.cart[i].card[0]==0x00fe || sd->status.cart[i].card[0]==(short)0xff00) {
			WBUFW(buf,n*20+16)=sd->status.cart[i].card[0];
			WBUFW(buf,n*20+18)=sd->status.cart[i].card[1];
			WBUFW(buf,n*20+20)=sd->status.cart[i].card[2];
			WBUFW(buf,n*20+22)=sd->status.cart[i].card[3];
		} else {
			if(sd->status.cart[i].card[0] > 0 && (j=itemdb_viewid(sd->status.cart[i].card[0])) > 0)
				WBUFW(buf,n*20+16)= j;
			else
				WBUFW(buf,n*20+16)= sd->status.cart[i].card[0];
			if(sd->status.cart[i].card[1] > 0 && (j=itemdb_viewid(sd->status.cart[i].card[1])) > 0)
				WBUFW(buf,n*20+18)= j;
			else
				WBUFW(buf,n*20+18)= sd->status.cart[i].card[1];
			if(sd->status.cart[i].card[2] > 0 && (j=itemdb_viewid(sd->status.cart[i].card[2])) > 0)
				WBUFW(buf,n*20+20)= j;
			else
				WBUFW(buf,n*20+20)= sd->status.cart[i].card[2];
			if(sd->status.cart[i].card[3] > 0 && (j=itemdb_viewid(sd->status.cart[i].card[3])) > 0)
				WBUFW(buf,n*20+22)= j;
			else
				WBUFW(buf,n*20+22)= sd->status.cart[i].card[3];
		}
		n++;
	}
	if(n){
		WBUFW(buf,2)=4+n*20;
		WFIFOSET(fd,WFIFOW(fd,2));
	}
	return 0;
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
int clif_openvendingreq(struct map_session_data *sd,int num)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x12d;
	WFIFOW(fd,2)=num;
	WFIFOSET(fd,packet_db[0x12d].len);

	return 0;
}

/*==========================================
 * 露店看板表示
 *------------------------------------------
 */
int clif_showvendingboard(struct block_list* bl,char *message,int fd)
{
	unsigned char buf[128];

	nullpo_retr(0, bl);

	WBUFW(buf,0)=0x131;
	WBUFL(buf,2)=bl->id;
	strncpy(WBUFP(buf,6),message,80);
	if(fd){
		memcpy(WFIFOP(fd,0),buf,packet_db[0x131].len);
		WFIFOSET(fd,packet_db[0x131].len);
	}else{
		clif_send(buf,packet_db[0x131].len,bl,AREA_WOS);
	}
	return 0;
}

/*==========================================
 * 露店看板消去
 *------------------------------------------
 */
int clif_closevendingboard(struct block_list* bl,int fd)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	WBUFW(buf,0)=0x132;
	WBUFL(buf,2)=bl->id;
	if(fd){
		memcpy(WFIFOP(fd,0),buf,packet_db[0x132].len);
		WFIFOSET(fd,packet_db[0x132].len);
	}else{
		clif_send(buf,packet_db[0x132].len,bl,AREA_WOS);
	}

	return 0;
}
/*==========================================
 * 露店アイテムリスト
 *------------------------------------------
 */
int clif_vendinglist(struct map_session_data *sd,int id,struct vending *vending)
{
	struct item_data *data;
	int i,j,n,index,fd;
	struct map_session_data *vsd;
	unsigned char *buf;

	nullpo_retr(0, sd);
	nullpo_retr(0, vending);
	nullpo_retr(0, vsd=map_id2sd(id));

	fd=sd->fd;
	buf = WFIFOP(fd,0);
	WBUFW(buf,0)=0x133;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<vsd->vend_num;i++){
		if(vending[i].amount<=0)
			continue;
		WBUFL(buf,8+n*22)=vending[i].value;
		WBUFW(buf,12+n*22)=vending[i].amount;
		WBUFW(buf,14+n*22)=(index=vending[i].index)+2;
		if(vsd->status.cart[index].nameid <= 0 || vsd->status.cart[index].amount <= 0)
			continue;
		data = itemdb_search(vsd->status.cart[index].nameid);
		WBUFB(buf,16+n*22)=data->type;
		if(data->view_id > 0)
			WBUFW(buf,17+n*22)=data->view_id;
		else
			WBUFW(buf,17+n*22)=vsd->status.cart[index].nameid;
		WBUFB(buf,19+n*22)=vsd->status.cart[index].identify;
		WBUFB(buf,20+n*22)=vsd->status.cart[index].attribute;
		WBUFB(buf,21+n*22)=vsd->status.cart[index].refine;
		if(vsd->status.cart[index].card[0]==0x00ff || vsd->status.cart[index].card[0]==0x00fe || vsd->status.cart[index].card[0]==(short)0xff00) {
			WBUFW(buf,22+n*22)=vsd->status.cart[index].card[0];
			WBUFW(buf,24+n*22)=vsd->status.cart[index].card[1];
			WBUFW(buf,26+n*22)=vsd->status.cart[index].card[2];
			WBUFW(buf,28+n*22)=vsd->status.cart[index].card[3];
		} else {
			if(vsd->status.cart[index].card[0] > 0 && (j=itemdb_viewid(vsd->status.cart[index].card[0])) > 0)
				WBUFW(buf,22+n*22)= j;
			else
				WBUFW(buf,22+n*22)= vsd->status.cart[index].card[0];
			if(vsd->status.cart[index].card[1] > 0 && (j=itemdb_viewid(vsd->status.cart[index].card[1])) > 0)
				WBUFW(buf,24+n*22)= j;
			else
				WBUFW(buf,24+n*22)= vsd->status.cart[index].card[1];
			if(vsd->status.cart[index].card[2] > 0 && (j=itemdb_viewid(vsd->status.cart[index].card[2])) > 0)
				WBUFW(buf,26+n*22)= j;
			else
				WBUFW(buf,26+n*22)= vsd->status.cart[index].card[2];
			if(vsd->status.cart[index].card[3] > 0 && (j=itemdb_viewid(vsd->status.cart[index].card[3])) > 0)
				WBUFW(buf,28+n*22)= j;
			else
				WBUFW(buf,28+n*22)= vsd->status.cart[index].card[3];
		}
		n++;
	}
	if(n > 0){
		WBUFW(buf,2)=8+n*22;
		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return 0;
}

/*==========================================
 * 露店アイテム購入失敗
 *------------------------------------------
*/
int clif_buyvending(struct map_session_data *sd,int index,int amount,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x135;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOB(fd,6)=fail;
	WFIFOSET(fd,packet_db[0x135].len);

	return 0;
}

/*==========================================
 * 露店開設成功
 *------------------------------------------
*/
int clif_openvending(struct map_session_data *sd,int id,struct vending *vending)
{
	struct item_data *data;
	int i,j,n,index,fd;
	unsigned char *buf;

	nullpo_retr(0, sd);

	fd=sd->fd;
	buf = WFIFOP(fd,0);

	WBUFW(buf,0)=0x136;
	WBUFL(buf,4)=id;
	for(i=0,n=0;i<sd->vend_num;i++){
		if (sd->vend_num > 2+pc_checkskill(sd,MC_VENDING)) return 0;
		WBUFL(buf,8+n*22)=vending[i].value;
		WBUFW(buf,12+n*22)=(index=vending[i].index)+2;
		WBUFW(buf,14+n*22)=vending[i].amount;
		if(sd->status.cart[index].nameid <= 0 || sd->status.cart[index].amount <= 0 || sd->status.cart[index].identify==0 ||
			sd->status.cart[index].attribute==1) // Prevent unidentified and broken items from being sold [Valaris]
			continue;
		data = itemdb_search(sd->status.cart[index].nameid);
		WBUFB(buf,16+n*22)=data->type;
		if(data->view_id > 0)
			WBUFW(buf,17+n*22)=data->view_id;
		else
			WBUFW(buf,17+n*22)=sd->status.cart[index].nameid;
		WBUFB(buf,19+n*22)=sd->status.cart[index].identify;
		WBUFB(buf,20+n*22)=sd->status.cart[index].attribute;
		WBUFB(buf,21+n*22)=sd->status.cart[index].refine;
		if(sd->status.cart[index].card[0]==0x00ff || sd->status.cart[index].card[0]==0x00fe || sd->status.cart[index].card[0]==(short)0xff00) {
			WBUFW(buf,22+n*22)=sd->status.cart[index].card[0];
			WBUFW(buf,24+n*22)=sd->status.cart[index].card[1];
			WBUFW(buf,26+n*22)=sd->status.cart[index].card[2];
			WBUFW(buf,28+n*22)=sd->status.cart[index].card[3];
		} else {
			if(sd->status.cart[index].card[0] > 0 && (j=itemdb_viewid(sd->status.cart[index].card[0])) > 0)
				WBUFW(buf,22+n*22)= j;
			else
				WBUFW(buf,22+n*22)= sd->status.cart[index].card[0];
			if(sd->status.cart[index].card[1] > 0 && (j=itemdb_viewid(sd->status.cart[index].card[1])) > 0)
				WBUFW(buf,24+n*22)= j;
			else
				WBUFW(buf,24+n*22)= sd->status.cart[index].card[1];
			if(sd->status.cart[index].card[2] > 0 && (j=itemdb_viewid(sd->status.cart[index].card[2])) > 0)
				WBUFW(buf,26+n*22)= j;
			else
				WBUFW(buf,26+n*22)= sd->status.cart[index].card[2];
			if(sd->status.cart[index].card[3] > 0 && (j=itemdb_viewid(sd->status.cart[index].card[3])) > 0)
				WBUFW(buf,28+n*22)= j;
			else
				WBUFW(buf,28+n*22)= sd->status.cart[index].card[3];
		}
		n++;
	}
	if(n > 0){
		WBUFW(buf,2)=8+n*22;
		WFIFOSET(fd,WFIFOW(fd,2));
	}

	return n;
}

/*==========================================
 * 露店アイテム販売報告
 *------------------------------------------
*/
int clif_vendingreport(struct map_session_data *sd,int index,int amount)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x137;
	WFIFOW(fd,2)=index+2;
	WFIFOW(fd,4)=amount;
	WFIFOSET(fd,packet_db[0x137].len);

	return 0;
}

/*==========================================
 * パーティ作成完了
 *------------------------------------------
 */
int clif_party_created(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xfa;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_db[0xfa].len);
	return 0;
}
/*==========================================
 * パーティ情報送信
 *------------------------------------------
 */
int clif_party_info(struct party *p,int fd)
{
	unsigned char buf[1024];
	int i,c;
	struct map_session_data *sd=NULL;

	nullpo_retr(0, p);

	WBUFW(buf,0)=0xfb;
	memcpy(WBUFP(buf,4),p->name,24);
	for(i=c=0;i<MAX_PARTY;i++){
		struct party_member *m=&p->member[i];
		if(m->account_id>0){
			if(sd==NULL) sd=m->sd;
			WBUFL(buf,28+c*46)=m->account_id;
			memcpy(WBUFP(buf,28+c*46+ 4),m->name,24);
			memcpy(WBUFP(buf,28+c*46+28),m->map,16);
			WBUFB(buf,28+c*46+44)=(m->leader)?0:1;
			WBUFB(buf,28+c*46+45)=(m->online)?0:1;
			c++;
		}
	}
	WBUFW(buf,2)=28+c*46;
	if(fd>=0){	// fdが設定されてるならそれに送る
		memcpy(WFIFOP(fd,0),buf,WBUFW(buf,2));
		WFIFOSET(fd,WFIFOW(fd,2));
		return 9;
	}
	if(sd!=NULL)
		clif_send(buf,WBUFW(buf,2),&sd->bl,PARTY);
	return 0;
}
/*==========================================
 * パーティ勧誘
 *------------------------------------------
 */
int clif_party_invite(struct map_session_data *sd,struct map_session_data *tsd)
{
	int fd;
	struct party *p;

	nullpo_retr(0, sd);
	nullpo_retr(0, tsd);

	fd=tsd->fd;

	if( (p=party_search(sd->status.party_id))==NULL )
		return 0;

	WFIFOW(fd,0)=0xfe;
	WFIFOL(fd,2)=sd->status.account_id;
	memcpy(WFIFOP(fd,6),p->name,24);
	WFIFOSET(fd,packet_db[0xfe].len);
	return 0;
}

/*==========================================
 * パーティ勧誘結果
 *------------------------------------------
 */
int clif_party_inviteack(struct map_session_data *sd,char *nick,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xfd;
	memcpy(WFIFOP(fd,2),nick,24);
	WFIFOB(fd,26)=flag;
	WFIFOSET(fd,packet_db[0xfd].len);
	return 0;
}

/*==========================================
 * パーティ設定送信
 * flag & 0x001=exp変更ミス
 *        0x010=item変更ミス
 *        0x100=一人にのみ送信
 *------------------------------------------
 */
int clif_party_option(struct party *p,struct map_session_data *sd,int flag)
{
	unsigned char buf[16];

	nullpo_retr(0, p);

//	if(battle_config.etc_log)
//		printf("clif_party_option: %d %d %d\n",p->exp,p->item,flag);
	if(sd==NULL && flag==0){
		int i;
		for(i=0;i<MAX_PARTY;i++)
			if((sd=map_id2sd(p->member[i].account_id))!=NULL)
				break;
	}
	if(sd==NULL)
		return 0;
	WBUFW(buf,0)=0x101;
	WBUFW(buf,2)=((flag&0x01)?2:p->exp);
	WBUFW(buf,4)=((flag&0x10)?2:p->item);
	if(flag==0)
		clif_send(buf,packet_db[0x101].len,&sd->bl,PARTY);
	else {
		memcpy(WFIFOP(sd->fd,0),buf,packet_db[0x101].len);
		WFIFOSET(sd->fd,packet_db[0x101].len);
	}
	return 0;
}
/*==========================================
 * パーティ脱退（脱退前に呼ぶこと）
 *------------------------------------------
 */
int clif_party_leaved(struct party *p,struct map_session_data *sd,int account_id,char *name,int flag)
{
	unsigned char buf[64];
	int i;

	nullpo_retr(0, p);

	WBUFW(buf,0)=0x105;
	WBUFL(buf,2)=account_id;
	memcpy(WBUFP(buf,6),name,24);
	WBUFB(buf,30)=flag&0x0f;

	if((flag&0xf0)==0){
		if(sd==NULL)
			for(i=0;i<MAX_PARTY;i++)
				if((sd=p->member[i].sd)!=NULL)
					break;
		if(sd!=NULL)
			clif_send(buf,packet_db[0x105].len,&sd->bl,PARTY);
	}else if(sd!=NULL){
		memcpy(WFIFOP(sd->fd,0),buf,packet_db[0x105].len);
		WFIFOSET(sd->fd,packet_db[0x105].len);
	}
	return 0;
}
/*==========================================
 * パーティメッセージ送信
 *------------------------------------------
 */
int clif_party_message(struct party *p,int account_id,char *mes,int len)
{
	struct map_session_data *sd;
	int i;

	nullpo_retr(0, p);

	for(i=0;i<MAX_PARTY;i++){
		if((sd=p->member[i].sd)!=NULL)
			break;
	}
	if(sd!=NULL){
		unsigned char buf[1024];
		WBUFW(buf,0)=0x109;
		WBUFW(buf,2)=len+8;
		WBUFL(buf,4)=account_id;
		memcpy(WBUFP(buf,8),mes,len);
		clif_send(buf,len+8,&sd->bl,PARTY);
	}
	return 0;
}
/*==========================================
 * パーティ座標通知
 *------------------------------------------
 */
int clif_party_xy(struct party *p,struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x107;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=sd->bl.x;
	WBUFW(buf,8)=sd->bl.y;
	clif_send(buf,packet_db[0x107].len,&sd->bl,PARTY_SAMEMAP_WOS);
//	if(battle_config.etc_log)
//		printf("clif_party_xy %d\n",sd->status.account_id);
	return 0;
}
/*==========================================
 * パーティHP通知
 *------------------------------------------
 */
int clif_party_hp(struct party *p,struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x106;
	WBUFL(buf,2)=sd->status.account_id;
	WBUFW(buf,6)=(sd->status.hp > 0x7fff)? 0x7fff:sd->status.hp;
	WBUFW(buf,8)=(sd->status.max_hp > 0x7fff)? 0x7fff:sd->status.max_hp;
	clif_send(buf,packet_db[0x106].len,&sd->bl,PARTY_AREA_WOS);
//	if(battle_config.etc_log)
//		printf("clif_party_hp %d\n",sd->status.account_id);
	return 0;
}
/*==========================================
 * パーティ場所移動（未使用）
 *------------------------------------------
 */
int clif_party_move(struct party *p,struct map_session_data *sd,int online)
{
	unsigned char buf[128];

	nullpo_retr(0, sd);
	nullpo_retr(0, p);

	WBUFW(buf, 0)=0x104;
	WBUFL(buf, 2)=sd->status.account_id;
	WBUFL(buf, 6)=0;
	WBUFW(buf,10)=sd->bl.x;
	WBUFW(buf,12)=sd->bl.y;
	WBUFB(buf,14)=!online;
	memcpy(WBUFP(buf,15),p->name,24);
	memcpy(WBUFP(buf,39),sd->status.name,24);
	memcpy(WBUFP(buf,63),map[sd->bl.m].name,16);
	clif_send(buf,packet_db[0x104].len,&sd->bl,PARTY);
	return 0;
}
/*==========================================
 * 攻撃するために移動が必要
 *------------------------------------------
 */
int clif_movetoattack(struct map_session_data *sd,struct block_list *bl)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, bl);

	fd=sd->fd;
	WFIFOW(fd, 0)=0x139;
	WFIFOL(fd, 2)=bl->id;
	WFIFOW(fd, 6)=bl->x;
	WFIFOW(fd, 8)=bl->y;
	WFIFOW(fd,10)=sd->bl.x;
	WFIFOW(fd,12)=sd->bl.y;
	WFIFOW(fd,14)=sd->attackrange;
	WFIFOSET(fd,packet_db[0x139].len);
	return 0;
}
/*==========================================
 * 製造エフェクト
 *------------------------------------------
 */
int clif_produceeffect(struct map_session_data *sd,int flag,int nameid)
{
	int view,fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	// 名前の登録と送信を先にしておく
	if( map_charid2nick(sd->status.char_id)==NULL )
		map_addchariddb(sd->status.char_id,sd->status.name);
	clif_solved_charname(sd,sd->status.char_id);

	WFIFOW(fd, 0)=0x18f;
	WFIFOW(fd, 2)=flag;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd, 4)=view;
	else
		WFIFOW(fd, 4)=nameid;
	WFIFOSET(fd,packet_db[0x18f].len);
	return 0;
}

// pet
int clif_catch_process(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x19e;
	WFIFOSET(fd,packet_db[0x19e].len);

	return 0;
}

int clif_pet_rulet(struct map_session_data *sd,int data)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1a0;
	WFIFOB(fd,2)=data;
	WFIFOSET(fd,packet_db[0x1a0].len);

	return 0;
}

/*==========================================
 * pet卵リスト作成
 *------------------------------------------
 */
int clif_sendegg(struct map_session_data *sd)
{
	//R 01a6 <len>.w <index>.w*
	int i,n=0,fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1a6;
	if(sd->status.pet_id <= 0) {
		for(i=0,n=0;i<MAX_INVENTORY;i++){
			if(sd->status.inventory[i].nameid<=0 || sd->inventory_data[i] == NULL ||
			   sd->inventory_data[i]->type!=7 ||
		  	 sd->status.inventory[i].amount<=0)
				continue;
			WFIFOW(fd,n*2+4)=i+2;
			n++;
		}
	}
	WFIFOW(fd,2)=4+n*2;
	WFIFOSET(fd,WFIFOW(fd,2));

	return 0;
}

int clif_send_petdata(struct map_session_data *sd,int type,int param)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1a4;
	WFIFOB(fd,2)=type;
	WFIFOL(fd,3)=sd->pd->bl.id;
	WFIFOL(fd,7)=param;
	WFIFOSET(fd,packet_db[0x1a4].len);

	return 0;
}

int clif_send_petstatus(struct map_session_data *sd)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1a2;
	memcpy(WFIFOP(fd,2),sd->pet.name,24);
	WFIFOB(fd,26)=(battle_config.pet_rename == 1)? 0:sd->pet.rename_flag;
	WFIFOW(fd,27)=sd->pet.level;
	WFIFOW(fd,29)=sd->pet.hungry;
	WFIFOW(fd,31)=sd->pet.intimate;
	WFIFOW(fd,33)=sd->pet.equip;
	WFIFOSET(fd,packet_db[0x1a2].len);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_pet_emotion(struct pet_data *pd,int param)
{
	unsigned char buf[16];
	struct map_session_data *sd;

	nullpo_retr(0, pd);
	nullpo_retr(0, sd = pd->msd);

	memset(buf,0,packet_db[0x1aa].len);

	WBUFW(buf,0)=0x1aa;
	WBUFL(buf,2)=pd->bl.id;
	if(param >= 100 && sd->petDB->talk_convert_class) {
		if(sd->petDB->talk_convert_class < 0)
			return 0;
		else if(sd->petDB->talk_convert_class > 0) {
			param -= (pd->class - 100)*100;
			param += (sd->petDB->talk_convert_class - 100)*100;
		}
	}
	WBUFL(buf,6)=param;

	clif_send(buf,packet_db[0x1aa].len,&pd->bl,AREA);

	return 0;
}

int clif_pet_performance(struct block_list *bl,int param)
{
	unsigned char buf[16];

	nullpo_retr(0, bl);

	memset(buf,0,packet_db[0x1a4].len);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=4;
	WBUFL(buf,3)=bl->id;
	WBUFL(buf,7)=param;

	clif_send(buf,packet_db[0x1a4].len,bl,AREA);

	return 0;
}

int clif_pet_equip(struct pet_data *pd,int nameid)
{
	unsigned char buf[16];
	int view;

	nullpo_retr(0, pd);

	memset(buf,0,packet_db[0x1a4].len);

	WBUFW(buf,0)=0x1a4;
	WBUFB(buf,2)=3;
	WBUFL(buf,3)=pd->bl.id;
	if((view = itemdb_viewid(nameid)) > 0)
		WBUFL(buf,7)=view;
	else
		WBUFL(buf,7)=nameid;

	clif_send(buf,packet_db[0x1a4].len,&pd->bl,AREA);

	return 0;
}

int clif_pet_food(struct map_session_data *sd,int foodid,int fail)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1a3;
	WFIFOB(fd,2)=fail;
	WFIFOW(fd,3)=foodid;
	WFIFOSET(fd,packet_db[0x1a3].len);

	return 0;
}

/*==========================================
 * オートスペル リスト送信
 *------------------------------------------
 */
int clif_autospell(struct map_session_data *sd,int skilllv)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd, 0)=0x1cd;

	if(skilllv>0 && pc_checkskill(sd,MG_NAPALMBEAT)>0)
		WFIFOL(fd,2)= MG_NAPALMBEAT;
	else
		WFIFOL(fd,2)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_COLDBOLT)>0)
		WFIFOL(fd,6)= MG_COLDBOLT;
	else
		WFIFOL(fd,6)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_FIREBOLT)>0)
		WFIFOL(fd,10)= MG_FIREBOLT;
	else
		WFIFOL(fd,10)= 0x00000000;
	if(skilllv>1 && pc_checkskill(sd,MG_LIGHTNINGBOLT)>0)
		WFIFOL(fd,14)= MG_LIGHTNINGBOLT;
	else
		WFIFOL(fd,14)= 0x00000000;
	if(skilllv>4 && pc_checkskill(sd,MG_SOULSTRIKE)>0)
		WFIFOL(fd,18)= MG_SOULSTRIKE;
	else
		WFIFOL(fd,18)= 0x00000000;
	if(skilllv>7 && pc_checkskill(sd,MG_FIREBALL)>0)
		WFIFOL(fd,22)= MG_FIREBALL;
	else
		WFIFOL(fd,22)= 0x00000000;
	if(skilllv>9 && pc_checkskill(sd,MG_FROSTDIVER)>0)
		WFIFOL(fd,26)= MG_FROSTDIVER;
	else
		WFIFOL(fd,26)= 0x00000000;

	WFIFOSET(fd,packet_db[0x1cd].len);
	return 0;
}

/*==========================================
 * ディボーションの青い糸
 *------------------------------------------
 */
int clif_devotion(struct map_session_data *sd,int target)
{
	unsigned char buf[56];
	int n;

	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x1cf;
	WBUFL(buf,2)=sd->bl.id;
//	WBUFL(buf,6)=target;
	for(n=0;n<5;n++)
		WBUFL(buf,6+4*n)=sd->dev.val2[n];
//		WBUFL(buf,10+4*n)=0;
	WBUFB(buf,26)=8;
	WBUFB(buf,27)=0;

	clif_send(buf,packet_db[0x1cf].len,&sd->bl,AREA);
	return 0;
}

/*==========================================
 * 氣球
 *------------------------------------------
 */
int clif_spiritball(struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x1d0;
	WBUFL(buf,2)=sd->bl.id;
	WBUFW(buf,6)=sd->spiritball;
	clif_send(buf,packet_db[0x1d0].len,&sd->bl,AREA);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_combo_delay(struct block_list *bl,int wait)
{
	unsigned char buf[32];

	nullpo_retr(0, bl);

	WBUFW(buf,0)=0x1d2;
	WBUFL(buf,2)=bl->id;
	WBUFL(buf,6)=wait;
	clif_send(buf,packet_db[0x1d2].len,bl,AREA);

	return 0;
}
/*==========================================
 *白刃取り
 *------------------------------------------
 */
int clif_bladestop(struct block_list *src,struct block_list *dst,
	int bool)
{
	unsigned char buf[32];

	nullpo_retr(0, src);
	nullpo_retr(0, dst);

	WBUFW(buf,0)=0x1d1;
	WBUFL(buf,2)=src->id;
	WBUFL(buf,6)=dst->id;
	WBUFL(buf,10)=bool;

	clif_send(buf,packet_db[0x1d1].len,src,AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_changemapcell(int m,int x,int y,int cell_type,int type)
{
	struct block_list bl;
	char buf[32];

	bl.m = m;
	bl.x = x;
	bl.y = y;
	WBUFW(buf,0) = 0x192;
	WBUFW(buf,2) = x;
	WBUFW(buf,4) = y;
	WBUFW(buf,6) = cell_type;
	memcpy(WBUFP(buf,8),map[m].name,16);
	if(!type)
		clif_send(buf,packet_db[0x192].len,&bl,AREA);
	else
		clif_send(buf,packet_db[0x192].len,&bl,ALL_SAMEMAP);

	return 0;
}

/*==========================================
 * MVPエフェクト
 *------------------------------------------
 */
int clif_mvp_effect(struct map_session_data *sd)
{
	unsigned char buf[16];

	nullpo_retr(0, sd);

	WBUFW(buf,0)=0x10c;
	WBUFL(buf,2)=sd->bl.id;
	clif_send(buf,packet_db[0x10c].len,&sd->bl,AREA);
	return 0;
}
/*==========================================
 * MVPアイテム所得
 *------------------------------------------
 */
int clif_mvp_item(struct map_session_data *sd,int nameid)
{
	int view,fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x10a;
	if((view = itemdb_viewid(nameid)) > 0)
		WFIFOW(fd,2)=view;
	else
		WFIFOW(fd,2)=nameid;
	WFIFOSET(fd,packet_db[0x10a].len);
	return 0;
}
/*==========================================
 * MVP経験値所得
 *------------------------------------------
 */
int clif_mvp_exp(struct map_session_data *sd,int exp)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x10b;
	WFIFOL(fd,2)=exp;
	WFIFOSET(fd,packet_db[0x10b].len);
	return 0;
}

/*==========================================
 * ギルド作成可否通知
 *------------------------------------------
 */
int clif_guild_created(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x167;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x167].len);
	return 0;
}
/*==========================================
 * ギルド所属通知
 *------------------------------------------
 */
int clif_guild_belonginfo(struct map_session_data *sd,struct guild *g)
{
	int ps,fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, g);

	fd=sd->fd;
	ps=guild_getposition(sd,g);

	memset(WFIFOP(fd,0),0,packet_db[0x16c].len);
	WFIFOW(fd,0)=0x16c;
	WFIFOL(fd,2)=g->guild_id;
	WFIFOL(fd,6)=g->emblem_id;
	WFIFOL(fd,10)=g->position[ps].mode;
	memcpy(WFIFOP(fd,19),g->name,24);
	WFIFOSET(fd,packet_db[0x16c].len);
	return 0;
}
/*==========================================
 * ギルドメンバログイン通知
 *------------------------------------------
 */
int clif_guild_memberlogin_notice(struct guild *g,int idx,int flag)
{
	unsigned char buf[64];

	nullpo_retr(0, g);

	WBUFW(buf, 0)=0x16d;
	WBUFL(buf, 2)=g->member[idx].account_id;
	WBUFL(buf, 6)=g->member[idx].char_id;
	WBUFL(buf,10)=flag;
	if(g->member[idx].sd==NULL){
		struct map_session_data *sd=guild_getavailablesd(g);
		if(sd!=NULL)
			clif_send(buf,packet_db[0x16d].len,&sd->bl,GUILD);
	}else
		clif_send(buf,packet_db[0x16d].len,&g->member[idx].sd->bl,GUILD_WOS);
	return 0;
}
/*==========================================
 * ギルドマスター通知(14dへの応答)
 *------------------------------------------
 */
int clif_guild_masterormember(struct map_session_data *sd)
{
	int type=0x57,fd;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g!=NULL && strcmp(g->master,sd->status.name)==0)
		type=0xd7;
	WFIFOW(fd,0)=0x14e;
	WFIFOL(fd,2)=type;
	WFIFOSET(fd,packet_db[0x14e].len);
	return 0;
}
/*==========================================
 * Basic Info (Territories [Valaris])
 *------------------------------------------
 */
int clif_guild_basicinfo(struct map_session_data *sd)
{
	int fd,i,t=0;
	struct guild *g;
	struct guild_castle *gc=NULL;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;

	WFIFOW(fd, 0)=0x1b6;//0x150;
	WFIFOL(fd, 2)=g->guild_id;
	WFIFOL(fd, 6)=g->guild_lv;
	WFIFOL(fd,10)=g->connect_member;
	WFIFOL(fd,14)=g->max_member;
	WFIFOL(fd,18)=g->average_lv;
	WFIFOL(fd,22)=g->exp;
	WFIFOL(fd,26)=g->next_exp;
	WFIFOL(fd,30)=0;	// 上納
	WFIFOL(fd,34)=0;	// VW（性格の悪さ？：性向グラフ左右）
	WFIFOL(fd,38)=0;	// RF（正義の度合い？：性向グラフ上下）
	WFIFOL(fd,42)=0;	// 人数？
	memcpy(WFIFOP(fd,46),g->name,24);
	memcpy(WFIFOP(fd,70),g->master,24);
	memcpy(WFIFOP(fd,94),"",20);	// 本拠地
	for(i=0;i<MAX_GUILDCASTLE;i++){
		gc=guild_castle_search(i);
		if(!gc) continue;
			if(g->guild_id == gc->guild_id)	t++;
	}

	if      (t==1)  memcpy(WFIFOP(fd,94),"One Castle",20);
	else if (t==2)  memcpy(WFIFOP(fd,94),"Two Castles",20);
	else if (t==3)  memcpy(WFIFOP(fd,94),"Three Castles",20);
	else if (t==4)  memcpy(WFIFOP(fd,94),"Four Castles",20);
	else if (t==5)  memcpy(WFIFOP(fd,94),"Five Castles",20);
	else if (t==6)  memcpy(WFIFOP(fd,94),"Six Castles",20);
	else if (t==7)  memcpy(WFIFOP(fd,94),"Seven Castles",20);
	else if (t==8)  memcpy(WFIFOP(fd,94),"Eight Castles",20);
	else if (t==9)  memcpy(WFIFOP(fd,94),"Nine Castles",20);
	else if (t==10) memcpy(WFIFOP(fd,94),"Ten Castles",20);
	else if (t==11) memcpy(WFIFOP(fd,94),"Eleven Castles",20);
	else if (t==12) memcpy(WFIFOP(fd,94),"Twelve Castles",20);
	else if (t==13) memcpy(WFIFOP(fd,94),"Thirteen Castles",20);
	else if (t==14) memcpy(WFIFOP(fd,94),"Fourteen Castles",20);
	else if (t==15) memcpy(WFIFOP(fd,94),"Fifteen Castles",20);
	else if (t==16) memcpy(WFIFOP(fd,94),"Sixteen Castles",20);
	else if (t==17) memcpy(WFIFOP(fd,94),"Seventeen Castles",20);
	else if (t==18) memcpy(WFIFOP(fd,94),"Eighteen Castles",20);
	else if (t==19) memcpy(WFIFOP(fd,94),"Nineteen Castles",20);
	else if (t==20) memcpy(WFIFOP(fd,94),"Twenty Castles",20);
	else if (t==21) memcpy(WFIFOP(fd,94),"Twenty One Castles",20);
	else if (t==22) memcpy(WFIFOP(fd,94),"Twenty Two Castles",20);
	else if (t==23) memcpy(WFIFOP(fd,94),"Twenty Three Castles",20);
	else if (t==24) memcpy(WFIFOP(fd,94),"Twenty Four Castles",20);
	else if (t==MAX_GUILDCASTLE) memcpy(WFIFOP(fd,94),"Total Domination",20);
	else memcpy(WFIFOP(fd,94),"None Taken",20);

	WFIFOSET(fd,packet_db[WFIFOW(fd,0)].len);
	clif_guild_emblem(sd,g);	// Guild emblem vanish fix [Valaris]
	return 0;
}

/*==========================================
 * ギルド同盟/敵対情報
 *------------------------------------------
 */
int clif_guild_allianceinfo(struct map_session_data *sd)
{
	int fd,i,c;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x14c;
	for(i=c=0;i<MAX_GUILDALLIANCE;i++){
		struct guild_alliance *a=&g->alliance[i];
		if(a->guild_id>0){
			WFIFOL(fd,c*32+4)=a->opposition;
			WFIFOL(fd,c*32+8)=a->guild_id;
			memcpy(WFIFOP(fd,c*32+12),a->name,24);
			c++;
		}
	}
	WFIFOW(fd, 2)=c*32+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ギルドメンバーリスト
 *------------------------------------------
 */
int clif_guild_memberlist(struct map_session_data *sd)
{
	int fd;
	int i,c;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;

	WFIFOW(fd, 0)=0x154;
	for(i=0,c=0;i<g->max_member;i++){
		struct guild_member *m=&g->member[i];
		if(m->account_id==0)
			continue;
		WFIFOL(fd,c*104+ 4)=m->account_id;
		WFIFOL(fd,c*104+ 8)=m->char_id;
		WFIFOW(fd,c*104+12)=m->hair;
		WFIFOW(fd,c*104+14)=m->hair_color;
		WFIFOW(fd,c*104+16)=m->gender;
		WFIFOW(fd,c*104+18)=m->class;
		WFIFOW(fd,c*104+20)=m->lv;
		WFIFOL(fd,c*104+22)=m->exp;
		WFIFOL(fd,c*104+26)=m->online;
		WFIFOL(fd,c*104+30)=m->position;
		memset(WFIFOP(fd,c*104+34),0,50);	// メモ？
		memcpy(WFIFOP(fd,c*104+84),m->name,24);
		c++;
	}
	WFIFOW(fd, 2)=c*104+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職名リスト
 *------------------------------------------
 */
int clif_guild_positionnamelist(struct map_session_data *sd)
{
	int i,fd;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x166;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		WFIFOL(fd,i*28+4)=i;
		memcpy(WFIFOP(fd,i*28+8),g->position[i].name,24);
	}
	WFIFOW(fd,2)=i*28+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職情報リスト
 *------------------------------------------
 */
int clif_guild_positioninfolist(struct map_session_data *sd)
{
	int i,fd;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd, 0)=0x160;
	for(i=0;i<MAX_GUILDPOSITION;i++){
		struct guild_position *p=&g->position[i];
		WFIFOL(fd,i*16+ 4)=i;
		WFIFOL(fd,i*16+ 8)=p->mode;
		WFIFOL(fd,i*16+12)=i;
		WFIFOL(fd,i*16+16)=p->exp_mode;
	}
	WFIFOW(fd, 2)=i*16+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド役職変更通知
 *------------------------------------------
 */
int clif_guild_positionchanged(struct guild *g,int idx)
{
	struct map_session_data *sd;
	unsigned char buf[128];

	nullpo_retr(0, g);

	WBUFW(buf, 0)=0x174;
	WBUFW(buf, 2)=44;
	WBUFL(buf, 4)=idx;
	WBUFL(buf, 8)=g->position[idx].mode;
	WBUFL(buf,12)=idx;
	WBUFL(buf,16)=g->position[idx].exp_mode;
	memcpy(WBUFP(buf,20),g->position[idx].name,24);
	if( (sd=guild_getavailablesd(g))!=NULL )
		clif_send(buf,WBUFW(buf,2),&sd->bl,GUILD);
	return 0;
}
/*==========================================
 * ギルドメンバ変更通知
 *------------------------------------------
 */
int clif_guild_memberpositionchanged(struct guild *g,int idx)
{
	struct map_session_data *sd;
	unsigned char buf[64];

	nullpo_retr(0, g);

	WBUFW(buf, 0)=0x156;
	WBUFW(buf, 2)=16;
	WBUFL(buf, 4)=g->member[idx].account_id;
	WBUFL(buf, 8)=g->member[idx].char_id;
	WBUFL(buf,12)=g->member[idx].position;
	if( (sd=guild_getavailablesd(g))!=NULL )
		clif_send(buf,WBUFW(buf,2),&sd->bl,GUILD);
	return 0;
}
/*==========================================
 * ギルドエンブレム送信
 *------------------------------------------
 */
int clif_guild_emblem(struct map_session_data *sd,struct guild *g)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, g);

	fd=sd->fd;

	if(g->emblem_len<=0)
		return 0;
	WFIFOW(fd,0)=0x152;
	WFIFOW(fd,2)=g->emblem_len+12;
	WFIFOL(fd,4)=g->guild_id;
	WFIFOL(fd,8)=g->emblem_id;
	memcpy(WFIFOP(fd,12),g->emblem_data,g->emblem_len);
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルドスキル送信
 *------------------------------------------
 */
int clif_guild_skillinfo(struct map_session_data *sd)
{
	int fd;
	int i,id,c,up=1;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd,0)=0x0162;
	WFIFOW(fd,4)=g->skill_point;
	for(i=c=0;i<MAX_GUILDSKILL;i++){
		if(g->skill[i].id>0){
			WFIFOW(fd,c*37+ 6) = id = g->skill[i].id;
			WFIFOW(fd,c*37+ 8) = guild_skill_get_inf(id);
			WFIFOW(fd,c*37+10) = 0;
			WFIFOW(fd,c*37+12) = g->skill[i].lv;
			WFIFOW(fd,c*37+14) = guild_skill_get_sp(id,g->skill[i].lv);
			WFIFOW(fd,c*37+16) = guild_skill_get_range(id);
			memset(WFIFOP(fd,c*37+18),0,24);
			if(g->skill[i].lv < guild_skill_get_max(id)) {
				//Kafra and Guardian changed to require Approval [Sara]
				if (g->skill[i].id == GD_KAFRACONTACT && guild_checkskill(g,GD_APPROVAL) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_GUARDIANRESEARCH && guild_checkskill(g,GD_APPROVAL) <= 0)
					up = 0;
				//Glory skill requirements -- Pretty sure correct [Sara]
				else if (g->skill[i].id == GD_LEADERSHIP && guild_checkskill(g,GD_GLORYGUILD) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_GLORYWOUNDS && guild_checkskill(g,GD_GLORYGUILD) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_SOULCOLD && guild_checkskill(g,GD_GLORYWOUNDS) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_HAWKEYES && guild_checkskill(g,GD_LEADERSHIP) <= 0)
					up = 0;
				//Activated skill requirements -- Just guesses [Sara]
				else if (g->skill[i].id == GD_BATTLEORDER && guild_checkskill(g,GD_APPROVAL) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_REGENERATION && guild_checkskill(g,GD_APPROVAL) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_RESTORE && guild_checkskill(g,GD_REGENERATION) <= 0)
					up = 0;
				else if (g->skill[i].id == GD_EMERGENCYCALL && guild_checkskill(g,GD_APPROVAL) <= 0)
					up = 0;
				if (g->skill[i].id == GD_GUARDUP && guild_checkskill(g,GD_GUARDIANRESEARCH) <= 0)
					up = 0;
				//Unadded yet? Has extension description in kRO tables
				else if (g->skill[i].id == GD_DEVELOPMENT)
					up = 0;
				else
					up = 1;
			}
			else {
				up = 0;
			}
			WFIFOB(fd,c*37+42)= up;
			c++;
		}
	}
	WFIFOW(fd,2)=c*37+6;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}
/*==========================================
 * ギルド告知送信
 *------------------------------------------
 */
int clif_guild_notice(struct map_session_data *sd,struct guild *g)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, g);

	fd=sd->fd;
	if(*g->mes1==0 && *g->mes2==0)
		return 0;
	WFIFOW(fd,0)=0x16f;
	memcpy(WFIFOP(fd,2),g->mes1,60);
	memcpy(WFIFOP(fd,62),g->mes2,120);
	WFIFOSET(fd,packet_db[0x16f].len);
	return 0;
}

/*==========================================
 * ギルドメンバ勧誘
 *------------------------------------------
 */
int clif_guild_invite(struct map_session_data *sd,struct guild *g)
{
	int fd;

	nullpo_retr(0, sd);
	nullpo_retr(0, g);

	fd=sd->fd;
	WFIFOW(fd,0)=0x16a;
	WFIFOL(fd,2)=g->guild_id;
	memcpy(WFIFOP(fd,6),g->name,24);
	WFIFOSET(fd,packet_db[0x16a].len);
	return 0;
}
/*==========================================
 * ギルドメンバ勧誘結果
 *------------------------------------------
 */
int clif_guild_inviteack(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x169;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x169].len);
	return 0;
}
/*==========================================
 * ギルドメンバ脱退通知
 *------------------------------------------
 */
int clif_guild_leave(struct map_session_data *sd,const char *name,const char *mes)
{
	unsigned char buf[128];

	nullpo_retr(0, sd);

	WBUFW(buf, 0)=0x15a;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	clif_send(buf,packet_db[0x15a].len,&sd->bl,GUILD);
	return 0;
}
/*==========================================
 * ギルドメンバ追放通知
 *------------------------------------------
 */
int clif_guild_explusion(struct map_session_data *sd,const char *name,const char *mes,
	int account_id)
{
	unsigned char buf[128];

	nullpo_retr(0, sd);

	WBUFW(buf, 0)=0x15c;
	memcpy(WBUFP(buf, 2),name,24);
	memcpy(WBUFP(buf,26),mes,40);
	memcpy(WBUFP(buf,66),"dummy",24);
	clif_send(buf,packet_db[0x15c].len,&sd->bl,GUILD);
	return 0;
}
/*==========================================
 * ギルド追放メンバリスト
 *------------------------------------------
 */
int clif_guild_explusionlist(struct map_session_data *sd)
{
	int fd;
	int i,c;
	struct guild *g;

	nullpo_retr(0, sd);

	fd=sd->fd;
	g=guild_search(sd->status.guild_id);
	if(g==NULL)
		return 0;
	WFIFOW(fd,0)=0x163;
	for(i=c=0;i<MAX_GUILDEXPLUSION;i++){
		struct guild_explusion *e=&g->explusion[i];
		if(e->account_id>0){
			memcpy(WFIFOP(fd,c*88+ 4),e->name,24);
			memcpy(WFIFOP(fd,c*88+28),e->acc,24);
			memcpy(WFIFOP(fd,c*88+52),e->mes,44);
			c++;
		}
	}
	WFIFOW(fd,2)=c*88+4;
	WFIFOSET(fd,WFIFOW(fd,2));
	return 0;
}

/*==========================================
 * ギルド会話
 *------------------------------------------
 */
int clif_guild_message(struct guild *g,int account_id,const char *mes,int len)
{
	struct map_session_data *sd;
	//unsigned char lbuf[255];
	//unsigned char *buf = lbuf;
        //if (len + 32 >= sizeof(lbuf))
        //    buf = malloc(len + 32);
	unsigned char buf[len+32];

	WBUFW(buf, 0)=0x17f;
	WBUFW(buf, 2)=len+4;
	memcpy(WBUFP(buf,4),mes,len);

	if( (sd=guild_getavailablesd(g))!=NULL )
		clif_send(buf,WBUFW(buf,2),&sd->bl,GUILD);
        //if ( buf != lbuf)
        //    free(buf);
	return 0;
}
/*==========================================
 * ギルドスキル割り振り通知
 *------------------------------------------
 */
int clif_guild_skillup(struct map_session_data *sd,int skill_num,int lv)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0) = 0x10e;
	WFIFOW(fd,2) = skill_num;
	WFIFOW(fd,4) = lv;
	WFIFOW(fd,6) = guild_skill_get_sp(skill_num,lv);
	WFIFOW(fd,8) = guild_skill_get_range(skill_num);
	WFIFOB(fd,10) = 1;
	WFIFOSET(fd,11);
	return 0;
}
/*==========================================
 * ギルド同盟要請
 *------------------------------------------
 */
int clif_guild_reqalliance(struct map_session_data *sd,int account_id,const char *name)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x171;
	WFIFOL(fd,2)=account_id;
	memcpy(WFIFOP(fd,6),name,24);
	WFIFOSET(fd,packet_db[0x171].len);
	return 0;
}
/*==========================================
 * ギルド同盟結果
 *------------------------------------------
 */
int clif_guild_allianceack(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x173;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x173].len);
	return 0;
}
/*==========================================
 * ギルド関係解消通知
 *------------------------------------------
 */
int clif_guild_delalliance(struct map_session_data *sd,int guild_id,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x184;
	WFIFOL(fd,2)=guild_id;
	WFIFOL(fd,6)=flag;
	WFIFOSET(fd,packet_db[0x184].len);
	return 0;
}
/*==========================================
 * ギルド敵対結果
 *------------------------------------------
 */
int clif_guild_oppositionack(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x181;
	WFIFOB(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x181].len);
	return 0;
}
/*==========================================
 * ギルド関係追加
 *------------------------------------------
 */
/*int clif_guild_allianceadded(struct guild *g,int idx)
{
	unsigned char buf[64];
	WBUFW(fd,0)=0x185;
	WBUFL(fd,2)=g->alliance[idx].opposition;
	WBUFL(fd,6)=g->alliance[idx].guild_id;
	memcpy(WBUFP(fd,10),g->alliance[idx].name,24);
	clif_send(buf,packet_db[0x185].len,guild_getavailablesd(g),GUILD);
	return 0;
}*/

/*==========================================
 * ギルド解散通知
 *------------------------------------------
 */
int clif_guild_broken(struct map_session_data *sd,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0x15e;
	WFIFOL(fd,2)=flag;
	WFIFOSET(fd,packet_db[0x15e].len);
	return 0;
}

/*==========================================
 * エモーション
 *------------------------------------------
 */
void clif_emotion(struct block_list *bl,int type)
{
	unsigned char buf[8];

	nullpo_retv(bl);

	WBUFW(buf,0)=0xc0;
	WBUFL(buf,2)=bl->id;
	WBUFB(buf,6)=type;
	clif_send(buf,packet_db[0xc0].len,bl,AREA);
}

/*==========================================
 * トーキーボックス
 *------------------------------------------
 */
void clif_talkiebox(struct block_list *bl,char* talkie)
{
	unsigned char buf[86];

	nullpo_retv(bl);

	WBUFW(buf,0)=0x191;
	WBUFL(buf,2)=bl->id;
	memcpy(WBUFP(buf,6),talkie,80);
	clif_send(buf,packet_db[0x191].len,bl,AREA);
}

/*==========================================
 * 結婚エフェクト
 *------------------------------------------
 */
void clif_wedding_effect(struct block_list *bl)
{
	unsigned char buf[6];

	nullpo_retv(bl);

	WBUFW(buf,0)=0x1ea;
	WBUFL(buf,2)=bl->id;
	clif_send(buf,packet_db[0x1ea].len,bl,AREA);
}
/*==========================================
 * あなたに逢いたい使用時名前叫び
 *------------------------------------------

void clif_callpartner(struct map_session_data *sd)
{
	unsigned char buf[26];
	char *p;

	nullpo_retv(sd);

	if(sd->status.partner_id){
		WBUFW(buf,0)=0x1e6;
		p = map_charid2nick(sd->status.partner_id);
		if(p){
			memcpy(WBUFP(buf,2),p,24);
		}else{
			map_reqchariddb(sd,sd->status.partner_id);
			chrif_searchcharid(sd->status.partner_id);
			WBUFB(buf,2) = 0;
		}
		clif_send(buf,packet_db[0x1e6].len,&sd->bl,AREA);
	}
	return;
}
*/
/*==========================================
 * 座る
 *------------------------------------------
 */
void clif_sitting(int fd, struct map_session_data *sd) 
{
	nullpo_retv(sd);

	fd=sd->fd;

	WBUFW(fd, 0) = 0x8a;
	WBUFL(fd, 2) = sd->bl.id;
	WBUFB(fd,26) = 2;
	clif_send(WFIFOP(fd,0),packet_db[0x8a].len,&sd->bl,AREA);
}

/*==========================================
 *
 *------------------------------------------
 */
int clif_disp_onlyself(struct map_session_data *sd,char *mes,int len)
{
	//unsigned char lbuf[255];
	//unsigned char *buf = (len + 32 >= sizeof(lbuf)) ? malloc(len + 32) : lbuf;
	unsigned char buf[len+32];

	nullpo_retr(0, sd);

	WBUFW(buf, 0)=0x17f;
	WBUFW(buf, 2)=len+8;
	memcpy(WBUFP(buf,4),mes,len+4);

	clif_send(buf,WBUFW(buf,2),&sd->bl,SELF);

        //if (buf != lbuf)
        //    free(buf);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */

int clif_GM_kickack(struct map_session_data *sd,int id)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xcd;
	WFIFOL(fd,2)=id;
	WFIFOSET(fd,packet_db[0xcd].len);
	return 0;
}

int clif_GM_kick(struct map_session_data *sd,struct map_session_data *tsd,int type)
{
	int fd;
	nullpo_retr(0, tsd);

	if(type)
		clif_GM_kickack(sd,tsd->status.account_id);
	tsd->opt1 = tsd->opt2 = 0;
	fd = tsd->fd;
	WFIFOW(fd,0)=0x18b;
	WFIFOW(fd,2)=0;
	WFIFOSET(fd,packet_db[0x18b].len);
	clif_setwaitclose(fd);

	return 0;
}
/*==========================================
 * Wis拒否許可応答
 *------------------------------------------
 */
int clif_wisexin(struct map_session_data *sd,int type,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xd1;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_db[0xd1].len);

	return 0;
}
/*==========================================
 * Wis全拒否許可応答
 *------------------------------------------
 */
int clif_wisall(struct map_session_data *sd,int type,int flag)
{
	int fd;

	nullpo_retr(0, sd);

	fd=sd->fd;
	WFIFOW(fd,0)=0xd2;
	WFIFOB(fd,2)=type;
	WFIFOB(fd,3)=flag;
	WFIFOSET(fd,packet_db[0xd2].len);

	return 0;
}
/*==========================================
 * サウンドエフェクト
 *------------------------------------------
 */
void clif_soundeffect(struct map_session_data *sd,struct block_list *bl,char *name,int type)
{
	int fd;

	nullpo_retv(sd);
	nullpo_retv(bl);

	fd=sd->fd;
	WFIFOW(fd,0)=0x1d3;
	memcpy(WFIFOP(fd,2),name,24);
	WFIFOB(fd,26)=type;
	WFIFOL(fd,27)=0;
	WFIFOL(fd,31)=bl->id;
	WFIFOSET(fd,packet_db[0x1d3].len);

	return;
}
// displaying special effects (npcs, weather, etc) [Valaris]
int clif_specialeffect(struct block_list *bl, int type, int flag) {
	unsigned char buf[24];

	nullpo_retr(0, bl);

	memset(buf, 0, packet_db[0x1f3].len);

	WBUFW(buf,0) = 0x1f3;
	WBUFL(buf,2) = bl->id;
	WBUFL(buf,6) = type;

	if (flag==2) {
		struct map_session_data *sd=NULL;
		int i;
		for(i=0; i < fd_max; i++) {
			if (session[i] && (sd = session[i]->session_data) != NULL && sd->state.auth && sd->bl.m == bl->m)
				clif_specialeffect(&sd->bl,type,1);
		}
	}
	
	else if (flag==1)
		clif_send(buf, packet_db[0x1f3].len, bl, SELF);
	else if (!flag)
		clif_send(buf, packet_db[0x1f3].len, bl, AREA);

	return 0;

}
// ------------
// clif_parse_*
// ------------
// パケット読み取って色々操作
/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_WantToConnection(int fd,struct map_session_data *sd, int cmd)
{
	struct map_session_data *old_sd;
	int account_id,char_id,login_id1,sex;
	unsigned int client_tick;

	if(sd){
		if(battle_config.error_log)
			printf("clif_parse_WantToConnection : invalid request?\n");
		return;
	}

	sd=session[fd]->session_data=(struct map_session_data *)aCalloc(1,sizeof(struct map_session_data));
	if (sd == NULL) {
		printf("out of memory : clif_parse_WantToConnection\n");
		exit(1);
	}
	sd->fd = fd;

	account_id = RFIFOL(fd,packet_db[cmd].pos[0]);
	char_id = RFIFOL(fd,packet_db[cmd].pos[1]);
	login_id1 = RFIFOL(fd,packet_db[cmd].pos[2]);
	client_tick = RFIFOL(fd,packet_db[cmd].pos[3]);
	sex = RFIFOB(fd,packet_db[cmd].pos[4]);

	pc_setnewpc(sd,account_id,char_id,login_id1,client_tick,sex,fd);
	if((old_sd=map_id2sd(account_id)) != NULL){
			// if same account already connected, we disconnect the 2 sessions
		old_sd->new_fd=fd;
	} else {
		map_addiddb(&sd->bl);
	}

	chrif_authreq(sd);

	WFIFOL(fd,0)=sd->bl.id;
	WFIFOSET(fd,4);

	return;
}

/*==========================================
 * 007d クライアント側マップ読み込み完了
 * map侵入時に必要なデータを全て送りつける
 *------------------------------------------
 */
void clif_parse_LoadEndAck(int fd,struct map_session_data *sd, int cmd)
{
//	struct item_data* item;
	int i;
	nullpo_retv(sd);

	if(sd->bl.prev != NULL)
		return;

	// 接続ok時
	//clif_authok();
	if(sd->npc_id) npc_event_dequeue(sd);
	clif_skillinfoblock(sd);
	pc_checkitem(sd);
	//guild_info();

	// loadendack時
	// next exp
	clif_updatestatus(sd,SP_NEXTBASEEXP);
	clif_updatestatus(sd,SP_NEXTJOBEXP);
	// skill point
	clif_updatestatus(sd,SP_SKILLPOINT);
	// item
	clif_itemlist(sd);
	clif_equiplist(sd);
	// cart
	if(pc_iscarton(sd)){
		clif_cart_itemlist(sd);
		clif_cart_equiplist(sd);
		clif_updatestatus(sd,SP_CARTINFO);
	}
	// param all
	clif_initialstatus(sd);
	// party
	party_send_movemap(sd);
	// guild
	guild_send_memberinfoshort(sd,1);
	// 119
	// 78

	if(battle_config.pc_invincible_time > 0) {
		if(map[sd->bl.m].flag.gvg)
			pc_setinvincibletimer(sd,battle_config.pc_invincible_time<<1);
		else
			pc_setinvincibletimer(sd,battle_config.pc_invincible_time);
	}

	map_addblock(&sd->bl);	// ブロック登録
	clif_spawnpc(sd);	// spawn

	// weight max , now
	clif_updatestatus(sd,SP_MAXWEIGHT);
	clif_updatestatus(sd,SP_WEIGHT);

	// pvp
	if(sd->pvp_timer!=-1 && !battle_config.pk_mode)
		delete_timer(sd->pvp_timer,pc_calc_pvprank_timer);
	if(map[sd->bl.m].flag.pvp){
		if(!battle_config.pk_mode) { // remove pvp stuff for pk_mode [Valaris]
			sd->pvp_timer=add_timer(gettick()+200,pc_calc_pvprank_timer,sd->bl.id,0);
			sd->pvp_rank=0;
			sd->pvp_lastusers=0;
			sd->pvp_point=5;
		}
		clif_set0199(sd->fd,1);
	} else {
		sd->pvp_timer=-1;
	}
	if(map[sd->bl.m].flag.gvg) {
		clif_set0199(sd->fd,3);
	}

	// pet
	if(sd->status.pet_id > 0 && sd->pd && sd->pet.intimate > 0) {
		map_addblock(&sd->pd->bl);
		clif_spawnpet(sd->pd);
		clif_send_petdata(sd,0,0);
		clif_send_petdata(sd,5,0x14);
		clif_send_petstatus(sd);
	}

	if(sd->state.connect_new) {
		sd->state.connect_new = 0;
		if(sd->status.class != sd->view_class)
			clif_changelook(&sd->bl,LOOK_BASE,sd->view_class);
		if(sd->status.pet_id > 0 && sd->pd && sd->pet.intimate > 900)
			clif_pet_emotion(sd->pd,(sd->pd->class - 100)*100 + 50 + pet_hungry_val(sd));

/*						Stop players from spawning inside castles [Valaris]					*/

                        {
			struct guild_castle *gc=guild_mapname2gc(map[sd->bl.m].name);
			if (gc)
				pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,2);
                        }

/*						End Addition [Valaris]			*/

		}

	// view equipment item
#if PACKETVER < 4
	clif_changelook(&sd->bl,LOOK_WEAPON,sd->status.weapon);
	clif_changelook(&sd->bl,LOOK_SHIELD,sd->status.shield);
#else
	clif_changelook(&sd->bl,LOOK_WEAPON,0);
#endif
	if(battle_config.save_clothcolor==1 && sd->status.clothes_color > 0)
		clif_changelook(&sd->bl,LOOK_CLOTHES_COLOR,sd->status.clothes_color);

	if(sd->status.hp<sd->status.max_hp>>2 && pc_checkskill(sd,SM_AUTOBERSERK)>0 &&
		(sd->sc_data[SC_PROVOKE].timer==-1 || sd->sc_data[SC_PROVOKE].val2==0 ))
		// オートバーサーク発動
		skill_status_change_start(&sd->bl,SC_PROVOKE,10,1,0,0,0,0);
	
//	if(time(&timer) < ((weddingtime=pc_readglobalreg(sd,"PC_WEDDING_TIME")) + 3600))
//		skill_status_change_start(&sd->bl,SC_WEDDING,0,weddingtime,0,0,36000,0);

	if(battle_config.muting_players && sd->status.manner < 0)
		skill_status_change_start(&sd->bl,SC_NOCHAT,0,0,0,0,0,0);

	// option
	clif_changeoption(&sd->bl);
	if(sd->sc_data[SC_TRICKDEAD].timer != -1)
		skill_status_change_end(&sd->bl,SC_TRICKDEAD,-1);
	if(sd->sc_data[SC_SIGNUMCRUCIS].timer != -1 && !battle_check_undead(7,sd->def_ele))
		skill_status_change_end(&sd->bl,SC_SIGNUMCRUCIS,-1);
	if(sd->special_state.infinite_endure && sd->sc_data[SC_ENDURE].timer == -1)
		skill_status_change_start(&sd->bl,SC_ENDURE,10,1,0,0,0,0);
	for(i=0;i<MAX_INVENTORY;i++){
		if(sd->status.inventory[i].equip && sd->status.inventory[i].equip & 0x0002 && sd->status.inventory[i].attribute==1)
			skill_status_change_start(&sd->bl,SC_BROKNWEAPON,0,0,0,0,0,0);
		if(sd->status.inventory[i].equip && sd->status.inventory[i].equip & 0x0010 && sd->status.inventory[i].attribute==1)
			skill_status_change_start(&sd->bl,SC_BROKNARMOR,0,0,0,0,0,0);
	}

	map_foreachinarea(clif_getareachar,sd->bl.m,sd->bl.x-AREA_SIZE,sd->bl.y-AREA_SIZE,sd->bl.x+AREA_SIZE,sd->bl.y+AREA_SIZE,0,sd);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_TickSend(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	sd->client_tick=RFIFOL(fd,packet_db[cmd].pos[0]);
	sd->server_tick=gettick();
	clif_servertick(sd);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_WalkToXY(int fd, struct map_session_data *sd, int cmd) {
	int x, y;

	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}

	if (sd->npc_id != 0 || sd->vender_id != 0)
		return;

	if(sd->skilltimer != -1 && pc_checkskill(sd,SA_FREECAST) <= 0) // フリーキャスト
		return;

	if(sd->chatID)
		return;

	if(sd->canmove_tick > gettick())
		return;

	// ステータス異常やハイディング中(トンネルドライブ無)で動けない
	if((sd->opt1 > 0 && sd->opt1 != 6) ||
		sd->sc_data[SC_ANKLE].timer !=-1 || //アンクルスネア
		sd->sc_data[SC_AUTOCOUNTER].timer !=-1 || //オートカウンター
		sd->sc_data[SC_TRICKDEAD].timer !=-1 || //死んだふり
		sd->sc_data[SC_BLADESTOP].timer !=-1 || //白刃取り
		sd->sc_data[SC_SPIDERWEB].timer !=-1 || //スパイダーウェッブ
		(sd->sc_data[SC_DANCING].timer !=-1 && sd->sc_data[SC_DANCING].val4) //合奏スキル演奏中は動けない
		) //
		return;
	if( (sd->status.option&2) && pc_checkskill(sd,RG_TUNNELDRIVE) <= 0)
		return;

	if(sd->invincible_timer != -1)
		pc_delinvincibletimer(sd);

	pc_stopattack(sd);

	x = RFIFOB(fd,packet_db[cmd].pos[0])*4+(RFIFOB(fd,packet_db[cmd].pos[0]+1)>>6);
	y = ((RFIFOB(fd,packet_db[cmd].pos[0]+1)&0x3f)<<4)+(RFIFOB(fd,packet_db[cmd].pos[0]+2)>>4);

	pc_walktoxy(sd,x,y);

}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_QuitGame(int fd,struct map_session_data *sd, int cmd)
{
	unsigned int tick=gettick();
	struct skill_unit_group* sg;

	nullpo_retv(sd);

	WFIFOW(fd,0)=0x18b;
	if ((!pc_isdead(sd) && (sd->opt1 || (sd->opt2 && !(night_flag == 1 && sd->opt2 == STATE_BLIND)))) ||
		sd->skilltimer != -1 ||
		(DIFF_TICK(tick , sd->canact_tick) < 0) ||
	    (sd->sc_data && sd->sc_data[SC_DANCING].timer!=-1 && sd->sc_data[SC_DANCING].val4 && (sg=(struct skill_unit_group *)sd->sc_data[SC_DANCING].val2) && sg->src_id == sd->bl.id)) {
		WFIFOW(fd,2)=1;
		WFIFOSET(fd,packet_db[0x18b].len);
		return;
	}

	/*	Rovert's prevent logout option fixed [Valaris]	*/
	if ((battle_config.prevent_logout && (gettick() - sd->canlog_tick) >= 10000) || (!battle_config.prevent_logout)) {
		clif_setwaitclose(fd);
		WFIFOW(fd,2)=0;
	} else {
		WFIFOW(fd,2)=1;
	}
	WFIFOSET(fd,packet_db[0x18b].len);
	clif_setwaitclose(fd);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_GetCharNameRequest(int fd,struct map_session_data *sd, int cmd)
{
	struct block_list *bl;
	int account_id;

	account_id = RFIFOL(fd,packet_db[cmd].pos[0]);
	bl=map_id2bl(account_id);
	if(bl==NULL)
		return;

	WFIFOW(fd,0)=0x95;
	WFIFOL(fd,2)=account_id;

	switch(bl->type){
	case BL_PC:
	  {
			struct map_session_data *ssd=(struct map_session_data *)bl;
			struct party *p=NULL;
			struct guild *g=NULL;

			nullpo_retv(ssd);

			memcpy(WFIFOP(fd,6),ssd->status.name,24);
			if( ssd->status.guild_id>0 &&(g=guild_search(ssd->status.guild_id))!=NULL &&
				(ssd->status.party_id==0 ||(p=party_search(ssd->status.party_id))!=NULL) ){
				// ギルド所属ならパケット0195を返す
				int i,ps=-1;
				for(i=0;i<g->max_member;i++){
					if( g->member[i].account_id==ssd->status.account_id &&
						g->member[i].char_id==ssd->status.char_id )
						ps=g->member[i].position;
				}
				if(ps>=0 && ps<MAX_GUILDPOSITION){

					WFIFOW(fd, 0)=0x195;
					if(p)
						memcpy(WFIFOP(fd,30),p->name,24);
					else
						WFIFOB(fd,30)=0;
					memcpy(WFIFOP(fd,54),g->name,24);
					memcpy(WFIFOP(fd,78),g->position[ps].name,24);
					WFIFOSET(fd,packet_db[0x195].len);
					break;
				}
			}
			WFIFOSET(fd,packet_db[0x95].len);
		} break;
	case BL_PET:
		memcpy(WFIFOP(fd,6),((struct pet_data*)bl)->name,24);
		WFIFOSET(fd,packet_db[0x95].len);
		break;
	case BL_NPC:
		memcpy(WFIFOP(fd,6),((struct npc_data*)bl)->name,24);
		WFIFOSET(fd,packet_db[0x95].len);
		break;
	case BL_MOB:
		{
			struct mob_data *md = (struct mob_data *)bl;

			nullpo_retv(md);

			memcpy(WFIFOP(fd,6), md->name, 24);
			if (md->class >= 1285 && md->class <= 1288 && md->guild_id) {
				struct guild *g;
				struct guild_castle *gc = guild_mapname2gc(map[md->bl.m].name);
				if (gc && gc->guild_id > 0 && (g = guild_search(gc->guild_id)) != NULL) {
					WFIFOW(fd, 0) = 0x195;
					WFIFOB(fd,30) = 0;
					memcpy(WFIFOP(fd,54), g->name, 24);
					memcpy(WFIFOP(fd,78), gc->castle_name, 24);
					WFIFOSET(fd,packet_db[0x195].len);
				} else {
					WFIFOSET(fd,packet_db[0x95].len);
				}
			} else if (battle_config.show_mob_hp == 1) {
				char mobhp[50];
				sprintf(mobhp, "hp: %d/%d", md->hp, mob_db[md->class].max_hp);
				WFIFOW(fd, 0) = 0x195;
				memcpy(WFIFOP(fd,30), mobhp, 24);
				WFIFOB(fd,54) = 0;
				WFIFOB(fd,78) = 0;
				WFIFOSET(fd,packet_db[0x195].len);
			} else {
				WFIFOSET(fd,packet_db[0x95].len);
			}
		}
		break;
	default:
		if(battle_config.error_log)
			printf("clif_parse_GetCharNameRequest : bad type %d(%d)\n",bl->type,account_id);
		break;
	}
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_GlobalMessage(int fd, struct map_session_data *sd, int cmd) { // S 008c <len>.w <str>.?B
	char *message = (char *) malloc(RFIFOW(fd,2) + 128);
	char *buf = (char *) malloc(RFIFOW(fd,2) + 4);

	nullpo_retv(sd);

	memset(message, '\0', RFIFOW(fd,packet_db[cmd].pos[0]) + 128);
	memset(buf, '\0', RFIFOW(fd,packet_db[cmd].pos[0]) + 4);

	if((is_atcommand(fd, sd, RFIFOP(fd, packet_db[cmd].pos[1]), 0) != AtCommand_None) ||
            ( sd->sc_data && 
		(sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd->sc_data[SC_NOCHAT].timer!=-1 )) )	//チャット禁止
            {
                free(message);
                free(buf);
		return;
            }

	//printf("clif_parse_GlobalMessage: message: '%s'.\n", RFIFOP(fd,4));
	if(strncmp(RFIFOP(fd, packet_db[cmd].pos[1]), sd->status.name, strlen(sd->status.name)) != 0) {
		printf("Hack on global message: character '%s' (account: %d), use an other name to send a (normal) message.\n", sd->status.name, sd->status.account_id);

		// information is sended to all online GM
		sprintf(message, "Hack on global message (normal message): character '%s' (account: %d) uses an other name.", sd->status.name, sd->status.account_id);
		intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message, strlen(message) + 1);
		if (strlen(RFIFOP(fd, packet_db[cmd].pos[1])) == 0)
			strcpy(message, " This player sends a void name and a void message.");
		else
			sprintf(message, " This player sends (name:message): '%s'.", RFIFOP(fd, packet_db[cmd].pos[1]));
		intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message, strlen(message) + 1);
		// message about the ban
		if (battle_config.ban_spoof_namer > 0)
			sprintf(message, " This player has been banned for %d minute(s).", battle_config.ban_spoof_namer);
		else
			sprintf(message, " This player hasn't been banned (Ban option is disabled).");
		intif_wis_message_to_gm(wisp_server_name, battle_config.hack_info_GM_level, message, strlen(message) + 1);

		// if we ban people
		if (battle_config.ban_spoof_namer > 0) {
			chrif_char_ask_name(-1, sd->status.name, 2, 0, 0, 0, 0, battle_config.ban_spoof_namer, 0); // type: 2 - ban (year, month, day, hour, minute, second)
			clif_setwaitclose(fd); // forced to disconnect because of the hack
		}
		// but for the hacker, we display on his screen (he see/look no difference).
	} else {
		// send message to others
		WBUFW(buf,0) = 0x8d;
		WBUFW(buf,2) = RFIFOW(fd,2) + 4; // len of message - 4 + 8
		WBUFL(buf,4) = sd->bl.id;
		memcpy(WBUFP(buf,8), RFIFOP(fd,4), RFIFOW(fd,2) - 4);
		clif_send(buf, WBUFW(buf,2), &sd->bl, sd->chatID ? CHAT_WOS : AREA_CHAT_WOC);
	}

	// send back message to the speaker
	memcpy(WFIFOP(fd,0), RFIFOP(fd,0), RFIFOW(fd,packet_db[cmd].pos[0]));
	WFIFOW(fd,0) = 0x8e;
	WFIFOSET(fd, WFIFOW(fd,2));

        free(message);
        free(buf);

	return;
}

int clif_message(struct block_list *bl, char* msg)
{
	unsigned short msg_len = strlen(msg) + 1;
	unsigned char buf[256];

	nullpo_retr(0, bl);

	WBUFW(buf, 0) = 0x8d;
	WBUFW(buf, 2) = msg_len + 8;
	WBUFL(buf, 4) = bl->id;
	memcpy(WBUFP(buf, 8), msg, msg_len);

	clif_send(buf, WBUFW(buf,2), bl, AREA);

	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_MapMove(int fd, struct map_session_data *sd, int cmd) {
// /m /mapmove (as @rura GM command)
	char mapname[32];
	int x,y;

	if (battle_config.atc_gmonly == 0 ||
		pc_isGM(sd) >= get_atcommand_level(AtCommand_MapMove)) {
		memcpy(mapname,RFIFOP(fd,packet_db[cmd].pos[0]),16);
		mapname[16]=0;
		x=RFIFOW(fd,packet_db[cmd].pos[1]);
		y=RFIFOW(fd,packet_db[cmd].pos[2]);
		pc_setpos(sd,mapname,x,y,2);
	}
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ChangeDir(int fd,struct map_session_data *sd, int cmd)
{
	short headdir,dir;
	unsigned char buf[64];

	nullpo_retv(sd);

	headdir=RFIFOW(fd,packet_db[cmd].pos[0]);
	dir=RFIFOB(fd,packet_db[cmd].pos[1]);
	pc_setdir(sd,dir,headdir);

	WFIFOW(fd,0)=0x9c;
	WFIFOL(fd,2)=sd->bl.id;
	WFIFOW(fd,6)=headdir;
	WFIFOB(fd,8)=dir;
	if (sd->disguise > 23 && sd->disguise < 4001) // mob disguises [Valaris]
		clif_send(buf, packet_db[0x9c].len, &sd->bl, AREA);
	else
		clif_send(buf, packet_db[0x9c].len, &sd->bl, AREA_WOS);

}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_Emotion(int fd,struct map_session_data *sd, int cmd)
{
	int emotion;
	unsigned char buf[64];

	nullpo_retv(sd);

	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 2){
		emotion=RFIFOB(fd,packet_db[cmd].pos[0]);
		WBUFW(buf,0) = 0xc0;
		WBUFL(buf,2) = sd->bl.id;
		WBUFB(buf,6) = emotion;
		clif_send(buf, packet_db[0xc0].len, &sd->bl, AREA);
	} else
		clif_skill_fail(sd, 1, 0, 1);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_HowManyConnections(int fd, struct map_session_data *sd, int cmd) {
	WFIFOW(fd,0) = 0xc2;
	WFIFOL(fd,2) = map_getusers();
	WFIFOSET(fd,packet_db[0xc2].len);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ActionRequest(int fd, struct map_session_data *sd, int cmd)
{
	unsigned int tick;
	unsigned char buf[64];
	int action_type, target_id;

	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}
	if( sd->npc_id!=0 || sd->opt1 > 0 || sd->status.option&2 ||
		( sd->sc_data &&
		( sd->sc_data[SC_AUTOCOUNTER].timer != -1 ||	//オートカウンター
		sd->sc_data[SC_BLADESTOP].timer != -1 ||	//白刃取り
		sd->sc_data[SC_DANCING].timer!=-1 )) )	//ダンス中
			return;

	tick=gettick();

	pc_stop_walking(sd,0);
	pc_stopattack(sd);

	target_id = RFIFOL(fd,packet_db[cmd].pos[0]);
	action_type = RFIFOB(fd,packet_db[cmd].pos[1]);

	switch(action_type) {
	case 0x00: // once attack
	case 0x07: // continuous attack
		if(sd->sc_data[SC_WEDDING].timer != -1 || sd->view_class==22)
			return;
		if (sd->vender_id != 0)
			return;
		if (!battle_config.sdelay_attack_enable && pc_checkskill(sd, SA_FREECAST) <= 0) {
			if (DIFF_TICK(tick, sd->canact_tick) < 0) {
				clif_skill_fail(sd, 1, 4, 0);
				return;
			}
		}
		if (sd->invincible_timer != -1)
			pc_delinvincibletimer(sd);
		if (sd->attacktarget > 0) // [Valaris]
			sd->attacktarget = 0;
		pc_attack(sd, target_id, action_type != 0);
		break;
	case 0x02:	// sitdown
		if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 3) {
			pc_stop_walking(sd,1);
			skill_gangsterparadise(sd,1);/* ギャングスターパラダイス設定 */
			pc_setsit(sd);
			clif_sitting(fd,sd);
		} else
			clif_skill_fail(sd, 1, 0, 2);
		break;
	case 0x03:	// standup
		skill_gangsterparadise(sd,0);/* ギャングスターパラダイス解除 */
		pc_setstand(sd);
		WBUFW(buf, 0) = 0x8a;
		WBUFL(buf, 2) = sd->bl.id;
		WBUFB(buf,26) = 3;
		clif_send(buf, packet_db[0x8a].len, &sd->bl, AREA);
		break;
	}
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_Restart(int fd,struct map_session_data *sd, int cmd)
{
	int restarttype=RFIFOB(fd,packet_db[cmd].pos[0]);

	nullpo_retv(sd);

	switch(restarttype){
	case 0x00:
		if(pc_isdead(sd)){
			pc_setstand(sd);
			pc_setrestartvalue(sd,3);
			pc_setpos(sd,sd->status.save_point.map,sd->status.save_point.x,sd->status.save_point.y,2);
		}
		break;
	case 0x01:
		if(!pc_isdead(sd) && (sd->opt1 || (sd->opt2 && !(night_flag == 1 && sd->opt2 == STATE_BLIND))))
			return;

		/*	Rovert's Prevent logout option - Fixed [Valaris]	*/
		if ((battle_config.prevent_logout && (gettick() - sd->canlog_tick) >= 10000) || (!battle_config.prevent_logout)) {
			chrif_charselectreq(sd);
		} else {
			WFIFOW(fd,0)=0x18b;
			WFIFOW(fd,2)=1;
			WFIFOSET(fd,packet_db[0x18b].len);
			return;
		}
		chrif_charselectreq(sd);
		break;
	}
}

/*==========================================
 * Transmission of a wisp (S 0096 <len>.w <nick>.24B <message>.?B)
 *------------------------------------------
 */
void clif_parse_Wis(int fd,struct map_session_data *sd, int cmd)
{
	int len=RFIFOW(fd,packet_db[cmd].pos[0]);

	if( sd && sd->sc_data && 
		(sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd->sc_data[SC_NOCHAT].timer!=-1 ) )	//チャット禁止
		return;

	intif_wis_message(sd,RFIFOP(fd,packet_db[cmd].pos[1]),RFIFOP(fd,packet_db[cmd].pos[2]),len-packet_db[cmd].pos[2]);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_GMmessage(int fd, struct map_session_data *sd, int cmd) {
// /b
	int len;
	nullpo_retv(sd);

	len=RFIFOW(fd,packet_db[cmd].pos[0]);
	if (battle_config.atc_gmonly == 0 ||
		pc_isGM(sd) >= get_atcommand_level(AtCommand_Broadcast))
		intif_GMmessage(RFIFOP(fd,packet_db[cmd].pos[1]),len-packet_db[cmd].pos[1],0);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_TakeItem(int fd,struct map_session_data *sd, int cmd)
{
	struct flooritem_data *fitem;
	int map_object_id;

	nullpo_retv(sd);

	map_object_id = RFIFOL(fd,packet_db[cmd].pos[0]);
	fitem=(struct flooritem_data*)map_id2bl(map_object_id);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}

	if( sd->npc_id!=0 || sd->vender_id != 0 || sd->opt1 > 0 ||
		(sd->sc_data && (sd->sc_data[SC_AUTOCOUNTER].timer!=-1 || //オートカウンター
		sd->sc_data[SC_BLADESTOP].timer!=-1 || //白刃取り
		sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク
		sd->sc_data[SC_NOCHAT].timer!=-1 )) )	//会話禁止
		return;

	if(fitem==NULL || fitem->bl.m != sd->bl.m)
		return;

	pc_takeitem(sd,fitem);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_DropItem(int fd, struct map_session_data *sd, int cmd) {
	int item_index, item_amount;

	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}
	if( sd->npc_id!=0 || sd->vender_id != 0 || sd->opt1 > 0 ||
		(sd->sc_data && (sd->sc_data[SC_AUTOCOUNTER].timer!=-1 || //オートカウンター
		sd->sc_data[SC_BLADESTOP].timer!=-1 || //白刃取り
		sd->sc_data[SC_BERSERK].timer!=-1)) ) //バーサーク
		return;

	item_index = RFIFOW(fd,packet_db[cmd].pos[0])-2;
	item_amount = RFIFOW(fd,packet_db[cmd].pos[1]);

	pc_dropitem(sd,item_index,item_amount);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_UseItem(int fd, struct map_session_data *sd, int cmd) {
	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}
	if( sd->npc_id!=0 || sd->vender_id != 0 || sd->opt1 > 0 ||
		(sd->sc_data && (sd->sc_data[SC_TRICKDEAD].timer != -1 || //死んだふり
		sd->sc_data[SC_BLADESTOP].timer!=-1 ||	//白刃取り
		sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク
		sd->sc_data[SC_NOCHAT].timer!=-1 )) )	//会話禁止
		return;


	if (sd->invincible_timer != -1)
		pc_delinvincibletimer(sd);

	pc_useitem(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_EquipItem(int fd,struct map_session_data *sd, int cmd)
{
	int index;

	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}
	index = RFIFOW(fd,packet_db[cmd].pos[0])-2;
	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	if(sd->sc_data && ( sd->sc_data[SC_BLADESTOP].timer!=-1 || sd->sc_data[SC_BERSERK].timer!=-1 )) return;

	if(sd->status.inventory[index].identify != 1) {		// 未鑑定
		clif_equipitemack(sd,index,0,0);	// fail
		return;
	}

	if(sd->status.inventory[index].attribute != 0) {		// 破壊されている
		clif_equipitemack(sd,index,0,0);	// fail
		return;
	}
	//ペット用装備であるかないか
	if(sd->inventory_data[index]) {
		if(sd->inventory_data[index]->type != 8){
			if(sd->inventory_data[index]->type == 10)
				RFIFOW(fd,packet_db[cmd].pos[1])=0x8000;	// 矢を無理やり装備できるように（－－；
			pc_equipitem(sd,index,RFIFOW(fd,packet_db[cmd].pos[1]));
		} else
			pet_equipitem(sd,index);
	}
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_UnequipItem(int fd,struct map_session_data *sd, int cmd)
{
	int index;

	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}

	index = packet_db[cmd].pos[0]-2;
	if(sd->status.inventory[index].attribute == 1 && sd->sc_data && sd->sc_data[SC_BROKNWEAPON].timer!=-1)
		skill_status_change_end(&sd->bl,SC_BROKNWEAPON,-1);
	if(sd->status.inventory[index].attribute == 1 && sd->sc_data && sd->sc_data[SC_BROKNARMOR].timer!=-1)
		skill_status_change_end(&sd->bl,SC_BROKNARMOR,-1);
	if(sd->sc_data && ( sd->sc_data[SC_BLADESTOP].timer!=-1 || sd->sc_data[SC_BERSERK].timer!=-1 ))
		return;

	if(sd->npc_id!=0 || sd->vender_id != 0 || sd->opt1 > 0) return;
	pc_unequipitem(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,0);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcClicked(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(pc_isdead(sd)) {
		clif_clearchar_area(&sd->bl,1);
		return;
	}
	if(sd->npc_id!=0 || sd->vender_id != 0)
		return;
	npc_click(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcBuySellSelected(int fd,struct map_session_data *sd, int cmd)
{
	npc_buysellsel(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOB(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcBuyListSend(int fd,struct map_session_data *sd, int cmd)
{
	int fail=0,n;
	unsigned short *item_list;

	n = (RFIFOW(fd,packet_db[cmd].pos[0])-4) /4;
	item_list = (unsigned short*)RFIFOP(fd,packet_db[cmd].pos[1]);

	fail = npc_buylist(sd,n,item_list);

	WFIFOW(fd,0)=0xca;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_db[0xca].len);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcSellListSend(int fd,struct map_session_data *sd, int cmd)
{
	int fail=0,n;
	unsigned short *item_list;

	n = (RFIFOW(fd,packet_db[cmd].pos[0])-4) /4;
	item_list = (unsigned short*)RFIFOP(fd,packet_db[cmd].pos[1]);

	fail = npc_selllist(sd,n,item_list);

	WFIFOW(fd,0)=0xcb;
	WFIFOB(fd,2)=fail;
	WFIFOSET(fd,packet_db[0xcb].len);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_CreateChatRoom(int fd,struct map_session_data *sd, int cmd)
{
	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 4){
		int len = RFIFOW(fd,packet_db[cmd].pos[0]);
		chat_createchat(sd,RFIFOW(fd,packet_db[cmd].pos[1]),RFIFOB(fd,packet_db[cmd].pos[2]),RFIFOP(fd,packet_db[cmd].pos[3]),RFIFOP(fd,packet_db[cmd].pos[4]),len-packet_db[cmd].pos[4]);
	}
	else
		clif_skill_fail(sd,1,0,3);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ChatAddMember(int fd,struct map_session_data *sd, int cmd)
{
	chat_joinchat(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOP(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ChatRoomStatusChange(int fd,struct map_session_data *sd, int cmd)
{
	int len = RFIFOW(fd,packet_db[cmd].pos[0]);
	chat_changechatstatus(sd,RFIFOW(fd,packet_db[cmd].pos[1]),RFIFOB(fd,packet_db[cmd].pos[2]),RFIFOP(fd,packet_db[cmd].pos[3]),RFIFOP(fd,packet_db[cmd].pos[4]),len-packet_db[cmd].pos[4]);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ChangeChatOwner(int fd,struct map_session_data *sd, int cmd)
{
	chat_changechatowner(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_KickFromChat(int fd,struct map_session_data *sd, int cmd)
{
	chat_kickchat(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_ChatLeave(int fd,struct map_session_data *sd, int cmd)
{
	chat_leavechat(sd);
}

/*==========================================
 * 取引要請を相手に送る
 *------------------------------------------
 */
void clif_parse_TradeRequest(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(map[sd->bl.m].flag.notrade) return;

	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 1){
		trade_traderequest(sd,RFIFOL(sd->fd,packet_db[cmd].pos[0]));
	}
	else
		clif_skill_fail(sd,1,0,0);
}

/*==========================================
 * 取引要請
 *------------------------------------------
 */
void clif_parse_TradeAck(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(map[sd->bl.m].flag.notrade) return;

	trade_tradeack(sd,RFIFOB(sd->fd,packet_db[cmd].pos[0]));
}

/*==========================================
 * アイテム追加
 *------------------------------------------
 */
void clif_parse_TradeAddItem(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	trade_tradeadditem(sd,RFIFOW(sd->fd,packet_db[cmd].pos[0]),RFIFOL(sd->fd,packet_db[cmd].pos[1]));
}

/*==========================================
 * アイテム追加完了(ok押し)
 *------------------------------------------
 */
void clif_parse_TradeOk(int fd,struct map_session_data *sd, int cmd)
{
	trade_tradeok(sd);
}

/*==========================================
 * 取引キャンセル
 *------------------------------------------
 */
void clif_parse_TradeCancel(int fd,struct map_session_data *sd, int cmd)
{
	trade_tradecancel(sd);
}

/*==========================================
 * 取引許諾(trade押し)
 *------------------------------------------
 */
void clif_parse_TradeCommit(int fd,struct map_session_data *sd, int cmd)
{
	trade_tradecommit(sd);
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_StopAttack(int fd,struct map_session_data *sd, int cmd)
{
	pc_stopattack(sd);
}

/*==========================================
 * カートへアイテムを移す
 *------------------------------------------
 */
void clif_parse_PutItemToCart(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(sd->npc_id!=0 || sd->vender_id != 0)
		return;
	pc_putitemtocart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,RFIFOL(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * カートからアイテムを出す
 *------------------------------------------
 */
void clif_parse_GetItemFromCart(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	pc_getitemfromcart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,RFIFOL(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 * 付属品(鷹,ペコ,カート)をはずす
 *------------------------------------------
 */
void clif_parse_RemoveOption(int fd,struct map_session_data *sd, int cmd)
{
	if(pc_isriding(sd)) {	// jobchange when removing peco [Valaris]
		if(sd->status.class==13)
			sd->status.class=sd->view_class=7;

		if(sd->status.class==21)
			sd->status.class=sd->view_class=14;

		if(sd->status.class==4014)
			sd->status.class=sd->view_class=4008;

		if(sd->status.class==4022)
			sd->status.class=sd->view_class=4015;
	}

	pc_setoption(sd,0);
}

/*==========================================
 * チェンジカート
 *------------------------------------------
 */
void clif_parse_ChangeCart(int fd,struct map_session_data *sd, int cmd)
{
	pc_setcart(sd,RFIFOW(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 * ステータスアップ
 *------------------------------------------
 */
void clif_parse_StatusUp(int fd,struct map_session_data *sd, int cmd)
{
	pc_statusup(sd,RFIFOW(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 * スキルレベルアップ
 *------------------------------------------
 */
void clif_parse_SkillUp(int fd,struct map_session_data *sd, int cmd)
{
	pc_skillup(sd,RFIFOW(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 * スキル使用（ID指定）
 *------------------------------------------
 */
void clif_parse_UseSkillToId(int fd, struct map_session_data *sd, int cmd) {
	int skillnum, skilllv,lv, target_id;
	unsigned int tick=gettick();
	struct block_list *bl;

	nullpo_retv(sd);

	if(map[sd->bl.m].flag.noskill) return;
	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	if(sd->chatID) return;

	skilllv = RFIFOW(fd,packet_db[cmd].pos[0]);
	skillnum = RFIFOW(fd,packet_db[cmd].pos[1]);
	target_id = RFIFOL(fd,packet_db[cmd].pos[2]);

	bl=map_id2bl(target_id);
	if(bl && mob_gvmobcheck(sd, bl) == 0)
		return;

        if(skillnotok(skillnum, sd))
            return;

	if (sd->skilltimer != -1) {
		if (skillnum != SA_CASTCANCEL)
			return;
	} else if (DIFF_TICK(tick, sd->canact_tick) < 0) {
		clif_skill_fail(sd, skillnum, 4, 0);
		return;
	}

	if ((sd->sc_data[SC_TRICKDEAD].timer != -1 && skillnum != NV_TRICKDEAD) || 
		sd->sc_data[SC_BERSERK].timer != -1 || sd->sc_data[SC_NOCHAT].timer!=-1 ||
			sd->sc_data[SC_WEDDING].timer!=-1 || sd->view_class==22)
		return;
	if (sd->invincible_timer != -1)
		pc_delinvincibletimer(sd);
	if(sd->skillitem >= 0 && sd->skillitem == skillnum) {
		if(skilllv != sd->skillitemlv)
			skilllv = sd->skillitemlv;
		skill_use_id(sd,target_id,skillnum,skilllv);
	} else {
		sd->skillitem = sd->skillitemlv = -1;
		if(skillnum == MO_EXTREMITYFIST) {
			if((sd->sc_data[SC_COMBO].timer == -1 || (sd->sc_data[SC_COMBO].val1 != MO_COMBOFINISH && sd->sc_data[SC_COMBO].val1 != CH_CHAINCRUSH))) {
				if(!sd->state.skill_flag ) {
					sd->state.skill_flag = 1;
					clif_skillinfo(sd,MO_EXTREMITYFIST,1,-1);
					return;
				} else if (sd->bl.id == target_id) {
					clif_skillinfo(sd, MO_EXTREMITYFIST, 1, -1);
					return;
				}
			}
		}
		if( (lv = pc_checkskill(sd,skillnum)) > 0) {
			if(skilllv > lv)
				skilllv = lv;
			skill_use_id(sd,target_id,skillnum,skilllv);
			if(sd->state.skill_flag)
				sd->state.skill_flag = 0;
		}
	}
}

/*==========================================
 * スキル使用（場所指定）
 *------------------------------------------
 */
void clif_parse_UseSkillToPos(int fd, struct map_session_data *sd, int cmd) {
	int skillnum, skilllv, lv, x, y;
	unsigned int tick = gettick();

	nullpo_retv(sd);

	if(map[sd->bl.m].flag.noskill) return;
	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	if(sd->chatID) return;

	skilllv = RFIFOW(fd,packet_db[cmd].pos[0]);
	skillnum = RFIFOW(fd,packet_db[cmd].pos[1]);
	x = RFIFOW(fd,packet_db[cmd].pos[2]);
	y = RFIFOW(fd,packet_db[cmd].pos[3]);

	if(packet_db[cmd].pos[4]){
		if(pc_issit(sd)){
			clif_skill_fail(sd,skillnum,0,0);
			return;
		}
		memcpy(sd->message,RFIFOP(fd,packet_db[cmd].pos[4]),80);
	}

	if(sd->skilltimer != -1)
		return;
	else if(DIFF_TICK(tick , sd->canact_tick) < 0) {
		clif_skill_fail(sd,skillnum,4,0);
		return;
	}

	if((sd->sc_data[SC_TRICKDEAD].timer != -1 && skillnum != NV_TRICKDEAD) || 
		sd->sc_data[SC_BERSERK].timer!=-1 || sd->sc_data[SC_NOCHAT].timer!=-1 ||
			sd->sc_data[SC_WEDDING].timer!=-1 || sd->view_class==22)
		return;
	if (sd->invincible_timer != -1)
		pc_delinvincibletimer(sd);
	if(sd->skillitem >= 0 && sd->skillitem == skillnum) {
		if(skilllv != sd->skillitemlv)
			skilllv = sd->skillitemlv;
		skill_use_pos(sd,x,y,skillnum,skilllv);
	} else {
		sd->skillitem = sd->skillitemlv = -1;
		if( (lv = pc_checkskill(sd,skillnum)) > 0) {
			if(skilllv > lv)
				skilllv = lv;
			skill_use_pos(sd,x,y,skillnum,skilllv);
		}
	}
}

/*==========================================
 * スキル使用（map指定）
 *------------------------------------------
 */
void clif_parse_UseSkillMap(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);
	
	if(map[sd->bl.m].flag.noskill) return;
	if(sd->chatID) return;

	if(sd->npc_id!=0 || sd->vender_id != 0 || (sd->sc_data && 
		(sd->sc_data[SC_TRICKDEAD].timer != -1 ||
		sd->sc_data[SC_BERSERK].timer!=-1 ||
		sd->sc_data[SC_NOCHAT].timer!=-1 ||
		sd->sc_data[SC_WEDDING].timer!=-1 ||
		sd->view_class==22)))
		return;

	if(sd->invincible_timer != -1)
		pc_delinvincibletimer(sd);

	skill_castend_map(sd,RFIFOW(fd,packet_db[cmd].pos[0]),RFIFOP(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * メモ要求
 *------------------------------------------
 */
void clif_parse_RequestMemo(int fd,struct map_session_data *sd, int cmd)
{
	pc_memo(sd,-1);
}
/*==========================================
 * アイテム合成
 *------------------------------------------
 */
void clif_parse_ProduceMix(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	sd->state.produce_flag = 0;
	skill_produce_mix(sd,RFIFOW(fd,packet_db[cmd].pos[0]),RFIFOW(fd,packet_db[cmd].pos[1]),RFIFOW(fd,packet_db[cmd].pos[2]),RFIFOW(fd,packet_db[cmd].pos[3]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcSelectMenu(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	sd->npc_menu=RFIFOB(fd,packet_db[cmd].pos[1]);
	npc_scriptcont(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcNextClicked(int fd,struct map_session_data *sd, int cmd)
{
	npc_scriptcont(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcAmountInput(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

#define RFIFOL_(fd,pos) (*(int*)(session[fd]->rdata+session[fd]->rdata_pos+(pos)))
	//Input Value overflow Exploit FIX
	sd->npc_amount=RFIFOL(fd,packet_db[cmd].pos[1]); //fixed by Lupus. npc_amount is (int) but was RFIFOL changing it to (unsigned int)
#undef RFIFOL_

	npc_scriptcont(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcStringInput(int fd,struct map_session_data *sd, int cmd)
{
	int len = RFIFOW(fd,packet_db[cmd].pos[0]);

	nullpo_retv(sd);

	if(len-packet_db[cmd].pos[2]+1 >= sizeof(sd->npc_str)){
		printf("clif: input string too long !\n");
		memcpy(sd->npc_str,RFIFOP(fd,packet_db[cmd].pos[2]),sizeof(sd->npc_str));
		sd->npc_str[sizeof(sd->npc_str)-1]=0;
	}else
		strncpy(sd->npc_str,RFIFOP(fd,packet_db[cmd].pos[2]),256);
	npc_scriptcont(sd,RFIFOL(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 *
 *------------------------------------------
 */
void clif_parse_NpcCloseClicked(int fd,struct map_session_data *sd, int cmd)
{
	npc_scriptcont(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

/*==========================================
 * アイテム鑑定
 *------------------------------------------
 */
void clif_parse_ItemIdentify(int fd,struct map_session_data *sd, int cmd)
{
	pc_item_identify(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2);
}
/*==========================================
 * 矢作成
 *------------------------------------------
 */
void clif_parse_SelectArrow(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	sd->state.make_arrow_flag = 0;
	skill_arrow_create(sd,RFIFOW(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * オートスペル受信
 *------------------------------------------
 */
void clif_parse_AutoSpell(int fd,struct map_session_data *sd, int cmd)
{
	skill_autospell(sd,RFIFOW(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * カード使用
 *------------------------------------------
 */
void clif_parse_UseCard(int fd,struct map_session_data *sd, int cmd)
{
	clif_use_card(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2);
}
/*==========================================
 * カード挿入装備選択
 *------------------------------------------
 */
void clif_parse_InsertCard(int fd,struct map_session_data *sd, int cmd)
{
	pc_insert_card(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,RFIFOW(fd,packet_db[cmd].pos[1])-2);
}

/*==========================================
 * 0193 キャラID名前引き
 *------------------------------------------
 */
void clif_parse_SolveCharName(int fd,struct map_session_data *sd, int cmd)
{
	int char_id;

	char_id = RFIFOL(fd,packet_db[cmd].pos[0]);
	clif_solved_charname(sd,char_id);
}

/*==========================================
 * 0197 /resetskill /resetstate
 *------------------------------------------
 */
void clif_parse_ResetChar(int fd, struct map_session_data *sd, int cmd) {
	nullpo_retv(sd);

	if (battle_config.atc_gmonly == 0 || pc_isGM(sd) >= get_atcommand_level(AtCommand_ResetState)) {
		switch(RFIFOW(fd,packet_db[cmd].pos[0])){
		case 0:
			pc_resetstate(sd);
			break;
		case 1:
			pc_resetskill(sd);
			break;
		}
	}
}

/*==========================================
 * 019c /lb等
 *------------------------------------------
 */
void clif_parse_LGMmessage(int fd,struct map_session_data *sd, int cmd)
{
	int len = RFIFOW(fd,packet_db[cmd].pos[0]);

	nullpo_retv(sd);

	if ((battle_config.atc_gmonly == 0 || pc_isGM(sd)) &&
	    (pc_isGM(sd) >= get_atcommand_level(AtCommand_LocalBroadcast))) {
		WFIFOW(fd,0)=0x9a;
		WFIFOW(fd,2)=len;
		memcpy(WBUFP(fd,4), RFIFOP(fd,packet_db[cmd].pos[1]), len-packet_db[cmd].pos[1]);
		clif_send(WFIFOP(fd,0), len, &sd->bl, ALL_SAMEMAP);
	}
}

/*==========================================
 * カプラ倉庫へ入れる
 *------------------------------------------
 */
void clif_parse_MoveToKafra(int fd, struct map_session_data *sd, int cmd) {
	int item_index, item_amount;

	nullpo_retv(sd);

	if (sd->npc_id != 0 || sd->vender_id != 0)
		return;
	item_index = RFIFOW(fd,packet_db[cmd].pos[0])-2;
	item_amount = RFIFOL(fd,packet_db[cmd].pos[1]);

	if(item_index < 0 || item_index > MAX_INVENTORY)	return;
	if(item_amount <=0 )	return;
	if(itemdb_isdropable(sd->status.inventory[item_index].nameid) == 0) return;

	if (sd->state.storage_flag)
		storage_guild_storageadd(sd, item_index, item_amount);
	else
		storage_storageadd(sd, item_index, item_amount);
}

/*==========================================
 * カプラ倉庫から出す
 *------------------------------------------
 */
void clif_parse_MoveFromKafra(int fd,struct map_session_data *sd, int cmd)
{
	int item_index,item_amount;

	nullpo_retv(sd);

	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	item_index = RFIFOW(fd,packet_db[cmd].pos[0])-1;
	item_amount = RFIFOL(fd,packet_db[cmd].pos[1]);

	if(sd->state.storage_flag)
		storage_guild_storageget(sd,item_index,item_amount);
	else
		storage_storageget(sd,item_index,item_amount);
}

/*==========================================
 * カプラ倉庫へカートから入れる
 *------------------------------------------
 */
void clif_parse_MoveToKafraFromCart(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if (sd->npc_id != 0 || sd->vender_id != 0 || sd->trade_partner != 0) return;

	if(sd->state.storage_flag)
		storage_guild_storageaddfromcart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,RFIFOL(fd,packet_db[cmd].pos[1]));
	else
		storage_storageaddfromcart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2,RFIFOL(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 * カプラ倉庫から出す
 *------------------------------------------
 */
void clif_parse_MoveFromKafraToCart(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(sd->npc_id!=0 || sd->vender_id != 0) return;
	if(sd->state.storage_flag)
		storage_guild_storagegettocart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-1,RFIFOL(fd,packet_db[cmd].pos[1]));
	else
		storage_storagegettocart(sd,RFIFOW(fd,packet_db[cmd].pos[0])-1,RFIFOL(fd,packet_db[cmd].pos[1]));
}

/*==========================================
 * カプラ倉庫を閉じる
 *------------------------------------------
 */
void clif_parse_CloseKafra(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(sd->state.storage_flag)
		storage_guild_storageclose(sd);
	else
		storage_storageclose(sd);
}

/*==========================================
 * パーティを作る
 *------------------------------------------
 */
void clif_parse_CreateParty(int fd, struct map_session_data *sd, int cmd) {
	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 7){
		party_create(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
	}
	else
		clif_skill_fail(sd,1,0,4);
}

/*==========================================
 * パーティを作る
 *------------------------------------------
 */
void clif_parse_CreateParty2(int fd,struct map_session_data *sd, int cmd)
{
	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 7){
		party_create(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
	}
	else
		clif_skill_fail(sd,1,0,4);
}

/*==========================================
 * パーティに勧誘
 *------------------------------------------
 */
void clif_parse_PartyInvite(int fd,struct map_session_data *sd, int cmd)
{
	party_invite(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * パーティ勧誘返答
 *------------------------------------------
 */
void clif_parse_ReplyPartyInvite(int fd,struct map_session_data *sd, int cmd)
{
	if(battle_config.basic_skill_check == 0 || pc_checkskill(sd,NV_BASIC) >= 5){
		party_reply_invite(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]));
	}
	else {
		party_reply_invite(sd,RFIFOL(fd,packet_db[cmd].pos[0]),-1);
		clif_skill_fail(sd,1,0,4);
	}
}
/*==========================================
 * パーティ脱退要求
 *------------------------------------------
 */
void clif_parse_LeaveParty(int fd,struct map_session_data *sd, int cmd)
{
	party_leave(sd);
}
/*==========================================
 * パーティ除名要求
 *------------------------------------------
 */
void clif_parse_RemovePartyMember(int fd,struct map_session_data *sd, int cmd)
{
	party_removemember(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOP(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * パーティ設定変更要求
 *------------------------------------------
 */
void clif_parse_PartyChangeOption(int fd,struct map_session_data *sd, int cmd)
{
	party_changeoption(sd,RFIFOW(fd,packet_db[cmd].pos[0]),RFIFOW(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * パーティメッセージ送信要求
 *------------------------------------------
 */
void clif_parse_PartyMessage(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if (is_atcommand(fd, sd, RFIFOP(fd, packet_db[cmd].pos[1]),0) != AtCommand_None)
		return;
	if(sd->sc_data &&
		(sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd->sc_data[SC_NOCHAT].timer!=-1))		//チャット禁止
		return;

	party_send_message(sd,RFIFOP(fd,packet_db[cmd].pos[1]),RFIFOW(fd,packet_db[cmd].pos[0])-packet_db[cmd].pos[1]);
}

/*==========================================
 * 露店閉鎖
 *------------------------------------------
 */
void clif_parse_CloseVending(int fd,struct map_session_data *sd, int cmd)
{
	vending_closevending(sd);
}

/*==========================================
 * 露店アイテムリスト要求
 *------------------------------------------
 */
void clif_parse_VendingListReq(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	vending_vendinglistreq(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
	if(sd->npc_id)
		npc_event_dequeue(sd);
}

/*==========================================
 * 露店アイテム購入
 *------------------------------------------
 */
void clif_parse_PurchaseReq(int fd,struct map_session_data *sd, int cmd)
{
	vending_purchasereq(sd,RFIFOW(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]),RFIFOP(fd,packet_db[cmd].pos[2]));
}

/*==========================================
 * 露店開設
 *------------------------------------------
 */
void clif_parse_OpenVending(int fd,struct map_session_data *sd, int cmd)
{
	vending_openvending(sd,RFIFOW(fd,packet_db[cmd].pos[0]),RFIFOP(fd,packet_db[cmd].pos[1]),RFIFOB(fd,packet_db[cmd].pos[2]),RFIFOP(fd,packet_db[cmd].pos[3]));
}

/*==========================================
 * ギルドを作る
 *------------------------------------------
 */
void clif_parse_CreateGuild(int fd,struct map_session_data *sd, int cmd)
{
	guild_create(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * ギルドマスターかどうか確認
 *------------------------------------------
 */
void clif_parse_GuildCheckMaster(int fd,struct map_session_data *sd, int cmd)
{
	clif_guild_masterormember(sd);
}
/*==========================================
 * ギルド情報要求
 *------------------------------------------
 */
void clif_parse_GuildRequestInfo(int fd,struct map_session_data *sd, int cmd)
{
	switch(RFIFOL(fd,packet_db[cmd].pos[0])){
	case 0:	// ギルド基本情報、同盟敵対情報
		clif_guild_basicinfo(sd);
		clif_guild_allianceinfo(sd);
		break;
	case 1:	// メンバーリスト、役職名リスト
		clif_guild_positionnamelist(sd);
		clif_guild_memberlist(sd);
		break;
	case 2:	// 役職名リスト、役職情報リスト
		clif_guild_positionnamelist(sd);
		clif_guild_positioninfolist(sd);
		break;
	case 3:	// スキルリスト
		clif_guild_skillinfo(sd);
		break;
	case 4:	// 追放リスト
		clif_guild_explusionlist(sd);
		break;
	default:
		if(battle_config.error_log)
			printf("clif: guild request info: unknown type %d\n",RFIFOL(fd,packet_db[cmd].pos[0]));
		break;
	}
}
/*==========================================
 * ギルド役職変更
 *------------------------------------------
 */
void clif_parse_GuildChangePositionInfo(int fd,struct map_session_data *sd, int cmd)
{
	int i;
	for(i=4;i<RFIFOW(fd,packet_db[cmd].pos[0]);i+=40){
		guild_change_position(sd,RFIFOL(fd,i),
			RFIFOL(fd,i+4),RFIFOL(fd,i+12),RFIFOP(fd,i+16));
	}
}
/*==========================================
 * ギルドメンバ役職変更
 *------------------------------------------
 */
void clif_parse_GuildChangeMemberPosition(int fd,struct map_session_data *sd, int cmd)
{
	int i;

	nullpo_retv(sd);

	for(i=4;i<RFIFOW(fd,packet_db[cmd].pos[0]);i+=12){
		guild_change_memberposition(sd->status.guild_id,
			RFIFOL(fd,i),RFIFOL(fd,i+4),RFIFOL(fd,i+8));
	}
}

/*==========================================
 * ギルドエンブレム要求
 *------------------------------------------
 */
void clif_parse_GuildRequestEmblem(int fd,struct map_session_data *sd, int cmd)
{
	struct guild *g=guild_search(RFIFOL(fd,packet_db[cmd].pos[0]));
	if(g!=NULL)
		clif_guild_emblem(sd,g);
}

/*==========================================
 * ギルドエンブレム変更
 *------------------------------------------
 */
void clif_parse_GuildChangeEmblem(int fd,struct map_session_data *sd, int cmd)
{
	guild_change_emblem(sd,RFIFOW(fd,packet_db[cmd].pos[0])-packet_db[cmd].pos[1],RFIFOP(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * ギルド告知変更
 *------------------------------------------
 */
void clif_parse_GuildChangeNotice(int fd,struct map_session_data *sd, int cmd)
{
	guild_change_notice(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOP(fd,packet_db[cmd].pos[1]),RFIFOP(fd,packet_db[cmd].pos[2]));
}

/*==========================================
 * ギルド勧誘
 *------------------------------------------
 */
void clif_parse_GuildInvite(int fd,struct map_session_data *sd, int cmd)
{
	guild_invite(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * ギルド勧誘返信
 *------------------------------------------
 */
void clif_parse_GuildReplyInvite(int fd,struct map_session_data *sd, int cmd)
{
	guild_reply_invite(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOB(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * ギルド脱退
 *------------------------------------------
 */
void clif_parse_GuildLeave(int fd,struct map_session_data *sd, int cmd)
{
	guild_leave(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]),RFIFOL(fd,packet_db[cmd].pos[2]),RFIFOP(fd,packet_db[cmd].pos[3]));
}
/*==========================================
 * ギルド追放
 *------------------------------------------
 */
void clif_parse_GuildExplusion(int fd,struct map_session_data *sd, int cmd)
{
	guild_explusion(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]),RFIFOL(fd,packet_db[cmd].pos[2]),RFIFOP(fd,packet_db[cmd].pos[3]));
}

/*==========================================
 * ギルド会話
 *------------------------------------------
 */
void clif_parse_GuildMessage(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if (is_atcommand(fd, sd, RFIFOP(fd, packet_db[cmd].pos[1]), 0) != AtCommand_None)
		return;
	if(sd->sc_data &&
		(sd->sc_data[SC_BERSERK].timer!=-1 ||	//バーサーク時は会話も不可
		sd->sc_data[SC_NOCHAT].timer!=-1))		//チャット禁止
		return;

	guild_send_message(sd,RFIFOP(fd,packet_db[cmd].pos[1]),RFIFOW(fd,packet_db[cmd].pos[0])-packet_db[cmd].pos[1]);
}

/*==========================================
 * ギルド同盟要求
 *------------------------------------------
 */
void clif_parse_GuildRequestAlliance(int fd,struct map_session_data *sd, int cmd)
{
	guild_reqalliance(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * ギルド同盟要求返信
 *------------------------------------------
 */
void clif_parse_GuildReplyAlliance(int fd,struct map_session_data *sd, int cmd)
{
	guild_reply_reqalliance(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * ギルド関係解消
 *------------------------------------------
 */
void clif_parse_GuildDelAlliance(int fd,struct map_session_data *sd, int cmd)
{
	guild_delalliance(sd,RFIFOL(fd,packet_db[cmd].pos[0]),RFIFOL(fd,packet_db[cmd].pos[1]));
}
/*==========================================
 * ギルド敵対
 *------------------------------------------
 */
void clif_parse_GuildOpposition(int fd,struct map_session_data *sd, int cmd)
{
	guild_opposition(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}
/*==========================================
 * ギルド解散
 *------------------------------------------
 */
void clif_parse_GuildBreak(int fd,struct map_session_data *sd, int cmd)
{
	guild_break(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
}

// pet
void clif_parse_PetMenu(int fd,struct map_session_data *sd, int cmd)
{
	pet_menu(sd,RFIFOB(fd,packet_db[cmd].pos[0]));
}
void clif_parse_CatchPet(int fd,struct map_session_data *sd, int cmd)
{
	pet_catch_process2(sd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

void clif_parse_SelectEgg(int fd,struct map_session_data *sd, int cmd)
{
	pet_select_egg(sd,RFIFOW(fd,packet_db[cmd].pos[0])-2);
}

void clif_parse_SendEmotion(int fd,struct map_session_data *sd, int cmd)
{
	nullpo_retv(sd);

	if(sd->pd)
		clif_pet_emotion(sd->pd,RFIFOL(fd,packet_db[cmd].pos[0]));
}

void clif_parse_ChangePetName(int fd,struct map_session_data *sd, int cmd)
{
	pet_change_name(sd,RFIFOP(fd,packet_db[cmd].pos[0]));
}

// Kick (right click menu for GM "(name) force to quit")
void clif_parse_GMKick(int fd,struct map_session_data *sd, int cmd)
{
	struct block_list *target;
	int tid = RFIFOL(fd,packet_db[cmd].pos[0]);

	nullpo_retv(sd);

	if ((battle_config.atc_gmonly == 0 || pc_isGM(sd)) &&
	    (pc_isGM(sd) >= get_atcommand_level(AtCommand_Kick))) {
		target = map_id2bl(tid);
		if(target) {
			if(target->type == BL_PC) {
				struct map_session_data *tsd = (struct map_session_data *)target;
				if(pc_isGM(sd) > pc_isGM(tsd))
					clif_GM_kick(sd,tsd,1);
				else
					clif_GM_kickack(sd,0);
			} else if (target->type == BL_MOB) {
				struct mob_data *md = (struct mob_data *)target;
				sd->state.attack_type = 0;
				mob_damage(&sd->bl,md,md->hp,2);
			} else
				clif_GM_kickack(sd, 0);
		} else
			clif_GM_kickack(sd, 0);
	}
}

/*==========================================
 * GM funtions
 *------------------------------------------
 */
void clif_parse_GMHide(int fd,struct map_session_data *sd, int cmd) {
	nullpo_retv(sd);

	if(pc_isGM(sd) >= get_atcommand_level(AtCommand_Hide)) {
		if(sd->status.option&0x40){
			sd->status.option&=~0x40;
			clif_displaymessage(fd,"invisible off!");
		}else{
			sd->status.option|=0x40;
			clif_displaymessage(fd,"invisible!");
		}
		clif_changeoption(&sd->bl);
	}
}

/*==========================================
 * GMによるチャット禁止時間付与
 *------------------------------------------
 */
void clif_parse_GMReqNoChat(int fd,struct map_session_data *sd, int cmd)
{
	int tid = RFIFOL(fd,packet_db[cmd].pos[0]);
	int type = RFIFOB(fd,packet_db[cmd].pos[1]);
	int limit = RFIFOW(fd,packet_db[cmd].pos[2]);
	struct block_list *bl = map_id2bl(tid);
	struct map_session_data *dstsd;
	int dstfd;

	nullpo_retv(sd);

	if(!battle_config.muting_players) {
		clif_displaymessage(fd, "Muting is disabled.");
		return;
	}

	if(type == 0)
		limit = 0 - limit;
	if(bl->type == BL_PC && (dstsd =(struct map_session_data *)bl)){
		if((tid == bl->id && type == 2 && !pc_isGM(sd)) || (pc_isGM(sd) > pc_isGM(dstsd)) ){
			dstfd = dstsd->fd;
			WFIFOW(dstfd,0)=0x14b;
			WFIFOB(dstfd,2)=(type==2)?1:type;
			memcpy(WFIFOP(dstfd,3),sd->status.name,24);
			WFIFOSET(dstfd,packet_db[0x14b].len);
			dstsd->status.manner -= limit;
			if(dstsd->status.manner < 0)
				skill_status_change_start(bl,SC_NOCHAT,0,0,0,0,0,0);
			else{
				dstsd->status.manner = 0;
				skill_status_change_end(bl,SC_NOCHAT,-1);
			}
		printf("name:%s type:%d limit:%d manner:%d\n",dstsd->status.name,type,limit,dstsd->status.manner);
		}
	}

	return;
}
/*==========================================
 * GMによるチャット禁止時間参照（？）
 *------------------------------------------
 */
void clif_parse_GMReqNoChatCount(int fd,struct map_session_data *sd, int cmd)
{
	int tid = RFIFOL(fd,packet_db[cmd].pos[0]);

	WFIFOW(fd,0)=0x1e0;
	WFIFOL(fd,2)=tid;
	snprintf(WFIFOP(fd,6),24,"%d",tid);
//	memcpy(WFIFOP(fd,6),"TESTNAME",24);
	WFIFOSET(fd,packet_db[0x1e0].len);

	return;
}

int monk(struct map_session_data *sd, struct block_list *target, int type) {
//R 01d1 <Monk id>L <Target monster id>L <Bool>L
	int fd=sd->fd;
	WFIFOW(fd,0)=0x1d1;
	WFIFOL(fd,2)=sd->bl.id;
	WFIFOL(fd,6)=target->id;
	WFIFOL(fd,10)=type;
	WFIFOSET(fd,packet_db[0x1d1].len);

	return 0;
}

/*==========================================
 * スパノビの/doridoriによるSPR2倍
 *------------------------------------------
 */
void clif_parse_sn_doridori(int fd, struct map_session_data *sd, int cmd) {
	if (sd)
		sd->doridori_counter = 1;

	return;
}
/*==========================================
 * スパノビの爆裂波動
 *------------------------------------------
 */
void clif_parse_sn_explosionspirits(int fd, struct map_session_data *sd, int cmd) 
{
	if(sd){
		int nextbaseexp=pc_nextbaseexp(sd);
		struct pc_base_job s_class = pc_calc_base_job(sd->status.class);
		if (battle_config.etc_log){
			if(nextbaseexp != 0)
				printf("SuperNovice explosionspirits!! %d %d %d %d\n",sd->bl.id,s_class.job,sd->status.base_exp,(int)((double)1000*sd->status.base_exp/nextbaseexp));
			else
				printf("SuperNovice explosionspirits!! %d %d %d 000\n",sd->bl.id,s_class.job,sd->status.base_exp);
		}
		if(s_class.job == 23 && sd->status.base_exp > 0 && nextbaseexp > 0 && (int)((double)1000*sd->status.base_exp/nextbaseexp)%100==0){
			clif_skill_nodamage(&sd->bl,&sd->bl,MO_EXPLOSIONSPIRITS,5,1);
			skill_status_change_start(&sd->bl,SkillStatusChangeTable[MO_EXPLOSIONSPIRITS],5,0,0,0,skill_get_time(MO_EXPLOSIONSPIRITS,5),0 );
		}
	}
	return;
}

/*==========================================
 * Friends List (davidsiaw)
 *------------------------------------------
 */
void clif_friends_list_send(struct map_session_data *sd)
{	
	unsigned char buf[1024];
	int i,n=0;
	
	//  Packet name
	WBUFW(buf,0)=0x201;

	// check all 20 friends. stop at 0
	for (i=0;i<20;i++)
	{	
		if (!sd->status.friend_id[i])
			break;

		WBUFL(buf,4 + 32 * n + 1)=sd->status.friend_id[i];
		//WBUFB(buf,4 + 32 * n + 5)=(online[n])?0:1; // <- We don't know this yet. I'd reckon its 5 but... i could be wrong.
		
		memcpy(WBUFP(buf,4 + 32 * n + 8),&sd->status.friend_name[i],23);
		
		n++;
	}

	WBUFW(buf,2) = 4 + 32 * n;

	memcpy(WFIFOP(sd->fd,0), buf, sizeof(buf));
	WFIFOSET(sd->fd,sizeof(buf));
	//clif_send(buf,WBUFW(buf,2),&sd->bl,SELF);

}
void clif_parse_friends_list_add(int fd,struct map_session_data *sd)
{
	struct map_session_data *f_sd = map_nick2sd(RFIFOP(fd,2));

	int i=0;

	while (sd->status.friend_id[i]) { // Find an empty slot

		if (sd->status.friend_id[i] == f_sd->status.char_id) {
			clif_displaymessage(fd,"Friend already exists");  // Friend already exists
			return;
		}
		i++;
		if (i==20) {
			clif_displaymessage(fd,"Friends list is full.");
			return;
		}

	}
		
	sd->status.friend_id[i] = f_sd->status.char_id;
	memcpy(sd->status.friend_name[i],f_sd->status.name,23);
	clif_displaymessage(fd,"Friend added.");

	for (i=i+1; i<20; i++)
	{
		sd->status.friend_id[i] = 0;
	}

		clif_friends_list_send(sd);

	//printf("clif_parse_friends_list_add");
	
}
void clif_parse_friends_list_remove(int fd,struct map_session_data *sd)
{
	// 0x203 </o> <ID to be removed W 4B>
	
	int id = RFIFOL(fd,3);
	char mes[80];

	int i=0;


	while (sd->status.friend_id[i] != id)
	{
		sprintf(mes, "Compare: %d and %d", id, sd->status.friend_id[i]);
		clif_displaymessage(fd,mes);
		i++;
		if (i==20)
		{
			clif_displaymessage (fd,"Name not found in list");
			clif_friends_list_send(sd);
			return;
		}
	}

	sd->status.friend_id[i] = 0;
	memcpy(sd->status.friend_name[i],"",23);

	// move all chars up

	for (i=i+1; i<20; i++)
	{	
		if(sd->status.friend_name[i])
		{
			sd->status.friend_id[i] = sd->status.friend_id[i-1];
			memcpy(sd->status.friend_name[i],sd->status.friend_name[i-1],23);
		}
		else
		{
			sd->status.friend_id[i] = 0;
			memcpy(sd->status.friend_name[i],"",23);
		}

	}

	clif_friends_list_send(sd);
	
}

/*==========================================
 * Whisper Ignoring [ /ex /in ]
 *------------------------------------------
 */
int pstrcmp(const void *a, const void *b)
{
	return strcmp((char *)a, (char *)b);
}
void clif_parse_wisexin(int fd,struct map_session_data *sd, int cmd)
{
	int type = RFIFOB(fd,packet_db[cmd].pos[1]);
	int i,flag=1;

	if(sd){
		qsort(sd->wis_refusal[0], MAX_WIS_REFUSAL, sizeof(sd->wis_refusal[0]), pstrcmp);
		if(type==0){	//ex
			for(i=0;i<MAX_WIS_REFUSAL;i++){	//すでに追加されていれば何もしない
				if(strcmp(sd->wis_refusal[i],RFIFOP(fd,packet_db[cmd].pos[0]))==0){
					flag=0;
					clif_wisexin(sd, type, flag);
					return;
				}
			}
			for(i=0;i<MAX_WIS_REFUSAL;i++){	//空の拒否リストに追加(とりあえず)
				if(sd->wis_refusal[i][0]==0){
					memcpy(sd->wis_refusal[i],RFIFOP(fd,packet_db[cmd].pos[0]),24);
					flag=0;
					break;
				}
			}
			if(flag==1){
					memcpy(sd->wis_refusal[MAX_WIS_REFUSAL-1],RFIFOP(fd,packet_db[cmd].pos[0]),24);
					flag=0;
			}
		}else{		//in
			for(i=0;i<MAX_WIS_REFUSAL;i++){	//一致する拒否リストを空に
				if(strcmp(sd->wis_refusal[i],RFIFOP(fd,packet_db[cmd].pos[0]))==0){
					sd->wis_refusal[i][0]=0;
					flag=0;
				}
			}
			sd->wis_all = 0;
		}
		clif_wisexin(sd, type, flag);
	}
	return;
}

/*==========================================
 * View Whisper List [ /ex ]
 *------------------------------------------
 */
void clif_parse_wisexlist(int fd,struct map_session_data *sd, int cmd)
{
	int i,j=0,count=0;

	if(sd){
		qsort(sd->wis_refusal[0], MAX_WIS_REFUSAL, sizeof(sd->wis_refusal[0]), pstrcmp);
		for(i=0;i<MAX_WIS_REFUSAL;i++){	//中身があるのを数える
			if(sd->wis_refusal[i][0]!=0)
				count++;
		}
		WFIFOW(fd,0)=0xd4;
		WFIFOW(fd,2)=4+(24*count);
		for(i=0;i<MAX_WIS_REFUSAL;i++){
			if(sd->wis_refusal[i][0]!=0){
				memcpy(WFIFOP(fd,4+j*24),sd->wis_refusal[i],24);
				j++;
			}
		}
		WFIFOSET(fd,WFIFOW(fd,2));
		if(count>=MAX_WIS_REFUSAL)	//満タンなら最後の1個を消す
			sd->wis_refusal[MAX_WIS_REFUSAL-1][0]=0;
	}

	return;
}

/*==========================================
 * Block and Allow all Whispers /exall /inall
 *------------------------------------------
 */
void clif_parse_wisall(int fd,struct map_session_data *sd, int cmd)
{
	int type = RFIFOB(fd,packet_db[cmd].pos[0]);
	int i;

	if(sd){
		if(type==0)	//exall
			sd->wis_all = 1;
		else{		//inall
			for(i=0;i<MAX_WIS_REFUSAL;i++)	//拒否リストを空に
				sd->wis_refusal[i][0]=0;
			sd->wis_all = 0;
		}
		clif_wisall(sd, type, 0);
	}
}

/*==========================================
 * /killall
 *------------------------------------------
 */
void clif_parse_GMkillall(int fd,struct map_session_data *sd, int cmd)
{
	char message[50];
	
	nullpo_retv(sd);
	
	strncpy(message,sd->status.name,24);
	is_atcommand(fd,sd,strcat(message," : @kickall"),0);
	return;
}

/*==========================================
 * /summon
 *------------------------------------------
 */
void clif_parse_GMsummon(int fd,struct map_session_data *sd, int cmd)
{
	char message[100];
	
	nullpo_retv(sd);
	
	strncpy(message,sd->status.name,24);
	strcat(message," : @recall ");
	strncat(message,RFIFOP(fd,packet_db[cmd].pos[0]),24);
	is_atcommand(fd,sd,message,0);
	return;
}
/*==========================================
 * /shift
 *------------------------------------------
 */
void clif_parse_GMshift(int fd,struct map_session_data *sd, int cmd)
{
	char message[100];
	
	nullpo_retv(sd);
	
	strncpy(message,sd->status.name,24);
	strcat(message," : @jumpto ");
	strncat(message,RFIFOP(fd,packet_db[cmd].pos[0]),24);
	is_atcommand(fd,sd,message,0);
	return;
}
/*==========================================
 * Debugging
 *------------------------------------------
 */
void clif_parse_debug(int fd,struct map_session_data *sd, int cmd)
{
	int i;

	printf("packet debug 0x%4X\n",cmd);
	printf("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
	for(i=0;i<packet_db[cmd].len;i++){
		if((i&15)==0)
			printf("\n%04X ",i);
		printf("%02X ",RFIFOB(fd,i));
	}
	printf("\n");
}

/*==========================================
 * クライアントからのパケット解析
 * socket.cのdo_parsepacketから呼び出される
 *------------------------------------------
 */
static int clif_parse(int fd) {
	int packet_len = 0, cmd;
	struct map_session_data *sd;
	
	sd = session[fd]->session_data;

	// 接続が切れてるので後始末
	if (!chrif_isconnect())
		session[fd]->eof = 1;
	if(session[fd]->eof) { // char鯖に繋がってない間は接続禁止 (!chrif_isconnect())
		if (sd && sd->state.auth) {
			clif_quitsave(fd, sd);
			if(sd->status.name!=NULL && battle_config.etc_log)
				printf("Player [%s] Has Logged Off Your Server.\n",sd->status.name);	// Player logout display [Valaris]
		} else if (sd) { // not authentified! (refused by char-server or disconnect before to be authentified)
//			printf("Player with account [%d] has logged off your server (not auth account).\n", sd->bl.id); // Player logout display [Yor]
			map_deliddb(&sd->bl); // account_id has been included in the DB before auth answer
		}
		close(fd);
		delete_session(fd);
		return 0;
	}

	if(RFIFOREST(fd)<2)
		return 0;

	cmd = RFIFOW(fd,0);

	// 管理用パケット処理
	if (cmd >= 30000) {
		switch(cmd) {
		case 0x7530: // Athena情報所得
			WFIFOW(fd,0) = 0x7531;
			WFIFOB(fd,2) = ATHENA_MAJOR_VERSION;
			WFIFOB(fd,3) = ATHENA_MINOR_VERSION;
			WFIFOB(fd,4) = ATHENA_REVISION;
			WFIFOB(fd,5) = ATHENA_RELEASE_FLAG;
			WFIFOB(fd,6) = ATHENA_OFFICIAL_FLAG;
			WFIFOB(fd,7) = ATHENA_SERVER_MAP;
			WFIFOW(fd,8) = ATHENA_MOD_VERSION;
			WFIFOSET(fd,10);
			RFIFOSKIP(fd,2);
			break;
		case 0x7532: // 接続の切断
			close(fd);
			session[fd]->eof = 1;
			break;
		}
		return 0;
	}

	// ゲーム用以外パケットか、認証を終える前に0072以外が来たら、切断する
	if(cmd>=MAX_PACKET_DB || packet_db[cmd].len==0 ||
	   ((!sd || (sd && sd->state.auth==0)) && packet_db[cmd].func!=clif_parse_WantToConnection) ){

		close(fd);
		session[fd]->eof = 1;
		if(packet_db[cmd].len==0) {
			printf("clif_parse : %d %d %x\n",fd,packet_db[cmd].len,cmd);
			printf("%x length 0 packet disconnect %d\n",cmd,fd);
		}
		return 0;
	}

	// パケット長を計算
	packet_len = packet_db[cmd].len;
	if(packet_len==-1){
		if(RFIFOREST(fd)<4)
			return 0;	// 可変長パケットで長さの所までデータが来てない
		packet_len = RFIFOW(fd,2);
		if(packet_len<4 || packet_len>32768){
			close(fd);
			session[fd]->eof =1;
			return 0;
		}
	}
	if (RFIFOREST(fd) < packet_len)
		return 0; // まだ1パケット分データが揃ってない

	if (sd && sd->state.auth == 1 && sd->state.waitingdisconnect == 1) { // 切断待ちの場合パケットを処理しない

	}else if(packet_db[cmd].func){
		// パケット処理
		packet_db[cmd].func(fd,sd,cmd);
	} else {
		// 不明なパケット
		if (battle_config.error_log) {
			if (fd)
				printf("\nclif_parse: session #%d, packet 0x%x, lenght %d\n", fd, cmd, packet_len);
#ifdef DUMP_UNKNOWN_PACKET
			{
				int i;
				FILE *fp;
				char packet_txt[256] = "save/packet.txt";
				time_t now;
				printf("---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
				for(i=0;i<packet_len;i++){
					if((i&15)==0)
						printf("\n%04X ",i);
					printf("%02X ",RFIFOB(fd,i));
				}
				if (sd && sd->state.auth) {
					if (sd->status.name != NULL)
						printf("\nAccount ID %d, character ID %d, player name %s.\n",
						       sd->status.account_id, sd->status.char_id, sd->status.name);
					else
						printf("\nAccount ID %d.\n", sd->bl.id);
				} else if (sd) // not authentified! (refused by char-server or disconnect before to be authentified)
					printf("\nAccount ID %d.\n", sd->bl.id);

				if ((fp = fopen(packet_txt, "a")) == NULL) {
					printf("clif.c: cant write [%s] !!! data is lost !!!\n", packet_txt);
					return 1;
				} else {
					time(&now);
					if (sd && sd->state.auth) {
						if (sd->status.name != NULL)
							fprintf(fp, "%sPlayer with account ID %d (character ID %d, player name %s) sent wrong packet:\n",
							        asctime(localtime(&now)), sd->status.account_id, sd->status.char_id, sd->status.name);
						else
							fprintf(fp, "%sPlayer with account ID %d sent wrong packet:\n", asctime(localtime(&now)), sd->bl.id);
					} else if (sd) // not authentified! (refused by char-server or disconnect before to be authentified)
						fprintf(fp, "%sPlayer with account ID %d sent wrong packet:\n", asctime(localtime(&now)), sd->bl.id);

					fprintf(fp, "\t---- 00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F");
					for(i = 0; i < packet_len; i++) {
						if ((i & 15) == 0)
							fprintf(fp, "\n\t%04X ", i);
						fprintf(fp, "%02X ", RFIFOB(fd,i));
					}
					fprintf(fp, "\n\n");
					fclose(fp);
				}
			}
#endif
		}
	}
	RFIFOSKIP(fd, packet_len);

	return 0;
}

/*==========================================
 * Read the Packet Database file.
 *------------------------------------------
 */
static int packetdb_readdb(void)
{
	FILE *fp;
	char line[1024];
	int ln=0;
	int cmd,j;
	char *str[32],*p,*str2[32],*p2;

	memset(packet_db,0,sizeof(packet_db));

	if( (fp=fopen("db/packet_db.txt","r"))==NULL ){
		printf("can't read db/packet_db.txt\n");
		exit(1);
	}
	while(fgets(line,1020,fp)){
		if(line[0]=='/' && line[1]=='/')
			continue;
		memset(str,0,sizeof(str));
		for(j=0,p=line;j<4 && p;j++){
			str[j]=p;
			p=strchr(p,',');
			if(p) *p++=0;
		}
		if(str[0]==NULL)
			continue;

		cmd=strtol(str[0],(char **)NULL,0);
		if(cmd<=0 || cmd>=MAX_PACKET_DB)
			continue;

		if(str[1]==NULL){
			printf("packet_db: packet len error\n");
			exit(1);
		}
		packet_db[cmd].len = atoi(str[1]);

		if(str[2]==NULL){
			ln++;
			continue;
		}
		for(j=0;j<sizeof(clif_parse_func)/sizeof(clif_parse_func[0]);j++){
			if(clif_parse_func[j].name == NULL){
				printf("packet_db: %d 0x%x no func %s\n",ln+1,cmd,str[2]);
				exit(1);
			}
			if( strcmp(str[2],clif_parse_func[j].name)==0){
				packet_db[cmd].func=clif_parse_func[j].func;
				break;
			}
		}
		if(str[3]==NULL){
			printf("packet_db: packet error\n");
			exit(1);
		}
		for(j=0,p2=str[3];p2;j++){
			str2[j]=p2;
			p2=strchr(p2,':');
			if(p2) *p2++=0;
			packet_db[cmd].pos[j]=atoi(str2[j]);
		}

		ln++;
//		if(packet_db[cmd].len > 2 && packet_db[cmd].pos[0] == 0)
//			printf("packet_db:? %d 0x%x %d %s %p\n",ln,cmd,packet_db[cmd].len,str[2],packet_db[cmd].func);
	}
	fclose(fp);
	printf("read db/packet_db.txt done (Packets=%d)\n",ln);
	return 0;

}

/*==========================================
 *
 *------------------------------------------
 */
int do_final_clif(void)
{
	delete_session(map_fd);
	return 0;
}

/*==========================================
 *
 *------------------------------------------
 */
int do_init_clif(void) {
	int i;

	packetdb_readdb();
	set_defaultparse(clif_parse);
	for(i = 0; i < 10; i++) {
		if (make_listen_port(map_port))
			break;
#ifdef _WIN32
		Sleep(20000);
#else
		sleep(20);
#endif
	}
	if(i==10){
		printf("cant bind game port\n");
		exit(1);
	}

	add_timer_func_list(clif_waitclose, "clif_waitclose");
	add_timer_func_list(clif_clearchar_delay_sub, "clif_clearchar_delay_sub");

	return 0;
}


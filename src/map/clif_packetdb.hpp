// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_PACKETDB_HPP
#define CLIF_PACKETDB_HPP

	#define packet(cmd,length) packetdb_addpacket(cmd,length,NULL,0)
	#define parseable_packet(cmd,length,func,...) packetdb_addpacket(cmd,length,func,__VA_ARGS__,0)
	#define ack_packet(type,cmd,length,...) \
		packetdb_addpacket(cmd,length,NULL,__VA_ARGS__,0); \
		packet_db_ack[type] = cmd

	packet(0x0064,55);
	packet(0x0065,17);
	packet(0x0066,6);
	packet(0x0067,37);
	packet(0x0068,46);
	packet(0x0069,-1);
	packet(0x006a,23);
	packet(0x006b,-1);
	packet(0x006c,3);
	packet(0x006d,108);
	packet(0x006e,3);
	packet(0x006f,2);
	packet(0x0070,6);
	packet(0x0071,28);
	parseable_packet(0x0072,19,clif_parse_WantToConnection,2,6,10,14,18);
	packet(0x0073,11);
	packet(0x0074,3);
	packet(0x0075,-1);
	packet(0x0076,9);
	packet(0x0077,5);
	packet(0x0078,54);
	packet(0x0079,53);
	packet(0x007a,58);
	packet(0x007b,60);
	packet(0x007c,41);
	parseable_packet(0x007d,2,clif_parse_LoadEndAck,0);
	parseable_packet(0x007e,6,clif_parse_TickSend,2);
	packet(0x007f,6);
	packet(0x0080,7);
	packet(0x0081,3);
	packet(0x0082,2);
	packet(0x0083,2);
	packet(0x0084,2);
	parseable_packet(0x0085,5,clif_parse_WalkToXY,2);
	packet(0x0086,16);
	packet(0x0087,12);
	packet(0x0088,10);
	parseable_packet(0x0089,7,clif_parse_ActionRequest,2,6);
	packet(0x008a,29);
	packet(0x008b,2);
	parseable_packet(0x008c,-1,clif_parse_GlobalMessage,2,4);
	packet(0x008d,-1);
	packet(0x008e,-1);
	//packet(0x008f,-1);
	parseable_packet(0x0090,7,clif_parse_NpcClicked,2,6);
	packet(0x0091,22);
	packet(0x0092,28);
	packet(0x0093,2);
	parseable_packet(0x0094,6,clif_parse_GetCharNameRequest,2);
	packet( HEADER_ZC_ACK_REQNAME_TITLE, sizeof( PACKET_ZC_ACK_REQNAME_TITLE ) );
	parseable_packet(0x0096,-1,clif_parse_WisMessage,2,4,28);
	packet(0x0097,-1);
	packet(0x0098,3);
	parseable_packet(0x0099,-1,clif_parse_Broadcast,2,4);
	packet(0x009a,-1);
	parseable_packet(0x009b,5,clif_parse_ChangeDir,2,4);
	packet(0x009c,9);
	packet( HEADER_ZC_ITEM_ENTRY, sizeof( struct PACKET_ZC_ITEM_ENTRY ) );
	packet(0x009e,17);
	parseable_packet(0x009f,6,clif_parse_TakeItem,2);
	packet( additemType, sizeof( struct packet_additem ) );
	packet(0x00a1,6);
	parseable_packet(0x00a2,6,clif_parse_DropItem,2,4);
	packet( inventorylistnormalType, -1 );
	packet( inventorylistequipType, -1 );
	packet( storageListNormalType, -1 );
	packet( storageListEquipType, -1 );
	parseable_packet(0x00a7,8,clif_parse_UseItem,2,4);
	packet( useItemAckType, sizeof( struct PACKET_ZC_USE_ITEM_ACK ) );
	parseable_packet(0x00a9,6,clif_parse_EquipItem,2,4);
	ack_packet(ZC_WEAR_EQUIP_ACK,0x00aa,7,2,4,6);
	parseable_packet(0x00ab,4,clif_parse_UnequipItem,2);
	packet(0x00ac,7);
	//packet(0x00ad,-1);
	packet(0x00ae,-1);
	packet(0x00af,6);
	packet(0x00b0,8);
	packet(0x00b1,8);
	parseable_packet(0x00b2,3,clif_parse_Restart,2);
	packet(0x00b3,3);
	packet(0x00b4,-1);
	packet(0x00b5,6);
	packet(0x00b6,6);
	packet(0x00b7,-1);
	parseable_packet(0x00b8,7,clif_parse_NpcSelectMenu,2,6);
	parseable_packet(0x00b9,6,clif_parse_NpcNextClicked,2);
	packet(0x00ba,2);
	parseable_packet(0x00bb,5,clif_parse_StatusUp,2,4);
	packet(0x00bc,6);
	packet(0x00bd,44);
	packet(0x00be,5);
	parseable_packet(0x00bf,3,clif_parse_Emotion,2);
	packet(0x00c0,7);
	parseable_packet(0x00c1,2,clif_parse_HowManyConnections,0);
	packet(0x00c2,6);
	packet(0x00c3,8);
	packet(0x00c4,6);
	parseable_packet(0x00c5,7,clif_parse_NpcBuySellSelected,2,6);
	packet(0x00c6,-1);
	packet(0x00c7,-1);
	parseable_packet(0x00c8,-1,clif_parse_NpcBuyListSend,2,4);
	parseable_packet(0x00c9,-1,clif_parse_NpcSellListSend,2,4);
	packet(0x00ca,3);
	packet(0x00cb,3);
	parseable_packet(0x00cc,6,clif_parse_GMKick,2);
	packet(0x00cd,3);
	parseable_packet(0x00ce,2,clif_parse_GMKickAll,0);
	parseable_packet(0x00cf,27,clif_parse_PMIgnore,2,26);
	parseable_packet(0x00d0,3,clif_parse_PMIgnoreAll,2);
	packet(0x00d1,4);
	packet(0x00d2,4);
	parseable_packet(0x00d3,2,clif_parse_PMIgnoreList,0);
	packet(0x00d4,-1);
	parseable_packet(0x00d5,-1,clif_parse_CreateChatRoom,2,4,6,7,15);
	packet(0x00d6,3);
	packet(0x00d7,-1);
	packet(0x00d8,6);
	parseable_packet(0x00d9,14,clif_parse_ChatAddMember,2,6);
	packet(0x00da,3);
	packet(0x00db,-1);
	packet(0x00dc,28);
	packet(0x00dd,29);
	parseable_packet(0x00de,-1,clif_parse_ChatRoomStatusChange,2,4,6,7,15);
	packet(0x00df,-1);
	parseable_packet(0x00e0,30,clif_parse_ChangeChatOwner,2,6);
	packet(0x00e1,30);
	parseable_packet(0x00e2,26,clif_parse_KickFromChat,2);
	parseable_packet(0x00e3,2,clif_parse_ChatLeave,0);
	parseable_packet(0x00e4,6,clif_parse_TradeRequest,2);
	packet(0x00e5,26);
	parseable_packet(0x00e6,3,clif_parse_TradeAck,2);
	packet(0x00e7,3);
	parseable_packet(0x00e8,8,clif_parse_TradeAddItem,2,4);
	packet( tradeaddType, sizeof( struct PACKET_ZC_ADD_EXCHANGE_ITEM ) );
	packet(0x00ea,5);
	parseable_packet(0x00eb,2,clif_parse_TradeOk,0);
	packet(0x00ec,3);
	parseable_packet(0x00ed,2,clif_parse_TradeCancel,0);
	packet(0x00ee,2);
	parseable_packet(0x00ef,2,clif_parse_TradeCommit,0);
	packet(0x00f0,3);
	packet(0x00f1,2);
	packet(0x00f2,6);
	parseable_packet(0x00f3,8,clif_parse_MoveToKafra,2,4);
	packet( storageaddType, sizeof( struct PACKET_ZC_ADD_ITEM_TO_STORE ) );
	parseable_packet(0x00f5,8,clif_parse_MoveFromKafra,2,4);
	packet(0x00f6,8);
	parseable_packet(0x00f7,2,clif_parse_CloseKafra,0);
	packet(0x00f8,2);
	parseable_packet(0x00f9,26,clif_parse_CreateParty,2);
	packet(0x00fa,3);
	packet(0x00fb,-1);
	parseable_packet(0x00fc,6,clif_parse_PartyInvite,2);
	packet(0x00fd,27);
	packet(0x00fe,30);
	parseable_packet(0x00ff,10,clif_parse_ReplyPartyInvite,2,6);
	parseable_packet(0x0100,2,clif_parse_LeaveParty,0);
	packet(0x0101,6);
	parseable_packet(0x0102,6,clif_parse_PartyChangeOption,2);
	parseable_packet(0x0103,30,clif_parse_RemovePartyMember,2,6);
	packet(0x0104,79);
	packet(0x0105,31);
	packet(0x0106,10);
	packet(0x0107,10);
	parseable_packet(0x0108,-1,clif_parse_PartyMessage,2,4);
	packet(0x0109,-1);
	packet( HEADER_ZC_MVP_GETTING_ITEM, sizeof( struct PACKET_ZC_MVP_GETTING_ITEM ) );
	packet(0x010b,6);
	packet(0x010c,6);
	packet(0x010d,2);
	packet(0x010e,11);
	packet(0x010f,-1);
	packet( HEADER_ZC_ACK_TOUSESKILL, sizeof( PACKET_ZC_ACK_TOUSESKILL ) );
	packet(0x0111,39);
	parseable_packet(0x0112,4,clif_parse_SkillUp,2);
	parseable_packet(0x0113,10,clif_parse_UseSkillToId,2,4,6);
	packet(0x0114,31);
	packet(0x0115,35);
	parseable_packet(0x0116,10,clif_parse_UseSkillToPos,2,4,6,8);
	packet(0x0117,18);
	parseable_packet(0x0118,2,clif_parse_StopAttack,0);
	packet(0x0119,13);
	packet(0x011a,15);
	parseable_packet(0x011b,20,clif_parse_UseSkillMap,2,4);
	packet(0x011c,68);
	parseable_packet(0x011d,2,clif_parse_RequestMemo,0);
	packet(0x011e,3);
	packet(0x011f,16);
	packet(0x0120,6);
	packet(0x0121,14);
	packet( cartlistequipType, -1 );
	packet( cartlistnormalType, -1 );
	packet( cartaddType, sizeof( struct PACKET_ZC_ADD_ITEM_TO_CART ) );
	packet(0x0125,8);
	parseable_packet(0x0126,8,clif_parse_PutItemToCart,2,4);
	parseable_packet(0x0127,8,clif_parse_GetItemFromCart,2,4);
	parseable_packet(0x0128,8,clif_parse_MoveFromKafraToCart,2,4);
	parseable_packet(0x0129,8,clif_parse_MoveToKafraFromCart,2,4);
	parseable_packet(0x012a,2,clif_parse_RemoveOption,0);
	packet(0x012b,2);
	packet(0x012c,3);
	packet(0x012d,4);
	parseable_packet(0x012e,2,clif_parse_CloseVending,0);
	parseable_packet(0x012f,-1,clif_parse_OpenVending,2,4,0,84);
	parseable_packet(0x0130,6,clif_parse_VendingListReq,2);
	packet(0x0131,86);
	packet(0x0132,6);
	packet( vendinglistType, -1 );
	parseable_packet(0x0134,-1,clif_parse_PurchaseReq,2,4,8);
	packet(0x0135,7);
	packet(openvendingType,-1);
	packet(0x0137,6);
	packet(0x0138,3);
	packet(0x0139,16);
	packet(0x013a,4);
	packet(0x013b,4);
	packet(0x013c,4);
	packet(0x013d,6);
	packet(0x013e,24);
	parseable_packet(0x013f,26,clif_parse_GM_Item_Monster,2);
	parseable_packet(0x0140,22,clif_parse_MapMove,2,18,20);
	packet(0x0141,14);
	packet(0x0142,6);
	parseable_packet(0x0143,10,clif_parse_NpcAmountInput,2,6);
	packet(0x0144,23);
	packet(0x0145,19);
	parseable_packet(0x0146,6,clif_parse_NpcCloseClicked,2);
	packet(0x0147,39);
	packet(0x0148,8);
	parseable_packet(0x0149,9,clif_parse_GMReqNoChat,2,6,7);
	packet(0x014a,6);
	packet(0x014b,27);
	packet(0x014c,-1);
	parseable_packet(0x014d,2,clif_parse_GuildCheckMaster,0);
	packet(0x014e,6);
	parseable_packet(0x014f,6,clif_parse_GuildRequestInfo,2);
	packet(0x0150,110);
	parseable_packet(0x0151,6,clif_parse_GuildRequestEmblem,2);
	packet(0x0152,-1);
	parseable_packet(0x0153,-1,clif_parse_GuildChangeEmblem,2,4);
	packet(0x0154,-1);
	parseable_packet(0x0155,-1,clif_parse_GuildChangeMemberPosition,2,4);
	packet(0x0156,-1);
	packet(0x0157,6);
	packet(0x0158,-1);
	parseable_packet(0x0159,54,clif_parse_GuildLeave,2,6,10,14);
	packet(0x015a,66);
	parseable_packet(0x015b,54,clif_parse_GuildExpulsion,2,6,10,14);
	packet(0x015c,90);
	parseable_packet(0x015d,42,clif_parse_GuildBreak,2);
	packet(0x015e,6);
	packet(0x015f,42);
	packet(0x0160,-1);
	parseable_packet(0x0161,-1,clif_parse_GuildChangePositionInfo,2,4);
	packet(0x0162,-1);
	packet(0x0163,-1);
	packet(0x0164,-1);
	parseable_packet(0x0165,30,clif_parse_CreateGuild,2,6);
	packet(0x0166,-1);
	packet(0x0167,3);
	parseable_packet(0x0168,14,clif_parse_GuildInvite,2,6,10);
	packet(0x0169,3);
	packet(0x016a,30);
	parseable_packet(0x016b,10,clif_parse_GuildReplyInvite,2,6);
	packet(0x016c,43);
	packet(0x016d,14);
	parseable_packet(0x016e,186,clif_parse_GuildChangeNotice,2,6,66);
	packet(0x016f,182);
	parseable_packet(0x0170,14,clif_parse_GuildRequestAlliance,2,6,10);
	packet(0x0171,30);
	parseable_packet(0x0172,10,clif_parse_GuildReplyAlliance,2,6);
	packet(0x0173,3);
	packet(0x0174,-1);
	packet(0x0175,6);
	packet(0x0176,106);
	packet(0x0177,-1);
	parseable_packet(0x0178,4,clif_parse_ItemIdentify,2);
	packet(0x0179,5);
	parseable_packet(0x017a,4,clif_parse_UseCard,2);
	packet(0x017b,-1);
	parseable_packet(0x017c,6,clif_parse_InsertCard,2,4);
	packet(0x017d,7);
	parseable_packet(0x017e,-1,clif_parse_GuildMessage,2,4);
	packet(0x017f,-1);
	parseable_packet(0x0180,6,clif_parse_GuildOpposition,2);
	packet(0x0181,3);
	packet(0x0182,106);
	parseable_packet(0x0183,10,clif_parse_GuildDelAlliance,2,6);
	packet(0x0184,10);
	packet(0x0185,34);
	//packet(0x0186,-1);
	packet(0x0187,6);
	packet(0x0188,8);
	packet(0x0189,4);
	parseable_packet(0x018a,4,clif_parse_QuitGame,2);
	packet(0x018b,4);
	packet(0x018c,29);
	packet(0x018d,-1);
	parseable_packet( HEADER_CZ_REQMAKINGITEM, sizeof( struct PACKET_CZ_REQMAKINGITEM ), clif_parse_ProduceMix, 0 );
	packet( HEADER_ZC_ACK_REQMAKINGITEM, sizeof( PACKET_ZC_ACK_REQMAKINGITEM ) );
	parseable_packet(0x0190,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	packet( HEADER_ZC_TALKBOX_CHATCONTENTS, sizeof( struct PACKET_ZC_TALKBOX_CHATCONTENTS ) );
	packet(0x0192,24);
	parseable_packet(0x0193,6,clif_parse_SolveCharName,2);
	packet(0x0194,30);
	packet( HEADER_ZC_ACK_REQNAMEALL, sizeof( struct PACKET_ZC_ACK_REQNAMEALL ) );
	packet(0x0196,9);
	parseable_packet(0x0197,4,clif_parse_ResetChar,2);
	parseable_packet(0x0198,8,clif_parse_GMChangeMapType,2,4,6);
	packet(0x0199,4);
	packet(0x019a,14);
	packet(0x019b,10);
	parseable_packet(0x019c,-1,clif_parse_LocalBroadcast,2,4);
	parseable_packet(0x019d,6,clif_parse_GMHide,2);
	packet(0x019e,2);
	parseable_packet(0x019f,6,clif_parse_CatchPet,2);
	packet(0x01a0,3);
	parseable_packet(0x01a1,3,clif_parse_PetMenu,2);
	packet(0x01a2,35);
	packet(0x01a3,5);
	packet(0x01a4,11);
	parseable_packet(0x01a5,26,clif_parse_ChangePetName,2);
	packet(0x01a6,-1);
	parseable_packet(0x01a7,4,clif_parse_SelectEgg,2);
	packet(0x01a8,4);
	parseable_packet(0x01a9,6,clif_parse_SendEmotion,2);
	packet(0x01aa,10);
	packet(0x01ab,12);
	packet(0x01ac,6);
	packet(0x01ad,-1);
	parseable_packet( HEADER_CZ_REQ_MAKINGARROW, sizeof( PACKET_CZ_REQ_MAKINGARROW ), clif_parse_SelectArrow, 0 );
	parseable_packet(0x01af,4,clif_parse_ChangeCart,2);
	packet(0x01b0,11);
	packet(0x01b1,7);
	parseable_packet(0x01b2,-1,clif_parse_OpenVending,2,4,84,85);
	packet(0x01b3,67);
	packet(0x01b4,12);
	packet(0x01b5,18);
	packet(0x01b6,114);
	packet(0x01b7,6);
	packet(0x01b8,3);
	packet(0x01b9,6);
	parseable_packet(0x01ba,26,clif_parse_GMShift,2);
	parseable_packet(0x01bb,26,clif_parse_GMShift,2);
	parseable_packet(0x01bc,26,clif_parse_GMRecall,2);
	parseable_packet(0x01bd,26,clif_parse_GMRecall,2);
	packet(0x01be,2);
	packet(0x01bf,3);
	packet(0x01c0,2);
	packet(0x01c1,14);
	packet(0x01c2,10);
	packet(0x01c3,-1);
	packet(0x01c6,4);
	packet(0x01c7,2);
	packet(0x01c9,97);
	//packet(0x01ca,-1);
	packet(0x01cb,9);
	packet(0x01cc,9);
	packet(0x01cd,30);
	parseable_packet(0x01ce,6,clif_parse_AutoSpell,2);
	packet(0x01cf,28);
	packet(0x01d0,8);
	packet(0x01d1,14);
	packet(0x01d2,10);
	packet(0x01d3,35);
	packet(0x01d4,6);
	parseable_packet(0x01d5,-1,clif_parse_NpcStringInput,2,4,8);
	packet(0x01d6,4);
	packet(0x01d7,11);
	packet(0x01d8,54);
	packet(0x01d9,53);
	packet(0x01da,60);
	packet(0x01db,2);
	packet(0x01dc,-1);
	packet(0x01dd,47);
	packet(0x01de,33);
	parseable_packet(0x01df,6,clif_parse_GMReqAccountName,2);
	packet(0x01e0,30);
	packet(0x01e1,8);
	packet(0x01e2,34);
	packet(0x01e3,14);
	packet(0x01e4,2);
	packet(0x01e5,6);
	packet(0x01e6,26);
	parseable_packet(0x01e7,2,clif_parse_NoviceDoriDori,0);
	parseable_packet(0x01e8,28,clif_parse_CreateParty2,2,26,27);
	packet(0x01e9,81);
	packet(0x01ea,6);
	packet(0x01eb,10);
	packet(0x01ec,26);
	parseable_packet(0x01ed,2,clif_parse_NoviceExplosionSpirits,0);
	packet(0x01f0,-1);
	packet(0x01f1,-1);
	packet(0x01f2,20);
	packet(0x01f3,10);
	packet(0x01f4,32);
	packet(0x01f5,9);
	packet(0x01f6,34);
	parseable_packet(0x01f7,14,clif_parse_Adopt_reply,2,6,10);
	packet(0x01f8,2);
	parseable_packet(0x01f9,6,clif_parse_Adopt_request,2);
	packet(0x01fa,48);
	packet(0x01fb,56);
	packet(0x01fc,-1);
	parseable_packet( HEADER_CZ_REQ_ITEMREPAIR, sizeof( struct PACKET_CZ_REQ_ITEMREPAIR ), clif_parse_RepairItem, 0 );
	packet(0x01fe,5);
	packet(0x01ff,10);
	packet(0x0200,26);
	packet(0x0201,-1);
	parseable_packet(0x0202,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0203,10,clif_parse_FriendsListRemove,2,6);
	packet(0x0204,18);
	packet(0x0205,26);
	packet(0x0206,11);
	packet(0x0207,34);
	parseable_packet(0x0208,11,clif_parse_FriendsListReply,2,6,10);
	packet(0x0209,36);
	packet(0x020a,10);
	//packet(0x020b,-1);
	//packet(0x020c,-1);
	packet(0x020d,-1);
	packet(0x8b3,-1);
	ack_packet(ZC_CLEAR_DIALOG,0x8d6,6,2);

// 2004-07-05aSakexe
#if PACKETVER >= 20040705
	parseable_packet(0x0072,22,clif_parse_WantToConnection,5,9,13,17,21);
	parseable_packet(0x0085,8,clif_parse_WalkToXY,5);
	parseable_packet(0x00a7,13,clif_parse_UseItem,5,9);
	parseable_packet(0x0113,15,clif_parse_UseSkillToId,4,9,11);
	parseable_packet(0x0116,15,clif_parse_UseSkillToPos,4,9,11,13);
	parseable_packet(0x0190,95,clif_parse_UseSkillToPosMoreInfo,4,9,11,13,15);
	parseable_packet(0x0208,14,clif_parse_FriendsListReply,2,6,10);
	packet(0x020e,24);
#endif

// 2004-07-13aSakexe
#if PACKETVER >= 20040713
	parseable_packet(0x0072,39,clif_parse_WantToConnection,12,22,30,34,38);
	parseable_packet(0x0085,9,clif_parse_WalkToXY,6);
	parseable_packet(0x009b,13,clif_parse_ChangeDir,5,12);
	parseable_packet(0x009f,10,clif_parse_TakeItem,6);
	parseable_packet(0x00a7,17,clif_parse_UseItem,6,13);
	parseable_packet(0x0113,19,clif_parse_UseSkillToId,7,9,15);
	parseable_packet(0x0116,19,clif_parse_UseSkillToPos,7,9,15,17);
	parseable_packet(0x0190,99,clif_parse_UseSkillToPosMoreInfo,7,9,15,17,19);
#endif

// 2004-07-26aSakexe
#if PACKETVER >= 20040726
	parseable_packet(0x0072,14,clif_parse_DropItem,5,12);
	parseable_packet(0x007e,33,clif_parse_WantToConnection,12,18,24,28,32);
	parseable_packet(0x0085,20,clif_parse_UseSkillToId,7,12,16);
	parseable_packet(0x0089,15,clif_parse_GetCharNameRequest,11);
	parseable_packet(0x008c,23,clif_parse_UseSkillToPos,3,6,17,21);
	parseable_packet(0x0094,10,clif_parse_TakeItem,6);
	parseable_packet(0x009b,6,clif_parse_WalkToXY,3);
	parseable_packet(0x009f,13,clif_parse_ChangeDir,5,12);
	parseable_packet(0x00a2,103,clif_parse_UseSkillToPosMoreInfo,3,6,17,21,23);
	parseable_packet(0x00a7,12,clif_parse_SolveCharName,8);
	parseable_packet(0x00f3,-1,clif_parse_GlobalMessage,2,4);
	parseable_packet(0x00f5,17,clif_parse_UseItem,6,12);
	parseable_packet(0x00f7,10,clif_parse_TickSend,6);
	parseable_packet(0x0113,16,clif_parse_MoveToKafra,5,12);
	parseable_packet(0x0116,2,clif_parse_CloseKafra,0);
	parseable_packet(0x0190,26,clif_parse_MoveFromKafra,10,22);
	parseable_packet(0x0193,9,clif_parse_ActionRequest,3,8);
#endif

// 2004-08-09aSakexe
#if PACKETVER >= 20040809
	parseable_packet(0x0072,17,clif_parse_DropItem,8,15);
	parseable_packet(0x007e,37,clif_parse_WantToConnection,9,21,28,32,36);
	parseable_packet(0x0085,26,clif_parse_UseSkillToId,11,18,22);
	parseable_packet(0x0089,12,clif_parse_GetCharNameRequest,8);
	parseable_packet(0x008c,40,clif_parse_UseSkillToPos,5,15,29,38);
	parseable_packet(0x0094,13,clif_parse_TakeItem,9);
	parseable_packet(0x009b,15,clif_parse_WalkToXY,12);
	parseable_packet(0x009f,12,clif_parse_ChangeDir,7,11);
	parseable_packet(0x00a2,120,clif_parse_UseSkillToPosMoreInfo,5,15,29,38,40);
	parseable_packet(0x00a7,11,clif_parse_SolveCharName,7);
	parseable_packet(0x00f5,24,clif_parse_UseItem,9,20);
	parseable_packet(0x00f7,13,clif_parse_TickSend,9);
	parseable_packet(0x0113,23,clif_parse_MoveToKafra,5,19);
	parseable_packet(0x0190,26,clif_parse_MoveFromKafra,11,22);
	parseable_packet(0x0193,18,clif_parse_ActionRequest,7,17);
#endif

// 2004-08-16aSakexe
#if PACKETVER >= 20040816
	parseable_packet(0x0212,26,clif_parse_GMRc,2);
	parseable_packet(0x0213,26,clif_parse_Check,2);
	packet(0x0214,42);
#endif

// 2004-08-17aSakexe
#if PACKETVER >= 20040817
	parseable_packet(0x020f,10,clif_parse_PVPInfo,2,6);
	packet(0x0210,22);
#endif

// 2004-09-06aSakexe
#if PACKETVER >= 20040906
	parseable_packet(0x0072,20,clif_parse_UseItem,9,20);
	parseable_packet(0x007e,19,clif_parse_MoveToKafra,3,15);
	parseable_packet(0x0085,23,clif_parse_ActionRequest,9,22);
	parseable_packet(0x0089,9,clif_parse_WalkToXY,6);
	parseable_packet(0x008c,105,clif_parse_UseSkillToPosMoreInfo,10,14,18,23,25);
	parseable_packet(0x0094,17,clif_parse_DropItem,6,15);
	parseable_packet(0x009b,14,clif_parse_GetCharNameRequest,10);
	parseable_packet(0x009f,-1,clif_parse_GlobalMessage,2,4);
	parseable_packet(0x00a2,14,clif_parse_SolveCharName,10);
	parseable_packet(0x00a7,25,clif_parse_UseSkillToPos,10,14,18,23);
	parseable_packet(0x00f3,10,clif_parse_ChangeDir,4,9);
	parseable_packet(0x00f5,34,clif_parse_WantToConnection,7,15,25,29,33);
	parseable_packet(0x00f7,2,clif_parse_CloseKafra,0);
	parseable_packet(0x0113,11,clif_parse_TakeItem,7);
	parseable_packet(0x0116,11,clif_parse_TickSend,7);
	parseable_packet(0x0190,22,clif_parse_UseSkillToId,9,15,18);
	parseable_packet(0x0193,17,clif_parse_MoveFromKafra,3,13);
#endif

// 2004-09-20aSakexe
#if PACKETVER >= 20040920
	parseable_packet(0x0072,18,clif_parse_UseItem,10,14);
	parseable_packet(0x007e,25,clif_parse_MoveToKafra,6,21);
	parseable_packet(0x0085,9,clif_parse_ActionRequest,3,8);
	parseable_packet(0x0089,14,clif_parse_WalkToXY,11);
	parseable_packet(0x008c,109,clif_parse_UseSkillToPosMoreInfo,16,20,23,27,29);
	parseable_packet(0x0094,19,clif_parse_DropItem,12,17);
	parseable_packet(0x009b,10,clif_parse_GetCharNameRequest,6);
	parseable_packet(0x00a2,10,clif_parse_SolveCharName,6);
	parseable_packet(0x00a7,29,clif_parse_UseSkillToPos,6,20,23,27);
	parseable_packet(0x00f3,18,clif_parse_ChangeDir,8,17);
	parseable_packet(0x00f5,32,clif_parse_WantToConnection,10,17,23,27,31);
	parseable_packet(0x0113,14,clif_parse_TakeItem,10);
	parseable_packet(0x0116,14,clif_parse_TickSend,10);
	parseable_packet(0x0190,14,clif_parse_UseSkillToId,4,7,10);
	parseable_packet(0x0193,12,clif_parse_MoveFromKafra,4,8);
#endif

// 2004-10-05aSakexe
#if PACKETVER >= 20041005
	parseable_packet(0x0072,17,clif_parse_UseItem,6,13);
	parseable_packet(0x007e,16,clif_parse_MoveToKafra,5,12);
	parseable_packet(0x0089,6,clif_parse_WalkToXY,3);
	parseable_packet(0x008c,103,clif_parse_UseSkillToPosMoreInfo,2,6,17,21,23);
	parseable_packet(0x0094,14,clif_parse_DropItem,5,12);
	parseable_packet(0x009b,15,clif_parse_GetCharNameRequest,11);
	parseable_packet(0x00a2,12,clif_parse_SolveCharName,8);
	parseable_packet(0x00a7,23,clif_parse_UseSkillToPos,3,6,17,21);
	parseable_packet(0x00f3,13,clif_parse_ChangeDir,5,12);
	parseable_packet(0x00f5,33,clif_parse_WantToConnection,12,18,24,28,32);
	parseable_packet(0x0113,10,clif_parse_TakeItem,6);
	parseable_packet(0x0116,10,clif_parse_TickSend,6);
	parseable_packet(0x0190,20,clif_parse_UseSkillToId,7,12,16);
	parseable_packet(0x0193,26,clif_parse_MoveFromKafra,10,22);
#endif

// 2004-10-25aSakexe
#if PACKETVER >= 20041025
	parseable_packet(0x0072,13,clif_parse_UseItem,5,9);
	parseable_packet(0x007e,13,clif_parse_MoveToKafra,6,9);
	parseable_packet(0x0085,15,clif_parse_ActionRequest,4,14);
	parseable_packet(0x008c,108,clif_parse_UseSkillToPosMoreInfo,6,9,23,26,28);
	parseable_packet(0x0094,12,clif_parse_DropItem,6,10);
	parseable_packet(0x009b,10,clif_parse_GetCharNameRequest,6);
	parseable_packet(0x00a2,16,clif_parse_SolveCharName,12);
	parseable_packet(0x00a7,28,clif_parse_UseSkillToPos,6,9,23,26);
	parseable_packet(0x00f3,15,clif_parse_ChangeDir,6,14);
	parseable_packet(0x00f5,29,clif_parse_WantToConnection,5,14,20,24,28);
	parseable_packet(0x0113,9,clif_parse_TakeItem,5);
	parseable_packet(0x0116,9,clif_parse_TickSend,5);
	parseable_packet(0x0190,26,clif_parse_UseSkillToId,4,10,22);
	parseable_packet(0x0193,22,clif_parse_MoveFromKafra,12,18);
#endif

// 2004-11-01aSakexe
#if PACKETVER >= 20041101
	packet(0x0084,-1);
	packet(0x0215,6);
#endif

// 2004-11-08aSakexe
#if PACKETVER >= 20041108
	packet(0x0084,2);
	packet(0x0216,6);
	parseable_packet(0x0217,2,clif_parse_Blacksmith,0);
	parseable_packet(0x0218,2,clif_parse_Alchemist,0);
	packet(0x0219,282);
	packet(0x021a,282);
	packet(0x021b,10);
	packet(0x021c,10);
#endif

// 2004-11-15aSakexe
#if PACKETVER >= 20041115
	parseable_packet(0x021d,6,clif_parse_LessEffect,2);
#endif

// 2004-11-29aSakexe
#if PACKETVER >= 20041129
	parseable_packet(0x0072,22,clif_parse_UseSkillToId,8,12,18);
	parseable_packet(0x007e,30,clif_parse_UseSkillToPos,4,9,22,28);
	parseable_packet(0x0085,-1,clif_parse_GlobalMessage,2,4);
	parseable_packet(0x0089,7,clif_parse_TickSend,3);
	parseable_packet(0x008c,13,clif_parse_GetCharNameRequest,9);
	parseable_packet(0x0094,14,clif_parse_MoveToKafra,4,10);
	parseable_packet(0x009b,2,clif_parse_CloseKafra,0);
	parseable_packet(0x009f,18,clif_parse_ActionRequest,6,17);
	parseable_packet(0x00a2,7,clif_parse_TakeItem,3);
	parseable_packet(0x00a7,7,clif_parse_WalkToXY,4);
	parseable_packet(0x00f3,8,clif_parse_ChangeDir,3,7);
	parseable_packet(0x00f5,29,clif_parse_WantToConnection,3,10,20,24,28);
	parseable_packet(0x00f7,14,clif_parse_SolveCharName,10);
	parseable_packet(0x0113,110,clif_parse_UseSkillToPosMoreInfo,4,9,22,28,30);
	parseable_packet(0x0116,12,clif_parse_DropItem,4,10);
	parseable_packet(0x0190,15,clif_parse_UseItem,3,11);
	parseable_packet(0x0193,21,clif_parse_MoveFromKafra,4,17);
	packet(0x0221,-1);
	parseable_packet(0x0222,6,clif_parse_WeaponRefine,2);
	packet( HEADER_ZC_ACK_WEAPONREFINE, sizeof( struct PACKET_ZC_ACK_WEAPONREFINE ) );
#endif

// 2004-12-13aSakexe
#if PACKETVER >= 20041213
	//skipped: many packets being set to -1
	packet(0x0066,3);
	packet(0x0070,3);
	packet(0x01ca,3);
	packet(0x021e,6);
	packet(0x021f,66);
	packet(0x0220,10);
#endif

// 2005-01-10bSakexe
#if PACKETVER >= 20050110
	parseable_packet(0x0072,26,clif_parse_UseSkillToId,8,16,22);
	parseable_packet(0x007e,114,clif_parse_UseSkillToPosMoreInfo,10,18,22,32,34);
	parseable_packet(0x0085,23,clif_parse_ChangeDir,12,22);
	parseable_packet(0x0089,9,clif_parse_TickSend,5);
	parseable_packet(0x008c,8,clif_parse_GetCharNameRequest,4);
	parseable_packet(0x0094,20,clif_parse_MoveToKafra,10,16);
	parseable_packet(0x009b,32,clif_parse_WantToConnection,3,12,23,27,31);
	parseable_packet(0x009f,17,clif_parse_UseItem,5,13);
	parseable_packet(0x00a2,11,clif_parse_SolveCharName,7);
	parseable_packet(0x00a7,13,clif_parse_WalkToXY,10);
	parseable_packet(0x00f3,-1,clif_parse_GlobalMessage,2,4);
	parseable_packet(0x00f5,9,clif_parse_TakeItem,5);
	parseable_packet(0x00f7,21,clif_parse_MoveFromKafra,11,17);
	parseable_packet(0x0113,34,clif_parse_UseSkillToPos,10,18,22,32);
	parseable_packet(0x0116,20,clif_parse_DropItem,15,18);
	parseable_packet(0x0190,20,clif_parse_ActionRequest,9,19);
	parseable_packet(0x0193,2,clif_parse_CloseKafra,0);
#endif

// 2005-03-28aSakexe
#if PACKETVER >= 20050328
	packet(0x0224,10);
	parseable_packet(0x0225,2,clif_parse_Taekwon,0);
	packet(0x0226,282);
#endif

// 2005-04-04aSakexe
#if PACKETVER >= 20050404
	packet(0x0227,18);
	packet(0x0228,18);
#endif

// 2005-04-11aSakexe
#if PACKETVER >= 20050411
	packet(0x0229,15);
	packet(0x022a,58);
	packet(0x022b,57);
	packet(0x022c,64);
#endif

// 2005-04-25aSakexe
#if PACKETVER >= 20050425
	parseable_packet(0x022d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0232,9,clif_parse_HomMoveTo,2,6);
	parseable_packet(0x0233,11,clif_parse_HomAttack,2,6,10);
	parseable_packet(0x0234,6,clif_parse_HomMoveToMaster,2);
#endif

// 2005-05-09aSakexe
#if PACKETVER >= 20050509
	parseable_packet(0x0072,25,clif_parse_UseSkillToId,6,10,21);
	parseable_packet(0x007e,102,clif_parse_UseSkillToPosMoreInfo,5,9,12,20,22);
	parseable_packet(0x0085,11,clif_parse_ChangeDir,7,10);
	parseable_packet(0x0089,8,clif_parse_TickSend,4);
	parseable_packet(0x008c,11,clif_parse_GetCharNameRequest,7);
	parseable_packet(0x0094,14,clif_parse_MoveToKafra,7,10);
	parseable_packet(0x009b,26,clif_parse_WantToConnection,4,9,17,21,25);
	parseable_packet(0x009f,14,clif_parse_UseItem,4,10);
	parseable_packet(0x00a2,15,clif_parse_SolveCharName,11);
	parseable_packet(0x00a7,8,clif_parse_WalkToXY,5);
	parseable_packet(0x00f5,8,clif_parse_TakeItem,4);
	parseable_packet(0x00f7,22,clif_parse_MoveFromKafra,14,18);
	parseable_packet(0x0113,22,clif_parse_UseSkillToPos,5,9,12,20);
	parseable_packet(0x0116,10,clif_parse_DropItem,5,8);
	parseable_packet(0x0190,19,clif_parse_ActionRequest,5,18);
#endif

// 2005-05-23aSakexe
#if PACKETVER >= 20050523
	packet(0x022e,69);
	packet(0x0230,12);
#endif

// 2005-05-30aSakexe
#if PACKETVER >= 20050530
	packet(0x022e,71);
	packet(0x0235,-1);
	packet(0x0236,10);
	parseable_packet(0x0237,2,clif_parse_RankingPk,0);
	packet(0x0238,282);
#endif

// 2005-05-31aSakexe
#if PACKETVER >= 20050531
	packet(0x0216,2);
	packet(0x0239,11);
#endif

// 2005-06-08aSakexe
#if PACKETVER >= 20050608
	packet(0x0216,6);
	parseable_packet(0x0217,2,clif_parse_Blacksmith,0);
	packet(0x022f,5);
	parseable_packet(0x0231,26,clif_parse_ChangeHomunculusName,2);
	packet(0x023a,4);
	parseable_packet(0x023b,36,clif_parse_StoragePassword,2,4,20);
	packet(0x023c,6);
#endif

// 2005-06-22aSakexe
#if PACKETVER >= 20050622
	packet(0x022e,71);
#endif

// 2005-06-28aSakexe
#if PACKETVER >= 20050628
	parseable_packet(0x0072,34,clif_parse_UseSkillToId,6,17,30);
	parseable_packet(0x007e,113,clif_parse_UseSkillToPosMoreInfo,12,15,18,31,33);
	parseable_packet(0x0085,17,clif_parse_ChangeDir,8,16);
	parseable_packet(0x0089,13,clif_parse_TickSend,9);
	parseable_packet(0x008c,8,clif_parse_GetCharNameRequest,4);
	parseable_packet(0x0094,31,clif_parse_MoveToKafra,16,27);
	parseable_packet(0x009b,32,clif_parse_WantToConnection,9,15,23,27,31);
	parseable_packet(0x009f,19,clif_parse_UseItem,9,15);
	parseable_packet(0x00a2,9,clif_parse_SolveCharName,5);
	parseable_packet(0x00a7,11,clif_parse_WalkToXY,8);
	parseable_packet(0x00f5,13,clif_parse_TakeItem,9);
	parseable_packet(0x00f7,18,clif_parse_MoveFromKafra,11,14);
	parseable_packet(0x0113,33,clif_parse_UseSkillToPos,12,15,18,31);
	parseable_packet(0x0116,12,clif_parse_DropItem,3,10);
	parseable_packet(0x0190,24,clif_parse_ActionRequest,11,23);
	packet(0x0216,-1);
	packet(0x023d,-1);
	packet(0x023e,4);
#endif

// 2005-07-18aSakexe
#if PACKETVER >= 20050718
	parseable_packet(0x0072,19,clif_parse_UseSkillToId,5,11,15);
	parseable_packet(0x007e,110,clif_parse_UseSkillToPosMoreInfo,9,15,23,28,30);
	parseable_packet(0x0085,11,clif_parse_ChangeDir,6,10);
	parseable_packet(0x0089,7,clif_parse_TickSend,3);
	parseable_packet(0x008c,11,clif_parse_GetCharNameRequest,7);
	parseable_packet(0x0094,21,clif_parse_MoveToKafra,12,17);
	parseable_packet(0x009b,31,clif_parse_WantToConnection,3,13,22,26,30);
	parseable_packet(0x009f,12,clif_parse_UseItem,3,8);
	parseable_packet(0x00a2,18,clif_parse_SolveCharName,14);
	parseable_packet(0x00a7,15,clif_parse_WalkToXY,12);
	parseable_packet(0x00f5,7,clif_parse_TakeItem,3);
	parseable_packet(0x00f7,13,clif_parse_MoveFromKafra,5,9);
	parseable_packet(0x0113,30,clif_parse_UseSkillToPos,9,15,23,28);
	parseable_packet(0x0116,12,clif_parse_DropItem,6,10);
	parseable_packet(0x0190,21,clif_parse_ActionRequest,5,20);
	packet(0x0216,6);
	parseable_packet(0x023f,2,clif_parse_Mail_refreshinbox,0);
	packet(0x0240,8);
	parseable_packet(0x0241,6,clif_parse_Mail_read,2);
	packet(0x0242,-1);
	parseable_packet(0x0243,6,clif_parse_Mail_delete,2);
	parseable_packet(0x0244,6,clif_parse_Mail_getattach,2);
	packet(0x0245,7);
	parseable_packet(0x0246,4,clif_parse_Mail_winopen,2);
	parseable_packet(0x0247,8,clif_parse_Mail_setattach,2,4);
	packet(0x0248,68);
	packet(0x0249,3);
	packet(0x024a,70);
	parseable_packet(0x024b,4,clif_parse_Auction_cancelreg,2);
	parseable_packet(0x024c,8,clif_parse_Auction_setitem,2,4);
	packet(0x024d,14);
	parseable_packet(0x024e,6,clif_parse_Auction_cancel,2);
	parseable_packet(0x024f,10,clif_parse_Auction_bid,2,6);
	packet(0x0250,3);
	packet(0x0251,2);
	packet(0x0252,-1);
#endif

// 2005-07-19bSakexe
#if PACKETVER >= 20050719
	parseable_packet(0x0072,34,clif_parse_UseSkillToId,6,17,30);
	parseable_packet(0x007e,113,clif_parse_UseSkillToPosMoreInfo,12,15,18,31,33);
	parseable_packet(0x0085,17,clif_parse_ChangeDir,8,16);
	parseable_packet(0x0089,13,clif_parse_TickSend,9);
	parseable_packet(0x008c,8,clif_parse_GetCharNameRequest,4);
	parseable_packet(0x0094,31,clif_parse_MoveToKafra,16,27);
	parseable_packet(0x009b,32,clif_parse_WantToConnection,9,15,23,27,31);
	parseable_packet(0x009f,19,clif_parse_UseItem,9,15);
	parseable_packet(0x00a2,9,clif_parse_SolveCharName,5);
	parseable_packet(0x00a7,11,clif_parse_WalkToXY,8);
	parseable_packet(0x00f5,13,clif_parse_TakeItem,9);
	parseable_packet(0x00f7,18,clif_parse_MoveFromKafra,11,14);
	parseable_packet(0x0113,33,clif_parse_UseSkillToPos,12,15,18,31);
	parseable_packet(0x0116,12,clif_parse_DropItem,3,10);
	parseable_packet(0x0190,24,clif_parse_ActionRequest,11,23);
#endif

// 2005-08-01aSakexe
#if PACKETVER >= 20050801
	packet(0x0245,3);
	packet(0x0251,4);
#endif

// 2005-08-08aSakexe
#if PACKETVER >= 20050808
	parseable_packet(0x024d,12,clif_parse_Auction_register,2,6,10);
	packet(0x024e,4);
#endif

// 2005-08-17aSakexe
#if PACKETVER >= 20050817
	packet(0x0253,3);
	parseable_packet(0x0254,3,clif_parse_FeelSaveOk,2);
#endif

// 2005-08-29aSakexe
#if PACKETVER >= 20050829
	packet(0x0240,-1);
	parseable_packet(0x0248,-1,clif_parse_Mail_send,2,4,28,68,69);
	packet(0x0255,5);
	packet(0x0256,-1);
	packet(0x0257,8);
#endif

// 2005-09-12bSakexe
#if PACKETVER >= 20050912
	packet(0x0256,5);
	packet(0x0258,2);
	packet(0x0259,3);
#endif

// 2005-10-10aSakexe
#if PACKETVER >= 20051010
	packet(0x020e,32);
	packet(0x025a,-1);
	parseable_packet( HEADER_CZ_REQ_MAKINGITEM, sizeof( struct PACKET_CZ_REQ_MAKINGITEM ), clif_parse_Cooking, 0 );
#endif

// 2005-10-13aSakexe
#if PACKETVER >= 20051013
	packet(0x007a,6);
	packet(0x0251,32);
	parseable_packet(0x025c,4,clif_parse_Auction_buysell,2);
#endif

// 2005-10-17aSakexe
#if PACKETVER >= 20051017
	packet(0x007a,58);
	parseable_packet(0x025d,6,clif_parse_Auction_close,2);
	packet(0x025e,4);
#endif

// 2005-10-24aSakexe
#if PACKETVER >= 20051024
	packet(0x025f,6);
	packet(0x0260,6);
#endif

// 2005-11-07aSakexe
#if PACKETVER >= 20051107
	parseable_packet(0x024e,6,clif_parse_Auction_cancel,2);
	parseable_packet(0x0251,34,clif_parse_Auction_search,2,4,8,32);
#endif

// 2006-01-09aSakexe
#if PACKETVER >= 20060109
	packet(0x0261,11);
	packet(0x0262,11);
	packet(0x0263,11);
	packet(0x0264,20);
	packet(0x0265,20);
	packet(0x0266,30);
	packet(0x0267,4);
	packet(0x0268,4);
	packet(0x0269,4);
	packet(0x026a,4);
	packet(0x026b,4);
	packet(0x026c,4);
	packet(0x026d,4);
	packet(0x026f,2);
	packet(0x0270,2);
	packet(0x0271,38);
	packet(0x0272,44);
#endif

// 2006-01-26aSakexe
#if PACKETVER >= 20060126
	packet(0x0271,40);
#endif

// 2006-03-06aSakexe
#if PACKETVER >= 20060306
	packet(0x0273,6);
	packet(0x0274,8);
#endif

// 2006-03-13aSakexe
#if PACKETVER >= 20060313
	parseable_packet(0x0273,30,clif_parse_Mail_return,2,6);
#endif

// 2006-03-27aSakexe
#if PACKETVER >= 20060327
	parseable_packet(0x0072,26,clif_parse_UseSkillToId,11,18,22);
	parseable_packet(0x007e,120,clif_parse_UseSkillToPosMoreInfo,5,15,29,38,40);
	parseable_packet(0x0085,12,clif_parse_ChangeDir,7,11);
	//parseable_packet(0x0089,13,clif_parse_TickSend,9);
	parseable_packet(0x008c,12,clif_parse_GetCharNameRequest,8);
	parseable_packet(0x0094,23,clif_parse_MoveToKafra,5,19);
	parseable_packet(0x009b,37,clif_parse_WantToConnection,9,21,28,32,36);
	parseable_packet(0x009f,24,clif_parse_UseItem,9,20);
	parseable_packet(0x00a2,11,clif_parse_SolveCharName,7);
	parseable_packet(0x00a7,15,clif_parse_WalkToXY,12);
	parseable_packet(0x00f5,13,clif_parse_TakeItem,9);
	parseable_packet(0x00f7,26,clif_parse_MoveFromKafra,11,22);
	parseable_packet(0x0113,40,clif_parse_UseSkillToPos,5,15,29,38);
	parseable_packet(0x0116,17,clif_parse_DropItem,8,15);
	parseable_packet(0x0190,18,clif_parse_ActionRequest,7,17);
#endif

// 2006-10-23aSakexe
#if PACKETVER >= 20061023
	packet(0x006d,110);
#endif

//2006-04-24aSakexe to 2007-01-02aSakexe
#if PACKETVER >= 20060424
	packet(0x023e,8);
	packet(0x0277,84);
	packet(0x0278,2);
	packet(0x0279,2);
	packet(0x027a,-1);
	packet(0x027b,14);
	packet(0x027c,60);
	packet(0x027d,62);
	packet(0x027e,-1);
	packet(0x027f,8);
	packet(0x0280,12);
	packet(0x0281,4);
	packet(0x0282,284);
	packet(0x0283,6);
	packet(0x0284,14);
	packet(0x0285,6);
	packet(0x0286,4);
	packet(0x0287,-1);
	packet(0x0288,6);
	packet(0x0289,8);
	packet(0x028a,18);
	packet(0x028b,-1);
	packet(0x028c,46);
	packet(0x028d,34);
	packet(0x028e,4);
	packet(0x028f,6);
	packet(0x0290,4);
	packet(0x0291,4);
	parseable_packet(0x0292,2,clif_parse_AutoRevive,0);
	packet(0x0293,70);
	packet(0x0294,10);
	packet( HEADER_ZC_CASH_TIME_COUNTER, sizeof( struct PACKET_ZC_CASH_TIME_COUNTER ) );
	packet( HEADER_ZC_CASH_ITEM_DELETE, sizeof( struct PACKET_ZC_CASH_ITEM_DELETE ) );
	packet(0x029c,66);
	packet(0x029d,-1);
	packet(0x029e,11);
	parseable_packet(0x029f,3,clif_parse_mercenary_action,2);
	packet(0x02a0,-1);
	packet(0x02a1,-1);
	packet(0x02a2,8);
#endif

// 2007-01-08aSakexe
#if PACKETVER >= 20070108
	parseable_packet(0x0072,30,clif_parse_UseSkillToId,10,14,26);
	parseable_packet(0x007e,120,clif_parse_UseSkillToPosMoreInfo,10,19,23,38,40);
	parseable_packet(0x0085,14,clif_parse_ChangeDir,10,13);
	parseable_packet(0x0089,11,clif_parse_TickSend,7);
	parseable_packet(0x008c,17,clif_parse_GetCharNameRequest,13);
	parseable_packet(0x0094,17,clif_parse_MoveToKafra,4,13);
	parseable_packet(0x009b,35,clif_parse_WantToConnection,7,21,26,30,34);
	parseable_packet(0x009f,21,clif_parse_UseItem,7,17);
	parseable_packet(0x00a2,10,clif_parse_SolveCharName,6);
	parseable_packet(0x00a7,8,clif_parse_WalkToXY,5);
	parseable_packet(0x00f5,11,clif_parse_TakeItem,7);
	parseable_packet(0x00f7,15,clif_parse_MoveFromKafra,3,11);
	parseable_packet(0x0113,40,clif_parse_UseSkillToPos,10,19,23,38);
	parseable_packet(0x0116,19,clif_parse_DropItem,11,17);
	parseable_packet(0x0190,10,clif_parse_ActionRequest,4,9);
#endif

// 2007-01-22aSakexe
#if PACKETVER >= 20070122
	packet(0x02a3,18);
	packet(0x02a4,2);
#endif

// 2007-01-29aSakexe
#if PACKETVER >= 20070129
	packet(0x029b,72);
	packet(0x02a3,-1);
	packet(0x02a4,-1);
	packet(0x02a5,8);
#endif

// 2007-02-05aSakexe
#if PACKETVER >= 20070205
	packet(0x02aa,4);
	packet(0x02ab,36);
	packet(0x02ac,6);
#endif

// 2007-02-12aSakexe
#if PACKETVER >= 20070212
	parseable_packet(0x0072,25,clif_parse_UseSkillToId,6,10,21);
	parseable_packet(0x007e,102,clif_parse_UseSkillToPosMoreInfo,5,9,12,20,22);
	parseable_packet(0x0085,11,clif_parse_ChangeDir,7,10);
	parseable_packet(0x0089,8,clif_parse_TickSend,4);
	parseable_packet(0x008c,11,clif_parse_GetCharNameRequest,7);
	parseable_packet(0x0094,14,clif_parse_MoveToKafra,7,10);
	parseable_packet(0x009b,26,clif_parse_WantToConnection,4,9,17,21,25);
	parseable_packet(0x009f,14,clif_parse_UseItem,4,10);
	parseable_packet(0x00a2,15,clif_parse_SolveCharName,11);
	//parseable_packet(0x00a7,8,clif_parse_WalkToXY,5);
	parseable_packet(0x00f5,8,clif_parse_TakeItem,4);
	parseable_packet(0x00f7,22,clif_parse_MoveFromKafra,14,18);
	parseable_packet(0x0113,22,clif_parse_UseSkillToPos,5,9,12,20);
	parseable_packet(0x0116,10,clif_parse_DropItem,5,8);
	parseable_packet(0x0190,19,clif_parse_ActionRequest,5,18);
#endif

// 2007-02-27aSakexe to 2007-10-02aSakexe
#if PACKETVER >= 20070227
	parseable_packet(0x0288,10,clif_parse_npccashshop_buy,2,4,6);
	packet(0x0289,12);
	packet(0x02a6,22);
	packet(0x02a7,22);
	packet(0x02a8,162);
	packet(0x02a9,58);
	packet(0x02ad,8);
	packet(0x02b0,85);
	packet(0x02b1,-1);
	packet(0x02b2,-1);
	packet(0x02b3,107);
	packet(0x02b4,6);
	packet(0x02b5,-1);
	parseable_packet(0x02b6,7,clif_parse_questStateAck,2,6);
	packet(0x02b7,7);
	packet( HEADER_ZC_ITEM_PICKUP_PARTY, sizeof( struct PACKET_ZC_ITEM_PICKUP_PARTY ) );
	packet(0x02b9,191);
	parseable_packet(0x02ba,11,clif_parse_Hotkey,2,4,5,9);
	packet(0x02bb,8);
	packet(0x02bc,6);
	packet(0x02bf,10);
	packet(0x02c0,2);
	packet(0x02c1,-1);
	packet(0x02c2,-1);
	parseable_packet(0x02c4,26,clif_parse_PartyInvite2,2);
	packet(0x02c5,30);
	packet(0x02c6,30);
	parseable_packet(0x02c7,7,clif_parse_ReplyPartyInvite2,2,6);
	parseable_packet(0x02c8,3,clif_parse_PartyTick,2);
	packet(0x02c9,3);
	packet(0x02ca,3);
	packet(0x02cb,20);
	packet(0x02cc,4);
	packet(0x02cd,26);
	packet(0x02ce,10);
	parseable_packet(0x02cf,6,clif_parse_MemorialDungeonCommand,2);
	ack_packet(ZC_NOTIFY_BIND_ON_EQUIP,0x02d3,4,2);
	packet(0x02d5,2);
	parseable_packet(0x02d6,6,clif_parse_ViewPlayerEquip,2);
	packet( viewequipackType, -1 );
	parseable_packet(0x02d8,10,clif_parse_configuration,2,6);
	packet(0x02d9,10);
	packet(0x02da,3);
	parseable_packet(0x02db,-1,clif_parse_BattleChat,2,4);
	packet(0x02dc,-1);
	packet(0x02dd,32);
	packet(0x02de,6);
	packet(0x02df,36);
	packet(0x02e0,34);
#endif

// 2007-10-23aSakexe
#if PACKETVER >= 20071023
	packet(0x02cb,65);
	packet(0x02cd,71);
#endif

// 2007-11-06aSakexe
#if PACKETVER >= 20071106
	packet(0x0078,55);
	packet(0x007c,42);
	packet(0x022c,65);
	packet(0x029b,80);
#endif

// 2007-11-13aSakexe
#if PACKETVER >= 20071113
	packet(0x02e1,33);
#endif

// 2007-11-20aSakexe
#if PACKETVER >= 20071120
	//packet(0x01df,10 <- ???);
	packet(0x02e2,14);
	packet(0x02e3,25);
	packet(0x02e4,8);
	packet(0x02e5,8);
	packet(0x02e6,6);
#endif

// 2007-11-27aSakexe
#if PACKETVER >= 20071127
	packet(0x02e7,-1);
#endif

// 2008-01-02aSakexe
#if PACKETVER >= 20080102
	parseable_packet(0x01df,6,clif_parse_GMReqAccountName,2);
	packet(0x02eb,13);
	packet(0x02ec,67);
	packet(0x02ed,59);
	packet(0x02ee,60);
	packet(0x02ef,8);
#endif

// 2008-03-18aSakexe
#if PACKETVER >= 20080318
	packet(0x02bf,-1);
	packet(0x02c0,-1);
	packet(0x02f0,10);
	parseable_packet(0x02f1,2,clif_parse_progressbar,0);
	packet(0x02f2,2);
#endif

// 2008-03-25bSakexe
#if PACKETVER >= 20080325
	packet(0x02f3,-1);
	packet(0x02f4,-1);
	packet(0x02f5,-1);
	packet(0x02f6,-1);
	packet(0x02f7,-1);
	packet(0x02f8,-1);
	packet(0x02f9,-1);
	packet(0x02fa,-1);
	packet(0x02fb,-1);
	packet(0x02fc,-1);
	packet(0x02fd,-1);
	packet(0x02fe,-1);
	packet(0x02ff,-1);
	packet(0x0300,-1);
#endif

// 2008-04-01aSakexe
#if PACKETVER >= 20080401
	packet(0x0301,-1);
	packet(0x0302,-1);
	packet(0x0303,-1);
	packet(0x0304,-1);
	packet(0x0305,-1);
	packet(0x0306,-1);
	packet(0x0307,-1);
	packet(0x0308,-1);
	packet(0x0309,-1);
	packet(0x030a,-1);
	packet(0x030b,-1);
	packet(0x030c,-1);
	packet(0x030d,-1);
	packet(0x030e,-1);
	packet(0x030f,-1);
	packet(0x0310,-1);
	packet(0x0311,-1);
	packet(0x0312,-1);
	packet(0x0313,-1);
	packet(0x0314,-1);
	packet(0x0315,-1);
	packet(0x0316,-1);
	packet(0x0317,-1);
	packet(0x0318,-1);
	packet(0x0319,-1);
	packet(0x031a,-1);
	packet(0x031b,-1);
	packet(0x031c,-1);
	packet(0x031d,-1);
	packet(0x031e,-1);
	packet(0x031f,-1);
	packet(0x0320,-1);
	packet(0x0321,-1);
	packet(0x0322,-1);
	packet(0x0323,-1);
	packet(0x0324,-1);
	packet(0x0325,-1);
	packet(0x0326,-1);
	packet(0x0327,-1);
	packet(0x0328,-1);
	packet(0x0329,-1);
	packet(0x032a,-1);
	packet(0x032b,-1);
	packet(0x032c,-1);
	packet(0x032d,-1);
	packet(0x032e,-1);
	packet(0x032f,-1);
	packet(0x0330,-1);
	packet(0x0331,-1);
	packet(0x0332,-1);
	packet(0x0333,-1);
	packet(0x0334,-1);
	packet(0x0335,-1);
	packet(0x0336,-1);
	packet(0x0337,-1);
	packet(0x0338,-1);
	packet(0x0339,-1);
	packet(0x033a,-1);
	packet(0x033b,-1);
	packet(0x033c,-1);
	packet(0x033d,-1);
	packet(0x033e,-1);
	packet(0x033f,-1);
	packet(0x0340,-1);
	packet(0x0341,-1);
	packet(0x0342,-1);
	packet(0x0343,-1);
	packet(0x0344,-1);
	packet(0x0345,-1);
	packet(0x0346,-1);
	packet(0x0347,-1);
	packet(0x0348,-1);
	packet(0x0349,-1);
	packet(0x034a,-1);
	packet(0x034b,-1);
	packet(0x034c,-1);
	packet(0x034d,-1);
	packet(0x034e,-1);
	packet(0x034f,-1);
	packet(0x0350,-1);
	packet(0x0351,-1);
	packet(0x0352,-1);
	packet(0x0353,-1);
	packet(0x0354,-1);
	packet(0x0355,-1);
	packet(0x0356,-1);
	packet(0x0357,-1);
	packet(0x0358,-1);
	packet(0x0359,-1);
	packet(0x035a,-1);
#endif

// 2008-05-27aSakexe
#if PACKETVER >= 20080527
	packet(0x035b,-1);
	packet(0x035c,2);
	packet(0x035d,-1);
	packet(0x035e,2);
	packet(0x035f,-1);
	packet(0x0389,-1);
#endif

// 2008-08-20aSakexe
#if PACKETVER >= 20080820
	packet(0x040c,-1);
	packet(0x040d,-1);
	packet(0x040e,-1);
	packet(0x040f,-1);
	packet(0x0410,-1);
	packet(0x0411,-1);
	packet(0x0412,-1);
	packet(0x0413,-1);
	packet(0x0414,-1);
	packet(0x0415,-1);
	packet(0x0416,-1);
	packet(0x0417,-1);
	packet(0x0418,-1);
	packet(0x0419,-1);
	packet(0x041a,-1);
	packet(0x041b,-1);
	packet(0x041c,-1);
	packet(0x041d,-1);
	packet(0x041e,-1);
	packet(0x041f,-1);
	packet(0x0420,-1);
	packet(0x0421,-1);
	packet(0x0422,-1);
	packet(0x0423,-1);
	packet(0x0424,-1);
	packet(0x0425,-1);
	packet(0x0426,-1);
	packet(0x0427,-1);
	packet(0x0428,-1);
	packet(0x0429,-1);
	packet(0x042a,-1);
	packet(0x042b,-1);
	packet(0x042c,-1);
	packet(0x042d,-1);
	packet(0x042e,-1);
	packet(0x042f,-1);
	packet(0x0430,-1);
	packet(0x0431,-1);
	packet(0x0432,-1);
	packet(0x0433,-1);
	packet(0x0434,-1);
	packet(0x0435,-1);
#endif

// 2008-09-10aSakexe
#if PACKETVER >= 20080910
	parseable_packet(0x0436,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0437,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0438,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0439,8,clif_parse_UseItem,2,4);
#endif

// 2008-11-13aSakexe
#if PACKETVER >= 20081113
	packet(0x043d,8);
	packet(0x043e,-1);
	packet(0x043f,8);
#endif

// 2008-11-26aSakexe
#if PACKETVER >= 20081126
	packet(0x01a2,37);
	packet(0x0440,10);
	packet(0x0441,4);
#endif

// 2008-12-10aSakexe
#if PACKETVER >= 20081210
	packet(0x0442,-1);
	parseable_packet(0x0443,8,clif_parse_SkillSelectMenu,2,6);
#endif

// 2009-01-14aSakexe
#if PACKETVER >= 20090114
	packet(0x043f,25);
	packet(0x0444,-1);
	packet(0x0445,10);
#endif

// 2009-02-18aSakexe
#if PACKETVER >= 20090218
	packet(0x0446,14);
#endif

// 2009-02-25aSakexe
#if PACKETVER >= 20090225
	packet(0x0448,-1);
#endif

// 2009-03-30aSakexe
#if PACKETVER >= 20090330
	packet(0x0449,4);
#endif

// 2009-04-08aSakexe
#if PACKETVER >= 20090408
	packet(0x02a6,-1);
	packet(0x02a7,-1);
	parseable_packet(0x044a,6,clif_parse_client_version,2);
#endif

// Renewal Clients
// 2008-08-27aRagexeRE
#if PACKETVER >= 20080827
	parseable_packet(0x0072,22,clif_parse_UseSkillToId,9,15,18);
	packet(0x007c,44);
	parseable_packet(0x007e,105,clif_parse_UseSkillToPosMoreInfo,10,14,18,23,25);
	parseable_packet(0x0085,10,clif_parse_ChangeDir,4,9);
	parseable_packet(0x0089,11,clif_parse_TickSend,7);
	parseable_packet(0x008c,14,clif_parse_GetCharNameRequest,10);
	parseable_packet(0x0094,19,clif_parse_MoveToKafra,3,15);
	parseable_packet(0x009b,34,clif_parse_WantToConnection,7,15,25,29,33);
	parseable_packet(0x009f,20,clif_parse_UseItem,7,20);
	parseable_packet(0x00a2,14,clif_parse_SolveCharName,10);
	parseable_packet(0x00a7,9,clif_parse_WalkToXY,6);
	parseable_packet(0x00f5,11,clif_parse_TakeItem,7);
	parseable_packet(0x00f7,17,clif_parse_MoveFromKafra,3,13);
	parseable_packet(0x0113,25,clif_parse_UseSkillToPos,10,14,18,23);
	parseable_packet(0x0116,17,clif_parse_DropItem,6,15);
	parseable_packet(0x0190,23,clif_parse_ActionRequest,9,22);
	packet(0x02e2,20);
	packet(0x02e3,22);
	packet(0x02e4,11);
	packet(0x02e5,9);
#endif

// 2008-09-10aRagexeRE
#if PACKETVER >= 20080910
	parseable_packet(0x0436,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0437,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0438,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0439,8,clif_parse_UseItem,2,4);
#endif

// 2008-11-12aRagexeRE
#if PACKETVER >= 20081112
	packet(0x043d,8);
	//packet(0x043e,-1);
	packet(0x043f,8);
#endif

// 2008-12-17aRagexeRE
#if PACKETVER >= 20081217
	packet(0x01a2,37);
	//packet(0x0440,10);
	//packet(0x0441,4);
	//packet(0x0442,8);
	//packet(0x0443,8);
#endif

// 2008-12-17bRagexeRE
#if PACKETVER >= 20081217
	packet(0x006d,114);
#endif

// 2009-01-21aRagexeRE
#if PACKETVER >= 20090121
	packet(0x043f,25);
	//packet(0x0444,-1);
	//packet(0x0445,10);
#endif

// 2009-02-18aRagexeRE
#if PACKETVER >= 20090218
	//packet(0x0446,14);
#endif

// 2009-02-26cRagexeRE
#if PACKETVER >= 20090226
	//packet(0x0448,-1);
#endif

// 2009-04-01aRagexeRE
#if PACKETVER >= 20090401
	//packet(0x0449,4);
#endif

// 2009-05-14aRagexeRE
#if PACKETVER >= 20090514
	//packet(0x044b,2);
#endif

// 2009-05-20aRagexeRE
#if PACKETVER >= 20090520
	//packet(0x07d0,6);
	//packet(0x07d1,2);
	//packet(0x07d2,-1);
	//packet(0x07d3,4);
	//packet(0x07d4,4);
	//packet(0x07d5,4);
	//packet(0x07d6,4);
	parseable_packet( 0x0447, 2, clif_parse_blocking_playcancel, 0 );
#endif

// 2009-06-03aRagexeRE
#if PACKETVER >= 20090603
	parseable_packet(0x07d7,8,clif_parse_PartyChangeOption,2,6,7);
	packet(0x07d8,8);
	packet(0x07d9,254);
	parseable_packet(0x07da,6,clif_parse_PartyChangeLeader,2);
#endif

// 2009-06-10aRagexeRE
#if PACKETVER >= 20090610
	//packet(0x07db,8);
#endif

// 2009-06-17aRagexeRE
#if PACKETVER >= 20090617
	packet(0x07d9,268);
	//packet(0x07dc,6);
	//packet(0x07dd,54);
	//packet(0x07de,30);
	//packet(0x07df,54);
#endif

// 2009-07-01aRagexeRE
#if PACKETVER >= 20090701
	//packet(0x0275,37);
	//packet(0x0276,-1);
#endif

// 2009-07-08aRagexeRE
#if PACKETVER >= 20090708
	//packet(0x07e0,58);
#endif

// 2009-07-15aRagexeRE
#if PACKETVER >= 20090715
	packet(0x07e1,15);
#endif

// 2009-08-05aRagexeRE
#if PACKETVER >= 20090805
	packet(0x07e2,8);
#endif

// 2009-08-18aRagexeRE
#if PACKETVER >= 20090818
	packet(0x07e3,6);
	parseable_packet(0x07e4,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	packet(0x07e6,8);
#endif

// 2009-08-25aRagexeRE
#if PACKETVER >= 20090825
	//packet(0x07e6,28);
	packet(0x07e7,5);
#endif

// 2009-09-22aRagexeRE
#if PACKETVER >= 20090922
	packet(0x07e5,8);
	packet(0x07e6,8);
	packet(0x07e7,32);
	packet(0x07e8,-1);
	packet(0x07e9,5);
#endif

// 2009-09-29aRagexeRE
#if PACKETVER >= 20090929
	//packet(0x07ea,2);
	//packet(0x07eb,-1);
	//packet(0x07ec,6);
	//packet(0x07ed,8);
	//packet(0x07ee,6);
	//packet(0x07ef,8);
	//packet(0x07f0,4);
	//packet(0x07f2,4);
	//packet(0x07f3,3);
#endif

// 2009-10-06aRagexeRE
#if PACKETVER >= 20091006
	//packet(0x07ec,8);
	//packet(0x07ed,10);
	//packet(0x07f0,8);
	//packet(0x07f1,15);
	//packet(0x07f2,6);
	//packet(0x07f3,4);
	//packet(0x07f4,3);
#endif

// 2009-10-27aRagexeRE
#if PACKETVER >= 20091027
	parseable_packet(0x07f5,6,clif_parse_GMFullStrip,2);
	packet(0x07f6,14);
#endif

// 2009-11-03aRagexeRE
#if PACKETVER >= 20091103
	packet(0x07f7,-1);
	packet(0x07f8,-1);
	packet(0x07f9,-1);
#endif

// 2009-11-17aRagexeRE
#if PACKETVER >= 20091117
	packet(0x07fa,8);
#endif

// 2009-11-24aRagexeRE
#if PACKETVER >= 20091124
	packet(0x07fb,25);
#endif

// 2009-12-01aRagexeRE
#if PACKETVER >= 20091201
	packet(0x07fc,10);
	ack_packet(ZC_BROADCASTING_SPECIAL_ITEM_OBTAIN,0x07fd,-1,0);
	packet(0x07fe,26);
	//packet(0x07ff,-1);
#endif

// 2009-12-15aRagexeRE
#if PACKETVER >= 20091215
	//packet(0x0801,-1);
#endif

// 2009-12-22aRagexeRE
#if PACKETVER >= 2009122
	parseable_packet(0x0802,18,clif_parse_PartyBookingRegisterReq,2,4,6); // Booking System
	packet(0x0803,4);
	packet(0x0804,8); // Booking System
	packet(0x0805,-1);
	parseable_packet(0x0806,4,clif_parse_PartyBookingDeleteReq,2); // Booking System
	//packet(0x0807,2);
	packet(0x0808,4); // Booking System
	//packet(0x0809,14);
	//packet(0x080A,50);
	//packet(0x080B,18);
	//packet(0x080C,6);
#endif

// 2009-12-29aRagexeRE
#if PACKETVER >= 20091229
	parseable_packet(0x0804,14,clif_parse_PartyBookingSearchReq,2,4,6,8,12); // Booking System
	parseable_packet(0x0806,2,clif_parse_PartyBookingDeleteReq,0); // Booking System
	packet(0x0807,4);
	parseable_packet(0x0808,14,clif_parse_PartyBookingUpdateReq,2); // Booking System
	packet(0x0809,50);
	packet(0x080A,18);
	packet(0x080B,6); // Booking System
#endif

// 2010-01-05aRagexeRE
#if PACKETVER >= 20100105
	parseable_packet(0x0801,-1,clif_parse_PurchaseReq2,2,4,8,12);
#endif

// 2010-01-26aRagexeRE
#if PACKETVER >= 20100126
	//packet(0x080C,2);
	//packet(0x080D,3);
	packet(0x080E,14);
#endif

// 2010-02-09aRagexeRE
#if PACKETVER >= 20100209
	//packet(0x07F0,6);
#endif

// 2010-03-03aRagexeRE
#if PACKETVER >= 20100303
	packet(0x0810,3);
	parseable_packet(0x0811,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//packet(0x0812,86);
	//packet(0x0813,6);
	//packet(0x0814,6);
	//packet(0x0815,-1);
	//packet(0x0817,-1);
	//packet(0x0818,6);
	//packet(0x0819,4);
#endif

// 2010-03-09aRagexeRE
#if PACKETVER >= 20100309
	packet(0x0813,-1);
	//packet(0x0814,2);
	//packet(0x0815,6);
	packet(0x0816,6);
	packet(0x0818,-1);
	//packet(0x0819,10);
	//packet(0x081A,4);
	//packet(0x081B,4);
	//packet(0x081C,6);
	packet(0x081d,22);
	packet(0x081e,8);
#endif

// 2010-03-23aRagexeRE
#if PACKETVER >= 20100323
	//packet(0x081F,-1);
#endif

// 2010-04-06aRagexeRE
#if PACKETVER >= 20100406
	//packet(0x081A,6);
#endif

// 2010-04-13aRagexeRE
#if PACKETVER >= 20100413
	//packet(0x081A,10);
	packet(0x0820,11);
	//packet(0x0821,2);
	//packet(0x0822,9);
	//packet(0x0823,-1);
#endif

// 2010-04-14dRagexeRE
#if PACKETVER >= 20100414
	//packet(0x081B,8);
#endif

// 2010-04-20aRagexeRE
#if PACKETVER >= 20100420
	packet(0x0812,8);
	packet(0x0814,86);
	parseable_packet(0x0815,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0817,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0819,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	packet(0x081a,4);
	packet(0x081b,10);
	packet(0x081c,10);
	packet( HEADER_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER, sizeof( struct PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER ) );
#endif

// 2010-06-01aRagexeRE
#if PACKETVER >= 20100601
	//packet(0x0825,-1);
	//packet(0x0826,4);
	parseable_packet(0x0835,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	packet( HEADER_ZC_SEARCH_STORE_INFO_ACK, -1 );
	packet(0x0837,3);
	//packet(0x0838,3);
#endif

// 2010-06-08aRagexeRE
#if PACKETVER >= 20100608
	parseable_packet(0x0838,2,clif_parse_SearchStoreInfoNextPage,0);
	packet(0x083A,4); // Search Stalls Feature
	parseable_packet(0x083B,2,clif_parse_CloseSearchStoreInfo,0);
	parseable_packet( HEADER_CZ_SSILIST_ITEM_CLICK, sizeof( struct PACKET_CZ_SSILIST_ITEM_CLICK ), clif_parse_SearchStoreInfoListItemClick, 0 );
	packet(0x083D,6);
#endif

// 2010-06-15aRagexeRE
#if PACKETVER >= 20100615
	//packet(0x083E,26);
#endif

// 2010-06-22aRagexeRE
#if PACKETVER >= 20100622
	//packet(0x083F,22);
#endif

// 2010-06-29aRagexeRE
#if PACKETVER >= 20100629
	ack_packet(ZC_WEAR_EQUIP_ACK,0x00AA,9,2,4,6,8);
	//packet(0x07F1,18);
	//packet(0x07F2,8);
	//packet(0x07F3,6);
#endif

// 2010-07-01aRagexeRE
#if PACKETVER >= 20100701
	packet(0x083A,5); // Search Stalls Feature
#endif

// 2010-07-13aRagexeRE
#if PACKETVER >= 20100713
	//packet(0x0827,6);
	//packet(0x0828,14);
	//packet(0x0829,6);
	//packet(0x082A,10);
	//packet(0x082B,6);
	//packet(0x082C,14);
	//packet(0x0840,-1);
	//packet(0x0841,19);
#endif

// 2010-07-14aRagexeRE
#if PACKETVER >= 20100714
	//packet(0x0841,4);
#endif

// 2010-08-03aRagexeRE
#if PACKETVER >= 20100803
	packet(0x0839,66);
	parseable_packet(0x0842,6,clif_parse_GMRecall2,2);
	parseable_packet(0x0843,6,clif_parse_GMRemove2,2);
#endif

// 2010-11-24aRagexeRE
#if PACKETVER >= 20101124
	parseable_packet(0x0288,-1,clif_parse_npccashshop_buy,2,4,8,10);
	parseable_packet(0x0436,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x035f,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0360,6,clif_parse_TickSend,2);
	parseable_packet(0x0361,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0362,6,clif_parse_TakeItem,2);
	parseable_packet(0x0363,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0364,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0365,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0366,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0367,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0369,6,clif_parse_SolveCharName,2);
	packet(0x0856,-1);
	packet(0x0857,-1);
	packet(0x0858,-1);
	ack_packet(ZC_WEAR_EQUIP_ACK,0x08d0,9,2,4,6,8);
#endif

// 2011-10-05aRagexeRE
#if PACKETVER >= 20111005
	parseable_packet(0x0364,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0817,6,clif_parse_TickSend,2);
	parseable_packet(0x0366,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0815,6,clif_parse_TakeItem,2);
	parseable_packet(0x0885,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0893,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0897,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0369,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x08ad,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0838,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0439,8,clif_parse_UseItem,2,4);
	packet(0x08d2,10);
	packet(0x08d1,7);
	parseable_packet(0x0846,4,clif_parse_CashShopReqTab,2); //2011-07-18
#endif

// 2011-11-02aRagexe
#if PACKETVER >= 20111102
	parseable_packet(0x0436,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0898,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0281,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x088d,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x083c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x08aa,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x02c4,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0811,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	packet(0x0890,8);
	parseable_packet(0x08a5,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0835,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x089b,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08a1,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x089e,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x08ab,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x088b,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x08a2,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,2,6,10);
	packet(0x08cf,10); //Amulet spirits
#endif

// 2012-03-07fRagexeRE
#if PACKETVER >= 20120307
	parseable_packet(0x086A,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0887,6,clif_parse_TickSend,2);
	parseable_packet(0x0890,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0865,6,clif_parse_TakeItem,2);
	parseable_packet(0x02C4,6,clif_parse_DropItem,2,4);
	parseable_packet(0x093B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0963,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0863,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0861,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0929,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0885,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0889,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0870,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x0926,41,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0884,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,2,6,10);
	parseable_packet(0x0439,8,clif_parse_UseItem,2,4);
	parseable_packet(0x0365,41,clif_parse_PartyBookingRegisterReq,2,4,6);
	// New Packet
	packet(0x090F,-1); // ZC_NOTIFY_NEWENTRY7
	packet(0x0914,-1); // ZC_NOTIFY_MOVEENTRY8
	packet(0x0915,-1); // ZC_NOTIFY_STANDENTRY9
#endif

// 2012-04-10aRagexeRE
#if PACKETVER >= 20120410
	parseable_packet(0x089c,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0885,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0961,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0288,-1,clif_parse_npccashshop_buy,2,4,8,10);
	parseable_packet(0x091c,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x094b,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0439,8,clif_parse_UseItem,2,4);
	parseable_packet(0x0945,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,2,6,10);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0886,6,clif_parse_TickSend,2);
	parseable_packet(0x0871,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0938,6,clif_parse_TakeItem,2);
	parseable_packet(0x0891,6,clif_parse_DropItem,2,4);
	parseable_packet(0x086c,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a6,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0889,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0884,6,clif_parse_SolveCharName,2);
	packet(0x08e6,4);
	parseable_packet(0x08e7,10,clif_parse_PartyBookingSearchReq,2,4,6,8,12);
	packet(0x08e8,-1);
	parseable_packet(0x08e9,2,clif_parse_PartyBookingDeleteReq,0);
	packet(0x08ea,4);
	parseable_packet(0x08eb,39,clif_parse_PartyBookingUpdateReq,2);
	packet(0x08ec,73);
	packet(0x08ed,43);
	packet(0x08ee,6);
	parseable_packet(0x08ef,6,NULL,2);
	packet(0x08f0,6);
	parseable_packet(0x08f1,6,NULL,2);
	packet(0x08f2,36);
	packet(0x08f3,-1);
	packet(0x08f4,6);
	parseable_packet(0x08f5,-1,NULL,2,4);
	packet(0x08f6,22);
	packet(0x08f7,3);
	packet(0x08f8,7);
	packet(0x08f9,6);
	packet(0x08fa,6);
	parseable_packet(0x08fb,6,NULL,2);
	parseable_packet(0x0907,5,clif_parse_MoveItem,2,4);
	packet(0x0908,5);
	parseable_packet(0x08D7,28,clif_parse_bg_queue_apply_request,2,4);
	packet(0x08D8,27);
	packet(0x08D9,30);
	parseable_packet(0x08DA,26,clif_parse_bg_queue_cancel_request,2);
	packet(0x08DB,27);
	packet(0x08DC,26);
	parseable_packet(0x08DD,27,clif_parse_dull,2,3);
	packet(0x08DE,27);
	packet(0x08DF,50);
	parseable_packet(0x08E0,51,clif_parse_bg_queue_lobby_reply,2,3,27);
	packet(0x08E1,51);
	parseable_packet(0x090A,26,clif_parse_bg_queue_request_queue_number,2);
	packet( HEADER_ZC_ENTRY_QUEUE_INIT , sizeof(PACKET_ZC_ENTRY_QUEUE_INIT) );
	packet(0x0977,14); //Monster HP Bar
	parseable_packet(0x0916,26,clif_parse_GuildInvite2,2);
	parseable_packet(0x091d,41,clif_parse_PartyBookingRegisterReq,2,4,6);
	ack_packet(ZC_PERSONAL_INFOMATION,0x08cb,10,2,4,6,8,10,11,13,15); //Still need further information
	// Merge Item
	ack_packet(ZC_MERGE_ITEM_OPEN,0x096D,-1,2,4); // ZC_MERGE_ITEM_OPEN
	parseable_packet(0x096E,-1,clif_parse_merge_item_req,2,4); // CZ_REQ_MERGE_ITEM
	ack_packet(ZC_ACK_MERGE_ITEM,0x096F,7,2,4,6); // ZC_ACK_MERGE_ITEM
	parseable_packet(0x0974,2,clif_parse_merge_item_cancel,0); // CZ_CANCEL_MERGE_ITEM
	parseable_packet(0x0844,2,clif_parse_cashshop_open_request,0);
	packet(0x0849,16); //clif_cashshop_result
	parseable_packet(0x0848,-1,clif_parse_cashshop_buy,2,6,4,10);
	parseable_packet(0x084a,2,clif_parse_cashshop_close,0);
	parseable_packet(0x08c9,2,clif_parse_cashshop_list_request,0);
#endif

// 2012-04-18aRagexeRE [Special Thanks to Judas!]
#if PACKETVER >= 20120418
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08A8,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08E5,41,clif_parse_PartyBookingRegisterReq,2,4,6); //Added to prevent disconnections
	packet(0x08d2,10);
#endif

// 2012-06-18
#if PACKETVER >= 20120618
	packet(0x0983,29); // ZC_MSG_STATE_CHANGE3
	parseable_packet(0x0861,41,clif_parse_PartyBookingRegisterReq,2,4,6); //actually 12-05-03
#endif

// 2012-07-02aRagexeRE (unstable)
#if PACKETVER >= 20120702
	parseable_packet(0x0363,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0364,6,clif_parse_TickSend,2);
	parseable_packet(0x085a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0861,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0862,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0863,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0886,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0889,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x089e,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089f,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a0,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x094a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0953,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0960,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0879,41,clif_parse_PartyBookingRegisterReq,2,4,6);
#endif

#if PACKETVER >= 20121212
	packet(0x08C7,20);
#endif

// 2013-03-20Ragexe (Judas)
#if PACKETVER >= 20130320
	parseable_packet(0x014f,6,clif_parse_GuildRequestInfo,2);
	//parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,6,clif_parse_TickSend,2);
	parseable_packet(0x0365,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,2,6,10);
	parseable_packet(0x0438,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0447,2,clif_parse_blocking_playcancel,0); // CZ_BLOCKING_PLAY_CANCEL
	parseable_packet(0x044A,6,clif_parse_client_version,2);
	parseable_packet(0x0844,2,clif_parse_cashshop_open_request,0);
	packet(0x0849,16); //clif_cashshop_result
	parseable_packet(0x0848,-1,clif_parse_cashshop_buy,2,6,4,10);
	parseable_packet(0x084a,2,clif_parse_cashshop_close,0);
	packet(0x084b,19); //fallitem4
	parseable_packet(0x085a,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x085d,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0868,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x086d,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x086f,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0874,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0881,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0886,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0888,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x088e,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0897,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0898,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x089b,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08ac,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08c9,2,clif_parse_cashshop_list_request,0);
	packet(0x08cf,10); //Amulet spirits
	packet(0x08d2,10);
	parseable_packet(0x0907,5,clif_parse_MoveItem,2,4);
	packet(0x0908,5);
	parseable_packet(0x0922,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x092e,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0933,6,clif_parse_TakeItem,2);
	parseable_packet(0x0938,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x093f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0947,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x094c,6,clif_parse_SolveCharName,2);
	parseable_packet(0x094e,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0959,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x095a,8,clif_parse_Mail_setattach,2,4);
	packet(0x0977,14); //Monster HP Bar
	parseable_packet(0x0978,6,clif_parse_reqworldinfo,2);
	packet(0x0979,50); //ackworldinfo
	ack_packet(ZC_PERSONAL_INFOMATION,0x097b,16,2,4,8,12,16,17,21,25); //Still need further information
	//ack_packet(ZC_PERSONAL_INFOMATION_CHN,0x0981,12,2,4,6,8,12,13,15,17,10); // Disabled until further information is found.
	parseable_packet(0x0998,8,clif_parse_EquipItem,2,4); // CZ_REQ_WEAR_EQUIP_V5
	ack_packet(ZC_WEAR_EQUIP_ACK,0x0999,11,2,4,8,10); // cz_wear_equipv5
	packet(0x099a,9); // take_off_equipv5
	packet(0x099b,8); //maptypeproperty2
	// New Packets
	packet(0x08C8,34); // ZC_NOTIFY_ACT3
	packet(0x08ff,24); // ZC_EFST_SET_ENTER
	packet(0x0984,28); // ZC_EFST_SET_ENTER2
	packet(0x099f,22); // ZC_SKILL_ENTRY4
#endif

// 2013-06-05Ragexe
#if PACKETVER >= 20130605
	parseable_packet(0x097C,4,clif_parse_ranklist,2);
#endif

// 2013-07-10Ragexe
#if PACKETVER >= 20130710
	parseable_packet(0x0848,-1,clif_parse_cashshop_buy,2,6,4,10);
	packet(0x097D,288); //ZC_ACK_RANKING
	packet(0x097E,12); //ZC_UPDATE_RANKING_POINT
#endif

// 2013-07-17Ragexe
#if PACKETVER >= 20130717
	ack_packet(ZC_BANKING_CHECK,0x09A6,12,2,10);
	parseable_packet(0x09A7,10,clif_parse_BankDeposit,2,6);
	ack_packet(ZC_ACK_BANKING_DEPOSIT,0x09A8,16,2,4,12);
	parseable_packet(0x09A9,10,clif_parse_BankWithdraw,2,6);
	ack_packet(ZC_ACK_BANKING_WITHDRAW,0x09AA,16,2,4,12);
	parseable_packet(0x09AB,6,clif_parse_BankCheck,2);
	parseable_packet(0x09B6,6,clif_parse_BankOpen,2);
	ack_packet(ZC_ACK_OPEN_BANKING,0x09B7,4,2);
	parseable_packet(0x09B8,6,clif_parse_BankClose,2);
	ack_packet(ZC_ACK_CLOSE_BANKING,0x09B9,4,2);
#endif

// 2013-07-31cRagexe
#if PACKETVER >= 20130731
	packet(0x09ca,23); // ZC_SKILL_ENTRY5
	packet(0x09cb,17); // ZC_USE_SKILL2
#endif

// 2013-08-07Ragexe
#if PACKETVER >= 20130807
	ack_packet(ZC_C_MARKERINFO,0x09C1,10,2,6,8);
	// Merge Item
	ack_packet(ZC_MERGE_ITEM_OPEN,0x096D,-1,2,4); // ZC_MERGE_ITEM_OPEN
	parseable_packet(0x096E,-1,clif_parse_merge_item_req,2,4); // CZ_REQ_MERGE_ITEM
	ack_packet(ZC_ACK_MERGE_ITEM,0x096F,7,2,4,6,7); // ZC_ACK_MERGE_ITEM
	parseable_packet(0x0974,2,clif_parse_merge_item_cancel,0); // CZ_CANCEL_MERGE_ITEM
	packet(0x9CD,8); // ZC_MSG_COLOR
#endif

// 2013-08-21bRagexe
#if PACKETVER >= 20130821
	packet(0x09D1,14);
#endif

// 2013-12-23Ragexe
#if PACKETVER >= 20131223
	//New Packets
	parseable_packet(0x09CE,102,clif_parse_GM_Item_Monster,2);
	parseable_packet(0x09D4,2,clif_parse_NPCShopClosed,0);
	//NPC Market
	packet(0x09D5,-1);
	parseable_packet( HEADER_CZ_NPC_MARKET_PURCHASE, -1, clif_parse_NPCMarketPurchase, 0 );
	packet(0x09D7,-1);
	parseable_packet(0x09D8,2,clif_parse_NPCMarketClosed,0);
	// Clan System
	packet(0x0988,6);
	packet(0x0989,2);
	packet(0x098A,-1);
	parseable_packet(0x098D,-1,clif_parse_clan_chat,2,4);
	packet(0x098E,-1);
	// Sale
	parseable_packet( HEADER_CZ_REQ_CASH_BARGAIN_SALE_ITEM_INFO, -1, clif_parse_sale_search, 0 );
	packet( HEADER_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO, sizeof( PACKET_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO ) );
	parseable_packet( HEADER_CZ_REQ_APPLY_BARGAIN_SALE_ITEM, sizeof( PACKET_CZ_REQ_APPLY_BARGAIN_SALE_ITEM ), clif_parse_sale_add, 0 );
	packet(0x09AF,4);
	parseable_packet( HEADER_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM, sizeof( PACKET_CZ_REQ_REMOVE_BARGAIN_SALE_ITEM ), clif_parse_sale_remove, 0 );
	packet(0x09B1,4);
	packet( HEADER_ZC_NOTIFY_BARGAIN_SALE_SELLING, sizeof( PACKET_ZC_NOTIFY_BARGAIN_SALE_SELLING ) );
	packet( HEADER_ZC_NOTIFY_BARGAIN_SALE_CLOSE, sizeof( PACKET_ZC_NOTIFY_BARGAIN_SALE_CLOSE ) );
	parseable_packet(0x09B4,6,clif_parse_sale_open,2);
	parseable_packet(0x09BC,6,clif_parse_sale_close,2);
	parseable_packet(0x09C3,8,clif_parse_sale_refresh,2,6);
	packet( HEADER_ZC_ACK_COUNT_BARGAIN_SALE_ITEM, sizeof( PACKET_ZC_ACK_COUNT_BARGAIN_SALE_ITEM ) );
	// New Packet
	packet(0x097A,-1); // ZC_ALL_QUEST_LIST2
	packet(0x09DB,-1); // ZC_NOTIFY_MOVEENTRY10
	packet(0x09DC,-1); // ZC_NOTIFY_NEWENTRY10
	packet(0x09DD,-1); // ZC_NOTIFY_STANDENTRY10
	packet(0x09DF,7); // ZC_ACK_WHISPER02
#endif

// 2014-02-05bRagexeRE
#if PACKETVER >= 20140205
	packet(0x09DA,-1);
#endif

// 2014-10-08Ragexe
#if PACKETVER >= 20141008
	parseable_packet(0x9FB, -1, clif_parse_pet_evolution, 2, 4); // CZ_PET_EVOLUTION
	packet(0x09FC, 6); // ZC_PET_EVOLUTION_RESULT
#endif

// 2014-10-16Ragexe
#if PACKETVER >= 20141016
	packet(0x09DF,7);
	// New packet
	packet(0x0A00,269); // ZC_SHORTCUT_KEY_LIST_V3
	parseable_packet(0x0A01,3,clif_parse_HotkeyRowShift,2); // CZ_SHORTCUTKEYBAR_ROTATE
	packet(0x0A02,4); // ZC_DRESSROOM_OPEN
	packet(0x0A0E,14); // ZC_BATTLEFIELD_NOTIFY_HP2
	packet(0x09F7,75); // ZC_PROPERTY_HOMUN_2
	packet(0x09E5,18); // ZC_DELETEITEM_FROM_MCSTORE2
	packet(0x09E6,22); // ZC_UPDATE_ITEM_FROM_BUYING_STORE2
	// Roulette System [Yommy]
	parseable_packet(0x0A19,2,clif_parse_roulette_open,0); // CZ_REQ_OPEN_ROULETTE
	packet(0x0A1A,23); // ZC_ACK_OPEN_ROULETTE
	parseable_packet(0x0A1B,2,clif_parse_roulette_info,0); // CZ_REQ_ROULETTE_INFO
	packet(0x0A1C,-1); // ZC_ACK_ROULETTE_INFO
	parseable_packet(0x0A1D,2,clif_parse_roulette_close,0); // CZ_REQ_CLOSE_ROULETTE
	packet(0x0A1E,3); // ZC_ACK_CLOSE_ROULETTE
	parseable_packet(0x0A1F,2,clif_parse_roulette_generate,0); // CZ_REQ_GENERATE_ROULETTE
	packet( roulettgenerateackType, sizeof( struct packet_roulette_generate_ack ) ); // ZC_ACK_GENERATE_ROULETTE
	parseable_packet(0x0A21,3,clif_parse_roulette_item,2); // CZ_RECV_ROULETTE_ITEM
	packet(0x0A22,5); // ZC_RECV_ROULETTE_ITEM
#endif

// 2014-10-22bRagexe
#if PACKETVER >= 20141022
	packet(0x006d,149);
	packet(0x08e3,149);
	// New Packet
	packet(0x0A18,14); // ZC_ACCEPT_ENTER3
	packet(0x0A27,8);
	packet(0x0A28,3); // ZC_ACK_OPENSTORE2
	packet(0x09FD,-1); // ZC_NOTIFY_MOVEENTRY11
	packet(0x09FE,-1); // ZC_NOTIFY_NEWENTRY11
	packet(0x09FF,-1); // ZC_NOTIFY_STANDENTRY11
#endif

#if PACKETVER_MAIN_NUM >= 20150507 || PACKETVER_RE_NUM >= 20150429 || defined(PACKETVER_ZERO)
	packet( HEADER_ZC_HAT_EFFECT, -1 );
#endif

// 2015-05-13aRagexe
#if PACKETVER >= 20150513
	// RODEX Mail system
	packet(0x09E7,3); // ZC_NOTIFY_UNREADMAIL
	parseable_packet(0x09E8,11,clif_parse_Mail_refreshinbox,2,3); // CZ_OPEN_MAILBOX
	parseable_packet(0x09E9,2,clif_parse_dull,0); // CZ_CLOSE_MAILBOX
	parseable_packet(0x09EA,11,clif_parse_Mail_read,2,3); // CZ_REQ_READ_MAIL
	packet(rodexread,-1); // ZC_ACK_READ_MAIL
	parseable_packet(0x09EC,-1,clif_parse_Mail_send,2,4,28,52,60,62,64); // CZ_REQ_WRITE_MAIL
	packet(0x09ED,3); // ZC_ACK_WRITE_MAIL
	parseable_packet(0x09EE,11,clif_parse_Mail_refreshinbox,2,3); // CZ_REQ_NEXT_MAIL_LIST
	parseable_packet(0x09EF,11,clif_parse_Mail_refreshinbox,2,3); // CZ_REQ_REFRESH_MAIL_LIST
	packet(0x09F0,-1); // ZC_ACK_MAIL_LIST
	parseable_packet(0x09F1,11,clif_parse_Mail_getattach,0); // CZ_REQ_ZENY_FROM_MAIL
	packet(0x09F2,12); // ZC_ACK_ZENY_FROM_MAIL
	parseable_packet(0x09F3,11,clif_parse_Mail_getattach,0); // CZ_REQ_ITEM_FROM_MAIL
	packet(0x09F4,12); // ZC_ACK_ITEM_FROM_MAIL
	parseable_packet(0x09F5,11,clif_parse_Mail_delete,0); // CZ_REQ_DELETE_MAIL
	packet(0x09F6,11); // ZC_ACK_DELETE_MAIL
	parseable_packet(0x0A03,2,clif_parse_Mail_cancelwrite,0); // CZ_REQ_CANCEL_WRITE_MAIL
	parseable_packet(0x0A04,6,clif_parse_Mail_setattach,2,4); // CZ_REQ_ADD_ITEM_TO_MAIL
	packet( rodexadditem, sizeof( struct PACKET_ZC_ADD_ITEM_TO_MAIL ) ); // ZC_ACK_ADD_ITEM_TO_MAIL
	parseable_packet(0x0A06,6,clif_parse_Mail_winopen,2,4); // CZ_REQ_REMOVE_ITEM_MAIL
	packet(0x0A07,9); // ZC_ACK_REMOVE_ITEM_MAIL
	parseable_packet(0x0A08,26,clif_parse_Mail_beginwrite,0); // CZ_REQ_OPEN_WRITE_MAIL
	packet(0x0A12,27); // ZC_ACK_OPEN_WRITE_MAIL
	parseable_packet(0x0A13,26,clif_parse_Mail_Receiver_Check,2); // CZ_CHECK_RECEIVE_CHARACTER_NAME
	packet(0x0A14,10); // ZC_CHECK_RECEIVE_CHARACTER_NAME
	packet(0x0A32,2); // ZC_OPEN_RODEX_THROUGH_NPC_ONLY
	// OneClick Itemidentify
	parseable_packet(0x0A35,4,clif_parse_Oneclick_Itemidentify,2); // CZ_REQ_ONECLICK_ITEMIDENTIFY
	// Achievement System
	packet(0x0A23,-1); // ZC_ALL_ACH_LIST
	packet(0x0A24,66); // ZC_ACH_UPDATE
	parseable_packet(0x0A25,6,clif_parse_AchievementCheckReward,0); // CZ_REQ_ACH_REWARD
	packet(0x0A26,7); // ZC_REQ_ACH_REWARD_ACK
	// Title System
	parseable_packet(0x0A2E,6,clif_parse_change_title,0); // CZ_REQ_CHANGE_TITLE
	packet(0x0A2F,7); // ZC_ACK_CHANGE_TITLE

	// Quest UI
	packet(0x08FE,-1); // ZC_HUNTING_QUEST_INFO
	packet(0x09F8,-1); // ZC_ALL_QUEST_LIST3
	packet(0x09F9,143); // ZC_ADD_QUEST_EX
	packet(0x09FA,-1); // ZC_UPDATE_MISSION_HUNT_EX
#endif

// 2015-05-20aRagexe
#if PACKETVER >= 20150520
	parseable_packet( HEADER_CZ_REQ_APPLY_BARGAIN_SALE_ITEM2, sizeof( PACKET_CZ_REQ_APPLY_BARGAIN_SALE_ITEM ), clif_parse_sale_add, 0 );
#endif

// 2015-09-16Ragexe
#if PACKETVER >= 20150916
	// New Packet
	packet(0x097F,-1); // ZC_SELECTCART
	parseable_packet(0x0980,7,clif_parse_SelectCart,2,6); // CZ_SELECTCART
#endif

// 2016-03-02bRagexe
#if PACKETVER >= 20160302
	packet(0x0A51,34);
#endif

// 2016-03-30aRagexe
#if PACKETVER >= 20160330
	parseable_packet(0x0A6E,-1,clif_parse_Mail_send,2,4,28,52,60,62,64,68); // CZ_REQ_WRITE_MAIL2
#endif

// 2016-05-25aRagexeRE
#if PACKETVER >= 20160525
	parseable_packet(0x0A77,15,clif_parse_camerainfo,0);
	packet(0x0A78, 15);
#endif

// 2016-06-01aRagexe
#if PACKETVER >= 20160601
	packet(0x0A7D,-1);
#endif

// 2016-06-22aRagexeRE
#if PACKETVER >= 20160622
	packet(0x0A84,94);
#endif

// 2016-10-12aRagexeRE
#if PACKETVER >= 20161012
	packet( HEADER_ZC_REFINE_OPEN_WINDOW, sizeof( struct PACKET_ZC_REFINE_OPEN_WINDOW ) );
	parseable_packet( HEADER_CZ_REFINE_ADD_ITEM, sizeof( struct PACKET_CZ_REFINE_ADD_ITEM ), clif_parse_refineui_add, 0 );
	packet( HEADER_ZC_REFINE_ADD_ITEM, -1 );
	parseable_packet( HEADER_CZ_REFINE_ITEM_REQUEST, sizeof( struct PACKET_CZ_REFINE_ITEM_REQUEST ), clif_parse_refineui_refine, 0 );
	parseable_packet( HEADER_CZ_REFINE_WINDOW_CLOSE, sizeof( struct PACKET_CZ_REFINE_WINDOW_CLOSE ), clif_parse_refineui_close, 0 );
#endif

// 2016-10-26bRagexeRE
#if PACKETVER >= 20161026
	packet(0x0AA5,-1);
#endif

// 2017-02-08bRagexeRE
#if PACKETVER >= 20170208
	parseable_packet(0x0A97,8,clif_parse_equipswitch_add,2,4);
	packet(0x0A98,12);
	parseable_packet(0x0A99,8,clif_parse_equipswitch_remove,2,4,6);
	packet(0x0A9A,10);
	packet(0x0A9B,-1);
	parseable_packet(0x0A9C,2,clif_parse_equipswitch_request,0);
	packet(0x0A9D,4);
#endif

// 2017-03-15cRagexeRE
#if PACKETVER >= 20170315
	packet(0xac7,156);
#endif

// 2017-04-19bRagexeRE
#if PACKETVER >= 20170419
	parseable_packet(0x0AC0,26,clif_parse_Mail_refreshinbox,2,10);
	parseable_packet(0x0AC1,26,clif_parse_Mail_refreshinbox,2,10);
#endif

// 2017-05-02dRagexeRE
#if PACKETVER >= 20170502
	packet(0x0A43,85);
	packet(0x0A44,-1);
	packet(0x0AB2,7);
	packet(0x0ABD,10);
	packet(0x0A98,10);
	parseable_packet(0x0A99,4,clif_parse_equipswitch_remove,2,4);
	parseable_packet(0x0ACE,4,clif_parse_equipswitch_request_single,0);
#endif

// 2017-08-30bRagexeRE
#if PACKETVER >= 20170830
	packet(0x0ACB,12);
	packet(0x0ACC,18);
#endif

// 2017-10-25eRagexeRE
#if PACKETVER >= 20171025
	packet(0x0ADE,6);
#endif

// 2018-01-03aRagexeRE or 2018-01-03bRagexeRE
#if PACKETVER >= 20180103
	parseable_packet(0x0ae8,2,clif_parse_changedress,0);
#endif

// 2018-02-07bRagexeRE
#if PACKETVER >= 20180207
	parseable_packet(0x0AF4,11,clif_parse_UseSkillToPos,2,4,6,8,10);
#endif

// 2018-02-21aRagexeRE or 2018-02-21bRagexeRE
#if PACKETVER >= 20180221
	packet(0x0206,35); // ZC_FRIENDS_STATE
	packet(0x0af7,32);
#endif

// 2018-03-07bRagexeRE
#if PACKETVER >= 20180307
	parseable_packet(0x0A68,3,clif_parse_open_ui,2);
	packet(0x0AE2,7);
	parseable_packet(0x0AEF,2,clif_parse_attendance_request,0);
	packet(0x0AF0,10);
#endif

// 2018-03-21aRagexeRE
#if PACKETVER >= 20180321
	parseable_packet( 0x0A49, sizeof( struct PACKET_CZ_PRIVATE_AIRSHIP_REQUEST ), clif_parse_private_airship_request, 0 );
	packet(0x0A4A,6);
	packet(0x0A4B,22);
	packet(0x0A4C,28);
#endif

// 2018-04-18bRagexeRE
#if PACKETVER >= 20180418
	packet(0x0ADD, 22);
#endif

#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002 || PACKETVER_ZERO_NUM >= 20181010
	parseable_packet( 0x0B10, sizeof( struct PACKET_CZ_START_USE_SKILL ), clif_parse_StartUseSkillToId, 0 );
	parseable_packet( 0x0B11, sizeof( struct PACKET_CZ_STOP_USE_SKILL ), clif_parse_StopUseSkillToId, 0 );
#endif

#if PACKETVER_MAIN_NUM >= 20181031 || PACKETVER_RE_NUM >= 20181031 || PACKETVER_ZERO_NUM >= 20181114
	parseable_packet( 0x0B14, sizeof( struct PACKET_CZ_INVENTORY_EXPAND ), clif_parse_dull, 0 );
	parseable_packet( 0x0B16, sizeof( struct PACKET_CZ_INVENTORY_EXPAND_CONFIRMED ), clif_parse_dull, 0 );
	parseable_packet( 0x0B19, sizeof( struct PACKET_CZ_INVENTORY_EXPAND_REJECTED ), clif_parse_dull, 0 );
#endif

#if PACKETVER_MAIN_NUM >= 20190227 || PACKETVER_RE_NUM >= 20190220 || PACKETVER_ZERO_NUM >= 20190220
	parseable_packet( 0x0B1C, sizeof( struct PACKET_CZ_PING ), clif_parse_dull, 0 );
#endif

#if PACKETVER >= 20190724
	parseable_packet(HEADER_CZ_GUILD_EMBLEM_CHANGE2, sizeof( PACKET_CZ_GUILD_EMBLEM_CHANGE2 ), clif_parse_GuildChangeEmblem2, 0 );
	packet(HEADER_ZC_CHANGE_GUILD, sizeof(PACKET_ZC_CHANGE_GUILD));
#endif

#if PACKETVER_MAIN_NUM >= 20190522 || PACKETVER_RE_NUM >= 20190508 || PACKETVER_ZERO_NUM >= 20190605
	parseable_packet( 0x0B21, sizeof( struct PACKET_CZ_SHORTCUT_KEY_CHANGE2 ), clif_parse_Hotkey, 0 );
	parseable_packet( 0x0B22, sizeof( struct PACKET_CZ_SHORTCUTKEYBAR_ROTATE2 ), clif_parse_HotkeyRowShift, 0 );
#endif

#if PACKETVER_MAIN_NUM >= 20190703 || PACKETVER_RE_NUM >= 20190703 || PACKETVER_ZERO_NUM >= 20190709
	parseable_packet( HEADER_CZ_REQ_MOUNTOFF, sizeof( PACKET_CZ_REQ_MOUNTOFF ), clif_parse_RemoveOption, 0 );
#endif

#if PACKETVER >= 20190724
	parseable_packet( 0x0b4c, 2, clif_parse_dull, 0 );
#endif

#if PACKETVER >= 20191224
	parseable_packet( HEADER_CZ_SE_CASHSHOP_OPEN2, sizeof( struct PACKET_CZ_SE_CASHSHOP_OPEN2 ), clif_parse_cashshop_open_request, 0 );
#endif

#if PACKETVER >= 20200724
	parseable_packet(0x00b24, 6, clif_parse_TraitStatusUp, 2, 4);
#endif

#endif /* CLIF_PACKETDB_HPP */

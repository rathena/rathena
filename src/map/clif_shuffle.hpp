// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CLIF_SHUFFLE_HPP
#define CLIF_SHUFFLE_HPP

// 2013-05-15aRagexe
#if PACKETVER == 20130515
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x0862,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0887,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08A1,6,clif_parse_TakeItem,2);
	//parseable_packet(0x08AA,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08AC,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x092D,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0931,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x093e,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0943,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0944,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0947,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0962,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0963,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-05-22Ragexe
#elif PACKETVER == 20130522
	parseable_packet(0x0360,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0362,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0368,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0369,6,clif_parse_SolveCharName,2);
	parseable_packet(0x07EC,6,clif_parse_TickSend,2);
	parseable_packet(0x0811,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x086A,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x086E,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0874,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x087E,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x088e,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x089B,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x089C,6,clif_parse_DropItem,2,4);
	parseable_packet(0x08A2,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08A9,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x08AC,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08a3,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a6,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x08aa,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0925,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0926,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x093e,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0950,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0952,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x095C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x095E,6,clif_parse_TakeItem,2);
	parseable_packet(0x095b,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0964,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	//parseable_packet(0x0965,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
// 2013-05-29Ragexe
#elif PACKETVER == 20130529
	parseable_packet(0x023B,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085A,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x085E,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0863,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0869,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0874,18,clif_parse_PartyBookingRegisterReq,2);
	parseable_packet(0x0876,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0877,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x0888,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x088E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0890,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0892,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0895,6,clif_parse_TakeItem,2);
	parseable_packet(0x0897,6,clif_parse_TickSend,2);
	parseable_packet(0x08A7,6,clif_parse_DropItem,2,4);
	parseable_packet(0x08A8,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0917,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0918,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0919,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0936,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0937,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0938,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0941,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0951,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0956,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0957,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0958,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0964,2,clif_parse_ReqCloseBuyingStore,0);
// 2013-06-05Ragexe
#elif PACKETVER == 20130605
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0883,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-06-12Ragexe
#elif PACKETVER == 20130612
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087E,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0919,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x093A,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0940,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0964,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-06-18Ragexe
#elif PACKETVER == 20130618
	parseable_packet(0x0281,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x02C4,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0363,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x085A,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0862,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0864,36,clif_parse_StoragePassword,2,4,20);
	//parseable_packet(0x0878,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x087A,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0885,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0887,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0889,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x088E,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0890,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0891,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x08A6,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08A7,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0917,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0930,6,clif_parse_TickSend,2);
	parseable_packet(0x0932,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0936,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0942,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0944,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0945,6,clif_parse_SolveCharName,2);
	parseable_packet(0x094F,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0951,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0953,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x095B,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0962,6,clif_parse_TakeItem,2);
	parseable_packet(0x096A,10,clif_parse_UseSkillToPos,2,4,6,8);
// 2013-06-26Ragexe
#elif PACKETVER == 20130626
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0365,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x0860,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088B,6,clif_parse_TakeItem,2);
	parseable_packet(0x088C,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x088F,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0894,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0895,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08A5,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x08AB,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0921,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0930,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x094D,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0952,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0960,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-07-03Ragexe
#elif PACKETVER == 20130703
	parseable_packet(0x0202,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0873,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0930,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x094A,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-07-10Ragexe
#elif PACKETVER == 20130710
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0880,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-07-17Ragexe
#elif PACKETVER == 20130717
	parseable_packet(0x02C4,6,clif_parse_TickSend,2);
	parseable_packet(0x0819,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x083C,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0862,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0863,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x086B,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	//parseable_packet(0x086C,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0882,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x088A,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x088C,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0897,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0898,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x089B,8,clif_parse_MoveToKafra,2,4);
	//parseable_packet(0x08A6,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08A9,6,clif_parse_TakeItem,2);
	parseable_packet(0x08AA,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0917,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0918,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x091D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x091E,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x092F,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x093B,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0952,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0956,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0958,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x095B,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0960,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0967,6,clif_parse_SolveCharName,2);
	parseable_packet(0x096A,2,clif_parse_ReqCloseBuyingStore,0);
// 2013-08-07Ragexe
#elif PACKETVER == 20130807
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0887,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2013-12-23Ragexe
#elif PACKETVER == 20131223
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08A4,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2014-10-16Ragexe
#elif PACKETVER == 20141016
	parseable_packet(0x022D,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086E,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0922,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0936,36,clif_parse_StoragePassword,0);
	parseable_packet(0x094B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0967,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2014-10-22bRagexe
#elif PACKETVER == 20141022
	parseable_packet(0x023b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0878,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x087d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0896,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0899,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08aa,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x08ab,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08ad,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x091a,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x092b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x093b,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0940,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x094e,6,clif_parse_TakeItem,2);
	parseable_packet(0x0955,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-01-07aRagexeRE
#elif PACKETVER == 20150107
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087c,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0895,36,clif_parse_StoragePassword,0);
	parseable_packet(0x092d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0943,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0947,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-01-14aRagexe
#elif PACKETVER == 20150114
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0868,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0899,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0946,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0955,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0957,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-01-28aRagexe
#elif PACKETVER == 20150128
	parseable_packet(0x0202,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x023b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x035f,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0365,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	//parseable_packet(0x0368,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0838,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x085a,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0864,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x086d,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0870,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0874,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0875,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0876,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x087d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0888,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x089a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08ab,6,clif_parse_TakeItem,2);
	parseable_packet(0x091f,6,clif_parse_TickSend,2);
	parseable_packet(0x0927,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0929,36,clif_parse_StoragePassword,0);
	parseable_packet(0x092d,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0938,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x093a,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0944,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x094d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x094e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0952,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0963,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0968,6,clif_parse_DropItem,2,4);
// 2015-02-04aRagexe
#elif PACKETVER == 20150204
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0966,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-02-25aRagexeRE or 2015-02-26aRagexeRE
#elif PACKETVER == 20150225 || PACKETVER == 20150226
	parseable_packet(0x02c4,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0362,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,6,clif_parse_TakeItem,2);
	parseable_packet(0x0819,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0867,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0885,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0896,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x089b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x089c,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a4,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0940,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0946,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0948,6,clif_parse_DropItem,2,4);
	parseable_packet(0x094f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0952,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0955,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096a,19,clif_parse_WantToConnection,2,6,10,14,18);
// 2015-03-11aRagexeRE
#elif PACKETVER == 20150311
	parseable_packet(0x023b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0360,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0436,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0438,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0838,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x086a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x086c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x087b,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0883,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0886,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0888,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0896,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a1,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08a3,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a5,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08a6,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x091c,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0928,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x092a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x092e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x093b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0943,6,clif_parse_SolveCharName,2);
	//parseable_packet(0x0946,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0957,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0958,6,clif_parse_TickSend,2);
	parseable_packet(0x095b,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0963,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0964,8,clif_parse_MoveToKafra,2,4);
// 2015-03-25aRagexe
#elif PACKETVER == 20150325
	parseable_packet(0x0202,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0363,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0365,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0438,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0802,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0819,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x085d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x086f,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x087c,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x087e,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0883,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0885,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0891,6,clif_parse_GetCharNameRequest,2);
	//parseable_packet(0x0893,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0897,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0899,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08a1,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a7,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0919,36,clif_parse_StoragePassword,0);
	parseable_packet(0x092c,6,clif_parse_TakeItem,2);
	parseable_packet(0x0931,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0932,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0938,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0940,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0947,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x094a,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0950,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0954,6,clif_parse_TickSend,2);
	parseable_packet(0x0969,8,clif_parse_MoveFromKafra,2,4);
// 2015-04-01aRagexe
#elif PACKETVER == 20150401
	parseable_packet(0x0362,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0367,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0437,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x083c,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x085e,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x086f,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0875,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x087e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x088f,6,clif_parse_TickSend,2);
	parseable_packet(0x0895,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0898,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x089c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a5,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x091c,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0922,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0924,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0938,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0939,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x093a,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x093b,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x093e,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0946,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0949,6,clif_parse_TakeItem,2);
	parseable_packet(0x094b,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0953,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x095f,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0964,5,clif_parse_ChangeDir,2,4);
// 2015-04-22aRagexeRE
#elif PACKETVER == 20150422
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0955,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-04-29aRagexe
#elif PACKETVER == 20150429
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0867,36,clif_parse_StoragePassword,0);
	parseable_packet(0x086a,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0886,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x088f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0894,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0899,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x089f,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x08a6,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	//parseable_packet(0x08a8,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08ad,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0929,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x093d,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0943,6,clif_parse_TakeItem,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-05-07bRagexe
#elif PACKETVER == 20150507
	parseable_packet(0x023b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,6,clif_parse_TakeItem,2);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0864,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0887,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0889,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0924,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x092e,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x093b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0941,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x0942,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0953,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0955,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0958,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-05-13aRagexe
#elif PACKETVER == 20150513
	parseable_packet(0x022D,2,clif_parse_ReqCloseBuyingStore,0);
	//parseable_packet(0x02C4,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0864,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0879,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0883,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0885,6,clif_parse_DropItem,2,4);
	parseable_packet(0x08A8,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0923,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0924,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x0927,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x094A,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0958,6,clif_parse_TakeItem,2);
	parseable_packet(0x0960,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2015-05-27aRagexe
#elif PACKETVER == 20150527
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x083c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0940,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-06-17aRagexeRE or 2015-06-18aRagexeRE
#elif PACKETVER == 20150617 || PACKETVER == 20150618
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_TakeItem,2);
	parseable_packet(0x0362,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0365,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07ec,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x0811,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0869,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x086a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x086b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0870,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x087a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0886,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0894,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0940,6,clif_parse_DropItem,2,4);
	parseable_packet(0x094e,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-08-19aRagexeRE
#elif PACKETVER == 20150819
	parseable_packet(0x0202,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x022d,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0281,6,clif_parse_TakeItem,2);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x085d,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x0862,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0865,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0871,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0888,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0919,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091e,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0927,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0940,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0961,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0967,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-08-26aRagexeRE
#elif PACKETVER == 20150826
	parseable_packet(0x0362,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0368,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0436,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x07ec,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0819,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0861,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0865,5,clif_parse_WalkToXY,2);
	parseable_packet(0x086b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0870,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x087b,6,clif_parse_SolveCharName,2);
	parseable_packet(0x088b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x088d,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0890,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0891,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08a0,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08a1,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a4,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a8,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0924,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0928,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x092e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x093b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0945,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x094f,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0951,6,clif_parse_TickSend,2);
	parseable_packet(0x0959,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0964,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0968,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0969,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
// 2015-09-16Ragexe
#elif PACKETVER == 20150916
	parseable_packet(0x022D,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0817,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0835,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x085E,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0869,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0873,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0877,5,clif_parse_WalkToXY,2);
	parseable_packet(0x087F,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0881,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x089B,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x089C,6,clif_parse_TakeItem,2);
	parseable_packet(0x089E,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x08AC,6,clif_parse_TickSend,2);
	parseable_packet(0x0920,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0924,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x092E,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x092F,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0934,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0936,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x0938,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x093E,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0941,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0942,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0948,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//parseable_packet(0x094F,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x095A,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0960,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0961,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0969,19,clif_parse_WantToConnection,2,6,10,14,18);
// 2015-10-01bRagexeRE
#elif PACKETVER == 20151001
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,nullptr,2,4,6);
	parseable_packet(0x0366,90,nullptr,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0860,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-10-07aRagexeRE
#elif PACKETVER == 20151007
	parseable_packet(0x0202,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x0862,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x093f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x095f,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0961,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0967,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-10-14bRagexeRE
#elif PACKETVER == 20151014
	parseable_packet(0x0202,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0817,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0838,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x085a,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085c,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0860,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0863,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0867,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0872,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0874,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0881,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0883,6,clif_parse_TickSend,2);
	parseable_packet(0x0884,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0889,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x088e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	//parseable_packet(0x089a,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x089b,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x089f,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08aa,6,clif_parse_TakeItem,2);
	parseable_packet(0x091c,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x091d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0930,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0934,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0944,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x094f,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0956,6,clif_parse_SolveCharName,2);
	parseable_packet(0x095e,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0961,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0964,19,clif_parse_WantToConnection,2,6,10,14,18);
// 2015-10-22aRagexeRE
#elif PACKETVER == 20151022
	parseable_packet(0x023B,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x02C4,36,clif_parse_StoragePassword,0);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086A,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x091D,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0940,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2015-10-28cRagexeRE
#elif PACKETVER == 20151028
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0860,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-10-29aRagexe
#elif PACKETVER == 20151029
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0860,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2015-11-04aRagexe
#elif PACKETVER == 20151104
	parseable_packet(0x023B,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0360,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0363,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0437,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07EC,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0811,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0815,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0886,6,clif_parse_TickSend,2);
	parseable_packet(0x0887,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x088B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x088D,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x08A3,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08A5,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0928,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x0939,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x093A,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0940,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x0964,6,clif_parse_TakeItem,2);
// 2015-11-11aRagexeRE
#elif PACKETVER == 20151111
	parseable_packet(0x02C4,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x0802,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085D,-1,clif_parse_ItemListWindowSelected,2,4,8,8,12);
	parseable_packet(0x0862,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0871,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0885,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x089C,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0942,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x094A,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0958,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0966,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0967,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0969,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2015-11-18aRagexeRE
#elif PACKETVER == 20151118
	parseable_packet(0x022d,6,clif_parse_TickSend,2);
	parseable_packet(0x035f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0365,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086b,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x088b,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x08ab,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0921,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0925,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x092e,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x092f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x093c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0943,6,clif_parse_TakeItem,2);
	parseable_packet(0x0946,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x0957,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095c,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-11-25dRagexeRE
#elif PACKETVER == 20151125
	parseable_packet(0x0361,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0365,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0366,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0368,-1,clif_parse_ItemListWindowSelected,2,4,8,8,12);
	parseable_packet(0x0438,6,clif_parse_TakeItem,2);
	parseable_packet(0x0802,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0838,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x085E,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x085F,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0863,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0883,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0884,36,clif_parse_StoragePassword,2,4,20);
	//parseable_packet(0x0885,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x088C,6,clif_parse_TickSend,2);
	parseable_packet(0x088D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0899,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x089C,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x089F,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x08A9,6,clif_parse_DropItem,2,4);
	parseable_packet(0x08AD,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0920,6,clif_parse_SolveCharName,2);
	parseable_packet(0x092A,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x092E,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0939,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x093E,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0951,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0956,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0957,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0959,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
// 2015-12-02bRagexeRE
#elif PACKETVER == 20151202
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0870,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2015-12-09aRagexeRE
#elif PACKETVER == 20151209
	parseable_packet(0x0365,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0369,6,clif_parse_SolveCharName,2);
	parseable_packet(0x07E4,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x07EC,6,clif_parse_TakeItem,2);
	parseable_packet(0x0811,6,clif_parse_TickSend,2);
	parseable_packet(0x0819,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x085B,36,clif_parse_StoragePassword,2,4,20);
	parseable_packet(0x085D,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x085E,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0861,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0866,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0875,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x087A,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x087F,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x088E,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x088F,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0894,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x08A1,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0920,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x092D,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0930,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0932,-1,clif_parse_ItemListWindowSelected,2,4,8,8,12);
	parseable_packet(0x093B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0948,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x094A,18,clif_parse_PartyBookingRegisterReq,2,4,6);
	parseable_packet(0x0956,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	//parseable_packet(0x095C,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0961,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0964,26,clif_parse_FriendsListAdd,2);
// 2015-12-16aRagexeRE
#elif PACKETVER == 20151216
	parseable_packet(0x022D,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0361,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0364,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0436,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x085B,6,clif_parse_TickSend,2);
	parseable_packet(0x0864,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0865,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x086E,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x086a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0870,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0874,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0885,36,clif_parse_StoragePassword,0);
	parseable_packet(0x088B,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x089D,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x089E,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08A2,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08A9,6,clif_parse_TakeItem,2);
	parseable_packet(0x08AC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091D,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0944,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0947,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0949,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0954,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0960,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0966,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0968,10,clif_parse_UseSkillToId,2,4,6);
// 2015-12-23bRagexeRE
#elif PACKETVER == 20151223
	parseable_packet(0x02c4,8,clif_parse_MoveToKafra,2,4);
	//parseable_packet(0x0362,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0802,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0815,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0864,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0866,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x086e,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0872,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0875,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0876,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0881,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0884,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0886,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x088d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0890,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0891,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0898,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x08aa,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0918,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x091a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x091b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0920,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0923,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0924,6,clif_parse_TickSend,2);
	parseable_packet(0x095e,6,clif_parse_SolveCharName,2);
	parseable_packet(0x095f,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0965,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0967,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
// 2016-01-06aRagexeRE
#elif PACKETVER == 20160106
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07ec,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0861,6,clif_parse_TakeItem,2);
	parseable_packet(0x086a,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x086c,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0878,36,clif_parse_StoragePassword,0);
	parseable_packet(0x087a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x087f,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0885,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0889,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x088a,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0891,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a0,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x091d,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x0940,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-01-13cRagexeRE
#elif PACKETVER == 20160113
	parseable_packet(0x022d,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x023b,5,clif_parse_WalkToXY,2);
	parseable_packet(0x035f,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0815,36,clif_parse_StoragePassword,0);
	parseable_packet(0x085b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0864,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x086d,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0873,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0875,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0888,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x088c,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0892,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0893,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0899,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x089a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a0,6,clif_parse_TickSend,2);
	parseable_packet(0x08a6,6,clif_parse_TakeItem,2);
	parseable_packet(0x08aa,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0919,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x091b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0924,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0930,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0932,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x093c,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0941,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x094d,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x094f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0967,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
// 2016-01-20aRagexeRE
#elif PACKETVER == 20160120
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0865,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-01-27bRagexeRE
#elif PACKETVER == 20160127
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085e,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0922,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x095a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0961,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-02-03aRagexeRE
#elif PACKETVER == 20160203
	parseable_packet(0x0202,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0437,6,clif_parse_TickSend,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0811,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0835,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0872,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0873,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x088c,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x0918,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x093e,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0940,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0947,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0954,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x095a,6,clif_parse_TakeItem,2);
	parseable_packet(0x095d,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-02-11aRagexeRE
#elif PACKETVER == 20160211
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086c,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0870,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0886,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-02-17cRagexeRE
#elif PACKETVER == 20160217
	parseable_packet(0x0202,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x023b,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0362,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0365,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0864,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0870,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0873,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x087a,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0888,6,clif_parse_TickSend,2);
	parseable_packet(0x088d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x088f,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0899,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08a0,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a9,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08ac,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08ad,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x091d,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0920,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0926,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x092e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x093b,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x093e,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0941,6,clif_parse_TakeItem,2);
	parseable_packet(0x094a,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x094f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x095e,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0966,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0967,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0969,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
// 2016-02-24bRagexeRE
#elif PACKETVER == 20160224
	parseable_packet(0x022d,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0364,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0436,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0861,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x086b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0884,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0885,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0888,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08a9,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x0920,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0929,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x092f,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0936,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0938,6,clif_parse_TakeItem,2);
	parseable_packet(0x094c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0961,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-03-02bRagexeRE
#elif PACKETVER == 20160302
	parseable_packet(0x022d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0367,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0802,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0819,5,clif_parse_WalkToXY,2);
	parseable_packet(0x085b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0864,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0865,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0867,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0868,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0873,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0875,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x087a,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x087d,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0883,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08a6,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08a9,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x091a,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0927,6,clif_parse_TakeItem,2);
	//parseable_packet(0x092d,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x092f,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0945,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x094e,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0950,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0957,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x095a,6,clif_parse_TickSend,2);
	parseable_packet(0x0960,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0961,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0967,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0968,7,clif_parse_ActionRequest,2,6);
// 2016-03-09aRagexeRE
#elif PACKETVER == 20160309
	parseable_packet(0x023b,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0281,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0364,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0819,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0838,5,clif_parse_WalkToXY,2);
	parseable_packet(0x083c,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x085a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x085f,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0866,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x086a,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	//parseable_packet(0x0873,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x087e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x089b,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x089d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a7,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x091d,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0920,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0922,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//parseable_packet(0x0929,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x092a,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x092e,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0932,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x094f,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0956,6,clif_parse_TickSend,2);
	parseable_packet(0x095e,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x096a,6,clif_parse_TakeItem,2);
// 2016-03-16aRagexeRE
#elif PACKETVER == 20160316
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0922,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-03-23aRagexeRE
#elif PACKETVER == 20160323
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0365,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x0867,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0869,6,clif_parse_TakeItem,2);
	parseable_packet(0x086a,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0872,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0878,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0883,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0896,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x089a,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x091b,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0926,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0927,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0933,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x093c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-03-30aRagexeRE
#elif PACKETVER == 20160330
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0365,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0867,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x086d,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0878,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087f,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0889,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088b,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x088d,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0918,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0925,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x092a,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x092c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0930,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0939,6,clif_parse_DropItem,2,4);
	parseable_packet(0x093b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-04-06aRagexeRE
#elif PACKETVER == 20160406
	parseable_packet(0x0364,6,clif_parse_SolveCharName,2);
	parseable_packet(0x07e4,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0819,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x085a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x085c,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0869,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0877,6,clif_parse_TakeItem,2);
	parseable_packet(0x0878,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0879,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0884,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0892,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0895,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0898,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x089b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x089e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a1,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a9,2,clif_parse_ReqCloseBuyingStore,0);
	//parseable_packet(0x08ac,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0927,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x092d,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0933,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0934,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0940,6,clif_parse_TickSend,2);
	parseable_packet(0x0949,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x094d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0953,36,clif_parse_StoragePassword,0);
	parseable_packet(0x095d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x095f,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x0962,4,nullptr,0); // CZ_GANGSI_RANK
// 2016-04-14bRagexeRE
#elif PACKETVER == 20160414
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0363,6,clif_parse_TakeItem,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0862,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x087a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0880,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0885,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089e,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0918,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0922,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0927,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0931,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0934,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0945,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0953,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-04-20aRagexeRE
#elif PACKETVER == 20160420
	parseable_packet(0x022d,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x02c4,6,clif_parse_TickSend,2);
	parseable_packet(0x035f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0864,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0870,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0872,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0874,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0884,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0888,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x088b,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a5,36,clif_parse_StoragePassword,0);
	parseable_packet(0x092f,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0935,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x094e,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x095c,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-04-27aRagexeRE
#elif PACKETVER == 20160427
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0835,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0940,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-05-04aRagexeRE
#elif PACKETVER == 20160504
	parseable_packet(0x0202,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0363,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//parseable_packet(0x0365,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x083c,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x085f,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x086b,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x087f,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0884,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0886,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0887,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x088a,6,clif_parse_TakeItem,2);
	parseable_packet(0x088d,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x088f,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0890,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0893,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0898,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x089d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08ad,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0918,6,clif_parse_TickSend,2);
	parseable_packet(0x0921,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0922,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0924,5,clif_parse_WalkToXY,2);
	parseable_packet(0x093e,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0940,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0941,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0948,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0952,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x095b,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0969,36,clif_parse_StoragePassword,0);
// 2016-05-11aRagexeRE
#elif PACKETVER == 20160511
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085e,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0894,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x089b,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0918,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0920,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0940,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-05-18aRagexeRE
#elif PACKETVER == 20160518
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086c,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0874,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x089a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a9,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0928,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-05-25aRagexeRE
#elif PACKETVER == 20160525
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x085a,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x085e,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0867,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x086a,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0899,6,clif_parse_TakeItem,2);
	parseable_packet(0x089c,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x091d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x092c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0937,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0945,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x094a,8,clif_parse_MoveToKafra,2,4);
	//parseable_packet(0x094e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0951,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0956,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-06-01aRagexeRE
#elif PACKETVER == 20160601
	parseable_packet(0x0202,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x02c4,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0863,6,clif_parse_TakeItem,2);
	parseable_packet(0x0870,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x087d,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x088f,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0895,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x08a7,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08ac,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0924,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x095f,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0961,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-06-08aRagexeRE
#elif PACKETVER == 20160608
	parseable_packet(0x022d,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x02c4,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x035f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0437,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07ec,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0802,6,clif_parse_TickSend,2);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x085c,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0885,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0889,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0899,6,clif_parse_TakeItem,2);
	parseable_packet(0x089b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08a6,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x093b,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x094d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0958,36,clif_parse_StoragePassword,0);
	parseable_packet(0x095b,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0969,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-06-15aRagexeRE
#elif PACKETVER == 20160615
	parseable_packet(0x0281,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0363,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0364,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0369,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x083c,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0866,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x0870,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x087d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x087e,6,clif_parse_TakeItem,2);
	parseable_packet(0x087f,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	//parseable_packet(0x0884,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0887,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0888,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x088a,6,clif_parse_TickSend,2);
	parseable_packet(0x088d,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0891,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0898,6,clif_parse_DropItem,2,4);
	parseable_packet(0x092f,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x093e,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0947,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0948,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x094a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x094b,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0954,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0957,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0958,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x095c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x095e,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0961,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
// 2016-06-22aRagexeRE
#elif PACKETVER == 20160622
	parseable_packet(0x023b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x035f,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0361,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x07e4,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0861,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x0865,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x0867,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0880,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0887,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0890,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0891,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0892,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x089a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x089e,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a2,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08a8,36,clif_parse_StoragePassword,0);
	parseable_packet(0x091c,6,clif_parse_TakeItem,2);
	parseable_packet(0x092d,6,clif_parse_TickSend,2);
	parseable_packet(0x092f,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0936,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0937,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x093b,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x093f,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0946,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0959,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0965,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0969,6,clif_parse_DropItem,2,4);
// 2016-06-29aRagexeRE or 2016-06-30aRagexeRE
#elif PACKETVER == 20160629 || PACKETVER == 20160630
	parseable_packet(0x0202,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x022d,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x035f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0363,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0368,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x085c,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	//parseable_packet(0x085e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0860,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0861,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0863,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0867,36,clif_parse_StoragePassword,0);
	parseable_packet(0x086b,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0881,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0885,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x088e,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0893,6,clif_parse_TickSend,2);
	parseable_packet(0x091e,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0922,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0925,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0926,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x093e,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0946,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0948,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x094a,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0957,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x095a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0968,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0969,6,clif_parse_TakeItem,2);
	parseable_packet(0x096a,8,clif_parse_MoveToKafra,2,4);
// 2016-07-06cRagexeRE
#elif PACKETVER == 20160706
	parseable_packet(0x0362,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0436,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x085f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0860,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0869,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x086b,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0884,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x0886,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0889,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0892,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0899,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08a4,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08a5,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x08a8,6,clif_parse_TickSend,2);
	parseable_packet(0x0918,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x091b,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0924,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0926,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0927,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0929,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x092d,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0939,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x093d,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0944,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0945,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x094c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0952,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0957,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0958,6,clif_parse_TakeItem,2);
// 2016-07-13bRagexeRE
#elif PACKETVER == 20160713
	parseable_packet(0x022d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0363,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0364,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0838,6,clif_parse_TakeItem,2);
	parseable_packet(0x0860,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0865,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0869,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0875,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0877,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x087b,6,clif_parse_TickSend,2);
	parseable_packet(0x0883,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x088d,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0892,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x089a,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x089f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08a2,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a4,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x091c,5,clif_parse_WalkToXY,2);
	parseable_packet(0x091d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0921,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0922,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x092c,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0931,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0939,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0944,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0945,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0947,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0957,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x095b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
// 2016-07-20aRagexeRE
#elif PACKETVER == 20160720
	parseable_packet(0x0362,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0363,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0365,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x07e4,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0819,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0838,5,clif_parse_WalkToXY,2);
	parseable_packet(0x085b,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x086a,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x086d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x087f,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0883,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0887,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0897,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x089a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x089c,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x089e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a0,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08aa,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x0917,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x091c,6,clif_parse_TakeItem,2);
	parseable_packet(0x092a,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x093b,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x093e,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0946,6,clif_parse_TickSend,2);
	parseable_packet(0x094d,6,clif_parse_SolveCharName,2);
	//parseable_packet(0x0953,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x095b,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0960,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0969,26,clif_parse_PartyInvite2,2);
// 2016-07-27bRagexeRE
#elif PACKETVER == 20160727
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x023b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0362,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0363,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0436,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0438,6,clif_parse_TickSend,2);
	parseable_packet(0x07ec,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0866,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0868,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0869,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0874,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0877,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0883,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0887,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x088e,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0891,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x089f,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x08a2,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a4,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08a7,6,clif_parse_TakeItem,2);
	parseable_packet(0x092e,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0936,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0941,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0946,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0949,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0951,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x095f,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0966,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0969,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
// 2016-08-03bRagexeRE
#elif PACKETVER == 20160803
	parseable_packet(0x0364,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x085d,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0878,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x087f,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0881,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0886,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0887,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0888,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088b,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0891,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0895,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x089c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089e,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x08a1,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x091b,6,clif_parse_TakeItem,2);
	parseable_packet(0x0929,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0930,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0932,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0934,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0937,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x093a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x093e,6,clif_parse_TickSend,2);
	parseable_packet(0x093f,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0952,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0955,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0956,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0959,6,clif_parse_SolveCharName,2);
	parseable_packet(0x095a,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x096a,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
// 2016-08-10aRagexeRE
#elif PACKETVER == 20160810
	parseable_packet(0x0361,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0819,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0838,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x085d,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x085e,6,clif_parse_DropItem,2,4);
	parseable_packet(0x085f,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0860,6,clif_parse_SolveCharName,2);
	parseable_packet(0x086f,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0875,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0879,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x087a,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0885,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0888,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0890,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x089d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x089f,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x08a9,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091a,6,clif_parse_TakeItem,2);
	parseable_packet(0x091b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x091c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x0926,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x092b,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x092d,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0935,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0943,6,clif_parse_TickSend,2);
	parseable_packet(0x094b,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0959,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x095b,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0967,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
// 2016-08-31bRagexeRE
#elif PACKETVER == 20160831
	parseable_packet(0x022d,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0366,6,clif_parse_DropItem,2,4);
	parseable_packet(0x07ec,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0835,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0865,5,clif_parse_WalkToXY,2);
	parseable_packet(0x086d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0870,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0874,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0876,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0878,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x087c,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x08a8,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x08a9,6,clif_parse_TickSend,2);
	parseable_packet(0x0917,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x091b,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x092c,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x092e,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0938,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x093a,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0946,6,clif_parse_SolveCharName,2);
	parseable_packet(0x094a,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x094f,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0950,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0954,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0957,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x095e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0960,6,clif_parse_TakeItem,2);
	parseable_packet(0x0964,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0967,10,clif_parse_UseSkillToId,2,4,6);
// 2016-09-07aRagexeRE
#elif PACKETVER == 20160907
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x091c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-09-13aRagexeRE
#elif PACKETVER == 20160913
	parseable_packet(0x0361,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0817,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x085b,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0865,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0874,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0875,6,clif_parse_TickSend,2);
	parseable_packet(0x0879,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x087a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087b,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0887,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0889,6,clif_parse_TakeItem,2);
	parseable_packet(0x088e,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x088f,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0891,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0892,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x089b,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a5,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0928,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0935,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x093a,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0949,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x094a,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0950,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0952,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0954,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x0962,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0963,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0968,2,clif_parse_ReqCloseBuyingStore,0);
// 2016-09-21bRagexeRE
#elif PACKETVER == 20160921
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x094a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-09-28dRagexeRE
#elif PACKETVER == 20160928
	parseable_packet(0x0202,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0366,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0436,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0811,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0838,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0864,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0866,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x086d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0872,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0878,6,clif_parse_SolveCharName,2);
	parseable_packet(0x087f,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0889,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x088e,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0897,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x089a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a2,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a9,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0919,5,clif_parse_WalkToXY,2);
	parseable_packet(0x091e,6,clif_parse_TickSend,2);
	parseable_packet(0x0927,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x092d,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0944,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x094d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x094e,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0953,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0955,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0957,6,clif_parse_TakeItem,2);
	//parseable_packet(0x095a,4,nullptr,0); // CZ_GANGSI_RANK
// 2016-10-05aRagexeRE
#elif PACKETVER == 20161005
	parseable_packet(0x0202,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0838,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0863,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0886,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088e,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0891,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0892,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x089b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x089c,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a0,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08ac,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08ad,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0918,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0919,6,clif_parse_SolveCharName,2);
	//parseable_packet(0x091e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x092b,6,clif_parse_TickSend,2);
	parseable_packet(0x0931,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0932,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x093b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0942,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0944,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0945,5,clif_parse_WalkToXY,2);
	parseable_packet(0x094a,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x094d,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0952,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x095b,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0967,10,clif_parse_UseSkillToId,2,4,6);
// 2016-10-12aRagexeRE
#elif PACKETVER == 20161012
	parseable_packet(0x023b,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0364,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0365,6,clif_parse_TickSend,2);
	parseable_packet(0x0369,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x07ec,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0819,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x085b,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x085e,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0863,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0868,6,clif_parse_TakeItem,2);
	parseable_packet(0x086d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0872,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0875,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0880,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0893,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a0,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x092d,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0936,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0937,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0939,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0943,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0944,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x094f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0951,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x095c,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0962,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0966,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0967,36,clif_parse_StoragePassword,0);
// 2016-10-19aRagexeRE
#elif PACKETVER == 20161019
	parseable_packet(0x022d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0361,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0889,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0892,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0946,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0963,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-10-26bRagexeRE
#elif PACKETVER == 20161026
	parseable_packet(0x0363,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0438,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0802,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x085a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x085f,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0861,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0862,6,clif_parse_TickSend,2);
	parseable_packet(0x086a,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x086c,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	//parseable_packet(0x086e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087a,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	//parseable_packet(0x087c,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x087f,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0886,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0891,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0894,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0898,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x091a,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x091b,6,clif_parse_TakeItem,2);
	parseable_packet(0x0926,6,clif_parse_SolveCharName,2);
	parseable_packet(0x092c,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x092e,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x092f,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0930,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x094b,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0953,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x095c,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x095e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0962,5,clif_parse_ChangeDir,2,4);
// 2016-11-02aRagexeRE or 2016-11-03aRagexeRE
#elif PACKETVER == 20161102 || PACKETVER == 20161103
	parseable_packet(0x0361,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x0367,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0436,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0802,6,clif_parse_TakeItem,2);
	parseable_packet(0x0838,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x083c,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x085f,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0869,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x086c,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x086f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0874,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0886,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0890,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08a2,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x08aa,5,clif_parse_WalkToXY,2);
	parseable_packet(0x091b,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0922,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0925,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0928,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x092f,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0936,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0946,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0949,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x095e,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0964,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0965,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0966,6,clif_parse_TickSend,2);
// 2016-11-09bRagexeRE
#elif PACKETVER == 20161109
	parseable_packet(0x02c4,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0361,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0362,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,6,clif_parse_TickSend,2);
	parseable_packet(0x0366,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,6,clif_parse_SolveCharName,2);
	parseable_packet(0x085d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x085e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0865,5,clif_parse_WalkToXY,2);
	parseable_packet(0x086a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x086d,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0870,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0876,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x087a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0881,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x088e,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0891,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0898,6,clif_parse_TakeItem,2);
	parseable_packet(0x089a,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089d,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//parseable_packet(0x089f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08a7,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08ad,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0927,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0937,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x093c,36,clif_parse_StoragePassword,0);
	parseable_packet(0x093f,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0954,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0956,5,clif_parse_ChangeDir,2,4);
// 2016-11-16cRagexeRE
#elif PACKETVER == 20161116
	parseable_packet(0x0368,6,clif_parse_TickSend,2);
	parseable_packet(0x0369,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0835,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x085f,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0864,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x086f,6,clif_parse_TakeItem,2);
	parseable_packet(0x0885,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x088b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x088d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x088f,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0890,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0892,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0893,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a1,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a2,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08aa,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08ac,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0920,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0925,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x092a,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0931,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x093c,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x094a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0952,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0957,6,clif_parse_SolveCharName,2);
	parseable_packet(0x095b,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x095d,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095f,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0967,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
// 2016-11-23aRagexeRE
#elif PACKETVER == 20161123
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0362,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0437,6,clif_parse_TickSend,2);
	parseable_packet(0x085c,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0861,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0862,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0866,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x086f,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0871,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x087f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0880,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0882,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x088b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x089c,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a9,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x08aa,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x091a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0926,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x092a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x092f,6,clif_parse_TakeItem,2);
	parseable_packet(0x0930,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0941,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x094d,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x094f,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	//parseable_packet(0x095a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x095b,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0962,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_SolveCharName,2);
// 2016-11-30bRagexeRE
#elif PACKETVER == 20161130
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,6,clif_parse_TickSend,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x088f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0931,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0943,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0954,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0959,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-12-07eRagexeRE
#elif PACKETVER == 20161207
	parseable_packet(0x023b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0867,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x0868,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0875,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x087e,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0886,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08a1,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a2,6,clif_parse_TakeItem,2);
	parseable_packet(0x08ad,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0918,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x091d,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x0943,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0965,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-12-14bRagexeRE
#elif PACKETVER == 20161214
	parseable_packet(0x022d,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0281,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x02c4,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0364,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0436,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	//parseable_packet(0x0819,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085a,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0862,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x086d,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0887,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0895,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0899,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08a6,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x092e,6,clif_parse_TakeItem,2);
	parseable_packet(0x093d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2016-12-21aRagexeRE
#elif PACKETVER == 20161221
	parseable_packet(0x035f,6,clif_parse_TakeItem,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0366,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0438,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0817,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x085b,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0866,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0876,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0881,6,clif_parse_GetCharNameRequest,2);
	//parseable_packet(0x0884,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0885,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088c,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0890,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0899,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x089a,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089b,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08aa,6,clif_parse_TickSend,2);
	parseable_packet(0x091e,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0926,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0928,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x092c,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x092e,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0930,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0943,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0946,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x094b,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x095a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0964,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0965,5,clif_parse_ChangeDir,2,4);
// 2016-12-28aRagexeRE
#elif PACKETVER == 20161228
	parseable_packet(0x0362,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x085a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x085e,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0865,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x086a,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x086c,6,clif_parse_TakeItem,2);
	parseable_packet(0x086d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0870,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0871,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0875,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x087f,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0886,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0889,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0893,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089f,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a2,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08a3,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x08a5,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08ab,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08ac,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08ad,36,clif_parse_StoragePassword,0);
	parseable_packet(0x091c,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0929,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x092c,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0934,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0935,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	//parseable_packet(0x0938,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x093d,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0944,6,clif_parse_TickSend,2);
// 2017-01-04bRagexeRE
#elif PACKETVER == 20170104
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x085a,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x087f,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0896,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x091b,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0940,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-01-11aRagexeRE
#elif PACKETVER == 20170111
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0877,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x087f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x088a,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a1,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a3,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a6,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x091a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x091b,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0940,6,clif_parse_DropItem,2,4);
	parseable_packet(0x094c,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0961,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0969,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-01-18aRagexeRE
#elif PACKETVER == 20170118
	parseable_packet(0x022d,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0364,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0862,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0865,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x086f,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0873,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x089e,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08ad,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x091f,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0927,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0933,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0958,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0962,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x096a,8,clif_parse_MoveToKafra,2,4);
// 2017-01-25aRagexeRE
#elif PACKETVER == 20170125
	parseable_packet(0x0438,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0811,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x086e,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0876,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0877,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0879,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087b,6,clif_parse_TakeItem,2);
	parseable_packet(0x087d,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0881,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x0884,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0893,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0894,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0895,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0898,6,clif_parse_SolveCharName,2);
	parseable_packet(0x089b,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x08a5,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x091b,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x091c,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0920,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0929,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x092b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0930,5,clif_parse_WalkToXY,2);
	parseable_packet(0x093c,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0943,6,clif_parse_TickSend,2);
	parseable_packet(0x0944,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x095c,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0965,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0968,2,clif_parse_SearchStoreInfoNextPage,0);
// 2017-02-01aRagexeRE
#elif PACKETVER == 20170201
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0815,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085d,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x085e,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0875,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0879,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0881,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0884,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0885,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0886,36,clif_parse_StoragePassword,0);
	parseable_packet(0x088b,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x08a4,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0919,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0920,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0938,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0940,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x094c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0966,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0969,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-02-08aRagexeRE
#elif PACKETVER == 20170208
	//parseable_packet(0x02c4,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0367,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085c,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0860,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x087a,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0892,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x08a1,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08ac,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0921,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0923,6,clif_parse_TakeItem,2);
	parseable_packet(0x092d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0932,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0937,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-02-15aRagexeRE
#elif PACKETVER == 20170215
	parseable_packet(0x02c4,36,clif_parse_StoragePassword,0);
	parseable_packet(0x035f,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0360,6,clif_parse_TickSend,2);
	parseable_packet(0x0811,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x083c,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x085c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0876,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x087c,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x087d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x087e,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0883,6,clif_parse_SolveCharName,2);
	//parseable_packet(0x0884,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088a,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x088b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x088c,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0890,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x0896,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x089b,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a2,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08a8,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x091c,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0925,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x092b,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x092d,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0942,6,clif_parse_TakeItem,2);
	parseable_packet(0x094e,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x095f,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0962,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0969,5,clif_parse_WalkToXY,2);
// 2017-02-22aRagexeRE
#elif PACKETVER == 20170222
	parseable_packet(0x0202,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085f,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0866,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0870,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0871,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x0877,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0889,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0894,6,clif_parse_TakeItem,2);
	parseable_packet(0x08a3,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08a8,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0937,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0939,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0943,36,clif_parse_StoragePassword,0);
	parseable_packet(0x095d,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0962,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-02-28aRagexeRE
#elif PACKETVER == 20170228
	parseable_packet(0x022d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0360,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0362,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0819,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x085e,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0863,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x086b,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0873,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0874,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0876,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0883,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0884,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0889,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0893,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x089e,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a0,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x08a2,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x08a6,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08a7,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x091f,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x092a,6,clif_parse_TakeItem,2);
	parseable_packet(0x092e,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0937,6,clif_parse_TickSend,2);
	//parseable_packet(0x093e,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0944,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0947,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0948,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0952,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0955,18,clif_parse_PartyBookingRegisterReq,2,4);
// 2017-03-08bRagexeRE
#elif PACKETVER == 20170308
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-03-15cRagexeRE
#elif PACKETVER == 20170315
	parseable_packet(0x02c4,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x035f,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0360,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0366,6,clif_parse_TakeItem,2);
	parseable_packet(0x0367,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0436,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x07ec,6,clif_parse_TickSend,2);
	//parseable_packet(0x085c,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0863,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x086a,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0872,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x087b,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0884,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x088b,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x088d,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088f,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0892,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x089c,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08aa,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091a,6,clif_parse_DropItem,2,4);
	parseable_packet(0x091b,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x091d,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0920,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0922,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0944,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x094a,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x094e,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0950,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0952,36,clif_parse_StoragePassword,0);
// 2017-03-22aRagexeRE
#elif PACKETVER == 20170322
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022d,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023b,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x091a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-03-29dRagexeRE
#elif PACKETVER == 20170329
	parseable_packet(0x0281,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0363,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085d,36,clif_parse_StoragePassword,0);
	parseable_packet(0x087a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0888,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x08a8,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0917,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0926,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0929,6,clif_parse_TakeItem,2);
	parseable_packet(0x092e,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0937,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0939,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0949,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x095f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-04-05bRagexeRE
#elif PACKETVER == 20170405
	parseable_packet(0x022d,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0281,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0363,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,6,clif_parse_TakeItem,2);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085f,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0860,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0864,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0865,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x086f,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0893,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08a5,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x094c,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x094f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0964,6,clif_parse_DropItem,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-04-12aRagexeRE
#elif PACKETVER == 20170412
	parseable_packet(0x023b,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0365,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0863,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0869,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x086d,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0878,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x0879,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x087b,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x088b,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0890,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0893,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0898,6,clif_parse_SolveCharName,2);
	parseable_packet(0x089a,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x089c,6,clif_parse_DropItem,2,4);
	parseable_packet(0x08a1,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x091a,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x091e,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0929,6,clif_parse_TickSend,2);
	//parseable_packet(0x092e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0938,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0942,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0945,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0949,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x094f,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0952,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0959,6,clif_parse_TakeItem,2);
	parseable_packet(0x095b,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x095c,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x095d,36,clif_parse_StoragePassword,0);
// 2017-04-19bRagexeRE
#elif PACKETVER == 20170419
	parseable_packet(0x0811,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0819,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0838,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x085a,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x085e,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0862,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0868,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x086a,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0872,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0881,36,clif_parse_StoragePassword,0);
	parseable_packet(0x088d,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088f,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0897,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0898,6,clif_parse_TickSend,2);
	parseable_packet(0x089d,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x08aa,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091b,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0920,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0922,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0930,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0931,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0935,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x093a,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x093f,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0942,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x095c,6,clif_parse_TakeItem,2);
	parseable_packet(0x095d,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x0963,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0965,6,clif_parse_ReqClickBuyingStore,2);
// 2017-04-26dRagexeRE
#elif PACKETVER == 20170426
	parseable_packet(0x0281,36,clif_parse_StoragePassword,0);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0866,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x086f,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087a,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0887,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0899,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089c,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08a2,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x08a4,6,clif_parse_TakeItem,2);
	//parseable_packet(0x091f,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0927,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0940,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0958,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0963,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-05-02dRagexeRE
#elif PACKETVER == 20170502
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035f,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07e4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07ec,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083c,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0875,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0894,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x089c,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x093c,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0950,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096a,6,clif_parse_GetCharNameRequest,2);
// 2017-05-17aRagexeRE
#elif PACKETVER == 20170517
	//parseable_packet(0x0364,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0367,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0437,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0802,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0815,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0817,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0868,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0875,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x087b,6,clif_parse_SolveCharName,2);
	parseable_packet(0x087d,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x088c,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x088d,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0894,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0896,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0899,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x089e,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x089f,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08a2,6,clif_parse_TickSend,2);
	parseable_packet(0x08a8,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08aa,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x091b,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0923,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x093b,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0945,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0946,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0947,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0958,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0960,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0964,6,clif_parse_TakeItem,2);
// 2017-05-24aRagexeRE
#elif PACKETVER == 20170524
	parseable_packet(0x0364,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0368,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0802,6,clif_parse_DropItem,2,4);
	parseable_packet(0x085e,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x085f,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0860,6,clif_parse_TickSend,2);
	parseable_packet(0x0864,6,clif_parse_TakeItem,2);
	parseable_packet(0x0866,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0868,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x086d,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0873,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0874,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x087d,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0882,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x088d,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0894,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089c,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08a1,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	//parseable_packet(0x091e,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0923,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0925,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0934,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0946,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x0958,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095a,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x095b,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0964,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0967,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0968,6,clif_parse_SolveCharName,2);
// 2017-05-31aRagexeRE
#elif PACKETVER == 20170531
	parseable_packet(0x0361,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0369,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x07e4,5,clif_parse_WalkToXY,2);
	parseable_packet(0x07ec,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0819,6,clif_parse_TickSend,2);
	//parseable_packet(0x085b,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x085f,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0861,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x0868,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0873,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0875,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0878,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x087b,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0885,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088b,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x088d,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0894,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x089a,36,clif_parse_StoragePassword,0);
	parseable_packet(0x089c,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08a2,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x08ac,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x08ad,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x092d,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0933,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0937,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0940,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0945,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0963,6,clif_parse_TakeItem,2);
	parseable_packet(0x0968,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
// 2017-06-07cRagexeRE
#elif PACKETVER == 20170607
	parseable_packet(0x0361,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0364,36,clif_parse_StoragePassword,0);
	parseable_packet(0x07e4,6,clif_parse_TickSend,2);
	parseable_packet(0x085a,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x085e,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0862,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0863,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0864,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0871,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0873,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0875,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0885,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x088a,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0897,6,clif_parse_TakeItem,2);
	parseable_packet(0x089d,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08a9,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08ab,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0917,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0918,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0919,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0925,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0927,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x0931,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0934,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0938,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x093d,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0942,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0944,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0949,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
// 2017-06-14bRagexeRE
#elif PACKETVER == 20170614
	parseable_packet(0x023B,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0361,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0364,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0367,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0437,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0838,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x083C,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0860,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0865,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0866,6,clif_parse_TickSend,2);
	parseable_packet(0x0867,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x086B,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x086C,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0877,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0879,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x087D,6,clif_parse_SolveCharName,2);
	parseable_packet(0x087E,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0889,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0899,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x089D,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x08A2,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x08AD,6,clif_parse_TakeItem,2);
	parseable_packet(0x091B,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0928,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x092F,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0936,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0944,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0957,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0963,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
// 2017-06-21aRagexeRE
#elif PACKETVER == 20170621
	parseable_packet(0x0202,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,6,clif_parse_TakeItem,2);
	parseable_packet(0x0365,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0366,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0802,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085D,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x087D,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0885,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0889,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08A8,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0956,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0957,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x095B,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x095C,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0961,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-06-28bRagexeRE
#elif PACKETVER == 20170628
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0863,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-07-05aRagexeRE
#elif PACKETVER == 20170705
	parseable_packet(0x0202,36,clif_parse_StoragePassword,0);
	parseable_packet(0x02C4,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0879,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0886,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x088D,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088E,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x089A,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x089D,6,clif_parse_DropItem,2,4);
	parseable_packet(0x091A,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x092F,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0930,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0932,6,clif_parse_TakeItem,2);
	parseable_packet(0x0934,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x094C,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-07-12bRagexeRE
#elif PACKETVER == 20170712
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0944,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-07-19aRagexeRE
#elif PACKETVER == 20170719
	parseable_packet(0x022D,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0367,2,clif_parse_ReqCloseBuyingStore,0);
	//parseable_packet(0x0368,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0369,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x07E4,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x085A,6,clif_parse_TickSend,2);
	parseable_packet(0x085E,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0863,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x086E,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x087D,6,clif_parse_TakeItem,2);
	parseable_packet(0x0881,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0882,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0885,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0891,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0898,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x089A,36,clif_parse_StoragePassword,0);
	parseable_packet(0x089D,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x08A6,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08A8,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x091B,6,clif_parse_DropItem,2,4);
	parseable_packet(0x091F,6,clif_parse_SolveCharName,2);
	parseable_packet(0x092C,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x092E,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x092F,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x093D,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x093E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0944,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0946,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0966,8,clif_parse_MoveToKafra,2,4);
// 2017-07-26cRagexeRE
#elif PACKETVER == 20170726
	parseable_packet(0x0363,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0364,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0366,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0369,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0438,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0838,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0873,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0874,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0878,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0881,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0888,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x088E,5,clif_parse_WalkToXY,2);
	//parseable_packet(0x08A3,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08A7,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08AA,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x08AB,6,clif_parse_TakeItem,2);
	parseable_packet(0x08AC,6,clif_parse_TickSend,2);
	parseable_packet(0x091D,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x091E,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x091F,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0921,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0923,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0943,6,clif_parse_DropItem,2,4);
	parseable_packet(0x094F,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0950,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0952,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0954,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x095A,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0963,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
// 2017-08-01aRagexeRE
#elif PACKETVER == 20170801
	parseable_packet(0x022D,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0281,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0362,5,clif_parse_HomMenu,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087D,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x08A6,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x094F,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x095A,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-08-09cRagexeRE 
//#elif PACKETVER == 20170809
// 2017-08-16dRagexeRE
#elif PACKETVER == 20170816
	parseable_packet(0x022D,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x035F,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0361,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0362,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0438,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x085A,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0862,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0864,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x087E,6,clif_parse_TakeItem,2);
	parseable_packet(0x0881,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0882,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0884,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0888,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0889,6,clif_parse_TickSend,2);
	parseable_packet(0x08A3,26,clif_parse_FriendsListAdd,2);
	//parseable_packet(0x08A7,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08A9,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08AC,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x091C,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0921,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0925,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x092C,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x093A,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x093D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0940,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0941,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0950,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0959,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0960,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
// 2017-08-23aRagexeRE
#elif PACKETVER == 20170823
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x086C,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x086D,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08AC,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x095B,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-08-30bRagexeRE
#elif PACKETVER == 20170830
	parseable_packet(0x0281,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x02C4,6,clif_parse_TakeItem,2);
	parseable_packet(0x0363,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0364,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0860,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0865,5,clif_parse_WalkToXY,2);
	parseable_packet(0x086A,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0875,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0884,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0885,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0888,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0897,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0899,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089A,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x089E,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08A2,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08A8,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x091E,6,clif_parse_TickSend,2);
	parseable_packet(0x0921,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0925,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x092E,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0939,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x093E,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0940,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0942,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0943,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0947,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0951,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0959,10,clif_parse_UseSkillToPos,2,4,6,8);
// 2017-09-06cRagexeRE
#elif PACKETVER == 20170906
	//parseable_packet(0x0202,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0281,36,clif_parse_StoragePassword,0);
	parseable_packet(0x02C4,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0802,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0860,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0866,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x086C,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08A2,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08A3,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x08A7,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x091A,6,clif_parse_TakeItem,2);
	parseable_packet(0x091E,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0953,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0281,6,clif_parse_GetCharNameRequest,2);
// 2017-09-13bRagexeRE
#elif PACKETVER == 20170913
	parseable_packet(0x0281,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x035F,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0437,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x07E4,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0817,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0835,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x085A,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0860,6,clif_parse_TakeItem,2);
	parseable_packet(0x0865,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0866,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x088C,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0890,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0891,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0892,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08A6,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x08A7,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08AA,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08AB,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08AC,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08AD,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x091B,6,clif_parse_TickSend,2);
	parseable_packet(0x091D,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x091E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0920,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0923,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0925,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0927,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x095A,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x095C,6,clif_parse_SolveCharName,2);
// 2017-09-20bRagexeRE
#elif PACKETVER == 20170920
	parseable_packet(0x0369,6,clif_parse_TakeItem,2);
	parseable_packet(0x0436,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x07EC,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x085A,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0861,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0862,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0864,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0865,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x086A,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x086C,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0874,2,clif_parse_ReqCloseBuyingStore,0);
	//parseable_packet(0x0875,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0889,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x088E,6,clif_parse_TickSend,2);
	parseable_packet(0x089B,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0919,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x091E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0921,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0923,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0926,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x092E,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0937,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0939,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0945,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x094C,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x095D,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0961,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0966,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x096A,2,clif_parse_SearchStoreInfoNextPage,0);
// 2017-09-27bRagexeRE or 2017-09-27dRagexeRE
#elif PACKETVER == 20170927
	parseable_packet(0x02C4,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x035F,6,clif_parse_GetCharNameRequest,2);
	//parseable_packet(0x0361,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0362,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0366,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x085C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0873,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0875,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x087D,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x087E,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x088B,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0899,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x089A,36,clif_parse_StoragePassword,0);
	parseable_packet(0x089B,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08A3,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08A5,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x08A6,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x08AD,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x091E,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0922,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0923,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0927,5,clif_parse_WalkToXY,2);
	parseable_packet(0x093B,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0942,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0945,6,clif_parse_TickSend,2);
	parseable_packet(0x094B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x094D,6,clif_parse_TakeItem,2);
	parseable_packet(0x0959,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x095A,10,clif_parse_UseSkillToPos,2,4,6,8);
// 2017-10-02cRagexeRE
#elif PACKETVER == 20171002
	parseable_packet(0x022D,6,clif_parse_DropItem,2,4);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0885,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0897,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0899,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x089D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0928,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x092D,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0934,36,clif_parse_StoragePassword,0);
	parseable_packet(0x093B,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x093D,6,clif_parse_TakeItem,2);
	//parseable_packet(0x093E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0943,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x095F,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-10-11bRagexeRE
#elif PACKETVER == 20171011
	parseable_packet(0x023B,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0882,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0950,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0954,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-10-18aRagexeRE
#elif PACKETVER == 20171018
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0363,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0364,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,6,clif_parse_TakeItem,2);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x086A,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x087A,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087E,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0889,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089A,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089F,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08A6,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0938,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0944,36,clif_parse_StoragePassword,0);
	parseable_packet(0x094A,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x094F,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-10-25bRagexeRE or 2017-10-25cRagexeRE or 2017-10-25dRagexeRE or 2017-10-25eRagexeRE
#elif PACKETVER == 20171025
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08A2,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-11-01bRagexeRE
#elif PACKETVER == 20171101
	parseable_packet(0x022D,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0368,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0369,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0438,6,clif_parse_TickSend,2);
	parseable_packet(0x0835,6,clif_parse_DropItem,2,4);
	parseable_packet(0x085B,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0860,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x086C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0872,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0876,5,clif_parse_ChangeDir,2,4);
	//parseable_packet(0x0886,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088E,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0890,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0895,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0899,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x089B,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x089C,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x08A0,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x08AB,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x08AD,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x091B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0939,5,clif_parse_WalkToXY,2);
	parseable_packet(0x094A,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x094D,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0952,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0957,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x095A,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0962,6,clif_parse_TakeItem,2);
	parseable_packet(0x0966,10,clif_parse_UseSkillToPos,2,4,6,8);
// 2017-11-08bRagexeRE
#elif PACKETVER == 20171108
	parseable_packet(0x0202,6,clif_parse_TickSend,2);
	parseable_packet(0x0361,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x07E4,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0815,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0819,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0838,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x085D,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0863,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0878,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x087E,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0884,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x0896,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0897,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x08A2,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x08A9,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08AD,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x091D,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x091F,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0940,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0941,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0945,6,clif_parse_TakeItem,2);
	//parseable_packet(0x0947,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0949,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x094E,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0958,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x095A,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0963,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0965,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0967,10,clif_parse_UseSkillToPos,2,4,6,8);
// 2017-11-15aRagexeRE
#elif PACKETVER == 20171115
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0365,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0802,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	//parseable_packet(0x086D,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x086F,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x087E,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x0883,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088B,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0890,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0898,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08A4,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0926,6,clif_parse_TakeItem,2);
	parseable_packet(0x0958,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x095A,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-11-22bRagexeRE
#elif PACKETVER == 20171122
	parseable_packet(0x0281,6,clif_parse_SolveCharName,2);
	parseable_packet(0x02C4,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	//parseable_packet(0x035F,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0838,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x083C,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x085B,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0862,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0867,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0877,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x0885,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0890,6,clif_parse_TickSend,2);
	parseable_packet(0x0891,6,clif_parse_TakeItem,2);
	parseable_packet(0x0893,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0897,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0898,6,clif_parse_DropItem,2,4);
	parseable_packet(0x089A,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x089E,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x08A6,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x08A9,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x091E,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0920,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0923,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0934,36,clif_parse_StoragePassword,0);
	parseable_packet(0x093B,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0945,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0946,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0947,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0962,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0968,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
// 2017-11-29aRagexeRE
#elif PACKETVER == 20171129
	parseable_packet(0x02C4,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x035F,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0363,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0365,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0862,6,clif_parse_TakeItem,2);
	parseable_packet(0x086D,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x0876,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0878,36,clif_parse_StoragePassword,0);
	parseable_packet(0x088A,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x089C,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x08A5,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0940,6,clif_parse_TickSend,2);
	//parseable_packet(0x094B,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0953,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0966,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-12-06aRagexeRE
#elif PACKETVER == 20171206
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0867,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x086A,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x086E,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0885,6,clif_parse_TickSend,2);
	parseable_packet(0x0888,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0897,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x089D,36,clif_parse_StoragePassword,0);
	parseable_packet(0x08A2,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x08A4,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x091D,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0923,8,clif_parse_MoveToKafra,2,4);
	//parseable_packet(0x092E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0936,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0942,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0958,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0961,6,clif_parse_TakeItem,2);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-12-13bRagexeRE
#elif PACKETVER == 20171213
	parseable_packet(0x0202,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0860,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0881,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0890,36,clif_parse_StoragePassword,0);
	parseable_packet(0x091A,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0957,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2017-12-20aRagexeRE
#elif PACKETVER == 20171220
	parseable_packet(0x0281,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0366,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0369,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0436,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0437,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x085E,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0861,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0872,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0873,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0880,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0882,6,clif_parse_TickSend,2);
	parseable_packet(0x0885,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x088C,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0899,36,clif_parse_StoragePassword,0);
	parseable_packet(0x089E,5,clif_parse_WalkToXY,2);
	parseable_packet(0x08A7,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x091B,6,clif_parse_TakeItem,2);
	parseable_packet(0x091E,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0924,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0929,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0933,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x093E,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0941,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0946,4,nullptr,0); // CZ_GANGSI_RANK
	//parseable_packet(0x094E,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0951,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0957,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0960,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0964,26,clif_parse_PartyInvite2,2);
// 2017-12-27aRagexeRE
#elif PACKETVER == 20171227
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0802,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x087D,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0888,8,clif_parse_MoveFromKafra,2,4);
	//parseable_packet(0x088A,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x088D,6,clif_parse_TakeItem,2);
	parseable_packet(0x08A0,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x08A5,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x092C,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x092E,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0938,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0945,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0946,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0969,6,clif_parse_DropItem,2,4);
	parseable_packet(0x096A,18,clif_parse_PartyBookingRegisterReq,2,4);
// 2018-01-03aRagexeRE or 2018-01-03bRagexeRE
#elif PACKETVER == 20180103
	parseable_packet(0x02C4,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0363,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,19,clif_parse_WantToConnection,2,6,10,14,18);
	//parseable_packet(0x0865,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x086B,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x086D,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x0872,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0876,6,clif_parse_TickSend,2);
	parseable_packet(0x0879,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x088E,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0899,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x089F,6,clif_parse_SolveCharName,2);
	parseable_packet(0x08A9,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x08AB,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x08AC,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x091D,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0922,2,clif_parse_SearchStoreInfoNextPage,0);
	//parseable_packet(0x0926,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0927,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x092C,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0935,6,clif_parse_TakeItem,2);
	parseable_packet(0x0938,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0941,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0946,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0948,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x094E,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x095D,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x095F,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0960,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
// 2018-01-17aRagexeRE
#elif PACKETVER == 20180117
	parseable_packet(0x0202,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x022D,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x023B,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0361,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0436,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0875,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2018-01-24bRagexeRE
#elif PACKETVER == 20180124
	parseable_packet(0x035F,6,clif_parse_TickSend,2);
	parseable_packet(0x0360,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0436,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0802,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x085F,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0868,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x086A,18,clif_parse_PartyBookingRegisterReq,2,4);
	//parseable_packet(0x086F,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x087A,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0888,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0890,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0919,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0940,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0946,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x094D,6,clif_parse_TakeItem,2);
	parseable_packet(0x0958,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0961,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2018-02-07bRagexeRE
#elif PACKETVER == 20180207
	parseable_packet(0x0281,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x035F,6,clif_parse_ReqClickBuyingStore,2);
	//parseable_packet(0x0360,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0362,6,clif_parse_DropItem,2,4);
	//parseable_packet(0x0363,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0364,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0365,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0366,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0368,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0369,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0437,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0438,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x07E4,6,clif_parse_TakeItem,2);
	parseable_packet(0x07EC,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0802,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0811,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0815,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0817,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0819,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0835,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x0838,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x083C,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x0870,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0881,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x092C,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x092E,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0940,6,clif_parse_TickSend,2);
	parseable_packet(0x0950,36,clif_parse_StoragePassword,0);
	parseable_packet(0x096A,6,clif_parse_GetCharNameRequest,2);
// 2018-02-13aRagexeRE
#elif PACKETVER == 20180213
	parseable_packet(0x0369,36,clif_parse_StoragePassword,0);
	parseable_packet(0x0802,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0817,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x085A,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x086F,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x0874,6,clif_parse_TickSend,2);
	parseable_packet(0x0875,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0878,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x087B,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0882,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x088C,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0892,6,clif_parse_TakeItem,2);
	parseable_packet(0x0898,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	//parseable_packet(0x089C,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x08A3,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x08A5,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x08A9,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x08AD,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0917,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0922,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0924,10,clif_parse_UseSkillToPos,2,4,6,8);
	//parseable_packet(0x0926,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0933,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0936,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x093C,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0943,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0955,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x095A,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0962,5,clif_parse_HomMenu,2,4);
// 2018-02-21aRagexeRE or 2018-02-21bRagexeRE
#elif PACKETVER == 20180221
	parseable_packet(0x0202,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	//parseable_packet(0x0366,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0436,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x0838,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0867,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x086C,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x086F,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0871,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0876,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0879,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x087D,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0880,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0881,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0883,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x088F,6,clif_parse_SolveCharName,2);
	//parseable_packet(0x0891,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x0897,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0899,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x089D,6,clif_parse_TickSend,2);
	parseable_packet(0x0917,36,clif_parse_StoragePassword,0);
	parseable_packet(0x091E,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0929,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x093D,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x094B,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x094D,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x094E,6,clif_parse_TakeItem,2);
	parseable_packet(0x0957,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0964,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x096A,7,clif_parse_ActionRequest,2,6);
// 2018-03-07bRagexeRE
#elif PACKETVER == 20180307
	parseable_packet(0x035F,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0437,6,clif_parse_DropItem,2,4);
	parseable_packet(0x07E4,19,clif_parse_WantToConnection,2,6,10,14,18);
	parseable_packet(0x0861,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
	parseable_packet(0x0862,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0864,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x086C,6,clif_parse_TickSend,2);
	parseable_packet(0x0870,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	parseable_packet(0x0872,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0877,5,clif_parse_WalkToXY,2);
	parseable_packet(0x088D,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0893,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x089B,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
	parseable_packet(0x08A6,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x08AA,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x08AB,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0917,10,clif_parse_UseSkillToPos,2,4,6,8);
	parseable_packet(0x0920,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0937,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0939,36,clif_parse_StoragePassword,0);
	parseable_packet(0x093D,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0941,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0944,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x0948,26,clif_parse_PartyInvite2,2);
	//parseable_packet(0x0951,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0954,6,clif_parse_TakeItem,2);
	parseable_packet(0x0957,6,clif_parse_SolveCharName,2);
	parseable_packet(0x0969,7,clif_parse_ActionRequest,2,6);
	//parseable_packet(0x0281,4,nullptr,0); // CZ_GANGSI_RANK
// Clients after 2018-03-07bRagexeRE do not have shuffled packets anymore
#elif PACKETVER > 20180307
	parseable_packet(0x0202,26,clif_parse_FriendsListAdd,2);
	parseable_packet(0x022D,5,clif_parse_HomMenu,2,4);
	parseable_packet(0x023B,36,clif_parse_StoragePassword,0);
	//parseable_packet(0x0281,4,nullptr,0); // CZ_GANGSI_RANK
	parseable_packet(0x02C4,26,clif_parse_PartyInvite2,2);
	parseable_packet(0x035F,5,clif_parse_WalkToXY,2);
	parseable_packet(0x0360,6,clif_parse_TickSend,2);
	parseable_packet(0x0361,5,clif_parse_ChangeDir,2,4);
	parseable_packet(0x0362,6,clif_parse_TakeItem,2);
	parseable_packet(0x0363,6,clif_parse_DropItem,2,4);
	parseable_packet(0x0364,8,clif_parse_MoveToKafra,2,4);
	parseable_packet(0x0365,8,clif_parse_MoveFromKafra,2,4);
	parseable_packet(0x0366,10,clif_parse_UseSkillToPos,2,4,6,8);
#if PACKETVER_MAIN_NUM >= 20190904 || PACKETVER_RE_NUM >= 20190904 || PACKETVER_ZERO_NUM >= 20190828
	parseable_packet(0x0367,31,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
#else
	parseable_packet(0x0367,90,clif_parse_UseSkillToPosMoreInfo,2,4,6,8,10);
#endif
	parseable_packet(0x0368,6,clif_parse_GetCharNameRequest,2);
	parseable_packet(0x0369,6,clif_parse_SolveCharName,2);
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
	parseable_packet( 0x0436, 23, clif_parse_WantToConnection, 2, 6, 10, 14, 22 );
#else
	parseable_packet( 0x0436, 19, clif_parse_WantToConnection, 2, 6, 10, 14, 18 );
#endif
	parseable_packet(0x0437,7,clif_parse_ActionRequest,2,6);
	parseable_packet(0x0438,10,clif_parse_UseSkillToId,2,4,6);
	parseable_packet(0x07E4,-1,clif_parse_ItemListWindowSelected,2,4,8,12);
	//parseable_packet(0x07EC,8,nullptr,0); // CZ_JOIN_BATTLE_FIELD
	parseable_packet(0x0802,18,clif_parse_PartyBookingRegisterReq,2,4);
	parseable_packet(0x0811,-1,clif_parse_ReqOpenBuyingStore,2,4,8,9,89);
	parseable_packet(0x0815,2,clif_parse_ReqCloseBuyingStore,0);
	parseable_packet(0x0817,6,clif_parse_ReqClickBuyingStore,2);
	parseable_packet(0x0819,-1,clif_parse_ReqTradeBuyingStore,2,4,8,12);
	parseable_packet(0x0835,-1,clif_parse_SearchStoreInfo,2,4,5,9,13,14,15);
	parseable_packet(0x0838,2,clif_parse_SearchStoreInfoNextPage,0);
	parseable_packet(0x083C,sizeof(struct PACKET_CZ_SSILIST_ITEM_CLICK),clif_parse_SearchStoreInfoListItemClick,0);
#endif

#endif /* CLIF_SHUFFLE_HPP */

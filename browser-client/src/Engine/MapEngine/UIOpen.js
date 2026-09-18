/**
 * Engine/MapEngine/UIOpen.js
 *
 * Manage some UI open when requested by server
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import Configs from 'Core/Configs.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import CheckAttendance from 'UI/Components/CheckAttendance/CheckAttendance.js';
import EnchantGradeUI from 'UI/Components/EnchantGrade/EnchantGrade.js';
import EnchantUI from 'UI/Components/Enchant/Enchant.js';

/**
 * Received data and request to open a specific UI
 *
 * @param {object} pkt - PACKET.ZC.UI_OPEN
 */
function onUIOpen(pkt) {
	// Opens an UI window of the given type and initializes it with the given data
	// 0AE2 <type>.B <data>.L
	// type:
	//    0 = BANK_UI
	//    1 = STYLIST_UI
	//    2 = CAPTCHA_UI
	//    3 = MACRO_UI
	//    4 = UI_UNUSED
	//    5 = TIPBOX_UI
	//    6 = RENEWQUEST_UI
	//    7 = ATTENDANCE_UI
	//    8 = ENCHANTGRADE_UI
	//    9 = CHANGE_MATERIAL_UI
	//    10 = ENCHANT_UI

	switch (pkt.ui_type) {
		case 7:
			if (Configs.get('enableCheckAttendance') && PACKETVER.value >= 20180307) {
				CheckAttendance.prepare();
				CheckAttendance.setData(pkt.data);
				CheckAttendance.cleanUI();
				CheckAttendance.append();
				CheckAttendance.ui.show();
				CheckAttendance.focus();
			}
			break;
		case 8:
			if (PACKETVER.value >= 20200724) {
				EnchantGradeUI.prepare();
				EnchantGradeUI.onOpenEnchantGradeUI();
			}
			break;
		case 10:
			if (PACKETVER.value >= 20211103) {
				EnchantUI.prepare();
				EnchantUI.onOpenEnchantUI(pkt.data);
			}
			break;
		default:
			console.error(`[PACKET.ZC.UI_OPEN] not implemented (${pkt.ui_type})`);
	}
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.UI_OPEN, onUIOpen);
	Network.hookPacket(PACKET.ZC.UI_OPEN_V3, onUIOpen);
}

/**
 * UI/Components/CashShopIcon/CashShopIcon.js
 *
 * CashShop Icon
 *
 * @author Alisonrag
 *
 */

import CashShop from 'UI/Components/CashShop/CashShop.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './CashShopIcon.html?raw';
import cssText from './CashShopIcon.css?raw';

/**
 * Create Component
 */
const CashShopIcon = new GUIComponent('CashShopIcon', cssText);

CashShopIcon.render = () => htmlText;

/**
 * One-time setup — bind events here (runs once during prepare)
 */
CashShopIcon.init = function init() {
	const root = this.getRoot();
	const btn = root.querySelector('.cashshop-icon');
	if (btn) {
		btn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		btn.addEventListener('click', onClickCashShopIcon);
	}
};

/**
 * Handle click on CashShop icon
 */
function onClickCashShopIcon() {
	if (CashShop.ui.is(':visible')) {
		const pkt = new PACKET.CZ.CASH_SHOP_CLOSE();
		Network.sendPacket(pkt);
		CashShop.remove();
	} else {
		if (PACKETVER.value >= 20191224) {
			const pkt = new PACKET.CZ.SE_CASHSHOP_OPEN2();
			pkt.tab = 0;
			Network.sendPacket(pkt);
		} else {
			const pkt = new PACKET.CZ.SE_CASHSHOP_OPEN1();
			Network.sendPacket(pkt);
		}
	}
}

CashShopIcon.needFocus = false;
CashShopIcon.mouseMode = GUIComponent.MouseMode.CROSS;

/**
 * Create component and export it
 */
export default UIManager.addComponent(CashShopIcon);

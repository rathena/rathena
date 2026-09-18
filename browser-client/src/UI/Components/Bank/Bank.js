/**
 * UI/Components/Bank/Bank.js
 *
 * Bank window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './Bank.html?raw';
import cssText from './Bank.css?raw';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import NpcBox from 'UI/Components/NpcBox/NpcBox.js';
import NpcMenu from 'UI/Components/NpcMenu/NpcMenu.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
/**
 * Create Component
 */
const Bank = new GUIComponent('Bank', cssText);

/**
 * Render HTML
 */
Bank.render = () => htmlText;

/**
 * Max Int
 */
const maxInt = 2147483647;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'Bank',
	{
		x: 230,
		y: 295
	},
	2.0
);

/**
 * Initialize UI
 */
Bank.init = function init() {
	const root = this.getRoot();
	let isMax = false;
	const inputDepo = root.querySelector('.depo');

	this.draggable();

	root.querySelector('.plus').addEventListener('click', () => {
		if (isMax || inputDepo.value === '') {
			inputDepo.value = 1;
		} else {
			inputDepo.value = parseInt(inputDepo.value) + 1;
		}
		isMax = false;
		inputDepo.select();
	});

	root.querySelector('.minus').addEventListener('click', () => {
		if (isMax || inputDepo.value === '') {
			inputDepo.value = 0;
		} else {
			inputDepo.value = Math.max(parseInt(inputDepo.value) - 1, 0);
		}
		isMax = false;
		inputDepo.select();
	});

	root.querySelector('.max').addEventListener('click', () => {
		isMax = true;
		inputDepo.value = 'MAX';
		inputDepo.select();
	});

	root.querySelector('.tenmil').addEventListener('click', () => {
		isMax = false;
		inputDepo.value = addValueToInput(inputDepo.value, 10000000);
		inputDepo.select();
	});

	root.querySelector('.onemil').addEventListener('click', () => {
		isMax = false;
		inputDepo.value = addValueToInput(inputDepo.value, 1000000);
		inputDepo.select();
	});

	root.querySelector('.hundtsn').addEventListener('click', () => {
		isMax = false;
		inputDepo.value = addValueToInput(inputDepo.value, 100000);
		inputDepo.select();
	});

	function addValueToInput(inputValue, addValue) {
		if (isMax && inputValue === 'MAX') {
			return 'MAX';
		}
		const currentValue = parseInt(inputValue) || 0;
		if (!isMax) {
			return currentValue + addValue;
		}
	}

	root.querySelector('.deposit').addEventListener('click', () => {
		sendDepositRequest(inputDepo.value);
	});

	root.querySelector('.withdraw').addEventListener('click', () => {
		sendWithdrawRequest(inputDepo.value);
	});

	root.querySelector('.close').addEventListener('click', reqCloseBank);

	inputDepo.addEventListener('click', () => {
		inputDepo.select();
	});
};

/**
 * Check if input value is valid
 */
function CheckValue(value) {
	const root = Bank.getRoot();
	const error = root.querySelector('.errorupdate');

	if (value === '') {
		if (error) {
			error.textContent = DB.getMessage(2781);
		}
		ChatBox.addText(DB.getMessage(2779), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return false;
	}

	if (typeof value === 'string' && value !== 'MAX' && !/^\d+$/.test(value)) {
		if (error) {
			error.textContent = DB.getMessage(2782);
		}
		ChatBox.addText(DB.getMessage(2488), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return false;
	}

	if (parseInt(value) <= 0) {
		if (error) {
			error.textContent = DB.getMessage(2784);
		}
		ChatBox.addText(DB.getMessage(2769), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return false;
	}

	if (parseInt(value) > maxInt) {
		if (error) {
			error.textContent = DB.getMessage(2783);
		}
		ChatBox.addText(DB.getMessage(2768), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return false;
	}

	return true;
}

/**
 * Send Request to server to deposit
 */
function sendDepositRequest(value) {
	const root = Bank.getRoot();
	const input = root.querySelector('.depo');

	if (CheckValue(value) === false) {
		input.value = '';
		return;
	}

	if (value === 'MAX') {
		const inbank = root.querySelector('.inbank.currency');
		const getval = parseInt(getIntValueFromFormattedString(inbank.textContent));
		if (Session.zeny + getval > maxInt && getval < maxInt) {
			value = parseInt(maxInt) - getval;
		} else {
			if (Session.zeny === 0) {
				const error = root.querySelector('.errorupdate');
				if (error) {
					error.textContent = DB.getMessage(2785);
				}
				ChatBox.addText(DB.getMessage(2770), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
				input.value = '';
				return;
			} else {
				value = Session.zeny;
			}
		}
	}

	const pkt = new PACKET.CZ.REQ_BANKING_DEPOSIT();
	pkt.AID = Session.AID;
	pkt.money = value;
	Network.sendPacket(pkt);

	input.value = '';
}

/**
 * Send Request to server to withdraw
 */
function sendWithdrawRequest(value) {
	const root = Bank.getRoot();
	const input = root.querySelector('.depo');
	const error = root.querySelector('.errorupdate');

	if (CheckValue(value) === false) {
		input.value = '';
		return;
	}

	if (value === 'MAX') {
		const inbank = root.querySelector('.inbank.currency');
		const getval = parseInt(getIntValueFromFormattedString(inbank.textContent));
		if (Session.zeny + getval > maxInt && Session.zeny < maxInt) {
			value = parseInt(maxInt) - Session.zeny;
		} else if (getval === 0) {
			if (error) {
				error.textContent = DB.getMessage(2785);
			}
			ChatBox.addText(DB.getMessage(2770), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			input.value = '';
			return;
		} else {
			value = getval;
		}
	}

	if (parseInt(Session.zeny) + parseInt(value) > parseInt(maxInt)) {
		if (error) {
			error.textContent = DB.getMessage(2787);
		}
		ChatBox.addText(DB.getMessage(2459), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		input.value = '';
		return;
	}

	const pkt = new PACKET.CZ.REQ_BANKING_WITHDRAW();
	pkt.AID = Session.AID;
	pkt.money = value;
	Network.sendPacket(pkt);

	input.value = '';
}

/**
 * Get int value from the input box
 */
function getIntValueFromFormattedString(formattedString) {
	const strippedString = formattedString.replace(/,/g, '').replace('z', '');
	const intValue = parseInt(strippedString, 10);
	if (!isNaN(intValue)) {
		return intValue;
	} else {
		return null;
	}
}

/**
 * Append to body
 */
Bank.onAppend = function onAppend() {
	const root = this.getRoot();

	this._host.style.top = Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight) + 'px';
	this._host.style.left = Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth) + 'px';

	const input = root.querySelector('.depo');
	input.value = '';
	input.focus();
};

/**
 * Key Handler
 */
Bank.onKeyDown = function onKeyDown(event) {
	if (InputBox._host && InputBox._host.style.display !== 'none' && InputBox.__active) {
		return true;
	}

	if (NpcMenu._host && NpcMenu._host.style.display !== 'none' && NpcMenu.__active) {
		return true;
	}

	if (NpcBox._host && NpcBox._host.style.display !== 'none' && NpcBox.__active) {
		return true;
	}

	if (this.isEditableFocused()) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			reqCloseBank();
			event.stopImmediatePropagation();
			return false;
		}
		if (event.which === KEYS.ENTER) {
			event.stopImmediatePropagation();
			return false;
		}
		event.stopImmediatePropagation();
		return true;
	}

	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		reqCloseBank();
		event.stopImmediatePropagation();
		return false;
	}

	return true;
};

/**
 * Process shortcut
 */
Bank.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			this.toggle();
			break;
	}
};

/**
 * Request to toggle open/close bank
 */
Bank.toggle = function toggle() {
	if (!Bank.__active) {
		reqOpenBank();
	} else {
		reqCloseBank();
	}
};

/**
 * Request to open bank
 */
function reqOpenBank() {
	const pkt = new PACKET.CZ.REQ_BANK_OPEN();
	pkt.AID = Session.AID;
	Network.sendPacket(pkt);
}

/**
 * Request to close bank
 */
function reqCloseBank() {
	const pkt = new PACKET.CZ.REQ_BANK_CLOSE();
	pkt.AID = Session.AID;
	Network.sendPacket(pkt);
}

/**
 * Remove Bank from window
 */
Bank.onRemove = function onRemove() {
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	const root = this.getRoot();
	const error = root.querySelector('.errorupdate');
	if (error) {
		error.textContent = '';
	}
};

/**
 * Public methods for Engine/MapEngine/Bank.js
 */
Bank.updateBankDisplay = function updateBankDisplay(bankMoney, handMoney) {
	const root = this.getRoot();
	const inbank = root.querySelector('.inbank.currency');
	const onhand = root.querySelector('.onhand.currency');
	if (inbank) {
		inbank.textContent = bankMoney.toLocaleString() + 'z';
	}
	if (onhand) {
		onhand.textContent = handMoney.toLocaleString() + 'z';
	}
};

Bank.setError = function setError(message) {
	const root = this.getRoot();
	const error = root.querySelector('.errorupdate');
	if (error) {
		error.textContent = message;
	}
};

Bank.clearError = function clearError() {
	const root = this.getRoot();
	const error = root.querySelector('.errorupdate');
	if (error) {
		error.textContent = '';
	}
};

Bank.clearInput = function clearInput() {
	const root = this.getRoot();
	const input = root.querySelector('.depo');
	if (input) {
		input.value = '';
	}
};

Bank.focusInput = function focusInput() {
	const root = this.getRoot();
	const input = root.querySelector('.depo');
	if (input) {
		input.focus();
	}
};

Bank.getBankAmount = function getBankAmount() {
	const root = this.getRoot();
	const inbank = root.querySelector('.inbank.currency');
	if (inbank) {
		return inbank.textContent;
	}
	return '0z';
};

Bank.mouseMode = GUIComponent.MouseMode.STOP;
Bank.captureKeyEvents = true;
Bank.needFocus = true;

/**
 * Create component and export it
 */
export default UIManager.addComponent(Bank);

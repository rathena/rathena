/**
 * UI/Components/EnchantGrade/EnchantGrade.js
 *
 * EnchantGrade Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import EffectConst from 'DB/Effects/EffectConst.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import EffectManager from 'Renderer/EffectManager.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import Announce from 'UI/Components/Announce/Announce.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import htmlText from './EnchantGrade.html?raw';
import cssText from './EnchantGrade.css?raw';

/**
 * Create Component
 */
const EnchantGrade = new GUIComponent('EnchantGrade', cssText);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return EnchantGrade._shadow || EnchantGrade._host;
}

/**
 * Render HTML
 */
EnchantGrade.render = () => htmlText;

/**
 * Variables for current EnchantGrade Session
 */
let gradingMaterials = [];
let EnchantGrade_item_index = 0;
let EnchantGrade_item_mat = 0;
let EnchantGrade_result = 0;
let EnchantGrade_can_cont = true;
let EnchantGrade_blessing_used = false;
let EnchantGrade_currentBlessing = 0;
let EnchantGrade_current_success = 0;
let materialNormal, materialOver, materialPick, startNormal, startDisable;
let _materialSlotAbort = null;
EnchantGrade.imageLoopTimeout = {};

/**
 * Mapping for Grade Level
 * @var {Grade} : @var {Grade String}
 */
const GradeMapping = {
	1: 'D',
	2: 'C',
	3: 'B',
	4: 'A'
};

/**
 * Mapping for notification messages based on scenarios
 * @var {scenario} : @var {array of message objects}
 * message object: { id: messageID, color: hexColor }
 */
const scenarioMsgMapping = {
	initial: [
		{ id: 3870, color: '#de0808' },
		{ id: 3871, color: '#00ef5a' },
		{ id: 3872, color: '#00ef5a' }
	],
	waiting: [{ id: 3707, color: '#de0808' }],
	cannot_proceed: [{ id: 3821, color: '#de0808' }],
	insufficient: [{ id: 3708, color: '#de0808' }],
	destroyable: [{ id: 3712, color: '#de0808' }],
	non_destroyable: [{ id: 3822, color: '#00ef5a' }]
};

/**
 * Variables for images to play in between phases
 */
const images = {
	idle: ['bg_gradeitem1.bmp', 'bg_gradeitem2.bmp', 'bg_gradeitem3.bmp', 'bg_gradeitem4.bmp', 'bg_gradeitem5.bmp'],
	process: ['gradewheel1.bmp', 'gradewheel2.bmp', 'gradewheel3.bmp']
};

/**
 * Initialize UI
 */
EnchantGrade.init = function init() {
	const root = _root();

	this._host.style.top = '200px';
	this._host.style.left = '300px';

	const closeBtn = root.querySelector('.close_btn');
	if (closeBtn) {
		closeBtn.addEventListener('click', onEnchantGradeClose);
	}

	const bigCloseBtn = root.querySelector('.footer .big_close_btn');
	if (bigCloseBtn) {
		bigCloseBtn.addEventListener('click', onEnchantGradeClose);
	}

	this.draggable('.titlebar');

	const dropProxy = root.querySelector('.enchant_drop_proxy');
	if (dropProxy) {
		dropProxy.addEventListener('drop', onItemDrop);
		dropProxy.addEventListener('dragover', stopPropagation);
	}

	const enchantContainer = root.querySelector('.enchant_container');
	if (enchantContainer) {
		enchantContainer.addEventListener('dragstart', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemDragStart(e);
			}
		});
		enchantContainer.addEventListener('dragend', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemDragEnd(e);
			}
		});
		enchantContainer.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemInfo.call(item, e);
			}
		});
		enchantContainer.addEventListener('dblclick', e => {
			const item = e.target.closest('.item');
			if (item) {
				onRemoveItem();
			}
		});
	}

	const materialSlots = root.querySelectorAll('.material_slot');
	materialSlots.forEach(slot => {
		slot.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemInfo.call(item, e);
			}
		});
	});

	const bedContainer = root.querySelector('.BED_container');
	if (bedContainer) {
		bedContainer.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemInfo.call(item, e);
			}
		});
	}

	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_material.bmp', d => {
		materialNormal = d;
		root.querySelectorAll('.material_slot').forEach(slot => {
			slot.style.backgroundImage = `url(${d})`;
		});
	});
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_start_disable.bmp', d => {
		startDisable = d;
		const startBtn = root.querySelector('.start_grade');
		if (startBtn) {
			startBtn.style.backgroundImage = `url(${d})`;
		}
	});
};

/**
 * Append to body
 */
EnchantGrade.onAppend = function onAppend() {
	const root = _root();
	EnchantGrade.hammer = 0;

	const enchantGradeButton = root.querySelector('.EnchantGrade_button');
	if (enchantGradeButton) {
		enchantGradeButton.style.display = 'block';
	}

	setMessages('initial');
	clearTimeout(EnchantGrade.imageLoopTimeout);
};

/**
 * Remove EnchantGrade Window (and so clean up items)
 */
EnchantGrade.onRemove = function onRemove() {
	onRemoveItem();
	clearEnchantGradeStates();
};

/**
 * Completely clean-up variables for grading
 */
function clearEnchantGradeStates() {
	gradingMaterials = [];
	EnchantGrade_item_index = 0;
	EnchantGrade_item_mat = 0;
	EnchantGrade_result = 0;
	EnchantGrade_can_cont = true;
	EnchantGrade_blessing_used = false;
	EnchantGrade_currentBlessing = 0;
	EnchantGrade_current_success = 0;
	EnchantGrade.imageLoopTimeout = {};
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Open EnchantGrade UI
 */
EnchantGrade.onOpenEnchantGradeUI = function onOpenEnchantGradeUI() {
	EnchantGrade.append();

	const invUI = Inventory.getUI();
	const isInventoryOpen =
		invUI && invUI._host ? invUI._host.isConnected && invUI._host.style.display !== 'none' : false;

	if (!isInventoryOpen) {
		invUI.toggle();
	}

	const hostEl = EnchantGrade._host;
	const hostRect = hostEl.getBoundingClientRect();
	const invHost = invUI._host || (invUI.ui && invUI.ui[0]);
	const invHeight = invHost ? invHost.offsetHeight : 0;

	if (invHost) {
		invHost.style.position = 'absolute';
		invHost.style.top = `${hostRect.top ? hostRect.top + (hostRect.height - invHeight) : 200}px`;
		invHost.style.left = `${hostRect.left ? hostRect.left + hostRect.width : 300}px`;
	}

	return false;
};

/**
 * Handles sending request to server to close EnchantGrade UI
 */
function onEnchantGradeClose() {
	EnchantGrade.remove();

	const pkt = new PACKET.CZ.GRADE_ENCHANT_CLOSE_UI();
	Network.sendPacket(pkt);
}

/**
 * Function to control phases and image looping
 * @param {string} phase - The phase to control
 * @param {boolean} shouldLoop - Whether the phase should loop
 * @param {number} interval - Interval between image changes in milliseconds
 * @param {string} targetdiv - The target div to update images
 * @param {function} callback - A callback function to execute after the phase completes
 */
function controlPhase(phase, shouldLoop, interval, targetdiv, callback) {
	if (EnchantGrade.imageLoopTimeout[phase]) {
		clearTimeout(EnchantGrade.imageLoopTimeout[phase]);
		EnchantGrade.imageLoopTimeout[phase] = null;
	}

	let currentImageIndex = 0;
	const imageArray = images[phase];

	if (!imageArray) {
		console.error('Invalid phase:', phase);
		return;
	}

	function showImages() {
		Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/' + imageArray[currentImageIndex], function (data) {
			const root = _root();
			const target = root.querySelector(targetdiv);
			if (target) {
				target.style.backgroundImage = `url(${data})`;
				target.style.visibility = 'visible';
			}
			currentImageIndex++;

			if (currentImageIndex >= imageArray.length) {
				if (shouldLoop) {
					currentImageIndex = 0;
				} else {
					EnchantGrade.imageLoopTimeout[phase] = null;
					if (callback && typeof callback === 'function') {
						callback();
					}
					return;
				}
			}

			EnchantGrade.imageLoopTimeout[phase] = setTimeout(showImages, interval);
		});
	}

	showImages();
}

/**
 * Stops the image loop for a given phase
 * @param {string} phase - The phase to stop the image loop for
 */
function stopPhase(phase) {
	if (EnchantGrade.imageLoopTimeout[phase]) {
		clearTimeout(EnchantGrade.imageLoopTimeout[phase]);
		EnchantGrade.imageLoopTimeout[phase] = null;
	}
}

/**
 * Drop an item from inventory to EnchantGrade UI
 *
 * @param {event}
 */
function onItemDrop(event) {
	let item, data;
	event.stopImmediatePropagation();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	if (data.type !== 'item' || data.from !== 'Inventory') {
		return;
	}

	if (item) {
		EnchantGrade.onRequestItemEnchantGrade(item);
	}
}

/**
 * Handles sending the server packet request to EnchantGrade an item
 */
EnchantGrade.onRequestItemEnchantGrade = function onRequestItemEnchantGrade(item) {
	const pkt = new PACKET.CZ.GRADE_ENCHANT_SELECT_EQUIPMENT();
	pkt.index = item.index;
	Network.sendPacket(pkt);
};

/**
 * Enables the item drop proxy to add dropped items in the EnchantGrade window
 */
function enableDropProxy() {
	const root = _root();
	const proxy = root.querySelector('.enchant_drop_proxy');
	if (proxy) {
		proxy.style.display = 'block';
	}
	const container = root.querySelector('.enchant_container');
	if (container) {
		container.style.visibility = 'hidden';
	}
}

/**
 * Disable the item drop proxy to add dropped items in the EnchantGrade window
 */
function disableDropProxy() {
	const root = _root();
	const proxy = root.querySelector('.enchant_drop_proxy');
	if (proxy) {
		proxy.style.display = 'none';
	}
	const container = root.querySelector('.enchant_container');
	if (container) {
		container.style.visibility = 'visible';
	}
}

/**
 * Handles the packet received from the server
 * @param {pkt} - PACKET.ZC.REFINING_MATERIAL_LIST
 */
function onEnchantGradeUIUpdateMaterials(pkt) {
	const root = _root();

	if (pkt && pkt.materialList.length > 0) {
		disableDropProxy();

		const existingItem = root.querySelector('.enchant_container .item');
		if (existingItem) {
			onRemoveItem();
		}

		EnchantGrade_item_index = pkt.index;
		EnchantGrade_current_success = pkt.success_chance;
		const item = Inventory.getUI().getItemByIndex(pkt.index);

		gradingMaterials = pkt.materialList;

		onPopulateMaterials();

		clearTimeout(EnchantGrade.imageLoopTimeout['idle']);
		controlPhase('idle', true, 225, '.grade_tank_container');

		const it = DB.getItemInfo(item.ITID);
		const content = root.querySelector('.enchant_container');

		EnchantGrade_can_cont = EnchantGrade_current_success > 0;
		setMessages(EnchantGrade_can_cont ? 'waiting' : 'cannot_proceed');

		const itemDiv = document.createElement('div');
		itemDiv.className = 'item';
		itemDiv.setAttribute('data-index', item.index);
		itemDiv.setAttribute('draggable', 'true');
		itemDiv.innerHTML = '<div class="icon"></div><div class="grade"></div>';
		content.appendChild(itemDiv);

		Client.loadFile(
			DB.INTERFACE_PATH +
				'item/' +
				(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
				'.bmp',
			function (data) {
				const icon = content.querySelector(`.item[data-index="${item.index}"] .icon`);
				if (icon) {
					icon.style.backgroundImage = `url(${data})`;
				}
			}
		);

		if (item.enchantgrade) {
			Client.loadFile(
				DB.INTERFACE_PATH + 'grade_enchant/grade_icon' + item.enchantgrade + '.bmp',
				function (data) {
					const grade = content.querySelector(`.item[data-index="${item.index}"] .grade`);
					if (grade) {
						grade.style.backgroundImage = `url(${data})`;
					}
				}
			);
		}

		const gradeChance = root.querySelector('.probability');
		if (gradeChance) {
			gradeChance.textContent = `${EnchantGrade_current_success}%`;
		}

		const zenyCost = root.querySelector('.zeny_cost_container');
		if (zenyCost) {
			zenyCost.textContent = '0';
		}

		const bless = pkt.blessing_info;
		if (!bless || !bless.id) {
			return;
		}

		const blessItem = DB.getItemInfo(bless.id);
		const invBless = Inventory.getUI().getItemById(bless.id);
		const invCount = invBless ? invBless.count : 0;

		const bedContainer = root.querySelector('.BED_container');
		if (bedContainer) {
			bedContainer.innerHTML =
				`<div class="item" data-index="${bless.id}">` +
				'<div class="icon"></div>' +
				'</div>' +
				`<div class="additional_mat_name" style="margin-right: 3px; margin-left: 7px;">${blessItem.identifiedDisplayName}</div>` +
				`<div class="additonal_mat_amount">${EnchantGrade_currentBlessing} ea</div>`;
		}

		Client.loadFile(
			DB.INTERFACE_PATH +
				'item/' +
				(blessItem.IsIdentified ? blessItem.identifiedResourceName : blessItem.unidentifiedResourceName) +
				'.bmp',
			data => {
				const icon = root.querySelector('.BED_container .icon');
				if (icon) {
					icon.style.backgroundImage = `url(${data})`;
				}
			}
		);

		const addBtn = root.querySelector('.add_material_btn');
		if (addBtn) {
			const newAddBtn = addBtn.cloneNode(true);
			addBtn.parentNode.replaceChild(newAddBtn, addBtn);

			newAddBtn.addEventListener('click', () => {
				if (EnchantGrade_currentBlessing >= bless.max_blessing) {
					return;
				}
				if (invCount < (EnchantGrade_currentBlessing + 1) * bless.amount) {
					return;
				}

				EnchantGrade_blessing_used = true;
				EnchantGrade_currentBlessing++;

				updateBlessing();
			});

			newAddBtn.addEventListener('mouseenter', e => {
				showTooltip(DB.getMessage(3976).replace('%d', bless.amount), e, newAddBtn);
			});

			newAddBtn.addEventListener('mousemove', e => {
				showTooltip(DB.getMessage(3976).replace('%d', bless.amount), e, newAddBtn);
			});

			newAddBtn.addEventListener('mouseleave', hideTooltip);
		}

		const removeBtn = root.querySelector('.remove_material_btn');
		if (removeBtn) {
			const newRemoveBtn = removeBtn.cloneNode(true);
			removeBtn.parentNode.replaceChild(newRemoveBtn, removeBtn);

			newRemoveBtn.addEventListener('click', () => {
				if (EnchantGrade_currentBlessing <= 0) {
					EnchantGrade_blessing_used = false;
					return;
				}

				EnchantGrade_currentBlessing--;

				updateBlessing();
			});
		}

		function updateBlessing() {
			const addedChance = EnchantGrade_currentBlessing * bless.bonus;
			const totalBlessing = EnchantGrade_currentBlessing * bless.amount;
			const prob = root.querySelector('.probability');
			if (prob) {
				prob.textContent = `${EnchantGrade_current_success + addedChance}%`;
			}

			const matAmount = root.querySelector('.BED_container .additonal_mat_amount');
			if (matAmount) {
				matAmount.textContent = `${totalBlessing} ea`;
			}
		}

		function showTooltip(text, e, targetEl) {
			const tooltip = root.querySelector('.enchant_tooltip');
			const rootEl = root.querySelector('#EnchantGrade');

			if (!tooltip || !targetEl || !rootEl) {
				return;
			}

			tooltip.innerHTML = text;
			tooltip.style.display = 'block';

			const btnRect = targetEl.getBoundingClientRect();
			const rootRect = rootEl.getBoundingClientRect();

			const x = e.clientX - rootRect.left;
			const y = btnRect.top - rootRect.top;

			tooltip.style.left = `${x - tooltip.offsetWidth / 2}px`;
			tooltip.style.top = `${y - tooltip.offsetHeight - 6}px`;
		}

		function hideTooltip() {
			const tooltip = root.querySelector('.enchant_tooltip');
			if (tooltip) {
				tooltip.style.display = 'none';
			}
		}
	} else {
		return false;
	}
}

/**
 * Handles populating materials needed for grading
 * Controls for selecting and de-selecting materials
 */
function onPopulateMaterials() {
	const root = _root();

	root.querySelectorAll('.material_slot').forEach(slot => {
		slot.innerHTML = '';
	});

	for (let i = 0; i < gradingMaterials.length; i++) {
		(index => {
			const material = gradingMaterials[index];
			const it = DB.getItemInfo(material.itemId);
			const materialDiv = root.querySelector(`.material_slot${i + 1}`);

			if (!materialDiv) {
				return;
			}

			materialDiv.innerHTML = '';

			materialDiv.dataset.index = index;
			materialDiv.dataset.itemid = material.itemId;
			materialDiv.dataset.amount = material.amount;
			materialDiv.dataset.price = material.price;
			materialDiv.dataset.downgrade = material.downgrade;
			materialDiv.dataset.breakable = material.breakable;

			materialDiv.innerHTML =
				`<div class="item" data-index="${material.itemId}" draggable="false">` +
				'<div class="icon"></div></div>' +
				`<div class="material_name">${it.identifiedDisplayName}</div>` +
				`<div class="material_amount">${material.amount} ea</div>`;

			Client.loadFile(
				DB.INTERFACE_PATH +
					'item/' +
					(it.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
					'.bmp',
				function (data) {
					const icon = materialDiv.querySelector(`.item[data-index="${material.itemId}"] .icon`);
					if (icon) {
						icon.style.backgroundImage = `url(${data})`;
					}
				}
			);
		})(i);
	}

	let selectedMaterialBtn = null;

	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_material.bmp', d => (materialNormal = d));
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_material_over.bmp', d => (materialOver = d));
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_material_pick.bmp', d => (materialPick = d));
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_start.bmp', d => (startNormal = d));
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/btn_start_disable.bmp', d => (startDisable = d));

	if (_materialSlotAbort) {
		_materialSlotAbort.abort();
	}
	_materialSlotAbort = new AbortController();
	const _slotSignal = _materialSlotAbort.signal;

	root.querySelectorAll('.material_slot').forEach(slot => {
		slot.addEventListener(
			'mouseenter',
			function () {
				if (this === selectedMaterialBtn) {
					return;
				}
				this.style.backgroundImage = `url(${materialOver})`;
			},
			{ signal: _slotSignal }
		);
		slot.addEventListener(
			'mouseleave',
			function () {
				if (this === selectedMaterialBtn) {
					return;
				}
				this.style.backgroundImage = `url(${materialNormal})`;
			},
			{ signal: _slotSignal }
		);
		slot.addEventListener(
			'click',
			function () {
				if (!this.dataset.index) {
					return;
				}

				if (EnchantGrade_blessing_used) {
					UIManager.showMessageBox(DB.getMessage(3831), 'ok');
					onResetBlessing();
				}

				if (selectedMaterialBtn && selectedMaterialBtn !== this) {
					selectedMaterialBtn.style.backgroundImage = `url(${materialNormal})`;
				}

				selectedMaterialBtn = this;
				this.style.backgroundImage = `url(${materialPick})`;

				EnchantGrade_item_mat = Number(this.dataset.index);
				const material_ITID = Number(this.dataset.itemid);
				const material_amount = Number(this.dataset.amount);
				const price = Number(this.dataset.price);
				const breakable = Number(this.dataset.breakable);

				let insufficent = false;

				const zenyCost = root.querySelector('.zeny_cost_container');
				if (zenyCost) {
					zenyCost.textContent = price;
				}

				const item = Inventory.getUI().getItemById(material_ITID);
				const material_count = item ? item.count : 0;

				if (!item || material_count < material_amount) {
					insufficent = true;
				}

				if (Session.zeny < price) {
					insufficent = true;
				}

				const scenarios = [];
				scenarios.push(breakable === 1 ? 'destroyable' : 'non_destroyable');
				if (insufficent) {
					scenarios.push('insufficient');
				}

				if (EnchantGrade_can_cont) {
					setMessages(...scenarios);
				}

				if (insufficent === false) {
					const startButton = root.querySelector('.start_grade');
					if (startButton) {
						const newStartBtn = startButton.cloneNode(true);
						startButton.parentNode.replaceChild(newStartBtn, startButton);

						if (EnchantGrade_can_cont === true) {
							newStartBtn.style.backgroundImage = `url(${startNormal})`;
							newStartBtn.addEventListener('click', () => {
								onRequestEnchantGrade();
							});
						} else {
							newStartBtn.style.backgroundImage = `url(${startDisable})`;
						}
					}
				}
			},
			{ signal: _slotSignal }
		);
	});
}

/**
 * Reset the blessing usage flag and the current blessing count.
 * Update the UI accordingly.
 */
function onResetBlessing() {
	const root = _root();
	EnchantGrade_blessing_used = false;
	EnchantGrade_currentBlessing = 0;

	const prob = root.querySelector('.probability');
	if (prob) {
		prob.textContent = `${EnchantGrade_current_success}%`;
	}

	const matAmount = root.querySelector('.BED_container .additonal_mat_amount');
	if (matAmount) {
		matAmount.textContent = `${EnchantGrade_currentBlessing} ea`;
	}
}

/**
 * Handles clearance of variables and UI components when changing Item
 */
function onRemoveItem() {
	const root = _root();

	gradingMaterials = [];

	if (EnchantGrade.imageLoopTimeout) {
		Object.keys(EnchantGrade.imageLoopTimeout).forEach(stopPhase);
	}

	const tankContainer = root.querySelector('.grade_tank_container');
	if (tankContainer) {
		tankContainer.style.visibility = 'hidden';
	}

	const enchantContainer = root.querySelector('.enchant_container');
	if (enchantContainer) {
		enchantContainer.innerHTML = '';
	}

	const probability = root.querySelector('.probability');
	if (probability) {
		probability.textContent = '';
	}

	const addBtn = root.querySelector('.add_material_btn');
	if (addBtn) {
		const newAddBtn = addBtn.cloneNode(true);
		addBtn.parentNode.replaceChild(newAddBtn, addBtn);
	}

	const bedContainer = root.querySelector('.BED_container');
	if (bedContainer) {
		bedContainer.innerHTML = '';
	}

	const tooltip = root.querySelector('.enchant_tooltip');
	if (tooltip) {
		tooltip.innerHTML = '';
		tooltip.style.display = 'none';
	}

	const zenyCost = root.querySelector('.zeny_cost_container');
	if (zenyCost) {
		zenyCost.textContent = '';
	}

	const notificationContainer = root.querySelector('.notification_container');
	if (notificationContainer) {
		notificationContainer.innerHTML = '';
	}
	setMessages('initial');

	if (_materialSlotAbort) {
		_materialSlotAbort.abort();
		_materialSlotAbort = null;
	}

	const slots = root.querySelectorAll('.material_slot');
	slots.forEach(slot => {
		delete slot.dataset.index;
		delete slot.dataset.itemid;
		delete slot.dataset.amount;
		delete slot.dataset.price;
		delete slot.dataset.downgrade;
		delete slot.dataset.breakable;
		slot.innerHTML = '';
		slot.classList.remove('selected');
		if (materialNormal) {
			slot.style.backgroundImage = `url(${materialNormal})`;
		}
	});

	const startBtn = root.querySelector('.start_grade');
	if (startBtn) {
		const newStartBtn = startBtn.cloneNode(true);
		startBtn.parentNode.replaceChild(newStartBtn, startBtn);
		newStartBtn.style.backgroundImage = `url(${startDisable})`;
	}

	enableDropProxy();
}

/**
 * Send the server request to EnchantGrade the item
 */
function onRequestEnchantGrade() {
	UIManager.showPromptBox(DB.getMessage(3823), 'ok', 'cancel', () => {
		const pkt = new PACKET.CZ.GRADE_ENCHANT_REQ();
		pkt.index = EnchantGrade_item_index;
		pkt.material_index = EnchantGrade_item_mat;
		pkt.blessing_flag = EnchantGrade_blessing_used ? 1 : 0;
		pkt.blessing_amount = EnchantGrade_currentBlessing;
		pkt.protect_flag = 0;
		Network.sendPacket(pkt);
	});
}

/**
 * Handles the received EnchantGrade result packet
 * @param {pkt} - PACKET_ZC_GRADE_ENCHANT_ACK
 * pkt.result:
 * 0 = The grade has been successfully upgraded.
 * 1= Refinement failed.
 * 2= The refine level has decreased.
 * 3= Equipment destroyed.
 * 4= The equipment is protected.
 */
function onEnchantGradeResult(pkt) {
	if (pkt) {
		EnchantGrade_result = pkt.result;

		controlPhase('process', true, 200, '.grade-wheel');

		playEffect(EffectConst.EF_NEW_INTRO, 2000, () => {
			stopPhase('process');

			switch (EnchantGrade_result) {
				case 0:
					playEffect(EffectConst.EF_NEW_SUCCESS, 2000, () => {
						onRemoveItem();
					});
					break;
				case 1:
					playEffect(EffectConst.EF_NEW_FAILURE, 2000, () => {
						onRemoveItem();
					});
					break;
				default:
					break;
			}
		});

		const item = Inventory.getUI().removeItem(pkt.index, 1);
		if (item) {
			item.enchantgrade = pkt.grade;
			item.RefiningLevel = pkt.result === 0 ? 0 : item.RefiningLevel;
			Inventory.getUI().addItem(item);
		}
	}
}

/**
 * Set the messages to be displayed in the notification area.
 * @param {...scenarioKeys} The keys of the messages to be displayed.
 */
function setMessages(...scenarioKeys) {
	const root = _root();
	const messages = [].concat(...scenarioKeys.map(key => scenarioMsgMapping[key] || []));
	const notificationArea = root.querySelector('.notification_container');

	if (!notificationArea) {
		return;
	}

	notificationArea.innerHTML = '';

	messages.slice(0, 3).forEach(msg => {
		const line = document.createElement('div');
		Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/caution.bmp', function (data) {
			line.innerHTML = `<img src="${data}" style="margin-right: 2px;"><span style="color: ${msg.color};">${DB.getMessage(msg.id)}</span>`;
		});
		notificationArea.appendChild(line);
	});
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const ITID = parseInt(this.getAttribute('data-index'), 10);
	const item = Inventory.getUI().getItemById(ITID)
		? Inventory.getUI().getItemById(ITID)
		: Inventory.getUI().getItemByIndex(ITID);

	if (!item) {
		return false;
	}

	if (ItemCompare.ui) {
		ItemCompare.remove();
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		if (ItemCompare.ui) {
			ItemCompare.remove();
		}
		return false;
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);

	const compareItem = Equipment.getUI().isInEquipList(item.location);

	if (compareItem && Inventory.getUI().itemcomp) {
		ItemCompare.prepare();
		ItemCompare.append();
		ItemCompare.uid = compareItem.ITID;
		ItemCompare.setItem(compareItem);
	}

	return false;
}

/**
 * Check if EnchantGrade UI is open
 */
EnchantGrade.isEnchantGradeOpen = function isEnchantGradeOpen() {
	return !!(EnchantGrade._host && EnchantGrade._host.isConnected);
};

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	event.dataTransfer.setData('text', event.target.id);
}

/**
 * End of dragging an item
 */
function onItemDragEnd(event) {
	const rect = EnchantGrade._host.getBoundingClientRect();
	const mouseX = event.clientX;
	const mouseY = event.clientY;

	if (mouseX < rect.left || mouseX > rect.right || mouseY < rect.top || mouseY > rect.bottom) {
		onRemoveItem();
	}
}

/**
 * EnchantGrade announcement received
 * @param {object} pkt - PACKET.ZC.GRADE_ENCHANT_BROADCAST_RESULT
 */
function onBroadcastEnchantGradeResult(pkt) {
	if (pkt) {
		const item = DB.getItemInfo(pkt.itemId);
		const itemName = item.identifiedDisplayName;
		let messageID;
		switch (pkt.status) {
			case 0:
				messageID = 3719;
				break;
			case 1:
				messageID = 3718;
				break;
			default:
				break;
		}

		const message = DB.getMessage(messageID)
			.replace('%s', pkt.char_name)
			.replace('%s', GradeMapping[pkt.grade])
			.replace('%s', itemName);
		ChatBox.addText(message, ChatBox.TYPE.ANNOUNCE, ChatBox.FILTER.PUBLIC_CHAT, '#FFB563');
		Announce.append();
		Announce.set(message, '#FFB563');
	}
}

/**
 * Play an effect on the client
 * @param {number} effectId - The id of the effect to play
 * @param {number} duration - The duration of the effect in milliseconds
 * @param {function} [onEnd] - A callback to execute when the effect ends
 */
function playEffect(effectId, duration, onEnd) {
	const EF_Init_Par = {
		effectId: effectId,
		ownerAID: Session.AID
	};

	EffectManager.spam(EF_Init_Par);

	if (onEnd) {
		setTimeout(onEnd, duration);
	}
}

/**
 * Packet Hooks to functions
 */
Network.hookPacket(PACKET.ZC.GRADE_ENCHANT_MATERIAL_LIST, onEnchantGradeUIUpdateMaterials);
Network.hookPacket(PACKET.ZC.GRADE_ENCHANT_ACK, onEnchantGradeResult);
Network.hookPacket(PACKET.ZC.GRADE_ENCHANT_BROADCAST_RESULT, onBroadcastEnchantGradeResult);

/**
 * Create component and export it
 */
export default UIManager.addComponent(EnchantGrade);

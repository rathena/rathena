/**
 * UI/Components/Reputation/Reputation.js
 *
 * Display Reputation
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './Reputation.html?raw';
import cssText from './Reputation.css?raw';

/**
 * Create Component
 */
const Reputation = new GUIComponent('Reputation', cssText);

/**
 * @var {Preferences} Window preferences
 */
const _preferences = Preferences.get(
	'Reputation',
	{
		x: 400,
		y: 200,
		show: true
	},
	1.0
);

/**
 * @var Reputation UI
 * Store Background Images Data
 */
let bg, bg_highlight, indicator_empty, indicator_blue, indicator_red;

/**
 * @var Reputation UI
 * Store Pagination Data
 */
Reputation.page = {
	current: 0,
	perPage: 8,
	reputeIds: []
};

/**
 * @var Reputation UI
 * Store ReputeID and Points
 */
Reputation.reputeState = {
	// reputeId: points
};

/**
 * @var Reputation UI
 * Store Variables for Highlighted ReputeID
 */
Reputation.highlight = {
	reputeId: null,
	active: false,
	consumed: false
};

/**
 * Helper: query inside shadow root
 */
function _root() {
	return Reputation._shadow || Reputation._host;
}

/**
 * Render HTML
 */
Reputation.render = () => htmlText;

/**
 * Input protection for search field
 */
Reputation.captureKeyEvents = true;

/**
 * Initialize component
 */
Reputation.init = function init() {
	const root = _root();

	// Store our image assets
	Client.loadFile(DB.INTERFACE_PATH + 'reputation/bg_info.bmp', d => (bg = d));
	Client.loadFile(DB.INTERFACE_PATH + 'reputation/bg_highlight.bmp', d => (bg_highlight = d));
	Client.loadFile(DB.INTERFACE_PATH + 'reputation/light_empty.bmp', d => (indicator_empty = d));
	Client.loadFile(DB.INTERFACE_PATH + 'reputation/light_blue.bmp', d => (indicator_blue = d));
	Client.loadFile(DB.INTERFACE_PATH + 'reputation/light_red.bmp', d => (indicator_red = d));

	// Binds
	this.draggable(root.querySelector('.titlebar'));

	root.querySelectorAll('.base').forEach(el => {
		el.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
			e.preventDefault();
		});
	});

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', onClose);
	}

	const bigCloseBtn = root.querySelector('.big_btn_close');
	if (bigCloseBtn) {
		bigCloseBtn.addEventListener('click', onClose);
	}

	const prevBtn = root.querySelector('.page_prev');
	if (prevBtn) {
		prevBtn.addEventListener('click', () => {
			if (Reputation.page.current > 0) {
				renderReputePage(Reputation.page.current - 1);
			}
		});
	}

	const nextBtn = root.querySelector('.page_next');
	if (nextBtn) {
		nextBtn.addEventListener('click', () => {
			const maxPage = Math.ceil(Reputation.page.reputeIds.length / Reputation.page.perPage);

			if (Reputation.page.current < maxPage - 1) {
				renderReputePage(Reputation.page.current + 1);
			}
		});
	}
};

/**
 * Guard keyboard events when search input is focused
 */
Reputation.onKeyDown = function onKeyDown(event) {
	if (Reputation.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}
	return true;
};

/**
 * Called when the component is appended to the body.
 * Initializes the reputation system by building the group selector,
 * binding group selector events and rendering the default view.
 */
Reputation.onAppend = function onAppend() {
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), window.innerHeight - (this._host.offsetHeight || 0))}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), window.innerWidth - (this._host.offsetWidth || 0))}px`;

	buildGroupSelector();
	bindGroupSelector();
	bindSearch();
	buildAllReputeEntries();
	filterByGroup('all');
};

/**
 * Once remove from body, save user preferences
 */
Reputation.onRemove = function onRemove() {
	_preferences.show = this._host.style.display !== 'none';
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();
};

/**
 * Request to toggle open/close reputation
 */
Reputation.toggle = function toggle() {
	const root = _root();
	const selector = root.querySelector('#repute_groups');
	const searchInput = root.querySelector('.rep_group_searchbar');
	const isVisible = this._host.style.display !== 'none' && this._host.offsetParent !== null;

	if (isVisible) {
		this._host.style.display = 'none';
		return;
	}

	// Reset
	if (selector) {
		selector.value = 'all';
	}
	if (searchInput) {
		searchInput.value = '';
	}
	filterByGroup('all');

	// Reset paging explicitly
	Reputation.page.current = 0;
	Reputation.focus();
	this._host.style.display = '';
};

/**
 * Build the group selector based on the ReputeGroupTable in DB
 */
function buildGroupSelector() {
	const root = _root();
	const groupSelector = root.querySelector('#repute_groups');
	if (!groupSelector) {
		return;
	}
	const ReputeGroupTable = DB.getReputeGroup();

	Object.entries(ReputeGroupTable).forEach(([groupIndex, group]) => {
		const option = document.createElement('option');
		option.className = `repute_group_${groupIndex}`;
		option.value = groupIndex;
		option.textContent = group.Name;
		groupSelector.appendChild(option);
	});
}

/**
 * Bind the group selector with the reputation page
 */
function bindGroupSelector() {
	const root = _root();
	const groupSelector = root.querySelector('#repute_groups');
	if (!groupSelector) {
		return;
	}

	groupSelector.addEventListener('change', function () {
		const selectedGroup = this.value;
		updateGroupTotalPoints(selectedGroup);
		const searchInput = root.querySelector('.rep_group_searchbar');
		if (searchInput) {
			searchInput.value = '';
		}
		filterByGroup(selectedGroup);
	});
}

/**
 * Bind search functionality to the input field and button
 */
function bindSearch() {
	const root = _root();
	const input = root.querySelector('.rep_group_searchbar');
	const button = root.querySelector('.search');

	if (input) {
		input.addEventListener('keydown', function (e) {
			if (e.key === 'Enter') {
				performSearch(this.value.trim());
			}
		});
	}

	if (button) {
		button.addEventListener('click', () => {
			if (input) {
				performSearch(input.value.trim());
			}
		});
	}
}

/**
 * Build all reputation entries once
 */
function buildAllReputeEntries() {
	const root = _root();
	const content = root.querySelector('.content');
	if (!content) {
		return;
	}
	const reputeIds = Object.keys(DB.getReputeInfo()).map(Number);
	Reputation.page.reputeIds = reputeIds;

	content.innerHTML = '';
	reputeIds.forEach(reputeId => {
		const info = DB.getReputeData(reputeId);
		if (!info) {
			return;
		}
		const points = Reputation.reputeState?.[reputeId] || 0;
		const entry = createReputeEntry(reputeId, info);
		entry.dataset.visible = isReputeVisible(info, points) ? 'true' : 'false';
		content.appendChild(entry);
		updateReputeUI(reputeId, points);
	});
}

/**
 * Update the total points display for a given group id
 * @param {string} groupId
 */
function updateGroupTotalPoints(groupId) {
	const root = _root();
	const wrapper = root.querySelector('.rep_group_total_points_wrapper');
	if (!wrapper) {
		return;
	}
	const indicator = wrapper.querySelector('.rep_indicator');
	const pointsValue = wrapper.querySelector('.rep_group_total_points_value');

	if (groupId === 'all') {
		wrapper.style.display = 'none';
		return;
	}

	wrapper.style.display = 'flex';

	// Get all repute IDs in this group
	let reputeIds = DB.getReputeGroupList(groupId);

	// Convert to array
	if (reputeIds && typeof reputeIds === 'object') {
		reputeIds = Object.values(reputeIds);
	}

	if (!reputeIds || !reputeIds.length) {
		if (indicator) {
			indicator.style.backgroundImage = `url(${indicator_empty})`;
		}
		if (pointsValue) {
			pointsValue.textContent = '0';
		}
		return;
	}

	// Add Total Points
	let totalPoints = 0;
	reputeIds.forEach(reputeId => {
		totalPoints += Reputation.reputeState?.[reputeId] || 0;
	});

	// Update indicator
	if (indicator) {
		if (totalPoints > 0) {
			indicator.style.backgroundImage = `url(${indicator_blue})`;
		} else if (totalPoints < 0) {
			indicator.style.backgroundImage = `url(${indicator_red})`;
		} else {
			indicator.style.backgroundImage = `url(${indicator_empty})`;
		}
	}

	if (pointsValue) {
		pointsValue.textContent = `${totalPoints} P`;
	}
}

/**
 * Checks if a reputation entry is visible based on the info.Invisible value and the current points value
 * @param {object} info
 * @param {number} points
 * @returns {boolean}
 */
function isReputeVisible(info, points) {
	switch (info.Invisible) {
		case 'VISIBLE_FALSE':
			return false;
		case 'VISIBLE_EXIST':
			return points !== 0;
		case 'VISIBLE_TRUE':
		default:
			return true;
	}
}

/**
 * Filters the reputation entries based on the given group ID
 * @param {string} groupId
 */
function filterByGroup(groupId) {
	const root = _root();
	const items = root.querySelectorAll('.rep_group_wrapper');
	const totalWrapper = root.querySelector('.rep_group_total_points_wrapper');

	if (groupId === 'all') {
		items.forEach(el => {
			const reputeId = Number(el.dataset.reputeId);
			const info = DB.getReputeData(reputeId);
			const points = Reputation.reputeState?.[reputeId] || 0;
			el.dataset.visible = isReputeVisible(info, points) ? 'true' : 'false';
		});
		if (totalWrapper) {
			totalWrapper.style.display = 'none';
		}
	} else {
		const groupList = Object.values(DB.getReputeGroupList(groupId) || {});
		items.forEach(el => {
			const reputeId = Number(el.dataset.reputeId);
			const info = DB.getReputeData(reputeId);
			const points = Reputation.reputeState?.[reputeId] || 0;
			el.dataset.visible = groupList.includes(reputeId) && isReputeVisible(info, points) ? 'true' : 'false';
		});
		updateGroupTotalPoints(groupId);
		if (totalWrapper) {
			totalWrapper.style.display = 'flex';
		}
	}

	const visibleIds = [];
	items.forEach(el => {
		if (el.dataset.visible === 'true') {
			visibleIds.push(Number(el.dataset.reputeId));
		}
	});
	Reputation.page.reputeIds = visibleIds;
	Reputation.page.current = 0;
	renderReputePage(0);
}

/**
 * Update the reputation pager UI.
 */
function updateReputePager() {
	const root = _root();
	const total = Reputation.page.reputeIds.length;
	const perPage = Reputation.page.perPage;

	const totalPages = Math.max(1, Math.ceil(total / perPage));
	const current = Reputation.page.current + 1;

	const prevBtn = root.querySelector('.page_prev');
	const nextBtn = root.querySelector('.page_next');
	const text = root.querySelector('.page_text');

	if (text) {
		text.textContent = `${current} / ${totalPages}`;
	}
	if (prevBtn) {
		prevBtn.style.visibility = current === 1 ? 'hidden' : 'visible';
	}
	if (nextBtn) {
		nextBtn.style.visibility = current === totalPages ? 'hidden' : 'visible';
	}

	if (totalPages === 1) {
		if (prevBtn) {
			prevBtn.style.visibility = 'hidden';
		}
		if (nextBtn) {
			nextBtn.style.visibility = 'hidden';
		}
	}
}

/**
 * Render a page of reputation entries based on current page index.
 * @param {number} pageIndex
 */
function renderReputePage(pageIndex) {
	const root = _root();
	const { perPage } = Reputation.page;
	const items = root.querySelectorAll('.rep_group_wrapper');

	Reputation.page.current = pageIndex;

	items.forEach(el => {
		el.style.display = 'none';
	});

	// Filter only visible entries according to Invisible property
	const visibleItems = [];
	items.forEach(el => {
		if (el.dataset.visible === 'true') {
			visibleItems.push(el);
		}
	});

	const start = pageIndex * perPage;
	const end = start + perPage;

	const pageItems = visibleItems.slice(start, end);
	pageItems.forEach(el => {
		el.style.display = '';
	});

	// Show Highlight
	if (Reputation.highlight.active && !Reputation.highlight.consumed) {
		pageItems.forEach(el => {
			const id = Number(el.dataset.reputeId);

			if (id === Reputation.highlight.reputeId) {
				el.style.backgroundImage = `url(${bg_highlight})`;

				// Consume Highlight
				Reputation.highlight.consumed = true;
				Reputation.highlight.active = false;
			}
		});
	} else {
		clearHighlights();
	}

	updateReputePager();
}

/**
 * Create a reputation entry element from given info
 * @param {number} reputeId
 * @param {object} info
 * @returns {HTMLDivElement}
 */
function createReputeEntry(reputeId, info) {
	const wrapper = document.createElement('div');
	wrapper.className = 'rep_group_wrapper';
	wrapper.dataset.reputeId = reputeId;
	wrapper.style.backgroundImage = `url(${bg})`;

	// Name
	const name = document.createElement('div');
	name.className = 'rep_group_name';
	name.textContent = info.Name;
	wrapper.appendChild(name);

	// Indicators
	const indicatorWrapper = document.createElement('div');
	indicatorWrapper.className = 'rep_group_indicator_wrapper';

	const indicatorCount = info.MaxPoint_Positive / 1000;

	for (let i = 1; i <= indicatorCount; i++) {
		const indicator = document.createElement('div');
		indicator.className = `rep_group_indicator indicator_${i}`;
		indicator.dataset.index = i;
		indicator.style.backgroundImage = `url(${indicator_empty})`;
		indicatorWrapper.appendChild(indicator);
	}

	wrapper.appendChild(indicatorWrapper);

	// Progress bar
	const progress = document.createElement('div');
	progress.className = 'rep_group_progess_bar';
	wrapper.appendChild(progress);

	// Points
	const points = document.createElement('div');
	points.className = 'rep_id_points';
	points.textContent = '0 / 1000';
	wrapper.appendChild(points);

	return wrapper;
}

/**
 * Update the UI of a specific Repute entry
 * @param {string} reputeId
 * @param {number} points
 */
function updateReputeUI(reputeId, points) {
	const root = _root();
	const wrapper = root.querySelector(`.rep_group_wrapper[data-repute-id="${reputeId}"]`);

	if (!wrapper) {
		return;
	}

	const indicators = wrapper.querySelectorAll('.rep_group_indicator');
	const absPoints = Math.abs(points);
	const fullIndicators = Math.floor(absPoints / 1000);
	const remainder = absPoints % 1000;

	// Update indicators
	indicators.forEach((el, index) => {
		el.style.backgroundImage =
			index < fullIndicators
				? points >= 0
					? `url(${indicator_blue})`
					: `url(${indicator_red})`
				: `url(${indicator_empty})`;
	});

	// Update points text
	const pointsEl = wrapper.querySelector('.rep_id_points');
	if (pointsEl) {
		pointsEl.textContent = `${remainder} / 1000`;
	}

	// Update progress bar
	const progress = wrapper.querySelector('.rep_group_progess_bar');
	if (progress) {
		const percent = (remainder / 1000) * 100;
		const fillColor = points >= 0 ? '#7b95ce' : '#f60206';
		progress.style.backgroundImage = `linear-gradient(to right, ${fillColor} 0%, ${fillColor} ${percent}%, transparent ${percent}%)`;
	}
}

/**
 * Perform search on the reputation entries based on the given query.
 * @param {string} query
 */
function performSearch(query) {
	const root = _root();
	const items = root.querySelectorAll('.rep_group_wrapper');
	const totalWrapper = root.querySelector('.rep_group_total_points_wrapper');
	const groupSelector = root.querySelector('#repute_groups');

	if (!query) {
		if (groupSelector) {
			groupSelector.value = 'all';
		}
		if (totalWrapper) {
			totalWrapper.style.display = 'none';
		}
		filterByGroup('all');
		return;
	}

	const lowerQuery = query.toLowerCase();

	const visibleIds = [];
	items.forEach(el => {
		const reputeId = Number(el.dataset.reputeId);
		const info = DB.getReputeData(reputeId);
		const points = Reputation.reputeState?.[reputeId] || 0;

		const matches = info?.Name.toLowerCase().includes(lowerQuery);
		const show = matches && isReputeVisible(info, points);
		el.dataset.visible = show ? 'true' : 'false';
		if (show) {
			visibleIds.push(reputeId);
		}
	});

	Reputation.page.reputeIds = visibleIds;
	Reputation.page.current = 0;
	renderReputePage(0);
}

/**
 * Closing window
 */
function onClose() {
	Reputation._host.style.display = 'none';
}

/**
 * Packet received from server
 * Initializes the Reputation UI
 * @param {object} pkt - PACKET.ZC.REPUTE_INFO
 */
function onReputeInfo(pkt) {
	if (!Reputation.__active) {
		Reputation.append();
		Reputation._host.style.display = 'none';
	}

	const root = _root();

	// Get currently selected group
	const groupSelector = root.querySelector('#repute_groups');
	const selectedGroup = groupSelector ? groupSelector.value : 'all';

	// Track which groups need updating
	let updateSelectedGroup = false;

	pkt.reputeInfo.forEach(entry => {
		// Update individual UI
		Reputation.reputeState[entry.type] = entry.points;
		updateReputeUI(entry.type, entry.points);

		// Check if the repute ID belongs to the currently selected group
		if (selectedGroup && selectedGroup !== 'all') {
			const reputeList = Object.values(DB.getReputeGroupList(selectedGroup) || {});
			if (reputeList.includes(entry.type)) {
				updateSelectedGroup = true;
			}
		}
	});

	// Update total points only if relevant
	if (updateSelectedGroup) {
		updateGroupTotalPoints(selectedGroup);
	}
}

/**
 * Sets up Highlight, selects the given group,
 * and filters the reputation entries normally
 *
 * @param {object} pkt - PACKET.ZC.REPUTE_OPEN
 */
function onReputeOpen(pkt) {
	const groupId = pkt.table === 0 ? 'all' : pkt.table;
	const highlightId = pkt.type || null;

	// setup highlight
	Reputation.highlight.reputeId = highlightId;
	Reputation.highlight.active = !!highlightId;
	Reputation.highlight.consumed = false;

	const root = _root();

	// select group
	const groupSelector = root.querySelector('#repute_groups');
	if (groupSelector) {
		groupSelector.value = groupId;
	}

	// filter normally
	filterByGroup(groupId);

	Reputation._host.style.display = '';
}

/**
 * Clears all highlight effects from the reputation entries.
 */
function clearHighlights() {
	const root = _root();
	root.querySelectorAll('.rep_group_wrapper').forEach(el => {
		el.style.backgroundImage = `url(${bg})`;
		el.dataset.highlight = 'false';
	});
}

/**
 * Packet Hooks to functions
 */
Network.hookPacket(PACKET.ZC.REPUTE_INFO, onReputeInfo);
Network.hookPacket(PACKET.ZC_REPUTE_OPEN, onReputeOpen);

/**
 * Create component and export it
 */
export default UIManager.addComponent(Reputation);

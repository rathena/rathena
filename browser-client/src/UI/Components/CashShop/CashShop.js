/**
 * UI/Components/CashShop/CashShop.js
 *
 * CashShop windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Stephen-A
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import KEYS from 'Controls/KeyEventHandler.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Renderer from 'Renderer/Renderer.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './CashShop.html?raw';
import cssText from './CashShop.css?raw';

const CashShop = new GUIComponent('CashShop', cssText);

/**
 * Store cash shop items
 */
CashShop.list = [];

const _preferences = Preferences.get(
	'CashShop',
	{
		x: 80,
		y: 100,
		magnet_top: false,
		magnet_bottom: false,
		magnet_left: false,
		magnet_right: false
	},
	1.0
);

/**
 * Store cash shop items
 */

CashShop.cashShopListItem = [];
CashShop.csListItemSearchResult = [];

/**
 * Place the default banner image here
 */
CashShop.cashShopBannerTable = [];

/**
 * Active cash shop menu
 */
CashShop.activeCashMenu = 0;

/**
 * Current Cash shop points user
 */
CashShop.cashPoint = 0;

/**
 * Current Kafra points user
 */
CashShop.kafraPoints = 0;

/**
 * Current Item in cart item
 */
CashShop.cartItem = [];

/**
 * Current cart item total price
 */
CashShop.cartItemTotalPrice = 0;

/**
 *  Check your item_cash_db.txt if how many tab is active
 */
CashShop.totalActiveCategory = 3;

/**
 * Pagination
 */
CashShop.pageOffset = 1;
CashShop.pageLimit = 9;
CashShop.currentPage = 1;
CashShop.totalPage = 0;
CashShop.isFirstPage = true;
CashShop.isLastPage = false;
CashShop.cartItemLen = 0;
CashShop.checkCartItemLen = 0;
CashShop.pageEnd = 9;
CashShop.isSearch = false;

/**
 * Banner Carousel
 */
CashShop.bannerInterval = null;
CashShop.currentBannerIndex = 0;
CashShop.bannerRotationTime = 5000; // 5 seconds

/**
 * Has input fields, protect key events
 */
CashShop.captureKeyEvents = true;

/**
 * Helper: query inside shadow root
 */
function _root() {
	return CashShop._shadow || CashShop._host;
}

/**
 * Helper: process data-* attributes on newly inserted content
 */
function _processContent(container) {
	const selector = '[data-background],[data-hover],[data-down],[data-active],[data-text],[data-preload]';
	if (container.matches && container.matches(selector)) {
		GUIComponent.processDataAttrs(container);
	}
	container.querySelectorAll(selector).forEach(node => GUIComponent.processDataAttrs(node));
}

/**
 * Render HTML
 */
CashShop.render = () => htmlText;

CashShop.init = function init() {
	const root = _root();

	const baseBtns = root.querySelectorAll('.titlebar .base');
	baseBtns.forEach(btn => {
		btn.addEventListener('mousedown', stopPropagation);
	});

	const closeBtn = root.querySelector('.titlebar .close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => {
			const pkt = new PACKET.CZ.CASH_SHOP_CLOSE();
			Network.sendPacket(pkt);
			CashShop.remove();
		});
	}

	this.draggable('.titlebar');

	const container = root.querySelector('.ui-component-root') || root;

	container.addEventListener('click', e => {
		const addCartBtn = e.target.closest('.purchase-btn-container button');
		if (addCartBtn) {
			onClickActionAddCartItem(e);
			return;
		}

		const purchaseBtn = e.target.closest('#purchase-btn');
		if (purchaseBtn) {
			onClickActionBuyItem();
			return;
		}

		const tab = e.target.closest('#panel-menu .tab');
		if (tab) {
			onClickMenu(tab);
			return;
		}

		const pagiHandler = e.target.closest('.panel-pagination .pagi-handler');
		if (pagiHandler) {
			onClickPagination(pagiHandler);
			return;
		}

		const searchBtn = e.target.closest('.cashshop-search-btn');
		if (searchBtn) {
			onClickSearch();
			return;
		}

		const counterBtn = e.target.closest('#cart-list .items .item .counter-btn');
		if (counterBtn) {
			onClickActionCounterButtonCart(counterBtn);
			return;
		}

		const deleteBtn = e.target.closest('#cart-list .items .item .delete-item');
		if (deleteBtn) {
			onClickDeleteItemInCart(deleteBtn);
			return;
		}

		const bannerDot = e.target.closest('.banner-dot');
		if (bannerDot) {
			const index = parseInt(bannerDot.getAttribute('data-index'), 10);
			CashShop.goToBanner(index);
			return;
		}

		const bannerSlide = e.target.closest('.banner-slide');
		if (bannerSlide) {
			const url = bannerSlide.getAttribute('data-url');
			if (url && url !== 'undefined' && url !== 'null') {
				window.open(url, '_blank');
			}
			return;
		}

		const chargeBtn = e.target.closest('.panel-cart-charge-btn');
		if (chargeBtn) {
			window.open(DB.getMessage(3301), '_blank');
			return;
		}
	});

	container.addEventListener('mouseover', e => {
		const tab = e.target.closest('#panel-menu .tab');
		if (tab) {
			onMouseOverTab(tab);
		}
	});

	container.addEventListener('mousemove', e => {
		const tab = e.target.closest('#panel-menu .tab');
		if (tab) {
			onMouseMoveTab(e);
		}
	});

	container.addEventListener('mouseout', e => {
		const tab = e.target.closest('#panel-menu .tab');
		if (tab) {
			onMouseOutTab();
		}
	});

	// Drop to cart
	const cartBody = root.querySelector('.panel-cart-body');
	if (cartBody) {
		cartBody.addEventListener('dragover', stopPropagation);
		cartBody.addEventListener('drop', onDropToCart);
	}

	// Items events (delegated)
	const panelItems = root.querySelector('.panel-content .panel-items');
	if (panelItems) {
		panelItems.addEventListener('dragstart', function (e) {
			const item = e.target.closest('.item');
			if (item) onItemDragStart.call(item, e);
		});
		panelItems.addEventListener('dragend', function (e) {
			const item = e.target.closest('.item');
			if (item) onItemDragEnd.call(item, e);
		});
		panelItems.addEventListener('contextmenu', function (e) {
			const item = e.target.closest('.item');
			if (item) onItemInfo.call(item, e);
		});
	}

	// Prevent dragover on whole component
	container.addEventListener('dragover', stopPropagation);

	const cartListItems = root.querySelector('#cart-list .items');
	if (cartListItems && cartListItems.children.length > 0) {
		onResetCartListCashShop();
	}

	CashShop.loadCashShopBanner();
};

CashShop.onAppend = function OnAppend() {
	const hostHeight = this._host.offsetHeight || 540;
	const hostWidth = this._host.offsetWidth || 723;
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostWidth)}px`;

	this.magnet.TOP = _preferences.magnet_top;
	this.magnet.BOTTOM = _preferences.magnet_bottom;
	this.magnet.LEFT = _preferences.magnet_left;
	this.magnet.RIGHT = _preferences.magnet_right;

	CashShop.loadComponentCashShop();
};

/**
 * Remove Cash shop
 */
CashShop.onRemove = function onRemove() {
	_preferences.x = parseInt(this._host.style.left, 10) || 0;
	_preferences.y = parseInt(this._host.style.top, 10) || 0;
	_preferences.magnet_top = this.magnet.TOP;
	_preferences.magnet_bottom = this.magnet.BOTTOM;
	_preferences.magnet_left = this.magnet.LEFT;
	_preferences.magnet_right = this.magnet.RIGHT;
	_preferences.save();

	CashShop.stopBannerRotation();

	CashShop.csListItemSearchResult = [];
	CashShop.cashShopBannerTable = [];

	// empty .panel-items
	const panelItems = _root().querySelector('.panel-items');
	if (panelItems) panelItems.innerHTML = '';

	// empty .cart-list .items
	CashShop.cartItem = [];
	CashShop.cartItemTotalPrice = 0;
	CashShop.cartItemLen = 0;
	CashShop.checkCartItemLen = 0;
	const cartListItems = _root().querySelector('.cart-list .items');
	if (cartListItems) cartListItems.innerHTML = '';

	// set price to 0C
	const totalPrice = _root().querySelector('.cart-footer-action .total-price span');
	if (totalPrice) totalPrice.textContent = '0 C';
	const freePoints = _root().querySelector('#use-free-points');
	if (freePoints) freePoints.value = '0';
};

CashShop.clean = function clean() {
	CashShop.cashShopListItem = [];
	CashShop.csListItemSearchResult = [];
	CashShop.cashShopBannerTable = [];
	CashShop.cartItem = [];
	CashShop.cartItemTotalPrice = 0;
	CashShop.cartItemLen = 0;
	CashShop.checkCartItemLen = 0;
	Session.requestCashShop = true;
};

CashShop.loadCashShopBanner = function loadCashShopBanner() {
	this.cashShopBannerTable = DB.getCashShopBannerTable();
	const root = _root();

	if (CashShop.cashShopBannerTable?.length > 0) {
		const banners = CashShop.cashShopBannerTable;
		const slidesContainer = root.querySelector('.banner-slides');
		const dotsContainer = root.querySelector('.banner-dots');

		if (slidesContainer) slidesContainer.innerHTML = '';
		if (dotsContainer) dotsContainer.innerHTML = '';

		banners.forEach((banner, index) => {
			// Slide
			const slide = document.createElement('div');
			slide.className = 'banner-slide' + (index === 0 ? ' active' : '');
			slide.setAttribute('data-index', index);
			slide.setAttribute('data-url', banner.url || '');
			slide.innerHTML = '<button class="btn-banner"></button>';

			Client.loadFile(DB.INTERFACE_PATH + 'cashshop/' + banner.bmp, function (data) {
				slide.style.backgroundImage = `url(${data})`;
			});

			if (slidesContainer) slidesContainer.appendChild(slide);

			// Dot
			const dot = document.createElement('li');
			dot.className = 'banner-dot' + (index === 0 ? ' active' : '');
			dot.setAttribute('data-background', 'cashshop/btn_ad_off.bmp');
			dot.setAttribute('data-active', 'cashshop/btn_ad_on.bmp');
			dot.setAttribute('data-index', index);
			dot.innerHTML = '<button class="btn-banner"></button>';

			if (dotsContainer) dotsContainer.appendChild(dot);
		});

		if (dotsContainer) _processContent(dotsContainer);

		CashShop.currentBannerIndex = 0;
	}
};

CashShop.onKeyDown = function onKeyDown(event) {
	if (this.isEditableFocused()) return;
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host && this._host.parentNode) {
		const pkt = new PACKET.CZ.CASH_SHOP_CLOSE();
		Network.sendPacket(pkt);
		CashShop.remove();
	}
};

CashShop.readPoints = function readPoints(cashPoint, kafraPoints, tab) {
	const root = _root();
	this.cashPoint = cashPoint;
	this.kafraPoints = kafraPoints;
	this.activeCashMenu = tab || 0; // 0x0845 no tab

	const cashpointSpan = root.querySelector('#cashpoint > span');
	if (cashpointSpan) cashpointSpan.textContent = this.cashPoint;

	const cashpointFooter = root.querySelector('.cashpoint_footer');
	if (cashpointFooter) cashpointFooter.textContent = this.cashPoint + ' C';

	const freePoint = root.querySelector('.free-point');
	if (freePoint) freePoint.textContent = this.kafraPoints + ' C';
};

/**
 * Search in a list for an item by its index
 *
 * @param {number} index
 * @returns {Item}
 */
CashShop.getItemByIndex = function getItemByIndex(index) {
	let i, count;
	let list;

	if (CashShop.isSearch) {
		list = CashShop.csListItemSearchResult;
		for (i = 0, count = list.length; i < count; ++i) {
			if (list[i].itemId === index) {
				return list[i];
			}
		}
	} else {
		if (CashShop.cashShopListItem[CashShop.activeCashMenu]) {
			list = CashShop.cashShopListItem[CashShop.activeCashMenu].items;
			for (i = 0, count = list.length; i < count; ++i) {
				if (list[i].itemId === index) {
					return list[i];
				}
			}
		}
	}

	return null;
};

/**
 * Success reponse buying cash shop item
 */
CashShop.setSuccessCashShopUpdate = function setSuccessCashShopUpdate(res) {
	if (res) {
		CashShop.checkCartItemLen += 1;
		switch (res.result) {
			case 0:
				if (CashShop.checkCartItemLen >= CashShop.cartItemLen) {
					CashShop.cartItemLen = 0;
					CashShop.checkCartItemLen = 0;
					UIManager.showMessageBox('Successfully done buying items from cash shop!', 'ok');
					ChatBox.addText(
						'Successfully done buying items from cash shop!',
						ChatBox.TYPE.INFO,
						ChatBox.FILTER.PUBLIC_LOG
					);
					const root = _root();
					const cashpointSpan = root.querySelector('#cashpoint span');
					if (cashpointSpan) cashpointSpan.textContent = res.cashPoints;
					const cashpointFooter = root.querySelector('.cashpoint_footer');
					if (cashpointFooter) cashpointFooter.textContent = res.cashPoints;
					onResetCartListCashShop();
				}
				break;
			case 2:
				//insuficient cashpoint or kafra points
				UIManager.showMessageBox('Insuficient cash points or kafra points!', 'ok');
				ChatBox.addText(
					'Insuficient cash points or kafra points!',
					ChatBox.TYPE.ERROR,
					ChatBox.FILTER.PUBLIC_LOG
				);
				break;

			case 4:
				//overweight limit
				UIManager.showMessageBox("You are over you're weight limit!", 'ok');
				ChatBox.addText('You are over youre weight limit!', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
				break;

			default:
				UIManager.showMessageBox('Something went wrong while using cashshop!', 'ok');
				ChatBox.addText(
					'Something went wrong while using cashshop!',
					ChatBox.TYPE.ERROR,
					ChatBox.FILTER.PUBLIC_LOG
				);
				break;
		}
	}
};

CashShop.readCashShopItems = function readCashShopItems(items) {
	CashShop.cashShopListItem.push({
		count: items.count,
		items: items.items,
		tabNum: items.tabNum
	});
};

/**
 * Load Cash Shop Components
 */
CashShop.loadComponentCashShop = function loadComponentCashShop() {
	const root = _root();

	if (CashShop.cashShopBannerTable?.length > 0) {
		CashShop.startBannerRotation();
	}

	CashShop.isSearch = false;

	// toggle active at activeCashmenu using data-index to search
	root.querySelectorAll('#panel-menu .tab').forEach(el => el.classList.remove('active'));
	const activeTab = root.querySelector(`#panel-menu .tab[data-index="${CashShop.activeCashMenu}"]`);
	if (activeTab) activeTab.classList.add('active');

	const tab_items =
		CashShop.cashShopListItem[CashShop.activeCashMenu]?.items?.length >= 0
			? CashShop.cashShopListItem[CashShop.activeCashMenu].items
			: [];
	CashShop.initPagination(tab_items);
	CashShop.renderCashShopItems(CashShop.paginate(tab_items, CashShop.pageOffset, CashShop.pageEnd));
};

CashShop.startBannerRotation = function () {
	this.stopBannerRotation();
	if (this.cashShopBannerTable.length > 1) {
		this.bannerInterval = setInterval(() => {
			this.nextBanner();
		}, this.bannerRotationTime);
	}
};

CashShop.stopBannerRotation = function () {
	if (this.bannerInterval) {
		clearInterval(this.bannerInterval);
		this.bannerInterval = null;
	}
};

CashShop.nextBanner = function () {
	let nextIndex = this.currentBannerIndex + 1;
	if (nextIndex >= this.cashShopBannerTable.length) {
		nextIndex = 0;
	}
	this.goToBanner(nextIndex);
};

CashShop.goToBanner = function (index) {
	if (index === this.currentBannerIndex) {
		return;
	}

	const root = _root();
	const slides = root.querySelectorAll('.banner-slide');
	const dots = root.querySelectorAll('.banner-dot');

	slides.forEach(s => s.classList.remove('active'));
	dots.forEach(d => d.classList.remove('active'));

	if (slides[index]) slides[index].classList.add('active');
	if (dots[index]) dots[index].classList.add('active');

	this.currentBannerIndex = index;

	// Reset timer on manual interaction
	this.startBannerRotation();
};

CashShop.renderCashShopItems = function renderCashShopItems(items) {
	const root = _root();
	const content = root.querySelector('#panel-items');
	if (!content) return;

	content.innerHTML = '';
	for (let i = 0; i < items.length; i++) {
		const item = structuredClone(items[i]);
		const it = DB.getItemInfo(item.itemId);
		content.insertAdjacentHTML(
			'beforeend',
			`<div class="item" draggable="true" title="${it.identifiedDisplayName}" data-index="${item.itemId}" data-background="cashshop/img_shop_itembg.bmp">
				<div class="top-con">
					<span class="item-name">${it.identifiedDisplayName}</span>
				</div>
				<div class="lower-con">
					<div class="item-left-img" data-background="${'collection/' + it.identifiedResourceName + '.bmp'}"></div>
					<div class="item-right-desc">
						<div class="item-desc-price">
							<span>${item.price}</span>
						</div>
						<div class="purchase-btn-container">
							<button class="add-to-cart" data-itemid="${item.itemId}" tab-index="${i}" data-background="cashshop/btn_add_normal.bmp" data-down="cashshop/btn_add_press.bmp">Purchase</button>
						</div>
					</div>
				</div>
			</div>`
		);
	}
	_processContent(content);
};

CashShop.initPagination = function initPagination(items) {
	const root = _root();
	CashShop.currentPage = 1;
	CashShop.pageOffset = 0;
	CashShop.pageEnd = CashShop.pageLimit;
	CashShop.isFirstPage = true;
	CashShop.isLastPage = items.length >= CashShop.pageLimit ? false : true;

	const arrowsL = CashShop.isFirstPage ? 'off' : 'on';
	const arrowsR = CashShop.isLastPage ? 'off' : 'on';

	CashShop.totalPage = Math.ceil(items.length / CashShop.pageLimit);

	const countPage = root.querySelector('.panel-pagination span.pagi-countpage');
	if (countPage) countPage.textContent = CashShop.totalPage;

	const changePage = root.querySelector('.panel-pagination span.pagi-changepage');
	if (changePage) changePage.textContent = CashShop.totalPage > 0 ? 1 : 0;

	const goNext = root.querySelector('.panel-pagination .go-next');
	const goLast = root.querySelector('.panel-pagination .go-last');
	const goPrev = root.querySelector('.panel-pagination .go-prev');
	const goFirst = root.querySelector('.panel-pagination .go-first');

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowR_' + arrowsR + '.bmp', function (data) {
		if (goNext) goNext.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowR2_' + arrowsR + '.bmp', function (data) {
		if (goLast) goLast.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowL_' + arrowsL + '.bmp', function (data) {
		if (goPrev) goPrev.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowL2_' + arrowsL + '.bmp', function (data) {
		if (goFirst) goFirst.style.backgroundImage = `url(${data})`;
	});
};

CashShop.paginate = function paginate(items, start, end) {
	return items.slice(start, end);
};

CashShop.paginationOffsetLimit = function paginationOffsetLimit() {
	const start = (CashShop.currentPage - 1) * CashShop.pageLimit;
	const end = CashShop.pageLimit * CashShop.currentPage;
	CashShop.pageOffset = start;
	CashShop.pageEnd = end;
};

function onClickPagination(target) {
	const root = _root();
	const index = parseInt(target.dataset.index);
	const items = CashShop.isSearch
		? CashShop.csListItemSearchResult
		: CashShop.cashShopListItem[CashShop.activeCashMenu]?.items || [];
	if (items.length === 0) {
		return;
	}

	CashShop.isFirstPage = false;
	CashShop.isLastPage = false;

	switch (index) {
		case 1: //go first//
			CashShop.currentPage = 1;
			CashShop.pageOffset = 0;
			CashShop.isFirstPage = true;
			break;
		case 2: //go prev//
			CashShop.currentPage--;
			if (CashShop.currentPage <= 1) {
				CashShop.currentPage = 1;
				CashShop.isFirstPage = true;
			}
			break;
		case 3: //go next//
			CashShop.currentPage++;
			if (CashShop.currentPage >= CashShop.totalPage) {
				CashShop.currentPage = CashShop.totalPage;
				CashShop.isLastPage = true;
			}
			break;
		case 4: //go last//
			CashShop.currentPage = CashShop.totalPage;
			CashShop.pageOffset = (CashShop.currentPage - 1) * CashShop.pageLimit;
			CashShop.isLastPage = true;
			break;
	}
	CashShop.paginationOffsetLimit();

	const arrowsL = CashShop.isFirstPage ? 'off' : 'on';
	const arrowsR = CashShop.isLastPage ? 'off' : 'on';

	const goNext = root.querySelector('.panel-pagination .go-next');
	const goLast = root.querySelector('.panel-pagination .go-last');
	const goPrev = root.querySelector('.panel-pagination .go-prev');
	const goFirst = root.querySelector('.panel-pagination .go-first');

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowR_' + arrowsR + '.bmp', function (data) {
		if (goNext) goNext.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowR2_' + arrowsR + '.bmp', function (data) {
		if (goLast) goLast.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowL_' + arrowsL + '.bmp', function (data) {
		if (goPrev) goPrev.style.backgroundImage = `url(${data})`;
	});

	Client.loadFile(DB.INTERFACE_PATH + 'cashshop/bt_arrowL2_' + arrowsL + '.bmp', function (data) {
		if (goFirst) goFirst.style.backgroundImage = `url(${data})`;
	});

	const changePage = root.querySelector('.panel-pagination span.pagi-changepage');
	if (changePage) changePage.textContent = CashShop.currentPage;

	CashShop.renderCashShopItems(CashShop.paginate(items, CashShop.pageOffset, CashShop.pageEnd));
}

function onClickSearch() {
	const root = _root();
	const searchInput = root.querySelector('.cashshop-search');
	const val = searchInput ? searchInput.value.toLowerCase() : '';
	const newList = [];

	CashShop.isSearch = true;
	CashShop.activeCashMenu = 9;
	if (val && CashShop.cashShopListItem.length > 0) {
		for (let i = 0; i < CashShop.cashShopListItem.length; ++i) {
			const items = CashShop.cashShopListItem[i].items;
			for (let iit = 0; iit < items.length; ++iit) {
				items[iit].tab = CashShop.cashShopListItem[i].tabNum;
				const it = DB.getItemInfo(items[iit].itemId);

				if (it.identifiedDisplayName) {
					const matches = new RegExp(val).test(it.identifiedDisplayName.toLowerCase());
					if (matches) {
						newList.push(items[iit]);
					}
				}
			}
		}
	}

	const searchResultTab = root.querySelector('#panel-menu .search-result');
	if (!searchResultTab) {
		root.querySelectorAll('#panel-menu .tab').forEach(el => el.classList.remove('active'));
	}

	if (newList.length === 0) {
		UIManager.showMessageBox('No items found in auction search', 'ok');
		return;
	}

	CashShop.csListItemSearchResult = newList;
	CashShop.initPagination(newList);
	CashShop.paginationOffsetLimit();
	CashShop.renderCashShopItems(CashShop.paginate(newList, CashShop.pageOffset, CashShop.pageEnd));
}

function onClickActionCounterButtonCart(target) {
	const counter = target.dataset.index;
	const itemEl = target.closest('.item');
	if (!itemEl) return;
	const itemId = parseInt(itemEl.getAttribute('data-index'), 10);
	const itemCart = CashShop.cartItem.find(i => i.itemId === itemId);

	if (!itemCart) return;

	if (itemCart.amount >= 99 && counter === 'up') {
		UIManager.showMessageBox('Max Quantity 99!', 'ok');
		ChatBox.addText('Max Quantity 99!', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	if (itemCart.amount <= 1 && counter === 'down') {
		UIManager.showMessageBox('Minimum Quantity 1!', 'ok');
		ChatBox.addText('Minimum Quantity 1!', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	if (counter === 'up') {
		itemCart.amount += 1;
	} else {
		itemCart.amount -= 1;
	}

	const cntEl = itemEl.querySelector('.item-counter .item-cnt');
	if (cntEl) cntEl.textContent = itemCart.amount;
	updateCartTotal();
}

function onClickDeleteItemInCart(target) {
	const itemEl = target.closest('.item');
	if (!itemEl) return;
	const itemId = parseInt(itemEl.getAttribute('data-index'), 10);

	CashShop.cartItem = CashShop.cartItem.filter(i => i.itemId != itemId);
	itemEl.remove();
	updateCartTotal();
}

function updateCartTotal() {
	const root = _root();
	if (CashShop.cartItem.length === 0) {
		CashShop.cartItemTotalPrice = 0;
	} else {
		CashShop.cartItemTotalPrice = CashShop.cartItem.reduce((total, item) => total + item.price * item.amount, 0);
	}
	const totalPriceEl = root.querySelector('.cart-footer-action .total-price span');
	if (totalPriceEl) totalPriceEl.textContent = CashShop.cartItemTotalPrice + ' C';
}

function onResetCartListCashShop() {
	const root = _root();
	CashShop.cartItem = [];
	CashShop.cartItemTotalPrice = 0;
	const totalPriceEl = root.querySelector('.cart-footer-action .total-price span');
	if (totalPriceEl) totalPriceEl.textContent = '0 C';
	const cartItems = root.querySelector('.panel-cart #cart-list .items');
	if (cartItems) cartItems.innerHTML = '';
}

function onDropToCart(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return false;
	}

	if (data.type === 'item' && data.from === 'CashShop') {
		InputBox.append();
		InputBox.setType('number', false, 1);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			addItemToCart(item.itemId, count);
			InputBox.remove();
		};
	}

	return false;
}

function addItemToCart(itemId, amount = 1) {
	const root = _root();
	const it = DB.getItemInfo(itemId);
	const cartContainer = root.querySelector('.panel-cart');
	const itemCart = CashShop.cartItem.find(i => i.itemId === itemId);
	let item = [];
	let tab = 0;

	if (CashShop.activeCashMenu !== 9) {
		item = CashShop.cashShopListItem[CashShop.activeCashMenu].items.find(i => i.itemId === itemId);
		tab = CashShop.cashShopListItem[CashShop.activeCashMenu].tabNum;
	} else {
		item = CashShop.csListItemSearchResult.find(i => i.itemId === itemId);
		tab = item.tab;
	}

	if (!item) {
		return;
	}

	const noItems = cartContainer ? cartContainer.querySelector('#cart-list .items .no-items') : null;
	if (noItems) {
		noItems.remove();
	}

	if (CashShop.cartItem.length > 7 && typeof itemCart === 'undefined') {
		UIManager.showMessageBox('8 Items can only be stored in cart!', 'ok');
		return;
	}

	if (item.amount >= 99) {
		UIManager.showMessageBox('Max Quantity 99!', 'ok');
		ChatBox.addText('Max Quantity 99!', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	if (typeof itemCart === 'undefined') {
		item.amount = amount;
		item.tab = tab;
		CashShop.cartItem.push(item);
		const html = `<li class="item" data-index="${itemId}" data-background="cashshop/img_shop_itembg2.bmp">
			<div class="inner-item-dt">
				<div class="delete-item" data-background="cashshop/btn_close.bmp"></div>
				<div class="item-dt-img"></div>
				<div class="item-dt-desc">
					<div class="item-desc-top">${it.identifiedDisplayName}</div>
					<div class="item-counter">
						<div class="item-cnt">${item.amount}</div>
						<button class="counter-btn item-cnt-up" data-index="up"></button>
						<button class="counter-btn item-cnt-down" data-index="down"></button>
					</div>
					<div class="item-desc-price">
						<span>${item.price}</span> C
					</div>
				</div>
			</div>
		</li>`;
		const itemsList = cartContainer ? cartContainer.querySelector('.items') : null;
		if (itemsList) {
			itemsList.insertAdjacentHTML('beforeend', html);
			_processContent(itemsList);
		}
		Client.loadFile(DB.INTERFACE_PATH + 'collection/' + it.identifiedResourceName + '.bmp', function (data) {
			const imgEl = cartContainer
				? cartContainer.querySelector(`.item[data-index="${itemId}"] .item-dt-img`)
				: null;
			if (imgEl) imgEl.style.backgroundImage = `url(${data})`;
		});
	} else {
		itemCart.amount += amount;
		const cntEl = cartContainer
			? cartContainer.querySelector(`.items .item[data-index="${itemId}"] .item-cnt`)
			: null;
		if (cntEl) cntEl.textContent = itemCart.amount;
	}
	updateCartTotal();
}

/**
 * Add cash item in cart list
 */
function onClickActionAddCartItem(e) {
	const btn = e.target.closest('.purchase-btn-container button');
	if (!btn) return;
	const itemId = parseInt(btn.dataset.itemid);
	addItemToCart(itemId);
}

/**
 * purchase item list in cart
 */
function onClickActionBuyItem() {
	const root = _root();
	const itemlist = CashShop.cartItem;
	CashShop.cartItemLen = itemlist.length;

	UIManager.showPromptBox('Are you sure you want to buy this items?', 'ok', 'cancel', () => {
		if (CashShop.cartItem.length > 0) {
			const pkt = new PACKET.CZ.SE_PC_BUY_CASHITEM_LIST();
			const freePointsInput = root.querySelector('#use-free-points');
			const useFreePoints = (freePointsInput ? freePointsInput.value : 0) || 0;
			if (useFreePoints >= 0 && useFreePoints <= CashShop.kafraPoints) {
				pkt.kafraPoints = useFreePoints;
				pkt.item_list = CashShop.cartItem;
				Network.sendPacket(pkt);
			} else {
				UIManager.showMessageBox('You dont have enough Kafra Points!', 'ok');
			}
		} else {
			UIManager.showMessageBox('No item in cart!', 'ok');
		}
	});
}

/**
 * Menu navigation
 */
function onClickMenu(target) {
	const root = _root();
	const selectedMenu = target.dataset.index.toUpperCase();

	const searchInput = root.querySelector('#cashshop-search');
	if (searchInput) searchInput.value = '';

	if (selectedMenu !== CashShop.activeCashMenu) {
		CashShop.isSearch = false;
		CashShop.activeCashMenu = selectedMenu;
		const panelItems = root.querySelector('.panel-items');
		if (panelItems) panelItems.innerHTML = '';
		root.querySelectorAll('#panel-menu .tab').forEach(el => el.classList.remove('active'));
		target.classList.add('active');

		const tab_items =
			CashShop.cashShopListItem[CashShop.activeCashMenu]?.items?.length >= 0
				? CashShop.cashShopListItem[CashShop.activeCashMenu].items
				: [];
		CashShop.initPagination(tab_items);
		CashShop.renderCashShopItems(CashShop.paginate(tab_items, CashShop.pageOffset, CashShop.pageEnd));
	}
}

/**
 * Hide the item name
 */
function onItemOut() {
	const overlay = _root().querySelector('.overlay');
	if (overlay) overlay.style.display = 'none';
}

function onMouseMoveTab(event) {
	const root = _root();
	const overlay = root.querySelector('.overlay');
	if (!overlay) return;
	const hostRect = CashShop._host.getBoundingClientRect();
	const x = event.clientX - hostRect.left;
	const y = event.clientY - hostRect.top - 30;

	Object.assign(overlay.style, {
		top: y + 'px',
		left: x + 'px'
	});
}

function onMouseOverTab(target) {
	const title = target.getAttribute('data-title');
	const overlay = _root().querySelector('.overlay');
	if (overlay) {
		overlay.textContent = title;
		overlay.style.display = 'block';
	}
}

function onMouseOutTab() {
	const overlay = _root().querySelector('.overlay');
	if (overlay) overlay.style.display = 'none';
}

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = CashShop.getItemByIndex(index);

	if (!item) {
		return;
	}

	// Set image to the drag drop element
	const img = new Image();
	const imgEl = this.querySelector('.item-left-img');
	const url = imgEl ? imgEl.style.backgroundImage.match(/\(([^)]+)/)?.[1] : null;
	if (url) {
		img.decoding = 'async';
		img.src = url.replace(/^"/, '').replace(/"$/, '');
		event.dataTransfer.setDragImage(img, 12, 12);
	}

	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'CashShop',
				data: item
			})
		)
	);

	onItemOut();
}

/**
 * Stop dragging an item
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = CashShop.getItemByIndex(index);

	if (!item) {
		return false;
	}

	// Don't add the same UI twice, remove it
	if (ItemInfo.uid === item.itemId) {
		ItemInfo.remove();
		return false;
	}

	const it = DB.getItemInfo(item.itemId);
	it.ITID = item.itemId;
	it.IsIdentified = true;

	// Add ui to window
	ItemInfo.append();
	ItemInfo.uid = it.ITID;
	ItemInfo.setItem(it);

	return false;
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
	return false;
}
export default UIManager.addComponent(CashShop);

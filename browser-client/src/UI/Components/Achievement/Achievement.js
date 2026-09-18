/**
 * UI/Components/Achievement/Achievement.js
 *
 * Achievement Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */
import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import 'UI/Elements/Elements.js';

import Preferences from 'Core/Preferences.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Session from 'Engine/SessionStorage.js';
import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';

import htmlText from './Achievement.html?raw';
import cssText from './Achievement.css?raw';

const _preferences = Preferences.get(
	'Achievement',
	{
		x: 100,
		y: 100
	},
	1.0
);

let MAJOR_CATEGORIES = [];

class AchievementComponent extends GUIComponent {
	constructor() {
		super('Achievement', cssText);
	}

	render() {
		return htmlText;
	}

	init() {
		const root = this.getRoot();

		MAJOR_CATEGORIES = [
			{ id: 0, name: DB.getMessage(2656) },
			{
				id: 1,
				name: DB.getMessage(2657),
				minorCategories: [
					{ id: 0, name: DB.getMessage(2658) },
					{ id: 1, name: DB.getMessage(2659) },
					{ id: 2, name: DB.getMessage(2660) }
				]
			},
			{
				id: 2,
				name: DB.getMessage(2661),
				minorCategories: [
					{ id: 0, name: DB.getMessage(2662) },
					{ id: 1, name: DB.getMessage(2663) },
					{ id: 2, name: DB.getMessage(2664) },
					{ id: 3, name: DB.getMessage(2665) },
					{ id: 4, name: DB.getMessage(2666) },
					{ id: 5, name: DB.getMessage(2667) }
				]
			},
			{
				id: 3,
				name: DB.getMessage(2668),
				minorCategories: [
					{ id: 0, name: DB.getMessage(2669) },
					{ id: 1, name: DB.getMessage(2670) }
				]
			},
			{
				id: 4,
				name: DB.getMessage(2671),
				minorCategories: [
					{ id: 0, name: DB.getMessage(2672) },
					{ id: 1, name: DB.getMessage(2673) }
				]
			},
			{
				id: 5,
				name: DB.getMessage(2674),
				minorCategories: [
					{ id: 0, name: DB.getMessage(2675) },
					{ id: 1, name: DB.getMessage(2676) },
					{ id: 2, name: DB.getMessage(2677) }
				]
			},
			{ id: 6, name: DB.getMessage(2678), minorCategories: [] }
		];

		this.currentFilter = 'complete'; // all, complete, incomplete
		this.currentMajor = 0;
		this.currentMinor = -1;
		this.selectedAchId = null;

		this.draggable('.titlebar');

		const closeBtn = root.querySelector('.close');
		if (closeBtn) {
			closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
			closeBtn.addEventListener('click', () => this.toggle());
		}

		// Bind filters
		const filters = root.querySelectorAll('.js-filters .radio');
		filters.forEach(el => {
			el.addEventListener('click', () => {
				const filter = el.dataset.filter;
				if (this.currentFilter === filter)
					this.currentFilter = 'all'; // toggle off
				else this.currentFilter = filter;
				this.renderList();
			});
		});

		// Bind claim
		const claimBtn = root.querySelector('.js-d-claim');
		if (claimBtn) {
			claimBtn.addEventListener('click', () => {
				if (this.selectedAchId !== null) {
					const pkt = new PACKET.CZ.REQ_ACH_REWARD();
					pkt.ach_id = this.selectedAchId;
					Network.sendPacket(pkt);
				}
			});
		}

		this.renderSidebar();
		this._host.style.display = 'none';
	}

	onAppend() {
		this._host.style.left = `${_preferences.x}px`;
		this._host.style.top = `${_preferences.y}px`;
		this._fixPositionOverflow();

		this.updateHeaderAndView();
	}

	onRemove() {
		_preferences.x = parseInt(this._host.style.left, 10);
		_preferences.y = parseInt(this._host.style.top, 10);
		_preferences.save();
	}

	toggle() {
		if (this.__active && this._host.style.display !== 'none') {
			this._host.style.display = 'none';
		} else {
			this.append();
			this._host.style.display = '';
			this._fixPositionOverflow();
			this.updateHeaderAndView();
		}
	}

	onShortCut(key) {
		if (key.cmd === 'TOGGLE') this.toggle();
	}

	updateHeaderAndView() {
		if (this._host.style.display === 'none') return;

		const root = this.getRoot();
		const ach = Session.Achievement || { rank: 0, total_points: 0, list: {} };

		root.querySelector('.js-rank').textContent = ach.rank;
		root.querySelector('.js-points').textContent = ach.total_points;

		const currentPoints = ach.current_rank_points || 0;
		const nextPoints = ach.next_rank_points || 0;
		const ratio = nextPoints > 0 ? Math.min(100, (currentPoints / nextPoints) * 100) : currentPoints > 0 ? 100 : 0;

		root.querySelector('.js-rank-progress-text').textContent = `${currentPoints}/${nextPoints}`;
		root.querySelector('.js-rank-progress-fill').style.width = `${ratio}%`;

		const catProgressEl = root.querySelector('.js-cat-progress');
		if (this.currentMajor === 0) {
			catProgressEl.style.display = 'none';
			this.renderOverview();
		} else {
			catProgressEl.style.display = 'block';

			let total = 0;
			let completed = 0;
			const allAch = DB.getAchievementTable();
			const sessAch = Session.Achievement && Session.Achievement.list ? Session.Achievement.list : {};

			Object.keys(allAch).forEach(achId => {
				const info = allAch[achId];
				if (!info || info.major !== this.currentMajor) return;
				if (this.currentMinor !== -1 && info.minor !== this.currentMinor) return;
				total++;
				if (sessAch[achId] && sessAch[achId].completed) completed++;
			});

			let catName = '';
			const majorObj = MAJOR_CATEGORIES.find(m => m.id === this.currentMajor);
			if (majorObj) {
				if (this.currentMinor !== -1 && majorObj.minorCategories) {
					const minorObj = majorObj.minorCategories.find(m => m.id === this.currentMinor);
					if (minorObj) catName = minorObj.name;
				} else {
					catName = majorObj.name;
				}
			}

			root.querySelector('.js-cat-name').textContent = catName;
			root.querySelector('.js-cat-progress-text').textContent = `(${completed}/${total})`;
			const catRatio = total > 0 ? (completed / total) * 100 : 0;
			root.querySelector('.js-cat-progress-fill').style.width = `${catRatio}%`;

			this.renderList();
		}
	}

	renderSidebar() {
		const root = this.getRoot();
		const sidebar = root.querySelector('.js-sidebar');
		sidebar.innerHTML = '';

		MAJOR_CATEGORIES.forEach(major => {
			const mDiv = document.createElement('div');
			mDiv.className = 'major-tab' + (this.currentMajor === major.id ? ' active' : '');

			mDiv.setAttribute('data-background', 'achievement_re/tab_out.bmp');
			mDiv.setAttribute('data-hover', 'achievement_re/tab_over.bmp');
			mDiv.setAttribute('data-down', 'achievement_re/tab_press.bmp');
			mDiv.setAttribute('data-active', 'achievement_re/tab_press.bmp');
			GUIComponent.processDataAttrs(mDiv);

			const tNode = document.createTextNode(major.name);
			mDiv.appendChild(tNode);

			mDiv.addEventListener('click', () => {
				this.currentMajor = major.id;
				this.currentMinor = -1;
				this.selectedAchId = null;
				this.renderSidebar();
				this.updateHeaderAndView();
			});
			sidebar.appendChild(mDiv);

			if (this.currentMajor === major.id && major.minorCategories && major.minorCategories.length > 0) {
				const minorContainer = document.createElement('div');
				minorContainer.className = 'minor-tabs';
				major.minorCategories.forEach(minor => {
					const minDiv = document.createElement('div');
					minDiv.className = 'minor-tab' + (this.currentMinor === minor.id ? ' active' : '');

					minDiv.setAttribute('data-background', 'achievement_re/tab_sub_out.bmp');
					minDiv.setAttribute('data-hover', 'achievement_re/tab_sub_over.bmp');
					minDiv.setAttribute('data-down', 'achievement_re/tab_sub_press.bmp');
					minDiv.setAttribute('data-active', 'achievement_re/tab_sub_press.bmp');
					GUIComponent.processDataAttrs(minDiv);

					minDiv.textContent = minor.name;
					minDiv.addEventListener('click', () => {
						this.currentMinor = minor.id;
						this.selectedAchId = null;
						this.renderSidebar();
						this.updateHeaderAndView();
					});
					minorContainer.appendChild(minDiv);
				});
				sidebar.appendChild(minorContainer);
			}
		});
	}

	renderOverview() {
		const root = this.getRoot();
		root.querySelector('.bg-list').style.display = 'none';
		root.querySelector('.bg-summary').style.display = 'block';
		root.querySelector('.js-pane-listd').style.display = 'none';
		root.querySelector('.js-filters').style.display = 'none';
		root.querySelector('.js-pane-overview').style.display = 'block';

		const container = root.querySelector('.js-overview-container');
		container.innerHTML = '';

		const allAch = DB.getAchievementTable();
		const sessAch = Session.Achievement && Session.Achievement.list ? Session.Achievement.list : {};

		// Pre-calculate per-major category and total completed achievements
		const counts = {};
		let totalCompleted = 0;
		MAJOR_CATEGORIES.forEach(m => {
			if (m.id !== 0) counts[m.id] = { total: 0, completed: 0, name: m.name };
		});

		Object.keys(allAch).forEach(achId => {
			const info = allAch[achId];
			if (info && counts[info.major] !== undefined) {
				counts[info.major].total++;
				const s = sessAch[achId];
				if (s && s.completed) {
					counts[info.major].completed++;
					totalCompleted++;
				}
			}
		});

		// render completed/total gauge
		const totalGauge = document.createElement('div');
		totalGauge.className = 'category-gauge category-total';
		const totalRatio = totalCompleted > 0 ? (totalCompleted / Object.keys(allAch).length) * 100 : 0;
		totalGauge.innerHTML = `
		<div class="category-total-text">
			<div class="name">${DB.getMessage(2646)}</div>
			<div class="progress-text">${totalCompleted}/${Object.keys(allAch).length}</div>
		</div>
		<div class="category-total-progress">
			<div class="bar-bg">
				<div class="bar-fill" style="width: ${totalRatio}%"></div>
			</div>
		</div>
	`;
		container.appendChild(totalGauge);

		// render category gauges
		Object.values(counts).forEach(c => {
			const gauge = document.createElement('div');
			gauge.className = 'category-gauge category-' + c.name.toLowerCase();

			const ratio = c.total > 0 ? (c.completed / c.total) * 100 : 0;

			gauge.innerHTML = `
			<div class="category-text">
				<div class="name">${c.name}</div>
				<div class="progress-text">${c.completed}/${c.total}</div>
			</div>
			<div class="category-progress">
				<div class="bar-bg">
					<div class="bar-fill" style="width: ${ratio}%"></div>
				</div>
			</div>
		`;
			container.appendChild(gauge);
		});

		// Recent completed achievements
		const completedList = [];
		Object.keys(sessAch).forEach(achId => {
			if (sessAch[achId] && sessAch[achId].completed) {
				completedList.push({ achId, s: sessAch[achId] });
			}
		});
		completedList.sort((a, b) => b.s.completed_at - a.s.completed_at);
		const recentList = completedList.slice(0, 2);

		if (recentList.length > 0) {
			const recentContainer = document.createElement('div');
			recentContainer.className = 'recent-achievements';

			const recentTitle = document.createElement('div');
			recentTitle.className = 'recent-title';
			recentTitle.textContent = DB.getMessage(2651);
			recentContainer.appendChild(recentTitle);

			const recentItems = document.createElement('div');
			recentItems.className = 'recent-items';

			recentList.forEach(itemData => {
				const { achId, s } = itemData;
				const info = allAch[achId] || {};

				const item = document.createElement('div');
				item.className = 'ach-item ach-item-overview';
				item.setAttribute('data-background', 'achievement_re/list_complete_out.bmp');
				item.setAttribute('data-hover', 'achievement_re/list_complete_press.bmp');
				item.setAttribute('data-down', 'achievement_re/list_complete_press.bmp');
				item.setAttribute('data-active', 'achievement_re/list_complete_press.bmp');

				let dtStr = '';
				let rewardBoxBg = '';
				const groupName = info.group ? info.group.toLowerCase() : '';
				const hasReward =
					info.reward &&
					(Array.isArray(info.reward) ? info.reward.length > 0 : Object.keys(info.reward).length > 0);
				const isClaimed = s && s.reward;
				const d = new Date(s.completed_at * 1000);
				dtStr = `${d.getFullYear()}.${d.getMonth() + 1}.${d.getDate()}`;

				if (hasReward) {
					if (!isClaimed) {
						rewardBoxBg = 'achievement_re/list_rewardbox_not_receive.bmp';
					} else {
						rewardBoxBg = '';
					}
				}

				item.innerHTML = `
					<div class="icon" data-background="achievement_re/icon_${groupName}.bmp"></div>
					<div class="title">${info.title || 'Unknown'}</div>
					<div class="desc">${info.content && info.content.summary ? info.content.summary : ''}</div>
					<div class="reward-icon" data-background="${isClaimed ? 'achievement_re/badge_complete.bmp' : ''}"></div>
					<div class="reward-pts">${info.score || 0}</div>
					<div class="date-completed" data-background="${rewardBoxBg}">${dtStr}</div>
				`;

				GUIComponent.processDataAttrs(item);
				item.querySelectorAll(
					'[data-background],[data-hover],[data-down],[data-active],[data-text],[data-preload]'
				).forEach(node => GUIComponent.processDataAttrs(node));

				recentItems.appendChild(item);
			});
			recentContainer.appendChild(recentItems);
			container.appendChild(recentContainer);
		}
	}

	renderList() {
		const root = this.getRoot();
		root.querySelector('.bg-list').style.display = 'block';
		root.querySelector('.bg-summary').style.display = 'none';
		root.querySelector('.js-pane-overview').style.display = 'none';
		root.querySelector('.js-pane-listd').style.display = 'block';
		root.querySelector('.js-filters').style.display = 'block';

		// Update filter radio buttons
		root.querySelectorAll('.js-filters .radio').forEach(el => {
			const img = el.querySelector('ui-image');
			if (el.dataset.filter === this.currentFilter) {
				img.setAttribute('src', 'achievement_re/btn_radio_on.bmp');
			} else {
				img.setAttribute('src', 'achievement_re/btn_radio_off.bmp');
			}
		});

		const listContainer = root.querySelector('.js-list-container');
		if (listContainer) listContainer.scrollTop = 0;

		const listEl = root.querySelector('.js-list');
		listEl.innerHTML = '';

		const allAch = DB.getAchievementTable();
		const sessAch = Session.Achievement && Session.Achievement.list ? Session.Achievement.list : {};

		const list = [];
		Object.keys(allAch).forEach(achId => {
			const info = allAch[achId];
			if (!info || info.major !== this.currentMajor) return;
			if (this.currentMinor !== -1 && info.minor !== this.currentMinor) return;

			const s = sessAch[achId];
			const isCompleted = s ? s.completed : false;

			list.push({ achId, info, s, isCompleted });
		});

		if (this.currentFilter === 'complete') {
			list.sort((a, b) => (b.isCompleted === a.isCompleted ? a.achId - b.achId : b.isCompleted ? 1 : -1));
		} else if (this.currentFilter === 'incomplete') {
			list.sort((a, b) => (b.isCompleted === a.isCompleted ? a.achId - b.achId : a.isCompleted ? 1 : -1));
		} else {
			list.sort((a, b) => a.achId - b.achId);
		}

		list.forEach(itemData => {
			const { achId, info, s, isCompleted } = itemData;

			if (this.selectedAchId === null) {
				this.selectedAchId = parseInt(achId, 10);
			}

			const item = document.createElement('div');
			item.className = 'ach-item' + (this.selectedAchId === parseInt(achId, 10) ? ' selected' : '');
			if (isCompleted) {
				item.setAttribute('data-background', 'achievement_re/list_complete_out.bmp');
				item.setAttribute('data-hover', 'achievement_re/list_complete_press.bmp');
				item.setAttribute('data-down', 'achievement_re/list_complete_press.bmp');
				item.setAttribute('data-active', 'achievement_re/list_complete_press.bmp');
			} else {
				item.setAttribute('data-background', 'achievement_re/list_out.bmp');
				item.setAttribute('data-hover', 'achievement_re/list_press.bmp');
				item.setAttribute('data-down', 'achievement_re/list_press.bmp');
				item.setAttribute('data-active', 'achievement_re/list_press.bmp');
			}

			let dtStr = '';
			let rewardBoxBg = '';

			const groupName = info.group ? info.group.toLowerCase() : '';
			const isClaimed = s && s.reward;
			const hasReward =
				info.reward &&
				(Array.isArray(info.reward) ? info.reward.length > 0 : Object.keys(info.reward).length > 0);

			if (hasReward) {
				if (!isCompleted) {
					rewardBoxBg = 'achievement_re/list_rewardbox_default.bmp';
				} else if (!isClaimed) {
					rewardBoxBg = 'achievement_re/list_rewardbox_not_receive.bmp';
				} else {
					rewardBoxBg = '';
				}
			}

			if (isCompleted) {
				const d = new Date(s.completed_at * 1000);
				dtStr = `${d.getFullYear()}.${d.getMonth() + 1}.${d.getDate()}`;
			}

			item.innerHTML = `
			<div class="icon" data-background="achievement_re/icon_${groupName}.bmp"></div>
			<div class="title">${info.title || 'Unknown'}</div>
			<div class="desc">${info.content && info.content.summary ? info.content.summary : ''}</div>
			<div class="reward-icon" data-background="${isCompleted ? 'achievement_re/badge_complete.bmp' : ''}"></div>
			<div class="reward-pts">${info.score || 0}</div>
			<div class="date-completed" data-background="${rewardBoxBg}">${dtStr}</div>
		`;

			GUIComponent.processDataAttrs(item);
			item.querySelectorAll(
				'[data-background],[data-hover],[data-down],[data-active],[data-text],[data-preload]'
			).forEach(node => GUIComponent.processDataAttrs(node));

			item.addEventListener('click', () => {
				this.selectedAchId = parseInt(achId, 10);
				listEl.querySelectorAll('.ach-item').forEach(el => el.classList.remove('selected'));
				item.classList.add('selected');
				this.renderDetail();
			});
			listEl.appendChild(item);
		});

		this.renderDetail();
	}

	renderDetail() {
		const root = this.getRoot();
		const allAch = DB.getAchievementTable();
		const sessAch = Session.Achievement && Session.Achievement.list ? Session.Achievement.list : {};

		const info = allAch[this.selectedAchId];
		const s = sessAch[this.selectedAchId];

		// show/hide reward images based on reward type
		const rewardTitle = root.querySelector('.reward-title');
		const rewardBuff = root.querySelector('.reward-buff');
		const rewardItem = root.querySelector('.reward-item');
		const overlay = root.querySelector('.js-overlay');

		if (!info) {
			root.querySelector('.js-d-title').textContent = 'Select an achievement';
			root.querySelector('.js-d-desc').textContent = '';
			root.querySelector('.js-d-goals').innerHTML = '';
			root.querySelector('.js-d-claim').style.display = 'none';
			rewardTitle.style.display = 'none';
			rewardBuff.style.display = 'none';
			rewardItem.style.display = 'none';

			return;
		}

		root.querySelector('.js-d-title').textContent = info.title || 'Unknown';
		root.querySelector('.js-d-desc').textContent = info.content && info.content.details ? info.content.details : '';
		const dStamp = root.querySelector('.d-stamp');
		// if completed display stamp on right side else empty
		if (s && s.completed) {
			dStamp.style.display = 'block';
		} else {
			dStamp.style.display = 'none';
		}
		const goalsEl = root.querySelector('.js-d-goals');
		goalsEl.innerHTML = '';

		if (info.resource) {
			Object.values(info.resource).forEach((res, idx) => {
				if (res.text) {
					const g = document.createElement('div');
					g.className = 'd-goal-item';
					g.textContent = res.text;
					goalsEl.appendChild(g);
				}
			});
		}

		// Clear previous events
		rewardTitle.onmouseover = rewardTitle.onmouseout = rewardTitle.onmousemove = null;
		rewardBuff.onmouseover = rewardBuff.onmouseout = rewardBuff.onmousemove = null;
		rewardItem.onmouseover = rewardItem.onmouseout = rewardItem.onmousemove = null;

		const bindOverlay = (el, text) => {
			el.onmouseover = () => {
				if (!text) return;
				overlay.textContent = text;
				overlay.style.display = 'block';

				const hostRect = this._host.getBoundingClientRect();
				const elRect = el.getBoundingClientRect();

				overlay.style.left =
					Math.max(0, elRect.left - hostRect.left + (elRect.width - overlay.offsetWidth) / 2) + 'px';
				overlay.style.top = elRect.top - hostRect.top - 20 + 'px';
			};
			el.onmouseout = () => {
				overlay.style.display = 'none';
			};
		};

		if (info.reward) {
			rewardTitle.style.display = info.reward.title ? 'block' : 'none';
			if (info.reward.title) bindOverlay(rewardTitle, DB.getTitleString(info.reward.title));

			rewardBuff.style.display = info.reward.buff ? 'block' : 'none';
			if (info.reward.buff) bindOverlay(rewardBuff, DB.getSkillName(info.reward.buff));

			rewardItem.style.display = info.reward.item ? 'block' : 'none';
			if (info.reward.item) {
				const it = DB.getItemInfo(info.reward.item);
				if (it && it.identifiedResourceName) {
					Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', function (data) {
						root.querySelector('.d-reward-item').style.backgroundImage = 'url(' + data + ')';
					});
					bindOverlay(rewardItem, DB.getItemName({ ITID: info.reward.item, IsIdentified: true }));

					rewardItem.oncontextmenu = event => {
						event.preventDefault();
						event.stopImmediatePropagation();

						if (ItemInfo.uid === info.reward.item) {
							ItemInfo.remove();
						}

						ItemInfo.append();
						ItemInfo.uid = info.reward.item;
						ItemInfo.setItem({ ITID: info.reward.item, IsIdentified: true, count: 1 });
						return false;
					};
				}
			}
		} else {
			rewardTitle.style.display = 'none';
			rewardBuff.style.display = 'none';
			rewardItem.style.display = 'none';
		}

		const claimBtn = root.querySelector('.js-d-claim');
		const hasReward =
			info.reward && (Array.isArray(info.reward) ? info.reward.length > 0 : Object.keys(info.reward).length > 0);
		if (hasReward && s && s.completed && !s.reward) {
			claimBtn.style.display = '';
		} else {
			claimBtn.style.display = 'none';
		}
	}
}

const Achievement = new AchievementComponent();
export default UIManager.addComponent(Achievement);

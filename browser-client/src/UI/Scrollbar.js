/**
 * UI/Scrollbar.js
 *
 * Manage custom scrollbar
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Texture from 'Utils/Texture.js';
import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';

/**
 * ScrollBar Namespace
 */
class ScrollBar {
	/**
	 * @var {boolean} does the scrollbar completely loaded ?
	 */
	static complete = false;

	/**
	 * @var {Object} skins library
	 */
	static skins = {};

	/**
	 * Initialize scrollbar
	 */
	static init() {
		// Already loaded
		if (ScrollBar.complete) {
			return;
		}

		// List of skins to load
		const skinsToLoad = [
			{
				name: 'default',
				files: [
					`${DB.INTERFACE_PATH}scroll0down.bmp`,
					`${DB.INTERFACE_PATH}scroll0mid.bmp`,
					`${DB.INTERFACE_PATH}scroll0up.bmp`,
					`${DB.INTERFACE_PATH}scroll0bar_down.bmp`,
					`${DB.INTERFACE_PATH}scroll0bar_mid.bmp`,
					`${DB.INTERFACE_PATH}scroll0bar_up.bmp`
				]
			},
			{
				name: 'chatbox',
				files: [
					`${DB.INTERFACE_PATH}basic_interface/dialscr_down.bmp`,
					null,
					`${DB.INTERFACE_PATH}basic_interface/dialscr_up.bmp`,
					null,
					null,
					null
				],
				colors: {
					track: '#000000',
					thumb: '#9a9a9a'
				},
				width: 10,
				btnHeight: 10,
				btnWidth: 10,
				trackWidth: 9
			}
		];

		let loadedCount = 0;

		skinsToLoad.forEach(skinInfo => {
			const files = skinInfo.files.filter(f => {
				return f !== null;
			});

			Client.loadFiles(files, function () {
				const args = arguments;
				const down = args[0];
				const mid = skinInfo.name === 'default' ? args[1] : null;
				const up = skinInfo.name === 'default' ? args[2] : args[1];

				const baseDown = skinInfo.name === 'default' ? args[3] : null;
				const baseMid = skinInfo.name === 'default' ? args[4] : null;
				const baseUp = skinInfo.name === 'default' ? args[5] : null;

				const finalizeSkin = function (thumbUrl) {
					ScrollBar.skins[skinInfo.name] = {
						name: skinInfo.name,
						down: down,
						up: up,
						mid: mid,
						thumb: thumbUrl,
						colors: skinInfo.colors || {},
						width: skinInfo.width,
						btnHeight: skinInfo.btnHeight,
						btnWidth: skinInfo.btnWidth,
						trackWidth: skinInfo.trackWidth
					};

					loadedCount++;
					if (loadedCount === skinsToLoad.length) {
						setupStyles();
						ScrollBar.complete = true;
					}
				};

				if (baseUp) {
					// Use a small helper to avoid deep nesting of Texture.load calls
					const loadedTextures = { down: null, mid: null, up: null };
					const checkAllLoaded = function () {
						if (loadedTextures.down && loadedTextures.mid && loadedTextures.up) {
							const base = document.createElement('canvas');
							const ctx = base.getContext('2d');
							const imgUp = loadedTextures.up;
							const imgMid = loadedTextures.mid;
							const imgDown = loadedTextures.down;

							base.width = imgUp.width;
							base.height = imgUp.height + imgMid.height + imgDown.height;

							ctx.drawImage(imgUp, 0, 0);
							ctx.drawImage(imgMid, 0, imgUp.height);
							ctx.drawImage(imgDown, 0, imgUp.height + imgMid.height);

							finalizeSkin(base.toDataURL());
						}
					};

					Texture.load(baseDown, function () {
						loadedTextures.down = this;
						checkAllLoaded();
					});
					Texture.load(baseMid, function () {
						loadedTextures.mid = this;
						checkAllLoaded();
					});
					Texture.load(baseUp, function () {
						loadedTextures.up = this;
						checkAllLoaded();
					});
				} else {
					finalizeSkin(null);
				}
			});
		});

		function setupStyles() {
			const css = [
				'.ro-custom-scrollbar { position: absolute; right: 0; top: 0; width: 13px; z-index: 100; display: flex; flex-direction: column; }',
				'.ro-custom-scrollbar .btn-up { height: 12px; background-repeat: no-repeat; cursor: pointer; }',
				'.ro-custom-scrollbar .btn-down { height: 13px; background-repeat: no-repeat; cursor: pointer; }',
				'.ro-custom-scrollbar .track { flex: 1; background-repeat: repeat-y; position: relative; cursor: pointer; }',
				'.ro-custom-scrollbar .thumb { position: absolute; top: 0; left: 0; width: 100%; min-height: 10px; cursor: pointer; border-color: transparent; border-style: solid; border-width: 4px 0; box-sizing: border-box; }'
			].join('\n');

			ScrollBar._cssText = css;
			const firstStyle = document.querySelector('style');
			if (firstStyle) {
				firstStyle.textContent += css;
			}
		}
	}

	/**
	 * Apply custom scrollbar to an element
	 * @param {HTMLElement} element
	 */
	static applyDOMScrollbar(element) {
		if (element._roScrollbarApplied) {
			const wrapper = element.querySelector(':scope > .ro-custom-scrollbar');
			if (!wrapper) {
				element._roScrollbarApplied = false;
			} else {
				const currentSkinName = element.dataset.scrollbarSkin || 'default';
				if (element._roScrollbarSkin !== currentSkinName) {
					// Skin changed, need to re-apply visuals
					element._roScrollbarApplied = false;
					wrapper.parentNode.removeChild(wrapper);
				} else {
					if (element._roScrollbarRestart) {
						element._roScrollbarRestart();
					}
					return;
				}
			}
		}

		if (!ScrollBar.complete) {
			setTimeout(() => {
				ScrollBar.applyDOMScrollbar(element);
			}, 100);
			return;
		}

		element._roScrollbarApplied = true;
		const shadowRoot = element.getRootNode();
		if (shadowRoot instanceof ShadowRoot && !shadowRoot.querySelector('style[data-scrollbar]')) {
			const scrollbarStyle = document.createElement('style');
			scrollbarStyle.setAttribute('data-scrollbar', '');
			scrollbarStyle.textContent = ScrollBar._cssText;
			shadowRoot.appendChild(scrollbarStyle);
		}

		const skinName = element.dataset.scrollbarSkin || 'default';
		const skin = ScrollBar.skins[skinName] || ScrollBar.skins['default'];

		element._roScrollbarSkin = skinName;

		const computedStyle = getComputedStyle(element);
		if (computedStyle.position === 'static') {
			element.style.position = 'relative';
		}

		// Ensure native scrollbar is removed and layout is preserved
		if (element._roOriginalPaddingRight === undefined) {
			element._roOriginalPaddingRight = parseInt(computedStyle.paddingRight) || 0;
		}

		element.style.overflowY = 'hidden';
		element.style.boxSizing = 'border-box';

		// Build scrollbar DOM wrapper
		const scrollbar = document.createElement('div');
		scrollbar.className = `ro-custom-scrollbar skin-${skinName}`;
		const upBtn = document.createElement('div');
		upBtn.className = 'btn-up';
		const track = document.createElement('div');
		track.className = 'track';
		const thumb = document.createElement('div');
		thumb.className = 'thumb';
		const downBtn = document.createElement('div');
		downBtn.className = 'btn-down';

		const width = skin.width || 13;
		scrollbar.style.width = `${width}px`;

		if (skin.btnHeight || skin.btnWidth) {
			const bHeight = skin.btnHeight || (skin.name === 'default' ? 12 : 13);
			const bWidth = skin.btnWidth || width;
			Object.assign(upBtn.style, { height: `${bHeight}px`, width: `${bWidth}px`, margin: '0 auto' });
			Object.assign(downBtn.style, { height: `${bHeight}px`, width: `${bWidth}px`, margin: '0 auto' });
		}

		track.appendChild(thumb);
		scrollbar.appendChild(upBtn);
		scrollbar.appendChild(track);
		scrollbar.appendChild(downBtn);
		element.appendChild(scrollbar);

		// Prevent clicks and interactions from passing through to the game world
		const stopEvents = [
			'mousedown',
			'mouseup',
			'click',
			'dblclick',
			'contextmenu',
			'pointerdown',
			'pointerup',
			'pointermove',
			'wheel'
		];
		for (const evtName of stopEvents) {
			scrollbar.addEventListener(evtName, e => {
				e.stopPropagation();
			});
		}

		// Apply background styles from skin
		Object.assign(upBtn.style, { backgroundImage: `url(${skin.up})`, backgroundColor: 'transparent' });
		Object.assign(downBtn.style, { backgroundImage: `url(${skin.down})`, backgroundColor: 'transparent' });

		if (skin.mid) {
			Object.assign(track.style, { backgroundImage: `url(${skin.mid})`, backgroundColor: 'transparent' });
		} else {
			Object.assign(track.style, {
				backgroundImage: 'none',
				backgroundColor: skin.colors.track || 'transparent',
				border: 'none',
				width: `${skin.trackWidth || width}px`,
				margin: '0 auto'
			});
		}

		if (skin.thumb) {
			Object.assign(thumb.style, {
				webkitBorderImage: `url(${skin.thumb}) 4 0 4 0 fill`,
				borderImage: `url(${skin.thumb}) 4 0 4 0 fill`,
				backgroundColor: 'transparent'
			});
		} else {
			const tWidth = skin.trackWidth || width;
			Object.assign(thumb.style, {
				backgroundColor: skin.colors.thumb || 'grey',
				webkitBorderImage: 'none',
				borderImage: 'none',
				width: `${tWidth}px`
			});
		}

		let isDragging = false;
		let startY = 0;
		let startThumbY = 0;

		/**
		 * Update thumb position relative to scroll position.
		 * Reads scrollTop/scrollHeight/clientHeight and writes only the thumb,
		 * wrapper and paddingRight styles — it never writes element.scrollTop,
		 * so a 'scroll' listener calling this cannot trigger a re-entrant scroll.
		 */
		const updateThumb = () => {
			const h = element.clientHeight;
			const sh = element.scrollHeight;

			if (sh <= h) {
				scrollbar.style.display = 'none';
				element.style.paddingRight = `${element._roOriginalPaddingRight}px`;
				return;
			}

			scrollbar.style.display = '';
			element.style.paddingRight = `${element._roOriginalPaddingRight + (skin.width || 13)}px`;

			const st = element.scrollTop;

			// Sync wrapper size and position
			Object.assign(scrollbar.style, {
				top: `${st}px`,
				height: `${h}px`,
				right: '0px'
			});

			const trackHeight = track.clientHeight;
			if (trackHeight <= 0) {
				return;
			}

			const ratio = h / sh;
			const thumbHeight = Math.max(10, Math.floor(trackHeight * ratio));
			thumb.style.height = `${thumbHeight}px`;

			const maxScrollTop = sh - h;
			const maxThumbTop = trackHeight - thumbHeight;
			const thumbTop = (st / maxScrollTop) * maxThumbTop;

			thumb.style.top = `${thumbTop}px`;
		};

		let poller = null;

		element._roScrollbarRestart = () => {
			updateThumb();
			if (poller) {
				clearInterval(poller);
			}
			poller = setInterval(() => {
				if (!element.isConnected) {
					clearInterval(poller);
					poller = null;
					return;
				}
				updateThumb();
			}, 300);
		};

		// Start tracking content height
		element._roScrollbarRestart();

		// Sync the thumb on any scroll offset change, including programmatic
		// scrollTop writes (keyboard navigation, auto-scroll-to-bottom, resets).
		// An overflow:hidden element still fires 'scroll' when scrollTop changes,
		// so this removes the up-to-300ms poller lag for driven scrolling.
		// A skin change re-runs this body with a fresh updateThumb closure, so
		// drop the previous handler before re-binding — otherwise a second
		// listener tied to the now-detached wrapper would stack up. (The sibling
		// wheel/button listeners bound to `element` below predate this and still
		// leak this way on skin change; left as-is to keep the fix scoped.)
		if (element._roScrollHandler) {
			element.removeEventListener('scroll', element._roScrollHandler);
		}
		element._roScrollHandler = updateThumb;
		element.addEventListener('scroll', updateThumb);

		element.addEventListener('wheel', e => {
			const h = element.clientHeight;
			const sh = element.scrollHeight;
			if (sh <= h) {
				return;
			}

			const delta = e.deltaY > 0 ? 1 : -1;
			element.scrollTop += delta * 20;

			updateThumb();
			e.preventDefault();
			e.stopPropagation();
		});

		upBtn.addEventListener('mousedown', () => {
			element.scrollTop -= 20;
			updateThumb();
		});

		downBtn.addEventListener('mousedown', () => {
			element.scrollTop += 20;
			updateThumb();
		});

		thumb.addEventListener('pointerdown', e => {
			isDragging = true;
			startY = e.clientY;
			startThumbY = parseInt(thumb.style.top, 10) || 0;
			e.preventDefault();

			if (e.pointerId !== undefined) {
				try {
					thumb.setPointerCapture(e.pointerId);
				} catch (_e) {
					// Ignore DOM exceptions on capture
				}
			}
		});

		thumb.addEventListener('pointermove', e => {
			if (!isDragging) {
				return;
			}

			const h = element.clientHeight;
			const sh = element.scrollHeight;
			const trackHeight = track.clientHeight;
			const thumbHeight = thumb.clientHeight;

			const maxScrollTop = sh - h;
			const maxThumbTop = trackHeight - thumbHeight;

			const deltaY = e.clientY - startY;
			const newThumbTop = Math.max(0, Math.min(startThumbY + deltaY, maxThumbTop));

			const percentage = newThumbTop / maxThumbTop;
			element.scrollTop = percentage * maxScrollTop;
			updateThumb();
		});

		thumb.addEventListener('pointerup', e => {
			isDragging = false;
			if (e.pointerId !== undefined) {
				try {
					thumb.releasePointerCapture(e.pointerId);
				} catch (_e) {
					// Ignore DOM exceptions
				}
			}
		});

		thumb.addEventListener('pointercancel', e => {
			isDragging = false;
			if (e.pointerId !== undefined) {
				try {
					thumb.releasePointerCapture(e.pointerId);
				} catch (_e) {
					// Ignore DOM exceptions
				}
			}
		});

		track.addEventListener('mousedown', e => {
			if (e.target === thumb) {
				return;
			}
			const clickY = e.offsetY;
			const thumbTop = parseInt(thumb.style.top, 10) || 0;
			if (clickY < thumbTop) {
				element.scrollTop -= element.clientHeight;
			} else {
				element.scrollTop += element.clientHeight;
			}
			updateThumb();
		});

		setTimeout(updateThumb, 0);
	}
}
export default ScrollBar;

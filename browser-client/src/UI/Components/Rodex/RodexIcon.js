/**
 * UI/Components/Rodex/RodexIcon.js
 *
 * Rodex Icon
 *
 * @author Alisonrag
 *
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './RodexIcon.html?raw';
import cssText from './RodexIcon.css?raw';
import Rodex from 'UI/Components/Rodex/Rodex.js';

/**
 * Create Component
 */
const RodexIcon = new GUIComponent('RodexIcon', cssText);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return RodexIcon._shadow || RodexIcon._host;
}

/**
 * Render HTML
 */
RodexIcon.render = () => htmlText;

/**
 * Apply preferences once append to body
 */
RodexIcon.onAppend = function OnAppend() {
	const root = _root();
	const icon = root.querySelector('.rodex-icon');
	icon.addEventListener('click', onClickRodexIcon);
};

RodexIcon.toggle = function toggle() {
	if (this._host.style.display === 'none') {
		this._host.style.display = '';
		this.focus();
	} else {
		this._host.style.display = 'none';
	}
};

function onClickRodexIcon() {
	Rodex.openRodexBox();
	Rodex.append();
	Rodex._host.style.display = '';
	Rodex.focus();
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(RodexIcon);

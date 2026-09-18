/**
 * UI/Components/MakeItemSelection/ItemConvertSelection/MakeModelMessage/MakeModelMessage.js
 *
 * MakeModelMessage windows
 *
 * @author Francisco Wallison
 */

import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './MakeModelMessage.html?raw';
import cssText from './MakeModelMessage.css?raw';
import ConvertItems from 'UI/Components/MakeItemSelection/ItemConvertSelection/ConvertItems.js';

/**
 * Create MakeModelMessage namespace
 */
const MakeModelMessage = new GUIComponent('MakeModelMessage', cssText);

MakeModelMessage.render = () => htmlText;

/**
 * Initialize UI
 */
MakeModelMessage.init = function init() {
	const root = this.getRoot();

	this._host.style.top = `${(Renderer.height - 200) / 2}px`;
	this._host.style.left = `${(Renderer.width - 200) / 2}px`;

	root.querySelector('ui-button.ok').addEventListener('click', onSendMaterial);
	root.querySelector('ui-button.cancel').addEventListener('click', onClose);

	this.draggable(root.querySelector('.titlebar'));
};

function onSendMaterial(event) {
	event.stopImmediatePropagation();
	ConvertItems.validItemSend(true);
}

function onClose(event) {
	event.stopImmediatePropagation();
	ConvertItems.validItemSend(false);
}

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(MakeModelMessage);

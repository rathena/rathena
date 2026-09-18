/**
 * UI/Components/JoystickUI/JoystickUI.js
 *
 * Joystick window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import JoystickModule from './JoystickModule.js';
import JoystickSelectionUI from './JoystickSelectionUI.js';
import htmlText from './JoystickUI.html?raw';
import cssText from './JoystickUI.css?raw';
import JoystickUIRenderer from './JoystickUIRenderer.js';

const JoystickUI = new GUIComponent('JoystickUI', cssText);
JoystickUI.render = () => htmlText;

JoystickUI.onAppend = function () {
	JoystickUIRenderer.attach(this.ui);
	this._host.style.display = 'none';
	JoystickSelectionUI.append();
};

JoystickUI.onRemove = function () {
	JoystickModule.dispose();
};

JoystickUI.onRestore = function () {
	JoystickModule.prepare();
};

JoystickUI.show = function () {
	this._host.style.display = 'block';
};

JoystickUI.hide = function () {
	this._host.style.display = 'none';
};

export default UIManager.addComponent(JoystickUI);

/**
 * UI/Components/SoundOption/SoundOption.js
 *
 * Manage sound volume
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import Preferences from 'Core/Preferences.js';
import AudioSettings from 'Preferences/Audio.js';
import AudioManager from 'Audio/BGM.js';
import SoundManager from 'Audio/SoundManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './SoundOption.html?raw';
import cssText from './SoundOption.css?raw';

const SoundOption = new GUIComponent('SoundOption', cssText);

SoundOption.render = () => htmlText;

const _preferences = Preferences.get('SoundOption', { x: 300, y: 300 }, 1.0);

SoundOption.init = function init() {
	const root = this.getRoot();

	const baseBtn = root.querySelector('.base');
	if (baseBtn) {
		baseBtn.addEventListener('mousedown', function (event) {
			event.stopImmediatePropagation();
		});
	}

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', function () {
			SoundOption.remove();
		});
	}

	const soundSlider = root.querySelector('.sound');
	if (soundSlider) {
		soundSlider.addEventListener('change', onSoundVolumeUpdate);
	}

	const bgmSlider = root.querySelector('.bgm');
	if (bgmSlider) {
		bgmSlider.addEventListener('change', onBGMVolumeUpdate);
	}

	const soundState = root.querySelector('.sound_state');
	if (soundState) {
		soundState.addEventListener('change', onToggleSound);
	}

	const bgmState = root.querySelector('.bgm_state');
	if (bgmState) {
		bgmState.addEventListener('change', onToggleBGM);
	}

	this.draggable('.titlebar');
};

SoundOption.onAppend = function onAppend() {
	this._host.style.top = _preferences.y + 'px';
	this._host.style.left = _preferences.x + 'px';

	const root = this.getRoot();

	const soundSlider = root.querySelector('.sound');
	if (soundSlider) soundSlider.value = AudioSettings.Sound.volume * 100;

	const bgmSlider = root.querySelector('.bgm');
	if (bgmSlider) bgmSlider.value = AudioSettings.BGM.volume * 100;

	const soundState = root.querySelector('.sound_state');
	if (soundState) soundState.checked = AudioSettings.Sound.play;

	const bgmState = root.querySelector('.bgm_state');
	if (bgmState) bgmState.checked = AudioSettings.BGM.play;
};

SoundOption.onRemove = function onRemove() {
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.save();
};

function onSoundVolumeUpdate() {
	AudioSettings.Sound.volume = parseInt(this.value, 10) / 100;
	AudioSettings.save();
	SoundManager.setVolume(AudioSettings.Sound.volume);
}

function onToggleSound() {
	const oldVolume = AudioSettings.Sound.volume;
	AudioSettings.Sound.play = this.checked;

	if (AudioSettings.Sound.play) {
		SoundManager.setVolume(AudioSettings.Sound.volume);
	} else {
		SoundManager.setVolume(0);
		SoundManager.stop();
	}

	AudioSettings.Sound.volume = oldVolume;
	AudioSettings.save();
}

function onBGMVolumeUpdate() {
	AudioSettings.BGM.volume = parseInt(this.value, 10) / 100;
	AudioSettings.save();
	AudioManager.setVolume(AudioSettings.BGM.volume);
}

function onToggleBGM() {
	AudioSettings.BGM.play = this.checked;
	AudioSettings.save();

	if (AudioSettings.BGM.play) {
		AudioManager.play(AudioManager.filename);
	} else {
		AudioManager.stop();
	}
}

export default UIManager.addComponent(SoundOption);

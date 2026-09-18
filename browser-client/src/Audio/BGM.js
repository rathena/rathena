/**
 * Audio/BGM.js
 *
 * BGM Manager
 *
 * Class to Manage BGM (RO background music)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import Preferences from 'Preferences/Audio.js';

let _playToken = 0;

/**
 * BGM NameSpace
 */
class BGM {
	static filename = null;
	static volume = Preferences.BGM.volume;
	static extension = 'mp3';
	static isInit = false;
	static audio = document.createElement('audio');

	// Cache to persist playback position when switching BGMs
	static cache = {
		filename: null,
		currentTime: 0
	};

	/**
	 * Initialize player
	 * Fixed a known bug
	 */
	static init() {
		if (BGM.isInit) {
			return;
		}

		BGM.isInit = true;

		// Buggy looping for HTM5 Audio...
		if (typeof BGM.audio.loop === 'boolean') {
			BGM.audio.loop = true;
			return;
		}

		// Work around
		BGM.audio.addEventListener(
			'ended',
			() => {
				BGM.audio.currentTime = 0;
				if (BGM.cache.filename === BGM.filename) {
					BGM.cache.currentTime = 0;
				}
				BGM.audio.play();
			},
			false
		);
	}

	/**
	 * Test audio extension from a list to see what format the browser can read
	 *
	 * @param {Array} extensions list
	 */
	static setAvailableExtensions(extensions) {
		let i, count;
		const audio = BGM.audio;

		if (!extensions || !extensions.length) {
			extensions = ['mp3'];
		}

		// Find supported audio file from list
		for (i = 0, count = extensions.length; i < count; ++i) {
			if (audio.canPlayType(`audio/${extensions[i]}`).replace(/no/i, '')) {
				BGM.extension = extensions[i];
				BGM.init();
				return;
			}
		}
	}

	/**
	 * Play the audio file specify
	 *
	 * @param {string} filename
	 */
	static play(filename) {
		if (!filename) return;
		if (filename.match(/bgm/i)) {
			filename = filename.match(/\w+\.mp3/i)?.toString();
			if (!filename) return;
		}
		if (BGM.filename === filename && BGM.audio && !BGM.audio.paused) {
			return;
		}

		// Save current progress before switching BGMs
		if (BGM.filename && BGM.audio) {
			BGM.cache.filename = BGM.filename;
			BGM.cache.currentTime = BGM.audio.currentTime;
		}

		BGM.filename = filename;
		const myToken = ++_playToken;
		if (Preferences.BGM.play) {
			Client.loadFile(`BGM/${filename}`, url => {
				if (myToken !== _playToken) {
					return;
				}
				BGM.load(url);
			});
		}
	}

	/**
	 * Load the audio file
	 *
	 * @param {string} url (HTTP / DATA URI or BLOB)
	 */
	static load(url) {
		if (!Preferences.BGM.play) {
			return;
		}

		// Add support for other extensions, only supported with
		// remote audio files.
		if (!url.match(/^(blob|data):/)) {
			url = url.replace(/mp3$/i, BGM.extension);
		}

		// Check if the new BGM matches the one in the cache
		const isSameBGM = BGM.cache.filename === BGM.filename;
		const targetTime = isSameBGM ? BGM.cache.currentTime : 0;

		BGM.audio.src = url;
		BGM.audio.volume = BGM.volume;

		// Restore playback position if the BGM is the same
		BGM.audio.currentTime = targetTime;

		const playPromise = BGM.audio.play();
		if (playPromise) {
			playPromise.catch(err => {
				if (err.name !== 'AbortError') {
					console.warn('Failed to play:', err);
				}
			});
		}
	}

	/**
	 * Stop the BGM
	 */
	static stop() {
		_playToken++;
		if (BGM.audio) {
			// Save current progress before pausing the global playback
			BGM.cache.filename = BGM.filename;
			BGM.cache.currentTime = BGM.audio.currentTime;
			BGM.audio.pause();
		}
	}

	/**
	 * Change the volume of the BGM
	 *
	 * @param {number} volume
	 */
	static setVolume(volume) {
		BGM.volume = volume;
		Preferences.BGM.volume = volume;
		Preferences.save();

		BGM.audio.volume = volume;
	}
}

/**
 * Export
 */
export default BGM;

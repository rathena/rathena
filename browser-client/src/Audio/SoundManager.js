/**
 * Audio/SoundManager.js
 *
 * Sound Manager
 *
 * Manage sounds effects
 * All browsers seems to support .wav file (with HTML5)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import Preferences from 'Preferences/Audio.js';
import Memory from 'Core/MemoryManager.js';
import glMatrix from 'Utils/gl-matrix.js';
import Session from 'Engine/SessionStorage.js';

const C_MAX_SOUND_INSTANCES = 10; //starting max, later balanced based on mediaPlayerCount
const C_MAX_CACHED_SOUND_INSTANCES = 30; //starting max, later balanced based on mediaPlayerCount
const C_MAX_MEDIA_PLAYERS = 800; //Browsers are limited to 1000 media players max (in Chrome). Let's not go all the way.
const C_SAME_SOUND_DELAY = 100; //ms
const C_CACHE_CLEANUP_TIME = 30000; //ms

/**
 * Sound memory
 */
const _sounds = {};

/**
 * Re-usable sounds
 */
const _cache = {};

/**
 * @Number of existing HTML Media players in the DOM
 */
let mediaPlayerCount = 0;
let _playGen = 0;

/**
 * @Constructor
 */
class SoundManager {
	/**
	 * @var {float} sound volume
	 *
	 */
	static volume = Preferences.Sound.volume;

	/**
	 * Play a wav sound
	 *
	 * @param {string} filename
	 * @param {optional|number} vol (volume)
	 */
	static play(filename, vol) {
		let volume;
		if (vol) {
			volume = vol * this.volume;
		} else {
			volume = this.volume;
		}
		if (volume <= 0 || !Preferences.Sound.play) {
			return;
		}
		if (!(filename in _sounds)) {
			_sounds[filename] = {};
			_sounds[filename].instances = [];
			_sounds[filename].lastTick = 0;
		}
		// Re-usable sound from cache
		const sound = getSoundFromCache(filename);
		if (sound) {
			sound.volume = Math.min(volume, 1.0);
			sound._volume = volume;
			const playPromise = sound.play();
			if (playPromise) {
				playPromise.catch(err => {
					// blob revogado / src inválido → descarta e recarrega do zero
					if (err.name === 'NotSupportedError' || err.name === 'AbortError') {
						const idx = _sounds[filename]?.instances.indexOf(sound);
						if (idx !== undefined && idx !== -1) {
							_sounds[filename].instances.splice(idx, 1);
						}
						sound.remove();
						mediaPlayerCount--;
						SoundManager.play(filename, vol);
						return;
					}
					console.warn('Failed to play sound:', err);
				});
			}
			_sounds[filename].instances.push(sound);
			_sounds[filename].lastTick = Date.now();
			return;
		}
		const myGen = _playGen;
		Client.loadFile(`data/wav/${filename}`, url => {
			if (myGen !== _playGen || !(filename in _sounds)) {
				return;
			}
			if (
				_sounds[filename].lastTick > Date.now() - C_SAME_SOUND_DELAY ||
				_sounds[filename].instances.length > balancedMax(C_MAX_SOUND_INSTANCES)
			) {
				return;
			}
			const audio = document.createElement('audio');
			mediaPlayerCount++;
			audio.filename = filename;
			audio.src = url;
			audio.volume = Math.min(volume, 1.0);
			audio._volume = volume;
			audio.addEventListener('error', onSoundError, false);
			audio.addEventListener('ended', onSoundEnded, false);
			audio.play().catch(err => {
				if (err.name !== 'AbortError') {
					console.warn('Failed to play sound:', err);
				}
			});
			_sounds[filename].instances.push(audio);
			_sounds[filename].lastTick = Date.now();
		});
	}

	/**
	 * Play a wav sound with calculated position for volume
	 *
	 * @param {string} filename
	 * @param {optional|number} vol (volume)
	 */
	static playPosition(filename, srcPosition) {
		const dist = Math.floor(glMatrix.vec2.dist(srcPosition, Session.Entity.position));
		const vol = Math.max(1 - Math.abs(((dist - 1) * (1 - 0.01)) / (25 - 1) + 0.01), 0.1);
		SoundManager.play(filename, vol);
	}

	/**
	 * Stop a specify sound, or all sounds.
	 *
	 * @param {optional|string} filename to stop
	 */
	static stop(filename) {
		if (filename) {
			if (filename in _sounds) {
				while (_sounds[filename].instances.length > 0) {
					const s = _sounds[filename].instances.shift();
					s.pause();
					s.remove();
					mediaPlayerCount--;
				}
				delete _sounds[filename];
			}
			return;
		}
		_playGen++;
		// limpa instâncias ativas
		Object.keys(_sounds).forEach(key => {
			while (_sounds[key].instances.length > 0) {
				const s = _sounds[key].instances.shift();
				s.pause();
				s.remove();
				mediaPlayerCount--;
			}
			delete _sounds[key];
		});
		// limpa cache (senão sobram <audio> com src revogado)
		Object.keys(_cache).forEach(key => {
			_cache[key].instances.forEach(s => {
				if (s.cleanupHandle) {
					clearTimeout(s.cleanupHandle);
				}
				s.remove();
				mediaPlayerCount--;
			});
			delete _cache[key];
		});
		const list = Memory.search(/\.wav$/);
		list.forEach(key => {
			Memory.remove(key);
		});
	}

	/**
	 * Change volume of all sounds
	 *
	 * @param {number} volume
	 */
	static setVolume(volume) {
		this.volume = Math.min(volume, 1.0);

		Preferences.Sound.volume = this.volume;
		Preferences.save();

		Object.keys(_sounds).forEach(key => {
			_sounds[key].instances.forEach(sound => {
				sound.volume = Math.min(sound._volume * this.volume, 1.0);
			});
		});
	}
}
/**
 * Move sound to cache.
 * ff we have a request to play the same sound again, get it back
 * Will avoid to re-create sound object at each request (re-usable object)
 */
function onSoundEnded() {
	if (_sounds[this.filename]) {
		const pos = _sounds[this.filename].instances.indexOf(this);

		if (pos !== -1) {
			_sounds[this.filename].instances.splice(pos, 1);
			if (_sounds[this.filename].instances.length === 0) {
				delete _sounds[this.filename]; //This can cause some errors, but whatever. Everything for performance!
			}
		}

		addSoundToCache(this);
	}
}

/**
 * Clear sound from dom on error
 */
function onSoundError() {
	const entry = _sounds[this.filename];
	if (entry) {
		const pos = entry.instances.indexOf(this);
		if (pos !== -1) {
			entry.instances.splice(pos, 1);
			if (entry.instances.length === 0) {
				delete _sounds[this.filename];
			}
		}
	}
	this.remove();
	mediaPlayerCount--;
}

/**
 * Add sound to cache and set associated vars
 *
 * @param {Audio} sound element
 */
function addSoundToCache(sound) {
	if (sound.filename) {
		if (!(sound.filename in _cache)) {
			_cache[sound.filename] = new Object();
			_cache[sound.filename].instances = new Array();
		}

		//Don't cache too many instances (self balancing formula based on total media players)
		if (_cache[sound.filename].instances.length < balancedMax(C_MAX_CACHED_SOUND_INSTANCES)) {
			sound.currentTime = 0; //reset to start to save seeking time on next play THIS IS IMPORTANT! It improves the performance by 10 fold for whatever reason...

			sound.cleanupHandle = setTimeout(() => {
				cleanupCache(sound);
			}, C_CACHE_CLEANUP_TIME);
			_cache[sound.filename].instances.push(sound); //put to the end
		} else {
			sound.remove(); //remove from dom if too many instances are already stored
			mediaPlayerCount--;
		}
	}
}

/**
 * Remove sound from cache and return it
 * Check at the same time to remove sound not used since some times.
 *
 * @param {string} filename
 * @param {Audio} sound element
 */
function getSoundFromCache(filename) {
	let out = null;

	if (filename in _cache) {
		if (_cache[filename].instances.length > 0) {
			out = _cache[filename].instances.pop(); //remove last instance from cache (newest)
			if (out.cleanupHandle) {
				clearTimeout(out.cleanupHandle); //cancel cleanup
			}
		}
	}

	return out;
}

/**
 * Remove sound from cache if it was sitting there for too long
 *
 * @param {Audio} sound element
 */
function cleanupCache(sound) {
	if (sound.filename && sound.filename in _cache && _cache[sound.filename].instances.length > 0) {
		const pos = _cache[sound.filename].instances.indexOf(sound);

		if (pos !== -1) {
			_cache[sound.filename].instances.splice(pos, 1);
			/*if(_cache[sound.filename].instances.length == 0){
					delete _cache[sound.filename];
				}*/
			//don't remove the key itself from the cache, because that can cause conflict in the push to instances
			sound.remove();
			mediaPlayerCount--;
		}
	}
}

/**
 * Returns a balanced value for max audio instance number based on the currently existing HTML Media players in the DOM
 *
 * @param {CONST} max instance const value
 */
function balancedMax(maxConst) {
	return Math.ceil(maxConst * (1 - mediaPlayerCount / C_MAX_MEDIA_PLAYERS));
}

/**
 * Export
 */
export default SoundManager;

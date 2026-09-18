/**
 * Core/Preferences.js
 *
 * Store informations in local storage (window position, noctrl, etc.)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

const Storage = {
	get: function Get(key, fn) {
		const out = {};
		out[key] = localStorage.getItem(key);
		fn(out);
	},
	set: function Set(obj, fn) {
		const keys = Object.keys(obj);
		let i, count;

		for (i = 0, count = keys.length; i < count; ++i) {
			localStorage.setItem(keys[i], obj[keys[i]]);
		}

		if (fn) {
			fn();
		}
	}
};
class Preferences {
	/**
	 * Get back values
	 *
	 * @param {string} key
	 * @param {mixed} default value
	 * @param {number} optional version
	 */
	static get(key, def, version) {
		Storage.get(key, function (value) {
			version = version || 0.0;

			// Not existing, storing it
			if (!value[key] || JSON.parse(value[key])._version !== version) {
				Preferences.save(def);
				return;
			}

			const data = JSON.parse(value[key]);
			data._key = key;
			data._version = version;
			data.save = selfSave;

			const keys = Object.keys(data);
			const count = keys.length;

			for (let i = 0; i < count; ++i) {
				def[keys[i]] = data[keys[i]];
			}
		});

		def._key = key;
		def._version = version;
		def.save = selfSave;

		return def;
	}

	/**
	 * Save value in storage
	 *
	 * @param {string} key
	 * @param {object} value to store
	 */
	static save(data) {
		const key = data._key;
		delete data._key;
		delete data.save;

		const store = {};
		store[key] = JSON.stringify(data);

		Storage.set(store);

		data._key = key;
		data.save = selfSave;
	}
}
/**
 * Save from object
 */
function selfSave() {
	Preferences.save(this);
}

/**
 * Export
 */
export default Preferences;

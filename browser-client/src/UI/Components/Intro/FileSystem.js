/**
 * UI/Components/Intro/FileSystem.js
 *
 * FileSystem API Utility
 *
 * @author Vincent Thibault
 */

/**
 * @var {FileSystem}
 */
let _fs;

/**
 * @var {TemporaryStorage} compatibility
 */
const temporaryStorage = navigator.temporaryStorage || navigator.webkitTemporaryStorage;

/**
 * @var {RequestFileSystem} compatibility
 */
const requestFileSystem = window.requestFileSystem || window.webkitRequestFileSystem;

/**
 * Clean Up FileSystem files
 *
 * @param {function} callback
 */
function cleanUp(callback) {
	_fs.root.createReader().readEntries(function (entries) {
		let i,
			j = 0;
		const count = entries.length;

		function removed() {
			if (++j >= count) {
				callback();
			}
		}

		for (i = 0; i < count; ++i) {
			if (entries[i].isDirectory) {
				entries[i].removeRecursively(removed);
			} else {
				entries[i].remove(removed);
			}
		}

		if (!count) {
			callback();
		}
	});
}

/**
 * Get the size of the FileSystem API
 *
 * @param {function} callback
 */
function getSize(callback) {
	if (!temporaryStorage || !requestFileSystem) {
		callback(0);
		return;
	}

	temporaryStorage.queryUsageAndQuota(function (used) {
		if (!used) {
			callback(0);
			return;
		}

		requestFileSystem(window.TEMPORARY, used, function (fs) {
			_fs = fs;

			// Remove upload folder
			fs.root.getDirectory(
				'/__tmp_upload/',
				{ create: false },
				function (dirEntry) {
					dirEntry.removeRecursively(function () {
						getSize(callback);
					});
					// no upload directory, end.
				},
				function noDirectory() {
					callback(used);
				}
			);
		});
	});
}

/**
 * Export
 */
export default {
	cleanup: cleanUp,
	getSize: getSize
};

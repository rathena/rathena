/**
 * api.js
 *
 * Robrowser application entry, starting instance.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */
/* eslint-disable */
(function ROAPI() {
	'use strict';

	/**
	 * @Constructor
	 */
	function ROBrowser(options) {
		if (typeof options === 'object') {
			Object.assign(this.config, options);
		}
	}

	/**
	 * @Enum Robrowser type
	 */
	ROBrowser.TYPE = {
		POPUP: 1,
		FRAME: 2,
		INLINE: 3
	};

	/**
	 * @Enum Robrowser Applications
	 */
	ROBrowser.APP = {
		ONLINE: 1,
		MAPVIEWER: 2,
		GRFVIEWER: 3,
		MODELVIEWER: 4,
		STRVIEWER: 5,
		GRANNYMODELVIEWER: 6, //sound weird O_o
		EFFECTVIEWER: 7
	};

	/**
	 * @type {Object} ROBrowser configuration object
	 */
	ROBrowser.prototype.config = {
		/**
		 * @type {number} screen width
		 */
		width: 0,

		/**
		 * @type {number} screen height
		 */
		height: 0,

		/**
		 * @type {mixed} grf listing
		 *
		 * a) {Array} of GRFs:
		 *    [ 'custom.grf', 'palette.grf', 'data.grf' ]
		 *
		 * b) {string} DATA.INI filename to load
		 *    'DATA.INI'
		 *
		 * c) {RegExp} to filter grf files:
		 *     /\.grf$/i
		 */
		grfList: null,

		/**
		 * @type {servers} server listing
		 *
		 * a) {string} clientinfo file to load
		 *    'data/clientinfo.xml'
		 *
		 * b) {Array} server list to display:
		 */
		servers: 'data/clientinfo.xml',

		/**
		 * @type {string} Host where to download files
		 */
		remoteClient: 'https://grf.robrowser.com/',

		/**
		 * @type {number|string} packet version
		 *
		 * Supported value:
		 *    a) YYYYMMDD     (number: date you want)
		 *    c) 'executable' (detect packetver from executable compilation date)
		 */
		packetver: 'auto',

		/**
		 * @type {number} character info block size
		 * If not set, it will try to guess the type based on the packetver and the block total length
		 */
		charBlockSize: 0,

		/**
		 * @type {string} client hash to send to server
		 */
		clientHash: null,

		/**
		 * @type {string} calculate client hash
		 */
		calculateHash: false,

		/**
		 * @type {string} files to hash for hash calculation
		 */
		hashFiles: [],

		/**
		 * @type {constant} application name (see: ROBrowser.APP.* )
		 *
		 * Known applications:
		 *   a) ROBrowser.APP.ONLINE    - RoBrowser online mode
		 *   b) ROBrowser.APP.GRFVIEWER - parse and visualize GRF contents
		 *   c) ROBrowser.APP.MAPVIEWER - parse and visualize maps
		 */
		application: ROBrowser.APP.ONLINE,

		/**
		 * @type {constant} container type (see: ROBrowser.TYPE.POPUP)
		 */
		type: ROBrowser.TYPE.POPUP,

		/**
		 * @type {string} element ID
		 * If using container type 'frame', place the content in the HTMLElement specify
		 */
		target: null,

		/**
		 * @type {boolean} is in development mode ?
		 */
		development: false,

		/**
		 * @type {boolean} load lua files ?
		 */
		loadLua: false,

		/**
		 * @type {function} callback to execute once roBrowser is ready
		 */
		onReady: null,

		/**
		 * @type {boolean} use API once ready ?
		 */
		api: false,

		/**
		 * @type {string} proxy server ex: 'ws://pserver.com:5200/'
		 */
		socketProxy: null,

		/**
		 * @type {string} web-server api: 'http://<yourserverip>:<port>/' (used only to electron build)
		 */
		webserverAddress: null,

		/**
		 * @type {boolean}dump packet as hex in console ?
		 */
		packetDump: false,

		/**
		 * @type {integer|boolean|array} packetKeys
		 * see: http://hercules.ws/board/topic/1105-hercules-wpe-free-june-14th-patch/
		 *
		 * Supported value:
		 * - integer : client date,
		 *         ex: packetKeys: 20131223,
		 * - boolean : supported ? If it's the case, will use the executable compiled date to get the keys
		 *         ex: packetKeys: true,
		 * - array: the keys you want to use:
		 *         ex: packetKeys: [0xFF2615DE, 0x96AAE533, 0x1166CC33],
		 */
		packetKeys: false,

		/**
		 * @type {boolean} should we save files in chrome filesystem ?
		 *
		 * If set to true, then we try to save the files loaded from server/grfs on a filesystem to load
		 * them faster the next time.
		 *
		 * Only working on Chrome, status: deprecated.
		 */
		saveFiles: true,

		/**
		 * @type {boolean} skip server list if only one server define ?
		 *
		 * If set to true and the server list (clientinfo, char-server list) just have one
		 * element defined, the window will be skipped and you will auto-connect to the server.
		 *
		 * Set to false, will display the server list even if there is just one server set.
		 */
		skipServerList: true,

		/**
		 * @type {boolean} do we skip the intro ?
		 * Note: if you skip it, the user will not be able to load their local fullclient
		 */
		skipIntro: false,

		/**
		 * @type {Array} do you want to auto-login to the server ?
		 * Can be used in a securized session to auto-connect to the server without inserting login-pass (ie: Facebook app ?)
		 * Using as autoLogin: ["username", "userpass"]
		 */
		autoLogin: [],

		/**
		 * @type {boolean} Enable Cash Shop UI
		 */
		enableCashShop: false,

		/**
		 * @type {boolean} Enable Achievements UI
		 */
		enableAchievements: false,

		/**
		 * @type {boolean} Enable Bank UI
		 */
		enableBank: false,

		/**
		 * @type {boolean} Enable Damage Suffix
		 */
		enableDmgSuffix: false,

		/**
		 * @type {boolean} Enable Map Name Banner
		 */
		enableMapName: false,

		/**
		 * @type {boolean} Enable Check Attendance UI
		 */
		enableCheckAttendance: false,

		/**
		 * @type {boolean} Enable Homunculus Auto Feed for older PACKETVER than 20170920
		 */
		enableHomunAutoFeed: false,

		/**
		 * @type {boolean} User interface version selection mode (PacketVer | PreRenewal | Renewal)
		 */
		clientVersionMode: 'PacketVer',

		/**
		 * @type {mixed} set a version to avoid browser cache problem so
		 * your users wil get the latest version running instead of a
		 * cached one.
		 */
		version: '',

		/**
		 * @type {URL} URL to the server's registration site
		 */
		registrationweb: '',

		/**
		 * @type {Object} Dettings for World Map
		 */
		worldMapSettings: {},

		/**
		 * @type {boolean} Enable use address to connect in all servers (login, char, map)
		 */
		forceUseAddress: false,

		/**
		 * @type {boolean} Enable console in non-development environment
		 */
		enableConsole: false,

		/**
		 * @type {boolean} Force disable console in any environment
		 */
		disableConsole: false,

		/**
		 * @type {Object} Settings for display aura levels
		 */
		aura: {},

		/**
		 * @type {Array} list of extensions you want to use for your BGMs.
		 * It will test each extensions until there is one it can read.
		 *
		 * Examples: ['ogg', 'mp4', 'mp3']
		 * Will try to see if it can load '.ogg' audio file, if it fail, will try to see if it can load .mp4, etc.
		 */
		BGMFileExtension: ['mp3'],

		/**
		 * @type {Object} Define plugin to execute
		 * It will test each extensions until there is one it can read.
		 */
		plugins: {},

		// Custom camera support
		ThirdPersonCamera: false,
		FirstPersonCamera: false,
		CameraMaxZoomOut: 5,

		/**
		 * @type {integer} transition duration in milliseconds
		 * used for fade out and fade in when teleport/change map
		 * the total will be the double in ms because its used on fade in then on fade out
		 * eg: if you use 500ms the transition will take 1000ms in total
		 */
		transitionDuration: 500
	};

	/**
	 * @type {string} roBrowser api window path
	 */
	ROBrowser.prototype.baseUrl = (function () {
		var script = document.getElementsByTagName('script');
		return script[script.length - 1].src
			.replace(/\/[^\/]+\.js.*/, '/api.js') // redirect compiled script
			.replace(/\/src\/.*/, '/api.js'); // fix error with cache (FF)
	})().replace('.js', '.html');

	/**
	 * Start ROBrowser Instance
	 */
	ROBrowser.prototype.start = function Start() {
		switch (this.config.type) {
			// Create Popup
			case ROBrowser.TYPE.POPUP:
				this.config.width = this.config.width || '800';
				this.config.height = this.config.height || '600';

				this._APP = window.open(
					this.baseUrl + '?' + this.config.version,
					'_blank',
					[
						'directories=0',
						'fullscreen=0',
						'top=' + ((window.innerHeight || document.body.clientHeight) - this.config.height) / 2,
						'left=' + ((window.innerWidth || document.body.clientWidth) - this.config.width) / 2,
						'height=' + this.config.height,
						'width=' + this.config.width,
						'location=0',
						'menubar=0',
						'resizable=0',
						'scrollbars=0',
						'status=0',
						'toolbar=0'
					].join(',')
				);
				break;

			// Append ROBrowser to an element
			case ROBrowser.TYPE.FRAME:
				this.config.width = this.config.width || '100%';
				this.config.height = this.config.height || '100%';

				var frame = document.createElement('iframe');
				frame.src = this.baseUrl + '?' + Math.random() + location.hash; // fix bug on firefox
				frame.width = this.config.width;
				frame.height = this.config.height;
				frame.style.border = 'none';

				frame.setAttribute('allowfullscreen', 'true');
				frame.setAttribute('webkitallowfullscreen', 'true');
				frame.setAttribute('mozallowfullscreen', 'true');

				if (this.config.target) {
					while (this.config.target.firstChild) {
						this.config.target.removeChild(this.config.target.firstChild);
					}
					this.config.target.appendChild(frame);
				}

				// Remove parent page preloader — the iframe handles its own loading
				var parentPreloader = document.getElementById('ro-preloader');
				if (parentPreloader) parentPreloader.remove();

				this._APP = frame.contentWindow;
				break;

			// Append roBrowser inline locally
			case ROBrowser.TYPE.INLINE:
				this.config.width = this.config.width || '100%';
				this.config.height = this.config.height || '100%';

				if (this.config.target) {
					while (this.config.target.firstChild) {
						this.config.target.removeChild(this.config.target.firstChild);
					}
					var container = document.createElement('div');
					container.style.width =
						typeof this.config.width === 'number' ? this.config.width + 'px' : this.config.width;
					container.style.height =
						typeof this.config.height === 'number' ? this.config.height + 'px' : this.config.height;
					container.style.position = 'relative';
					this.config.target.appendChild(container);
					// redefine target to the inner container
					this.config.target = container;
				}

				window.ROConfig = this.config;

				var url = new URL(this.baseUrl);
				var path = url.pathname;
				var projectRoot = path.split('/applications/')[0] + '/';

				if (!document.querySelector('script[type="importmap"]')) {
					var sharedScript = document.createElement('script');
					sharedScript.src = projectRoot + 'applications/shared/importmap.js';
					sharedScript.dataset.projectRoot = projectRoot;
					sharedScript.onload = function () {
						if (!document.querySelector('script[data-api="robrowser-main"]')) {
							var s2 = document.createElement('script');
							s2.type = 'module';
							s2.dataset.api = 'robrowser-main';
							s2.textContent = "import '" + projectRoot + "src/main.js';";
							document.head.appendChild(s2);
						}
					};
					sharedScript.onerror = function () {
						console.error('Failed to load roBrowser import map from: ' + sharedScript.src);
					};
					document.head.appendChild(sharedScript);
				} else if (!document.querySelector('script[data-api="robrowser-main"]')) {
					var s2 = document.createElement('script');
					s2.type = 'module';
					s2.dataset.api = 'robrowser-main';
					s2.textContent = "import '" + projectRoot + "src/main.js';";
					document.head.appendChild(s2);
				}

				var self = this;
				window.addEventListener(
					'robrowser-ready',
					function onAppReady() {
						window.removeEventListener('robrowser-ready', onAppReady, false);
						if (self.onReady) {
							self.onReady();
						}
					},
					false
				);

				this._APP = window;
				break;
		}

		// Wait for robrowser to be ready
		var _this = this;
		function OnMessage(event) {
			if (_this.baseUrl.indexOf(event.origin) === 0) {
				clearInterval(_this._Interval);
				window.removeEventListener('message', OnMessage, false);

				if (_this.onReady) {
					_this.onReady();
				}
			}
		}

		// Start waiting for robrowser
		if (this.config.type !== ROBrowser.TYPE.INLINE) {
			this._Interval = setInterval(WaitForInitialization.bind(this), 100);
			window.addEventListener('message', OnMessage, false);
		}
	};

	/**
	 * Spam the window until there is an answer
	 * No onload event from external iframe/popup
	 */
	function WaitForInitialization() {
		this._APP.postMessage(JSON.parse(JSON.stringify(this.config)), '*');
	}

	/**
	 * Export
	 */
	window.ROBrowser = ROBrowser;
})();

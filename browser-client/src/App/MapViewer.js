/**
 * App/MapViewer.js
 *
 * Start a hacked map-viewer
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Queue from 'Utils/Queue.js';
import Configs from 'Core/Configs.js';
import Client from 'Core/Client.js';
import Thread from 'Core/Thread.js';
import BGM from 'Audio/BGM.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import Camera from 'Renderer/Camera.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Entity from 'Renderer/Entity/Entity.js';
import Mouse from 'Controls/MouseEventHandler.js';
import MapControl from 'Controls/MapControl.js';
import Intro from 'UI/Components/Intro/Intro.js';

/**
 * MapViewer namespace
 */
const MapViewer = {};

/**
 * @var {object} Entity to target
 */
// Registered as the session entity: MapRenderer reads Session.Entity.position while rendering
MapViewer.spot = Session.Entity = new Entity();

/**
 * @var {HTMLElement} <select>
 */
MapViewer.dropDown = null;

/**
 * Initialize MapViewer
 */
MapViewer.init = function Init() {
	// Increase max intersection test (because of the max zoom)
	Altitude.MAX_INTERSECT_COUNT = 500;

	let ready = false;
	let maptoload = '';
	const q = new Queue();

	// Resources sharing
	if (Configs.get('api')) {
		q.add(function () {
			function onAPIMessage(event) {
				if (typeof event.data !== 'object') {
					return;
				}
				// Only accept messages from the parent window or opener (the page that loaded us)
				if (event.source !== window.parent && event.source !== window.opener) {
					return;
				}

				switch (event.data.type) {
					case 'init':
						BGM.setAvailableExtensions(['mp3']);
						Thread.delegate(event.source, event.origin);
						Thread.init();
						Renderer.init();
						q._next();
						break;

					case 'load':
						MapRenderer.currentMap = '';

						if (ready) {
							MapRenderer.setMap(event.data.data.replace('data/', ''));
						} else {
							maptoload = event.data.data.replace('data/', '');
						}
						if (MapViewer.dropDown && MapViewer.dropDown.parentNode) {
							document.body.removeChild(MapViewer.dropDown);
						}
						event.stopPropagation();
						break;

					case 'stop': {
						const gl = Renderer.getContext();
						MapRenderer.free();
						Renderer.stop();
						gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
						event.stopPropagation();
						break;
					}
				}
			}

			window.addEventListener('message', onAPIMessage, false);
		});
	}

	// Normal access
	else {
		// Waiting for the Thread to be ready
		q.add(function () {
			BGM.setAvailableExtensions(['mp3']);
			Thread.hook('THREAD_READY', q.next);
			Thread.init();
		});

		// Initialize renderer
		q.add(function () {
			Renderer.init();
			q._next();
		});

		// Start Intro, wait the user to add files
		q.add(function () {
			Intro.onFilesSubmit = function (files) {
				Client.onFilesLoaded = q.next;
				Client.init(files);
			};
			Intro.append();
		});
	}

	// Start the MapViewer instance
	q.add(function () {
		Intro.remove();

		MapRenderer.onLoad = MapViewer.onLoad;
		MapControl.init();
		MapControl.onRequestWalk = MapViewer.onMouseDown;

		// Direct access from API
		if (Configs.get('api')) {
			ready = true;
			maptoload = maptoload || location.hash.substr(1).replace('data/', '');
			MapRenderer.setMap(maptoload);
			return;
		}

		MapRenderer.setMap('guild_vs4.rsw');

		// Initialize dropdown
		Client.search(/data\\([^\0]+\.rsw)/gi, function (mapList) {
			let i, count;

			mapList.sort();

			MapViewer.dropDown = document.createElement('select');
			MapViewer.dropDown.style.zIndex = 50;
			MapViewer.dropDown.style.position = 'relative';

			for (i = 0, count = mapList.length; i < count; ++i) {
				mapList[i] = mapList[i].substr(5); // Remove 'data\\' part
				MapViewer.dropDown.add(new Option(mapList[i], mapList[i]), null);
			}

			MapViewer.dropDown.onchange = function OnChange() {
				MapRenderer.setMap(this.value);
				document.body.removeChild(MapViewer.dropDown);
			};
		});
	});

	// Start queue system
	q.run();
};

/**
 * Once map is ready to render
 */
MapViewer.onLoad = function () {
	BGM.stop();

	if (!Configs.get('api')) {
		document.body.appendChild(MapViewer.dropDown);
	}

	MapViewer.spot.position[0] = Altitude.width >> 1;
	MapViewer.spot.position[1] = Altitude.height >> 1;
	MapViewer.spot.position[2] = Altitude.getCellHeight(MapViewer.spot.position[0], MapViewer.spot.position[1]);

	Camera.setTarget(MapViewer.spot);
	Camera.init();

	Camera.altitudeTo = -200;
	Camera.zoomFinal = 200;
};

/**
 * Overwrite Map Engine mouse down
 */
MapViewer.onMouseDown = function OnMouseDown() {
	if (Mouse.world.x > -1 && Mouse.world.y > -1) {
		MapViewer.spot.position[0] = Mouse.world.x;
		MapViewer.spot.position[1] = Mouse.world.y;
		MapViewer.spot.position[2] = Mouse.world.z;
	}
};

export default MapViewer.init;

MapViewer.init();

# Getting started

This guide has the goal to help you to Setup/Play RoBrowser. If there's any trouble during the process, please open an [issue](https://github.com/MrAntares/roBrowserLegacy/issues/new) with all your concerns and points to improve.

- [Getting started](#getting-started)
- [1. Important notes before starting](#1-important-notes-before-starting)
- [2. Local Installation](#2-local-installation)
    - [2.1 Prerequisites](#21-prerequisites)
- [3. Setup Instructions](#3-setup-instructions)
    - [3.1 Setup: NodeJS and NVM](#31-setup-nodejs-and-nvm)
    - [3.2 Setup: Node wsproxy](#32-setup-node-wsproxy)
    - [3.3 Setup: Game Files](#33-setup-game-files)
    - [3.4 Setup: Game Server](#34-setup-game-server)
    - [3.5 Setup: RoBrowser](#35-setup-robrowser)
    - [3.6 Build Information](#36-build-information)
- [4. Serving and Running roBrowser](#4-serving-and-running-robrowser)
    - [4.1 Serving Game: Live Server/Production](#41-serving-game-live-serverproduction)
    - [4.2 Serving Game: Development](#42-serving-game-development)
    - [4.3 Building via CLI](#43-building-via-cli)
- [5. Add game assets](#5-add-game-assets)
    - [Local Assets](#local-assets)
    - [Remote Client](#remote-client)
- [6. Adding Custom Plugins](#6-adding-custom-plugins)
- [7. ROBrowser Settings Overview](#7-robrowser-settings-overview)
    - [7.1 Configuration Files](#71-configuration-files)
    - [7.2 Configuration Options](#72-configuration-options)
    - [7.3 Configuration webserver adress](#73-configuring-the-webserver-api)
- [8. Play the Game](#8-play-the-game)
- [9. Troubleshooting](#9-troubleshooting)
    - [9.1 Troubleshooting: The screen is weird and/or the developer console (F12) says it can't load game assets](#91-troubleshooting-the-screen-is-weird-andor-the-developer-console-f12-says-it-cant-load-game-assets)
    - [9.2 Troubleshooting: Screen is blank](#92-troubleshooting--screen-is-blank)
    - [9.3 Troubleshooting: AI%5cConst.js (404 not found)](#93-troubleshooting-ai5cconstjs-404-not-found)
    - [9.4 Troubleshooting: Wrong Response Status - 403 Not Found, 403 Forbidden](#94-troubleshooting--wrong-response-status---403-not-found-403-forbidden)
    - [9.5 Troubleshooting: Other](#95-troubleshooting-other)
- [10. FAQ](#10-faq)
    - [10.1 Which client/packet version should I use?](#101-which-clientpacket-version-should-i-use)

# 1. Important notes before starting

![Browser Console with Packets exchange](img/browser-console.png)
In case of any error start by opening developer mode and check the browser `Console` (`F12` or `CTRL+Shift+I` in most browsers). Don't forget to adjust the level filters if you don't see everything. Also worth checking the `Network` tab.

> For public servers using secure web protocols `https` and `wss` is a de-facto must, since most browsers don't allow non-secure websocket calls on the internet anymore.

# 2. Local Installation

This guide section will help you running robrowser locally.

## 2.1 Prerequisites

Here's a list of things that you will need to have for a sucessfull installation and gameplay of your favorite server.

| Item            | Description                                                                                                                                                                                                                                                                                         |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| RoBrowserLegacy | [Client](https://github.com/MrAntares/roBrowserLegacy) to Run Ragnarok Online on browser                                                                                                                                                                                                            |
| NodeJS 22+      | [NodeJS](https://nodejs.org/en/download/current) to compile RoBrowser assets                                                                                                                                                                                                                        |
| wsproxy         | A [Websocket Proxy](https://github.com/herenow/wsProxy#readme) to interligate Login + Server + Map into one endpoint                                                                                                                                                                                |
| Game Server     | Your Emulator that usually is [rAthena](https://github.com/rathena/rathena) or [Hercules](https://github.com/HerculesWS/Hercules/)                                                                                                                                                                  |
| Game Files      | A [kRO](https://rathena.org/board/topic/106413-kro-full-client-2023-04-04-includes-bgm-rsu/) or any RO installation files to use as your base for the server                                                                                                                                        |
| Remote Client   | If you don't want to use local files you will need a Remote Client to serve the game assets to roBrowser hia http. Default version: [PHP](https://github.com/MrAntares/roBrowserLegacy-RemoteClient-PHP) Other versions: [JS](https://github.com/FranciscoWallison/roBrowserLegacy-RemoteClient-JS) |
| Browser         | Any browser that is compatible with [OpenGL ES 2.0](https://caniuse.com/?search=opengl) (Requirement for RoBrowser)                                                                                                                                                                                 |
| Embed Server    | Any programming language that has embed servers to use. Example: NodeJS, PHP or Ruby. It's totally up to you.                                                                                                                                                                                       |

Currently tested browsers:

- Chrome: _(Desktop & Mobile)_
- FireFox: _(Desktop & Mobile)_
- Safari: _(Desktop & Mobile)_
- Edge: _(Desktop)_

> Others will probably work as well especially if **Chromium based**, but there might be slight differences.

# 3. Setup Instructions

## 3.1 Setup: NodeJS and NVM

> [!NOTE]
> If you already have NodeJS LTS installed, [skip this step](#RoBrowser).

Make sure you're running NodeJS LTS or 22 above. Validate your version using the command:

```shell
node --version
# v22.10.0 (example output)
```

If your Node version is below 22, you can use NVM (Node Version Manager) to install a newer version.

To install NVM:

```shell
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.0/install.sh | bash
```

After installing NVM, restart your terminal and verify the installation:

```bash
nvm --version
# 0.39.0
```

Now install the latest LTS version of Node:

```bash
nvm install --lts
# Downloading and installing node latest version
# Now using node v22.10.0 (npm v10.2.3)
```

Verify that Node is at the correct version:

```bash
node --version
# v22.10.0
npm --version
# v10.2.3
```

Now you're good to go and compile the needed files to your Ro-Browser application!

## 3.2 Setup: Node wsproxy

![Ro Browser App flow fluxogram](img/ro-browser-app-flow.png)

To run **roBrowserLegacy** it will be necessary a proxy layer between your browser and the _map_server_, _login_server_, and _char_server_. Instead connect into 3 instances, you will only connect to the main proxy.

To install **wsproxy**, use the command below:

```shell
npm install wsproxy -g
```

For more info read [the wsProxy Readme.](https://github.com/herenow/wsProxy#readme)

## 3.3 Setup: Game Files

![Nemo's platform listing all Clients](img/client-list.png)

All client/packet versions are supported, but the number of missing features increases with higher dates, since we need time to get everything implemented. We advise you to use versions older than `2015` for the best experience, but this is not a restriction, only a suggestion.

The game files needs to match the version/episode that you will play. Due that, we recommend to always start with **kRO Client** or you can choose a different Client dowloading from one of **Hercules assets** on [Nemo](http://nemo.herc.ws/downloads/) website.

## 3.4 Setup: Game Server

You need a game server that is compatible with the original game. There are many implementations/versions/forks that are compatible, you can use any of them, but we suggest using one of the following two, because we test using these emulators:

- [rAthena](https://github.com/rathena/rathena)
- [Hercules](https://github.com/HerculesWS/Hercules/)

> [!WARNING]
>
> - Disable packet_obfuscation on the game server. (Not supported yet, causes invalid packets)

## 3.5 Setup: RoBrowser

First, you will need the project in your machine. You can fork your own version or just clone the repository by typing:

```
git clone https://github.com/MrAntares/roBrowserLegacy.git
```

Or you can just download our latest release [here.](https://github.com/MrAntares/roBrowserLegacy/archive/refs/heads/master.zip)

Having the project extracted in your machine, you will need to install the node dependencies by running:

```shell
npm install
```

## 3.6 Build Information

The project uses a custom build system based on Vite and Rollup for bundling ES6 modules into optimized files for production.

### Build Commands

- `npm run build` - Build the default application (Online.js) using the custom builder.
- `npm run build:all` - Build all applications.
- `npm run build:all:minify` - Build all applications with minification enabled.
- `npm run build:pwa` - Build the Progressive Web App (PWA) version, including manifest and service worker.
- `npm run build:nw` - Build NW.js desktop executables for Windows (x86 and x64).

### Specific Application Builds

- `npm run build:online` - Build Online.js (full game client).
- `npm run build:mapviewer` - Build MapViewer.js.
- `npm run build:grfviewer` - Build GrfViewer.js.
- `npm run build:modelviewer` - Build ModelViewer.js.
- `npm run build:strviewer` - Build StrViewer.js.
- `npm run build:effectviewer` - Build EffectViewer.js.
- `npm run build:threadhandler` - Build ThreadEventHandler.js.
- `npm run build:html` - Generate HTML files only.
- `npm run build:ai` - Build AI scripts only.

### Custom Builder Options

The build system is powered by `applications/tools/builder-web.mjs`. You can run it directly:

- `node ./applications/tools/builder-web.mjs` - Basic build (Online.js).
- `node ./applications/tools/builder-web.mjs -O -H` - Build Online.js, ThreadEventHandler.js, and HTML.
- `node ./applications/tools/builder-web.mjs --all` - Build all apps.
- `node ./applications/tools/builder-web.mjs --all --m` - Build all with minification.

Supported flags:

- `--all` or `-a`: Build all applications.
- `--m`: Enable minification with Terser.
- Specific app flags: `-O` (Online), `-V` (MapViewer), `-G` (GrfViewer), `-M` (ModelViewer), `-S` (StrViewer), `-E` (EffectViewer), `-T` (ThreadEventHandler), `-H` (HTML), `-PWA` (PWA build).

Output is generated in the `dist/Web/` directory for web builds and `dist/Desktop/` for NW.js builds.

# 4. Serving and Running roBrowser

Now we have everything to make our game run as expected. Before start, we have to start a web-server to host all our roBrowserLegacy. By default, our project already has one installed in the node dependencies as Vite dev server, configured under the project's [package.json](../package.json) pre-configured commands.

Running the dev server:

```bash
npm run live

# robrowser@1.0.0 live
# > vite --open applications/browser-examples/index.html
#   VITE vX.X.X  ready in XXX ms
#   ➜  Local:   http://localhost:3000/
```

Or if you're using PHP or Ruby you can type any of these commands:

```shell
php -S 0.0.0.0:8000

# or

ruby -run -ehttpd . -p8000
```

With Vite, access `http://localhost:3000` for dev or `http://localhost:4173` for serve. With PHP or Ruby, access `http://localhost:8000`.

## 4.1 Serving Game: Live Server/Production

There's two builds available today: Development and Production.

This step/section is only recommended for a "Live" server. It will only pack all the resource files into one file to speed up loading.

First, we have to access the **roBrowserLegacy** project root:

```bash
cd path/to/robrowserlegacy
```

Then you can simply run:

```bash
npm run build -- -O -H
```

Second build preserving html file and config.js you can use:

```bash
npm run build -- -O
```

It will generate 3 files inside `dist/Web`, which are **Online.js**, **ThreadEventHandler.js** and **index.html** that you have to move/copy into your **project root**.

```
cp dist/Web/* .
```

Now you're good to open your browser at `http://localhost:3000` and see your roBrowser base client running!

> [!TIP]
> In case you want to minify your roBrowser, go on the instance config and turn the `development` flag into `false`.

## 4.2 Serving Game: Development

![RO Browser running in Chromium](img/ro-browser-dev.png)

If you want to contribute directly on RoBrowserLegacy source, you should focus on `/src` path.

First, go to your roBrowserLegacy Folder:

```shell
cd path/to/robrowserlegacy
```

For a true development server with hot reload, use:

```shell
npm run live
# > vite --open applications/browser-examples/index.html
```

Or run Vite directly:

```shell
npm run dev
# > vite
```

`npm run serve` is a preview of the built app (not the full HMR dev mode):

```shell
npm run serve
# > robrowser@1.0.0 serve
# > vite preview
#   ➜  Local:   http://localhost:4173/
```

Since we're under development purposes (modifying the source/testing), set `development: true` in `dist/Web/index.html` when using the preview mode.

## 4.3 Building via CLI

The browser-based builder from the RequireJS era has been removed. Use the CLI builder instead:

```bash
node ./applications/tools/builder-web.mjs --all
```

See Section 3.6 for all available build commands and flags.

# 5. Add game assets

TODO: improve this flow (important!)

## Local Assets

- Enable Intro screen in the configs by setting `skipIntro: false`
- Drag your game assets into the box on your Intro sceen

## Remote Client

- Install a Remote Client to your web server.
    - Available versions:
        - [PHP](https://github.com/MrAntares/roBrowserLegacy-RemoteClient-PHP)
        - [JS](https://github.com/FranciscoWallison/roBrowserLegacy-RemoteClient-JS)

- copy your `.grf` under `resources` directory (only unencripted, 0x200 version is supported)
- alternatively, if you don't want to use GRFs directly, then you can extract your GRFs into the `data` directory (not recommended, but it works fine)
- copy your `DATA.INI` (GRF loading order) under `resources` directory
- copy your BGM `.mp3` under `BGM` directory
- copy your data directory under `data` directory
- copy your System files under `System` directory
- copy your `clientinfo.xml` (client-server information) under `data` directory
- copy your AI files under `AI` directory
- check the `configs.php` if it is configured properly for your use and make sure `DEBUG` is set to false
- check the `.htaccess` file if the ErrorDocument option points to the `index.php` via the correct url. If you don't run roBrowser from the www root and you use remote client then you need to adjust this url (see examples in the file)

# 6. Adding Custom Plugins

- copy your custom plugins into `src\Plugins`

Some examples: https://github.com/MrAntares/roBrowserLegacy-plugins

# 7. ROBrowser Settings Overview

## 7.1 Configuration Files

roBrowser uses a two-file configuration system that separates default settings from local customizations:

| File              | Purpose                                                                                                |
| ----------------- | ------------------------------------------------------------------------------------------------------ |
| `Config.js`       | Default configuration shipped with roBrowser. This file can be overwritten during updates/deployments. |
| `Config.local.js` | **Optional.** Your local overrides that persist across updates. This file is gitignored.               |

### Setting Up Your Configuration

1. **For basic setups**: Edit `Config.js` directly in `applications/pwa/`

2. **For production/deployment setups** (recommended):
    - Keep `Config.js` with sensible defaults
    - Copy `Config.local.js.example` to `Config.local.js`
    - Add your server-specific settings to `Config.local.js`

Example `Config.local.js`:

```js
window.ROConfigLocal = {
	servers: [
		{
			display: 'My Server',
			address: '192.168.1.100',
			port: 6900,
			packetver: 20180620,
			socketProxy: 'ws://my-proxy.example.com'
		}
	],
	skipIntro: true,
	remoteClient: 'https://my-grf-server.com/'
};
```

Settings in `Config.local.js` are deep-merged with `Config.js`, so you only need to specify the values you want to override.

## 7.2 Configuration Options

Here you have a list with all of init variables on ROBrowser. Let's take a look:

```js
var ROConfig = {
	// GLOBAL VARIABLES
	type: ROBrowser.TYPE.FRAME, // Possible: .FRAME (instert into current document), .POPUP (open new window)
	target: document.getElementById('robrowser'), // When using TYPE.FRAME this is the id of the target iframe in the current document
	application: ROBrowser.APP.ONLINE, // Possible: .ONLINE (game), .MAPVIEWER, .GRFVIEWER, .MODELVIEWER, .STRVIEWER, .GRANNYMODELVIEWER (not implemented)
	width: 800, // Only affects TYPE.POPUP
	height: 600, // Only affects TYPE.POPUP

	// SERVER CONFIG
	servers: [
		{
			// Game server info. You must configure this! You can have multiple servers like: servers: [{..}, {..}, {..}], you can also use clientinfo to set the server list (servers: "data/clientinfo.xml",)
			display: 'Demo Server', // Display name, can be anything
			desc: "roBrowser's demo server", // Description, can be anything
			address: '127.0.0.1', // Must match your game server's
			port: 6900, // Must match your game server's
			langtype: 12, // Will determine your game files charpage
			packetver: 20191223, // Must match your game server's
			grfList: 'DATA.INI', // By default uses DATA.INI to get grf list, but you can define an array (grfList: ['custom.grf', 'palette.grf', 'data.grf'],) or a regex (grfList: /\.grf$/i,)
			remoteClient: 'http://127.0.0.1/client', // Your remote client address. Defaults to https://grf.robrowser.com/
			renewal: true, // Must match your game server's type (true/false). When using clientinfo.xml you can add the <renewal>true</renewal> custom tag.
			packetKeys: false, // Packet encryption keys ( not implemented?? )
			socketProxy: 'ws://127.0.0.1:5999/', // The websocket proxy's address you set up previously for robrowser (wsproxy)
			adminList: [2000000], // List admins' account IDs here like: [2000000, 2000001, 2000002 .... etc]
			aura: { defaultLv: 99 } // optional aura levels
		}
	],

	// OTHER CONFIG - These can be part of the server config as well, thus making them adjustable per server
	development: false, // When false needs a compiled Online.js in the root (faster load). When true, the client will directly use the javascript files from src/ (slower load, for debugging/development only). Additionally, the console output is enabled when set to true to help debugging.
	enableConsole: false, // Enables the console output in a non-development environment (development: false)
	disableConsole: false, // Force disables console output in any environment (regardless or the development config option)
	packetDump: false, // Dump packet as hex to the console?
	skipServerList: false, // Skip server selection?
	skipIntro: false, // Skip intro page?
	version: 20230927.0959, // Update this value every time you update your robrowser, to trigger a source refresh on every browser. Recommended to use a decimal timestamp (YYYYMMDD.hhmm), but can be anything
	forceUseAddress: false, // Some times when connect to server, the server return 127.0.0.1 as IP to connect. This key force robrowse to ignore and use IP set at 'address'
	worldMapSettings: {
		// Settings for world map.
		episode: 98, // Episode content to show (0-98, eg:14.2, default:98 = latest)
		add: [], // Optional, Array of maps to custom show  (eg: ['rachel', 'ra_fild01'])
		remove: [] // Optional, Array of maps to custom remove (eg: ['alberta', 'pay_fild03'])
	},
	enableAchievements: false, // Enable Achievements UI
	enableCashShop: false, // Enable Cash Shop UI?
	enableBank: false, // Enable Bank UI? (Requires PACKETVER 20130724 above)
	enableMapName: false, // Enable Map Name Banner? (Requires client data (GRF) newer than 2019.06.19)
	enableRefineUI: false, // Enable Renewal Refine UI? (Requires client data (GRF) newer than 2016.10.12) (Should also enable in server side)
	enableDmgSuffix: false, // Enable Damage Suffix (>1M = K, >100M = M) - Requires client data (GRF) newer or equals to 2019.05.08
	enableCheckAttendance: false, // Enable Check Attendance? (Requires PACKETVER 20180307 above)
	enableHomunAutoFeed: false, // Enable Homunculus Auto Feed for older PACKETVER than 20170920
	loadLua: false, // Enable this option to load LUA tables (currently only item table) from client/System/...
	customItemInfo: ['kRO.lua', 'jRO.lua', 'lua files514/iteminfo.lua'], // Customized iteminfo array-list, it loads using firt to last priority

	//clientHash:    '113e195e6c051bb1cfb12a644bb084c5', // Set fixed client hash value here (less secure, for development only)
	calculateHash: false, // When true, the client will calculate it's own hash and send that value (slower, more secure, only when development is false). Must provide the list of files in hashFiles!
	hashFiles: ['api.html', 'api.js', 'Online.js', 'ThreadEventHandler.js'], // List of files to calculate the Hash based on. Add all files your robrowser uses, including your "main page/index.html" where you set up the RoBrowser Config. Only used when calculateHash is true.
	transitionDuration: 500, // used for fade out and fade in when teleport/change map
	/* Plugins */
	plugins: {
		/* Syntax */
		// Simple (no parameters):
		// PluginName: 'Plugin_JS_Path_In_PluginsFolder_Without_Extension',
		// Complex (with configurable parameters):
		// PluginName: { path:'Plugin_JS_Path_In_PluginsFolder_Without_Extension', pars: { param1: <val1>, param2: <val2>, param3: <val3>... } }
		/* Example: */
		// KeyToMove: 'KeyToMove/KeyToMove'
		// IntroMessage: { path:'IntroMessage/IntroMessage', pars: { newsUrl: 'https://yourserver.com/news/news.html' } }
	},

	/* Custom, "for fun" camera modes */
	ThirdPersonCamera: false, // When true you can zoom in more and rotate camera around player more freely with mouse
	FirstPersonCamera: false, // When true you can look from the player's head, like an FPS game and rotate camera without limit
	CameraMaxZoomOut: 5 // How far can you zoom out the camera, default:5. Note: Extreme values can break camera and/or mouse.
};
```

> **Note:** When using `Config.js` and `Config.local.js`, use `window.ROConfigBase` and `window.ROConfigLocal` respectively. The `type` and `application` options should use string values (`'FRAME'`, `'ONLINE'`) which are converted to their constants at runtime.
> In case of the `langtype` option, we added support for some custom types:

Normal

- 160: Central European
- 161: Greek
- 162: Turkish
- 163: Hebrew
- 164: Estonian, Latvian, Lithuaninan

Unicode

- 240: UTF-8
- 241: UTF-16LE
- 242: UTF-16BE

**Using these custom types makes roBrowser incompatible with other clients without modifying them as well! Only use them if you know what you are doing!**

You can set up your own `index.html` / integrate roBrowser into your website as well based on the .examples/ and this example above.

## 7.3 Configuring the WebServer API

ROBrowser automatically detects the runtime environment:

- Browser (http / https) → uses Vite proxy
- Electron / app:// / file:// → uses `webserverAddress`

No manual switching is required.

## Web version (Vite)

Configure the WebServer IP in:

```
vite.config.js
```

```js
proxy: {
  '/emblem': {
    target: 'http://127.0.0.1:8888'
  },
  '/userconfig': {
    target: 'http://127.0.0.1:8888'
  }
}
```

Change to your server:

```js
target: 'http://YOUR_SERVER_IP:8888';
```

This works for both:

```
http://localhost:3000
https://localhost:3000
```

No additional configuration required.

## Web version — Production

In production, you need to configure a reverse proxy on your web server to forward /emblem and /userconfig requests to the emulator's WebServer.

Nginx

```
location /emblem {
    proxy_pass http://127.0.0.1:8888;
    proxy_set_header Host $host;
}
location /userconfig {
    proxy_pass http://127.0.0.1:8888;
    proxy_set_header Host $host;
}
```

Apache

```
ProxyPass /emblem http://127.0.0.1:8888/emblem
ProxyPassReverse /emblem http://127.0.0.1:8888/emblem
ProxyPass /userconfig http://127.0.0.1:8888/userconfig
ProxyPassReverse /userconfig http://127.0.0.1:8888/userconfig
Requires mod_proxy and mod_proxy_http enabled.
```

Replace 127.0.0.1:8888 with your emulator's WebServer address.

This also solves mixed-content issues — your site serves over HTTPS and the reverse proxy handles the HTTP connection to the emulator internally.

## Electron version

Electron does not use the Vite proxy. Configure:

```js
webserverAddress: 'http://127.0.0.1:8888';
```

Example:

```js
webserverAddress: 'http://192.168.0.10:8888';
```

Browser builds ignore this setting.

**Note:** The emulator WebServer only supports HTTP. Always use `http://` in `webserverAddress`.

## Automatic behavior

| Environment            | Uses proxy       | Uses webserverAddress |
| ---------------------- | ---------------- | --------------------- |
| http (vite)            | yes              | no                    |
| https (vite)           | yes              | no                    |
| Production (nginx etc) | You configure it | no                    |
| electron (app://)      | no               | yes                   |

## 8. Play the Game

- Access to `http://localhost:3000/` after moving all the files from your `ro-browser/dist/Web` to `ro-browser` (project root).

# 9. Troubleshooting

## 9.1 Troubleshooting: The screen is weird and/or the developer console (F12) says it can't load game assets

Your remote client is not configured properly.

- Check the `client/configs.php` and make sure `DEBUG` is set to false.
- Check the `client/.htaccess` file if the ErrorDocument option points to the `client/index.php` via the correct url. If you don't run roBrowser from the www root and you use remote client then you need to adjust this url (see examples in the file).

If it is still not working you can try setting `DEBUG` to true and open the `http://localhost:3000/client/index.php` in your browser to see the debug trace. You can also call files directly from your game data to see if they load properly, eg: `http://localhost:3000/client/data/texture/black.bmp`. After debugging set `DEBUG` to false.

## 9.2 Troubleshooting: Screen is blank

Check that you don't have an extension using [postMessage](https://developer.mozilla.org/en-US/docs/Web/API/Window/message_event), it will conflict with code in `api.html` which listen for message.

## 9.3 Troubleshooting: AI%5cConst.js (404 not found)

![](img/start-ai-error.png)

You probably forgot the step about `AI` `require` replacement in `Add game assets` section

## 9.4 Troubleshooting: Wrong Response Status - 403 Not Found, 403 Forbidden

You probably have a server security issue if your server is public. Check your certificates and make sure you configured everything to run securely, you provided the required configuration values in `https`/`wss` and that the main page of roBrowser is also opened with `https`. Redirecting every `http` call to `https` on the webserver is also probably a good idea.

## 9.5 Troubleshooting: Other

I personally had to disable `metamask` extension.

# 10. FAQ

## 10.1 Which client/packet version should I use?

Robrowser is not like the desktop client. It doesn't have a fixed client version. It inherits version from the date closest (behind) to the set date, thus every version is supported at any time. The real question is how many features are implemented at any set date? We continously add features and work on new additions, but generaly, the older the version date, the more complete the feature list. The "latest" supported version is what you can login with and play, without the game falling apart. You have to try it, because it is always evolving and it depends on your server's source's version. If you want a "close enough" version then use a date 4~5 years old or check the packet structure and versions in the source in the `src/Network` folder. You can check the UI modules as well in `src/UI/Components`. We add every old and new UI and feature tied to it's appropriate date, so you can time travel to any point in history using the clientver date and see how the client used to look like in old episodes. So really the question should be: What episode you want to use? Set the date accordingly. You can find the episode/update dates online. Also keep in mind that the client date must be equal to the server's date, so changes in packets don't cause problems.

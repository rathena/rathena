## ROBrowser Legacy

This is a continuation of [roBrowser](https://www.robrowser.com/) expanded with some features. This repo is not directly forked from the original repository due to safety concerns, but it is roBrowser.

If you wish to discuss anything related to this project, or you want to join, contact us on [Discord](https://discord.gg/8JdHwM4Kqm) or in the [GIT Discussions](https://github.com/MrAntares/roBrowserLegacy/discussions)

For info on how to setup the client read the contents of our [Getting Started doc](https://github.com/MrAntares/roBrowserLegacy/blob/master/docs/README.md). For the original branche's docs please visit the https://www.robrowser.com/ site.

## DEMO

[![Start Demo](https://img.shields.io/badge/%E2%96%B6%20Start%20Demo-Play%20Now-e8b84b?style=for-the-badge&labelColor=cc0000)](https://mrantares.github.io/roBrowserLegacy/master)

_Use `<Username>_M` or `<Username>_F` to register a male or a female account on the login screen, or use the Register/Request button to navigate to the server's account registration page._

More live examples:

- [#robrowser-servers on Discord](https://discord.gg/MFtJj9n5Hr)
- [roBrowserLegacy Servers on Discussions](https://github.com/MrAntares/roBrowserLegacy/discussions/categories/robrowserlegacy-servers)

## Quick Start

```bash
git clone https://github.com/MrAntares/roBrowserLegacy.git
cd roBrowserLegacy
npm install
npm run live          # Dev server with Vite (opens browser)
npm run build:all     # Build all applications to dist/Web/
```

#### Repo info:

| ![GitHub](https://img.shields.io/github/license/MrAntares/roBrowserLegacy.svg) | ![commit activity](https://img.shields.io/github/commit-activity/w/MrAntares/roBrowserLegacy) | ![GitHub repo size](https://img.shields.io/github/repo-size/MrAntares/roBrowserLegacy.svg) | ![CodeQL](https://img.shields.io/github/actions/workflow/status/MrAntares/roBrowserLegacy/analysis_codeql.yml?label=CodeQL) | ![Build & Tests](https://img.shields.io/github/actions/workflow/status/MrAntares/roBrowserLegacy/build.yml?branch=master&label=Build%20%26%20Tests&logo=vitest) | ![Lint](https://img.shields.io/github/actions/workflow/status/MrAntares/roBrowserLegacy/lint.yml?branch=master&label=Lint&logo=eslint) | ![Format](https://img.shields.io/github/actions/workflow/status/MrAntares/roBrowserLegacy/format.yml?branch=master&label=Format&logo=prettier) |
| ------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------- |

## Guide

Checkout the [getting started guide](docs/README.md)

## Wiki

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/MrAntares/roBrowserLegacy)

## Tech Stack

- **ES6 Modules** — Modern `import`/`export` syntax (migrated from AMD/RequireJS)
- **Vite** — Build tool and dev server (replaced RequireJS optimizer and live-server)
- **WebGL** — 3D/2D rendering via OpenGL ES 2.0
- **WebSockets** — Network communication via wsProxy
- **ESLint + Prettier** — Code quality and formatting
- **Web Workers** — Background processing for GRF decompression and pathfinding

## Remote Client

Remote Client serves game assets to roBrowser via http by extracting them from their GRFs. You will need to setup a remote client if you want to serve the game assets centrally from your server. roBrowser can use local game assets via the Intro screen by dragging them into the file box. The original implementation of the Remote Client is written in PHP:

- [roBrowserLegacy-RemoteClient-PHP](https://github.com/MrAntares/roBrowserLegacy-RemoteClient-PHP)

Other implementations may arise and when they do we will list them here:

- [roBrowserLegacy-RemoteClient-JS](https://github.com/FranciscoWallison/roBrowserLegacy-RemoteClient-JS)
- [roBrowserLegacy-RemoteClient-Rust](https://github.com/Flux159/roBrowserLegacy-RemoteClient-Rust)

## WebSocket Proxy

The game server uses TCP/IP to communicate with the client, while roBrowser being a web page can't use TCP/IP. We use the WebSocket API to communicate with a proxy server that translates the packets into TCP/IP packets. This server is called wsProxy. You will need to install and configure wsProxy to make roBrowser able to connect to a game server. For more info, please visit the [roBrowserLegacy-wsProxy](https://github.com/MrAntares/roBrowserLegacy-wsProxy) repository.

## Plugins

For available plugins and information on how to install them please visit the [roBrowserLegacy-plugins](https://github.com/MrAntares/roBrowserLegacy-plugins) repository.

## Contributing

See [CONTRIBUTING](./docs/CONTRIBUTING.md)

All credits to the original owners/creators and the new ones.
<a href="https://github.com/MrAntares/roBrowserLegacy/graphs/contributors">
<img src="https://contrib.rocks/image?repo=MrAntares/roBrowserLegacy" />
</a>

## Contact

- Join us on [Discord](https://discord.gg/8JdHwM4Kqm)
- Or in the [GIT Discussions](https://github.com/MrAntares/roBrowserLegacy/discussions)

## Star History

<a href="https://www.star-history.com/?repos=MrAntares%2FroBrowserLegacy&type=date&legend=top-left">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/chart?repos=MrAntares/roBrowserLegacy&type=date&theme=dark&legend=top-left&sealed_token=n-LYAsHxuqeB0pWAcr0VyUok1yv6L_vgDU_o45SAzDD0v8KE-9l9Mqlcr0dcnX606Bg9Yf_Coue-KXc4lhBj_dwAIAVmxloWaG5KiVlETd2asCYPRWu9-RzCKESTbAwZOolsyYjnA-IS6njNlSjYzTJfSDLMmT742KWezS2E7iECIz-sCoDor0QaRlep" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/chart?repos=MrAntares/roBrowserLegacy&type=date&legend=top-left&sealed_token=n-LYAsHxuqeB0pWAcr0VyUok1yv6L_vgDU_o45SAzDD0v8KE-9l9Mqlcr0dcnX606Bg9Yf_Coue-KXc4lhBj_dwAIAVmxloWaG5KiVlETd2asCYPRWu9-RzCKESTbAwZOolsyYjnA-IS6njNlSjYzTJfSDLMmT742KWezS2E7iECIz-sCoDor0QaRlep" />
   <img alt="Star History Chart" src="https://api.star-history.com/chart?repos=MrAntares/roBrowserLegacy&type=date&legend=top-left&sealed_token=n-LYAsHxuqeB0pWAcr0VyUok1yv6L_vgDU_o45SAzDD0v8KE-9l9Mqlcr0dcnX606Bg9Yf_Coue-KXc4lhBj_dwAIAVmxloWaG5KiVlETd2asCYPRWu9-RzCKESTbAwZOolsyYjnA-IS6njNlSjYzTJfSDLMmT742KWezS2E7iECIz-sCoDor0QaRlep" />
 </picture>
</a>

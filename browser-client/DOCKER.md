# Docker Deployment

roBrowserLegacy ships two Docker image variants and a unified server
([roBrowserLegacy-RemoteClient-JS](https://github.com/FranciscoWallison/roBrowserLegacy-RemoteClient-JS))
that handles three responsibilities in a single process:

- Serving the roBrowserLegacy web client as static files
- Serving GRF game assets over HTTP (the "Remote Client")
- Proxying WebSocket connections to the rAthena TCP game server

---

## All-in-One quick start (rAthena + roBrowser + Remote Client)

For a single-VM setup that runs rAthena, the roBrowserLegacy dev frontend, and
the PHP Remote Client together, use `docker-compose.AIO.yaml`.

### 1. Drop your rAthena source

Copy your rAthena source into the `rathena/` folder **without overwriting** the
existing files in it (e.g. `rathena/DropYourRemoteClientHereDontOverride.txt`).

### 2. Drop your Remote Client

Copy your remote client assets into the `roBrowserLegacy-RemoteClient-PHP/`
folder (see `roBrowserLegacy-RemoteClient-PHP/DropYourRemoteClientHere.txt`).

### 3. Change Target build PACKETVER in docker-compose.AIO

Find the line "BUILDER_CONFIGURE" to change to your packet version builder target

```bash
BUILDER_CONFIGURE: '--enable-packetver=20250618' # <- Configure your wanted packet version here
```

### 4. Build and run

```bash
docker compose -f docker-compose.AIO.yaml up --build
```

### 5. Open the game

http://localhost:3000/applications/browser-examples/demo.html

---

## Image variants

| Variant  | Tag suffix                 | Size    | GRF assets                 |
| -------- | -------------------------- | ------- | -------------------------- |
| **slim** | `-slim` e.g. `:1.0.0-slim` | ~400 MB | Must be mounted at runtime |
| **full** | _(none)_ e.g. `:1.0.0`     | ~5 GB   | Baked in at build time     |

> **The slim image is recommended for most deployments.**
>
> GRF files (`.grf`) are copyrighted by Gravity Co., Ltd. Baking them into a
> publicly distributed Docker image would constitute redistribution of
> copyrighted game assets, which is not permitted. The full image should only
> be used in **private, self-hosted registries** where the image is never
> pushed to a public registry.
>
> The full image is also only built when `GRF_BASE_URL`, `S3_ACCESS_KEY_ID`,
> and `S3_SECRET_ACCESS_KEY` secrets are configured in your CI environment.
> If these secrets are absent, only the slim image is built - the full job
> skips gracefully with no build failure.

Both variants are built from `Dockerfile.remoteclient`. Build and publish
them to your own container registry using the instructions in
[Building images locally](#building-images-locally) or set up a CI pipeline
following the [CI/CD](#cicd-github-actions) section below.

---

## Quick start

### 1. Create your server config

```bash
cp applications/pwa/Config.local.js.example Config.local.js
```

Edit `Config.local.js` - at minimum set:

```js
address:   '127.0.0.1',   // your rAthena login server IP
port:      6900,
packetver: 20200401,       // must match your rAthena PACKETVER
renewal:   true,           // true = Renewal, false = Pre-renewal/Classic
```

> **`forceUseAddress: true` is critical** for any non-localhost deployment.
> Without it, the client uses the internal IP returned by the char/map server
> packets, which is unreachable outside the server's local network.

### 2. Run the full image (GRFs baked in)

```bash
ROBROWSER_TAG=1.0.0 docker compose -f docker-compose.remoteclient.yaml up -d
```

### 3. Run the slim image (GRFs mounted from host)

Uncomment the GRF volume mounts in `docker-compose.remoteclient.yaml`, set `GRF_PATH`, then:

```bash
ROBROWSER_TAG=1.0.0-slim \
GRF_PATH="/path/to/kRO client" \
docker compose -f docker-compose.remoteclient.yaml up -d
```

Or directly with `docker run`:

```bash
GRF="/path/to/kRO client"

docker run -d --name robrowserlegacy --network host \
  -v "${GRF}/data.grf:/app/resources/data.grf:ro" \
  -v "${GRF}/rdata.grf:/app/resources/rdata.grf:ro" \
  -v "${GRF}/DATA.INI:/app/resources/DATA.INI:ro" \
  -v "${GRF}/BGM:/app/BGM:ro" \
  -v "$(pwd)/Config.local.js:/robrowser/Config.local.js:ro" \
  ghcr.io/<your-github-user>/robrowserlegacy:latest-slim
```

### 4. Open the game

```
http://localhost:3338/
```

---

## Configuration reference

`Config.local.js` is loaded by the browser at runtime and merged over the
defaults in `src/Config.js`. Copy `applications/pwa/Config.local.js.example`
to get started.

| Field             | Description                                                                |
| ----------------- | -------------------------------------------------------------------------- |
| `address`         | Login server IP or hostname                                                |
| `port`            | Login server port (default: `6900`)                                        |
| `packetver`       | Must match rAthena `PACKETVER` (e.g. `20200401`)                           |
| `renewal`         | `true` for Renewal, `false` for Pre-renewal/Classic                        |
| `socketProxy`     | WebSocket proxy URL - points at this container (`ws://localhost:3338/ws/`) |
| `remoteClient`    | GRF asset server URL - points at this container (`http://localhost:3338/`) |
| `forceUseAddress` | **Set `true`** for any containerised or NAT deployment                     |
| `skipIntro`       | Skip the intro video (`false` by default)                                  |
| `skipServerList`  | Auto-select first server (`false` by default)                              |

> **Remote access:** If your browser is on a different machine than the
> container, replace `localhost` with the container host's IP or hostname in
> `socketProxy` and `remoteClient`, and set `CLIENT_PUBLIC_URL` to the same
> base URL when running the container.

---

## Networking

By default the embedded WebSocket proxy only forwards to `127.0.0.1:6900`,
`127.0.0.1:6121`, and `127.0.0.1:5121`. This means the container must be able
to reach rAthena on localhost.

**Host networking (Linux, simplest)**

```bash
docker run -d --network host ...
```

rAthena and the container share the host network stack so `127.0.0.1` works
as-is. Not available on Docker Desktop for macOS/Windows.

**Bridge networking or remote rAthena (Kubernetes, Docker Compose, macOS/Windows)**

Set `WS_ALLOWED_TARGETS` to the real host:port list of your rAthena server:

```bash
docker run -d \
  -e WS_ALLOWED_TARGETS="10.0.0.5:6900,10.0.0.5:6121,10.0.0.5:5121" \
  -e CLIENT_PUBLIC_URL="http://<your-host>:3338" \
  ...
```

On Docker Desktop (macOS/Windows) use the magic hostname:

```bash
-e WS_ALLOWED_TARGETS="host.docker.internal:6900,host.docker.internal:6121,host.docker.internal:5121"
```

> **Security note:** `WS_ALLOWED_TARGETS` is an explicit allowlist. Only
> `host:port` pairs listed here can be reached through the proxy - arbitrary
> targets are still rejected.

> **Dependency note:** `WS_ALLOWED_TARGETS` support requires
> [FranciscoWallison/roBrowserLegacy-RemoteClient-JS#14](https://github.com/FranciscoWallison/roBrowserLegacy-RemoteClient-JS/pull/14)
> to be merged upstream. The `Dockerfile.remoteclient` in this PR will be
> updated to pin to that commit once it is merged.

### Container environment variables

| Variable              | Default                    | Description                                                                                  |
| --------------------- | -------------------------- | -------------------------------------------------------------------------------------------- |
| `PORT`                | `3338`                     | Port the server listens on                                                                   |
| `CLIENT_PUBLIC_URL`   | `http://localhost:3338`    | Public base URL of this container (used for CORS)                                            |
| `WS_ALLOWED_TARGETS`  | _(empty - localhost only)_ | Comma-separated `host:port` list the WS proxy may forward to. See [Networking](#networking). |
| `ENABLE_WSPROXY`      | `true`                     | Enable the embedded WebSocket proxy                                                          |
| `ENABLE_STATIC_SERVE` | `true`                     | Serve the roBrowserLegacy web client as static files                                         |
| `CACHE_MAX_FILES`     | `10000`                    | Max number of GRF files to keep in memory cache                                              |
| `CACHE_MAX_MEMORY_MB` | `2048`                     | Memory budget for the GRF file cache                                                         |
| `CACHE_WARM_UP`       | `true`                     | Pre-populate cache on startup                                                                |
| `CACHE_WARM_UP_LIMIT` | `500`                      | Max files to pre-load during warm-up                                                         |

---

## Building images locally

### Slim (no GRFs, ~400 MB)

```bash
docker buildx build \
  -f Dockerfile.remoteclient \
  --build-arg BUILD_VERSION=dev \
  -t robrowserlegacy:dev-slim \
  --load .
```

### Full (GRFs baked in, ~5 GB)

GRF assets are fetched from a private HTTPS endpoint using
[BuildKit secrets](https://docs.docker.com/build/building/secrets/) - they are
never written to any image layer or visible in `docker history`.

Prepare tarballs once from your kRO client directory:

```bash
KRO="/path/to/kRO client"
tar -czf ragnarok-grfs.tar.gz -C "$KRO" data.grf rdata.grf DATA.INI
tar -czf ragnarok-bgm.tar.gz  -C "$KRO" BGM
# Upload both to your HTTPS host
```

Then build:

```bash
S3_ACCESS_KEY_ID=your-key \
S3_SECRET_ACCESS_KEY=your-secret \
GRF_BASE_URL=https://your-host/bucket/prefix \
docker buildx build \
  -f Dockerfile.remoteclient \
  --build-arg BUILD_VERSION=1.0.0 \
  --secret id=grf_base_url,env=GRF_BASE_URL \
  --secret id=s3_access_key_id,env=S3_ACCESS_KEY_ID \
  --secret id=s3_secret_access_key,env=S3_SECRET_ACCESS_KEY \
  --target full \
  -t robrowserlegacy:1.0.0 \
  --load .
```

---

## CI/CD (GitHub Actions)

`docker.yml` builds and publishes both variants to GHCR on every tag push.

Required repository secrets:

| Secret                 | Description                           |
| ---------------------- | ------------------------------------- |
| `GRF_BASE_URL`         | Base HTTPS URL of the GRF asset store |
| `S3_ACCESS_KEY_ID`     | S3/MinIO access key ID                |
| `S3_SECRET_ACCESS_KEY` | S3/MinIO secret access key            |

If the GRF secrets are absent, only the slim image is built - the full job
skips gracefully.

Trigger a build by pushing a semver tag:

```bash
git tag v1.0.0 && git push origin v1.0.0
```

Images published:

```
ghcr.io/<owner>/robrowserlegacy:1.0.0        # full, linux/amd64 + linux/arm64
ghcr.io/<owner>/robrowserlegacy:1.0.0-slim   # slim, linux/amd64 + linux/arm64
ghcr.io/<owner>/robrowserlegacy:latest       # full, latest tag
ghcr.io/<owner>/robrowserlegacy:latest-slim  # slim, latest tag
```

---

## What assets are needed

| File        | Required | Notes                                                      |
| ----------- | -------- | ---------------------------------------------------------- |
| `data.grf`  | Yes      | Main game assets - textures, maps, sprites, DB tables      |
| `rdata.grf` | Yes      | kRO-specific patches                                       |
| `DATA.INI`  | Yes      | Tells the server which GRF files to load and in what order |
| `BGM/`      | No       | Background music MP3s - game is silent without it          |

`data/`, `System/`, and `AI/` directories from the kRO client are **not
needed** - their contents are already inside `data.grf` for a standard
unmodified client. Custom servers with patched loose files can mount them as
additional volumes.

---

## Health check

Both image variants include a `HEALTHCHECK` that polls `http://localhost:PORT/`
every 30 seconds with a 60-second start period (to allow GRF indexing to
complete). Use `docker ps` to monitor status:

```
robrowserlegacy   Up 2 minutes (healthy)
```

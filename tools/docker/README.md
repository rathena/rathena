# Docker

Note that this Dockerized environment **is not suitable** for production deployments, see [Installations](https://github.com/rathena/rathena/wiki/installations) instead.

### Quick Start

```bash
cd tools/docker
make init
make build
make admin
```

### Setup Steps

1. `make init` - Start the database and builder containers (ensure port `3306` is free)
2. `make build` - Compile the servers and start/restart game services
3. `make admin` - Create an admin account (defaults: `admin`/`admin`/`M`/`admin@rathena.dev`)
4. `make stop` - Stop all containers when done

### Account Management

Create an admin account (full GM powers, group 99):
```bash
make admin                          # Defaults: admin/admin/M/admin@rathena.dev
# OR
make admin NAME=bob PASS=secret SEX=F EMAIL=bob@example.com
```

Create a regular player account:
```bash
make account NAME=player1 PASS=secret123 SEX=M EMAIL=player@example.com
```

### Other Commands

| Command | Description |
|---------|-------------|
| `make logs` | Attach to container output/logs (Ctrl+C to exit) |
| `make start` | Start all containers at once (db, builder, and game servers) |
| `make stop` | Stop all containers |
| `make shell` | Open a shell in the builder container for development |
| `make ps` | Check container status |
| `make nuke` | Stop containers and **delete the database** (start fresh) |

### Tips & Tricks

- Ensure you don't have a database running locally and listening on port `3306` this will cause the database container to fail starting up.
- All file edits within the repository are reflected inside the container, so you can develop in your preferred text editor or IDE.
  - Files into `./asset` take precedence over `conf/import/` counterpart
- Connect to the local database with following credentials:
  - Host: `localhost`
  - Port: `3306`
  - User: `ragnarok`
  - Password: `ragnarok`
- On first start up all `/sql-files/*.sql` files are imported into the database. This does not happen on future start ups unless the volume has been deleted.
- Database is saved to local disk so state is persisted between shutdowns and start ups. To fully erase your database and start fresh, use `make nuke`.
- If you have modified the `Dockerfile`, be sure to rebuild the docker image with `docker compose build`

# Docker

Note that this Dockerized environment **is not suitable** for production deployments, see [Installations](https://github.com/rathena/rathena/wiki/installations) instead.

## How to setup a local development environment :computer:

Runs the `builder` container, which builds the `rathena:local` Docker image and runs it as a container to configure and compile the server.
```bash
docker-compose up builder
```

Runs the server containers `db`, `login`, `chat`, and `map` (skips builder).
Ensure port `3306` is free.
```bash
docker-compose up
```

Optionally use -d (detached) for releasing the terminal.
```bash
docker-compose up -d
```

Closes the server containers and releases resources (if using detached).
```bash
docker-compose down
```

## Tips & tricks for local development :beginner:

All commands expect you to have change directory to this directory, `$projectRoot/tools/docker`.

### Builder Configuration

Change the value of `BUILDER_CONFIGURE` environment variable of the `builder` service in order to change the parameters sent to the `./configure` command.

```yaml
builder:
    # omitting other configs
    environment:
        BUILDER_CONFIGURE: "--enable-packetver=20211103"
```

### Re-Compiling the server

If you have already compiled the project once, you might want to connect directly to the builder service and run commands from there.

```bash
# runs only the builder container opening a bash terminal
docker-compose run builder bash
```

Alternativelly you can just execute the `builder` container.

```bash
# runs the builder.sh script which compiles the server again
docker-compose up builder
```

### Tips & tricks for local development :beginner:

- Ensure you don't have a database running locally and listening on port `3306` this will cause the database container to fail starting up.
- All file edits within the repository are reflected inside the container, so you can develop in your preferred text editor or IDE.
  - Files into ./asset take precedence over conf/import/ counterpart
- Connect to the local database with following credentials:
  - Host: `localhost`
  - Port: `3306`
  - User: `ragnarok`
  - Password: `ragnarok`
- On first start up all `/sql-files/*.sql` files are imported into the database. This does not happen on future start ups unless the volume has been deleted.
- Database is saved to local disk so state is persisted between shutdowns and start ups. To fully erase your database and start fresh, delete the volume with `docker-compose down --volumes`
- Check the status of containers with `docker-compose ps`
- If you have modified the `Dockerfile`, be sure to rebuild the docker image with `docker-compose build`

### F.A.Q

`ls: can't open '.': Permission denied` turn off selinux.

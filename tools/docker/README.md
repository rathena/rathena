# Docker

Note that this docker environment has a bit unusual setup given the nature of the Docker tool itself.

> [!IMPORTANT]
> Source files have been stripped out of the resulting container (it doesn't make sense to ship the full source to production, right?).

### How to setup a local development environment :computer:

1. Run either `patch.ps1` or `patch.sh` based on your host OS. These are for generating the import folders and patching the configs we need to make docker containers talk to each other.
2. `docker-compose up -d` to spin up dev container and database (ensure port `3306` is free)
3. `docker-compose down` outside the dev container when done to close database and free resources
4. Change the value of `BUILDER_CONFIGURE` environment variable of the `login` service in order to change the parameters sent to the `./configure` command

### Rebuilding the source :wrench:

1. Take down the services with `docker-compose down`
2. Rebuild everything with `docker-compose up [-d] --build`

#### Tips & tricks for local development :beginner:

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

### Shipping to production

Since now you have the container compiled with the binaries ready to go, the only thing you need to get this up and running in your VPS is to:

1. Push the image to your registry [ref](https://docs.docker.com/reference/cli/docker/image/push/)
2. Copy over `docker-compose.yml` and update the `image: "rathena:local"` to match your `registry:tag`
3. Then its business as usual (`docker-compose up -d`)

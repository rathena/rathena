# Docker

Note that this Dockerized environment **is not suitable** for production deployments, see [Installations](https://github.com/rathena/rathena/wiki/installations) instead.

### How to setup a local development environment :computer:

1. `docker-compose up -d` to spin up dev container and database (ensure port `3306` is free)
2. `docker exec -it rathena bash` to connect to dev container
3. All rAthena development commands can be executed inside the dev container, such as compiling (`./configure`, `make clean server`) and starting the server (`./athena-start`, `gdb map-server`, etc ...)
4. `docker-compose down` outside the dev container when done to close database and free resources

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

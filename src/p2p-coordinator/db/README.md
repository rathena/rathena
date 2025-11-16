# Database Implementation for AI-MMORPG World

## PostgreSQL Schema

- All DDL for required tables is in [`schema_postgres.sql`](./schema_postgres.sql).
- Partitioning is set up for `combat_logs` and `ownership_migrations` (by month).
- Foreign keys, constraints, and indexes are included for data integrity and performance.

## Migrations

- Migration scripts are in [`migrations/`](./migrations/).
  - `V1__init_schema_up.sql`: Applies the initial schema.
  - `V1__init_schema_down.sql`: Rolls back the initial schema.
- Use a migration tool (e.g., Flyway, sql-migrate) or run manually with `psql`:
  - `psql -d <dbname> -f migrations/V1__init_schema_up.sql`
  - `psql -d <dbname> -f migrations/V1__init_schema_down.sql`

## DragonflyDB/Redis Initialization

- Initialization script: [`redis_init.lua`](./redis_init.lua)
- Key patterns, TTLs, GeoHash indexing, and cache warming are defined.
- Run with `redis-cli --eval db/redis_init.lua` or integrate into your app's startup.

## Integration

- No existing database access code was found; integration points are ready for new development.
- Use the provided schemas and key patterns for all future database interactions.

## Testing

1. **PostgreSQL**
   - Create a test database.
   - Apply the schema and migration scripts.
   - Verify all tables, partitions, and constraints are created.
   - Test rollback by running the down migration.

2. **DragonflyDB/Redis**
   - Start a DragonflyDB/Redis instance.
   - Run the initialization script.
   - Verify key creation, TTLs, and GeoHash indexing.

## Notes

- All scripts are designed to be idempotent and safe for production use.
- Review partition creation for each new month as needed.
- For further schema changes, add new migration scripts with both up and down logic.

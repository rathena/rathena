# rAthena Metrics Collection (Prometheus)

This setup collects map-server metrics exported to `./log/map_metrics.prom` via node-exporter's textfile collector.

## Services

- `map-server` writes metrics to `/rathena/log/map_metrics.prom`
- `node-exporter` reads that file and exposes it on `:9100/metrics`
- `prometheus` scrapes `node-exporter` and exposes UI on `:9090`
- `grafana` connects to Prometheus and exposes UI on `:3000`

## Start

From `rathena/tools/docker`:

```bash
docker compose up -d node-exporter prometheus grafana
```

## Verify

```bash
curl -s http://127.0.0.1:9100/metrics | rg "map_|server_"
curl -s http://127.0.0.1:9090/api/v1/targets
```

Prometheus UI:

- <http://127.0.0.1:9090>

Grafana UI:

- <http://127.0.0.1:3000>
- Login: `admin` / `admin`
- The `rAthena Metrics` dashboard is provisioned automatically in folder `rAthena`.

## Notes

- Metrics export settings are configured in:
  - `conf/import/map_conf.txt`
  - `tools/docker/asset/map_conf.txt`
- Export path:
  - `./log/map_metrics.prom`

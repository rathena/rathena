param(
    [ValidateSet('start', 'stop', 'status', 'logs', 'build')]
    [string]$Action = 'start'
)
$ErrorActionPreference = 'Stop'
$compose = Join-Path $PSScriptRoot 'compose.yml'
switch ($Action) {
    'start' { & docker compose -f $compose up -d --build }
    'stop' { & docker compose -f $compose down }
    'status' { & docker compose -f $compose ps }
    'logs' { & docker compose -f $compose logs --tail 100 -f }
    'build' { & docker compose -f $compose build login }
}
if ($LASTEXITCODE -ne 0) { throw "Docker Compose falhou (codigo $LASTEXITCODE). Verifique se o Docker Desktop esta iniciado." }

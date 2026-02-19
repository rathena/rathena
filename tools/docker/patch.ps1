# ----------- Copy import-tmpl folders ----------
$paths = @(
    @{ Src = "conf/import-tmpl"; Dst = "conf/import" },
    @{ Src = "db/import-tmpl"; Dst = "db/import" },
    @{ Src = "conf/msg_conf/import-tmpl"; Dst = "conf/msg_conf/import" }
)

foreach ($p in $paths) {
    if (-not (Test-Path $p.Dst)) {
        New-Item -ItemType Directory -Path $p.Dst | Out-Null
    }

    Get-ChildItem $p.Src | ForEach-Object {
        $target = Join-Path $p.Dst $_.Name
        if (-not (Test-Path $target)) {
            Copy-Item $_.FullName $target -Recurse
        }
    }
}

# ----------- Patch conf files in place ----------
function Patch-ConfFile {
    param(
        [string]$File,
        [hashtable]$KeyValues
    )

    $content = Get-Content $File

    foreach ($kv in $KeyValues.GetEnumerator()) {
        $key = $kv.Key
        $value = $kv.Value
        $found = $false

        for ($i = 0; $i -lt $content.Length; $i++) {
            if ($content[$i] -match "^$key:") {
                $content[$i] = "$key: $value"
                $found = $true
                break
            }
        }

        if (-not $found) {
            $content += "$key: $value"
        }
    }

    Set-Content $File $content
}

Patch-ConfFile "conf/import/char_conf.txt" @{ login_ip="login"; char_ip="127.0.0.1" }
Patch-ConfFile "conf/import/inter_conf.txt" @{
    login_server_ip="db";
    ipban_db_ip="db";
    char_server_ip="db";
    map_server_ip="db";
    log_db_ip="db"
}
Patch-ConfFile "conf/import/map_conf.txt" @{ char_ip="char"; map_ip="127.0.0.1" }

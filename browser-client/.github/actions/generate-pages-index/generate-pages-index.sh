#!/usr/bin/env bash
set -euo pipefail

PAGES_DIR="${1:-.}"
cd "$PAGES_DIR"

mapfile -t DIRS < <(find . -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | sort)
has_dir() { [ -d "$1" ]; }

{
  echo '<!doctype html>'
  echo '<html lang="en">'
  echo '<head>'
  echo '  <meta charset="utf-8" />'
  echo '  <meta name="viewport" content="width=device-width, initial-scale=1" />'
  echo '  <title>Branch Demo Browser</title>'
  echo '  <style>'
  echo '    body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Arial,sans-serif;max-width:980px;margin:40px auto;padding:0 16px;}'
  echo '    h1{margin:0 0 8px;}'
  echo '    .hint{color:#666;margin:0 0 24px;}'
  echo '    .featured{display:flex;gap:12px;flex-wrap:wrap;margin:18px 0 26px;}'
  echo '    .pill{display:inline-flex;align-items:center;gap:10px;padding:12px 14px;border-radius:999px;border:1px solid #ddd;background:#fff;text-decoration:none;color:#111;font-weight:700;}'
  echo '    .pill:hover{border-color:#999;}'
  echo '    .tag{font-size:12px;padding:2px 8px;border-radius:999px;background:#e8b84b;color:#111;font-weight:800;}'
  echo '    ul{list-style:none;padding:0;margin:0;}'
  echo '    li{margin:10px 0;padding:12px 14px;border:1px solid #ddd;border-radius:10px;display:flex;justify-content:space-between;gap:12px;align-items:center;}'
  echo '    a{color:#0969da;text-decoration:none;font-weight:600;}'
  echo '    a:hover{text-decoration:underline;}'
  echo '    .name{font-family:ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;font-size:13px;color:#111;}'
  echo '  </style>'
  echo '</head>'
  echo '<body>'
  echo '  <h1>Branch Demo Browser</h1>'
  echo '  <p class="hint">Available deployed branch previews (folders on <code>gh-pages</code>).</p>'

  echo '  <div class="featured">'
  if has_dir "main"; then
    echo '    <a class="pill" href="./main/"><span class="tag">MAIN</span><span>Open main demo</span></a>'
  fi
  if has_dir "master"; then
    echo '    <a class="pill" href="./master/"><span class="tag">MASTER</span><span>Open master demo</span></a>'
  fi
  echo '  </div>'

  echo '  <ul>'
  for d in "${DIRS[@]}"; do
    if [ "$d" = "main" ] || [ "$d" = "master" ]; then
      continue
    fi
    if [ "$d" = ".git" ] || [ "$d" = ".github" ] || [ "$d" = ".ssh" ]; then
      continue
    fi
    if [ "$d" = "." ] || [ "$d" = ".." ]; then
      continue
    fi
    echo "    <li><span class=\"name\">${d}</span><a href=\"./${d}/\">Open demo</a></li>"
  done
  echo '  </ul>'
  echo '</body>'
  echo '</html>'
} > index.html
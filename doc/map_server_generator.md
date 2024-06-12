# Map Server Generator

This is a tool to generate files that are hard to manually create.

## How to run
### Linux
Run `make tools`.
This creates a new binary called `map-server-generator`.

It can be ran with: `./map-server-generator`

### Windows
It can be ran with `./map-server-generator.exe`, or with the provided `.bat` files.

## Available options
On Linux, prefix with `--`

On Windows, prefix with `/`

option | feature
---|---
`generate-navi` | create navigation files
`generate-reputation` | create reputation bson files
`generate-itemmoveinfo` | create itemmoveinfov5.txt



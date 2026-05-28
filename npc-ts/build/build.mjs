// Replacement for the bare `esbuild` CLI call. Wires in the
// auto-await plugin so authors can write sync-looking `ctx.mes(...)`
// calls and the bundler inserts the awaits.

import { build } from "esbuild";
import { autoAwaitPlugin } from "./auto-await-plugin.mjs";

const watch = process.argv.includes("--watch");

const opts = {
    entryPoints: ["main.ts"],
    bundle: true,
    platform: "neutral",
    target: "es2022",
    format: "iife",
    outfile: "dist/main.js",
    sourcemap: true,
    // Pick up tsconfig's `paths` (`@api`, `@lib/*`) — esbuild reads
    // the same config TypeScript does.
    tsconfig: "tsconfig.json",
    plugins: [autoAwaitPlugin],
};

if (watch) {
    const ctx = await (await import("esbuild")).context(opts);
    await ctx.watch();
    console.log("auto-await build watching…");
} else {
    await build(opts);
    console.log("auto-await build complete.");
}

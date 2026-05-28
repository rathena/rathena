// esbuild plugin that rewrites `ctx.METHOD(...)` into
// `await ctx.METHOD(...)` so script authors don't have to sprinkle
// `await` everywhere. Runs at build time on every .ts file in the
// bundle.
//
// Why this exists: V8 has no user-level fibers — we can't actually
// turn an unresolved Promise into a blocking call from a sync
// function. The next best thing is to hide the await keyword: the
// runtime is still async, but the author writes:
//
//     async onClick(ctx) {
//         ctx.mes("Hi!");
//         const c = ctx.select(["yes", "no"]);
//         if (c === 0) ctx.mes("OK");
//         ctx.close();
//     }
//
// and we emit the equivalent with `await` inserted in front of every
// `ctx.X(` call. `ctx.player.name`, `ctx.npc.x` and other non-call
// member accesses are untouched.
//
// Caveats (acceptable for the POC, would need a proper AST pass for
// production):
//   • Comments containing `ctx.mes(...)` get rewritten. Harmless —
//     they stay inside the comment.
//   • String literals containing `ctx.mes(...)` get rewritten. Rare
//     in NPC scripts; would corrupt the string if it happens.
//   • `ctx.player.mes(...)` would NOT be rewritten because the regex
//     anchors on `ctx.NAME(`. If you add nested awaitable methods
//     under ctx.*, this needs to expand.

import { readFile } from "node:fs/promises";

// Match `ctx.someName(` not already preceded by `await `.
// JavaScript regex supports lookbehind in V8 / modern Node.
const CTX_CALL = /(?<!await\s)\bctx\.([a-zA-Z_$][\w$]*)\(/g;

export const autoAwaitPlugin = {
    name: "auto-await",
    setup(build) {
        build.onLoad({ filter: /\.ts$/ }, async (args) => {
            const source = await readFile(args.path, "utf8");
            // Skip files under `lib/` — those are real ES modules
            // authored with explicit async/await. The transform is
            // for NPC scripts (one-off, await-heavy) where the magic
            // is welcome; library code wants honest types.
            if (/\/lib\//.test(args.path)) {
                return { contents: source, loader: "ts" };
            }
            const contents = source.replace(CTX_CALL, "await ctx.$1(");
            return { contents, loader: "ts" };
        });
    },
};

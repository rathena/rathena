// Player-scoped helpers — gate checks (min level, item requirements,
// zeny), permanent-counter bumps, etc. Import from `@lib/player`.
//
// Lib code uses explicit `async` + `await` (the auto-await build
// transform skips this directory). `awaitable(ctx)` re-types ctx
// so its dialog primitives have honest Promise-returning signatures.

import { awaitable } from "./internal";

/**
 * Require the player to meet a minimum base level. Shows a dialog and
 * closes the conversation if the check fails. Returns `true` only if
 * the player passes — caller can early-return on `false`.
 *
 * @example
 * if (!(await requireLevel(ctx, 80))) return;
 * // …level-80+ content goes here…
 */
export async function requireLevel(ctx: NpcContext, minLevel: number): Promise<boolean> {
    if (!ctx.player) return false;
    if (ctx.player.baseLevel < minLevel) {
        const a = awaitable(ctx);
        await a.mes(`You need to be at least level ${minLevel} for this.`);
        await a.close();
        return false;
    }
    return true;
}

/**
 * Require the player to have at least one of the named jobs.
 *
 * @example
 * if (!(await requireJob(ctx, 4002, 4003, 4004))) return;
 */
export async function requireJob(ctx: NpcContext, ...jobIds: number[]): Promise<boolean> {
    if (!ctx.player) return false;
    if (!jobIds.includes(ctx.player.classId)) {
        const a = awaitable(ctx);
        await a.mes("Your class can't undertake this.");
        await a.close();
        return false;
    }
    return true;
}

/**
 * Require the player to carry at least `amount` of `itemId`.
 *
 * @example
 * if (!(await requireItem(ctx, 7227, 5))) return;  // need 5 Royal Jelly
 */
export async function requireItem(
    ctx: NpcContext,
    itemId: number,
    amount = 1,
): Promise<boolean> {
    if (!ctx.player) return false;
    if (ctx.player.countItem(itemId) < amount) {
        const a = awaitable(ctx);
        await a.mes(`You need ${amount}× item ${itemId} for this.`);
        await a.close();
        return false;
    }
    return true;
}

/**
 * Bump a persistent character counter (rAthena bare `var`). Returns
 * the new value.
 *
 * @example
 * const visits = bumpPermCounter(ctx, "kafraVisits");
 * ctx.mes(`This is your visit #${visits}.`);
 */
export function bumpPermCounter(
    ctx: NpcContext,
    key: string,
    by = 1,
): number {
    if (!ctx.player) return 0;
    const next = (Number(ctx.player.perm[key]) || 0) + by;
    ctx.player.perm[key] = next;
    return next;
}

/**
 * Charge the player a zeny cost. Returns `true` and deducts; returns
 * `false` if the player can't afford it (and closes with a message).
 *
 * @example
 * if (!(await chargeZeny(ctx, 1_000_000))) return;
 * ctx.mes("Thanks for your business!");
 */
export async function chargeZeny(ctx: NpcContext, cost: number): Promise<boolean> {
    if (!ctx.player) return false;
    if (ctx.player.zeny < cost) {
        const a = awaitable(ctx);
        await a.mes(`You need ${cost.toLocaleString()}z. You only have ${ctx.player.zeny.toLocaleString()}z.`);
        await a.close();
        return false;
    }
    ctx.player.zeny -= cost;
    return true;
}

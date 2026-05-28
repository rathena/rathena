// Internal helper for lib authors.
//
// The exported NpcContext in @api types its dialog methods as
// SYNC (returning `void` / `number`) so NPC scripts read sync after
// the auto-await build transform inserts the awaits. Lib code is
// NOT processed by that transform — it's real TS — so it needs to
// see the methods as Promise-returning to legitimately `await` them.
//
// `awaitable(ctx)` reshapes the same runtime object so its methods
// have honest async types. The cast is safe: the C++ host always
// returns Promises for the dialog primitives.

/**
 * `NpcContext` re-typed so the dialog primitives return Promises.
 * Methods on sub-objects (`ctx.player`, `ctx.world`, …) are left
 * untouched — most are non-suspending stubs and the lib author can
 * still use the sync-looking shape.
 */
export type AwaitableNpcContext = Omit<
    NpcContext,
    "mes" | "next" | "select" | "menu" | "close" | "input" | "inputString" | "sleep" | "doevent"
> & {
    mes(text: string): Promise<void>;
    next(): Promise<void>;
    select(options: string[]): Promise<number>;
    menu(options: string[]): Promise<number>;
    close(): Promise<void>;
    input(min?: number, max?: number, defaultValue?: number): Promise<number>;
    inputString(defaultValue?: string): Promise<string>;
    sleep(milliseconds: number): Promise<void>;
    doevent(eventTarget: string): Promise<void>;
};

/**
 * Cast an `NpcContext` to its async-typed equivalent. Use in lib code
 * to write `await a.mes(...)` cleanly without per-call casts.
 *
 * @example
 * import { awaitable } from "./internal";
 * export async function confirm(ctx: NpcContext, prompt: string) {
 *     const a = awaitable(ctx);
 *     await a.mes(prompt);
 *     return (await a.select(["Yes", "No"])) === 0;
 * }
 */
export function awaitable(ctx: NpcContext): AwaitableNpcContext {
    return ctx as unknown as AwaitableNpcContext;
}

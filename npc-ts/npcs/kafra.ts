// Demonstrates the module-based scripting flow:
//   • imports types from `@api`
//   • imports shared helpers from `@lib/*`
//   • `ctx.X(...)` calls are sync-looking (the auto-await transform
//     adds the awaits at build time)
//   • lib helpers are async — calls need explicit `await` (that's the
//     honest module-boundary; the transform doesn't follow imports)

import type { NpcRegistration } from "@api";
import { confirm, paginatedMes } from "@lib/dialog";
import { bumpPermCounter, requireLevel } from "@lib/player";
import { formatZeny } from "@lib/format";
// Ported global helpers — direct equivalents of rAthena's F_X functions.
import { F_Hi, F_GetNumSuffix, F_InsertPlural, F_SexMes } from "@lib/global";

export const tsGuide: NpcRegistration = {
    name: "ts_guide#prontera",
    map: "prontera",
    x: 155,
    y: 180,
    dir: 4,
    sprite: 757,
    async onClick(ctx) {
        if (!ctx.player) return;
        const p = ctx.player;

        // Sync helper from @lib/player.
        const visits = bumpPermCounter(ctx, "tsGuideVisits");

        // Async helpers: explicit await — lib calls cross the module
        // boundary where the auto-await magic stops.
        await paginatedMes(ctx, [
            // F_Hi → random greeting (from @lib/global).
            // F_SexMes → sex-conditioned greeting.
            // F_InsertPlural → "1 visit" / "5 visits", auto-pluralized.
            // F_GetNumSuffix → "1st" / "22nd" / "113th".
            [`[TS Guide]`, `${F_Hi()} ${F_SexMes(ctx, "Ma'am", "Sir")} ${p.name}!`,
             `This is your ${F_GetNumSuffix(visits)} visit (${F_InsertPlural(visits, "visit")} total).`],
            [`Level ${p.baseLevel} / job ${p.jobLevel}.`, `You're carrying ${formatZeny(p.zeny)}.`],
        ]);

        if (!(await requireLevel(ctx, 1))) return;

        // ctx.X(...) is sync-looking — the build step adds the awaits.
        const choice = ctx.select([
            "Show my stats",
            "Show my vitals",
            "Show my wallet",
            "Nothing, bye",
        ]);

        ctx.mes("[TS Guide]");
        if (choice === 0) {
            ctx.mes(`STR ${p.str}   AGI ${p.agi}   VIT ${p.vit}`);
            ctx.mes(`INT ${p.int}   DEX ${p.dex}   LUK ${p.luk}`);
            ctx.mes(`Status points: ${p.statusPoint}`);
            ctx.mes(`Skill points: ${p.skillPoint}`);
        } else if (choice === 1) {
            ctx.mes(`HP: ${p.hp} / ${p.maxHp}`);
            ctx.mes(`SP: ${p.sp} / ${p.maxSp}`);
            ctx.mes(`Weight: ${p.weight} / ${p.maxWeight}`);
        } else if (choice === 2) {
            ctx.mes(`Zeny: ${formatZeny(p.zeny)}`);
            ctx.mes(`Party id: ${p.partyId || "(none)"}`);
            ctx.mes(`Guild id: ${p.guildId || "(none)"}`);

            if (await confirm(ctx, "Want me to announce this to the world?")) {
                ctx.world.announce(`${p.name} has ${formatZeny(p.zeny)}!`);
            }
        } else {
            ctx.mes("Take care!");
        }

        ctx.close();
    },
};

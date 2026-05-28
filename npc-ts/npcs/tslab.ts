// Test NPC that exercises wired-up host APIs across the four categories:
//   1. Mob spawn + setUnitData          (ctx.world.spawnMob / setUnitData / killMonster)
//   2. Reputation + permissions          (ctx.player.{get,set,add}Reputation / permission*)
//   3. Instance vars + cell ops          (ctx.instance.{create,getVar,setVar} / ctx.world.{setCell,checkCell,setWall,delWall})
//   4. BG waiting room + warpPortal      (ctx.bg.create / ctx.npc.createWaitingRoom / ctx.world.warpPortal)
//
// Park me anywhere — defaults to Prontera's main square so I'm easy
// to click during smoke testing.

import type { NpcRegistration } from "@api";
import { confirm } from "@lib/dialog";

// rAthena cell type for cell ops. CELL_NOVENDING bans Merchant vending
// on the tile — visible side effect without breaking pathing.
const CELL_NOVENDING = 6;
// cell_chk lookups
const CELL_CHKWALL    = 1;
const CELL_CHKNPC     = 9;

// setunitdata MOB type codes (script_constants).
const UMOB_LEVEL = 1;
const UMOB_MAXHP = 3;
const UMOB_SPEED = 8;

const PORING_MOB_ID = 1002;

export const tsLab: NpcRegistration = {
    name: "ts_lab#prontera",
    map: "prontera",
    x: 158,
    y: 180,
    dir: 4,
    sprite: 99,  // Lab assistant sprite
    async onClick(ctx) {
        if (!ctx.player) return;

        ctx.mes("[TS Lab]");
        ctx.mes("Welcome to the V8 scripting smoke test.");
        ctx.mes("Pick a subsystem to exercise:");

        const choice = ctx.select([
            "Mob spawn + setUnitData",
            "Reputation + permissions",
            "Instance vars + cell ops",
            "Battleground + warp portal",
            "Cancel",
        ]);

        if (choice === 0) await testMobs(ctx);
        else if (choice === 1) await testReputation(ctx);
        else if (choice === 2) await testInstanceAndCells(ctx);
        else if (choice === 3) await testBgAndPortal(ctx);

        ctx.close();
    },
};

async function testMobs(ctx: any) {
    const map = ctx.player.mapName;
    const px  = ctx.player.x;
    const py  = ctx.player.y;

    ctx.mes("[TS Lab]");
    ctx.mes(`Spawning 3 Porings near you on ${map} (${px}, ${py})…`);

    // Drop 3 porings within a few tiles so they're easy to see.
    const gid = ctx.world.spawnMob(map, px + 2, py, "TS-Poring", PORING_MOB_ID, 3);
    ctx.mes(`spawnMob returned gid=${gid}.`);

    // setUnitData on the *first* one: jack its level + MaxHP, slow it down.
    if (gid > 0) {
        ctx.world.setUnitData(gid, UMOB_LEVEL, 99);
        ctx.world.setUnitData(gid, UMOB_MAXHP, 9999);
        ctx.world.setUnitData(gid, UMOB_SPEED, 1000);
        ctx.mes("First Poring boosted to lv99 / 9999 HP / speed 1000.");
    }

    const counted = ctx.world.mobCount(map, "all");
    ctx.mes(`mobCount("${map}", "all") = ${counted}`);

    if (await confirm(ctx, "Wipe every mob on this map now?")) {
        const killed = ctx.world.killMonsterAll(map);
        ctx.mes(`killMonsterAll returned ${killed}.`);
    }
}

async function testReputation(ctx: any) {
    const p = ctx.player;
    // Type 1 = RepPointsOrc — first entry in db/re/reputation.yml.
    const repType = 1;

    ctx.mes("[TS Lab]");
    ctx.mes(`Reading reputation type ${repType}…`);
    const before = p.getReputation(repType);
    ctx.mes(`Current points: ${before}`);

    if (await confirm(ctx, "Add +5 reputation?")) {
        p.addReputation(repType, 5);
        ctx.mes(`After addReputation(+5): ${p.getReputation(repType)}`);
    }

    if (await confirm(ctx, `Reset reputation to ${before}?`)) {
        p.setReputation(repType, before);
        ctx.mes(`Reset complete: ${p.getReputation(repType)}`);
    }

    ctx.mes("[TS Lab]");
    ctx.mes("Permission check via per-player override.");
    const before_perm = p.permissionCheck("any_warp");
    ctx.mes(`Has "any_warp": ${before_perm ? "yes" : "no"}.`);

    if (!before_perm && await confirm(ctx, "Grant 'any_warp' for this session?")) {
        p.permissionAdd("any_warp");
        ctx.mes(`After grant: ${p.permissionCheck("any_warp") ? "yes" : "no"}`);
        ctx.mes("Use @warp <map> to verify. I'll revoke it on the next click.");
        p.permissionRemove("any_warp");
        ctx.mes(`After revoke: ${p.permissionCheck("any_warp") ? "yes" : "no"}`);
    }
}

async function testInstanceAndCells(ctx: any) {
    ctx.mes("[TS Lab]");
    ctx.mes("Creating an instance (party-mode)…");
    // mode 2 = IM_PARTY. If you aren't in a party this will fail gracefully.
    const iid = ctx.instance.create("Endless Tower", 2);
    ctx.mes(`instance.create returned ${iid}.`);

    if (iid > 0) {
        ctx.instance.setVar("ts_lab_counter", 42, iid);
        const v = ctx.instance.getVar("ts_lab_counter", iid);
        ctx.mes(`getVar("ts_lab_counter"): ${v}  (expected 42)`);

        ctx.instance.setVar("ts_lab_counter", 0, iid);
        const v2 = ctx.instance.getVar("ts_lab_counter", iid);
        ctx.mes(`After clear: ${v2}  (expected 0)`);
    }

    ctx.mes("[TS Lab]");
    ctx.mes("Cell op demo: blocking vending on the tile in front of me…");
    const map = ctx.player.mapName;
    const x = 158, y = 178;  // one square south of my position

    ctx.world.setCell(map, x, y, x, y, CELL_NOVENDING, true);
    const npcHere = ctx.world.checkCell(map, x, y, CELL_CHKNPC);
    const wallHere = ctx.world.checkCell(map, x, y, CELL_CHKWALL);
    ctx.mes(`Tile (${x},${y}) — NPC: ${npcHere}, wall: ${wallHere}.`);
    ctx.mes("Stand on it and /@mc to try to vend — should be denied.");

    if (await confirm(ctx, "Drop a temporary 5-cell wall here?")) {
        ctx.world.setWall(map, x - 2, y, 5, 6, false, "ts_lab_wall");
        ctx.mes("Wall placed (named 'ts_lab_wall').");
        if (await confirm(ctx, "Remove the wall?")) {
            ctx.world.delWall("ts_lab_wall");
            ctx.mes("Wall removed.");
        }
    }

    if (await confirm(ctx, "Clear the NOVENDING flag on that tile?")) {
        ctx.world.setCell(map, x, y, x, y, CELL_NOVENDING, false);
        ctx.mes("Done.");
    }
}

async function testBgAndPortal(ctx: any) {
    ctx.mes("[TS Lab]");
    ctx.mes("Creating a battleground waiting room…");

    // Title shows above the NPC; trigger=2 fires when 2 are in.
    // maxLvl 175 — server's MAX_LEVEL cap. Going above triggers the
    // chat_createnpcchat lvl/zeny guard and the room never opens.
    ctx.npc.createWaitingRoom("TS Lab BG", 4, "", 2, 0, 1, 175);
    const usersBefore = ctx.npc.getWaitingRoomUsers();
    ctx.mes(`Waiting room created. Current users: ${usersBefore}.`);

    if (await confirm(ctx, "Build a BG team out of whoever's in the room right now?")) {
        // bg_id 0 if empty; map "-" leaves cemetery unset.
        const bgid = ctx.bg.waitingRoomToBg("-", 0, 0, "", "", ctx.npc.npcInfo(3));
        ctx.mes(`waitingRoomToBg returned ${bgid}.`);
        if (bgid > 0) {
            const score = ctx.bg.getData(bgid, 0);
            ctx.mes(`Team member count: ${score}`);
        }
    }

    if (await confirm(ctx, "Remove the waiting room?")) {
        ctx.npc.removeWaitingRoom();
        ctx.mes("Removed.");
    }

    ctx.mes("[TS Lab]");
    if (await confirm(ctx, "Drop a warp portal next to me → prontera (150,150)?")) {
        // Source (sx, sy) relative to the NPC — pick a cell next to ts_lab.
        ctx.world.warpPortal(159, 180, "prontera", 150, 150);
        ctx.mes("Portal placed at (159,180). Walk into it to test.");
    }
}

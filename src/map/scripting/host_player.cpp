#ifdef HAVE_TS_SCRIPTING

#include "host_player.hpp"

#include "arg_helpers.hpp"
#include "../clif.hpp"
#include "../itemdb.hpp"
#include "../log.hpp"
#include "../map.hpp"
#include "../mob.hpp"
#include "../npc.hpp"
#include "../pc.hpp"
#include "../pc_groups.hpp"
#include "../status.hpp"
#include "../storage.hpp"
#include "../../common/mapindex.hpp"
#include "../../common/showmsg.hpp"
#include "../../common/socket.hpp"

namespace rathena::scripting {

namespace {
using namespace args;

// Convenience: every callback first unwraps the PlayerHost. Returns
// quietly if the host pointer is missing (shouldn't happen in practice).
#define UNWRAP \
    auto* self = args::unwrap<PlayerHost>(info); \
    if (!self) return; \
    auto& sd = self->sd();
} // namespace

// =====================================================================
// Heal / SP / AP
// =====================================================================

void PlayerHost::heal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int hp = int_arg(info, 0);
    int sp = int_arg(info, 1);
    status_heal(&sd.bl, hp, sp, 0, 1);
}

void PlayerHost::healAp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int ap = int_arg(info, 0);
    status_heal(&sd.bl, 0, 0, ap, 1);
}

void PlayerHost::itemHeal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int hp = int_arg(info, 0);
    int sp = int_arg(info, 1);
    // The itemheal-rate adjustment lives in pc_itemheal; status_heal
    // is the underlying packet. We use status_heal as the simplest
    // wiring — adjust later if itemheal_rate handling matters.
    status_heal(&sd.bl, hp, sp, 0, 1);
}

void PlayerHost::percentHeal_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int hp = int_arg(info, 0);
    int sp = int_arg(info, 1);
    status_percent_heal(&sd.bl, static_cast<int8>(hp), static_cast<int8>(sp));
}

void PlayerHost::recovery_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    // Simplest correct behavior: full revive + full heal.
    if (pc_isdead(&sd)) status_revive(&sd.bl, 100, 100);
    else status_percent_heal(&sd.bl, 100, 100);
}

// =====================================================================
// Experience / level
// =====================================================================

void PlayerHost::giveExp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_exp base = static_cast<t_exp>(int_arg(info, 0));
    t_exp job  = static_cast<t_exp>(int_arg(info, 1));
    pc_gainexp(&sd, nullptr, base, job, 0);
}

void PlayerHost::baseExpRatio_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int percent = int_arg(info, 0);
    auto job = job_db.find(sd.status.class_);
    if (!job) { ret_int(info, 0); return; }
    int level = int_arg(info, 1, sd.status.base_level);
    if (level < 1 || level > MAX_LEVEL) { ret_int(info, 0); return; }
    t_exp need = job->base_exp[level - 1];
    ret_int(info, static_cast<int32_t>(need * percent / 100));
}
void PlayerHost::jobExpRatio_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int percent = int_arg(info, 0);
    auto job = job_db.find(sd.status.class_);
    if (!job) { ret_int(info, 0); return; }
    int level = int_arg(info, 1, sd.status.job_level);
    if (level < 1 || level > MAX_LEVEL) { ret_int(info, 0); return; }
    t_exp need = job->job_exp[level - 1];
    ret_int(info, static_cast<int32_t>(need * percent / 100));
}

// =====================================================================
// Job / class
// =====================================================================

void PlayerHost::jobChange_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int job = int_arg(info, 0);
    pc_jobchange(&sd, job, JOBL_THIRD);
}

void PlayerHost::changeBase_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int class_id = int_arg(info, 0);
    pc_jobchange(&sd, class_id, 0);
}

void PlayerHost::changeSex_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    sd.status.sex = sd.status.sex == 0 ? 1 : 0;
    // Trigger client-side refresh
    clif_changelook(&sd.bl, LOOK_BODY2, 0);
}

void PlayerHost::jobName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_str(info, job_name(int_arg(info, 0)));
}

// =====================================================================
// Movement
// =====================================================================

void PlayerHost::warp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto mname = str_arg(info, 0);
    int x = int_arg(info, 1);
    int y = int_arg(info, 2);
    if (mname == "Random") {
        pc_randomwarp(&sd, CLR_TELEPORT);
    } else if (mname == "SavePoint") {
        pc_setpos_savepoint(sd, CLR_TELEPORT);
    } else {
        auto mi = mapindex_name2id(mname.c_str());
        if (mi != 0) pc_setpos(&sd, mi, x, y, CLR_TELEPORT);
    }
}

void PlayerHost::savePoint_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto mname = str_arg(info, 0);
    int x = int_arg(info, 1);
    int y = int_arg(info, 2);
    int16 mi = mapindex_name2id(mname.c_str());
    if (mi != 0) pc_setsavepoint(&sd, mi, x, y);
}

void PlayerHost::save_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    savePoint_cb(info);  // alias
}

void PlayerHost::getSavePoint_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto* iso = info.GetIsolate();
    auto ctx  = iso->GetCurrentContext();
    auto obj  = v8::Object::New(iso);
    // save_point.map is the literal map name (char[]), not an index.
    (void)obj->Set(ctx,
        v8::String::NewFromUtf8(iso, "map").ToLocalChecked(),
        v8::String::NewFromUtf8(iso, sd.status.save_point.map).ToLocalChecked());
    (void)obj->Set(ctx,
        v8::String::NewFromUtf8(iso, "x").ToLocalChecked(),
        v8::Integer::New(iso, sd.status.save_point.x));
    (void)obj->Set(ctx,
        v8::String::NewFromUtf8(iso, "y").ToLocalChecked(),
        v8::Integer::New(iso, sd.status.save_point.y));
    info.GetReturnValue().Set(obj);
}

void PlayerHost::pushPc_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // TODO: skill_blown / unit_blown
}

void PlayerHost::warpPartner_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // TODO: locate partner_id and warp
}

// =====================================================================
// Items
// =====================================================================

void PlayerHost::giveItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int amount = int_arg(info, 1, 1);
    if (amount <= 0 || nameid == 0) return;
    if (item_db.find(nameid) == nullptr) return;
    struct item it = {};
    it.nameid = nameid;
    it.identify = 1;
    // ItemOpts (info[2]) — refine / cards / bound — not yet unpacked.
    pc_additem(&sd, &it, amount, LOG_TYPE_SCRIPT);
}

void PlayerHost::giveRentItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int seconds = int_arg(info, 1);
    if (nameid == 0 || seconds <= 0) return;
    if (item_db.find(nameid) == nullptr) return;
    struct item it = {};
    it.nameid = nameid;
    it.identify = 1;
    it.expire_time = static_cast<unsigned int>(time(nullptr) + seconds);
    pc_additem(&sd, &it, 1, LOG_TYPE_SCRIPT);
}

void PlayerHost::giveNamedItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // TODO: pc_additem with card_name set
}

void PlayerHost::giveRandomGroupItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int group_id = int_arg(info, 0);
    int qty = int_arg(info, 1, 1);
    if (group_id <= 0) return;
    itemdb_group.pc_get_itemgroup(static_cast<uint16>(group_id), false, sd);
    if (qty > 1) {
        for (int i = 1; i < qty; ++i) {
            itemdb_group.pc_get_itemgroup(static_cast<uint16>(group_id), false, sd);
        }
    }
}

void PlayerHost::giveGroupItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int group_id = int_arg(info, 0);
    if (group_id <= 0) return;
    itemdb_group.pc_get_itemgroup(static_cast<uint16>(group_id), false, sd);
}

void PlayerHost::delItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int amount = int_arg(info, 1, 1);
    if (nameid == 0 || amount <= 0) return;
    int16 idx = pc_search_inventory(&sd, nameid);
    if (idx < 0) return;
    pc_delitem(&sd, idx, amount, 0, 0, LOG_TYPE_SCRIPT);
}

void PlayerHost::delItemAtIndex_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int idx = int_arg(info, 0);
    int amount = int_arg(info, 1, 1);
    if (idx < 0 || idx >= MAX_INVENTORY) return;
    pc_delitem(&sd, idx, amount, 0, 0, LOG_TYPE_SCRIPT);
}

void PlayerHost::countItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int total = 0;
    if (nameid != 0) {
        for (int i = 0; i < MAX_INVENTORY; ++i) {
            if (sd.inventory.u.items_inventory[i].nameid == nameid) {
                total += sd.inventory.u.items_inventory[i].amount;
            }
        }
    }
    ret_int(info, total);
}

void PlayerHost::countBound_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int boundtype = int_arg(info, 0, 0);
    int total = 0;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        const auto& it = sd.inventory.u.items_inventory[i];
        if (it.nameid == 0) continue;
        if (boundtype == 0 ? it.bound != 0 : it.bound == boundtype) {
            total += it.amount;
        }
    }
    ret_int(info, total);
}

void PlayerHost::hasItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int need = int_arg(info, 1, 1);
    int total = 0;
    for (int i = 0; i < MAX_INVENTORY && total < need; ++i) {
        if (sd.inventory.u.items_inventory[i].nameid == nameid) {
            total += sd.inventory.u.items_inventory[i].amount;
        }
    }
    ret_bool(info, total >= need);
}

void PlayerHost::clearItems_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        if (sd.inventory.u.items_inventory[i].nameid != 0) {
            pc_delitem(&sd, i, sd.inventory.u.items_inventory[i].amount,
                       0, 0, LOG_TYPE_SCRIPT);
        }
    }
}

void PlayerHost::consumeItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int idx = pc_search_inventory(&sd, nameid);
    if (idx >= 0) pc_delitem(&sd, idx, 1, 0, 0, LOG_TYPE_CONSUME);
}

void PlayerHost::searchItem_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    info.GetReturnValue().Set(v8::Array::New(iso, 0));  // TODO: name-substring search
}

void PlayerHost::getInventory_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto* iso = info.GetIsolate();
    auto ctx  = iso->GetCurrentContext();
    auto out  = v8::Array::New(iso);
    uint32 idx_out = 0;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        const auto& it = sd.inventory.u.items_inventory[i];
        if (it.nameid == 0) continue;
        auto row = v8::Object::New(iso);
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "index").ToLocalChecked(),
                       v8::Integer::New(iso, i));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "itemId").ToLocalChecked(),
                       v8::Integer::NewFromUnsigned(iso, it.nameid));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "amount").ToLocalChecked(),
                       v8::Integer::New(iso, it.amount));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "identified").ToLocalChecked(),
                       v8::Boolean::New(iso, it.identify != 0));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "refine").ToLocalChecked(),
                       v8::Integer::New(iso, it.refine));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "bound").ToLocalChecked(),
                       v8::Integer::New(iso, it.bound));
        (void)row->Set(ctx, v8::String::NewFromUtf8(iso, "grade").ToLocalChecked(),
                       v8::Integer::New(iso, it.enchantgrade));
        (void)out->Set(ctx, idx_out++, row);
    }
    info.GetReturnValue().Set(out);
}

void PlayerHost::mergeItems_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // TODO: trigger merge UI
}

void PlayerHost::identifyAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        auto& it = sd.inventory.u.items_inventory[i];
        if (it.nameid != 0 && !it.identify) {
            it.identify = 1;
            clif_item_identified(sd, i, false);
        }
    }
}

void PlayerHost::checkWeight_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid nameid = static_cast<t_itemid>(uint_arg(info, 0));
    int amount = int_arg(info, 1, 1);
    ret_bool(info, pc_checkadditem(&sd, nameid, amount) != CHKADDITEM_OVERAMOUNT);
}

// =====================================================================
// Equipment
// =====================================================================

void PlayerHost::getEquipId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    ret_int(info, idx >= 0 ? sd.inventory.u.items_inventory[idx].nameid : 0);
}

void PlayerHost::getEquipName_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx < 0) { ret_str(info, ""); return; }
    auto it = item_db.find(sd.inventory.u.items_inventory[idx].nameid);
    ret_str(info, it ? it->name.c_str() : "");
}

void PlayerHost::getEquipUniqueId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    ret_int(info, idx >= 0 ? static_cast<int>(sd.inventory.u.items_inventory[idx].unique_id) : 0);
}

void PlayerHost::getEquipRefine_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    ret_int(info, idx >= 0 ? sd.inventory.u.items_inventory[idx].refine : 0);
}

void PlayerHost::getEquipWeaponLv_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0, EQI_COMPOUND_ON);
    int idx = (slot == EQI_COMPOUND_ON) ? current_equip_item_index : pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0 && sd.inventory_data[idx] && sd.inventory_data[idx]->type == IT_WEAPON) {
        ret_int(info, sd.inventory_data[idx]->weapon_level);
    } else ret_int(info, 0);
}
void PlayerHost::getEquipArmorLv_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0, EQI_COMPOUND_ON);
    int idx = (slot == EQI_COMPOUND_ON) ? current_equip_item_index : pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0 && sd.inventory_data[idx] && sd.inventory_data[idx]->type == IT_ARMOR) {
        ret_int(info, sd.inventory_data[idx]->armor_level);
    } else ret_int(info, 0);
}
void PlayerHost::getEquipCardCount_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx < 0) { ret_int(info, 0); return; }
    int c = 0;
    for (int i = 0; i < MAX_SLOTS; ++i) {
        if (sd.inventory.u.items_inventory[idx].card[i] != 0) ++c;
    }
    ret_int(info, c);
}
void PlayerHost::getEquipCardId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int cardSlot = int_arg(info, 1);
    int idx = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx < 0 || cardSlot < 0 || cardSlot >= MAX_SLOTS) { ret_int(info, 0); return; }
    ret_int(info, static_cast<int32_t>(sd.inventory.u.items_inventory[idx].card[cardSlot]));
}
void PlayerHost::getEnchantGrade_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    ret_int(info, idx >= 0 ? sd.inventory.u.items_inventory[idx].enchantgrade : 0);
}

void PlayerHost::isEquipped_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    ret_bool(info, pc_checkequip(&sd, equip_bitmask[slot]) >= 0);
}
void PlayerHost::isEquipEnableRef_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_bool(info, false);  // TODO: check item flags
}
void PlayerHost::getItemPos_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    ret_int(info, pc_checkequip(&sd, equip_bitmask[slot]));
}

void PlayerHost::equip_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    t_itemid id = static_cast<t_itemid>(uint_arg(info, 0));
    int idx = pc_search_inventory(&sd, id);
    if (idx < 0) return;
    pc_equipitem(&sd, idx, sd.inventory.u.items_inventory[idx].equip);
}

void PlayerHost::autoEquip_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    (void)info;  // TODO: autoequip flag
}

void PlayerHost::unequip_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0) pc_unequipitem(&sd, idx, 1);
}

void PlayerHost::delEquip_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0) pc_delitem(&sd, idx, 1, 0, 0, LOG_TYPE_SCRIPT);
}

void PlayerHost::breakEquip_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0) sd.inventory.u.items_inventory[idx].attribute = 1;
}

void PlayerHost::successRefine_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0 && sd.inventory.u.items_inventory[idx].refine < MAX_REFINE) {
        sd.inventory.u.items_inventory[idx].refine++;
    }
}

void PlayerHost::failRefine_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0) pc_delitem(&sd, idx, 1, 0, 0, LOG_TYPE_SCRIPT);
}

void PlayerHost::downRefine_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int slot = int_arg(info, 0);
    int idx  = pc_checkequip(&sd, equip_bitmask[slot]);
    if (idx >= 0 && sd.inventory.u.items_inventory[idx].refine > 0) {
        sd.inventory.u.items_inventory[idx].refine--;
    }
}

void PlayerHost::repair_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int brokenIdx = int_arg(info, 0);
    int seen = 0;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        if (sd.inventory.u.items_inventory[i].nameid != 0 &&
            sd.inventory.u.items_inventory[i].attribute) {
            if (seen == brokenIdx) {
                sd.inventory.u.items_inventory[i].attribute = 0;
                clif_equiplist(&sd);
                return;
            }
            ++seen;
        }
    }
}
void PlayerHost::repairAll_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        if (sd.inventory.u.items_inventory[i].attribute)
            sd.inventory.u.items_inventory[i].attribute = 0;
    }
    clif_equiplist(&sd);
}
void PlayerHost::removeCards_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void PlayerHost::getBrokenId_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int wanted = int_arg(info, 0);
    int seen = 0;
    for (int i = 0; i < MAX_INVENTORY; ++i) {
        if (sd.inventory.u.items_inventory[i].nameid != 0 &&
            sd.inventory.u.items_inventory[i].attribute) {
            if (seen == wanted) {
                ret_int(info, static_cast<int32_t>(sd.inventory.u.items_inventory[i].nameid));
                return;
            }
            ++seen;
        }
    }
    ret_int(info, 0);
}

// =====================================================================
// Skills
// =====================================================================

void PlayerHost::skillLv_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int skill_id = int_arg(info, 0);
    ret_int(info, pc_checkskill(&sd, skill_id));
}

void PlayerHost::addSkill_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int skill_id = int_arg(info, 0);
    int level    = int_arg(info, 1);
    pc_skill(&sd, skill_id, level, ADDSKILL_PERMANENT);
}

void PlayerHost::itemSkill_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int skill_id = int_arg(info, 0);
    int level    = int_arg(info, 1);
    pc_skill(&sd, skill_id, level, ADDSKILL_TEMP);
}

void PlayerHost::getSkillList_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    auto* iso = info.GetIsolate();
    info.GetReturnValue().Set(v8::Array::New(iso, 0));  // TODO
}

void PlayerHost::skillPointCount_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_int(info, sd.status.skill_point);
}

void PlayerHost::basicSkillCheck_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_bool(info, pc_checkskill(&sd, 1) >= 5);  // NV_BASIC level 5+
}

// =====================================================================
// Looks / mounts — most are setlook variants
// =====================================================================

void PlayerHost::setLook_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int type  = int_arg(info, 0);
    int value = int_arg(info, 1);
    clif_changelook(&sd.bl, type, value);
}
void PlayerHost::changeLook_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    setLook_cb(info);
}
void PlayerHost::getLook_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int type = int_arg(info, 0);
    int val = 0;
    switch (type) {
        case LOOK_HAIR:        val = sd.status.hair; break;
        case LOOK_HAIR_COLOR:  val = sd.status.hair_color; break;
        case LOOK_CLOTHES_COLOR: val = sd.status.clothes_color; break;
        case LOOK_BODY2:       val = sd.status.body; break;
        case LOOK_WEAPON:      val = sd.status.weapon; break;
        case LOOK_SHIELD:      val = sd.status.shield; break;
        default: break;
    }
    ret_int(info, val);
}

void PlayerHost::setFont_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    sd.status.font = static_cast<uint8>(int_arg(info, 0));
}
void PlayerHost::setCart_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { UNWRAP; pc_setcart(&sd, int_arg(info, 0, 1)); }
void PlayerHost::setFalcon_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; pc_setfalcon(&sd, bool_arg(info, 0, true) ? 1 : 0); }
void PlayerHost::setRiding_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; pc_setriding(&sd, bool_arg(info, 0, true) ? 1 : 0); }
void PlayerHost::setDragon_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    UNWRAP;
    int color = int_arg(info, 0, 0);
    int opt = OPTION_DRAGON1;
    if (color == 2) opt = OPTION_DRAGON2;
    else if (color == 3) opt = OPTION_DRAGON3;
    else if (color == 4) opt = OPTION_DRAGON4;
    else if (color == 5) opt = OPTION_DRAGON5;
    pc_setoption(&sd, sd.sc.option | opt);
}
void PlayerHost::setMadogear_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    bool flag = bool_arg(info, 0, true);
    int type = int_arg(info, 1, MADO_ROBOT);
    pc_setmadogear(&sd, flag, static_cast<e_mado_type>(type));
}
void PlayerHost::setMounting_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    if (sd.sc.getSCE(SC_ALL_RIDING))
        status_change_end(&sd.bl, SC_ALL_RIDING);
    else
        sc_start(&sd.bl, &sd.bl, SC_ALL_RIDING, 10000, 1, INFINITE_TICK);
}

void PlayerHost::checkCart_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; ret_bool(info, pc_iscarton(&sd)); }
void PlayerHost::checkFalcon_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_bool(info, pc_isfalcon(&sd)); }
void PlayerHost::checkRiding_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_bool(info, pc_isriding(&sd)); }
void PlayerHost::checkDragon_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_bool(info, pc_isridingdragon(&sd)); }
void PlayerHost::checkMadogear_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_bool(info, pc_ismadogear(&sd)); }
void PlayerHost::checkWug_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { UNWRAP; ret_bool(info, pc_iswug(&sd)); }
void PlayerHost::isMounting_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { UNWRAP; ret_bool(info, sd.sc.getSCE(SC_ALL_RIDING) != nullptr); }

// =====================================================================
// Options / status
// =====================================================================

void PlayerHost::setOption_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int opt  = int_arg(info, 0);
    bool on  = bool_arg(info, 1, true);
    pc_setoption(&sd, on ? (sd.sc.option | opt) : (sd.sc.option & ~opt));
}
void PlayerHost::checkOption_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_bool(info, (sd.sc.option & int_arg(info, 0)) != 0);
}
void PlayerHost::checkOption1_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_bool(info, (sd.sc.opt1 & int_arg(info, 0)) != 0);
}
void PlayerHost::checkOption2_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_bool(info, (sd.sc.opt2 & int_arg(info, 0)) != 0);
}

void PlayerHost::scStart_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int type  = int_arg(info, 0);
    int dur   = int_arg(info, 1);
    sc_start(&sd.bl, &sd.bl, static_cast<sc_type>(type), 10000, 1, dur);
}
void PlayerHost::scEnd_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int type = int_arg(info, 0, -1);
    if (type < 0) status_change_clear(&sd.bl, 0);
    else status_change_end(&sd.bl, static_cast<sc_type>(type));
}
void PlayerHost::getStatus_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_int(info, 0); }
void PlayerHost::isDead_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { UNWRAP; ret_bool(info, pc_isdead(&sd)); }
void PlayerHost::recalculateStat_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP; status_calc_pc(&sd, SCO_FORCE);
}
void PlayerHost::needStatusPoint_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    ret_int(info, 0);  // TODO
}

// =====================================================================
// Reset
// =====================================================================

void PlayerHost::resetStatus_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; pc_resetstate(&sd); }
void PlayerHost::resetSkill_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { UNWRAP; pc_resetskill(&sd, 1); }
void PlayerHost::resetFeel_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; pc_resetfeel(&sd); }
void PlayerHost::resetHate_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; pc_resethate(&sd); }

// =====================================================================
// Display effects
// =====================================================================

void PlayerHost::message_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto text = str_arg(info, 0);
    clif_displaymessage(sd.fd, text.c_str());
}
void PlayerHost::dispBottom_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto text = str_arg(info, 0);
    clif_messagecolor(&sd.bl,
        static_cast<unsigned long>(int_arg(info, 1, 0x00FFFFFF)),
        text.c_str(), false, SELF);
}
void PlayerHost::showScript_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto text = str_arg(info, 0);
    clif_showscript(&sd.bl, text.c_str(),
        static_cast<send_target>(int_arg(info, 1, SELF)));
}
void PlayerHost::cutin_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto file = str_arg(info, 0);
    clif_cutin(sd, file.c_str(), int_arg(info, 1));
}
void PlayerHost::emotion_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int e = int_arg(info, 0);
    clif_emotion(sd.bl, static_cast<emotion_type>(e));
}
void PlayerHost::miscEffect_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int e = int_arg(info, 0);
    clif_specialeffect(&sd.bl, e, AREA);
}
void PlayerHost::soundEffect_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto file = str_arg(info, 0);
    clif_soundeffect(sd.bl, file.c_str(), int_arg(info, 1, 0), SELF);
}
void PlayerHost::playBgm_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto file = str_arg(info, 0);
    clif_playBGM(sd, file.c_str());
}
void PlayerHost::viewpoint_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int act = int_arg(info, 0);
    int x = int_arg(info, 1), y = int_arg(info, 2);
    int point = int_arg(info, 3);
    int color = int_arg(info, 4);
    clif_viewpoint(sd, sd.bl.id, act, static_cast<uint16>(x), static_cast<uint16>(y), point, static_cast<uint32>(color));
}
void PlayerHost::showDigit_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int v = int_arg(info, 0);
    int t = int_arg(info, 1, 1);
    clif_showdigit(&sd, static_cast<unsigned char>(t), v);
}
void PlayerHost::hatEffect_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int effect = int_arg(info, 0);
    bool state = bool_arg(info, 1);
    if (state) sd.hatEffects.push_back(static_cast<uint32>(effect));
    clif_hat_effect_single(sd, effect, state);
}

// =====================================================================
// UI windows
// =====================================================================

void PlayerHost::openStorage_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    storage_storageopen(&sd);
}
void PlayerHost::openBank_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    UNWRAP;
    clif_ui_open(sd, OUT_UI_BANK, 0);
}
void PlayerHost::openMail_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { UNWRAP; clif_Mail_window(sd.fd, 0); }
void PlayerHost::openAuction_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_Auction_openwindow(&sd); }
void PlayerHost::openRefineUi_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_refineui_open(&sd); }
void PlayerHost::openStylist_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_ui_open(sd, OUT_UI_STYLIST, 0); }
void PlayerHost::openDressRoom_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_dressing_room(sd); }
void PlayerHost::openRoulette_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_roulette_open(&sd); }
void PlayerHost::openQuestUi_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void PlayerHost::openEnchantGrade_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_ui_open(sd, OUT_UI_ENCHANTGRADE, 0); }
void PlayerHost::openLaphineSynthesis_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void PlayerHost::openLaphineUpgrade_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::openItemEnchant_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    clif_ui_open(sd, OUT_UI_ENCHANT, int_arg(info, 0));
}
void PlayerHost::openItemReform_cb(const v8::FunctionCallbackInfo<v8::Value>& info){
    UNWRAP;
    clif_item_reform_open(sd, static_cast<t_itemid>(uint_arg(info, 0)));
}
void PlayerHost::specialPopup_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; clif_specialpopup(sd, int_arg(info, 0)); }
void PlayerHost::openTips_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::readBook_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   {
    UNWRAP;
    clif_readbook(sd.fd, int_arg(info, 0), int_arg(info, 1, 0));
}

// =====================================================================
// Spirit balls / reputation / fame / marriage — minimal placeholders
// =====================================================================

void PlayerHost::addSpiritBall_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int count = int_arg(info, 0, 1);
    int dur   = int_arg(info, 1, 30000);
    pc_addspiritball(&sd, dur, count);
}
void PlayerHost::delSpiritBall_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    pc_delspiritball(&sd, int_arg(info, 0, sd.spiritball), 0);
}
void PlayerHost::countSpiritBall_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_int(info, sd.spiritball); }

void PlayerHost::getReputation_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { ret_int(info, 0); }
void PlayerHost::setReputation_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::addReputation_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::getFame_cb(const v8::FunctionCallbackInfo<v8::Value>& info)         { UNWRAP; ret_int(info, sd.status.fame); }
void PlayerHost::addFame_cb(const v8::FunctionCallbackInfo<v8::Value>& info)         { UNWRAP; sd.status.fame += int_arg(info, 0); }
void PlayerHost::getFameRank_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { ret_int(info, 0); }

void PlayerHost::marry_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    auto name = str_arg(info, 0);
    auto* dst = map_nick2sd(name.c_str(), false);
    if (dst) pc_marriage(&sd, dst);
}
void PlayerHost::divorce_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP; pc_divorce(&sd);
}
void PlayerHost::adopt_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { (void)info; /* requires both parent sds; skip */ }
void PlayerHost::getPartnerId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { UNWRAP; ret_int(info, sd.status.partner_id); }
void PlayerHost::getMotherId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { UNWRAP; ret_int(info, sd.status.mother); }
void PlayerHost::getFatherId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { UNWRAP; ret_int(info, sd.status.father); }
void PlayerHost::getChildId_cb(const v8::FunctionCallbackInfo<v8::Value>& info)      { UNWRAP; ret_int(info, sd.status.child); }
void PlayerHost::isPartnerOn_cb(const v8::FunctionCallbackInfo<v8::Value>& info)     { ret_bool(info, false); }

// =====================================================================
// Permissions / VIP / misc
// =====================================================================

void PlayerHost::permissionCheck_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { ret_bool(info, false); }
void PlayerHost::permissionAdd_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::permissionRemove_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ (void)info; }
void PlayerHost::guildHasPermission_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ ret_bool(info, false); }

void PlayerHost::vipStatus_cb(const v8::FunctionCallbackInfo<v8::Value>& info)       { ret_int(info, 0); }
void PlayerHost::vipTime_cb(const v8::FunctionCallbackInfo<v8::Value>& info)         { (void)info; }
void PlayerHost::macroDetector_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }

void PlayerHost::charInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int t = int_arg(info, 0);
    switch (t) {
        case 0: ret_str(info, sd.status.name); break;
        case 1: ret_str(info, ""); break;  // party name — needs lookup
        case 2: ret_str(info, ""); break;  // guild name — needs lookup
        case 3: { auto mn = map_mapid2mapname(sd.bl.m); ret_str(info, mn ? mn : ""); break; }
        default: ret_str(info, ""); break;
    }
}
void PlayerHost::readParam_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    int param = int_arg(info, 0);
    ret_int(info, static_cast<int32_t>(pc_readparam(&sd, param)));
}
void PlayerHost::charId4Type_cb(const v8::FunctionCallbackInfo<v8::Value>& info) { UNWRAP; ret_int(info, sd.status.char_id); }
void PlayerHost::charIp_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    if (sd.fd < 0 || !session[sd.fd]) { ret_str(info, ""); return; }
    uint32 ip = session[sd.fd]->client_addr;
    char buf[32];
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
             (ip >> 24) & 0xff, (ip >> 16) & 0xff,
             (ip >> 8) & 0xff, ip & 0xff);
    ret_str(info, buf);
}
void PlayerHost::kick_cb(const v8::FunctionCallbackInfo<v8::Value>& info)        { UNWRAP; clif_GM_kick(nullptr, &sd); }
void PlayerHost::ignoreTimeout_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; sd.state.ignoretimeout = bool_arg(info, 0) ? 1 : 0; }
void PlayerHost::autoLoot_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    if (has(info, 0)) sd.state.autoloot = int_arg(info, 0);
    ret_int(info, sd.state.autoloot);
}
void PlayerHost::hasAutoLoot_cb(const v8::FunctionCallbackInfo<v8::Value>& info) {
    UNWRAP;
    ret_bool(info, sd.state.autoloot > 0);
}
void PlayerHost::jobCanEnterMap_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ ret_bool(info, true); }
void PlayerHost::checkVending_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; ret_bool(info, sd.state.vending != 0); }
void PlayerHost::checkChatting_cb(const v8::FunctionCallbackInfo<v8::Value>& info){ UNWRAP; ret_bool(info, sd.chatID != 0); }
void PlayerHost::checkIdle_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { ret_bool(info, false); }
void PlayerHost::navigateTo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { (void)info; }
void PlayerHost::clanJoin_cb(const v8::FunctionCallbackInfo<v8::Value>& info)    { (void)info; }
void PlayerHost::clanLeave_cb(const v8::FunctionCallbackInfo<v8::Value>& info)   { (void)info; }
void PlayerHost::cameraInfo_cb(const v8::FunctionCallbackInfo<v8::Value>& info)  { ret_null(info); }

// =====================================================================
// install_on_object — bind every callback onto the live ctx.player obj
// =====================================================================

void PlayerHost::install_on_object(v8::Isolate* iso, v8::Local<v8::Context> ctx,
                                   v8::Local<v8::Object> obj) {
#define BIND(name) args::bind_method<PlayerHost>(iso, ctx, obj, this, #name, &PlayerHost::name##_cb)
    BIND(heal); BIND(healAp); BIND(itemHeal); BIND(percentHeal); BIND(recovery);
    BIND(giveExp); BIND(baseExpRatio); BIND(jobExpRatio);
    BIND(jobChange); BIND(changeBase); BIND(changeSex); BIND(jobName);
    BIND(warp); BIND(savePoint); BIND(save); BIND(getSavePoint); BIND(pushPc); BIND(warpPartner);
    BIND(giveItem); BIND(giveRentItem); BIND(giveNamedItem);
    BIND(giveRandomGroupItem); BIND(giveGroupItem);
    BIND(delItem); BIND(delItemAtIndex);
    BIND(countItem); BIND(countBound); BIND(hasItem);
    BIND(clearItems); BIND(consumeItem); BIND(searchItem);
    BIND(getInventory); BIND(mergeItems); BIND(identifyAll); BIND(checkWeight);
    BIND(getEquipId); BIND(getEquipName); BIND(getEquipUniqueId); BIND(getEquipRefine);
    BIND(getEquipWeaponLv); BIND(getEquipArmorLv);
    BIND(getEquipCardCount); BIND(getEquipCardId); BIND(getEnchantGrade);
    BIND(isEquipped); BIND(isEquipEnableRef); BIND(getItemPos);
    BIND(equip); BIND(autoEquip); BIND(unequip); BIND(delEquip); BIND(breakEquip);
    BIND(successRefine); BIND(failRefine); BIND(downRefine);
    BIND(repair); BIND(repairAll); BIND(removeCards); BIND(getBrokenId);
    BIND(skillLv); BIND(addSkill); BIND(itemSkill); BIND(getSkillList);
    BIND(skillPointCount); BIND(basicSkillCheck);
    BIND(setLook); BIND(changeLook); BIND(getLook); BIND(setFont);
    BIND(setCart); BIND(setFalcon); BIND(setRiding); BIND(setDragon);
    BIND(setMadogear); BIND(setMounting);
    BIND(checkCart); BIND(checkFalcon); BIND(checkRiding); BIND(checkDragon);
    BIND(checkMadogear); BIND(checkWug); BIND(isMounting);
    BIND(setOption); BIND(checkOption); BIND(checkOption1); BIND(checkOption2);
    BIND(scStart); BIND(scEnd); BIND(getStatus); BIND(isDead);
    BIND(recalculateStat); BIND(needStatusPoint);
    BIND(resetStatus); BIND(resetSkill); BIND(resetFeel); BIND(resetHate);
    BIND(message); BIND(dispBottom); BIND(showScript); BIND(cutin); BIND(emotion);
    BIND(miscEffect); BIND(soundEffect); BIND(playBgm); BIND(viewpoint);
    BIND(showDigit); BIND(hatEffect);
    BIND(openStorage); BIND(openBank); BIND(openMail); BIND(openAuction);
    BIND(openRefineUi); BIND(openStylist); BIND(openDressRoom); BIND(openRoulette);
    BIND(openQuestUi); BIND(openEnchantGrade);
    BIND(openLaphineSynthesis); BIND(openLaphineUpgrade);
    BIND(openItemEnchant); BIND(openItemReform);
    BIND(specialPopup); BIND(openTips); BIND(readBook);
    BIND(addSpiritBall); BIND(delSpiritBall); BIND(countSpiritBall);
    BIND(getReputation); BIND(setReputation); BIND(addReputation);
    BIND(getFame); BIND(addFame); BIND(getFameRank);
    BIND(marry); BIND(divorce); BIND(adopt);
    BIND(getPartnerId); BIND(getMotherId); BIND(getFatherId); BIND(getChildId);
    BIND(isPartnerOn);
    BIND(permissionCheck); BIND(permissionAdd); BIND(permissionRemove);
    BIND(guildHasPermission);
    BIND(vipStatus); BIND(vipTime); BIND(macroDetector);
    BIND(charInfo); BIND(readParam); BIND(charId4Type); BIND(charIp);
    BIND(kick); BIND(ignoreTimeout); BIND(autoLoot); BIND(hasAutoLoot);
    BIND(jobCanEnterMap); BIND(checkVending); BIND(checkChatting); BIND(checkIdle);
    BIND(navigateTo); BIND(clanJoin); BIND(clanLeave); BIND(cameraInfo);
#undef BIND
}

} // namespace rathena::scripting

#endif // HAVE_TS_SCRIPTING

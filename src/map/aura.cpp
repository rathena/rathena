+// Copyright(c) Pandas Dev Teams - Licensed under GNU GPL
+// For more information, see LICENCE in the main folder
+
+#include "aura.hpp"
+
+#include "battle.hpp"
+#include "map.hpp"
+#include "pc.hpp"
+
+AuraDatabase aura_db;
+
+std::unordered_map<uint16, enum e_aura_special> special_effects{
+	{202, AURA_SPECIAL_HIDE_DISAPPEAR},
+	{362, AURA_SPECIAL_HIDE_DISAPPEAR}
+};
+
+//************************************
+// Method:      getDefaultLocation
+// Description: 获取 YAML 数据文件的具体路径
+// Access:      public
+// Returns:     const std::string
+// Author:      Sola丶小克(CairoLee)  2020/09/26 15:30
+//************************************
+const std::string AuraDatabase::getDefaultLocation() {
+	return std::string(db_path) + "/aura_db.yml";
+}
+
+//************************************
+// Method:      parseBodyNode
+// Description: 解析 Body 节点的主要处理函数
+// Access:      public
+// Parameter:   const ryml::NodeRef & node
+// Returns:     uint64
+// Author:      Sola丶小克(CairoLee)  2020/09/26 15:30
+//************************************
+uint64 AuraDatabase::parseBodyNode(const ryml::NodeRef& node) {
+	uint32 aura_id = 0;
+
+	if (!this->asUInt32(node, "AuraID", aura_id)) {
+		return 0;
+	}
+
+	if (aura_id <= 0) {
+		return 0;
+	}
+
+	auto aura = this->find(aura_id);
+	bool exists = aura != nullptr;
+
+	if (!exists) {
+		aura = std::make_shared<s_aura>();
+		aura->aura_id = aura_id;
+	}
+	else {
+		aura->effects.clear();
+	}
+
+	if (!this->nodeExists(node, "EffectList")) {
+		return 0;
+	}
+
+	for (const ryml::NodeRef& subNode : node["EffectList"]) {
+		uint16 effect_id = 0;
+		if (!this->asUInt16(subNode, "EffectID", effect_id)) {
+			return 0;
+		}
+
+		uint32 replay_interval = 0;
+		if (this->nodeExists(subNode, "ReplayInterval")) {
+			if (!this->asUInt32(subNode, "ReplayInterval", replay_interval)) {
+				return 0;
+			}
+		}
+
+		auto effect = std::make_shared<s_aura_effect>();
+		effect->effect_id = effect_id;
+		effect->replay_interval = replay_interval;
+
+		aura->effects.push_back(effect);
+	}
+
+	if (!exists) {
+		this->put(aura->aura_id, aura);
+	}
+
+	return 1;
+}
+
+//************************************
+// Method:      aura_search
+// Description: 根据光环编号获取 aura_db.yml 记录的信息
+// Access:      public
+// Parameter:   uint32 aura_id
+// Returns:     std::shared_ptr<s_aura>
+// Author:      Sola丶小克(CairoLee)  2020/09/26 16:30
+//************************************
+std::shared_ptr<s_aura> aura_search(uint32 aura_id) {
+	return aura_db.find(aura_id);
+}
+
+//************************************
+// Method:      aura_special
+// Description: 给定一个效果编号, 查询它是否是一个特殊效果, 并返回它的标记位
+// Access:      public
+// Parameter:   uint16 effect_id
+// Returns:     enum e_aura_special 特殊标记位
+// Author:      Sola丶小克(CairoLee)  2020/10/08 10:59
+//************************************
+enum e_aura_special aura_special(uint16 effect_id) {
+	if (special_effects.find(effect_id) != special_effects.end()) {
+		return special_effects[effect_id];
+	}
+	return AURA_SPECIAL_NOTHING;
+}
+
+//************************************
+// Method:      aura_effects_timer_sub
+// Description: 用来遍历附近玩家并对他们发送指定光环特效的处理函数
+// Access:      public
+// Parameter:   struct block_list * bl 遍历到附近的玩家单位
+// Parameter:   va_list ap
+// Returns:     int
+// Author:      Sola丶小克(CairoLee)  2021/12/29 21:27
+//************************************
+int aura_effects_timer_sub(struct block_list* bl, va_list ap) {
+	map_session_data* sd = nullptr;
+	struct block_list* effect_unit_bl = va_arg(ap, struct block_list*);
+	unsigned int effect_id = va_arg(ap, unsigned int);
+
+	if (!bl || !effect_unit_bl || bl->type != BL_PC) {
+		return 0;
+	}
+
+	sd = BL_CAST(BL_PC, bl);
+
+	// 主要是将观察者带入到是否隐藏光环的判断中
+	if (aura_need_hiding(effect_unit_bl, bl)) {
+		return 0;
+	}
+
+	clif_specialeffect_single(effect_unit_bl, effect_id, sd->fd);
+	return 1;
+}
+
+//************************************
+// Method:      aura_effects_timer
+// Description: 给非持久型特效进行定时播放的定时器处理函数
+// Access:      public
+// Parameter:   aura_effects_timer
+// Returns:
+// Author:      Sola丶小克(CairoLee)  2021/04/24 14:56
+//************************************
+TIMER_FUNC(aura_effects_timer) {
+	struct block_list* bl = map_id2bl(id);
+	if (!bl) {
+		delete_timer(tid, aura_effects_timer);
+		return 1;
+	}
+
+	struct s_unit_common_data* ucd = status_get_ucd(bl);
+	if (!ucd) {
+		delete_timer(tid, aura_effects_timer);
+		return 1;
+	}
+
+	for (auto it : ucd->aura.effects) {
+		if (it->replay_tid != tid) continue;
+		map_foreachinallrange(aura_effects_timer_sub, bl, AREA_SIZE, BL_PC, bl, it->effect_id);
+	}
+
+	return 0;
+}
+
+//************************************
+// Method:      aura_need_hiding
+// Description: 判断用于光环是否需要隐藏 (或者说不展现)
+// Access:      public
+// Parameter:   struct block_list * bl
+//				该参数用于指定需要判断哪个 bl 单位的光环是否需要被隐藏
+// Parameter:   struct block_list * observer_bl
+//				观察者的 bl 指针 (默认为 nullptr 表示没有观察者, 无需考虑 cloak 影响)
+//				通常情况下一个如果被检测的 bl 单位是一个 npc,
+//				那么可能会因为这个 npc 已经在某个 observer_bl 的视野中被隐藏/显示 (cloakonnpc/cloakoffnpc)
+//				因此想判断一个目标 bl 单位是否可以显示光环的时候, 把 observer_bl 带上判断就会代入观察者视野
+// Returns:     bool
+// Author:      Sola丶小克(CairoLee)  2021/12/29 22:46
+//************************************
+bool aura_need_hiding(struct block_list* bl, struct block_list* observer_bl) {
+	if (!bl) return true;
+
+	// 宠物没有 status_change, 当前也没有什么技能或者状态可以隐藏宠物,
+	// 因此如果是宠物的话, 就默认为无需隐藏.
+	if (bl->type == BL_PET) return false;
+
+	status_change* sc = status_get_sc(bl);
+	if (!sc) return true;
+
+	return status_ishiding(bl, observer_bl) || status_isinvisible(bl) || sc->getSCE(SC_CAMOUFLAGE);
+}
+
+//************************************
+// Method:      aura_effects_clear
+// Description: 将当前活跃的光环特效全部移除掉
+// Access:      public
+// Parameter:   struct s_unit_common_data * ucd
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2021/04/23 22:04
+//************************************
+void aura_effects_clear(struct block_list* bl) {
+	if (!bl) return;
+
+	struct s_unit_common_data* ucd = status_get_ucd(bl);
+	if (!ucd) return;
+
+	for (auto &it : ucd->aura.effects) {
+		clif_specialeffect_remove(bl, it->effect_id, AREA, bl);
+		if (it->replay_tid != INVALID_TIMER) {
+			delete_timer(it->replay_tid, aura_effects_timer);
+			it->replay_tid = INVALID_TIMER;
+		}
+	}
+	ucd->aura.effects.clear();
+}
+
+//************************************
+// Method:      aura_effects_refill
+// Description: 根据 ucd 中的光环信息将特效填充到生效列表
+// Access:      public
+// Parameter:   struct s_unit_common_data * ucd
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2021/04/21 23:23
+//************************************
+void aura_effects_refill(struct block_list* bl) {
+	if (!bl) return;
+
+	aura_effects_clear(bl);
+
+	struct s_unit_common_data* ucd = status_get_ucd(bl);
+	if (!ucd || !ucd->aura.id) return;
+
+	std::shared_ptr<s_aura> aura = aura_search(ucd->aura.id);
+	if (!aura) return;
+
+	for (auto it : aura->effects) {
+		auto effect = std::make_shared<s_aura_effect>();
+		effect->effect_id = it->effect_id;
+		effect->replay_interval = it->replay_interval;
+		effect->replay_tid = INVALID_TIMER;
+
+		if (effect->replay_interval) {
+			effect->replay_tid = add_timer_interval(
+				gettick() + 100, aura_effects_timer, bl->id, 0, effect->replay_interval
+			);
+		}
+
+		ucd->aura.effects.push_back(effect);
+	}
+}
+
+//************************************
+// Method:      aura_refresh_client
+// Description: 刷新客户端使之能看到光环的视觉效果
+// Access:      public
+// Parameter:   struct block_list * bl
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2021/04/24 18:16
+//************************************
+void aura_refresh_client(struct block_list* bl) {
+	if (!bl) return;
+	struct map_data* mapdata = map_getmapdata(bl->m);
+
+	switch (bl->type)
+	{
+	case BL_PC: {
+#if PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002
+		clif_send_auras(bl, AREA, true, AURA_SPECIAL_NOTHING);
+#else
+		TBL_PC* sd = map_id2sd(bl->id);
+		if (sd) {
+			pc_setpos(sd, sd->mapindex, sd->bl.x, sd->bl.y, CLR_OUTSIGHT);
+		}
+#endif // PACKETVER_MAIN_NUM >= 20181002 || PACKETVER_RE_NUM >= 20181002
+		break;
+	}
+	case BL_MOB:
+	case BL_PET:
+		// 若是魔物或宠物的话则使用 clif_clearunit_area 直接发 CLR_TRICKDEAD 清理缓存,
+		// 而不是和 else 分支一样广播 clif_outsight 封包,
+		// 否则单位在移动过程中发生光环替换的时候会有明显的消失后再出现的效果.
+		if (mapdata && mapdata->users) {
+			clif_clearunit_area(*bl, CLR_TRICKDEAD);
+			map_foreachinallrange(clif_insight, bl, AREA_SIZE, BL_PC, bl);
+		}
+		break;
+	default:
+		if (mapdata && mapdata->users) {
+			map_foreachinallrange(clif_outsight, bl, AREA_SIZE, BL_PC, bl);
+			map_foreachinallrange(clif_insight, bl, AREA_SIZE, BL_PC, bl);
+		}
+		break;
+	}
+}
+
+//************************************
+// Method:      aura_make_effective
+// Description: 为指定的单位设置光环并使其能够立刻刷新生效
+// Access:      public
+// Parameter:   struct block_list * bl
+// Parameter:   uint32 aura_id
+// Parameter:   bool pc_saved 若是玩家单位, 那么是否保存此光环到数据库
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2020/10/13 00:21
+//************************************
+void aura_make_effective(struct block_list* bl, uint32 aura_id, bool pc_saved) {
+	if (!bl) return;
+	if (aura_id && !aura_search(aura_id)) return;
+
+	map_freeblock_lock();
+
+	struct s_unit_common_data* ucd = status_get_ucd(bl);
+	if (ucd) {
+		ucd->aura.id = aura_id;
+		aura_effects_refill(bl);
+	}
+
+	switch (bl->type)
+	{
+	case BL_PC: {
+		map_session_data* sd = BL_CAST(BL_PC, bl);
+		if (sd && pc_saved) {
+			pc_setglobalreg(sd, add_str(AURA_VARIABLE), aura_id);
+		}
+		break;
+	}
+	default:
+		break;
+	}
+
+	aura_refresh_client(bl);
+
+	map_freeblock_unlock();
+}
+
+//************************************
+// Method:      aura_reload
+// Description: 重新载入 aura_db.yml 光环组合数据库
+// Access:      public
+// Parameter:   void
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2020/09/26 16:41
+//************************************
+void aura_reload(void) {
+	aura_db.clear();
+	aura_db.load();
+}
+
+//************************************
+// Method:      do_final_aura
+// Description: 释放 AuraDatabase 的实例对象 aura_db 所保存的内容
+// Access:      public
+// Parameter:   void
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2020/09/26 16:41
+//************************************
+void do_final_aura(void) {
+	aura_db.clear();
+}
+
+//************************************
+// Method:      do_init_aura
+// Description: 初始化光环机制, 从 aura_db.yml 光环组合数据库中载入数据
+// Access:      public
+// Parameter:   void
+// Returns:     void
+// Author:      Sola丶小克(CairoLee)  2020/09/26 16:41
+//************************************
+void do_init_aura(void) {
+	add_timer_func_list(aura_effects_timer, "aura_effects_timer");
+
+	aura_db.load();
+}

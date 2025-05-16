# List of client-side effects sorted by ID

The following is a compiled list of visual and sound effects which the client
can produce when receiving a packet with id `0x1f3 (01f3 <ID>.l <type>.l)`.
Each list entry contains a number and a short description of the effect.
You can produce these effects ingame by doing `"@effect <number>"`.
It's also possible to attach effects to item/npc scripts by using `specialeffect` or `specialeffect2`.

|Number|Constant|Description|
|---|---|---|
|0|EF_HIT1|	Regular Hit|
|1|EF_HIT2|	Bash|
|2|EF_HIT3|	Melee Skill Hit|
|3|EF_HIT4|	Melee Skill Hit|
|4|EF_HIT5|	Melee Skill Hit|
|5|EF_HIT6|	Melee Skill Hit|
|6|EF_ENTRY|Being Warped|
|7|EF_EXIT|	Item Heal effect|
|8|EF_WARP|	Yellow Ripple Effect|
|9|EF_ENHANCE|Different Type of Heal|
|10|EF_COIN|	Mammonite|
|11|EF_ENDURE|Endure|
|12|EF_BEGINSPELL|Yellow cast aura|
|13|EF_GLASSWALL|Blue Box|
|14|EF_HEALSP|Blue restoring effect|
|15|EF_SOULSTRIKE|Soul Strike|
|16|EF_BASH|	Hide|
|17|EF_MAGNUMBREAK|Magnum Break|
|18|EF_STEAL|Steal|
|19|EF_HIDING|(Invalid)|
|20|EF_PATTACK|Envenom/Poison|
|21|EF_DETOXICATION|Detoxify|
|22|EF_SIGHT|Sight|
|23|EF_STONECURSE|Stone Curse|
|24|EF_FIREBALL|Fire Ball|
|25|EF_FIREWALL|Fire Wall|
|26|EF_ICEARROW|A sound (a swipe?)|
|27|EF_FROSTDIVER|Frost Diver (Traveling to Target)|
|28|EF_FROSTDIVER2|Frost Diver (Hitting)|
|29|EF_LIGHTBOLT|Lightning Bolt|
|30|EF_THUNDERSTORM|Thunder Storm|
|31|EF_FIREARROW|Weird bubbles launching from feet|
|32|EF_NAPALMBEAT|Small clustered explosions|
|33|EF_RUWACH|Ruwach|
|34|EF_TELEPORTATION|Old Map Exit Animation (unused)|
|35|EF_READYPORTAL|Old Warp Portal (unused)|
|36|EF_PORTAL|Old Warp Portal (unused)|
|37|EF_INCAGILITY|AGI Up|
|38|EF_DECAGILITY|AGI Down|
|39|EF_AQUA|Aqua Benedicta|
|40|EF_SIGNUM|Signum Crucis|
|41|EF_ANGELUS|Angelus|
|42|EF_BLESSING|Blessing|
|43|EF_INCAGIDEX|Dex + Agi Up|
|44|EF_SMOKE|Little Fog Smoke.|
|45|EF_FIREFLY|Faint Little Ball Things.|
|46|EF_SANDWIND|Sand Wind|
|47|EF_TORCH|Torch|
|48|EF_SPRAYPOND|Small Piece of Glass|
|49|EF_FIREHIT|Firebolt/Wall Hits|
|50|EF_FIRESPLASHHIT|Spinning Fire Thing|
|51|EF_COLDHIT|Ice Elemental Hit|
|52|EF_WINDHIT|Wind Elemental Hit|
|53|EF_POISONHIT|Puff of Purpulish Smoke?|
|54|EF_BEGINSPELL2|Cast Initiation Aura (Water Element)|
|55|EF_BEGINSPELL3|Cast Initiation Aura (Fire Element)|
|56|EF_BEGINSPELL4|Cast Initiation Aura (Earth Element)|
|57|EF_BEGINSPELL5|Cast Initiation Aura (Wind Element)|
|58|EF_BEGINSPELL6|Cast Initiation Aura (Holy Element)|
|59|EF_BEGINSPELL7|Cast Initiation Aura (Poison Element)|
|60|EF_LOCKON|Cast target circle|
|61|EF_WARPZONE|Old Warp Portal (NPC Warp, unused)|
|62|EF_SIGHTRASHER|Sight Trasher|
|63|EF_BARRIER|Moonlight Sphere|
|64|EF_ARROWSHOT|Something Like Puruple/Yellow Light Bullet|
|65|EF_INVENOM|Something Like Absorb of Power|
|66|EF_CURE|	Cure|
|67|EF_PROVOKE|Provoke|
|68|EF_MVP|	MVP Banner|
|69|EF_SKIDTRAP|Skid Trap|
|70|EF_BRANDISHSPEAR|Brandish Spear|
|71|EF_CONE|Spiral White balls|
|72|EF_SPHERE|Bigger Spiral White balls|
|73|EF_BOWLINGBASH|Blue/White Small Aura|
|74|EF_ICEWALL|Ice Wall|
|75|EF_GLORIA|Gloria|
|76|EF_MAGNIFICAT|Magnificat|
|77|EF_RESURRECTION|Resurrection|
|78|EF_RECOVERY|Status Recovery|
|79|EF_EARTHSPIKE|Earth Spike|
|80|EF_SPEARBMR|Spear Boomerang|
|81|EF_PIERCE|Skill hit|
|82|EF_TURNUNDEAD|Turn Undead|
|83|EF_SANCTUARY|Sanctuary|
|84|EF_IMPOSITIO|Impositio Manus|
|85|EF_LEXAETERNA|Lex Aeterna|
|86|EF_ASPERSIO|Aspersio|
|87|EF_LEXDIVINA|Lex Divina|
|88|EF_SUFFRAGIUM|Suffragium|
|89|EF_STORMGUST|Storm Gust|
|90|EF_LORD|	Lord of Vermilion|
|91|EF_BENEDICTIO|B. S. Sacramenti|
|92|EF_METEORSTORM|Meteor Storm|
|93|EF_YUFITEL|Jupitel Thunder (Ball)|
|94|EF_YUFITELHIT|Jupitel Thunder (Hit)|
|95|EF_QUAGMIRE|Quagmire|
|96|EF_FIREPILLAR|Fire Pillar|
|97|EF_FIREPILLARBOMB|	Fire Pillar/Land Mine hit|
|98|EF_HASTEUP|Adrenaline Rush|
|99|EF_FLASHER|Flasher Trap|
|100|EF_REMOVETRAP|Yellow ball fountain|
|101|EF_REPAIRWEAPON|Weapon Repair|
|102|EF_CRASHEARTH|Hammerfall|
|103|EF_PERFECTION|Weapon Perfection|
|104|EF_MAXPOWER|Maximize Power|
|105|EF_BLASTMINE|(nothing)|
|106|EF_BLASTMINEBOMB|Blast Mine Trap|
|107|EF_CLAYMORE|Claymore Trap|
|108|EF_FREEZING|Freezing Trap|
|109|EF_BUBBLE|Bailaban Blue bubble Map Effect|
|110|EF_GASPUSH|Trap Used by Giearth|
|111|EF_SPRINGTRAP|Spring Trap|
|112|EF_KYRIE|Kyrie Eleison|
|113|EF_MAGNUS|Magnus Exorcismus|
|114|EF_BOTTOM|Old Magnus Exorcismus Map Unit (unused)|
|115|EF_BLITZBEAT|Blitz Beat|
|116|EF_WATERBALL|Fling Watersphere|
|117|EF_WATERBALL2|Waterball|
|118|EF_FIREIVY|Fling Firesphere|
|119|EF_DETECTING|Detect|
|120|EF_CLOAKING|Cloaking|
|121|EF_SONICBLOW|Sonic Blow (Part 1/2)|
|122|EF_SONICBLOWHIT|Multi hit effect|
|123|EF_GRIMTOOTH|Grimtooth Cast|
|124|EF_VENOMDUST|Venom Dust|
|125|EF_ENCHANTPOISON|Enchant Poison|
|126|EF_POISONREACT|Poison React|
|127|EF_POISONREACT2|Small Posion React|
|128|EF_OVERTHRUST|Over Thrust|
|129|EF_SPLASHER|Venom Splasher Explosion|
|130|EF_TWOHANDQUICKEN|Two-Hand Quicken|
|131|EF_AUTOCOUNTER|Auto-Counter Hit|
|132|EF_GRIMTOOTHATK|Grimtooth Hit|
|133|EF_FREEZE|Ice Effect (Used by NPCs)|
|134|EF_FREEZED|Ice Effect (Used by NPCs)|
|135|EF_ICECRASH|Ice Effect (Used by NPCs)|
|136|EF_SLOWPOISON|Slow Poison|
|137|EF_BOTTOM2|Old Sanctuary Map Unit (unused)|
|138|EF_FIREPILLARON|Fire pillar|
|139|EF_SANDMAN|Sandman Trap|
|140|EF_REVIVE|Ressurection Aura|
|141|EF_PNEUMA|Pneuma|
|142|EF_HEAVENSDRIVE|Heaven's Drive|
|143|EF_SONICBLOW2|Sonic Blow (Part 2/2)|
|144|EF_BRANDISH2|Brandish Spear Pre-Hit Effect|
|145|EF_SHOCKWAVE|Shockwave Trap|
|146|EF_SHOCKWAVEHIT|Shockwave Trap Hit|
|147|EF_EARTHHIT|Pierce Hit|
|148|EF_PIERCESELF|Pierce Cast Animation|
|149|EF_BOWLINGSELF|Bowling Bash|
|150|EF_SPEARSTABSELF|Pierce Cast Animation|
|151|EF_SPEARBMRSELF|Spear Boomerang Cast|
|152|EF_HOLYHIT|Turn Undead|
|153|EF_CONCENTRATION|Increase Concentration|
|154|EF_REFINEOK|Refine Success|
|155|EF_REFINEFAIL|Refine Fail|
|156|EF_JOBCHANGE|jobchange.str not found error|
|157|EF_LVUP|levelup.str not found error|
|158|EF_JOBLVUP|Job Level Up|
|159|EF_TOPRANK|PvP circle|
|160|EF_PARTY|PvP Party Circle|
|161|EF_RAIN|(Nothing)|
|162|EF_SNOW|Snow|
|163|EF_SAKURA|White Sakura Leaves|
|164|EF_STATUS_STATE|(Nothing)|
|165|EF_BANJJAKII|Comodo Fireworks Ball|
|166|EF_MAKEBLUR|Energy Coat (Visual Effect)|
|167|EF_TAMINGSUCCESS|(Nothing)|
|168|EF_TAMINGFAILED|(Nothing)|
|169|EF_ENERGYCOAT|Energy Coat Animation|
|170|EF_CARTREVOLUTION|Cart Revolution|
|171|EF_VENOMDUST2|Venom Dust Map Unit|
|172|EF_CHANGEDARK|Change Element (Dark)|
|173|EF_CHANGEFIRE|Change Element (Fire)|
|174|EF_CHANGECOLD|Change Element (Water)|
|175|EF_CHANGEWIND|Change Element (Wind)|
|176|EF_CHANGEFLAME|Change Element (Fire)|
|177|EF_CHANGEEARTH|Change Element (Earth)|
|178|EF_CHAINGEHOLY|Change Element (Holy)|
|179|EF_CHANGEPOISON|Change Element (Poison)|
|180|EF_HITDARK|Darkness Attack|
|181|EF_MENTALBREAK|Mental Breaker|
|182|EF_MAGICALATTHIT|Magical Hit|
|183|EF_SUI_EXPLOSION|Self Destruction|
|184|EF_DARKATTACK|(Nothing)|
|185|EF_SUICIDE|(Nothing)|
|186|EF_COMBOATTACK1|Combo Attack 1|
|187|EF_COMBOATTACK2|Combo Attack 2|
|188|EF_COMBOATTACK3|Combo Attack 3|
|189|EF_COMBOATTACK4|Combo Attack 4|
|190|EF_COMBOATTACK5|Combo Attack 5|
|191|EF_GUIDEDATTACK|Guided Attack|
|192|EF_POISONATTACK|Poison Attack|
|193|EF_SILENCEATTACK|Silence Attack|
|194|EF_STUNATTACK|Stun Attack|
|195|EF_PETRIFYATTACK|Petrify Attack|
|196|EF_CURSEATTACK|Curse Attack|
|197|EF_SLEEPATTACK|Sleep Attack|
|198|EF_TELEKHIT|(Nothing)|
|199|EF_PONG|	Small Popping Bubble Map Effect|
|200|EF_LEVEL99|Normal level 99 Aura (Middle)|
|201|EF_LEVEL99_2|Normal level 99 Aura (Bottom)|
|202|EF_LEVEL99_3|Lv 99 Aura Bubble|
|203|EF_GUMGANG|Fury (Visual Effect)|
|204|EF_POTION1|Red Herb/Potion|
|205|EF_POTION2|Orange Potion|
|206|EF_POTION3|Yellow Herb/Potion|
|207|EF_POTION4|White Herb/Potion|
|208|EF_POTION5|Blue Herb/Potion|
|209|EF_POTION6|Green Herb/Potion|
|210|EF_POTION7|Yellow Circle Healing Effect|
|211|EF_POTION8|Blue Circle Healing Effect|
|212|EF_DARKBREATH|Dark Breath|
|213|EF_DEFFENDER|Defender|
|214|EF_KEEPING|Keeping|
|215|EF_SUMMONSLAVE|Summon Slave|
|216|EF_BLOODDRAIN|Blood Drain|
|217|EF_ENERGYDRAIN|Energy Drain|
|218|EF_POTION_CON|Concentration Potion|
|219|EF_POTION_|Awakening Potion|
|220|EF_POTION_BERSERK|Berserk Potion|
|221|EF_POTIONPILLAR|Intense light beam|
|222|EF_DEFENDER|Defender (Crusader)|
|223|EF_GANBANTEIN|Holy Cast Aura|
|224|EF_WIND|	Wind (Map effect)|
|225|EF_VOLCANO|Volcano casting effect|
|226|EF_GRANDCROSS|Grand Cross Effect|
|227|EF_INTIMIDATE|Snatch|
|228|EF_CHOOKGI|(Nothing)|
|229|EF_CLOUD|(Nothing)|
|230|EF_CLOUD2|(Nothing)|
|231|EF_MAPPILLAR|Map Light Pillar Animation 1|
|232|EF_LINELINK|Sacrifice (Visual Effect)|
|233|EF_CLOUD3|Fog|
|234|EF_SPELLBREAKER|Spell Breaker|
|235|EF_DISPELL|Dispell|
|236|EF_DELUGE|Deluge Cast Aura|
|237|EF_VIOLENTGALE|Violent Gale Cast Aura|
|238|EF_LANDPROTECTOR|Magnetic Earth Cast Aura|
|239|EF_BOTTOM_VO|Volcano (Visual Effect)|
|240|EF_BOTTOM_DE|Deluge (Visual Effect)|
|241|EF_BOTTOM_VI|Violent Gale (Visual Effect)|
|242|EF_BOTTOM_LA|Magnetic Earth (Visual Effect)|
|243|EF_FASTMOVE|(Invalid)|
|244|EF_MAGICROD|Magic Rod|
|245|EF_HOLYCROSS|Holy Cross|
|246|EF_SHIELDCHARGE|Shield Charge|
|247|EF_MAPPILLAR2|Map Light Pillar Animation 2|
|248|EF_PROVIDENCE|Resistant Souls|
|249|EF_SHIELDBOOMERANG|Shield Boomerang|
|250|EF_SPEARQUICKEN|Spear Quicken|
|251|EF_DEVOTION|Devotion|
|252|EF_REFLECTSHIELD|Reflect Shield|
|253|EF_ABSORBSPIRITS|Absorb Spirit Spheres|
|254|EF_STEELBODY|Mental Strength (Visual Effect)|
|255|EF_FLAMELAUNCHER|Elemental Endow (Fire)|
|256|EF_FROSTWEAPON|Elemental Endow (Water)|
|257|EF_LIGHTNINGLOADER|Elemental Endow (Wind)|
|258|EF_SEISMICWEAPON|Elemental Endow (Earth)|
|259|EF_MAPPILLAR3|Map Light Pillar Animation 3|
|260|EF_MAPPILLAR4|Map Light Pillar Animation 4|
|261|EF_GUMGANG2|Fury Cast Animation|
|262|EF_TEIHIT1|Raging Quadruple Blow|
|263|EF_GUMGANG3|Raging Quadruple Blow 2|
|264|EF_TEIHIT2|(Nothing)|
|265|EF_TANJI|Throw Spirit Sphere|
|266|EF_TEIHIT1X|Raging Quadruple Blow 3|
|267|EF_CHIMTO|Occult Impaction|
|268|EF_STEALCOIN|Steal Coin|
|269|EF_STRIPWEAPON|Divest Weapon|
|270|EF_STRIPSHIELD|Divest Shield|
|271|EF_STRIPARMOR|Divest Armor|
|272|EF_STRIPHELM|Divest Helm|
|273|EF_CHAINCOMBO|Raging Quadruple Blow 4|
|274|EF_RG_COIN|Steal Coin Animation|
|275|EF_BACKSTAP|Back Stab Animation|
|276|EF_TEIHIT3|Raging Thrust|
|277|EF_BOTTOM_DISSONANCE|Dissoance Map Unit|
|278|EF_BOTTOM_LULLABY|Lullaby Map Unit|
|279|EF_BOTTOM_RICHMANKIM|Mr Kim a Rich Man Map Unit|
|280|EF_BOTTOM_ETERNALCHAOS|Eternal Chaos Map Unit|
|281|EF_BOTTOM_DRUMBATTLEFIELD|A Drum on the Battlefield Map Unit|
|282|EF_BOTTOM_RINGNIBELUNGEN|The Ring Of Nibelungen Map Unit|
|283|EF_BOTTOM_ROKISWEIL|Loki's Veil Map Unit|
|284|EF_BOTTOM_INTOABYSS|Into the Abyss Map Unit|
|285|EF_BOTTOM_SIEGFRIED|Invunerable Siegfriend Map Unit|
|286|EF_BOTTOM_WHISTLE|A Wistle Map Unit|
|287|EF_BOTTOM_ASSASSINCROSS|Assassin Cross of Sunset Map Unit|
|288|EF_BOTTOM_POEMBRAGI|A Poem of Bragi Map Unit|
|289|EF_BOTTOM_APPLEIDUN|The Apple Of Idun Map Unit|
|290|EF_BOTTOM_UGLYDANCE|Ugly Dance Map Unit|
|291|EF_BOTTOM_HUMMING|Humming Map Unit|
|292|EF_BOTTOM_DONTFORGETME|Please don't Forget Me Map Unit|
|293|EF_BOTTOM_FORTUNEKISS|Fortune's Kiss Map Unit|
|294|EF_BOTTOM_SERVICEFORYOU|Service For You Map Unit|
|295|EF_TALK_FROSTJOKE|Frost Joke|
|296|EF_TALK_SCREAM|Scream|
|297|EF_POKJUK|Fire Works (Visual Effect)|
|298|EF_THROWITEM|Acid Terror Animnation|
|299|EF_THROWITEM2|(Nothing)|
|300|EF_CHEMICALPROTECTION|Chemical Protection|
|301|EF_POKJUK_SOUND|Fire Works (Sound Effect)|
|302|EF_DEMONSTRATION|Bomb|
|303|EF_CHEMICAL2|(Unused)|
|304|EF_TELEPORTATION2|Teleportation Animation|
|305|EF_PHARMACY_OK|Pharmacy Success|
|306|EF_PHARMACY_FAIL|Pharmacy Failed|
|307|EF_FORESTLIGHT|Forest Light 1|
|308|EF_THROWITEM3|Throw Stone|
|309|EF_FIRSTAID|First Aid|
|310|EF_SPRINKLESAND|Sprinkle Sand|
|311|EF_LOUD|	Crazy Uproar|
|312|EF_HEAL|	Heal Effect|
|313|EF_HEAL2|Heal Effect 2|
|314|EF_EXIT2|Old Map Exit effect (Unused)|
|315|EF_GLASSWALL2|Safety Wall|
|316|EF_READYPORTAL2|Warp Portal Animation 1|
|317|EF_PORTAL2|Warp Portal Animation 2|
|318|EF_BOTTOM_MAG|Magnus Exorcisimus Map Unit|
|319|EF_BOTTOM_SANC|Sanctuary Map Unit|
|320|EF_HEAL3|Offensive Heal|
|321|EF_WARPZONE2|Warp NPC|
|322|EF_FORESTLIGHT2|Forest Light 2|
|323|EF_FORESTLIGHT3|Forest Light 3|
|324|EF_FORESTLIGHT4|Forest Light 4|
|325|EF_HEAL4|Heal Effect 4|
|326|EF_FOOT|	Chase Walk Left Foot|
|327|EF_FOOT2|Chse Walk Right Foot|
|328|EF_BEGINASURA|Monk Asura Strike|
|329|EF_TRIPLEATTACK|Triple Strike|
|330|EF_HITLINE|Combo Finish|
|331|EF_HPTIME|Natural HP Regeneration|
|332|EF_SPTIME|Natural SP Regeneration|
|333|EF_MAPLE|Autumn Leaves|
|334|EF_BLIND|Blind|
|335|EF_POISON|Poison|
|336|EF_GUARD|Kyrie Eleison/Parrying Shield|
|337|EF_JOBLVUP50|Class Change|
|338|EF_ANGEL2|Super Novice/Taekwon Level Up Angel|
|339|EF_MAGNUM2|Spiral Pierce|
|340|EF_CALLZONE|(Nothing)|
|341|EF_PORTAL3|Wedding Warp Portal|
|342|EF_COUPLECASTING|Wedding Skill|
|343|EF_HEARTCASTING|Another Merry Skill|
|344|EF_ENTRY2|Character map entry effect|
|345|EF_SAINTWING|Wings (Animated)|
|346|EF_SPHEREWIND|Like Moonlight But Blue|
|347|EF_COLORPAPER|Wedding Ceremony|
|348|EF_LIGHTSPHERE|Like 1000 Blade trepassing|
|349|EF_WATERFALL|Waterfall (Horizonatal)|
|350|EF_WATERFALL_90|Waterfall (Vertical)|
|351|EF_WATERFALL_SMALL|Small Waterfall (Horizonatal)|
|352|EF_WATERFALL_SMALL_90|Small Waterfall (Vertical)|
|353|EF_WATERFALL_T2|Dark Waterfall (Horizonatal)|
|354|EF_WATERFALL_T2_90|Dark Waterfall (Vertical)|
|355|EF_WATERFALL_SMALL_T2|Dark Small Waterfall (Horizonatal)|
|356|EF_WATERFALL_SMALL_T2_90|Dark Small Waterfall (Vertical)|
|357|EF_MINI_TETRIS|(Nothing)|
|358|EF_GHOST|Niflheim Ghost|
|359|EF_BAT|	Niflheim Bat Slow|
|360|EF_BAT2|	Niflheim Bat Fast|
|361|EF_SOULBREAKER|Soul Destroyer|
|362|EF_LEVEL99_4|Trancendant Level 99 Aura 1|
|363|EF_VALLENTINE|Valentine Day Heart With Wings|
|364|EF_VALLENTINE2|Valentine Day Heart|
|365|EF_PRESSURE|Gloria Domini|
|366|EF_BASH3D|Martyr's Reckoning|
|367|EF_AURABLADE|Aura Blade|
|368|EF_REDBODY|Berserk|
|369|EF_LKCONCENTRATION|Concentration|
|370|EF_BOTTOM_GOSPEL|Gospel Map Unit|
|371|EF_ANGEL|Level Up|
|372|EF_DEVIL|Death|
|373|EF_DRAGONSMOKE|House Smoke|
|374|EF_BOTTOM_BASILICA|Basilica|
|375|EF_ASSUMPTIO|Assumptio (Visual Effect)|
|376|EF_HITLINE2|Palm Strike|
|377|EF_BASH3D2|Matyr's Reckoning 2|
|378|EF_ENERGYDRAIN2|Soul Drain (1st Part)|
|379|EF_TRANSBLUEBODY|Soul Drain (2nd Part)|
|380|EF_MAGICCRASHER|Magic Crasher|
|381|EF_LIGHTSPHERE2|Blue Starburst (Unknown use)|
|382|EF_LIGHTBLADE|(Nothing)|
|383|EF_ENERGYDRAIN3|Health Conversion|
|384|EF_LINELINK2|Soul Change (Sound Effect)|
|385|EF_LINKLIGHT|Soul Change (Visual Effect)|
|386|EF_TRUESIGHT|True Sight|
|387|EF_FALCONASSAULT|Falcon Assault|
|388|EF_TRIPLEATTACK2|Focused Arrow Strike (Sound Effect)|
|389|EF_PORTAL4|Wind Walk|
|390|EF_MELTDOWN|Shattering Strike|
|391|EF_CARTBOOST|Cart Boost|
|392|EF_REJECTSWORD|Reject Sword|
|393|EF_TRIPLEATTACK3|Arrow Vulcan|
|394|EF_SPHEREWIND2|Sheltering Bliss|
|395|EF_LINELINK3|Marionette Control (Sound Effect)|
|396|EF_PINKBODY|Marionette Control (Visual Effect)|
|397|EF_LEVEL99_5|Trancended 99 Aura (Middle)|
|398|EF_LEVEL99_6|Trancended 99 Aura (Bottom)|
|399|EF_BASH3D3|Head Crush|
|400|EF_BASH3D4|Joint Beat|
|401|EF_NAPALMVALCAN|Napalm Vulcan Sound|
|402|EF_PORTAL5|Dangerous Soul Collect|
|403|EF_MAGICCRASHER2|Mind Breaker|
|404|EF_BOTTOM_SPIDER|Fiber Lock|
|405|EF_BOTTOM_FOGWALL|Wall Of Fog|
|406|EF_SOULBURN|Soul Burn|
|407|EF_SOULCHANGE|Soul Change|
|408|EF_BABY|	Mom, Dad, I love you! (Baby Skill)|
|409|EF_SOULBREAKER2|Meteor Assault|
|410|EF_RAINBOW|Rainbow|
|411|EF_PEONG|Leap|
|412|EF_TANJI2|Like Throw Spirit Sphere|
|413|EF_PRESSEDBODY|Axe Kick|
|414|EF_SPINEDBODY|Round Kick|
|415|EF_KICKEDBODY|Counter Kick|
|416|EF_AIRTEXTURE|(Nothing)|
|417|EF_HITBODY|Flash|
|418|EF_DOUBLEGUMGANG|Warmth Lightning|
|419|EF_REFLECTBODY|Kaite (Visual Effect)|
|420|EF_BABYBODY|Eswoo (Small) (Visual Effect)|
|421|EF_BABYBODY2|Eswoo (Alt. Small) (Visual Effect)|
|422|EF_GIANTBODY|Eswoo (Normal) (Visual Effect)|
|423|EF_GIANTBODY2|Eswoo (Alt. Normal) (Visual Effect)|
|424|EF_ASURABODY|Spirit Link (Visual Effect)|
|425|EF_4WAYBODY|Esma Hit (Visual Effect)|
|426|EF_QUAKEBODY|Sprint Collision (Visual Effect)|
|427|EF_ASURABODY_MONSTER|(Nothing)|
|428|EF_HITLINE3|(Nothing)|
|429|EF_HITLINE4|Taekwon Kick Hit 1|
|430|EF_HITLINE5|Taekwon Kick Hit 2|
|431|EF_HITLINE6|Taekwon Kick Hit 3|
|432|EF_ELECTRIC|Solar, Lunar and Stellar Perception (Visual Effect)|
|433|EF_ELECTRIC2|Solar, Lunar and Stellar Opposition (Visual Effect)|
|434|EF_STORMKICK|Taekwon Kick Hit 4|
|435|EF_HITLINE7|Whirlwind Kick|
|436|EF_STORMKICK|White Barrier (Unused)|
|437|EF_HALFSPHERE|White barrier 2 (Unused)|
|438|EF_ATTACKENERGY|Kaite Reflect Animation|
|439|EF_ATTACKENERGY2|Flying Side Kick|
|440|EF_ASSUMPTIO2|Assumptio (Animation)|
|441|EF_BLUECASTING|Comfort Skills Cast Aura|
|442|EF_RUN|Foot Prints caused by Sprint.|
|443|EF_STOPRUN|(Nothing)|
|444|EF_STOPEFFECT|Sprint Stop Animation|
|445|EF_JUMPBODY|High Jump (Jump)|
|446|EF_LANDBODY|High Jump (Return Down)|
|447|EF_FOOT3|Running Left Foot|
|448|EF_FOOT4|Running Right Foot|
|449|EF_TAE_READY|KA-Spell (1st Part)|
|450|EF_GRANDCROSS2|Darkcross|
|451|EF_SOULSTRIKE2|Dark Strike|
|452|EF_YUFITEL2|Something Like Jupitel Thunder|
|453|EF_NPC_STOP|Paralized|
|454|EF_DARKCASTING|Like Blind|
|455|EF_GUMGANGNPC|Another Warmth Lightning|
|456|EF_AGIUP|Power Up|
|457|EF_JUMPKICK|Flying Side Kick (2nd Part)|
|458|EF_QUAKEBODY2|Running/Sprint (running into a wall)|
|459|EF_STORMKICK1|Brown tornado that spins sprite (unused)|
|460|EF_STORMKICK2|Green tornado (unused)|
|461|EF_STORMKICK3|Blue tornado (unused)|
|462|EF_STORMKICK4|Kaupe Dodge Effect|
|463|EF_STORMKICK5|Kaupe Dodge Effect|
|464|EF_STORMKICK6|White tornado (unused)|
|465|EF_STORMKICK7|Purple tornado (unused)|
|466|EF_SPINEDBODY2|Another Round Kick|
|467|EF_BEGINASURA1|Warm/Mild Wind (Earth)|
|468|EF_BEGINASURA2|Warm/Mild Wind (Wind)|
|469|EF_BEGINASURA3|Warm/Mild Wind (Water)|
|470|EF_BEGINASURA4|Warm/Mild Wind (Fire)|
|471|EF_BEGINASURA5|Warm/Mild Wind (Undead)|
|472|EF_BEGINASURA6|Warm/Mild Wind (Shadow)|
|473|EF_BEGINASURA7|Warm/Mild Wind (Holy)|
|474|EF_AURABLADE2|(Nothing)|
|475|EF_DEVIL1|Demon of The Sun Moon And Stars (Level 1)|
|476|EF_DEVIL2|Demon of The Sun Moon And Stars (Level 2)|
|477|EF_DEVIL3|Demon of The Sun Moon And Stars (Level 3)|
|478|EF_DEVIL4|Demon of The Sun Moon And Stars (Level 4)|
|479|EF_DEVIL5|Demon of The Sun Moon And Stars (Level 5)|
|480|EF_DEVIL6|Demon of The Sun Moon And Stars (Level 6)|
|481|EF_DEVIL7|Demon of The Sun Moon And Stars (Level 7)|
|482|EF_DEVIL8|Demon of The Sun Moon And Stars (Level 8)|
|483|EF_DEVIL9|Demon of The Sun Moon And Stars (Level 9)|
|484|EF_DEVIL10|Demon of The Sun Moon And Stars (Level 10)|
|485|EF_DOUBLEGUMGANG2|Mental Strength Lightning but White|
|486|EF_DOUBLEGUMGANG3|Mental Strength Lightning|
|487|EF_BLACKDEVIL|Demon of The Sun Moon And Stars Ground Effect|
|488|EF_FLOWERCAST|Comfort Skills|
|489|EF_FLOWERCAST2|(Nothing)|
|490|EF_FLOWERCAST3|(Nothing)|
|491|EF_MOCHI|Element Potions|
|492|EF_LAMADAN|Cooking Foods|
|493|EF_EDP|	Enchant Deadly Poison|
|494|EF_SHIELDBOOMERANG2|Throwing Tomahawk|
|495|EF_RG_COIN2|Full Strip Sound|
|496|EF_GUARD2|Preserve|
|497|EF_SLIM|	Twilight Alchemy 1|
|498|EF_SLIM2|Twilight Alchemy 2|
|499|EF_SLIM3|Twilight Alchemy 3|
|500|EF_CHEMICALBODY|Player Become Blue with Blue Aura|
|501|EF_CASTSPIN|Chase Walk Animation|
|502|EF_PIERCEBODY|Player Become Yellow with Yellow Aura|
|503|EF_SOULLINK|Soul Link Word|
|504|EF_CHOOKGI2|(Nothing)|
|505|EF_MEMORIZE|Memorize|
|506|EF_SOULLIGHT|(Nothing)|
|507|EF_MAPAE|Authoritative Badge|
|508|EF_ITEMPOKJUK|Fire Cracker|
|509|EF_05VAL|Valentine Day Hearth (Wings)|
|510|EF_BEGINASURA11|Champion Asura Strike|
|511|EF_NIGHT|(Nothing)|
|512|EF_CHEMICAL2DASH|Chain Crush Combo|
|513|EF_GROUNDSAMPLE|Area Cast|
|514|EF_GI_EXPLOSION|Really Big Circle|
|515|EF_CLOUD4|Einbroch Fog|
|516|EF_CLOUD5|Airship Cloud|
|517|EF_BOTTOM_HERMODE|(Nothing)|
|518|EF_CARTTER|Cart Termination|
|519|EF_ITEMFAST|Speed Down Potion|
|520|EF_SHIELDBOOMERANG3|Shield Bumerang|
|521|EF_DOUBLECASTBODY|Player Become Red with Red Aura|
|522|EF_GRAVITATION|Gravitation Field|
|523|EF_TAROTCARD1|Tarot Card of Fate (The Fool)|
|524|EF_TAROTCARD2|Tarot Card of Fate (The Magician)|
|525|EF_TAROTCARD3|Tarot Card of Fate (The High Priestess)|
|526|EF_TAROTCARD4|Tarot Card of Fate (The Chariot)|
|527|EF_TAROTCARD5|Tarot Card of Fate (Strength)|
|528|EF_TAROTCARD6|Tarot Card of Fate (The Lovers)|
|529|EF_TAROTCARD7|Tarot Card of Fate (The Wheel of Fortune)|
|530|EF_TAROTCARD8|Tarot Card of Fate (The Hanged Man)|
|531|EF_TAROTCARD9|Tarot Card of Fate (Death)|
|532|EF_TAROTCARD10|Tarot Card of Fate (Temperance)|
|533|EF_TAROTCARD11|Tarot Card of Fate (The Devil)|
|534|EF_TAROTCARD12|Tarot Card of Fate (The Tower)|
|535|EF_TAROTCARD13|Tarot Card of Fate (The Star)|
|536|EF_TAROTCARD14|Tarot Card of Fate (The Sun)|
|537|EF_ACIDDEMON|Acid Demonstration|
|538|EF_GREENBODY|Player Become Green with Green Aura|
|539|EF_THROWITEM4|Throw Random Bottle|
|540|EF_BABYBODY_BACK|Instant Small->Normal|
|541|EF_THROWITEM5|(Nothing)|
|542|EF_BLUEBODY|KA-Spell (1st Part)|
|543|EF_HATED|Kahii|
|544|EF_REDLIGHTBODY|Warmth Red Sprite|
|545|EF_RO2YEAR|Sound And... PUFF Client Crash :P|
|546|EF_SMA_READY|Kaupe|
|547|EF_STIN|	Estin|
|548|EF_RED_HIT|Instant Red Sprite|
|549|EF_BLUE_HIT|Instant Blue Sprite|
|550|EF_QUAKEBODY3|Another Effect like Running Hit|
|551|EF_SMA	|EFfect Like Estun but with Circle|
|552|EF_SMA2|	(Nothing)|
|553|EF_STIN2|Esma|
|554|EF_HITTEXTURE|Large White Cloud|
|555|EF_STIN3|Estun|
|556|EF_SMA3|	(Nothing)|
|557|EF_BLUEFALL|Juperos Energy Waterfall (Horizontal)|
|558|EF_BLUEFALL_90|Juperos Energy Waterfall (Vertical)|
|559|EF_FASTBLUEFALL|Juperos Energy Waterfall Fast (Horizontal)|
|560|EF_FASTBLUEFALL_90|Juperos Energy Waterfall Fast (Vertical)|
|561|EF_BIG_PORTAL|Juperos Warp|
|562|EF_BIG_PORTAL2|Juperos Warp|
|563|EF_SCREEN_QUAKE|Earthquake Effect (Juperos Elevator)|
|564|EF_HOMUNCASTING|Wedding Cast|
|565|EF_HFLIMOON1|Filir Moonlight Lvl 1|
|566|EF_HFLIMOON2|Filir Moonlight Lvl 2|
|567|EF_HFLIMOON3|Filir Moonlight Lvl 3|
|568|EF_HO_UP|Another Job Level Up|
|569|EF_HAMIDEFENCE|Amistr Bulwark|
|570|EF_HAMICASTLE|Amistr Castling|
|571|EF_HAMIBLOOD|Amistr Bloodlust|
|572|EF_HATED2|Warmth Soul|
|573|EF_TWILIGHT1|Twilight Alchemy 1|
|574|EF_TWILIGHT2|Twilight Alchemy 2|
|575|EF_TWILIGHT3|Twilight Alchemy 3|
|576|EF_ITEM_THUNDER|Box Effect (Thunder)|
|577|EF_ITEM_CLOUD|Box Effect (Cloud)|
|578|EF_ITEM_CURSE|Box Effect (Curse)|
|579|EF_ITEM_ZZZ|Box Effect (Sleep)|
|580|EF_ITEM_RAIN|Box Effect (Rain)|
|581|EF_ITEM_LIGHT|Box Effect (Sunlight)|
|582|EF_ANGEL3|Another Super Novice/Taekwon Angel|
|583|EF_M01|	Warmth Hit|
|584|EF_M02|	Full Buster|
|585|EF_M03|	5 Medium Size Explosion|
|586|EF_M04|	Somatology Lab Mobs Aura|
|587|EF_M05|	Big Purple Flame|
|588|EF_M06|	Little Red Flame|
|589|EF_M07|	Eswoo|
|590|EF_KAIZEL|Running Stop|
|591|EF_KAAHI|(Nothing)|
|592|EF_CLOUD6|Thanatos Tower Bloody Clouds|
|593|EF_FOOD01|Food Effect (STR)|
|594|EF_FOOD02|Food Effect (INT)|
|595|EF_FOOD03|Food Effect (VIT)|
|596|EF_FOOD04|Food Effect (AGI)|
|597|EF_FOOD05|Food Effect (DEX)|
|598|EF_FOOD06|Food Effect (LUK)|
|599|EF_SHRINK|Cast Time Sound and Flashing Animation on Player|
|600|EF_THROWITEM6|Throw Venom Knife|
|601|EF_SIGHT2|Sight Blaster|
|602|EF_QUAKEBODY4|Close Confine (Grab Effect)|
|603|EF_FIREHIT2|Spinning fire ball (like 50, but smaller)|
|604|EF_NPC_STOP2|Close Confine (Ground Effect)|
|605|EF_NPC_STOP2_DEL|(Nothing)|
|606|EF_FVOICE|Pang Voice (Visual Effect)|
|607|EF_WINK|Wink of Charm (Visual Effect)|
|608|EF_COOKING_OK|Cooking Success|
|609|EF_COOKING_FAIL|Cooking Failed|
|610|EF_TEMP_OK|Success|
|611|EF_TEMP_FAIL|Failed|
|612|EF_HAPGYEOK|Korean Words and /no1 Emoticon|
|613|EF_THROWITEM7|Throw Shuriken|
|614|EF_THROWITEM8|Throw Kunai|
|615|EF_THROWITEM9|Throw Fumma Shuriken|
|616|EF_THROWITEM10|Throw Money|
|617|EF_BUNSINJYUTSU|Illusionary Shadow|
|618|EF_KOUENKA|Crimson Fire Bolossom|
|619|EF_HYOUSENSOU|Lightning Spear Of Ice|
|620|EF_BOTTOM_SUITON|Water Escape Technique|
|621|EF_STIN4|Wind Blade|
|622|EF_THUNDERSTORM2|Lightning Crash|
|623|EF_CHEMICAL4|Piercing Shot|
|624|EF_STIN5|Kamaitachi|
|625|EF_MADNESS_BLUE|Madness Canceller|
|626|EF_MADNESS_RED|Adjustment|
|627|EF_RG_COIN3|Disarm (Sound Effect)|
|628|EF_BASH3D5|Dust|
|629|EF_CHOOKGI3|(Nothing)|
|630|EF_KIRIKAGE|Shadow Slash|
|631|EF_TATAMI|Reverse Tatami Map Unit|
|632|EF_KASUMIKIRI|Mist Slash|
|633|EF_ISSEN|Final Strike|
|634|EF_KAEN|	Crimson Fire Formation|
|635|EF_BAKU|	Dragon Fire Formation|
|636|EF_HYOUSYOURAKU|Falling Ice Pillar|
|637|EF_DESPERADO|Desperado|
|638|EF_LIGHTNING_S|Ground Drift Grenade|
|639|EF_BLIND_S|Ground Drift Grenade|
|640|EF_POISON_S|Ground Drift Grenade|
|641|EF_FREEZING_S|Ground Drift Grenade|
|642|EF_FLARE_S|Ground Drift Grenade|
|643|EF_RAPIDSHOWER|Rapid Shower|
|644|EF_MAGICALBULLET|Magic Bullet|
|645|EF_SPREADATTACK|Spread Attack|
|646|EF_TRACKCASTING|Tracking (Shown While Casting)|
|647|EF_TRACKING|Tracking|
|648|EF_TRIPLEACTION|Triple Action|
|649|EF_BULLSEYE|Bull's Eye|
|650|EF_MAP_MAGICZONE|Ice Cave Level 4 Circle|
|651|EF_MAP_MAGICZONE2|Ice Cave Level 4 Big Circle|
|652|EF_DAMAGE1|Like Regeneration Number but Red with a Sound|
|653|EF_DAMAGE1_2|Like Regeneration Number but Red|
|654|EF_DAMAGE1_3|Like Regeneration Number but Purple|
|655|EF_UNDEADBODY|Mobs Skill (Change Undead Element)|
|656|EF_UNDEADBODY_DEL|Last animation before Change Undead Element finish|
|657|EF_GREEN_NUMBER|(Nothing)|
|658|EF_BLUE_NUMBER|(Nothing)|
|659|EF_RED_NUMBER|(Nothing)|
|660|EF_PURPLE_NUMBER|(Nothing)|
|661|EF_BLACK_NUMBER|(Nothing)|
|662|EF_WHITE_NUMBER|(Nothing)|
|663|EF_YELLOW_NUMBER|(Nothing)|
|664|EF_PINK_NUMBER|(Nothing)|
|665|EF_BUBBLE_DROP|Little Blue Ball Falling From the Sky|
|666|EF_NPC_EARTHQUAKE|Earthquake|
|667|EF_DA_SPACE|(Nothing)|
|668|EF_DRAGONFEAR|Dragonfear|
|669|EF_BLEEDING|Wide Bleeding|
|670|EF_WIDECONFUSE|Dragon fear (Visual Effect)|
|671|EF_BOTTOM_RUNNER|The Japan Earth Symbol (like 'Seven Wind Lv1', but on the ground)|
|672|EF_BOTTOM_TRANSFER|The Japan Wind Symbol (like 'Seven Wind Lv2', but on the ground)|
|673|EF_CRYSTAL_BLUE|Map turns Blue (like Soul Link)|
|674|EF_BOTTOM_EVILLAND|Evil Land Cell|
|675|EF_GUARD3|Like Parrying/Kyrie Eleison barrier but Yellow with small Cross in every barrier piece|
|676|EF_NPC_SLOWCAST|Slow Casting|
|677|EF_CRITICALWOUND|Critical Wounds/Bleeding Attack|
|678|EF_GREEN99_3|White 99 Aura Bubbles|
|679|EF_GREEN99_5|Green Aura (Middle)|
|680|EF_GREEN99_6|Green Aura (Bottom)|
|681|EF_MAPSPHERE|Dimensional Gorge Map Effect|
|682|EF_POK_LOVE|I Love You Banner|
|683|EF_POK_WHITE|Happy White Day Banner|
|684|EF_POK_VALEN|Happy Valentine Day Banner|
|685|EF_POK_BIRTH|Happy Birthday Banner|
|686|EF_POK_CHRISTMAS|Merry Christmas Banner|
|687|EF_MAP_MAGICZONE3|Cast Circle-Like effect 1|
|688|EF_MAP_MAGICZONE4|Cast Circle-Like effect 2|
|689|EF_DUST|Endless Tower Map Effect|
|690|EF_TORCH_RED|Burning Flame (Red)|
|691|EF_TORCH_GREEN|Burning Flame (Green)|
|692|EF_MAP_GHOST|Unknown Aura Bubbles (Small ghosts)|
|693|EF_GLOW1|Translucent yellow circle|
|694|EF_GLOW2|Translucent green circle|
|695|EF_GLOW4|Rotating green light|
|696|EF_TORCH_PURPLE|The same of 690 and 691 but Blue/Purple|
|697|EF_CLOUD7|(Nothing)|
|698|EF_CLOUD8|(Nothing)|
|699|EF_FLOWERLEAF|Fall of powder from the sky and raise of some leaf|
|700|EF_MAPSPHERE2|Big Colored Green Sphere.|
|701|EF_GLOW11|Huge Blue Sphere|
|702|EF_GLOW12|Little Colored Violet Sphere|
|703|EF_CIRCLELIGHT|Light Infiltration with fall of pownder|
|704|EF_ITEM315|Client Error (mobile_ef02.str)|
|705|EF_ITEM316|Client Error (mobile_ef01.str)|
|706|EF_ITEM317|Client Error (mobile_ef03.str)|
|707|EF_ITEM318|Client Crash :P|
|708|EF_STORM_MIN|Storm Gust (same as 89)|
|709|EF_POK_JAP|A Firework that split in 4 mini fireworks|
|710|EF_MAP_GREENLIGHT|A Sphere like Effect 701 but Green, and a bit more larger|
|711|EF_MAP_MAGICWALL|A big violet wall|
|712|EF_MAP_GREENLIGHT2|A Little Flame Sphere|
|713|EF_YELLOWFLY1|A lot of Very Small and Yellow Sphere|
|714|EF_YELLOWFLY2|(Nothing)|
|715|EF_BOTTOM_BLUE|Little blue Basilica|
|716|EF_BOTTOM_BLUE2|Same as 715|
|717|EF_WEWISH|Christmas Carol (copy of Angelus)|
|718|EF_FIREPILLARON2|Judex (Visual Effect)|
|719|EF_FORESTLIGHT5|Renovatio (light beam)|
|720|EF_SOULBREAKER3|Yellow version of Soul Breaker|
|721|EF_ADO_STR|Adoramus (lightning bolt)|
|722|EF_IGN_STR|Ignition Break (big explosion)|
|723|EF_CHIMTO2|Hundred Spear (sound effect)|
|724|EF_WINDCUTTER|Green version of Detecting|
|725|EF_DETECT2|Oratorio (like Detecting)|
|726|EF_FROSTMYSTY|Frost Misty (blue vapor and bubbles)|
|727|EF_CRIMSON_STR|Crimson Rock|
|728|EF_HELL_STR|Small fire (part of Hell Inferno)|
|729|EF_SPR_MASH|Marsh of Abyss (like Close Confine)|
|730|EF_SPR_SOULE|Small, cartoony explosion (part of Soul Expansion)|
|731|EF_DHOWL_STR|Dragon Howling (blinking, expanding circle)|
|732|EF_EARTHWALL|Spike from the ground|
|733|EF_SOULBREAKER4|Fluffy Ball flying by|
|734|EF_CHAINL_STR|Chain Lightning|
|735|EF_CHOOKGI_FIRE|(Nothing)|
|736|EF_CHOOKGI_WIND|(Nothing)|
|737|EF_CHOOKGI_WATER|(Nothing)|
|738|EF_CHOOKGI_GROUND|(Nothing)|
|739|EF_MAGENTA_TRAP|Old Magenta Trap|
|740|EF_COBALT_TRAP|Old Cobald Trap|
|741|EF_MAIZE_TRAP|Old Maize Trap|
|742|EF_VERDURE_TRAP|Old Verdure Trap|
|743|EF_NORMAL_TRAP|White Ranger Trap|
|744|EF_CLOAKING2|Camouflage|
|745|EF_AIMED_STR|Aimed Bolt (crosshairs)|
|746|EF_ARROWSTORM_STR|Arrow Storm|
|747|EF_LAULAMUS_STR|Falling white feathers|
|748|EF_LAUAGNUS_STR|Falling blue feathers|
|749|EF_MILSHIELD_STR|Millennium Shield|
|750|EF_CONCENTRATION2|Detonator (blue sparkles)|
|751|EF_FIREBALL2|Releasing summoned warlock spheres|
|752|EF_BUNSINJYUTSU2|Like Energy Coat, but not as dark|
|753|EF_CLEARTIME|Clearance|
|754|EF_GLASSWALL3|Green warp portal (root of Epiclesis)|
|755|EF_ORATIO|Oratio (spinning blue symbol)|
|756|EF_POTION_BERSERK2|Enchant Blade (like Berserk Potion)|
|757|EF_CIRCLEPOWER|Third Class Aura (Middle)|
|758|EF_ROLLING1|Rolling Cutter - Spin Count 1|
|759|EF_ROLLING2|Rolling Cutter - Spin Count 2|
|760|EF_ROLLING3|Rolling Cutter - Spin Count 3|
|761|EF_ROLLING4|Rolling Cutter - Spin Count 4|
|762|EF_ROLLING5|Rolling Cutter - Spin Count 5|
|763|EF_ROLLING6|Rolling Cutter - Spin Count 6|
|764|EF_ROLLING7|Rolling Cutter - Spin Count 7|
|765|EF_ROLLING8|Rolling Cutter - Spin Count 8|
|766|EF_ROLLING9|Rolling Cutter - Spin Count 9|
|767|EF_ROLLING10|Rolling Cutter - Spin Count 10|
|768|EF_PURPLEBODY|Blinking|
|769|EF_STIN6|Cross Ripper Slasher (flying knives)|
|770|EF_RG_COIN4|Strip sound|
|771|EF_POISONWAV|Poison sound|
|772|EF_POISONSMOKE|Poison particles|
|773|EF_GUMGANG4|Expanding purple aura (part of Phantom Menace)|
|774|EF_SHIELDBOOMERANG4|Axe Boomerang|
|775|EF_CASTSPIN2|Spinning character sprite|
|776|EF_VULCANWAV|Like Desperado sound effect|
|777|EF_AGIUP2|Faded light from the ground [S]|
|778|EF_DETECT3|Expanding white aura (like Clearance)|
|779|EF_AGIUP3|Faded light from the ground [S]|
|780|EF_DETECT4|Expanding red aura (from Infrared Scan)|
|781|EF_ELECTRIC3|Magnetic Field (purple chains)|
|782|EF_GUARD4|All-around shield [S]|
|783|EF_BOTTOM_BARRIER|Yellow shaft of light|
|784|EF_BOTTOM_STEALTH|White shaft of light|
|785|EF_REPAIRTIME|Upward flying wrenches|
|786|EF_NC_ANAL|Symbol with bleeping sound [S]|
|787|EF_FIRETHROW|Flare Launcher (line of fire)|
|788|EF_VENOMIMPRESS|Venom Impress (green skull)|
|789|EF_FROSTMISTY|Freezing Status Effect (two ancillas)|
|790|EF_BURNING|Burning Status Effect (flame symbol)|
|791|EF_COLDTHROW|Two ice shots|
|792|EF_MAKEHALLU|Upward streaming white particles|
|793|EF_HALLUTIME|Same, but more brief|
|794|EF_INFRAREDSCAN|Infrared Scan (red lasers)|
|795|EF_CRASHAXE|Power Swing (axe crash)|
|796|EF_GTHUNDER|Spinning blue triangles|
|797|EF_STONERING|Stapo|
|798|EF_INTIMIDATE2|Red triangles (like Intimidate)|
|799|EF_STASIS|Stasis (expanding blue mist) [S]|
|800|EF_REDLINE|Hell Inferno (red lights)|
|801|EF_FROSTDIVER3|Jack Frost unit (ice spikes)|
|802|EF_BOTTOM_BASILICA2|White Imprison|
|803|EF_RECOGNIZED|Recognized Spell|
|804|EF_TETRA|Tetra Vortex [S]|
|805|EF_TETRACASTING|Tetra Vortex cast animation (blinking colors)|
|806|EF_FIREBALL3|Flying by as fast as a rocket|
|807|EF_INTIMIDATE3|Kidnapping sound|
|808|EF_RECOGNIZED2|Like Recognized Spell, but one symbol|
|809|EF_CLOAKING3|Shadowy filter [S]|
|810|EF_INTIMIDATE4|Damp thud sound [S]|
|811|EF_STRETCH|Body Painting|
|812|EF_BLACKBODY|Black expanding aura|
|813|EF_ENERVATION|Masquerade - Enervation|
|814|EF_ENERVATION2|Masquerade - Groomy|
|815|EF_ENERVATION3|Masquerade - Ignorance|
|816|EF_ENERVATION4|Masquerade - Laziness|
|817|EF_ENERVATION5|Masquerade - Unlucky|
|818|EF_ENERVATION6|Masquerade - Weakness|
|819|EF_LINELINK4|(Nothing)|
|820|EF_RG_COIN5|Strip Accessory|
|821|EF_WATERFALL_ANI|Waterfall|
|822|EF_BOTTOM_MANHOLE|Dimension Door (spinning blue aura)|
|823|EF_MANHOLE|In-the-manhole effect|
|824|EF_MAKEFEINT|Some filter|
|825|EF_FORESTLIGHT6|Dimension Door (aura + blue light)|
|826|EF_DARKCASTING2|Expanding black casting anim.|
|827|EF_BOTTOM_ANI|Chaos Panic (spinning brown aura)|
|828|EF_BOTTOM_MAELSTROM|Maelstrom (spinning pink aura)|
|829|EF_BOTTOM_BLOODYLUST|Bloody Lust (spinning red aura)|
|830|EF_BEGINSPELL_N1|Blue aura (Arch Bishop cast animation)|
|831|EF_BEGINSPELL_N2|Blue cone [S]|
|832|EF_HEAL_N|Sonic Wave|
|833|EF_CHOOKGI_N|(Nothing)|
|834|EF_JOBLVUP50_2|Light shooting away circlish|
|835|EF_CHEMICAL2DASH2|Fastness yellow-reddish|
|836|EF_CHEMICAL2DASH3|Fastness yellow-pinkish|
|837|EF_ROLLINGCAST|Casting [S]|
|838|EF_WATER_BELOW|Watery aura|
|839|EF_WATER_FADE|[Client Error]|
|840|EF_BEGINSPELL_N3|Red cone|
|841|EF_BEGINSPELL_N4|Green cone|
|842|EF_BEGINSPELL_N5|Yellow cone|
|843|EF_BEGINSPELL_N6|White cone|
|844|EF_BEGINSPELL_N7|Purple cone|
|845|EF_BEGINSPELL_N8|light-bluish turquoise cone|
|846|EF_WATER_SMOKE|(Nothing)|
|847|EF_DANCE1|Gloomy Day (white/red light rays)|
|848|EF_DANCE2|Gloomy Day (white/blue light rays)|
|849|EF_LINKPARTICLE|(Nothing)|
|850|EF_SOULLIGHT2|(Nothing)|
|851|EF_SPR_PARTICLE|Green mushy-foggy stuff (dull)|
|852|EF_SPR_PARTICLE2|Green mushy-foggy stuff (bright)|
|853|EF_SPR_PLANT|Bright green flower area|
|854|EF_CHEMICAL_V|Blue beam of light with notes|
|855|EF_SHOOTPARTICLE|(Nothing)|
|856|EF_BOT_REVERB|Reverberation (red eighth notes)|
|857|EF_RAIN_PARTICLE|Severe Rainstorm (falling red and blue beams)|
|858|EF_CHEMICAL_V2|Deep Sleep Lullaby (two red beams and music notes)|
|859|EF_SECRA|Holograph of text (blue)|
|860|EF_BOT_REVERB2|Distorted note (blue)|
|861|EF_CIRCLEPOWER2|Green aura (from Circle of Life's Melody)|
|862|EF_SECRA2|Randomize Spell (holograph of text)|
|863|EF_CHEMICAL_V3|Dominion Impulse (two spears of light)|
|864|EF_ENERVATION7|Gloomy Day (colorful lines)|
|865|EF_CIRCLEPOWER3|Blue aura (from Song of Mana)|
|866|EF_SPR_PLANT2|Dance with a Warg (Wargs)|
|867|EF_CIRCLEPOWER4|Yellow aura (from Dance with a Warg)|
|868|EF_SPR_PLANT3|Song of Mana (Violies)|
|869|EF_RG_COIN6|Strip sound [S]|
|870|EF_SPR_PLANT4|Ghostly Succubuses of fire|
|871|EF_CIRCLEPOWER5|Red aura (from Lerad's Dew)|
|872|EF_SPR_PLANT5|Lerad's Dew (Minerals)|
|873|EF_CIRCLEPOWER6|Stargate-wormhole stuff (bright purple)|
|874|EF_SPR_PLANT6|Melody of Sink (Ktullanuxes)|
|875|EF_CIRCLEPOWER7|Stargate-wormhole stuff (bright turquoise)|
|876|EF_SPR_PLANT7|Warcry of Beyond (Garms)|
|877|EF_CIRCLEPOWER8|Stargate-wormhole stuff (white)|
|878|EF_SPR_PLANT8|Unlimited Humming Voice (Miyabi Ningyos)|
|879|EF_HEARTASURA|Siren's Voice (heart-like)|
|880|EF_BEGINSPELL_150|Bluish castish cone|
|881|EF_LEVEL99_150|Blue aura|
|882|EF_PRIMECHARGE|Whirl of fireflies (red)|
|883|EF_GLASSWALL4|Epiclesis (transparent green tree)|
|884|EF_GRADIUS_LASER|Green beam|
|885|EF_BASH3D6|Blue light beams|
|886|EF_GUMGANG5|Blue castish cone|
|887|EF_HITLINE8|Wavy sparks|
|888|EF_ELECTRIC4|Earth Shaker (same as 432)|
|889|EF_TEIHIT1T|Fast light beams|
|890|EF_SPINMOVE|Rotation|
|891|EF_FIREBALL4|Magic shots [S]|
|892|EF_TRIPLEATTACK4|Fastness with hitting sound[S]|
|893|EF_CHEMICAL3S|Blue-white light passing by|
|894|EF_GROUNDSHAKE|(Nothing)|
|895|EF_DQ9_CHARGE|Big wheel of flat light beams|
|896|EF_DQ9_CHARGE2|Still sun shaped lightning aura|
|897|EF_DQ9_CHARGE3|Animated sun shaped lightning aura|
|898|EF_DQ9_CHARGE4|Animated, curvy sun shaped lightning aura|
|899|EF_BLUELINE|White/red light shots from below|
|900|EF_SELFSCROLL|Animated, slow curvy sun shaped lightning aura|
|901|EF_SPR_LIGHTPRINT|Explosion|
|902|EF_PNG_TEST|Floating bedtable texture|
|903|EF_BEGINSPELL_YB|	Castish flamey cone|
|904|EF_CHEMICAL2DASH4|	Yellow/pink lights passing by|
|905|EF_GROUNDSHAKE2|Expanding circle|
|906|EF_PRESSURE2|Shield Press (falling shield)|
|907|EF_RG_COIN7|Chainy, metalish sound [S]|
|908|EF_PRIMECHARGE2|Prestige (sphere of yellow particles)|
|909|EF_PRIMECHARGE3|Banding (sphere of red particles)|
|910|EF_PRIMECHARGE4|Inspiration (sphere of blue particles)|
|911|EF_GREENCASTING|Green castish animation [S]|
|912|EF_WALLOFTHORN|Wall of Thorns unit (green fog cloud)|
|913|EF_FIREBALL5|Magic projectiles|
|914|EF_THROWITEM11|(Nothing)|
|915|EF_SPR_PLANT9|Crazy Weed|
|916|EF_DEMONICFIRE|Demonic Fire|
|917|EF_DEMONICFIRE2|More angry, demonic flames|
|918|EF_DEMONICFIRE3|Fire Insignia (demonic flames)|
|919|EF_HELLSPLANT|Hell's Plant (green snapping plant)|
|920|EF_FIREWALL2|Fire Walk unit|
|921|EF_VACUUM|Vacuum Extreme (whirlwind)|
|922|EF_SPR_PLANT10|Psychic Wave|
|923|EF_SPR_LIGHTPRINT2|Poison Buster|
|924|EF_POISONSMOKE2|Poisoning animation|
|925|EF_MAKEHALLU2|Some filter|
|926|EF_SHOCKWAVE2|Electric Walk unit|
|927|EF_SPR_PLANT11|Earth Grave (speary roots)|
|928|EF_COLDTHROW2|Ice cloud projectiles|
|929|EF_DEMONICFIRE4|Warmer (field of flames)|
|930|EF_PRESSURE3|Varetyr Spear (falling spear)|
|931|EF_LINKPARTICLE2|	(Nothing)|
|932|EF_SOULLIGHT3|Firefly|
|933|EF_CHAREFFECT|[Client Crash]|
|934|EF_GUMGANG6|White, castishly expanding cone|
|935|EF_FIREBALL6|Green magic projectile|
|936|EF_GUMGANG7|Red, castishly expanding cone|
|937|EF_GUMGANG8|Yellow, castishly expanding cone|
|938|EF_GUMGANG9|Dark-red, castishly expanding cone|
|939|EF_BOTTOM_DE2|Blue, conish aura|
|940|EF_COLDSTATUS|Snow flake|
|941|EF_SPR_LIGHTPRINT3|Explosion of red, demonic fire|
|942|EF_WATERBALL3|Expanding, white dome|
|943|EF_HEAL_N2|Green, fluffy projectile|
|944|EF_RAIN_PARTICLE2|Falling gems|
|945|EF_CLOUD9|(Nothing)|
|946|EF_YELLOWFLY3|Floating lights|
|947|EF_EL_GUST|Blue lightning sphere|
|948|EF_EL_BLAST|Two blue lightning spheres|
|949|EF_EL_AQUAPLAY|Flat, spinning diamond|
|950|EF_EL_UPHEAVAL|Circling, planetlike spheres|
|951|EF_EL_WILD_STORM|	Three lightning spheres|
|952|EF_EL_CHILLY_AIR|	Flat, spinning gem and two lightning spheres|
|953|EF_EL_CURSED_SOIL|	Spinning, planetlike spheres|
|954|EF_EL_COOLER|Two lightblue glowing spheres|
|955|EF_EL_TROPIC|Three spinning flame spheres|
|956|EF_EL_PYROTECHNIC|	Flame|
|957|EF_EL_PETROLOGY|Spinning planetlike sphere|
|958|EF_EL_HEATER|Two flames|
|959|EF_POISON_MIST|Purple flame|
|960|EF_ERASER_CUTTER|	Small yellow explosion|
|961|EF_SILENT_BREEZE|	Cartoony whirlwind|
|962|EF_MAGMA_FLOW|Rising fire|
|963|EF_GRAYBODY|Dark filter (like Stone Curse)|
|964|EF_LAVA_SLIDE|Same as 920|
|965|EF_SONIC_CLAW|Small white explosion|
|966|EF_TINDER_BREAKER|	Bone crack|
|967|EF_MIDNIGHT_FRENZY|	Another little explosion|
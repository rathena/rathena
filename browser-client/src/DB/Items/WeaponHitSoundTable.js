/**
 * DB/Items/WeaponHitSoundTable.js
 *
 * Weapon sound table resources
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WeaponType from './WeaponType.js';

const WeaponSound = {};

WeaponSound[WeaponType.NONE] = ['_hit_fist1.wav', '_hit_fist2.wav', '_hit_fist3.wav', '_hit_fist4.wav'];
WeaponSound[WeaponType.SHORTSWORD] = ['_hit_sword.wav']; // at some point this was _hit_dagger.wav but now it is _hit_short_sword.wav
WeaponSound[WeaponType.SWORD] = ['_hit_sword.wav'];
WeaponSound[WeaponType.TWOHANDSWORD] = ['_hit_sword.wav'];
WeaponSound[WeaponType.SPEAR] = ['_hit_spear.wav'];
WeaponSound[WeaponType.TWOHANDSPEAR] = ['_hit_spear.wav'];
WeaponSound[WeaponType.AXE] = ['_hit_axe.wav'];
WeaponSound[WeaponType.TWOHANDAXE] = ['_hit_axe.wav'];
WeaponSound[WeaponType.MACE] = ['_hit_mace.wav'];
WeaponSound[WeaponType.TWOHANDMACE] = ['_hit_mace.wav'];
WeaponSound[WeaponType.ROD] = ['_hit_rod.wav'];
WeaponSound[WeaponType.BOW] = ['_hit_arrow.wav'];
WeaponSound[WeaponType.KNUKLE] = ['_HIT_FIST2.wav'];
WeaponSound[WeaponType.INSTRUMENT] = ['_hit_mace.wav'];
WeaponSound[WeaponType.WHIP] = ['_hit_mace.wav'];
WeaponSound[WeaponType.BOOK] = ['_hit_mace.wav'];
WeaponSound[WeaponType.KATAR] = ['_hit_mace.wav'];
WeaponSound[WeaponType.GUN_HANDGUN] = ['_hit_\xb1\xc7\xc3\xd1.wav']; //_hit_±ÇÃÑ.wav
WeaponSound[WeaponType.GUN_RIFLE] = ['_hit_\xb6\xf3\xc0\xcc\xc7\xc3.wav']; //_hit_¶óÀÌÇÃ.wav
WeaponSound[WeaponType.GUN_GATLING] = ['_hit_\xb0\xb3\xc6\xb2\xb8\xb5\xc7\xd1\xb9\xdf.wav']; //_hit_°³Æ²¸µÇÑ¹ß.wav
WeaponSound[WeaponType.GUN_SHOTGUN] = ['_hit_\xbc\xa6\xb0\xc7.wav']; //_hit_¼¦°Ç.wav
WeaponSound[WeaponType.GUN_GRANADE] = ['_hit_\xb1\xd7\xb7\xb9\xb3\xd7\xc0\xcc\xb5\xe5\xb7\xb1\xc3\xc4.wav']; //_hit_±×·¹³×ÀÌµå·±ÃÄ.wav
WeaponSound[WeaponType.SYURIKEN] = ['_hit_mace.wav'];
WeaponSound[WeaponType.TWOHANDROD] = ['_hit_rod.wav'];
WeaponSound[WeaponType.LAST] = ['_hit_fist4.wav'];
WeaponSound[WeaponType.SHORTSWORD_SHORTSWORD] = ['_hit_sword.wav'];
WeaponSound[WeaponType.SWORD_SWORD] = ['_hit_sword.wav'];
WeaponSound[WeaponType.AXE_AXE] = ['_hit_axe.wav'];
WeaponSound[WeaponType.SHORTSWORD_SWORD] = ['_hit_sword.wav'];
WeaponSound[WeaponType.SHORTSWORD_AXE] = ['_hit_sword.wav'];
WeaponSound[WeaponType.SWORD_AXE] = ['_hit_sword.wav'];
WeaponSound[WeaponType.MAX] = ['_hit_mace.wav'];

export default WeaponSound;

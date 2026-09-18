/**
 * DB/Items/WeaponSoundTable.js
 *
 * Weapon sound table resources
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WeaponType from './WeaponType.js';

const WeaponSound = {};

WeaponSound[WeaponType.NONE] = ['attack_fist.wav'];
WeaponSound[WeaponType.SHORTSWORD] = ['attack_short_sword.wav', 'attack_short_sword_.wav'];
WeaponSound[WeaponType.SWORD] = ['attack_sword.wav'];
WeaponSound[WeaponType.TWOHANDSWORD] = ['attack_twohand_sword.wav'];
WeaponSound[WeaponType.SPEAR] = ['attack_spear.wav'];
WeaponSound[WeaponType.TWOHANDSPEAR] = ['attack_spear.wav'];
WeaponSound[WeaponType.AXE] = ['attack_axe.wav'];
WeaponSound[WeaponType.TWOHANDAXE] = ['attack_axe.wav'];
WeaponSound[WeaponType.MACE] = ['attack_mace.wav'];
WeaponSound[WeaponType.TWOHANDMACE] = ['attack_mace.wav'];
WeaponSound[WeaponType.ROD] = ['attack_rod.wav'];
WeaponSound[WeaponType.BOW] = ['attack_bow1.wav', 'attack_bow2.wav'];
WeaponSound[WeaponType.KNUKLE] = ['attack_fist.wav'];
WeaponSound[WeaponType.INSTRUMENT] = ['attack_mace.wav'];
WeaponSound[WeaponType.WHIP] = ['attack_whip.wav'];
WeaponSound[WeaponType.BOOK] = ['attack_book.wav'];
WeaponSound[WeaponType.KATAR] = ['attack_katar.wav'];
WeaponSound[WeaponType.GUN_HANDGUN] = [];
WeaponSound[WeaponType.GUN_RIFLE] = [];
WeaponSound[WeaponType.GUN_GATLING] = [];
WeaponSound[WeaponType.GUN_SHOTGUN] = [];
WeaponSound[WeaponType.GUN_GRANADE] = [];
WeaponSound[WeaponType.SYURIKEN] = ['attack_sword.wav'];
WeaponSound[WeaponType.TWOHANDROD] = ['attack_rod.wav'];
WeaponSound[WeaponType.LAST] = ['attack_fist.wav'];
WeaponSound[WeaponType.SHORTSWORD_SHORTSWORD] = ['attack_mace.wav'];
WeaponSound[WeaponType.SWORD_SWORD] = ['attack_mace.wav'];
WeaponSound[WeaponType.AXE_AXE] = ['attack_mace.wav'];
WeaponSound[WeaponType.SHORTSWORD_SWORD] = ['attack_mace.wav'];
WeaponSound[WeaponType.SHORTSWORD_AXE] = ['attack_mace.wav'];
WeaponSound[WeaponType.SWORD_AXE] = ['attack_mace.wav'];
WeaponSound[WeaponType.MAX] = ['attack_fist.wav'];

export default WeaponSound;

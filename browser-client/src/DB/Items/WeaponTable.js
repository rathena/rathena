/**
 * DB/Items/WeaponNameTable.js
 *
 * Weapon resource table name
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WeaponType from './WeaponType.js';

const WeaponName = {};

WeaponName[WeaponType.NONE] = '';
WeaponName[WeaponType.SHORTSWORD] = '_\xb4\xdc\xb0\xcb';
WeaponName[WeaponType.SWORD] = '_\xb0\xcb';
WeaponName[WeaponType.TWOHANDSWORD] = '_\xb0\xcb';
WeaponName[WeaponType.SPEAR] = '_\xc3\xa2';
WeaponName[WeaponType.TWOHANDSPEAR] = '_\xc3\xa2';
WeaponName[WeaponType.AXE] = '_\xb5\xb5\xb3\xa2';
WeaponName[WeaponType.TWOHANDAXE] = '_\xb5\xb5\xb3\xa2';
WeaponName[WeaponType.MACE] = '_\xc5\xac\xb7\xb4';
WeaponName[WeaponType.TWOHANDMACE] = '_\xc5\xac\xb7\xb4';
WeaponName[WeaponType.ROD] = '_\xb7\xd4\xb5\xe5';
WeaponName[WeaponType.BOW] = '_\xc8\xb0';
WeaponName[WeaponType.KNUKLE] = '_\xb3\xca\xc5\xac';
WeaponName[WeaponType.INSTRUMENT] = '_\xbe\xc7\xb1\xe2';
WeaponName[WeaponType.WHIP] = '_\xc3\xa4\xc2\xef';
WeaponName[WeaponType.BOOK] = '_\xc3\xa5';
WeaponName[WeaponType.KATAR] = '_\xc4\xab\xc5\xb8\xb8\xa3\x5f\xc4\xab\xc5\xb8\xb8\xa3';
WeaponName[WeaponType.GUN_HANDGUN] = '_\xb1\xc7\xc3\xd1';
WeaponName[WeaponType.GUN_RIFLE] = '_\xb1\xe2\xb0\xfc\xc3\xd1';
WeaponName[WeaponType.GUN_GATLING] = '_\xb1\xe2\xb0\xfc\xc3\xd1';
WeaponName[WeaponType.GUN_SHOTGUN] = '_\xb1\xe2\xb0\xfc\xc3\xd1';
WeaponName[WeaponType.GUN_GRANADE] = '_\xb1\xe2\xb0\xfc\xc3\xd1';
WeaponName[WeaponType.SYURIKEN] = '_\xbc\xf6\xb8\xae\xb0\xcb';
WeaponName[WeaponType.TWOHANDROD] = '_\xb7\xd4\xb5\xe5';
WeaponName[WeaponType.SHORTSWORD_SHORTSWORD] = '_\xb4\xdc\xb0\xcb\x5f\xb4\xdc\xb0\xcb';
WeaponName[WeaponType.SWORD_SWORD] = '_\xb0\xcb\x5f\xb0\xcb';
WeaponName[WeaponType.AXE_AXE] = '_\xb5\xb5\xb3\xa2\x5f\xb5\xb5\xb3\xa2';
WeaponName[WeaponType.SHORTSWORD_SWORD] = '_\xb4\xdc\xb0\xcb\x5f\xb0\xcb';
WeaponName[WeaponType.SHORTSWORD_AXE] = '_\xb4\xdc\xb0\xcb\x5f\xb5\xb5\xb3\xa2';
WeaponName[WeaponType.SWORD_AXE] = '_\xb0\xcb\x5f\xb5\xb5\xb3\xa2';

export default WeaponName;

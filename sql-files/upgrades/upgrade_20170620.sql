-- ----------------------------
-- Table structure for `mail_attachments`
-- ----------------------------

CREATE TABLE IF NOT EXISTS `mail_attachments` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `index` smallint(5) unsigned NOT NULL DEFAULT '0',
  `nameid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `amount` int(11) unsigned NOT NULL DEFAULT '0',
  `refine` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `attribute` tinyint(4) unsigned NOT NULL DEFAULT '0',
  `identify` smallint(6) NOT NULL DEFAULT '0',
  `card0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `card1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `card2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `card3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_id0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_val0` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_parm0` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `option_id1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_val1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_parm1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `option_id2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_val2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_parm2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `option_id3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_val3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_parm3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `option_id4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_val4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `option_parm4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unique_id` bigint(20) unsigned NOT NULL DEFAULT '0',
  `bound` tinyint(1) unsigned NOT NULL DEFAULT '0',
    PRIMARY KEY (`id`,`index`)
) ENGINE=MyISAM;

insert into `mail_attachments`
(`id`,`index`,`nameid`,`amount`,`refine`,`attribute`,`identify`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`unique_id`,`bound`)
select
`id`,'0',`nameid`,`amount`,`refine`,`attribute`,`identify`,`card0`,`card1`,`card2`,`card3`,`option_id0`,`option_val0`,`option_parm0`,`option_id1`,`option_val1`,`option_parm1`,`option_id2`,`option_val2`,`option_parm2`,`option_id3`,`option_val3`,`option_parm3`,`option_id4`,`option_val4`,`option_parm4`,`unique_id`,`bound`
from `mail`
where `nameid` <> 0;

alter table `mail`
	drop column `nameid`,
	drop column `amount`,
	drop column `refine`,
	drop column `attribute`,
	drop column `identify`,
	drop column `card0`,
	drop column `card1`,
	drop column `card2`,
	drop column `card3`,
	drop column `option_id0`,
	drop column `option_val0`,
	drop column `option_parm0`,
	drop column `option_id1`,
	drop column `option_val1`,
	drop column `option_parm1`,
	drop column `option_id2`,
	drop column `option_val2`,
	drop column `option_parm2`,
	drop column `option_id3`,
	drop column `option_val3`,
	drop column `option_parm3`,
	drop column `option_id4`,
	drop column `option_val4`,
	drop column `option_parm4`,
	drop column `unique_id`,
	drop column `bound`;

alter table `mail`
	modify `message` varchar(500) NOT NULL default '';

ALTER TABLE `mail`
	ADD COLUMN `type` smallint(5) NOT NULL default '0';

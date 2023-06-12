/**
	ATTENTION!!!
	Please make sure to create a backup of your live data before running this update.
	This update is rather complicated and although I have tried my best I cannot guarantee that I did not make any mistake. [Lemongrass]
	We would also advise you to check the data in the temporary table after the insert statement and before the update statements.
	You could do so by running
		select * from `tmp_randomoptionfix` where `new_index` <> `old_index`;
	to see where changes happen and options will be moved.
	For executing this update your user needs create table and drop table permissions. Either run the update with another user [admin or root for example] or give your database user these permissions temporarily.
	Do not forget to remove these permissions again, as they are usually not necessary and it is rather dangerous, if someone gets access to your database user.
	Additionally if you have more than one storage table (see conf/inter_server.yml) make sure to also update those tables (see instructions at the bottom of this script).








	Comment the line below to really run the script, this is a safeguard that you confirm you have read and understood what was written above.
**/
cancel;

-- Create a temporary table to store the data for the update
create table `tmp_randomoptionfix` (
	`id` int(11) unsigned NOT NULL,
	`new_index` smallint(5) NOT NULL,
	`old_index` smallint(5) NOT NULL,
	`option_id` smallint(5) NOT NULL,
	`option_val` smallint(5) NOT NULL,
	`option_parm` tinyint(3) NOT NULL,
	PRIMARY KEY ( `id`, `old_index` )
);

-- Create auction data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`auction_id` as `id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `auction`
			where `option_id0` <> 0
		union
			select
				`auction_id` as `id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `auction`
			where `option_id1` <> 0
		union
			select
				`auction_id` as `id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `auction`
			where `option_id2` <> 0
		union
			select
				`auction_id` as `id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `auction`
			where `option_id3` <> 0
		union
			select
				`auction_id` as `id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `auction`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `auction`
left join `tmp_randomoptionfix`
on `auction`.`auction_id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `auction`
left join `tmp_randomoptionfix`
on `auction`.`auction_id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `auction`
left join `tmp_randomoptionfix`
on `auction`.`auction_id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `auction`
left join `tmp_randomoptionfix`
on `auction`.`auction_id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `auction`
left join `tmp_randomoptionfix`
on `auction`.`auction_id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Create cart_inventory data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `cart_inventory`
			where `option_id0` <> 0
		union
			select
				`id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `cart_inventory`
			where `option_id1` <> 0
		union
			select
				`id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `cart_inventory`
			where `option_id2` <> 0
		union
			select
				`id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `cart_inventory`
			where `option_id3` <> 0
		union
			select
				`id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `cart_inventory`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `cart_inventory`
left join `tmp_randomoptionfix`
on `cart_inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `cart_inventory`
left join `tmp_randomoptionfix`
on `cart_inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `cart_inventory`
left join `tmp_randomoptionfix`
on `cart_inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `cart_inventory`
left join `tmp_randomoptionfix`
on `cart_inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `cart_inventory`
left join `tmp_randomoptionfix`
on `cart_inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Create guild_storage data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `guild_storage`
			where `option_id0` <> 0
		union
			select
				`id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `guild_storage`
			where `option_id1` <> 0
		union
			select
				`id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `guild_storage`
			where `option_id2` <> 0
		union
			select
				`id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `guild_storage`
			where `option_id3` <> 0
		union
			select
				`id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `guild_storage`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `guild_storage`
left join `tmp_randomoptionfix`
on `guild_storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `guild_storage`
left join `tmp_randomoptionfix`
on `guild_storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `guild_storage`
left join `tmp_randomoptionfix`
on `guild_storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `guild_storage`
left join `tmp_randomoptionfix`
on `guild_storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `guild_storage`
left join `tmp_randomoptionfix`
on `guild_storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Create inventory data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `inventory`
			where `option_id0` <> 0
		union
			select
				`id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `inventory`
			where `option_id1` <> 0
		union
			select
				`id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `inventory`
			where `option_id2` <> 0
		union
			select
				`id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `inventory`
			where `option_id3` <> 0
		union
			select
				`id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `inventory`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `inventory`
left join `tmp_randomoptionfix`
on `inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `inventory`
left join `tmp_randomoptionfix`
on `inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `inventory`
left join `tmp_randomoptionfix`
on `inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `inventory`
left join `tmp_randomoptionfix`
on `inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `inventory`
left join `tmp_randomoptionfix`
on `inventory`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Create mail_attachments data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `mail_attachments`
			where `option_id0` <> 0
		union
			select
				`id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `mail_attachments`
			where `option_id1` <> 0
		union
			select
				`id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `mail_attachments`
			where `option_id2` <> 0
		union
			select
				`id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `mail_attachments`
			where `option_id3` <> 0
		union
			select
				`id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `mail_attachments`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `mail_attachments`
left join `tmp_randomoptionfix`
on `mail_attachments`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `mail_attachments`
left join `tmp_randomoptionfix`
on `mail_attachments`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `mail_attachments`
left join `tmp_randomoptionfix`
on `mail_attachments`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `mail_attachments`
left join `tmp_randomoptionfix`
on `mail_attachments`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `mail_attachments`
left join `tmp_randomoptionfix`
on `mail_attachments`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Create storage data
insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
select
	`id`,
	row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
	`old_index`,
	`option_id`,
	`option_val`,
	`option_parm`
from (
	select * from (
		select
				`id`,
				0 as `old_index`,
				`option_id0` as `option_id`,
				`option_val0` as `option_val`,
				`option_parm0` as `option_parm`
			from `storage`
			where `option_id0` <> 0
		union
			select
				`id`,
				1 as `old_index`,
				`option_id1` as `option_id`,
				`option_val1` as `option_val`,
				`option_parm1` as `option_parm`
			from `storage`
			where `option_id1` <> 0
		union
			select
				`id`,
				2 as `old_index`,
				`option_id2` as `option_id`,
				`option_val2` as `option_val`,
				`option_parm2` as `option_parm`
			from `storage`
			where `option_id2` <> 0
		union
			select
				`id`,
				3 as `old_index`,
				`option_id3` as `option_id`,
				`option_val3` as `option_val`,
				`option_parm3` as `option_parm`
			from `storage`
			where `option_id3` <> 0
		union
			select
				`id`,
				4 as `old_index`,
				`option_id4` as `option_id`,
				`option_val4` as `option_val`,
				`option_parm4` as `option_parm`
			from `storage`
			where `option_id4` <> 0
	) t2
	order by `id`, `old_index`
) t ;

-- Fix option 0
update `storage`
left join `tmp_randomoptionfix`
on `storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
set
	`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 1
update `storage`
left join `tmp_randomoptionfix`
on `storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
set
	`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 2
update `storage`
left join `tmp_randomoptionfix`
on `storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
set
	`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 3
update `storage`
left join `tmp_randomoptionfix`
on `storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
set
	`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Fix option 4
update `storage`
left join `tmp_randomoptionfix`
on `storage`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
set
	`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
	`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
	`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
;

-- Delete the data to have a clean table for the next run
delete from `tmp_randomoptionfix`;

-- Repeat this for any other storage tables that you have created (for example VIP)
-- To do this copy paste the code below and replace ${other_storage} with the name of your table
/**
	-- Create ${other_storage} data
	insert into `tmp_randomoptionfix` ( `id`, `new_index`, `old_index`, `option_id`, `option_val`, `option_parm` )
	select
		`id`,
		row_number() over( partition by `id` order by `old_index` asc ) - 1 as `new_index`,
		`old_index`,
		`option_id`,
		`option_val`,
		`option_parm`
	from (
		select * from (
			select
					`id`,
					0 as `old_index`,
					`option_id0` as `option_id`,
					`option_val0` as `option_val`,
					`option_parm0` as `option_parm`
				from `${other_storage}`
				where `option_id0` <> 0
			union
				select
					`id`,
					1 as `old_index`,
					`option_id1` as `option_id`,
					`option_val1` as `option_val`,
					`option_parm1` as `option_parm`
				from `${other_storage}`
				where `option_id1` <> 0
			union
				select
					`id`,
					2 as `old_index`,
					`option_id2` as `option_id`,
					`option_val2` as `option_val`,
					`option_parm2` as `option_parm`
				from `${other_storage}`
				where `option_id2` <> 0
			union
				select
					`id`,
					3 as `old_index`,
					`option_id3` as `option_id`,
					`option_val3` as `option_val`,
					`option_parm3` as `option_parm`
				from `${other_storage}`
				where `option_id3` <> 0
			union
				select
					`id`,
					4 as `old_index`,
					`option_id4` as `option_id`,
					`option_val4` as `option_val`,
					`option_parm4` as `option_parm`
				from `${other_storage}`
				where `option_id4` <> 0
		) t2
		order by `id`, `old_index`
	) t ;

	-- Fix option 0
	update `${other_storage}`
	left join `tmp_randomoptionfix`
	on `${other_storage}`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 0
	set
		`option_id0` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
		`option_val0` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
		`option_parm0` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
	;

	-- Fix option 1
	update `${other_storage}`
	left join `tmp_randomoptionfix`
	on `${other_storage}`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 1
	set
		`option_id1` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
		`option_val1` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
		`option_parm1` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
	;

	-- Fix option 2
	update `${other_storage}`
	left join `tmp_randomoptionfix`
	on `${other_storage}`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 2
	set
		`option_id2` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
		`option_val2` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
		`option_parm2` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
	;

	-- Fix option 3
	update `${other_storage}`
	left join `tmp_randomoptionfix`
	on `${other_storage}`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 3
	set
		`option_id3` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
		`option_val3` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
		`option_parm3` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
	;

	-- Fix option 4
	update `${other_storage}`
	left join `tmp_randomoptionfix`
	on `${other_storage}`.`id` = `tmp_randomoptionfix`.`id` and `tmp_randomoptionfix`.`new_index` = 4
	set
		`option_id4` = coalesce( `tmp_randomoptionfix`.`option_id`, 0 ),
		`option_val4` = coalesce( `tmp_randomoptionfix`.`option_val`, 0 ),
		`option_parm4` = coalesce( `tmp_randomoptionfix`.`option_parm`, 0 )
	;

	-- Delete the data to have a clean table for the next run
	delete from `tmp_randomoptionfix`;
**/

drop table `tmp_randomoptionfix`;

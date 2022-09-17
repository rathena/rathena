UPDATE `mail`
SET `send_id`='0'
WHERE `send_id` NOT IN (
	select `char_id`
	from `char`
)
;

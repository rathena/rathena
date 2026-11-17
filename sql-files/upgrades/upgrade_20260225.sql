-- Convert the engine from MyISAM to InnoDB
ALTER TABLE `mail` ENGINE = InnoDB;
ALTER TABLE `mail_attachments` ENGINE = InnoDB;

-- Clean up old undeleted attachments, where the mail is lost
DELETE FROM `mail_attachments`
WHERE
	`id` NOT IN (
		SELECT `id` FROM `mail`
	)
;

-- Add a foreign key between mail and mail attachments
ALTER TABLE `mail_attachments`
ADD
	FOREIGN KEY (`id`)
	REFERENCES `mail`(`id`)
	ON DELETE CASCADE
	ON UPDATE CASCADE
;

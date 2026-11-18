<?php
// Copy this file to discord_item_report.conf.php and fill it in.
// discord_item_report.conf.php is gitignored - never commit the webhook URL.
return [
	// Discord: channel settings -> Integrations -> Webhooks -> New Webhook -> Copy Webhook URL
	'webhook_url' => 'https://discord.com/api/webhooks/XXXXXXXX/YYYYYYYY',

	// Same DB as conf/inter_athena.conf (map_server_* settings)
	'db_host' => '127.0.0.1',
	'db_port' => 3306,
	'db_user' => 'ragnarok',
	'db_pass' => 'ragnarok',
	'db_name' => 'ragnarok',

	// Seconds between checks when run with --loop
	'interval' => 30,
];

<?php
// Transcended RO - Item Checker -> Discord relay
//
// Reads new rows from `itemcheck_report` (written by the Item Checker NPC)
// and posts them to a Discord channel webhook, then marks them as sent.
//
// Usage:
//   php discord_item_report.php          run once (for Task Scheduler / cron)
//   php discord_item_report.php --loop   keep running, check every N seconds
//
// Config: copy discord_item_report.conf.example.php to
//         discord_item_report.conf.php and fill it in (that file is gitignored).

$confFile = __DIR__ . '/discord_item_report.conf.php';
if (!is_file($confFile)) {
	fwrite(STDERR, "Missing $confFile (copy discord_item_report.conf.example.php)\n");
	exit(1);
}
$conf = require $confFile;

function log_msg($msg) {
	echo '[' . date('Y-m-d H:i:s') . "] $msg\n";
}

function db_connect(array $conf) {
	$dsn = "mysql:host={$conf['db_host']};port={$conf['db_port']};dbname={$conf['db_name']};charset=utf8mb4";
	return new PDO($dsn, $conf['db_user'], $conf['db_pass'], [
		PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
		PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
	]);
}

// Returns [httpCode, retryAfterSeconds]
function discord_post($webhook, array $payload) {
	$ctx = stream_context_create(['http' => [
		'method' => 'POST',
		'header' => "Content-Type: application/json\r\nUser-Agent: TranscendedRO-ItemReport/1.0\r\n",
		'content' => json_encode($payload, JSON_UNESCAPED_UNICODE),
		'ignore_errors' => true,
		'timeout' => 15,
	]]);
	$body = @file_get_contents($webhook, false, $ctx);
	$code = 0;
	if (isset($http_response_header[0]) && preg_match('#\s(\d{3})\s#', $http_response_header[0], $m))
		$code = (int)$m[1];
	$retry = 0;
	if ($code == 429) {
		$j = json_decode((string)$body, true);
		$retry = isset($j['retry_after']) ? (float)$j['retry_after'] : 5;
	}
	return [$code, $retry];
}

function run_once(array $conf, $verbose) {
	$db = db_connect($conf);
	$rows = $db->query("SELECT * FROM `itemcheck_report` WHERE `discord_sent` = 0 ORDER BY `id` LIMIT 20")->fetchAll();
	$mark = $db->prepare("UPDATE `itemcheck_report` SET `discord_sent` = 1 WHERE `id` = ?");
	if ($verbose && !$rows)
		log_msg('Connected to DB - no new reports.');

	foreach ($rows as $r) {
		$payload = [
			'username' => 'Item Checker',
			'embeds' => [[
				'title' => 'Item needs ItemInfo',
				'color' => 0x00AA00,
				'fields' => [
					['name' => 'ItemID', 'value' => '`' . $r['nameid'] . '`', 'inline' => true],
					['name' => 'Name', 'value' => $r['item_name'] !== '' ? $r['item_name'] : '-', 'inline' => true],
					['name' => 'Reported by', 'value' => $r['char_name'] . ' (AID ' . $r['account_id'] . ')', 'inline' => false],
				],
				'footer' => ['text' => 'Report #' . $r['id'] . ' - ' . $r['report_time']],
			]],
		];

		list($code, $retry) = discord_post($conf['webhook_url'], $payload);
		if ($code >= 200 && $code < 300) {
			$mark->execute([$r['id']]);
			log_msg("Sent report #{$r['id']} (ItemID {$r['nameid']})");
		} elseif ($code == 429) {
			log_msg("Rate limited, waiting {$retry}s");
			usleep((int)($retry * 1000000) + 200000);
			return; // try the rest next round
		} else {
			log_msg("Discord error HTTP $code for report #{$r['id']} - will retry");
			return;
		}
		usleep(500000); // stay well under webhook rate limits
	}
}

$loop = in_array('--loop', $argv ?? [], true);
$interval = max(5, (int)$conf['interval']);
if ($loop)
	log_msg("Relay started - checking every {$interval}s. Close this window to stop.");
$first = true;
do {
	try {
		// Say "no new reports" on the first check (and in --once mode) only,
		// so the loop window doesn't fill up with idle lines.
		run_once($conf, $first || !$loop);
	} catch (Throwable $e) {
		log_msg('Error: ' . $e->getMessage());
	}
	$first = false;
	if ($loop)
		sleep($interval);
} while ($loop);

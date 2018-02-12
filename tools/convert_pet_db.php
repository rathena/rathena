#!/usr/bin/php
<?php
	// pet_db.txt to pet_db.yml converter
	// author : Secret <secret@rathena.org>
	// version : 2017/10/17
?>
<?php
	define("EOL", PHP_EOL);
	fwrite(STDERR, "----------------------------------------------------------------------"  . EOL);
	fwrite(STDERR, "                      ___   __  __                                    " . EOL);
	fwrite(STDERR, "                _____/   | / /_/ /_  ___  ____  ____ _                " . EOL);
	fwrite(STDERR, "               / ___/ /| |/ __/ __ \\/ _ \\/ __ \\/ __ `/                " . EOL);
	fwrite(STDERR, "              / /  / ___ / /_/ / / /  __/ / / / /_/ /                 " . EOL);
	fwrite(STDERR, "             /_/  /_/  |_\\__/_/ /_/\\___/_/ /_/\\__,_/                  " . EOL);
	fwrite(STDERR, "                                                                      " . EOL);
	fwrite(STDERR, "              pet_db.txt to pet_db.yml converter" . EOL);
	fwrite(STDERR, "----------------------------------------------------------------------"  . EOL);
	
	if( function_exists('yaml_emit') === FALSE )
		die("Please install the php_yaml extension first!");

	if( @$_SERVER["argc"] < 2 )
	{
		fwrite(STDERR, "Usage: {$_SERVER["argv"][0]} [file]".EOL);
		fwrite(STDOUT, "	Redirect the standard output to a file to save the output." . EOL);
		exit();
	}
	
	$input = @$_SERVER["argv"][1];
	$data = file($input);
	if( $data === FALSE )
		die("Invalid input file '".$input."'!");
	
	fwrite(STDERR, "Converting {$input}...".EOL);
	
	$pets = array();
	$failed = 0;
	foreach( $data as $line )
	{
		if(substr($line, 0, 2) === "//" || strlen(trim($line)) == 0)
			continue;
		else if(preg_match('/(\d+),([a-zA-Z0-9_]+),([a-zA-Z0-9_ .]+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),\{(.*)\},\{(.*)\}/', $line, $regs)) {
			$pet = array();
			$pet['MobID'] = (int)$regs[1];
			$pet['Name'] = $regs[2];
			$pet['JName'] = $regs[3];
			if($regs[4])
				$pet['TameWithID'] = (int)$regs[4];
			$pet['EggItemID'] = (int)$regs[5];
			if($regs[6])
				$pet['EquipItemID'] = (int)$regs[6];
			$pet['FoodItemID'] = (int)$regs[7];
			$pet['Fullness'] = (int)$regs[8];
			$pet['HungryDelay'] = (int)$regs[9];
			$pet['Intimacy'] = array();
			$pet['Intimacy']['Start'] = (int)$regs[12];
			$pet['Intimacy']['Fed'] = (int)$regs[10];
			$pet['Intimacy']['Overfed'] = ((int)$regs[11]) * -1;
			$pet['Intimacy']['Hungry'] = -5; // Default rAthena value
			$pet['Intimacy']['OwnerDie'] = ((int)$regs[13]) * -1;
			$pet['CaptureRate'] = (int)$regs[14];
			$pet['Speed'] = (int)$regs[15];
			$pet['SpecialPerformance'] = (bool)$regs[16];
			$pet['TalkConvertClass'] = (int)$regs[17];
			$pet['AttackRate'] = (int)$regs[18];
			$pet['RetaliateRate'] = (int)$regs[19];
			$pet['ChangeTargetRate'] = (int)$regs[20];
			if(strlen(trim($regs[22])) == 0)
				$pet['Script'] = '{}';
			else
				$pet['Script'] = '{ .@i = getpetinfo(PETINFO_INTIMATE); if(.@i >= PET_INTIMATE_LOYAL){ ' . trim($regs[22]) . ' } }';
			if(strlen(trim($regs[21])) == 0)
				$pet['SupportScript'] = '{}';
			else
				$pet['SupportScript'] = '{ ' . trim($regs[21]) . ' }';
			$pets[] = $pet;
		} else {
			fwrite(STDERR, "Invalid data " . $line . EOL);
			$failed++;
		}
	}
	
	fwrite(STDOUT, yaml_emit($pets) . EOL);
	fwrite(STDERR, "Done converting " . sizeof($pets) . " out of " . (sizeof($pets)+$failed) ." entries." . EOL);
?>

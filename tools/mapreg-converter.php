<?php
	// mapreg.txt -> sql import file converter
	// author : theultramage / Yommy
	// version: 16. august 2008
?>
<?php
	fwrite(STDERR, "mapreg txt->sql converter".PHP_EOL);
	fwrite(STDERR, "-------------------------".PHP_EOL);
	if( @$_SERVER["argc"] < 2 )
	{
		fwrite(STDERR, "Usage: {$_SERVER["argv"][0]} [file]".PHP_EOL);
		exit();
	}

	$input = @$_SERVER["argv"][1];
	$data = file($input);
	if( $data === FALSE )
		die("Invalid input file '".$input."'!");

	if( function_exists("mysql_escape_string") === FALSE )
		die("Please enable the php_mysql extension first!");

	fwrite(STDERR, "Converting {$input}...".PHP_EOL);
	define("EOL", PHP_EOL);

	foreach( $data as $line )
	{
		if( preg_match('/(.*),(\d+)\t(.*)/m', $line, $regs) )
			fwrite(STDOUT, "INSERT INTO `mapreg` (`varname`,`index`,`value`) VALUES ('".mysql_escape_string($regs[1])."',".mysql_escape_string($regs[2]).",'".mysql_escape_string(rtrim($regs[3]))."');".EOL);
		else
		if( preg_match('/(.*)\t(.*)/m', $line, $regs) )
			fprintf(STDOUT, "INSERT INTO `mapreg` (`varname`,`index`,`value`) VALUES ('".mysql_escape_string($regs[1])."',0,'".mysql_escape_string(rtrim($regs[2]))."');".EOL);
		else
			fprintf(STDERR, "Invalid data: ".$line.PHP_EOL);
	}

	fprintf(STDERR, "done.".PHP_EOL);
?>
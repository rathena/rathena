<?php
	// Visual Studio 9 to Visual Studio 8 project file converter
	// author : theultramage
	// version: 16. august 2008
?>
<?php
	fwrite(STDERR, "VS9 to VS8 project file converter".PHP_EOL);
	fwrite(STDERR, "---------------------------------".PHP_EOL);
	if( @$_SERVER["argc"] < 2 )
	{
		fwrite(STDERR, "Usage: {$_SERVER["argv"][0]} file.vcproj".PHP_EOL);
		exit();
	}

	$input = @$_SERVER["argv"][1];
	$data = file($input);
	if( $data === FALSE )
		die("invalid input file '".$input."'");

	fwrite(STDERR, "Converting {$input}...".PHP_EOL);
	
	$eol = ( strstr($data[0], "\r\n") !== FALSE ) ? "\r\n" : "\n";
	define("EOL", $eol);

	foreach( $data as $line )
	{
		if( strstr($line,'Version="9,00"') !== FALSE )
			fwrite(STDOUT, "\t".'Version="8,00"'.EOL);
		else
		if( strstr($line,'Version="9.00"') !== FALSE )
			fwrite(STDOUT, "\t".'Version="8.00"'.EOL);
		else
		if( strstr($line,'TargetFrameworkVersion') !== FALSE )
			;
		else
		if( strstr($line,'RandomizedBaseAddress') !== FALSE )
			;
		else
		if( strstr($line,'DataExecutionPrevention') !== FALSE )
			;
		else // default
			fwrite(STDOUT, $line);
	}

	fwrite(STDERR, "done.".PHP_EOL);
?>

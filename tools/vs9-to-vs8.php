<?php
	// Visual Studio 9 to Visual Studio 8 project file converter
	// author : theultramage
	// version: 15. august 2008
?>
<?php
	fprintf(STDERR, "VS9 to VS8 project file converter"."\n");
	fprintf(STDERR, "---------------------------------"."\n");
	if( @$_SERVER["argc"] < 2 )
	{
		fprintf(STDERR, "Usage: {$_SERVER["argv"][0]} file.vcproj"."\n");
		exit();
	}

	$input = @$_SERVER["argv"][1];
	$data = file($input);
	if( $data === FALSE )
		die("invalid input file '".$input."'");

	fprintf(STDERR, "Converting {$input}...");
	
	foreach( $data as $line )
	{
		if( strstr($line,'Version="9,00"') !== FALSE )
			fprintf(STDOUT, "\t".'Version="8,00"'."\n");
		else
		if( strstr($line,'Version="9.00"') !== FALSE )
			fprintf(STDOUT, "\t".'Version="8.00"'."\n");
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
			fprintf(STDOUT, $line);
	}

	fprintf(STDERR, "done."."\n");
?>

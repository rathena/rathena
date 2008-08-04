<?php
	// Visual Studio 9 to Visual Studio 8 project file converter
	// author : theultramage
	// version: 4. august 2008
?>
<?php
	echo "VS9 to VS8 project file converter"."\n";
	echo "---------------------------------"."\n";
	if( @$_SERVER["argc"] < 2 )
	{
		echo "Usage: {$_SERVER["argv"][0]} file.vcproj"."\n";
		exit();
	}

	$input = @$_SERVER["argv"][1];
	$data = file($input);
	if( $data === FALSE )
		die("invalid input file '".$input."'");

	echo "Converting {$input}...";
	
	foreach( $data as $line )
	{
		if( strstr($line,'Version="9,00"') !== FALSE )
			echo "\t".'Version="8,00"'."\n";
		else
		if( strstr($line,'Version="9.00"') !== FALSE )
			echo "\t".'Version="8.00"'."\n";
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
			echo $line;
	}

	echo "done."."\n";
?>

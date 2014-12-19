package rA_Common;
use strict;
use warnings;
use Exporter;

our @ISA= qw( Exporter );

# these CAN be exported.
our @EXPORT_OK = qw( GetValidateConf GetUserConf ExeQuery GetUserConf 
	GetValidAnwser GetValidateConf RootCo ShowConfig CheckUsedPort CheckAndLoadSQL);
# these are exported by default.
our @EXPORT = @EXPORT_OK; # qw( GetValidateConf GetUserConf ExeQuery GetUserConf GetValidAnwser GetValidateConf RootCo ShowConfig CheckUsedPort CheckAndLoadSQL);

sub GetValidateConf { my($rhConfig) = @_;
	my $rhUserConf;
	while (1) {
		$rhUserConf = GetUserConf($rhConfig);
		print "\n Please Check desired conf \n";
		ShowConfig($rhUserConf);
		print "Would you like to apply those setting ? [y/n] ";
		last if(GetValidAnwser("y|o|n") =~ /y|o/i);
		print "\n Restarting configuration sequence \n";
	}
	return $rhUserConf;
}

#remplis tous un hash
sub GetUserConf { my ($rhDefConf) = @_;
	my %hConf;
	my @sortedkeys = sort keys (%$rhDefConf);
	foreach my $sKey (@sortedkeys){
		my $sVal = $$rhDefConf{$sKey};
		print "$sKey : [$sVal] ";
		my $sAnwser = <>; chop($sAnwser);
		$hConf{"$sKey"} = $sAnwser || $sVal;
	}
	return \%hConf;
}

sub ShowConfig { my ($rhUserConf) = @_;
	my @sortedkeys = sort keys (%$rhUserConf);
	foreach my $sKey (@sortedkeys){
		my $sVal = $$rhUserConf{$sKey};
		if(ref($sVal) eq 'ARRAY') {  print " $sKey => [@$sVal] \n";}
		else { print " $sKey => [$sVal] \n"; }
	}
}

sub GetValidAnwser { my($sOptReg,$sAutoyes) = @_;
	my $sAnwser = "";
	if($sAutoyes) { $sAnwser="y"; print "\n"; }
	else {
	while(!($sAnwser =~ /$sOptReg/i)) {
	    $sAnwser = <>; chop($sAnwser);
	    print "Please enter a valid option : $sOptReg " unless($sAnwser =~ /$sOptReg/i);
	}
    }
    return $sAnwser;
}

sub CheckUsedPort { my($sPort) = @_;
    open PIPE,"netstat -nat |" or die $!;
    my @line = grep { /$sPort/ } <PIPE>;
    return scalar(@line);
}


sub RootCo { my($rhConfig) = @_;
	print "\n Entering RootCo \n";
	my $sDbH = 0;
	my $sDsn = $$rhConfig{"Dsn"}; #mysql server dest
	my $sUser = $$rhConfig{SQL_UID}; #verify desired user
	print "My dsn = $sDsn \n";
	if($sUser eq "root"){
		my $sPw = $$rhConfig{SQL_PW};
		$sDbH = DBI->connect($sDsn, "root", $sPw);
		unless($sDbH) { warn "Your root password doesn't seem valid for mysql, your desired-conf is wrong \n"; }
	}
	while($sDbH == 0) { #if can't use user to connect user root
		print "Please inser DB root passeword (this won't be saved in any configuration file, needed to create dbs and user)\n";
		my $sRPw = <>; chop($sRPw);
		$sDbH = DBI->connect($sDsn, "root", $sRPw);
	}

	return $sDbH;
}

sub CheckAndLoadSQL { my ($raFiles,$rhConfig,$sDBn) = @_;
    my $sHost = $$rhConfig{SQL_HOST};
    my $sPw = $$rhConfig{SQL_PW};
    my $sUser = $$rhConfig{SQL_UID};

    foreach(@$raFiles) {
	unless(-f -r $_){
	    print "File : $_ doesn't seem to exist or was unreadable skipped\n";
	    next;
	}
	my $sFileFullPath = Cwd::abs_path($_);
	system("mysql -u $sUser --password=$sPw -h $sHost $sDBn < $sFileFullPath");
    }
}

sub ExeQuery { my $sDbH = shift;
    my @aQuery = @_;
    print "my querys are = [ @aQuery ]\n";
    foreach(@aQuery) {
	unless($sDbH->do($_)){ print "Failed to execute query : $_ =>  $DBI::errstr \n"; }
    }
}

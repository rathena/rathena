#!/usr/bin/perl
# rAthena Configuration Script
# by lighta
# TODO:
# - Don't always override import/file, sed grep ?
use lib '.';
use File::Basename;
use DBI;
use DBD::mysql;
use YAML::XS;
use Cwd;
use Getopt::Long;
use Net::Ping;
use strict;
use rA_Common;
use LWP::Simple;

use constant {
    SERV_UID => "Serv_userid",
    SERV_PW => "Serv_userpass",
    LOGIN_PRIV_IP => "login_priv_ip",
	CHAR_PRIV_IP => "char_priv_ip",
    SERV_WAN_IP => "Serv_wan_ip",
    MAP_PORT => "Map_port",
    CHAR_PORT => "Char_port",
    LOGIN_PORT => "Login_port",
    MD5_ENABLE => "enable_MD5",
    PINCODE_ENABLE => "enable_pincode",
    SQL_HOST => "SQL_host",
    SQL_PORT => "SQL_port",
    SQL_UID => "SQL_userid",
    SQL_PW => "SQL_userpass",
    SQL_MAIN_DB => "SQL_maindb",
    SQL_LOG_DB => ,"SQL_logdb",

    MAP_CONF_FILE => "map_conf.txt",
    CHAR_CONF_FILE => "char_conf.txt",
    LOGIN_CONF_FILE => "login_conf.txt",
    INTER_CONF_FILE => "inter_conf.txt",
    DESD_CONF_FILE => ".tmp-desd_conf.yml",
    MIN_PORT => 2000, #below are usually reserved for system
    MAX_PORT => 65535,
};
# setup default options
my $sDsdFile    = DESD_CONF_FILE;
my $sAutoyes    = 0;
my $sForce      = 0;
my $sClean	= 0;
my $sTarget	= "All";
my $sHelp	= 0;
my $sOS;
GetArgs();
Main();

sub GetArgs {
    GetOptions(
    'f=s' => \$sDsdFile, #give desired conf file
    'auto=i' => \$sAutoyes,   #Force (auto-yes)
    'C=i'	=> \$sClean,	 #Clean (like force but remove before adding)
    'target=s'	=> \$sTarget,	 #Target (which setup to run)
    'Force=i'	=> \$sForce,	 #Force (bypass verification)
    'OS=s'	=> \$sOS, #OS (specify the OS you wish to use)
    'help!' => \$sHelp,
    ) or $sHelp=1; #display help if invalid option
    my $sValidTarget = "All|Conf|DB|Inst|Dump";
    
	
    if( $sHelp ) {
	print "Incorrect option specified. Available options are:\n"
	    ."\t --f filename => file (specify desiredconf to use)\n"
	    ."\t --auto => auto-yes to question? \n"
	    ."\t --C => Clean (remove file, db, user before adding new)\n"
	    ."\t --target => target (specify which setup to run [$sValidTarget])\n"
	    ."\t --Force => Force (bypass verification)\n"
	    ."\t --OS => (specify the OS you wish to use and avoid check)";
	exit;
    }
    unless($sTarget =~ /$sValidTarget/i){
	print "Incorrect target specified. Available targets are:\n"
	    ."\t --target => target (specify which setup to run [(default)$sValidTarget])\n";
	exit;
    }
    if($sDsdFile ne DESD_CONF_FILE && !(-e -r $sDsdFile)){
	print "File '$sDsdFile' could not be read or does not exist.\n";
	exit;
    }
}

sub Main {
    my($filename, $dir, $suffix) = fileparse($0);
    chdir $dir; #put ourself like was called in tools
    print "Running rAthena's configuration tool...\n";
    #default conf
    my $pubip = GetPublicIP();
    my %hDefConf = (    SERV_UID => "s1",
			SERV_PW => "p1",
			SERV_WAN_IP => $pubip || "localhost",
			LOGIN_PRIV_IP => "localhost",
			CHAR_PRIV_IP => "localhost",
			MAP_PORT => "5121",
			CHAR_PORT => "6121",
			LOGIN_PORT => "6900",
			MD5_ENABLE => "yes",
			PINCODE_ENABLE => "yes",
			SQL_HOST => "localhost",
			SQL_PORT => "3306",
			SQL_UID => "ragnarok",
			SQL_PW => "ragnarok",
			SQL_MAIN_DB => "ragnarok",
			SQL_LOG_DB => ,"ragnarok",
			);
			

	my $sBasedir = getcwd; #for setupdb
	if($sTarget =~ /All|Inst/i){ InstallSoft(); }
	if($sTarget =~ /All|Conf/i) { ConfigConf(\%hDefConf); chdir "$sBasedir"; }
	if($sTarget =~ /All|DB/i) { ConfigDB(\%hDefConf); chdir "$sBasedir"; }
	if($sTarget =~ /All|Dump/i) { chdir "~"; EnableCoredump(); }
	print "Setup successful. You can now launch and connect to the servers.\n";
	print "(Remember to update the client's 'clientinfo.xml' to match your changes.)\n";
}


sub EnableCoredump { 
	print "\n== Enabling Coredumps ==\n";
	my $sCurfile = "~/.bashrc";
	my @lines = ();
	my $sJump = .0;
	open PIPE,"sudo less $sCurfile |" or die $!;
	foreach(<PIPE>){
		if($_ =~ /ulimit -c unlimited/){
			$sJump = 1; #already in here nothing to do
			last; 
		}
		push(@lines,$_) if /ulimit/;
	}
	if(scalar(@lines)>0){
		print "ulimit instruction found in file '$sCurfile'.\n"
			."\t lines= \n @lines \n"
			."Are you sure you want to continue? [y/n] \n";
		$sJump=1 if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i);
	}
	system("sudo echo \"ulimit -c unlimited\" >> $sCurfile") if $sJump==0;
	$sJump=0;

	$sOS = GetOS() unless($sOS);
	if($sOS =~ /Fedora|CentOs/i){;
		open FILE, "</etc/security/limits.conf" || die;
		open FILE_TMP, ">tmp_limits.conf" || die;
		while(<FILE>){
			@lines=split(' ',$_);
			if($lines[2] eq "core" && $lines[3] eq "0"){
				$lines[3]="unlimited";
				print FILE_TMP "*    hard    core   unlimited\n";
				$sJump=1; #mark we have found it
			}
			else { print FILE_TMP $_; }
		}
		close FILE;
		close FILE_TMP;
		system("sudo mv tmp_limits.conf /etc/security/limits.conf") if $sJump==1; #don't overwrite if some config was already in there
		unlink "tmp_limits.conf";
	}

	$sCurfile = "/etc/sysctl.conf";
	@lines = ();
	open PIPE,"sudo less $sCurfile |" or die $!;
	foreach(<PIPE>){
		push(@lines,$_) if /^kernel.core/;
	}
	if(scalar(@lines)>0){
		print "ulimit instruction found in file '$sCurfile'.\n"
			."\t line= \n @lines \n"
			."Are you sure you want to continue? [y/n] \n";
		$sJump=2 if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i);
	}
    unless($sJump==2){
		system('sudo su root -c "echo \"echo kernel.core_uses_pid = 1 >> /etc/sysctl.conf\" | sudo bash"');
		system('sudo su root -c "echo \"echo kernel.core_pattern = /tmp/core-%e-%s-%u-%g-%p-%t >> /etc/sysctl.conf\" | sudo bash"');
		system('sudo su root -c "echo \"echo fs.suid_dumpable = 1 >> /etc/sysctl.conf\" | sudo bash"');
		system('sudo su root -c "sysctl -p"');
	}
}

sub GetPublicIP {
	print "\n== Resolving Public IP_addr";
	my $content = get("http://checkip.dyndns.org");
	$content =~ s/.*Current IP Address: ([\d.]+).*/$1/;
	$content =~ s/\r|\n//gm;
	print "\n found = $content ==\n";
	return $content;
}

sub GetOS {
	#yes we could $^0 or uname -r but $^0 only give perl binary build OS and uname hmm...
	open PIPE,"lsb_release -i  |" or die $!;
	my $sDistri = <PIPE>;
	if($sDistri){
		my @aDist =  split(":",$sDistri);
		$sDistri = $aDist[1];
		$sDistri =~ s/^\s+|\s+$//g;
		$sOS = $sDistri;
	}
	else {
		my @aSupportedOS = ("Debian","Ubuntu","Fedora","CentOs","FreeBSD");
		my $sOSregex = join("|",@aSupportedOS); 
		until($sOS =~ /$sOSregex/i){
			print "Please enter your OS [$sOSregex] or enter 'quit' to exit.\n";
			$sOS = <>; chomp($sOS);
			last if($sOS eq "quit");
		}
	}
	return $sOS;
}

sub InstallSoft {
    print "\n== Installing Software ==\n";
    print "NOTE: This auto-install feature is experimental. Package names vary in different distributions and versions, so they may be incorrect.\n";
    $sOS = GetOS() unless $sOS;
    if($sOS eq "quit"){ print "Skipping software installation...\n"; return; }
    elsif($sOS =~ /Ubuntu|Debian/i) { #tested on ubuntu 12.10,13.10
	my @aListSoft = ("gcc","gdb","zlibc","zlib1g-dev","make","git","mysql-client","mysql-server","mysql-common","libmysqlclient-dev","phpmyadmin","libpcre3-dev");
	print "Going to install: @aListSoft\n";
	system("sudo apt-get install @aListSoft");
	}
	elsif($sOS =~ /Fedora|CentOs/i){ #tested on fedora 18,19,20 /centos 5,6,7
	my @aListSoft = ("gcc","gdb","zlib","zlib-devel","make","git","mariadb-server","mariadb","mariadb-devel","phpmyadmin","pcre-devel");
#	my @aListSoft = ("gcc","gdb","zlib","zlib-devel","make","git","mysql-server","mysql-devel","phpmyadmin","pcre-devel");
	system("sudo yum install @aListSoft");
    }
    elsif($sOS =~ /FreeBSD/i){ #tested on FreeBSD 9.01
	system("portsnap fetch extract && portsnap update"); #fetch port lib and extract
	my @aDevel = ("binutils","git","autoconf","pcre","gmake","gdb");
	foreach(@aDevel){
	    system("cd /usr/ports/devel/$_ && make install clean"); #install devels
	}
#	system("cd /usr/ports/lang/gcc46 && make install"); #gcc4.6 use latest ? 4.8 ?
	system("cd /usr/ports/databases/mysql55-server && make install clean");
	#other utils ?
	system("cd /usr/ports/www/wget && make install clean");
	system("cd /usr/ports/archivers/unrar && make install clean");
    }
}

sub ConfigConf { my ($rhDefConf) = @_;
    print "\n== Setting Configurations ==\n";
    my $rhUserConf;
    while(1) {
	$rhUserConf = GetDesiredConf($rhDefConf);
	print "SetupConf using conf: \n";
	ShowConfig($rhUserConf);
	last if($sForce || AutoCheckConf($rhUserConf));
    }
    ApplySetupConf($rhUserConf);
}

sub ConfigDB { my ($rhDefConf) = @_;
    print "\n== Setting Up Databases ==\n";
    my $rhUserConf;
    while(1) {
		$rhUserConf = GetDesiredConf($rhDefConf);
		print "SetupDb using conf: \n";
		ShowConfig($rhUserConf);
		last if($sForce || AutoCheckConf($rhUserConf));
    }
    ApplySetupDB($rhUserConf);
}

#conf function
sub ApplySetupConf { my ($rhConfig) = @_;
    print "\nApplying configurations...\n";
    my @aTargetfile = (MAP_CONF_FILE,CHAR_CONF_FILE,LOGIN_CONF_FILE,INTER_CONF_FILE);
    my $sConfDir = "conf";
    my $sUserConfDir = "import";

    die "'$sConfDir' doesn't seem to exist or couldn't be read/written" unless(-d -r -w "../$sConfDir");
    chdir "../$sConfDir";
    print "Saving tmp user-conf.\n";
    YAML::XS::DumpFile(DESD_CONF_FILE,$rhConfig);
    unless(-d "$sUserConfDir") {
	print "Directory 'conf/import' doesn't exist. Create it? [y/n] (will be generated by compilation otherwise) \n";
	if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i) { die "Cannot apply configurations without 'import' folder, exiting...\n"; }
	mkdir "$sUserConfDir";
    }
    chdir $sUserConfDir;

    if($sClean){ unlink @aTargetfile; } #deleting file before applying conf if clean
    opendir(DIR, ".") or die $!;
    my @aDirfile = grep { /\.txt/ && -f "$_"  } readdir(DIR);
    close DIR;
    print "Current file in directory '@aDirfile' is target '@aTargetfile'.\n";

    foreach my $sCurfile(@aTargetfile) {
	print "Checking if target file '$sCurfile' exists... ";
	if(-e -r $sCurfile) {
	    print "Yes. Overwrite it? [y/n] \n";
	    if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i) {
		print "Only overwrite option currently supported. File skipped...\n\n";
		next;
	    }
	}
	else { print "No.\n" };

	print "\t Writing file '$sCurfile'...\n";
	if($sCurfile eq MAP_CONF_FILE) { ApplyMapConf($rhConfig,$sCurfile); }
	elsif($sCurfile eq CHAR_CONF_FILE) { ApplyCharConf($rhConfig,$sCurfile); }
	elsif($sCurfile eq LOGIN_CONF_FILE) { ApplyLoginConf($rhConfig,$sCurfile); }
	elsif($sCurfile eq INTER_CONF_FILE) { ApplyInterConf($rhConfig,$sCurfile); }
    }
}

sub ApplyMapConf { my ($rhUserConf,$sCurfile) = @_;
    open FILE, "> $sCurfile" || die "Couldn't open or create file '$sCurfile'.\n";
    print FILE "userid: " . $$rhUserConf{SERV_UID}."\n";
    print FILE "passwd: " . $$rhUserConf{SERV_PW}."\n\n";

    print FILE "map_ip: " . $$rhUserConf{SERV_WAN_IP}."\n";
    print FILE "map_port: " . $$rhUserConf{MAP_PORT}."\n";    
    print FILE "char_ip: " . $$rhUserConf{CHAR_PRIV_IP}."\n";
    print FILE "char_port: " . $$rhUserConf{CHAR_PORT}."\n";
}

sub ApplyCharConf { my ($rhUserConf,$sCurfile) = @_;
    open FILE, "> $sCurfile" || die "Couldn't open file '$sCurfile'.\n";
    print FILE "userid: " . $$rhUserConf{SERV_UID}."\n";
    print FILE "passwd: " . $$rhUserConf{SERV_PW}."\n\n";
	
    print FILE "char_ip: " . $$rhUserConf{SERV_WAN_IP}."\n";
    print FILE "char_port: " . $$rhUserConf{CHAR_PORT}."\n";
    print FILE "login_ip: " . $$rhUserConf{LOGIN_PRIV_IP}."\n";
    print FILE "login_port: " . $$rhUserConf{LOGIN_PORT}."\n";
    
    print FILE "pincode_enabled: " . $$rhUserConf{PINCODE_ENABLE}."\n";
}

sub ApplyLoginConf { my ($rhUserConf,$sCurfile) = @_;
    open FILE, "> $sCurfile" || die "Couldn't open file '$sCurfile'.\n";
    print FILE "login_port: " . $$rhUserConf{LOGIN_PORT}."\n";
    print FILE "use_MD5_passwords: " . $$rhUserConf{MD5_ENABLE}."\n";
}

sub ApplyInterConf { my ($rhUserConf,$sCurfile) = @_;
    open FILE, "> $sCurfile" || die "Couldn't open file '$sCurfile'.\n";

    print FILE "sql.db_hostname: " . $$rhUserConf{SQL_HOST}."\n";
    print FILE "sql.db_port: " . $$rhUserConf{SQL_PORT}."\n";
    print FILE "sql.db_username: " . $$rhUserConf{SQL_UID}."\n";
    print FILE "sql.db_password: " . $$rhUserConf{SQL_PW}."\n";
    print FILE "sql.db_database: " . $$rhUserConf{SQL_MAIN_DB}."\n\n";

    print FILE "char_server_ip: " . $$rhUserConf{SQL_HOST}."\n";
    print FILE "char_server_port: " . $$rhUserConf{SQL_PORT}."\n";
    print FILE "char_server_id: " . $$rhUserConf{SQL_UID}."\n";
    print FILE "char_server_pw: " . $$rhUserConf{SQL_PW}."\n";
    print FILE "char_server_db: " . $$rhUserConf{SQL_MAIN_DB}."\n\n";

    print FILE "sql.map_server_ip: " . $$rhUserConf{SQL_HOST}."\n";
    print FILE "sql.map_server_port: " . $$rhUserConf{SQL_PORT}."\n";
    print FILE "map_server_id: " . $$rhUserConf{SQL_UID}."\n";
    print FILE "map_server_pw: " . $$rhUserConf{SQL_PW}."\n";
    print FILE "map_server_db: " . $$rhUserConf{SQL_MAIN_DB}."\n\n";

    #todo may we want 2 schema ??
    print FILE "log_db_ip: " . $$rhUserConf{SQL_HOST} ."\n";
    print FILE "log_db_port: " . $$rhUserConf{SQL_PORT}."\n";
    print FILE "log_db_id: " . $$rhUserConf{SQL_UID}."\n";
    print FILE "log_db_pw: " . $$rhUserConf{SQL_PW}."\n";
    print FILE "log_db_db: " . $$rhUserConf{SQL_LOG_DB}."\n\n";
}

sub AutoCheckConf { my ($rhConfig) = @_;
    print "\n== Auto-Check Configuration ==\n";
	print "NOTE: You can use option --force=1 to bypass this.\n";
    foreach my $sKeys (keys %$rhConfig){
	my $sVal = $$rhConfig{$sKeys};
	if($sKeys =~ /PORT/) { #chek if valid port
	    if(($sVal<MIN_PORT) && ($sVal>MAX_PORT)) {
		warn "Invalid port specified for $sKeys => $sVal. Port must be in [".MIN_PORT.":".MAX_PORT."].\n";
		return 0;
	    }
	    elsif(!($sKeys =~ /SQL/) && CheckUsedPort($sVal)) { #skip SQL service
		warn "Port '$sVal' seems to already be in use by your system.\n";
		return 0;
	    }
	    elsif(CheckDupPort($rhConfig,$sKeys)) {
		warn "Port '$sVal' seems to already be used by another key in config.\n";
		return 0;
	    }
	}
	elsif($sKeys =~ /IP|HOST/){ #chek if ip valid, can we reach it ? trough SYN ACK
	    my $p = Net::Ping->new("syn");
	    my $sTest = $p->ping($sVal);
	    $p->close();
	    unless($sTest) {
		print "Invalid IP/Host, ping couldn't reach $sKeys => $sVal.\n";
		print "(NOTE: ICMP may just be unallowed.)\n";
		return 0;
	    }
	}
    }
    return 1;
}

sub CheckDupPort { my ($rhConfig,$sChkKeys) = @_;
    my $sChkport = $$rhConfig{$sChkKeys};
    foreach my $sKeys (keys %$rhConfig){
		next if($sKeys eq $sChkKeys); #skip ourself
		my $sVal = $$rhConfig{$sKeys};
		return 1 if($sChkport eq $sVal);
    }
    return 0;
}



#Db function
sub ApplySetupDB { my($rhConfig) = @_;
    my $sDbH; #db handle
    my $sHost = $$rhConfig{SQL_HOST};
    my $sPort = $$rhConfig{SQL_PORT};
    my $sDsn = "dbi:mysql::$sHost:$sPort"; #don't try to auto connect to db
    $$rhConfig{"Dsn"} = $sDsn;

    $sDbH = RootCo($rhConfig);
    CreateDB($sDbH,$rhConfig); #create db if not exist
    $sDbH = CreateUser($sDbH,$rhConfig); #loged as user now
    LoadSqlFile($sDbH,$rhConfig); #Load .sql file into db
    CreateServUser($sDbH,$rhConfig);
    print "Database setup successful.\n";
}



sub CreateDB { my($sDbH,$rhConfig) = @_;
    print "\n== Creating Databases ==\n";
    my $sDBn = $$rhConfig{SQL_MAIN_DB};
    my $sLogDBn = $$rhConfig{SQL_LOG_DB};
    my @aQuery = ("create database IF NOT EXISTS $sDBn;","create database IF NOT EXISTS $sLogDBn;");
    if($sClean){ #deleting database if clean
	unshift(@aQuery,"drop database IF EXISTS $sDBn;");
	unshift(@aQuery,"drop database IF EXISTS $sLogDBn;");
    }
    else {
	my $sRes = $sDbH->selectcol_arrayref('show databases');
	foreach my $db (@$sRes){ #relevant later for import
	    if($db eq "$sDBn") { ValidateDBMerge($db); } #may exit here
	    elsif ($db eq "$sLogDBn") { ValidateDBMerge($db); } #may exit here
	}
    }
    ExeQuery($sDbH,@aQuery);
}

sub ValidateDBMerge { my($sDBn) = @_;
    warn "Database '$sDBn' seems to already exist.\n";
    warn "Do you wish to continue loading data from the existing database? [y/n] \n";
    if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i) {
	print "Exiting setup, please try again with another dbname or manually...\n";
	exit;
    }
}

sub CreateUser { my($sDbH,$rhConfig) = @_;
    print "\n== Creating User ==\n";
    my $sDsn = $$rhConfig{"Dsn"};
    print "My dsn = $sDsn \n";
    my $sHost = $$rhConfig{SQL_HOST};
    my $sPw = $$rhConfig{SQL_PW};
    my $sUser = $$rhConfig{SQL_UID};
    my $sDBn = $$rhConfig{SQL_MAIN_DB};
    my $sLogDBn = $$rhConfig{SQL_LOG_DB};

    my @aQuery= ("GRANT ALL PRIVILEGES ON $sDBn.* TO $sUser\@'$sHost' IDENTIFIED BY '$sPw' WITH GRANT OPTION", #maindb
		 "GRANT ALL PRIVILEGES ON $sLogDBn.* TO $sUser\@'$sHost' IDENTIFIED BY '$sPw' WITH GRANT OPTION"); #logdb
    my $sUserDbh = DBI->connect($sDsn, $sUser, $sPw,  {"PrintError" => 0}); #try connect with user

    if($sUserDbh && !$sClean) {
	print "User '$sUser' seems to already exist, skipping creation...\n"
	    ."(Please check if you have correct privileges set for database '$sDBn'.)\n";
    }
    else { #create user only if not exist (or mode clean)
	if($sClean && $sUser ne "root"){ unshift(@aQuery,"DELETE FROM mysql.user WHERE User = '$sUser';"); }
	print "Creating user $sUser for databases '$sDBn' and '$sLogDBn' on '$sHost'.\n";
	ExeQuery($sDbH,@aQuery);
	$sUserDbh = DBI->connect($sDsn, $sUser, $sPw);
    }
    return $sUserDbh; #drop old co and connect with user now
}

sub LoadSqlFile { my($sDbH,$rhConfig) = @_;
    print "\n== Loading SQL Files ==\n";
    my $sDBn = $$rhConfig{SQL_MAIN_DB};
    my $sLogDBn = $$rhConfig{SQL_LOG_DB};

    my $sSqldir = "sql-files";
    my @aMainFiles = ("main.sql"); #add other file to load for main db here
    my @aLogFiles = ("logs.sql"); #add other file to load for log db here

    die "$sSqldir doesn't seem to exist or couldn't be read." unless(-d -r "../$sSqldir");
    chdir "../$sSqldir";
    print "Checking if target files exist:\n\tMain: [@aMainFiles]\n\tLog: [@aLogFiles]\n";

    CheckAndLoadSQL(\@aMainFiles,$rhConfig,$sDBn);
    CheckAndLoadSQL(\@aLogFiles,$rhConfig,$sLogDBn);

#    my $raMainQuerys = CheckAndAddQuery(\@aMainFiles,$rhConfig);
#    my $raLogQuerys = CheckAndAddQuery(\@aLogFiles,$rhConfig);
#    ExeQuery($sDbH,"use $sDBn;",@$raMainQuerys,"use $sLogDBn;", @$raLogQuerys);
}


# query failure atm (shitty perl)
#sub CheckAndAddQuery { my ($raFiles) = @_;
#    my @aQuery = ();
#    foreach(@$raFiles) {
#	unless(-f -r $_){
#	    print "File '$_' doesn't seem to exist or was unreadable, skipped...\n";
#	    next;
#	}
#	my $sFileFullPath = Cwd::abs_path($_);
#	my $sInfileQuery = "source $sFileFullPath";
#	#my $sInfileQuery = "\\. $sFileFullPath;";
#	push(@aQuery,$sInfileQuery);
#
#    }
#    return \@aQuery;
#}

sub CreateServUser { my($sDbH,$rhConfig) = @_;
    my $sUid = $$rhConfig{SERV_UID};
    my $sUpw = $$rhConfig{SERV_PW};
    my $sMD5 = $$rhConfig{MD5_ENABLE};
    my $sDBn = $$rhConfig{SQL_MAIN_DB};
    my @aQuery = ("use $sDBn;","DELETE FROM login WHERE sex='S';");
    if($sMD5){ push(@aQuery,"INSERT INTO login(account_id, userid, user_pass, sex) values(1,'$sUid',MD5('$sUpw'),'S');"); }
    else { push(@aQuery,"INSERT INTO login(account_id, userid, user_pass, sex) values(1,'$sUid','$sUpw','S');"); }
    ExeQuery($sDbH,@aQuery);
}

sub GetDesiredConf { my ($rhDefConf) = @_;
    print "Please enter desired configuration.\n";
    my $rhUserConf;
    my $sDesdConfFile = $sDsdFile;
    #if default search in conf otherwise get specified name with cwd
    if($sDsdFile eq DESD_CONF_FILE) { $sDesdConfFile = "../conf/".$sDsdFile; }

    print "Checking if there is a DesiredConf file...\n";
    if(-e -r $sDesdConfFile) {
	print "Found DesiredConf.\n";
	$rhUserConf = YAML::XS::LoadFile($sDesdConfFile);
	if(!($rhUserConf)){
	    print "DesiredConf is invalid or empty. Please check the file, and relaunch setup or enter Config.\n";
	    $rhUserConf=GetValidateConf($rhDefConf);
	}
	else {
	    ShowConfig($rhUserConf);
	    print "Would you like to apply these settings? [y/n] ";
	    if(GetValidAnwser("y|o|n",$sAutoyes) =~ /n/i) {  #no take user entry
		print "DesiredConf not applied. Please enter config.\n";
		$rhUserConf=GetValidateConf($rhDefConf);
	    }
	}
    }
    else { #no files take user entry
	print "No DesiredConf found. Please enter config.\n";
	$rhUserConf=GetValidateConf($rhDefConf);
    }
    return $rhUserConf;
}

#!/usr/bin/perl -w

##########################################################################
# INFORMATION TOOL ABOUT THE # OF ONLINE PLAYERS ON ATHENA SERVERS
#
# By connection on the athena login-server, this software displays the
# number of online players.
#
#-------------------------------------------------------------------------
# Software usage:
#   Configure the IP, the port and a valid account of the server.
#   After, use at your choice:
#   ./getlogincount.pl - display the number of online players on all servers.
#   ./getlogincount.pl --premier or
#   ./getlogincount.pl --first -- display the number of online players of the
#                              first server in the received list.
#   ./getlogincount.pl [servername] -- display the number of online players
#                                   of the specified server.
#
# If successfull, the software return the value 0.
#
##########################################################################

#------------------------------ CONFIGURATION ----------------------------

$loginserverip = "127.0.0.1";  # IP of the login-server
$loginserverport = 6900;       # port of the login-server
$loginaccount = "s1";          # a valid account name
$loginpasswd = "p1";           # the password of the valid account name

$connecttimeout = 10;          # Connection timeout (in seconds)

#-------------------------------------------------------------------------

use IO::Socket;

my($sname) = $ARGV[0];
if (!defined($sname)) {
	$sname = "";
}

# Connection to the login-server
my($so,$er) = ();
eval{
	$so = IO::Socket::INET->new(
		PeerAddr=> $loginserverip,
		PeerPort=> $loginserverport,
		Proto   => "tcp",
		Timeout => $connecttimeout) or $er=1;
};
if($er || $@){
	print "Can't not connect to the login-server [${loginserverip}:$loginserverport] !\n";
	exit(2);
}

# Request to connect on login-server
print $so pack("v V a24 a24 C",0x0064,9,$loginaccount,$loginpasswd,3);
$so->flush();

# Fail to connect
if(unpack("v", &soread(\$so,2)) != 0x0069) {
	print "Login error.\n";
	exit(3);
}

# Get length of the received packet
my($plen) = unpack("v",&soread(\$so,2))-4;

# Suppress information of the account (we need only information about the servers)
&soread(\$so,43);
$plen -= 43;

# Check about the number of online servers
if ($plen < 32) {
	printf "No server is connected to login-server.\n";
	exit(1);
}

# Read information of the servers
my(@slist) = ();
for(;$plen > 0;$plen -= 32) {
	my($name,$count) = unpack("x6 a20 V",&soread(\$so,32));
	$name = substr($name,0,index($name,"\0"));
	push @slist, [ $name, $count ];
}

# Display the result
if($sname eq "--first" || $sname eq "--premier") { # If we ask only for the first server
	printf "%-20s : %5d\n",$slist[0][0],$slist[0][1];
} elsif ($sname eq "") { # If we ask for all servers
	foreach $i(@slist) {
		printf "%-20s : %5d\n",$i->[0],$i->[1];
	}
} else { # If we ask for a specified server (by its name)
	my($flag) = 1;
	foreach $i(@slist) {
		if($i->[0] eq $sname) {
			printf "%-20s : %5d\n",$i->[0],$i->[1];
			$flag = 0;
		}
	}
	if($flag) { # If the server doesn't exist
		printf "The server [$sname] doesn't exist.\n";
		exit(1);
	}
}

# End of the software
$so->shutdown(2);
$so->close();
exit(0);

# Sub-function: get data from the socket
sub soread {
	my($so,$len) = @_;
	my($sobuf);
	if(read($$so,$sobuf,$len) < $len) {
		print "Socket read error.\n";
		exit(5);
	}
	return $sobuf;
};

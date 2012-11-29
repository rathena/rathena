//============================================================ 
//= rAthena ReadMe File
//===== By: ================================================== 
//= rAthena Development Team
//===== Compatible With: =====================================
//= rAthena SVN
//===== Description: ========================================= 
//= Basic information and installation guide with links to
//= various forum posts and wiki articles.
//============================================================

============================
||   Table of Contents    ||
============================
|| 1. What is rAthena?    ||
|| 2. Prerequisits        ||
|| 3. Installation        ||
|| 4. Troubleshooting     ||
|| 5. Helpful Links       ||
|| 6. More Documentation  ||
============================


============================
|| 1. What is rAthena?    ||
============================
	rAthena is a collaborative software development project revolving around the
creation of a robust massively multiplayer online role playing game (MMORPG)
server package. Written in C, the program is very versatile and provides NPCs,
warps and modifications. The project is jointly managed by a group of volunteers
located around the world as well as a tremendous community providing QA and
support. rAthena is a continuation of the eAthena project.



============================
|| 2. Prerequisists       ||
============================
	Before installing rAthena there are certain tools and applications you will need.
This differs between the varying operating systems available, so the following
is broken down into Windows and linux prerequisists.

	Windows
		* TortoiseSVN (http://tortoisesvn.net/downloads.html)
		* MySQL (http://www.mysql.com/downloads/mysql/)
		* MySQL Workbench (http://www.mysql.com/downloads/workbench/)
		* MS Visual C++ (http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express)

	Linux (names of packages may require specific version numbers on certain distributions)
		* gcc
		* make
		* mysql
		* mysql-devel
		* mysql-server
		* pcre-devel
		* subversion
		* zlib-devel



============================
|| 3. Installation        ||
============================
	This section is a very brief set of installation instructions. For more concise guides
relevant to your Operation System, please refer to the Wiki (links at the end of this file).

Windows
	* Install prerequisites
	* Create a folder to download rAthena into (e.g. C:\rAthena)
	* Right click this folder and select "SVN Checkout".
	* Paste the SVN URL into the box: https://rathena.svn.sourceforge.net/svnroot/rathena/trunk/
	* Open MySQL Workbench and create an instance to connect to your MySQL Server
	* Create a database (rathena), a user (rathena), give permissions (GRANT SELECT,INSERT,UPDATE,DELETE)
		and then login using the new user
	* Use MySQL Workbench to run the .sql files in /sql-files/ on the new rathena database

Linux
	(For CentOS) Type: yum install gcc make mysql mysql-devel mysql-server pcre-devel subversion zlib-devel
	(For Debian) Type: apt-get install subversion make gcc libmysqlclient-dev zlib1g-dev libpcre3-dev
	* Type: mysql_secure_installation
	* Start your MySQL server
	* Setup a MySQL user: CREATE USER 'rathena'@'localhost' IDENTIFIED BY 'password';
	* Assign permissions: GRANT SELECT,INSERT,UPDATE,DELETE ON `rathena\_rag`.* TO 'rathena'@'localhost';
	* Type: svn checkout https://rathena.svn.sourceforge.net/svnroot/rathena/trunk/ ~/trunk
	* Insert SQL files: mysql --user=root -p rathena_rag < trunk/sql-files/main.sql (and others)
	* Type: cd trunk && ./configure && make clean && make sql
	* When you're ready, type: ./athena-start start



============================
|| 4. Troubleshooting     ||
============================
	If you're having problems with starting your server, the first thing you should
do is check what's happening on your consoles. More often that not, all support issues
can be solved simply by looking at the error messages given.

Examples:

	1.) You get an error on your map-server_sql that looks something like this:
			[Error]: npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in 
			file 'npc/custom/jobmaster.txt', line '17'. Skipping line...
			* w1=prontera,153,193,6 script
			* w2=Job Master
			* w3=123,{
			* w4=

		If you look at the error, it's telling you that you're missing (or have an extra) TAB.
		This is easily fixed by looking at this part of the error: * w1=prontera,153,193,6 script
		If there was a TAB where it's supposed to be, that line would have prontera,153,193,6 at w1
		and 'script' at w2. As there's a space instead of a TAB, the two sections are read as a
		single parameter.

	2.) You have a default user/password warning similar to the following:
			[Warning]: Using the default user/password s1/p1 is NOT RECOMMENDED.
			[Notice]: Please edit your 'login' table to create a proper inter-server user/pa
			ssword (gender 'S')
			[Notice]: and then edit your user/password in conf/map_athena.conf (or conf/impo
			rt/map_conf.txt)

		Relax. This is just indicating that you're using the default username and password. To
		fix this, check over the part in the installation instructions relevant to the `login` table.
		
	3.) Your Map Server says the following:
			[Error]: make_connection: connect failed (socket #2, error 10061: No connection
			could be made because the target machine actively refused it.
			)!

		If this shows up on the map server, it generally means that there is no Char Server available
		to accept the connection.



============================
|| 5. Helpful Links       ||
============================
	The following list of links point to various helpfiles within the SVN, articles or
pages on the wiki or topics within the rAthena forum.

	* rAthena Forums
		http://rathena.org/

	* SVN Repository URL:
		https://rathena.svn.sourceforge.net/svnroot/rathena/trunk/

	* Full Installation Instructions:
		Windows		http://rathena.org/wiki/Installation_on_Windows
		CentOS		http://rathena.org/wiki/Installation_(CentOS)
		Debian		http://rathena.org/wiki/Installation_(Debian)
	
	* rAthena IRC Channel
		irc://irc.rathena.net



============================
|| 6. More Documentation  ||
============================
	rAthena has a large collection of help files and sample NPC scripts located in /doc/

	* Scripting
		It is recommended to look through /doc/script_commands.txt for help, pointers or
		even for ideas for your next NPC script. Most script commands have a usage example.
	
	* @commands
		In-game, Game Masters have the ability to use Atcommands (@) to control players, 
		create items, spawn mobs, reload configuration files and even control the weather.
		For an in-depth explaination, please see /doc/atcommands.txt

	* Permissions
		The rAthena emulator has a permission system that enables certain groups of players
		to perform certain actions, or have access to certain visual enhancements or in-game
		activity. To see what permissions are available, they are detailed in /doc/permissions.txt

There are more files in the /doc/ directory that will help you to create scripts or update the
mapcache, er even explain how the job system and item bonuses work. Before posting a topic asking
for help on the forums, we reccomend that all users take the time to look over this directory.
rAthena
=======

Build Status: [![Build Status](https://travis-ci.org/rathena/rathena.png?branch=master)](https://travis-ci.org/rathena/rathena) [![Build status](https://ci.appveyor.com/api/projects/status/8574b8nlwd57loda/branch/master?svg=true)](https://ci.appveyor.com/project/rAthenaAPI/rathena/branch/master)

Table of Contents
---------
1. What is rAthena?
2. Prerequisites
3. Installation
4. Troubleshooting
5. Helpful Links
6. More Documentation
7. How to Contribute

1. What is rAthena?
---------
rAthena is a collaborative software development project revolving around the
creation of a robust massively multiplayer online role playing game (MMORPG)
server package. Written in C, the program is very versatile and provides NPCs,
warps and modifications. The project is jointly managed by a group of volunteers
located around the world as well as a tremendous community providing QA and
support. rAthena is a continuation of the eAthena project.

2. Prerequisites
---------
Before installing rAthena there are certain tools and applications you will need.
This differs between the varying operating systems available, so the following
is broken down into Windows and Linux prerequisites.

* Windows
	* MySQL ( http://www.mysql.com/downloads/mysql/ )
	* MySQL Workbench ( http://www.mysql.com/downloads/workbench/ )
	* MS Visual Studio ( https://www.visualstudio.com/downloads/ )
	* TortoiseGIT ( http://code.google.com/p/tortoisegit/ )
	* git for Windows ( https://gitforwindows.org/ )

* Linux (names of packages may require specific version numbers on certain distributions)
	* gcc
	* g++
	* make
	* mysql
	* mysql-devel
	* mysql-server
	* pcre-devel
	* zlib-devel
	* git

3. Installation 
---------
This section is a very brief set of installation instructions. For more concise guides
relevant to your Operation System, please refer to the Wiki (links at the end of this file).

* Windows
	* Install prerequisites
	* Create a folder to download rAthena into (e.g. C:\rAthena)
	* Right click this folder and select "Git Clone"
	* Paste the GitHub URL into the box:

				https://github.com/rathena/rathena.git
				
	* Open MySQL Workbench and create an instance to connect to your MySQL Server
	* Create a database (rathena), a user (rathena), give permissions (GRANT SELECT,INSERT,UPDATE,DELETE)
		and then login using the new user
	* Use MySQL Workbench to run the .sql files in /sql-files/ on the new rathena database

* Linux
	* Type:
		* (For CentOS)

				yum install gcc g++ make mysql mysql-devel mysql-server pcre-devel zlib-devel git
		* (For Debian)

				apt-get install git make gcc g++ libmysqlclient-dev zlib1g-dev libpcre3-dev
	* Type:

				mysql_secure_installation
	* Start your MySQL server
	* Setup a MySQL user:

				CREATE USER 'rathena'@'localhost' IDENTIFIED BY 'password';
	* Assign permissions:

				GRANT SELECT,INSERT,UPDATE,DELETE ON `rathena\_rag`.* TO 'rathena'@'localhost';
	* Clone a GIT repository:

				git clone https://github.com/rathena/rathena.git ~/rathena
	* Insert SQL files:

				mysql --user=root -p rathena_rag < trunk/sql-files/main.sql (and others)
	* Configure and compile:

				./configure && make clean && make server
	* Configure and compile (for Centos 64 bit with Maria DB):

				./configure --enable-64bit --with-MYSQL_LIBS=/usr/lib64/libmysqlclient.so && make clean && make server
	* When you're ready, start the servers:

				./athena-start start

* Optional
	* If you want to have your own forked version but still get updates from the main rAthena repository
		* Fork this repository to your GitHub account
		* List the current configured remote repository for your fork:
		
				git remote -v

		* Specify a new remote upstream repository that will be synced with your fork:
		
				git remote add upstream https://github.com/rathena/rathena.git
				
		* Verify the new upstream repository you've specified for your fork:
		
				git remote -v
				
		* You should see the main rAthena repository as well as your forked repository
		* Now, when you want to get updates from rAthena, simply do:
		
				git pull upstream master

	* Remember that rAthena falls under [GNU GPLv3](https://github.com/rathena/rathena/blob/master/LICENSE).

4. Troubleshooting
---------
If you're having problems with starting your server, the first thing you should
do is check what's happening on your consoles. More often that not, all support issues
can be solved simply by looking at the error messages given.

Examples:

* You get an error on your map-server that looks something like this:

			[Error]: npc_parsesrcfile: Unable to parse, probably a missing or extra TAB in 
				file 'npc/custom/jobmaster.txt', line '17'. Skipping line...
				* w1=prontera,153,193,6 script
				* w2=Job Master
				* w3=123,{
				* w4=

    If you look at the error, it's telling you that you're missing (or have an extra) TAB.
		This is easily fixed by looking at this part of the error:

				* w1=prontera,153,193,6 script

	If there was a TAB where it's supposed to be, that line would have prontera,153,193,6 at w1
		and 'script' at w2. As there's a space instead of a TAB, the two sections are read as a
		single parameter.

* You have a default user/password warning similar to the following:

			[Warning]: Using the default user/password s1/p1 is NOT RECOMMENDED.
			[Notice]: Please edit your 'login' table to create a proper inter-server user/pa
			ssword (gender 'S')
			[Notice]: and then edit your user/password in conf/map_athena.conf (or conf/impo
			rt/map_conf.txt)

    Relax. This is just indicating that you're using the default username and password. To
		fix this, check over the part in the installation instructions relevant to the `login` table.
	
* Your map-server outputs the following:

			[Error]: make_connection: connect failed (socket #2, error 10061: No connection
			could be made because the target machine actively refused it.
			)!

    If this shows up on the map server, it generally means that there is no Char Server available
		to accept the connection.



5. Helpful Links
---------
* rAthena Forums
	* https://rathena.org/

* GIT Repository
	* https://github.com/rathena/rathena

* Full Installation Instructions
	* [Windows](https://github.com/rathena/rathena/wiki/Install-on-Windows)
	* [CentOS](https://github.com/rathena/rathena/wiki/Install-on-Centos)
	* [Debian](https://github.com/rathena/rathena/wiki/Install-on-Debian)
	* [FreeBSD](https://github.com/rathena/rathena/wiki/Install-on-FreeBSD)

* rAthena Discord Chat
	https://rathena.org/discord

* rAthena Wiki
	https://github.com/rathena/rathena/wiki

* Fork and Pull Request Q&A
	https://rathena.org/board/topic/86913-pull-request-qa/


6. More Documentation
---------
rAthena has a large collection of help files and sample NPC scripts located in the /doc/
directory. These include detailed explanations of NPC script commands, atcommands (@),
group permissions, item bonuses, and packet structures, among many other topics. We
recommend that all users take the time to look over this directory before asking for
assistance elsewhere.

7. How to Contribute
---------
Details on how to contribute to rAthena can be found in [CONTRIBUTING.md](https://github.com/rathena/rathena/blob/master/.github/CONTRIBUTING.md)!

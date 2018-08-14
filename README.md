<img src="branding/logo.png" align="right" height="90" />

# rAthena
[![Build Status](https://travis-ci.org/rathena/rathena.png?branch=master)](https://travis-ci.org/rathena/rathena) [![Build status](https://ci.appveyor.com/api/projects/status/8574b8nlwd57loda/branch/master?svg=true)](https://ci.appveyor.com/project/rAthenaAPI/rathena/branch/master)
> rAthena is a collaborative software development project revolving around the creation of a robust massively multiplayer online role playing game (MMORPG) server package. Written in C, the program is very versatile and provides NPCs, warps and modifications. The project is jointly managed by a group of volunteers located around the world as well as a tremendous community providing QA and support. rAthena is a continuation of the eAthena project.

[Forum](https://rathena.org/board)|[Discord](https://rathena.org/discord)|[Wiki](https://github.com/rathena/rathena/wiki)|[FluxCP](https://github.com/rathena/FluxCP)|[Fork and Pull Request Q&A](https://rathena.org/board/topic/86913-pull-request-qa/)
--------|--------|--------|--------|--------

### Table of Contents
1. [Prerequisites](#1-prerequisites)
2. [Installation](#2-installation)
3. [Troubleshooting](#3-troubleshooting)
4. [More Documentation](#4-more-documentation)
5. [How to Contribute](#5-how-to-contribute)

## 1. Prerequisites
Before installing rAthena there are certain tools and applications you will need.
This differs between the varying operating systems available, so the following
is broken down into Windows and Linux prerequisites.

### Windows
  * [MySQL](http://www.mysql.com/downloads/mysql/) or [MariaDB](https://downloads.mariadb.org/) as database to store data
  * [git for Windows](https://gitforwindows.org/) for keep in touch with rAthena's updates!
  * [MS Visual Studio](https://www.visualstudio.com/downloads/) as compiler
  * [MySQL Workbench](http://www.mysql.com/downloads/workbench/) as MySQL GUI

### Linux
Names of packages may require specific version numbers on certain distributions
  * gcc
  * g++
  * make
  * mysql
  * mysql-devel
  * mysql-server
  * pcre-devel
  * zlib-devel
  * git

## 2. Installation 

### Full Installation Instructions
  * [Windows](https://github.com/rathena/rathena/wiki/Install-on-Windows)
  * [CentOS](https://github.com/rathena/rathena/wiki/Install-on-Centos)
  * [Debian](https://github.com/rathena/rathena/wiki/Install-on-Debian)
  * [FreeBSD](https://github.com/rathena/rathena/wiki/Install-on-FreeBSD)

## 3. Troubleshooting

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


## 4. More Documentation
rAthena has a large collection of help files and sample NPC scripts located in the /doc/
directory. These include detailed explanations of NPC script commands, atcommands (@),
group permissions, item bonuses, and packet structures, among many other topics. We
recommend that all users take the time to look over this directory before asking for
assistance elsewhere.

## 5. How to Contribute
Details on how to contribute to rAthena can be found in [CONTRIBUTING.md](https://github.com/rathena/rathena/blob/master/.github/CONTRIBUTING.md)!

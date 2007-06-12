VSS2SVN Conversion Tool
=======================

Preface
-------

For those who never read any docs, please at least read the "Configuration" section further down. All the config files are also well documented, so maybe even less motivation to read this text. But please, I took my time to write it, take yours to read it. Thank you :-)


What is it?
-----------
This is a bash script to be run from cygwin. Its aim is to do a fully automatic conversion of a VSS database to Subversion with the help of the great vss2svn perl script. Once the script is tweaked for your needs, you start it, leave the computer alone for a few hours and come back when the conversion is done.


What it is not
--------------
This script is NOT a run-me-and-I-do-it-all-fully-automagically-for-you script. Not at all. In fact, you will still have to do a lot of tweaking and analyzing errors. However, once you have done that, the script will run through without any interaction.


Prerequisites
-------------
- vss2svn	http://www.pumacode.org/
- VSS
- cygwin	http://www.cygwin.com
- ActivePerl	http://www.activestate.com/	(optional)


Installation
------------
- Unzip convert.zip to some folder on your harddrive. Make sure you have plenty of disk space (approx. 10 times(!) size of your VSS database)
- Copy ssphys.exe into the folder "tools/vss2svn"
- Now you have two options: Either use precompiled or source version of vss2svn:
	a) Precompiled: Put vss2svn.exe into the folder "tools/vss2svn"
	b) Source: put the folder "script" from the SVN sources into the folder "tools/vss2svn"
- VSS needs to be installed, or at least SS.EXE and ANALYZE.EXE must be ready to use.
- Install cygwin with standard options + Devel/subversion. Maybe you want X, I did, it is up to you.


Process Overview
----------------
To give you an idea and better understanding of what is happening, here are the main steps the script does:

- copy VSS database to local hard drive for faster operation
- destroy unwanted projects from VSS database 
- run vss2svn
- rename some projects because of the silly svndumpfilter program
- split the big dumpfile into 2 smaller ones
- now split those 2 dumpfiles into single dumps per project
- create subversion repositories from the dumpfiles

You may ask what the renaming is for. Let me explain:
svndumpfilter should, in my opinion, be called svnDUMBfilter, because it is really stupid. Let's say you have a dumpfile that contains these two projects:
ProjectA
ProjectAB
How would you split this into two single dumps? I tried:
svndumpfilter include ProjectA < big.dump > ProjectA.dump
svndumpfilter include ProjectAB < big.dump > ProjectAB.dump

Doesn't give what you expected? Indeed not, ProjectA.dump still contains both projects. This also doesn't help:
svndumpfilter include ProjectA/ < big.dump > ProjectA.dump

I told you, svndumpfilter is dumb. Ok, it does have the exclude switch and if you exclude everything else, then you are set. Not very convenient for what we are up to. Also, I think this slowed svndumpfilter way down, but I may be wrong here.
So you need to look through all your projects and check, if projects have similar names like in my example and give your projects unique names.

The splitting up of the dumpfile in 2 (called nice and ugly) has this background:
You may get lots and lots of orphaned folders, like I did. Then some projects "depend" on these orphaned folders. Or maybe they depend on other projects. I called such projects "ugly". All such projects end up in the "ugly" dunpfile which makes analyzing dependencies and orphaned folders easier. All other projects are "nice" projects. Also, svndumpfilter gets really slow on big dumpfiles. Having 2 smaller ones greatly increases the splitting.


Configuration
-------------
Before you can use the script, you need to edit a few files to represent your set up:

1. config.sh:	Main config file to set up paths and such. It is very well commented.
2. rename.sed:	sed script file to do the renaming. If you don't have conflicting project names like described in the previous section, leave this file alone and change the parameter of dump_rename from 1 to 0 in the file "convert.sh".
3. projects/projects_destroy:	list of projects to be destroyed from VSS database
4. projects/projects_nice:	list of projects that have no dependencies
5. projects/projects_ugly:	list of projects that have dependencies
6. projects/dependency:	If a project has a dependency, then you need to create a file in this folder. The name of this file must be exactly the project name. In that file put a list of project names that shall be also included by svndumpfilter.


Usage
-----
Now we are finally ready to run the script! Follow these steps:
1. Make sure no one has access to the VSS database. You can't just copy it while people are using VSS! Do not lock it, otherwise SS.EXE won't be able to destroy projects.
2. Start cygwin
3. On the command line prompt, type the following two commands:
	cd /cygdrive/x/path/to/convert
	sh convert.sh
   Of course you will have to give the actual path in the first command.
4. Wait for the script to finish.
5. Analyze errors, fix them and rerun the script.

You will have to run the script several times. There will be errors, you will find out some dependencies or God knows what can happen. It is normal, so don't be afraid! This script was made so that in the end, after lots of analyzing and tweaking, you can start the script and then have it do its work without having to sit in front of the computer!

But before you make a subsequent run of the script, please check the variable "VSSDUMPDATE" in the "config.sh" file. You may want to set it to some fixed value now (see comment in config.sh).

Also note, that on subsequent runs, any step that has been previously made, is being skipped! E.g. if the script notices that the VSS database already has been copied, it won't copy again. Or if a dump file of the same name already exists, it will not overwrite this and skip the according step! This way you do not need to comment out the steps that you don't want to do a second time. If you want a step to be executed again, you have to delete that file, that a previous run of the script generates. The script always tells you, when it skipped a step and why it did so.

I think this is it! I hope this helps you converting your VSS database to Subversion. If you have questions regarding this script, please do not hesitate to ask me:

ich@der-ingo.de

I'll be happy to help. But bear in mind, that this script is by no means perfect. I made it in the process of converting the VSS database at my company, so it was for a rather specific set up. Making it ready for public use took a lot of my personal spare time. Always consider that when contacting me. Thank you :-)


Experimental
------------
If you get tons of orphaned folders like I did, and even worse, if a lot of your projects seem to depend on them (again like in my case) you may consider to use the vss2svn.pl perl script from the subversion repository instead of the precompiled binary.
For that you will have to install ActivePerl. Perl from cygwin did NOT work. I can't remember how to set ActivePerl up, you will have to fiddle or search the vss2svn mailing list. Then set the variable "USE_PERL" in "config.sh" to 1.

And now comes the "evil" part. If you look at the file "tools/ActionHandler.diff". It is a patch for ActionHandler.pl which you will find in "script\Vss2Svn" of the vss2svn subversion repository. After you have checked out the vss2svn sources, copy the script folder to "tools/vss2svn" and then apply this patch to "tools/vss2svn/script/Vss2svn/ActionHandler.pl".

The patch has the following purpose:
It will rename all those orphaned folders right at the source, that is, before the dumpfile is generated. This has the great advantage, that the vss2svn script takes care of creating all folders in leading to your actual renamed folder in the dumpfile.

However, this process is highly experimental and may actually fail or cause malfunction of the conversion. So be warned! However, in my case this worked absolutely perfectly. Maybe this is because I figured out really each single orhpaned folder, where it once came from!

Ok, now that you know what this patch is about, open ActionHandler.pl and find the line with:

our %orphanedReplace =

It is almost at the beginning of the file. You will see some example mappings from VSS names to real paths. Just edit this to suit your needs.

But I can promise you, it will take a long long time to find out what all these orphaned folders once really were. The files you want to look at to help you find this out are:
temp/_vss2svn/datacache.PhysicalAction.tmp.txt
temp/_vss2svn/datacache.VssAction.tmp.txt
Also, it helped me a great deal to actually leave the orphaned folders alone in a first run and add them as a dependency to all the projects that needed them. This made the script run through without errors and created the all the subversion repositories. Then I used a repository browser to actually see when what happened to all these orhpaned folders. If you have the date it will be a piece of cake finding this in the VSS history and you may find out what had really happened.

This is it! I hope you found this useful!

Before I finish:

A big THANK YOU goes to the creators of vss2svn which is an awesome tool. Without it I would not have managed to do the conversion so perfectly!



Gerbrunn, 2007-04-27
	Ingo Schmidt


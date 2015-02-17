vss2svn - Visual SourceSafe to Subversion Converter
=======

This is the source code for a tool that was written in the 2006-2007 timeframe by myself and several others to
convert a Visual SourceSafe (VSS) repository into the Subversion "dumpfile" format so that its history can be
imported into a Subversion repository.

Development & Documentation
--------

Active development on this tool has long since been ended but I decided to import it here in case anyone else
finds it useful. Feel free to fork it but I can't promise I'll accept pull requests as I don't have the
resources to verify or test your code.

I originally wrote this on Windows but it's probably easier to get it up and running on Linux since the
former doesn't work as well with Perl. The C program *should* compile fine on either platform.

The most up-to-date documentation can be found on
[the Google Code Wiki](https://code.google.com/p/vss2svn/wiki/Welcome), which has also been copied
[to the wiki here](wiki), though some formatting/links are broken.

How It Works
--------

What makes this tool different from many other similar tools is that it **does not require the Microsoft
VSS API** to be installed to use it. Instead, one rather intrepid developer in our group went and
reverse-engineered the VSS physical storage format and wrote a C program to parse those files and output
the results into an XML format.

The other half of this project is a Perl program that reads the resulting XML output and assembles it into
the Subversion dumpfile format. The dumpfile format is a complete archive of a repository and can be
imported using the
["svnadmin load"](http://svnbook.red-bean.com/en/1.7/svn.reposadmin.maint.html#svn.reposadmin.maint.migrate)
command.

The end result is that this tool often does a better job than even Microsoft's libraries at extracting old
history. Unfortunately VSS repositories are so susceptible to corruption that any repo of significant size,
age, and/or usage is almost guaranteed to be at least partially corrupted. The farther back in time the
history goes, the more likely it is to be less than correct.

So the general upshot is that, while this tool often works "better than most" at getting your *true* code
history out of VSS, some of that history may be gone forever. It's just the nature of the beast. Maybe
switching to a new Version Control System is a good chance to refactor your code base anyway? You may
want to consider migrating your old history to Subversion for historical reference only, then starting
fresh for further active development.

Other Versions
-------

Unfortunately the name "Vss2Svn" turned out to be rather generic and there is [at least one other completely
unrelated version](http://vss2svn.codeplex.com/) of a tool that serves the same general purpose.

This tool has had various homes throughout the years. Open source code collaboration wasn't nearly as good as
GitHub back then, ya know. Previous homes include:
* http://vss2svn.tigris.org/
* A personal Trac/Subversion site I set up at PumaCode.org (don't go there; it's run by spammers now)
* https://code.google.com/p/vss2svn/

**THE SOURCE CODE ON GITHUB SHOULD BE CONSIDERED THE "LATEST" VERSION** and supercedes all previous versions
of the codebase.

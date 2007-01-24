From Stephen Lee via vss2svn-users@lists.pumacode.org:
=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~

As I mentioned before, the 0.11.0 alpha1 (which still seems to be the latest .exe version actually available - I never got as far as sorting out the required perl environment under windows) got somewhat confused by my database...

Some files, and even entire projects were in the orphaned folder when they should have been in other locations, mainly it seemed due to confusion caused by multiple files/projects that had used the same name at different points in time, or had been renamed back and forth etc.

Others were "genuine orphans" - e.g. files that had been branched, and one of the branches deleted, or had been initially created in a project that was later destroyed after sharing the files to another project.

While I could have fixed it up afterwards in Subversion, this would have needed repeating for every labelled version (in my case entire subprojects that had been created very early in the development were incorrectly orphaned, and were thus missing from every labelled version)


The attached "filterorphan.cpp" is run as a filter on the dumpfile. I used the equivalent of the following command sequence for each iteration:

cl filterorphan.cpp
filterorphan.exe < vss2svn-dumpfile.txt > dumpfile-filtered.txt
rmdir /s /q importtest
svnadmin create importtest
svnadmin load importtest < dumpfile-filtered.txt


I've trimmed most of my database-specific content from the .cpp file, but left a few in to give an idea of how it would work. I had approx 300 orphans to deal with of which just over a dozen were "false orphans", and most of the others were easily identified in large batches due to being created/branched in a subsequently destroyed project.

The code is not necessarily the nicest you'll ever have seen (there's some stuff that it ends up doing 2 or 3 different ways), but was "good enough" and "does the job" for me... it is provided on an as-is basis for anyone who needs something similar and wants to tidy it up, or for anyone who wants a hint about the kind of stuff that the conversion script got wrong...


First time it is run, it will output to stderr a list of all orphans in a format than can be copy-pasted into the SetupReplacements function, and filled in as needed. I found it easiest to classify each "true orphan" into a subfolder of "orphaned" rather than the one-per-file orphaned subfolder that the script produces, and put the "false orphans" in the correct place.

I was also working with the  datacache.PhysicalAction.tmp.txt and datacache.VssAction.tmp.txt loaded into excel (and the former sorted by the timestamp column), which helps a lot with figuring out which file was which, and sorting out the true orphans from the false.

On later runs, the output to stderr will also include indications of progress and actions taken depending on which lines are uncommented at the time...


In basic terms the file works by reading the input file one byte at a time, and matching it against a number of potential triggers. If your database is much bigger than mine (which was a dumpfile <1GB) you might want something a bit more streamlined, or to translate the ideas into a language that's more efficient for stream manipulation (I used C-style C++ solely because the IDE and programmer's familiarity was already there. Given a freer choice it would have been a relatively poor choice for this problem). Other tools I might otherwise have used simply couldn't cope with a file that size, or with the text/binary mix.

"orphaned/_" was originally the only trigger, and the others were added incrementally as the errors in the svnadmin load command or other discrepancies dictated... for example suppressing creation of a dir that had already been created, creating a dir that did did not exist yet, duplicating an action on one shared copy of a file on another that the script had "forgotten" about, etc...

If anyone actually wants any other part of this program explaining feel free to contact me, but it is essentially the "final" (for me) iteration of a make-it-up-as-you-go-along cycle, at the end of which most of the head and labelled revisions were correct. Comments are unfortunately fairly sparse, being targeted mainly at "me trying to figure out what I was doing 1 or 2 months later". 

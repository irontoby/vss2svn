rem This script is run from /tags/x.y.z
call pod2text -l -w 79 install.pod > INSTALL.txt
call pod2text -l -w 79 readme.pod > README.txt
call pod2text -l -w 79 vss2svn.pl >> give-exe-help.pl
copy vss2svn.pl vss2svn-for-exe.pl
perl -i.bak -e "$a=`type give-exe-help.pl`;while(<>){s/## EXE PRECOMPILE HERE/$a/ && $b++;print;last if $b}" vss2svn-for-exe.pl
del vss2svn-for-exe.pl.bak
perl -e "use Cwd;($d=cwd) =~ s:.*/::;$cmd=qq{perlapp.exe --script vss2svn-for-exe.pl --exe vss2svn-$d.exe --freestanding --force --verbose};system $cmd"
del vss2svn-for-exe.pl
perl -e "use Cwd;($d=cwd) =~ s:.*/::;$cmd=qq{wzzip vss2svn-$d.zip CHANGES.txt INSTALL.txt README.txt vss2svn.pl};system $cmd"

@set oldPath=%PATH%
@set PATH=%PATH%;f:\src\vss2svn\vss\test\win32
@set PATH=%PATH%;f:\src\vss2svn\vss\test\win32;f:\src\vss2svn\ssphys\ssrep\debug
perl -Ilib test.pl
@set PATH=%oldPath%

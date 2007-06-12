#!/bin/bash
# vim:ts=2:

. ./config.sh
. ./functions.sh

echo -n "Starting conversion at "
date

vss_copy
vss_projects_destroy
vss2svn
dump_rename 1
dump_nice
dump_ugly
dump_split
svn_create

echo -n "Finished at "
date


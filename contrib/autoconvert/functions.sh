#!/bin/bash
# vim:ts=2:

function projects_get {
	cat $1 | while read project; do
		echo $project
		if [ -a "projects/dependency/$project" ]; then
			cat "projects/dependency/$project"
		fi
	done
}

function vss_copy {
	if [ -d "${PATH_TEMP}Repository" ]; then
		echo "- Not copying because $PATH_VSSREPOSITORY$FOLDER_VSSREPOSITORY already exists"
	else
		echo -n "- Copying the entire VSS Repository to local HD for faster operation..."
		cp -R "$PATH_VSSREPOSITORY$FOLDER_VSSREPOSITORY" "$PATH_TEMP"
		echo "done."
	fi
}

function vss_projects_destroy {
	cat projects/projects_destroy | while read project; do
		echo "$/$project"
		"$SS" Cp -I-Y "$/$project"
		"$SS" Destroy -I-Y "$/$project"
	done
	rm -rf "$PATH_TEMP$FOLDER_VSSREPOSITORY/data/backup/"
	"$ANALYZE" -C -D -F -I-Y -V1 "`cygpath -w $SSDIR/data`"
}

function vss2svn {
	if [ -d "${PATH_TEMP}_vss2svn" ]; then
		echo "- Not converting because directory ${PATH_TEMP}_vss2svn exists"
	else
		echo -n "Converting VSS to SVN. This takes some time..."
		if [ $USE_PERL -eq 1 ]; then
			cd "${PATH_TOOLS}vss2svn/script"
			perl vss2svn.pl --vssdir "../../../$PATH_TEMP$FOLDER_VSSREPOSITORY" --ssphys "../../../${PATH_TOOLS}vss2svn/ssphys.exe" --tempdir "../../../${PATH_TEMP}_vss2svn" --revtimerange 900 --dumpfile "../../../$VSSORIGDUMP" > "../../../${PATH_TEMP}vss2svn.log" 2> "../../../${PATH_TEMP}vss2svn.error.log"
			cd -
		else
				tools/vss2svn/vss2svn.exe --vssdir "$PATH_TEMP$FOLDER_VSSREPOSITORY" --ssphys "${PATH_TOOLS}vss2svn/ssphys.exe" --tempdir "${PATH_TEMP}_vss2svn" --revtimerange 900 --dumpfile "$VSSORIGDUMP" > "${PATH_TEMP}vss2svn.log" 2> "${PATH_TEMP}vss2svn.error.log"
		fi
		echo "done."
	fi
}

function dump_rename {
	if [ -a "$VSSRENAMEDDUMP" ]; then
		echo "- Not renaming because $VSSRENAMEDDUMP already exists"
	else
		if [ $1 -eq 1 ]; then
			echo -n "- Renaming projects because of the silly svndumpfilter. This takes a long long time..."
			./rename.sed -b < "$VSSORIGDUMP" > "$VSSRENAMEDDUMP"
			echo "done."
		else
			VSSRENAMEDDUMP="$VSSORIGDUMP"
		fi
	fi
}

function dump_nice {
	if [ -a "$VSSNICEDUMP" ]; then
		echo "- Not creating nice projects dump file because $VSSNICEDUMP already exists"
	else
		echo -n '- Generating a dump file for "nice" projects. This takes a long time, but greatly speeds things up later.'
		include=$(projects_get projects/projects_nice)
		svndumpfilter --quiet --drop-empty-revs --renumber-revs include $include < $VSSRENAMEDDUMP > $VSSNICEDUMP
		echo "done."
	fi
}

function dump_ugly {
	if [ -a "$VSSUGLYDUMP" ]; then
		echo "- Not creating ugly projects dump file because $VSSUGLYDUMP already exists"
	else
		echo -n '- Generating a dump file for "ugly" projects. This takes a long time, but greatly speeds things up later.'
		include=$(projects_get projects/projects_ugly )
		svndumpfilter --quiet --drop-empty-revs --renumber-revs include $include < $VSSRENAMEDDUMP > $VSSUGLYDUMP
		echo "done."
	fi
}

function dump_split {
	echo "Now we will split all the projects."
	cat projects/projects_nice | while read project; do
		echo -n $project"..."
		if [ -a ${PATH_DUMPSPLIT}${project}.dump ]; then
			echo -n "file exists, skipping..."
		else
			svndumpfilter --quiet --drop-empty-revs --renumber-revs include $project < $VSSNICEDUMP > ${PATH_DUMPSPLIT}${project}.dump
		fi
		echo "done."
	done
	cat projects/projects_ugly | while read project; do
		echo -n $project"..."
		if [ -a ${PATH_DUMPSPLIT}${project}.dump ]; then
			echo -n "file exists, skipping..."
		else
			if [ -a "projects/dependency/${project}" ]; then
				dependency=$(cat "projects/dependency/${project}")
			fi
			svndumpfilter --quiet --drop-empty-revs --renumber-revs include $project $dependency < $VSSUGLYDUMP > ${PATH_DUMPSPLIT}${project}.dump
		fi
		echo "done."
	done
	echo "Splitting is done."
}

function svn_create {
	echo "Creating Subversion repositories. Yet again, go have some coffee or tea..."
	cd "$PATH_DUMPSPLIT"
	find *.dump | while read file
	do
		folder=`echo "$file" | sed s/\.dump//`
		echo -n "Creating $folder..."
		if [ -d $folder ]; then
			echo -n "directory exists, skipping..."
		else
			svnadmin create "$folder"
			svn -m "tags and branches folders added" mkdir "file://`pwd`/$folder/tags" "file://`pwd`/$folder/branches"
			cat $file | sed -b -e "s/\(^Node-.*: \)$folder/\1trunk/" | svnadmin -q load "$folder"
		fi
		echo "done."
	done
	cd - > /dev/null
	echo "done."
}


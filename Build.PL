use Module::Build;

Module::Build->new
    ( dist_name => 'Vss2Svn',
      dist_version_from => 'script/vss2svn.pl',
      license => 'gpl',
      requires => {
	  'Getopt::Long' => '>= 2.34',
	  'DBI' => '>= 1.47',
	  'DBD::SQLite2' => '>= 0.33',
	  'XML::Simple' => '>= 2.14',
	  'File::Find' => 0,
	  'File::Path' => 0,
	  'Time::CTime' => 0,
	  'Data::Dumper' => 0,
      },
      script_files => { 'script/vss2svn.pl' => 'script/vss2svn.pl', 'ssphys/ssphys' => 'bin/ssphys'},
      pm_files => { 'script/Vss2Svn/ActionHandler.pm' => 'lib/Vss2Svn/ActionHandler.pm',
		    'script/Vss2Svn/DataCache.pm' =>  'lib/Vss2Svn/DataCache.pm',
		    'script/Vss2Svn/Dumpfile.pm' =>  'lib/Vss2Svn/Dumpfile.pm',
		    'script/Vss2Svn/SvnRevHandler.pm' =>  'lib/Vss2Svn/SvnRevHandler.pm',
		    'script/Vss2Svn/Dumpfile/Node.pm' =>  'lib/Vss2Svn/Dumpfile/Node.pm',
		    'script/Vss2Svn/Dumpfile/AutoProps.pm' =>  'lib/Vss2Svn/Dumpfile/AutoProps.pm',
		    'script/Vss2Svn/Dumpfile/LabelMapper.pm' =>  'lib/Vss2Svn/Dumpfile/LabelMapper.pm',
		    'script/Vss2Svn/Dumpfile/SanityChecker.pm' =>  'lib/Vss2Svn/Dumpfile/SanityChecker.pm',
		    },
      PL_files => { 'ssphys/ssphys.PL' => 'bin/ssphys' },
      )->create_build_script;

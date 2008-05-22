package Vss2Svn::DataCache;

use warnings;
use strict;

our(%gCfg);

# SQLite can get a bit slow when doing lots of sequential inserts, so we speed
# that up by using the sqlite2 "COPY" command, which allows reading in a tab-
# delimited file of data all at once. Each table that will be filled has a
# DataCache object created; the 'add' method adds a row of data to the cache,
# and the 'commit' method performs the COPY operation.

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $table, $autoinc, %flags) = @_;

    my $self =
        {
         table => $table,
         autoinc => $autoinc,
         pkey => -1,
         verbose => $gCfg{verbose},
         fh => undef,
         file => "$gCfg{cachedir}/datacache.$table.tmp.txt",
         reused => 0,
        };

    $self = bless($self, $class);

    if ($self->{verbose}) {
        print "\nSTARTING CACHE FOR $table\n";
    }

    $self->_delete_table();

    if (-e $self->{file}) {
        if (-f $self->{file} && $flags{-reuse_data}) {
            $self->{reused} = 1;
        } elsif (!(unlink($self->{file}))) {
    	    print "\nERROR: Could not delete existing cache file '$self->{file}'\n";
            return undef;
        }
    }

    if ( !open($self->{fh}, ">>$self->{file}") ) {
        print "\nERROR: Could not open file '$self->{file}'\n";
        return undef;
    }

    # we'll be sending UTF-8 to this handle
    binmode $self->{fh}, ":utf8";

    return $self;
}  #  End new

###############################################################################
#  _delete_table
###############################################################################
sub _delete_table {
    my($self) = @_;

    my $sth = $gCfg{dbh}->prepare("DELETE FROM $self->{table}");
    return $sth->execute;

}  #  End _delete_table

###############################################################################
#  add
###############################################################################
sub add {
    my($self, @data) = @_;

    if (ref($data[0]) eq 'ARRAY') {
        @data = @{ $data[0] };
    }

    if ($self->{autoinc}) {
        unshift(@data, ++$self->{pkey});
    }

    my $fh = $self->{fh};
    print $fh join("\t", map {&FormatCacheData($_)} @data), "\n";

}  #  End add

###############################################################################
#  commit
###############################################################################
sub commit {
    my($self) = @_;

    my($sql, $sth);

    close($self->{fh});

    if ($self->{verbose}) {
        print "\n\nCOMMITTING CACHE '$self->{table}' TO DATABASE\n"
    }

    $sql = "COPY $self->{table} FROM '$self->{file}'";

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

}  #  End commit

###############################################################################
#  FormatCacheData
###############################################################################
sub FormatCacheData {
    my($data) = @_;
    return '\\N' if !defined($data);

    $data =~ s/([\t\n\\])/\\$1/g;

    return $data;
}  #  End FormatCacheData

###############################################################################
#  SetCacheDir
###############################################################################
sub SetCacheDir {
    my($class, $dir) = @_;

    $gCfg{cachedir} = $dir;
}  #  End SetCacheDir

###############################################################################
#  SetDbHandle
###############################################################################
sub SetDbHandle {
    my($class, $dbh) = @_;

    $gCfg{dbh} = $dbh;
}  #  End SetDbHandle

###############################################################################
#  SetVerbose
###############################################################################
sub SetVerbose {
    my($class, $verbose) = @_;

    $gCfg{verbose} = $verbose;
}  #  End SetVerbose


1;
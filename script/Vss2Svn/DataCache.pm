package Vss2Svn::DataCache;

use warnings;
use strict;

our(%gCfg);

###############################################################################
#  new
###############################################################################
sub new {
    my($class, $table, $autoinc) = @_;

    my $self =
        {
         table => $table,
         autoinc => $autoinc,
         pkey => 0,
         verbose => $gCfg{verbose},
         fh => undef,
         file => "$gCfg{cachedir}\\datachache.$table.tmp.txt",
        };

    $self = bless($self, $class);

    if ($self->{verbose}) {
        print "\nSTARTING CACHE FOR $table\n";
    }

    $self->_delete_table();

    unlink($self->{file});

    if ( !open($self->{fh}, ">$self->{file}") ) {
        print "\nERROR: Could not open file '$self->{file}'\n";
        return undef;
    }

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
        print "\n\nCOMMITTING $self->{table} CACHE TO DATABASE\n"
    }

    $sql = "COPY $self->{table} FROM '$self->{file}'";

    $sth = $gCfg{dbh}->prepare($sql);
    $sth->execute();

    unlink $self->{file} unless $self->{verbose};

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
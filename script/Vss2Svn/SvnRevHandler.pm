package Vss2Svn::SvnRevHandler;

use warnings;
use strict;

our(%gCfg);

$gCfg{revtimerange} = 3600;

###############################################################################
#  new
###############################################################################
sub new {
    my($class) = @_;

    my $svncache = Vss2Svn::DataCache->new('SvnRevision', 1);

    if (!defined($svncache)) {
        print "\nERROR: Could not create cache 'SvnRevision'\n";
        return undef;
    }

    my $self =
        {
         svncache => $svncache,
         revnum => undef,
        };

    $self = bless($self, $class);

    $self->_init();
    return $self;

}  #  End new

###############################################################################
#  _init
###############################################################################
sub _init {
    my($self) = @_;

    $self->{timestamp} = undef;
    $self->{author} = undef;
    $self->{comment} = undef;
    $self->{seen} = {};

}  #  End _init

###############################################################################
#  check
###############################################################################
sub check {
    my($self, $data) = @_;

    my($physname, $itemtype, $actiontype, $timestamp, $author, $comment) =
        @{ $data }{qw( physname itemtype actiontype timestamp author comment )};
    my($prevtimestamp, $prevauthor, $prevcomment) =
        @{ $self }{qw( timestamp author comment )};

    # Any of the following cause a new SVN revision:
    #   * same file touched more than once
    #   * different author or comment
    #   * time range exceeds threshold num. of seconds (default 3600)
    #   * any action on a directory other than add

    my $wasseen = $self->{seen}->{$physname}++;

    no warnings 'uninitialized';
    if(($author ne $prevauthor) || ($comment ne $prevcomment) || $wasseen ||
       ($timestamp - $prevtimestamp > $gCfg{revtimerange}) ||
       ($itemtype == 1 && $actiontype ne 'ADD')) {

        $self->new_revision($data);

        if ($self->{verbose}) {
            print "\n**** NEW SVN REVISION ($self->{revnum}): ",
                join(',', $physname, $timestamp, $author, $comment), "\n";
        }

    }

    @{ $self }{qw( timestamp author comment)} =
        ($timestamp, $author, $comment);

}  #  End check

###############################################################################
#  new_revision
###############################################################################
sub new_revision {
    my($self, $data) = @_;

    $self->{svncache}->add( @{ $data }{qw(timestamp author comment)} );
    $self->{revnum} = $self->{svncache}->{pkey};
    $self->{seen} = {};

}  #  End new_revision

###############################################################################
#  commit
###############################################################################
sub commit {
    my($self) = @_;

    $self->{svncache}->commit();
}  #  End commit

###############################################################################
#  SetRevTimeRange
###############################################################################
sub SetRevTimeRange {
    my($class, $range) = @_;

    $gCfg{revtimerange} = $range;
}  #  End SetRevTimeRange


1;
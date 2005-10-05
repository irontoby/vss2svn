# Copyright 2004 Ed Price.
# 
# This file is part of vss2svn2 (see http://vss2svn2.tigris.org/).
# 
# vss2svn2 is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# vss2svn2 is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with vss2svn2; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

package VssCmd;

our $VERSION = '0.5.6';

# 

use warnings;
use strict;
use Carp;

sub first(&@);
our(%gErrMatch);


# provides access to real VSS repository.
# 
# uses SS.EXE command.

# XXX TODO:
# 
#  * consider "-S" or "-S-" to enable/disable "smart mode".
#  * consider "-NL" (long filename mode)
#  * consider all the crazy "-G" Get (et al) options...
#  * use "-C@file" for multiline comments.

#use VssPath;

# required options:
# 
#  * vss_dir
#  * vss_user
# 
# optional options:
# 
#  * vss_pass
# 
# XXX TODO: add nowarn, nodebug.
# XXX mostly duplicated from VssOle.pm.
sub new
{
    my ($class, %options) = @_;
    
    my $vss_dir = File::Spec->rel2abs($options{vss_dir});
    my $vss_log = File::Spec->rel2abs($options{vss_log});
    
    my $self =
    {
    	vss_cmd  => exists $options{vss_cmd} ? $options{vss_cmd} : "ss", 
        vss_dir  => $vss_dir,
        vss_user => $options{vss_user},
        vss_pass => exists $options{vss_pass} ? $options{vss_pass} : "",
        vss_log  => $vss_log,
		use_tempfiles => 1,
    };
    
    bless $self, $class;
}

# creates an empty repository.
# 
# currently requires VSS admin tools in PATH:
# 
#  * mkss.exe
#  * ddupd.exe
#  * ddconv.exe
# 
# note: if vss_dir (as supplied to VssOle->new) does not exist, it will
# be created.  if it does exist, it MUST BE EMPTY.
# 
# XXX duplicated from VssOle.pm.
sub create
{
    my ($self) = @_;
    croak "repository already open" if defined $self->{vss};
    
    my $vss_cmd     = $self->{vss_cmd};
    my $vss_dir     = $self->{vss_dir};
    my $vss_user    = $self->{vss_user};
    
    # XXX only "admin" seems to work.
    $self->WARN ("VSS user besides 'admin' probably wont work")
        unless lc($vss_user) eq "admin";
    
    # XXX dont know how to set password.
    $self->WARN ("password not supported")
        if $self->{vss_pass};
    
    my $srcsafe_ini = $vss_dir . "/srcsafe.ini";
    my $users_txt   = $vss_dir . "/users.txt";
    my $data_dir    = $vss_dir . "/data";
    my $users_dir   = $vss_dir . "/users";
    my $user_dir    = $vss_dir . "/users/$vss_user";
    my $ss_ini      = $vss_dir . "/users/$vss_user/ss.ini";
    
    # create VSS dir, if necessary
    if (not -e $vss_dir)
    {
        mkdir $vss_dir or croak "error creating VSS dir ($vss_dir): $!";
    }
    # make sure VSS dir is empty
    {
        opendir(DH, $vss_dir) or croak "error opening VSS dir ($vss_dir): $!";
        my @files = grep { $_ ne "." and $_ ne ".." } readdir(DH);
        croak "VSS dir ($vss_dir) not empty" if @files;
        closedir(DH);
    }
    
    # create srcsafe.ini
    open FH, ">$srcsafe_ini" or croak "error opening srcsafe.ini ($srcsafe_ini): $!";
    print FH "Data_path = data\n";
    print FH "Temp_path = temp\n";
    print FH "Users_path = users\n";
    print FH "Users_Txt = users.txt";
    close FH or warn "error closing srcsafe.ini ($srcsafe_ini): $!";
    
    # create users.txt
    open FH, ">$users_txt" or croak "error creating users.txt ($users_txt): $!";
    print FH "$vss_user = users\\$vss_user\\ss.ini\n";
    close FH or warn "error closing users.txt ($users_txt): $!";
    
    # create users dir, and users/<vss_user>
    mkdir $users_dir or croak "error creating users dir ($users_dir): $!";
    mkdir $user_dir or croak "error creating user dir ($user_dir): $!";
    
    # create (empty) "ss.ini" file for vss_user.
    open FH, ">$ss_ini" or croak "error opening user ss.ini ($ss_ini): $!";
    close FH or warn "error closing user ss.ini ($ss_ini): $!";
    
    # create data dir
    mkdir $data_dir or croak "error creating data dir ($data_dir): $!";
    
    # 
    # now populate data dir (the fun part).
    # 
    
    # 1. create empty VSS repository in version 4 format.
    # 
    # "mkss" adds the following files to $data_dir:
    # 
    #  * um.dat (user management)
    #  * aaaaaaaa
    #  * aaaaaaaa.a
    #  * aaaaaaaa.cnt
    
    $self->_exec ("mkss", $data_dir);
    
    # 2. convert to version 5 format.
    # 
    # "ddupd" prints "File um.dat may be corrupt" (why?)
    # and adds a "loggedin" folder.
    
    # XXX that message probably causes non-zero exit status...
    eval { $self->_exec ("ddupd", $data_dir); } or
        warn "ignoring DDUPD.EXE non-zero exit status";
    
    # 3. convert to version 6 format.
    # 
    # "ddconv" adds all the one-letter folders and some other stuff.
    
    $self->_exec ("ddconv", $data_dir);
}

# 
# SS.EXE "subcommand" list (from MSDN):
# 
# XXX TODO: which ones do wildcard matching, -R for recursion, etc.
# 
# command       priority
# -------       --------
# About         -
# Add           1
# Branch        2
# Checkin       1
# Checkout      -
# Cloak         -
# CP            0 (blocks everything)
# Create        1 (mkdir, essentially?  XXX whats diff b/t project and dir??)
# Decloak       -
# Delete        1
# Deploy        -
# Destroy       2
# Difference    -
# Directory     3 ("ls" basically)
# Filetype      3 (gets/sets whether file is Text vs Binary)
# FindinFiles   -
# Get           1
# Help          -
# History       3
# Label         2
# Links         3
# Merge         3
# Move          1
# Password      -
# Paths         3
# Pin           2
# Project       - (shows "CurrentProject")
# Properties    -
# Purge         2
# Recover       2
# Rename        1
# Rollback      3
# Share         2
# Status        -
# Undocheckout  -
# Unpin         2
# View          -
# Whoami        -
# WorkFold      0 (blocks everything)

# XXX in general maybe better to use hash args, eg:
# 
#   recurse => 1,
#   file    => "a.txt",
#   comment => "blah blah", etc.

# ----------
# priority 0
# ----------
# 
# CP            0 (blocks everything)
# WorkFold      0 (blocks everything)
# 
# note: renamed "CP" to "CurrentProject" to prevent confusion wrt
# 'copy'.

sub CurrentProject {
    my ($self, $proj) = @_;
    $self->_ss ("CP", $proj);
}

sub WorkFold {
    my ($self, $path) = @_;
    $path = File::Spec->rel2abs ($path);
    $self->_ss ("WorkFold", $path);
}

# ----------
# priority 1
# ----------
# 
# Add           1
# Checkin       1
# Create        1 (mkdir, essentially?  XXX whats diff b/t project and dir??)
# Delete        1
# Get           1
# Move          1
# Rename        1

# XXX can this add a dir?  or just a file?
sub Add {
    my ($self, $file) = @_;
    $self->_ss ("Add", $file);
}

sub Checkin {
    my ($self, $file) = @_;
    $self->_ss ("Checkin", $file);
}

sub Create {
    my ($self, $dir) = @_;
    $self->_ss ("Create", $dir);
}

sub Delete {
    my ($self, $file) = @_;
    $self->_ss ("Delete", $file);
}

sub Dir {
    my ($self, $file) = @_;
    return $self->_ss ("Dir", $file);
}

sub Get {
    my ($self, $file) = @_;
    $self->_ss ("Get", $file);
}

sub Move {
    my ($self, $src_file, $tgt_dir) = @_;
    $self->_ss ("Move", $src_file, $tgt_dir);
}

sub Rename {
    my ($self, $old_file, $new_file) = @_;
    $self->_ss ("Rename", $old_file, $new_file);
}

# ----------
# priority 2
# ----------
# 
# Branch        2
# Destroy       2
# Label         2
# Pin           2
# Purge         2
# Recover       2
# Share         2
# Unpin         2

# ----------
# priority 3
# ----------
# 
# Directory     3 ("ls" basically)
# Filetype      3 (gets/sets whether file is Text vs Binary)
# History       3
# Links         3
# Merge         3
# Paths         3
# Rollback      3

# ---------------
# everything else
# ---------------
# 
# About         -
# Checkout      -
# Cloak         -
# Decloak       -
# Deploy        -
# Difference    -
# FindinFiles   -
# Help          -
# Password      -
# Project       - (shows "CurrentProject")
# Properties    -
# Status        -
# Undocheckout  -
# View          -
# Whoami        -

# 
# internal methods
# 

sub _ss
{
    my ($self, $verb, @args) = @_;
    
    $ENV{SSDIR} = $self->{vss_dir};
    
    my @other = ();
    {
        push @other, "-I-Y"; # Yes to all prompts; XXX dubious...
        push @other, "-Y" . $self->{vss_user} . "," . $self->{vss_pass};
        push @other, "-O" . $self->{vss_log};
        #push @other, "-W"; # writable working copy; XXX doesnt work with all commands
    }
    
    return $self->_exec ($self->{vss_cmd}, $verb, @args, @other);
}

sub _exec
{
    my ($self, @command) = @_;

    $self->LOG ("EXEC: @command");
    my $ans = system (@command);

    # error-checking based on "perldoc -f system".
    die "error running '@command': $!" if $ans == -1;
    $ans /= 256;
    die "error ($ans) from '@command'" if $ans != 0;
}

# logging

sub ERROR {
    my ($self, $msg) = @_;
    print "# XXX (ERROR) $msg\n";
}

sub WARN {
    my ($self, $msg) = @_;
    print "# XXX (WARNING) $msg\n" unless $self->{nowarn};
}

sub DEBUG {
    my ($self, $msg) = @_;
    print "# $msg\n" unless $self->{nodebug};
}

sub LOG {
    my ($self, $msg) = @_;
    print "# $msg\n";
}


###############################################################################
#  Initialize
###############################################################################
sub Initialize {
    # see ss method for explanation of this
    %gErrMatch = (
                    GET    => 'is not an existing filename or project',
                    CREATE => 'Cannot change project to',
                    CP     => 'Cannot change project to',
                 );
}  # End Initialize

sub first(&@) {
	my $code = shift;
	&$code && return $_ for @_;
	return undef;
}


1;

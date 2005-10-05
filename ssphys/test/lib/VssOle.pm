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

package VssOle;

our $VERSION = '0.5.6';

# 

use warnings;
use strict;
use Carp;

# provides access to real VSS repository.
# 
# uses Win32::OLE currently.

use Win32::OLE;
use Win32::OLE::Const "Microsoft SourceSafe 6.0 Type Library";
eval { defined VSSFLAG_RECURSYES } or die "sanity check failed, VSSFLAG_RECURSYES undefined";

use VssPath;

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
sub new
{
    my ($class, %options) = @_;
    
    my $self =
    {
        vss_dir  => $options{vss_dir},
        vss_user => $options{vss_user},
        vss_pass => $options{vss_pass},
        vss      => undef, # defined by "open"
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

sub create
{
    my ($self) = @_;
    croak "repository already open" if defined $self->{vss};
    
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
    
    system ("mkss $data_dir"); # XXX error-checking?
    
    # 2. convert to version 5 format.
    # 
    # "ddupd" prints "File um.dat may be corrupt" (why?)
    # and adds a "loggedin" folder.
    
    system ("ddupd $data_dir"); # XXX error-checking?
    
    # 3. convert to version 6 format.
    # 
    # "ddconv" adds all the one-letter folders and some other stuff.
    
    system ("ddconv $data_dir"); # XXX error-checking?
}

# opens VSS repository.
sub open
{
    my ($self) = @_;
    
    if (defined$self->{vss})
    {
        croak "already open";
        return;
    }
    
    $self->DEBUG ("initializing VSS...");
    my $vss = Win32::OLE->new ("SourceSafe");
    _oerr();
    
    my $vss_dir  = $self->{vss_dir} . "\\foo"; # XXX weird, but...
    my $vss_user = $self->{vss_user};
    my $vss_pass = $self->{vss_pass};
    
    $self->DEBUG ("opening VSS...");
    $vss->Open ($vss_dir, $vss_user, $vss_pass);
    _oerr();
    
    $self->{vss} = $vss;
}

# retrieves actual file given VssPath string and output file
# XXX just auto-convert string in get_file_from_vss_path instead?
sub get_file_from_vss_path_string
{
    my ($self, $vss_path_string, $output_file) = @_;
    $self->get_file_from_vss_path (VssPath->new($vss_path_string), $output_file);
}

# retrieves actual file given VssPath and output file
sub get_file_from_vss_path
{
    my ($self, $vss_path, $output_file) = @_;
    
    $self->DEBUG("vss path: ". $vss_path->toString);
    $self->DEBUG("output file: $output_file");
    
    my $vss_item = $self->get_item_from_vss_path ($vss_path);
    
    $vss_item->Get($output_file);
    _oerr();
}

# returns VSSItem (OLE object) given simple VSS path.
# (wrapper on VSS OLE "VSSItem" method.)
sub OLE_VSSItem
{
    my ($self, $simple_path_string) = @_;
    my $ans = $self->{vss}->VSSItem ($simple_path_string);
    _oerr();
    # XXX check for undef?
    return $ans;
}

# -------------------
# VssPath <-> VSSItem
# -------------------

# returns VssPath for given VSSItem (OLE object)
sub get_vss_path_from_item
{
    my ($self, $item) = @_;
    
    my $name    = $item->{Name};
    my $version = $item->{VersionNumber};
    my $parent  = $item->{Parent};
    _oerr();
    
    $self->DEBUG ("get_vss_path_from_item: v$version of $name");
    
    # stop recursion at root.
    
    if ($name eq "")
    {
        $self->ERROR ("root item name not empty: $name") unless $item->{Spec} eq '$/';
        _owarn();
        return VssPath->new->push("", $version);
    }
    
    # otherwise, recurse.
    
    my $ans = $self->get_vss_path_from_item ($parent);
    $ans->push($name, $version);
    return $ans;
}

# returns VSSItem (OLE object) for given VssPath
sub get_item_from_vss_path
{
    my ($self, $vss_path) = @_;
    # XXX if $vss_path is a String instead of a VssPath, it might be nice
    # to autoinstantiate a VssPath.  what's the std way to do that in perl?
    
    my $item = undef;
    foreach ($vss_path->toArray)
    {
        my ($name, $version) = @$_; # each elt is [name, version]
        
        # top of path must be root
        if (not defined $item)
        {
            die "vss_path root name not empty: $name" unless $name eq "";
            
            my $vss = $self->{vss};
            confess "no vss (need VssOle 'open' maybe?)" unless defined $vss;
            
            $self->DEBUG ("getting vss_path root: no version");
            my $root = $vss->VSSItem("/");
            _oerr();
            confess "error getting VSS root" unless defined $root;
            
            $self->DEBUG ("getting vss_path root: v$version");
            $item = $root->Version($version);
            _oerr();
            confess "error getting VSS root v$version" unless defined $item;
        }
        else
        {
            $self->DEBUG ("getting vss_path: v$version of $name");
            my $sub_item_hash = $self->get_sub_item_name_hash_from_item ($item);
            if (exists $sub_item_hash->{$name})
            {
                $item = $sub_item_hash->{$name};
                $item = $item->Version ($version);
                _oerr();
                if (not defined $item)
                {
                    $self->ERROR ("error getting sub-item (by version): v$version of '$name' from " . $vss_path->toString);
                    return undef;
                }
            }
            else
            {
                $self->ERROR ("error getting sub-item: '$name' from " . $vss_path->toString);
                return undef;
            }
        }
    }
    return $item;
}

# ---------
# VSS stuff
# ---------

# returns list of VSSVersion (OLE objects) given VSSItem (OLE object)
sub get_versions_from_item
{
    my ($self, $item) = @_;
    
    # NB: VSS_FLAG_HISTIGNOREFILES means to exclude file
    # checkin history from PROJECT (ie directory) versions
    # enumeration.  it seems to have no effect on versions
    # enumeration on FILE items.
    
    my $flags = VSSFLAG_RECURSNO | VSSFLAG_HISTIGNOREFILES;
    
    my @ans = $self->_get_enum_list ($item->Versions($flags));
    
    return @ans if @ans;
    
    # XXX this can fail (with unpatched SSAPI.DLL...) due
    # to 0x80020009 OLE error on rename of component of
    # path "above" the item (ie parent dir, grandparent
    # dir, etc.)
    # 
    # one could attempt to workaround that by using the
    # new name...
    # 
    # but it happens with deletes too, where there is
    # there is no "new name" to use instead :(
    
    carp "get_versions_from_item problem...";
    $self->ERROR ("get_versions_from_item problem...");
    
    return ();
}

# returns VSSItem (OLE object) corresponding to given VSSVersion (OLE object).
sub get_item_from_version
{
    my ($self, $version) = @_;
    return $version->{VSSItem};
}

# returns list of VSSItem (OLE objects) which are "links" of the given item,
# ie "shared" nodes.  XXX DOES NOT WORK DUE TO OLE BUGS.
sub get_links_from_item
{
    my ($self, $item) = @_;
    return $self->_get_enum_list ($item->Links);
}

# returns list of VSSItem (OLE objects) given VSSItem (OLE object).
# the returned list consists of all the "sub-items" of the given item.
# ie, for a directory, returns all the directory entries.
# (returns empty list for a file.)
# XXX appears to return empty list on error too; thats probably wrong...
sub get_sub_items_from_item
{
    my ($self, $item) = @_;
    return () if $item->{Type} != 0;
    
    my $enum = $item->Items;
    
    # NOTE: i thought $items->Items(1) would be better because
    # i thought that would make "IncludeDeleted" flag true,
    # but actually, it didn't work that way.  adding "(1)"
    # resulted in deleted items NOT being listed.
    
    return () if not defined $enum;
    return $self->_get_enum_list ($enum);
}

# takes a single VSSItem (OLE object) argument.
# 
# returns a hashref mapping item "name" to item
# (VSSItem OLE object) for all sub-items of the
# given item, ie:
# 
#  { String sub_item_name => VSSItem sub_item }
# 
sub get_sub_item_name_hash_from_item
{
    my ($self, $item) = @_;
    my @items = $self->get_sub_items_from_item ($item);
    my $ans = {};
    foreach (@items)
    {
        my $name = $_->{Name};
        
        if (exists $ans->{$name})
        {
            # XXX Should Not Happen.  but some full
            # repository testing by dirk did find them!
            # his examples had one normal version number
            # and one crazy (> 40,000,000) version number.
            # so we'll try picking the lowest version...
            
            my $prev_item = $ans->{$name};
            my $prev_version = $prev_item->{VersionNumber};
            
            my $dupe_item = $_;
            my $dupe_version = $dupe_item->{VersionNumber};
            
            _oerr ();
            
            if ($dupe_version < $prev_version)
            {
                $ans->{$name} = $dupe_item;
                $self->WARN ("replacing duplicate sub-item (lower version): v$prev_version -> v$dupe_version of $name");
            }
            elsif ($dupe_version == $prev_version)
            {
                $self->WARN ("ignoring duplicate sub-item name (same version): v$dupe_version of $name");
            }
            else
            {
                $self->WARN ("ignoring duplicate sub-item name (higher version): v$dupe_version (vs v$prev_version) of $name");
            }
        }
        else
        {
            $ans->{$name} = $_;
        }
    }
    return $ans;
}

# takes two VSSItem (OLE object) arguments.  the first is the "current
# item" and the second is the "previous item" (intended for an item
# and a previous version of the same item).
# 
# returns a two-element list.
# 
# the first element is an arrayref of ADDED items (VSSItem OLE objects).
# the second elemnt is an arrayref of DELETED items (VSSItem OLE objects).
sub diff_sub_items
{
    my ($self, $curr_item, $prev_item) = @_;
    
    my $curr_sub_items = $self->get_sub_item_name_hash_from_item ($curr_item);
    my $prev_sub_items = $self->get_sub_item_name_hash_from_item ($prev_item);
    
    my @adds;
    my @dels;
    
    # what exists in CURR but not in PREV?  these were ADDED.
    
    foreach (keys %$curr_sub_items)
    {
        push @adds, $curr_sub_items->{$_} if not exists $prev_sub_items->{$_};
    }
    
    # what exists in PREV but not in CURR?  these were DELETED.
    
    foreach (keys %$prev_sub_items)
    {
        push @dels, $prev_sub_items->{$_} if not exists $curr_sub_items->{$_};
    }
    
    # return the adds and deletes.
    
    return (\@adds, \@dels);
}

# 
# OLE enumerations and dates.
# 

sub _get_enum_list
{
    my ($self, $enum_property) = @_;
    Carp::confess ("undefined enum_property") unless $enum_property;
    my $enum = Win32::OLE::Enum->new($enum_property);
    _owarn ();
    return () unless $enum; # XXX ignoring 0x80020009 OLE exception
    
    # NOTE: it seems that one MUST loop through the entire
    # enumeration (done by ->All), and one MUST NOT call "Reset".
    # 
    # otherwise the next attempt to use a (the same? or any?)
    # VSSVersions object will result in an OLE error ("history
    # operation is already in progress").
    
    my @ans = $enum->All;
    _oerr ();
    
    return @ans;
}

# given Win32::OLE::Variant date, returns "YYYY-MM-DD HH:MI:SS".
sub _get_string_from_variant_date
{
    my ($self, $ole_variant_date) = @_;
    # XXX check ISA Win32::OLE::Variant or something?
    my $ymd = $ole_variant_date->Date("yyyy-MM-dd");
    my $hms = $ole_variant_date->Time("HH:mm:ss");
    _oerr();
    # XXX can we get timezone??
    return "$ymd $hms";
}

# 
# OLE error checking
# 

sub _oerr {
    my ($self) = @_;
    my $error = Win32::OLE->LastError();
    if ($error) { Carp::confess($error); }
    1;
}

sub _owarn {
    my ($self) = @_;
    my $error = Win32::OLE->LastError();
    if ($error)
    {
        $self->WARN ("OLE WARNING: $error");
        Carp::carp($error) unless $self->{nodebug};
    }
    1;
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

1;

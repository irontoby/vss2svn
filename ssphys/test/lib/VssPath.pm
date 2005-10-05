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

package VssPath;

our $VERSION = '0.5.6';

=head1 NAME

VssPath - unambiguous specification of a version of a VSS item.

=head1 DESCRIPTION

A VssPath is like a regular "path" except that each component of the
path has a version number.  So it's a "versioned path", consisting of
a list of (Version, Name) pairs.

A VssPath unambiguously identifies a unique version of a "node" in a
VSS repository.

=head2 Constraints

 * Version is a positive integer.
 * Name is a string (without any "/" characters).

Addtionally, although it's not currently enforced here, the following
appear generally true of VSS:

 * root node has empty Name (zero-length "").
 * non-root node must have non-empty Name.
 * only the last path component can have version 1.

=head2 Textual representation

 * each path "component" represented as "<version>:<name>".
 * multiple components separated by slash ("/").

Examples:

 * root          => 1:
 * toplevel dir  => 2:/1:foo
 * toplevel file => 2:/1:foo

=head2 Note on uniqueness

Although a VssPath is unambiguous, it is I<not> unique.  There can be
(and generally are) multiple VssPaths which identify the same version
of the same VSS item.

For example, consider the following history:

                  1:
  mkdir foo       2:/1:foo
  mkdir bar       3:/1:bar

Note that there is only one version of "/foo", but it can be accessed
through both v2 and v3 of the root.

So the following are equivalent:

  2:/1:foo
  3:/1:foo

=cut

use warnings;
use strict;
use Carp;

# 

sub new
{
    my ($class, $string) = @_;
    my $self = defined $string ? _parse_string ($string) : [];
    bless $self, $class;
}

sub copy
{
    my ($self) = @_;
    return VssPath->new($self->toString);
}

sub push
{
    my ($self, $name, $version) = @_;
    _validate ($name, $version);
    push @$self, [$name, $version];
    return $self;
}

# returns [name, version] of the "end" of the path, and removes it from the path.
# returns undef (???) if path is root. (huh? doesnt root have name and version??)

sub pop
{
    my ($self) = @_;
    pop @$self;
}

# returns textual representation

sub toString
{
    my ($self) = @_;
    my $ans;
    foreach (@$self)
    {
        my ($name, $version) = @$_;
        $ans .= "/" if defined $ans;
        $ans .= "$version:$name";
    }
    return $ans;
}

# returns "normal" unversioned path (as string).

sub toSimplePath
{
    my ($self) = @_;
    my $ans;
    foreach (@$self)
    {
        my ($name, $version) = @$_;
        $ans .= "/" if defined $ans;
        $ans .= $name;
    }
    return $ans;
}

# parses textual representation

sub fromString
{
    my ($class, $string) = @_;
    bless _parse_string ($string), $class;
}

# returns array of [name, version] pairs

sub toArray
{
    my ($self) = @_;
    return @{$self->copy};
}

# return 'depth' of path.  root is considered depth zero, "/foo" is 1, etc.
# 
# XXX TODO tests!

sub depth
{
    my ($self) = @_;
    return scalar @$self;
}

# this returns true if self matches a PREFIX of another given VssPath.
# "matching" means that names are equal, and version numbers in self
# are GREATER THAN OR EQUAL to those in the other VssPath.
# 
# examples:
# 
# self      other     matchesPrefix?
# --------  --------  ------------------------------------------
# 1:        1:        yes
# 2:        1:        yes
# 1:        2:        no   (version is less than other)
# 1:/1:foo  1:        no   (cant match prefix of shorter path)
# 1:        1:/1:foo  yes  (1: matches 1:)
# 2:        1:/1:foo  yes  (2: matches 1:)
# 1:        2:/1:foo  no   (1: does not match 2:)
# 1:/1:foo  1:/1:foo  yes  (complete match)
# 2:/2:foo  1:/1:foo  yes  (complete match)
# 1:/1:foo  1:/2:foo  no   (1:foo version less than 2:foo)

sub matchesPrefix
{
    my ($self, $other) = @_;
    
    my @self_stack = @$self;
    my @other_stack = @$other;
    
    # longer path cant match prefix of shorter path.
    return 0 if $#self_stack > $#other_stack;
    
    for (my $i = 0; $i <= $#self_stack; $i++)
    {
        my ($self_name,  $self_version)  = @{$self_stack[$i]};
        my ($other_name, $other_version) = @{$other_stack[$i]};
        
        return 0 if $self_name ne $other_name; # XXX case sensitivity?
        return 0 if $self_version < $other_version;
    }
    
    return 1;
}

# 
# INTERNAL METHODS
# 

sub _parse_string
{
    my $string = shift;
    croak "bad input (trailing slash): $string" if $string =~ m|/$|;
    my @pieces = split /\//, $string;
    croak "bad input (no pieces?!): $string" unless @pieces;
    my $ans = [];
    for (@pieces)
    {
        croak "bad input (piece): $_ of $string" unless m|(\d+):(.*)|;
        my ($version, $name) = ($1, $2);
        _validate ($name, $version);
        CORE::push @$ans, [$name, $version];
    }
    return $ans;
}

sub _validate
{
    my ($name, $version) = @_;
    _validate_name ($name);
    _validate_version ($version);
}

sub _validate_name
{
    my $name = shift;
    croak "slash ('/') in name: $name" if $name =~ m|/|;
}

sub _validate_version
{
    my $version = shift;
    croak "non-decimal version: $version" unless $version =~ m|^\d+$|;
    croak "non-integer version: $version" unless $version eq int $version;
    croak "non-positive version: $version" unless $version > 0;
}

1;

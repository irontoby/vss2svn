use Test::More 'no_plan';
use strict;
use File::Temp qw(tempdir);
use VssCmd;
use Vss2Svn::VSS;

# XXX only runs on windows (ss.exe)

# 
# XXX everything from here ...
my $temp_dir = tempdir("tmpvssXXXXXX", CLEANUP => 0);
$temp_dir = File::Spec->rel2abs($temp_dir);
my $vss_log  = $temp_dir . "\\vsscmd.log";
my $work_dir = $temp_dir . "\\vss_wc";
my $vss_dir  = $temp_dir . "\\vss_repos";
my $vss_proj = '$/';
my $vss_user = "admin";
mkdir $work_dir or die "error on 'mkdir $work_dir': $!";
my $vss = VssCmd->new (vss_dir => $vss_dir, vss_user => $vss_user, vss_log => $vss_log);
$vss->create();
$vss->CurrentProject ($vss_proj);
$vss->WorkFold ($work_dir);

our ($TREE, $VSS);
&Vss2Svn::VSS::Initialize;
my $vss_args = {
#                    interactive => 'Y',
#                    timebias    => $gCfg{timebias},
					user		=> $vss_user,
#					passwd		=> 'admin',
					executable	=> "ssrep",
					silent      => 0,
					_debug      => 1,
                   };

$VSS = Vss2Svn::VSS->new($vss_dir, $vss_proj, $vss_args);

# XXX ... to here, could/should be wrapped up in some kinda library subroutine.
# 

my $orig_cwd = Cwd->cwd();
chdir $work_dir or die "chdir $work_dir: $!";
# allow tempdir CLEANUP to work
END { chdir $orig_cwd; }

# END OF CUTANDPASTE...

sub diff_items
{
    my ($self, $curr_items, $prev_items) = @_;
    
    my @adds;
    my @dels;
    
    # what exists in CURR but not in PREV?  these were ADDED.
    
    foreach (keys %$curr_items)
    {
        push @adds, $curr_items->{$_} if not exists $prev_items->{$_};
    }
    
    # what exists in PREV but not in CURR?  these were DELETED.
    
    foreach (keys %$prev_items)
    {
        push @dels, $prev_items->{$_} if not exists $curr_items->{$_};
    }
    
    # return the adds and deletes.
    
    return (\@adds, \@dels);
}
 
sub checkListing
{
    my ($self, $curr_items, $prev_items) = @_;
    
print ("curr: $curr_items\n");
print ("prev: $prev_items\n");
    my ($add_aref, $del_aref) = diff_items ($curr_items, $prev_items);
    
    my @adds = @$add_aref;
    my @dels = @$del_aref;
    
    # for ok, we want no euality of both lists (no addition, no deletion)
    ok (1) if $#dels == 0 and $#adds == 0;
    
    if ($#adds < 0) { fail ("failed, no added item."); }
    if ($#dels < 0) { fail ("failed, no deleted item."); }
    if ($#adds > 0) { fail ("failed, more than one added item."); }
    if ($#dels > 0) { fail ("failed, more than one deleted item."); }
    
    return undef;
}  

sub assert_project
{
    my ($listing, $project) = @_;
    if ($listing->{$project} eq undef) {
        fail ("$project not found in listing");
    } elsif (ref ($listing->{$project}) ne 'HASH') {
        fail ("$project is not a project");
    }
}

sub assert_file
{
    my ($listing, $file) = @_;
    if ($listing->{$file} eq undef) {
	fail ("$file not found in listing");
    } elsif ($listing->{$file} ne 1) {
	fail ("$file is not a file");
    }
}

my @sequence = ( ['add', 'test1'],
                 ['add', 'test2'],
                 ['create',  'project1'],
		 ['cp', 'project1'],
                 ['add',  'test1'],
                 ['add',  'test2'],
             );

sub createFile 
{
    my ($filename) = @_;
    # create empty file
    open(FH,">$work_dir\\$filename") or die "error creating file '$filename'";
    close FH;
}

sub createProject 
{
    my ($filename) = @_;
    mkdir ("$work_dir\\$filename") or die "error creating directory '$filename'";
}

my $filename;

$filename = 'test1';
createFile ($filename);
$vss->Add($filename);

$filename = 'test2';
createFile ($filename);
$vss->Add($filename);

$filename = 'project1';
createProject ($filename);
$vss->Create($filename);

$filename = 'project1\\test3';
createFile ($filename);
$vss->Add($filename);

$filename = 'project1\\test4';
createFile ($filename);
$vss->Add($filename);

$filename = 'project2';
createProject ($filename);
$vss->Create($filename);

$filename = 'project2\\test5';
createFile ($filename);
$vss->Add($filename);

$filename = 'project2\\test6';
createFile ($filename);
$vss->Add($filename);



my $listing;

# directory list of tip revision
$listing = $VSS->project_tree("",0)
 or fail ("dir \"\"");

$listing = $VSS->project_tree("\$/",0)
  or fail ("dir \$/");

$listing = $VSS->project_tree("\$/;4/project1",0)
  or fail ("dir \$/;4/project1");
ok (1, "dir");


sub dumpListing
{
    my ($listing) = @_;
    foreach my $key (sort keys %$listing) {
	my $val = $listing->{$key};
	    print ("$key => $val\n");
    }
}

# compare specific versions
$listing = $VSS->project_tree("$/;1",0);
#checkListing ($listing, ());
$listing = $VSS->project_tree("$/;2",0);
#checkListing ($listing, ("test1"));
$listing = $VSS->project_tree("$/;3",0);
#checkListing ($listing, ("test1", "test2"));
$listing = $VSS->project_tree("$/;4",0);
#checkListing ($listing, ("test1", "test2", "\$project1"));
assert_file ($listing, "test1");
assert_file ($listing, "test2");
assert_project ($listing, "project1");




# relative directory
# VSS.pm can not handle relative directories
#$TREE = $VSS->project_tree("\$",0)
#$TREE = $VSS->project_tree(";1",0)
#$TREE = $VSS->project_tree("\$;1/",0)
#$TREE = $VSS->project_tree("\$;4/project1",0);

# set cp to subdir
$vss->CurrentProject ("\$/project1");
#$TREE = $VSS->project_tree("",0)
#$TREE = $VSS->project_tree(".",0)
$TREE = $VSS->project_tree("\$/",0)
	or fail ("dir \$/");

ok (1, "dir");

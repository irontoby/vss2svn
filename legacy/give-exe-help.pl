###############################################################################
#  GiveExeHelp
###############################################################################
sub GiveExeHelp {
    my($msg, $verbose) = @_;
    
    $msg .= "\n" if $msg;
    
DATA_LINE:
    while (<DATA>) {
        last DATA_LINE if (m/^OPTIONS/ && !$verbose);
        $msg .= $_;
    }
    
    close DATA;
    
    if ($verbose) {
        print STDOUT "$msg\n";
        exit(0);
    }
    
    die "$msg\n";
}  #  End GiveExeHelp

1;

__END__

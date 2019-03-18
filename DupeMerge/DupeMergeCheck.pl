# Y:\Data\cpp\hardlinks\DupeMerge\bugs

use MD5;
use Getopt::Long;

sub ShowHelp
{
	&ShowVersion();
	print "   -l Set Directory, which should be examined.\n";
	print "   -o Set Output File\n";
	print "   -h This Help File\n";
	exit 1;
}

GetOptions( 'logfile=s', \$LogFile,
            'output=s', \$OutFile,
            'help=s', \$Help
);

&ShowHelp() if defined $opt_h;

if ( ! -s $LogFile )
{
  print "fatal error: -LogFile: <$LogFile> not existing\n";
  exit;
}

open (OUTPUT, ">".$OutFile) if defined $OutFile;


open (LOG, "<$LogFile");

my $block = 1;
my $DupeGroups = 0;

while (<LOG>)
{
   chop $_;
   $line = $_;
   
   if ($line =~ m/^'/ )
   {
      $line =~ s/'(.*)'.*/\1/;
   
#      $line =~ s/d:/y:/i;
      
      #
      # Get the MD5 fingerprint
      #
      my $md5 = new MD5;
      $md5->reset;
      
      if ( -s $line ) {
        open (MD5FILE, $line) or 

        binmode (MD5FILE);
        $md5->addfile(MD5FILE);
        close (MD5FILE);

        if ($block == 0)
        {
   	  $block = 1;
	  $blockfile = $line;
	  $blockdigest = $md5->hexdigest;

	  print OUTPUT $blockfile."\n";;
        }
        else
        {
	  # print $line."\n";
	  #
	  # Get the MD5 fingerprint
	  #
	  $digest = $md5->hexdigest;
	  if ($digest ne $blockdigest)
	  {
	     print OUTPUT "### Error $line\n";
	  }
	}
      }
      else
      {
	print OUTPUT "File '$line' not found \n";
      }
   }
   else
   {
      $block = 0;
      $DupeGroups++;
      print OUTPUT "-----------------\n";
   }
}

print OUTPUT "$DupeGroups found.\n";

close (LOG);


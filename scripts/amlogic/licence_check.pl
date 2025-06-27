#!/usr/bin/perl -W
# SPDX-License-Identifier: (GPL-2.0+ OR MIT)
#
# Copyright (c) 2019 Amlogic, Inc. All rights reserved.
#
use File::Basename;
use File::Find;

$sc_dir = File::Spec->rel2abs(dirname( "$0") ) ;
$sc_dir =~ s/\/scripts\/amlogic//;
my $top = "$sc_dir";

my $license_flag = 0;
my $copyright_flag = 0;

my $amlogic_flag = 0;

my $date_white_flag = 0;

my $nofix = 0;
my $skip_whitelist = 0;
my $with_dir = 0;

my $failno = 0;
my $shname = $0;
#@ARGV=("../../include/linux/amlogic","../../drivers/amlogic" ) ;

sub show_usage()
{
	my $usage=
	"
	usage: ./scripts/amlogic/licence_check.pl [-n | --nofix] [-s | --skip_whitelist] [-h | --help]  <dir path/file path>

	-n, --nofix            只提示不自动替换正确license

	-s, --skip_whitelist   禁用白名单


	案例:
	1. 扫描目录/文件, 并自动替换正确license
	./scripts/amlogic/licence_check.pl  drivers/amfc/amfc.c
	./scripts/amlogic/licence_check.pl  drivers/amfc/

	2. 扫描目录/文件, 只提示不自动替换正确license
	./scripts/amlogic/licence_check.pl --nofix drivers/amfc/amfc.c

	3. 扫描目录/文件, 禁用白名单, 自动替换正确license
	./scripts/amlogic/licence_check.pl --skip_whitelist drivers/amfc/

	4. 扫描目录/文件, 禁用白名单, 只提示不自动替换正确license
	./scripts/amlogic/licence_check.pl --skip_whitelist --nofix drivers/amfc/
	 \n\n";
	print "$usage";
}

my @path;
for(@ARGV)
{
	my $dir	=$_;
	if(/^\//)
	{
	}
	elsif(/--nofix|-n/)
	{
		#print "nofix\n";
		$nofix = 1;
		next;
	}
	elsif(/--skip_whitelist|-s/)
	{
		#print "skip_whitelist\n";
		$skip_whitelist = 1;
		next;
	}
	elsif(/--help|-h/)
	{
		show_usage();
		exit;
	}
	else
	{
		$with_dir = 1;
		$dir = File::Spec->rel2abs($dir);
		#print "\n Real2abs Dir: --$dir-- \n";
	}
	push(@path,$dir);
}

if(!$with_dir) {
	show_usage();
	exit;
}

my $licence_exp="WITH Linux-syscall-note";

my $licence_start="// SPDX-License-Identifier: (GPL-2.0+ OR MIT)\n";
my $licence_end=
"/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */\n\n";

my $date_white="2010|2011|2012|2013|2014|2015|2016|2017|2018|2019|2020|2021|2022|2023|2024|2025|2026|2027|2028|2029|2030|2031|2032|2033|2034|2035|2036|2037|2038|2039|2040";

my $licence_lines = 0;

my $licence_result = 0;

sub license_copyright_check
{
	my ($line) = @_;
	#print "$line\n";
	if ($line =~ /License|license|LICENSE/) {
		#print "test License\n";
		$license_flag = 1;
	}

	if ($line =~ /Copyright|copyright|COPYRIGHT/) {
		#print "test Copyright\n";
		$copyright_flag = 1;
	}

	#print "copyright_flag 1 $copyright_flag\n";
	#print "license_flag 1 $license_flag\n";
	#if ($line =~ /^.*\s*Copyright \(c\)\s+(\d{4})\s+Amlogic, Inc\. All rights reserved\.$/ && $copyright_flag && $license_flag) {
	if (($line =~ /^ \* Copyright \(c\)\s+(\d{4})\s+Amlogic, Inc\. All rights reserved\.$/) ||
		($line =~ /^.*\s*Copyright\s+\(c\)\s+\d{4}-\d{4}\s+Amlogic,\s+Inc\.\s+All\s+rights\s+reserved\.\s*$/)) {
		#print "--------test1--------\n";
		$amlogic_flag = 1;
		if($line !~ $date_white){
			$copyright_flag = 0;
			$license_flag = 0;
			$date_white_flag = 0;
		}
		else
		{
			$date_white_flag = 1;
		}
	}
	else
	{
		#print "--------test2--------\n";
		if ($licence_lines =~ 1)
		{
			if ($line ne $licence_start)
			{
				$license_flag = 0;
			}
			if ($line =~ $licence_exp)
			{
				$license_flag = 1;
			}
		}
		elsif ($licence_lines =~ 2)
		{
			if ($line ne "/*\n")
			{
				$license_flag = 0;
			}
		}
		elsif ($licence_lines =~ 3)
		{
			$amlogic_flag = 0;
		}
	}
}

my $whitelist_file = "scripts/amlogic/license_whitelist.txt";

sub license_white_check
{
	my ($file_name) = @_;
	#print "\n -- $file_name -- \n";
	my $abs_whitelist_file = "$top/$whitelist_file";
	#print "\n Real2abs abs_whitelist_file: --$abs_whitelist_file-- \n";
	open(my $f_in_whitelist, '<', $abs_whitelist_file) or die "Can't Open $abs_whitelist_file: For Read !!! \n";
	while ($line = <$f_in_whitelist>)
	{
		chomp $line;
		#print "\n -- $line -- \n";
		if($file_name =~ $line)
		{
			#print "\n file_name match line -- \n";
			close($f_in_whitelist);
			return 1;
		}
	}
	close($f_in_whitelist);
	return 0;
}

sub licence_process
{
	my ($file_name) = @_;
	#print "\n file_name: $file_name  \n";
	my $d = dirname($file_name);
	my $f = basename($file_name);
	#print "\n Abs <$d>,  f_ $f";
	#print "\n Top: <$top>  \n";

	$license_flag = 0;
	$copyright_flag = 0;
	$licence_lines = 0;
	$amlogic_flag = 0;
	$date_white_flag = 0;

	$licence_0=$licence_start.$licence_end;
	my $count = 0;
	my $text_0="";
	my $text_all=$licence_0;

	if(!$skip_whitelist && license_white_check($file_name))
	{
		return 1;
	}

	open(my $f_in, '<', $file_name) or die "Can't Open $file_name: For Read \n";
	my ($left,$right, $lineno,$space) = (0, 0, 0,0);
	while ($line = <$f_in>)
	{
		$text_0 .= $line;
		#Empty Line or Line marked by //
		if(($space==0) &&(($line =~/^\s*$/)||
		(($line =~/^\s*\/\//)&&($line !~ /\*\//))))
		{
			#print "\n Line $lineno is empty.";
			#print "$line\n";
			$licence_lines++;
			license_copyright_check($line);
		}
		elsif(($space==0) &&($line =~ /^\s*\/\*/))											#Match /*
		{
			$left ++;
			#print "\n L Matched: $lineno  $line, $left  ";								#Match that /* and */ in the same line
			$licence_lines++;
			license_copyright_check($line);
			if($line =~ /\*\//)
			{
				$right ++;
				#print "\n L Matched: $lineno  $line, $left  ";
				license_copyright_check($line);
			}
		}
		elsif(($space==0) &&($line =~ /\*\//)&& ($line !~ /\/\*/) )													#Match */
		{
			$right ++;
			#print "\n R Matched: $lineno  $line, $right  ";
			$licence_lines++;
			license_copyright_check($line);
			if($left == $right)
			{
				$space = 1;
			}
		}
		elsif($left==$right)	#Content Lines
		{
			if(($line =~/^\s*$/)&& ($count==0))
			{
				license_copyright_check($line);
			}
			else
			{
				#print $line;
				$space = 1;
				$count +=1;
				$text_all .=$line;
			}
		}
		else
		{
			#print "$line";
			$licence_lines++;
			license_copyright_check($line);
		}
		$lineno++;
	}
	close($f_in);
	#print "license_flag 1 = $license_flag\n";
	#print "licence_lines = $licence_lines\n";
	#print "amlogic_flag = $amlogic_flag\n";
	#print "date_white_flag = $date_white_flag\n";

	#print "$text_0\n";
	#print "$text_all\n";
	#if (!$licence_white_flag && $licence_lines =~ 4 && !$date_white_flag)
	if ($licence_lines =~ 4 && !$date_white_flag)
	{
		#print "-1\n";
		$license_flag = 0;
		$copyright_flag = 0;
	}

	if ($amlogic_flag && $licence_lines !~ 4)
	{
		#print "-2\n";
		$license_flag = 0;
		$copyright_flag = 0;
	}

	if ($amlogic_flag && $licence_lines =~ 4 && !$date_white_flag)
	{
		#print "-3\n";
		$license_flag = 0;
		$copyright_flag = 0;
	}

	if (!$amlogic_flag && $licence_lines =~ 4)
	{
		#print "-4\n";
		$license_flag = 0;
		$copyright_flag = 0;
	}
	#print "license_flag = $license_flag\n";
	#print "copyright_flag = $copyright_flag\n";

	if (!$license_flag || !$copyright_flag)
	{
		if($text_0 ne $text_all)
		{
			$failno ++;
			if($nofix)
			{
				print "\n  Licence_WARN: <";
				print File::Spec->abs2rel($file_name, $top).">\n";;
				$licence_result = 1;
			}
			else
			{
				print "\n  Licence_FIXED: <";
				print File::Spec->abs2rel($file_name, $top).">\n";;
				open(my $f_out, '>', $file_name)
				or die "Can't Open $file_name\n";
				print $f_out $text_all;
				close $f_out;
				$licence_result = 1;
			}
		}
		$text_all='';
	}
}
my ($c_cnt, $h_cnt) = (0, 0);
sub process
{
    my $file = $File::Find::name;
    if (-f $file)
     {
		if(($file =~ /.*\.[Cc]$/i) || ($file =~ /.*\.dtsi$/i) || ($file =~ /.*\.dts$/i))
		{
			$c_cnt++;
			$licence_start="// SPDX-License-Identifier: (GPL-2.0+ OR MIT)\n";
			licence_process($file);
		}
		if(($file =~ /.*\.[hH]$/i))
		{
			$c_cnt++;
			$licence_start="/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */\n";
			licence_process($file);
		}
	}
}

for(@path)
{
	#print "\n Fine $_ \n";
	unless (-e $_) {
		print "\n        $_ : No such file or directory\n";
		show_usage();
		exit;
	}
	find(\&process, $_);
}

if(!$licence_result)
{
	print "\n  Licence Check OK\n\n"
}

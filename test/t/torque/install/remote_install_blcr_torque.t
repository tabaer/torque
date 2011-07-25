#! /usr/bin/perl 

use strict;
use warnings;

use FindBin;
use TestLibFinder;
use lib test_lib_loc();

use CRI::Test;
use CRI::Util qw(
                   list2array
                 );

plan('no_plan');
setDesc('Install pbs_mom on Remote Nodes with BLCR');

# Variables
my $nodes_str      = $props->get_property('Torque.Remote.Nodes');
my @nodes          = list2array($nodes_str);
my $props_loc      = "$FindBin::Bin/../../../etc/props/default.props";
my $pbs_server     = $props->get_property('Test.Host');
my $pbs_server_loc = $props->get_property('Torque.Home.Dir') . "/server_name";

pass("No remote nodes given")
  if ! scalar @nodes;

foreach my $node (@nodes)
  {

  diag("Setting up torque on '$node'");

  my %ssh;

  # Set up the props file on the remote node
  my $cp_cmd = "cp $props_loc " . $props->get_property( 'Data.Props.Loc' );
  %ssh       = runCommandSsh($node, $cp_cmd);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$cp_cmd'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  # Install torque
  my $remote_install_bat = "$FindBin::Bin/../remote_reinstall_blcr.bat";
  %ssh                   = runCommandSsh($node, $remote_install_bat);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$remote_install_bat'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  # Set the pbs_mom server to trust the pbs_server
  my $cmd = "echo '$pbs_server' > $pbs_server_loc";
  %ssh    = runCommandSsh($node, $cmd);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$cmd'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  } # END foreach my $node (@nodes)

#! /usr/bin/perl 

use strict;
use warnings;

use FindBin;
use TestLibFinder;
use lib test_lib_loc();

use CRI::Test;
use CRI::Util qw(
                   list2array
                 );

plan('no_plan');
setDesc('Install pbs_mom on Remote Nodes with BLCR');

# Variables
my $nodes_str      = $props->get_property('Torque.Remote.Nodes');
my @nodes          = list2array($nodes_str);
my $props_loc      = "$FindBin::Bin/../../../etc/props/default.props";
my $pbs_server     = $props->get_property('Test.Host');
my $pbs_server_loc = $props->get_property('Torque.Home.Dir') . "/server_name";

pass("No remote nodes given")
  if ! scalar @nodes;

foreach my $node (@nodes)
  {

  diag("Setting up torque on '$node'");

  my %ssh;

  # Set up the props file on the remote node
  my $cp_cmd = "cp $props_loc " . $props->get_property( 'Data.Props.Loc' );
  %ssh       = runCommandSsh($node, $cp_cmd);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$cp_cmd'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  # Install torque
  my $remote_install_bat = "$FindBin::Bin/../remote_reinstall_blcr.bat";
  %ssh                   = runCommandSsh($node, $remote_install_bat);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$remote_install_bat'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  # Set the pbs_mom server to trust the pbs_server
  my $cmd = "echo '$pbs_server' > $pbs_server_loc";
  %ssh    = runCommandSsh($node, $cmd);
  cmp_ok($ssh{ 'EXIT_CODE' }, '==', 0, "Checking exit code of '$node:$cmd'")
    or die("Unable to install torque on '$node': $ssh{ 'STDERR' }");

  } # END foreach my $node (@nodes)


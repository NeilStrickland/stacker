<?php

require_once('cli_tools.inc');
require_once('../stacker.inc');

require_once($CFG->dirroot.'/user/lib.php');

// echo "mnet_localhost_id = " . $CFG->mnet_localhost_id . PHP_EOL;

$moodle_users = $DB->get_records('user');
$moodle_users_by_username = array();

foreach($moodle_users as $s) {
 if (! $s->username) {
  echo "Error : users {$s->id} has no username." . PHP_EOL;
  continue;
 }

 if (array_key_exists($s->username,$moodle_users_by_username)) {
  echo "Error : multiple records for username {$s->username}." . PHP_EOL;
  continue;
 }

 $moodle_users_by_username[$s->username] = $s;
}

$n = count($moodle_users);
echo "Moodle database: $n users." . PHP_EOL;

$somas_db_pass = trim(file_get_contents('/var/sangaku/somas_cred.txt'));

$somas_db = mysqli_connect('maths.shef.ac.uk',
                           'moodle_agent',
                           $somas_db_pass,
                           'pm6maths_somas',
                           3306);

if (! $somas_db) {
 trigger_error('Could not connect to database: ' . mysqli_connect_error());
 exit;
}

$users = array();

$q = <<<SQL
SELECT username,firstname,lastname,email FROM view_moodle_teachers
SQL;

$result = $somas_db->query($q);

if ($result === false) {
 $msg = "MySQL error: " . mysqli_error($somas_db) . '<br/>' . 
      "Query: $q<br/>";
 trigger_error($msg);
 
 exit;
}

while($user = mysqli_fetch_object($result)) {
 $user->username = strtolower(trim($user->username));
 $users[] = $user;
}

$n = count($users);

echo "SoMaS database : $n users." . PHP_EOL . PHP_EOL;

foreach($users as $user) {
 if (! $user->username) { continue; }
 
 if (array_key_exists($user->username,$moodle_users_by_username)) {
  $x = $moodle_users_by_username[$user->username];
  echo "User {$user->username} has Moodle ID {$x->id}." . PHP_EOL;
 } else {
  $user->auth = 'ldap';
  $user->confirmed = 1;
  $user->mnethostid = $CFG->mnet_localhost_id;
  try {
   $id = user_create_user($user, false, false); 
   echo "Added {$user->username} ({$user->firstname} {$user->lastname}) with ID {$id}" .
     PHP_EOL;
  } catch(moodle_exception $e) {
   echo "Failed to add {$user->username} ({$user->firstname} {$user->lastname})" .
    PHP_EOL . "<br/>" . PHP_EOL . $e->getMessage();
   exit;
  }
 }
}

echo "Done." . PHP_EOL;

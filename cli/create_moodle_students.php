<?php

require_once('cli_tools.inc');
require_once('../stacker.inc');

require_once($CFG->dirroot.'/user/lib.php');

// echo "mnet_localhost_id = " . $CFG->mnet_localhost_id . PHP_EOL;

$moodle_students = $DB->get_records('user');
$moodle_students_by_username = array();

foreach($moodle_students as $s) {
 if (! $s->username) {
  echo "Error : students {$s->id} has no username." . PHP_EOL;
  continue;
 }

 if (array_key_exists($s->username,$moodle_students_by_username)) {
  echo "Error : multiple records for username {$s->username}." . PHP_EOL;
  continue;
 }

 $moodle_students_by_username[$s->username] = $s;
}

$n = count($moodle_students);
echo "Moodle database: $n students." . PHP_EOL;

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

$students = array();

$q = <<<SQL
SELECT username,firstname,lastname,email,idnumber FROM view_moodle_students
SQL;

$result = $somas_db->query($q);

if ($result === false) {
 $msg = "MySQL error: " . mysqli_error($somas_db) . '<br/>' . 
      "Query: $q<br/>";
 trigger_error($msg);
 
 exit;
}

while($student = mysqli_fetch_object($result)) {
 $students[] = $student;
}

$n = count($students);

echo "SoMaS database : $n students." . PHP_EOL . PHP_EOL;

foreach($students as $student) {
 if (! $student->username) { continue; }
 
 if (array_key_exists($student->username,$moodle_students_by_username)) {
  $x = $moodle_students_by_username[$student->username];
  echo "Student {$student->username} has Moodle ID {$x->id}." . PHP_EOL;
 } else {
  $student->auth = 'ldap';
  $student->confirmed = 1;
  $student->mnethostid = $CFG->mnet_localhost_id;
  try {
   $id = user_create_user($student, false, false); 
   echo "Added {$student->username} ({$student->firstname} {$student->lastname}) with ID {$id}" .
     PHP_EOL;
  } catch(moodle_exception $e) {
   echo "Failed to add {$student->username} ({$student->firstname} {$student->lastname})" .
     PHP_EOL;
   exit;
  }
 }
}

echo "Done." . PHP_EOL;

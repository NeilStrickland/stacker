<?php

/* This script is incomplete and does not do anything in its current state.
 * It fetches a list of students from the SoMaS database and works out 
 * which of them need to be added to the Moodle database but does not
 * actually add them.  We instead use the "Synchronise users task" for
 * that, which is unsatisfactory because it sets the authentication field
 * to db instead of ldap.  At the moment we have to fix that manually.
 */

require_once('cli_tools.inc');
require_once('../stacker.inc');

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
SELECT username,firstname,lastname,email FROM view_moodle_students
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
  echo "To add: " . PHP_EOL;
  var_dump($student);
  exit;
 }
}

echo "Done." . PHP_EOL;

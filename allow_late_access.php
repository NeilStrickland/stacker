<?php

/* This script allows late access to all quizzes in the specified course. 
 * It can be run from the command line using a command like 
 * "php allow_late_access.php MAS123".  By default, if a student does
 * not visit a quiz when it is open, they will not be able to visit
 * it and view the solutions after it closes.  The purpose of this
 * script is to override that rule.
 */

require_once('cli_tools.inc');
require_once('stacker.inc');

if ($argc < 2) {
 echo "No course specified" . PHP_EOL;
 exit;
}

$course_name = $argv[1];

$C = new \stacker\course();
$C->load_by_name($course_name);

foreach($C->quizzes as $quiz) {
 echo "Allowing late access for {$quiz->name}" . PHP_EOL;
 $quiz->allow_late_access();
}

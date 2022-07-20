<?php

/* This script compiles a quiz. 
 * It can be run from the command line using a command like 
 * php compile_quizzes.php MAS123 "Quiz 6"
 *
 * This will not work if students have already attempted the
 * quiz; in that case, questions cannot be added, subtracted 
 * or moved, and any changes need to be done through the web
 * interface.  
 *
 * One can write "all" instead of a quiz name, then all 
 * quizzes will be compiled.
 * 
 * One can add 1 as an extra command line argument, like 
 * php compile_quizzes.php MAS123 "Quiz 6" 1
 * The quiz will then be compiled in debugging mode, with the
 * solution and other information included in the question body.
 */


define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../../config.php');
require_once('../stacker.inc');
cron_setup_user();

$source_dir = '/var/moodledata/stacker';

if ($argc < 2) {
 echo "No course specified" . PHP_EOL;
 exit;
}

$course_name = $argv[1];

$C = new \stacker\course();

try {
 $C->load_by_name($course_name);
} catch (Exception $e) {
 echo "Course not found: {$course_name}" . PHP_EOL;
 exit;
}

if ($argc < 3) {
 echo "No quiz specified" . PHP_EOL;
 exit;
}

$quiz_name = $argv[2];

$debug = 0;
if ($argc >= 4) {
 $debug = $argv[3] ? 1 : 0;
}

if ($quiz_name == 'all') {
 foreach($C->quizzes as $quiz) {
  echo "Compiling {$quiz->name} " . PHP_EOL;
  $quiz->compile_and_install();
  foreach($quiz->errors as $e) {
   echo $e . PHP_EOL;
  }
 }
} else {
 if (isset($C->quizzes_by_name[$quiz_name])) {
  $quiz = $C->quizzes_by_name[$quiz_name];
  echo "Compiling {$quiz->name} " . PHP_EOL;
  $quiz->compile_and_install();
  foreach($quiz->errors as $e) {
   echo $e . PHP_EOL;
  }
 } else {
  echo "Quiz {$quiz_name} not found" . PHP_EOL;
 }
}

exit;



<?php

define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../config.php');
require_once('stacker.inc');
cron_setup_user();

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
 $quiz_name = 'all';
} else {
 $quiz_name = $argv[2];
}

if ($quiz_name == 'all') {
 foreach($C->quizzes as $quiz) {
  $n = str_pad(substr($quiz->name,0,40),41);
  echo $n . ' : ' . $quiz->get_status() . PHP_EOL;
 }
} else {
 $quiz = $C->get_quiz_by_name($quiz_name);
 if ($quiz) {
  echo $quiz->get_status() . PHP_EOL;
 } else {
  echo "Quiz {$quiz_name} not found". PHP_EOL;
 }
}



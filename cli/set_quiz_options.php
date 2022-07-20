<?php

/* This script sets the standard SoMaS options for all quizzes 
 * in the specified course.  It can be run from the command line
 * using a command like "php set_quiz_options.php MAS123".
 */

define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../../config.php');
require_once('../stacker.inc');
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

foreach($C->quizzes as $quiz) {
 echo "Setting options for {$quiz->name}" . PHP_EOL;
 $quiz->set_options();
}


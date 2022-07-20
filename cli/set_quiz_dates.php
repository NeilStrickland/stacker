<?php

/* This script sets the dates for all quizzes in a specified course.
 * For MAS123, the dates are expected to be specified in a file 
 * /home/sa_pm1nps/Stack/dates/MAS123.csv
 * Each line there should consist of a quiz name, the opening 
 * time and the closing time, specified in terms of the academic
 * calendar, with strings like "S2W9M 09:00" for 9AM on the Monday
 * of week 9 in Semester 2.  Codes for days are 
 * U = Sun, M = Mon, T = Tue, W = Wed, R = Thu, F = Fri, S = Sat.
 * These are translated to actual dates using information in the
 * file weeks.csv.  See parse_time.inc for more information.
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

$weeks = read_weeks_file();
$errors = $C->read_dates_file();

if ($errors) {
 echo "Error(s) reading dates file" . PHP_EOL;
 foreach($errors as $e) {
  echo $e . PHP_EOL;
 }
 echo PHP_EOL;
}

foreach($C->quizzes as $quiz) {
 $quiz->set_dates($weeks);
 if (isset($quiz->new_dates_msg)) {
  echo $quiz->new_dates_msg . PHP_EOL;
 } else {
  echo $quiz->name . ": dates not set" . PHP_EOL;
 }
}

exit;



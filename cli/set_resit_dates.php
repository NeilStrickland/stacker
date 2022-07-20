<?php

/* If we decide to allow access to quizzes for students resitting
 * a module (say MAS123) we usually create a new course (say MAS123R)
 * for that purpose.  We use the Moodle web interface to copy all
 * quizzes from MAS123 to MAS123R.  We register resitters manually
 * for MAS123R.  We then use this script to set the dates.  Typically
 * we set the course to start on 1/7 and end on 1/9, and we set 
 * all quizzes to open and close simultaneously at the same time
 * that the course starts and ends.  This script just sets the dates.
 * It can be run from the command line as follows:
 * php set_resit_dates.php MAS123R 2022-07-01 2022-09-01 
 */

define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../../config.php');
require_once('../stacker.inc');
cron_setup_user();

if ($argc < 2) {
 echo "No course specified" . PHP_EOL;
 exit;
}

if ($argc < 3) {
 echo "No opening date specified" . PHP_EOL;
 exit;
}

if ($argc < 4) {
 echo "No closing date specified" . PHP_EOL;
 exit;
}

$course_name = $argv[1];
$opening_date = $argv[2];
$closing_date = $argv[3];
$opening_time = strtotime($opening_date . ' 00:00:00');
$closing_time = strtotime($closing_date . ' 23:59:00');

$C = new \stacker\course();

try {
 $C->load_by_name($course_name);
} catch (Exception $e) {
 echo "Course not found: {$course_name}" . PHP_EOL;
 exit;
}

foreach($C->quizzes as $quiz) {
 $id = $quiz->get_quizid();
 echo "Setting dates for {$quiz->get_quiz_name()}" . PHP_EOL;
 $id = $quiz->get_quizid();
 $quiz0 = quiz::create($id); 
 $quiz1 = $quiz0->get_quiz();
 $quiz1->timeopen  = $opening_time;
 $quiz1->timeclose = $closing_time;
 $DB->update_record('quiz',$quiz1);
}



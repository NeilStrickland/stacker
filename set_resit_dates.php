<?php

$source_dir = '/home/sa_pm1nps/Stack';
chdir('/var/www/html/moodle/scripts');
require_once('cli_tools.inc');

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

$C = new stacker_course();

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



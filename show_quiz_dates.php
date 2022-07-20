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

$weeks = read_weeks_file();
$errors = $C->read_dates_file();
$dates = '';

foreach($C->quizzes as $quiz) {
 $raw_quiz = $quiz->moodle_quiz->get_quiz();
 $dates .= 
  '"' . $raw_quiz->name . '","' .
  date('Y-m-d H:m',$raw_quiz->timeopen) . '","' .
  date('Y-m-d H:m',$raw_quiz->timeclose) . '"' . PHP_EOL;
}

echo $dates;

exit;



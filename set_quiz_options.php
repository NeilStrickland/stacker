<?php

require_once('cli_tools.inc');
require_once('stacker.inc');

$source_dir = '/home/sa_pm1nps/Stack';
chdir('/var/www/html/moodle/scripts');
require_once('cli_tools.inc');

if ($argc < 2) {
 echo "No course specified" . PHP_EOL;
 exit;
}

$course_name = $argv[1];

$C = new stacker_course();

try {
 $C->load_by_name($course_name);
} catch (Exception $e) {
 echo "Course not found: {$course_name}" . PHP_EOL;
 exit;
}

foreach($C->quizzes as $quiz) {
 $id = $quiz->get_quizid();
 $q = set_quiz_options($id);
 echo "Setting options for {$quiz->get_quiz_name()}" . PHP_EOL;
 $DB->update_record('quiz',$q);
}




<?php

require_once('cli_tools.inc');
require_once('stacker.inc');

if ($argc < 2) {
 echo "No course specified" . PHP_EOL;
 exit;
}

$course_name = $argv[1];

$C = new stacker_course();
$C->load_by_name($course_name);

foreach($C->quizzes as $quiz) {
 $id = $quiz->get_quizid();
 echo "Adding hints for {$quiz->get_quiz_name()}" . PHP_EOL;
 add_quiz_hints($id,4);
}




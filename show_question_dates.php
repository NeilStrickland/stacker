<?php

chdir('/var/www/html/moodle/scripts/stacker');
require_once('cli_tools.inc');
require_once('stacker.inc');

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

if ($argc < 3) {
 echo "No quiz specified" . PHP_EOL;
 exit;
}

$quiz_name = $argv[2];

if ($quiz_name == 'all') {
 foreach($C->quizzes as $quiz) {
  $quiz_name = $quiz->get_quiz_name();
  show_one($quiz_name);
  echo PHP_EOL;
 }
} else {
 show_one($quiz_name);
}

exit;

//////////////////////////////////////////////////////////////////////

function show_one($quiz_name) {
 global $DB,$C,$source_dir;
 
 $moodle_quiz = null;
 
 if (isset($C->quizzes_by_name[$quiz_name])) {
  $moodle_quiz = $C->quizzes_by_name[$quiz_name];
 } elseif (isset($C->quizzes_by_short_name[$quiz_name])) {
  $moodle_quiz = $C->quizzes_by_short_name[$quiz_name];
 } else {
  echo "Quiz not found: $quiz_name" . PHP_EOL;
  exit;
 }

 $stacker_quiz = new stacker\quiz();
 $stacker_quiz->name = $moodle_quiz->get_quiz_name();
 $stacker_quiz->set_dirs($source_dir . '/questions/' . $C->shortname);
 $stacker_quiz->munch_moodle_quiz($moodle_quiz);

 $stacker_quiz->get_status();
 
 $n = str_pad(substr($stacker_quiz->name,0,40),41);
 echo $n . $stacker_quiz->status; 
}


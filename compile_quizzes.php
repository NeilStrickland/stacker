<?php

$source_dir = '/home/sa_pm1nps/Stack';
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

$debug = 0;
if ($argc >= 4) {
 $debug = $argv[3] ? 1 : 0;
}

if ($quiz_name == 'all') {
 foreach($C->quizzes as $quiz) {
  $quiz_name = $quiz->get_quiz_name();
  compile_one($quiz_name,$debug);
 }
} else {
 compile_one($quiz_name,$debug);
}

exit;

//////////////////////////////////////////////////////////////////////

function compile_one($quiz_name,$debug = 0) {
 global $C,$source_dir;
 
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
 
 if (! file_exists($stacker_quiz->full_stack_file_name)) {
  $stacker_quiz->set_file_name($moodle_quiz->short_name);
  if (! file_exists($stacker_quiz->full_stack_file_name)) {
   echo "Source file not found for {$stacker_quiz->name} [{$stacker_quiz->full_stack_file_name}]" . PHP_EOL;
   return false;
  }
 }
 
 echo "Compiling {$stacker_quiz->full_stack_file_name}" . PHP_EOL;

 $doc = new DOMDocument();
 $doc->formatOutput = true;
 $stacker_quiz->compile($debug);
 $stacker_quiz->save_xml($doc);

 $quiz_id = $moodle_quiz->get_quizid();

 echo "Updating quiz slots (id = {$quiz_id})" . PHP_EOL;

 remove_questions_from_quiz($quiz_id);
 echo "Import : {$stacker_quiz->full_xml_file_name} " . PHP_EOL;
 import_xml($quiz_id,$stacker_quiz->xml_dir,$stacker_quiz->xml_file_name);
 use_default_category($quiz_id);

 if ($stacker_quiz->errors) {
  echo "Question compilation errors:" . PHP_EOL;
  var_dump($stacker_quiz->errors);
 }
}


<?php

$source_dir = '/home/sa_pm1nps/Stack';
chdir('/var/www/html/moodle/scripts/stacker');
require_once('cli_tools.inc');
require_once('parse_time.inc');
require_once($CFG->dirroot . '/mod/quiz/lib.php');

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

$weeks = read_weeks_file();

// $dates_file = $source_dir . "/questions/" . $course_name . "/dates.csv";
$dates_file = $source_dir . "/dates/" . $course_name . "_dates.csv";
$dates_file = realpath($dates_file);

$fh = fopen($dates_file,"r");
if ($fh === false) {
 echo "Dates file {$dates_file} not found" . PHP_EOL;
 exit;
}

$i = 1;
while(($x = fgetcsv($fh)) !== false) {
 if (count($x) == 0) { continue; }
 if (count($x) != 3) {
  echo "Bad line (number $i) in dates file" . PHP_EOL;
  continue;
 }
 
 $quiz_name = $x[0];
 $open_time = parse_time($x[1],$weeks);
 $close_time = parse_time($x[2],$weeks);

 if (isset($C->quizzes_by_short_name[$quiz_name])) {
  $quiz = $C->quizzes_by_short_name[$quiz_name];
 } else if (isset($C->quizzes_by_name[$quiz_name])) {
  $quiz = $C->quizzes_by_name[$quiz_name];
 } else {
  echo "Quiz $quiz_name not found" . PHP_EOL;
  continue;
 }

 $id = $quiz->get_quizid();
 $quiz0 = quiz::create($id); 
 $quiz1 = $quiz0->get_quiz();
 $quiz1->timeopen  = $open_time->timestamp;
 $quiz1->timeclose = $close_time->timestamp;
 $DB->update_record('quiz',$quiz1);
 quiz_update_events($quiz1);
 
 echo "$quiz_name : opens {$open_time->ymdhm}, closes {$close_time->ymdhm}" . PHP_EOL;
 
 $i++;
}

exit;



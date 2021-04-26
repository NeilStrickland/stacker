<?php

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

$weeks = array(1 => array(),2 => array());

if (! file_exists("weeks.csv")) {
 echo "Weeks file not found in " . getcwd() . PHP_EOL;
 exit; 
}

$fh = fopen("weeks.csv","r");
if ($fh === false) {
 echo "Weeks file not opened" . PHP_EOL;
 exit;
}

while(($x = fgetcsv($fh)) !== false) {
 $w = new stdClass();
 $w->ymd = $x[2];
 $w->start = strtotime($x[2]);
 $w->days = array();
 $w->days['M'] = strtotime($x[2]);
 $w->days['T'] = strtotime($x[2] . " +1 day");
 $w->days['W'] = strtotime($x[2] . " +2 days");
 $w->days['R'] = strtotime($x[2] . " +3 days");
 $w->days['F'] = strtotime($x[2] . " +4 days");
 if (false) {
  $w->days['mon'] = $w->days['M'];
  $w->days['tue'] = $w->days['T'];
  $w->days['wed'] = $w->days['W'];
  $w->days['thu'] = $w->days['R'];
  $w->days['fri'] = $w->days['F'];
  $w->days['monday']    = $w->days['M'];
  $w->days['tuesday']   = $w->days['T'];
  $w->days['wednesday'] = $w->days['W'];
  $w->days['thursday']  = $w->days['R'];
  $w->days['friday']    = $w->days['F'];
 }
 $weeks[(int) $x[0]][(int) $x[1]] = $w;
}

fclose($fh);

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
 $open_time = parse_time($x[1]);
 $close_time = parse_time($x[2]);

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

 echo "$quiz_name : opens {$open_time->ymdhm}, closes {$close_time->ymdhm}" . PHP_EOL;
 
 $i++;
}

exit;

//////////////////////////////////////////////////////////////////////

function parse_time($s) {
 global $weeks;
 
 if (preg_match("/^202[0-9]-[0-1][0-9]-[0-3][0-9] [0-2][0-9]:[0-5][0-9]$/",$s)) {
  $x = new stdClass;
  $x->string = $s;
  $x->timestamp = strtotime($s);
  $x->ymdhm = $s;
  return($x);
 }

 if (preg_match("/^S([1-2])W([0-9]*)([MTWRF]) ([0-2][0-9]:[0-5][0-9])$/",$s,$m)) {
  $semester = (int) $m[1];
  $week = (int) $m[2];
  $day_of_week = $m[3];
  $time = $m[4];
 } else if (preg_match("/^W([0-9]*)([MTWRF]) ([0-2][0-9]:[0-5][0-9])$/",$s,$m)) {
  $semester = 1;
  $week = (int) $m[1];
  $day_of_week = $m[2];
  $time = $m[3];
 } else {
  echo "Bad time string: |$s|" . PHP_EOL;
  exit;
 }

 $x = new stdClass;
 $x->string = $s;
 $x->ymdhm = date('Y-m-d ',$weeks[$semester][$week]->days[$day_of_week]) . $time;
 $x->timestamp = strtotime($x->ymdhm);
 return($x);
}


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

$dates_file = $source_dir . "/questions/" . $course_name . "/dates.csv";

echo "Dates file: $dates_file" . PHP_EOL;

$dates = "";

foreach($C->quizzes as $moodle_quiz) {
 $quiz_data = $moodle_quiz->get_quiz();
 $dates .= 
  '"' . $quiz_data->name . '","' .
  date('Y-m-d H:m',$quiz_data->timeopen) . '","' .
  date('Y-m-d H:m',$quiz_data->timeclose) . '"' . PHP_EOL;
}

file_put_contents($dates_file,$dates);
echo $dates;

exit;



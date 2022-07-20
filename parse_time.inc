<?php

/* We like to specify quiz opening and closing dates in terms
 * of the academic calendar, e.g.  S2W5T for Tuesday of week 5
 * in Semester 1.  To translate these specifications into 
 * actual dates we need information about the academic calendar,
 * which is stored in the file weeks.csv.  This file can be 
 * generated automatically using information fetched from 
 * maths.shef.ac.uk (and that information comes from the 
 * main SoMaS database).
 */

function read_weeks_file($file = 'weeks.csv') {
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
  $weeks[(int) $x[0]][(int) $x[1]] = $w;
 }

 fclose($fh);

 return $weeks;
}

function fetch_date_info($session = '') {
 $s = (int) $session;
 if (! $s) { $s = date('Y'); }
 $url = 'https://maths.shef.ac.uk/somas/date_info.php?session=' . $s;
 return json_decode(file_get_contents($url));
}

function weeks_from_date_info($di) {
 $weeks = array('1' => array(), '2' => array());
 for ($i = 1; $i <= 2; $i++) {
  for ($j = 0; $j <= 12; $j++) {
   $t = $di->week_start_time[$i][$j];
   $w = new stdClass();
   $w->ymd = date('Y-m-d',$t);
   $w->start = $t;
   $w->days = array();
   $w->days['U'] = strtotime($w->ymd . ' -1 day');
   $w->days['M'] = $t;
   $w->days['T'] = strtotime($w->ymd . ' +1 day');
   $w->days['W'] = strtotime($w->ymd . ' +2 day');
   $w->days['R'] = strtotime($w->ymd . ' +3 day');
   $w->days['F'] = strtotime($w->ymd . ' +4 day');
   $w->days['S'] = strtotime($w->ymd . ' +5 day');
   $weeks[$i][$j] = $w;
  }
 }

 return $weeks;
}

function make_weeks_file() {
 $di = fetch_date_info();
 $s = "";
 
 for ($i = 1; $i <= 2; $i++) {
  for ($j = 0; $j <= 12; $j++) {
   $t = $di->week_start_time[$i][$j];
   $s .= "$i,$j," . date('Y-m-d',$di->week_start_time[$i][$j]) . PHP_EOL;
  }
 }

 file_put_contents("weeks.csv",$s);

 return $s;
}

function parse_time($s, $weeks) {
 if (preg_match("/^202[0-9]-[0-1][0-9]-[0-3][0-9] [0-2][0-9]:[0-5][0-9]$/",$s)) {
  $x = new stdClass;
  $x->string = $s;
  $x->timestamp = strtotime($s);
  $x->ymdhm = $s;
  return($x);
 }

 if (preg_match("/^S([1-2])W([0-9]*)([UMTWRFS]) ([0-2][0-9]:[0-5][0-9])$/",$s,$m)) {
  $semester = (int) $m[1];
  $week = (int) $m[2];
  $day_of_week = $m[3];
  $time = $m[4];
 } else if (preg_match("/^W([0-9]*)([UMTWRFS]) ([0-2][0-9]:[0-5][0-9])$/",$s,$m)) {
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
 if (! isset($weeks[$semester])) { echo "T0\n"; var_dump($weeks); exit; }
 if (! isset($weeks[$semester][$week])) { echo "T1\n"; var_dump($weeks[$semester]); exit; }
 if (! isset($weeks[$semester][$week]->days)) { echo "T2\n"; exit; }
 if (! isset($weeks[$semester][$week]->days[$day_of_week])) { echo "T3\n"; exit; }
 
 $x->ymdhm = date('Y-m-d ',$weeks[$semester][$week]->days[$day_of_week]) . $time;
 $x->timestamp = strtotime($x->ymdhm);
 return($x);
}


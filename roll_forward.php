<?php

/* This script is intended to be run in the summer.  It sets the 
 * start and end dates for every course that ran in the previous
 * academic year, so that it now runs in the following academic
 * year.  It also sets starting and ending dates for all of the
 * quizzes.
 */

define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../config.php');
require_once('stacker.inc');
cron_setup_user();

require_once($CFG->dirroot.'/lib/moodlelib.php');

$di = fetch_date_info();
$sd = $di->session_dates;

$weeks = weeks_from_date_info($di);

$new_start_string = array(
 'Autumn' => $sd->intro_week,
 'Spring' => $sd->spring_start,
 'Year'   => $sd->intro_week,
 'Resit'  => $sd->spring_end
);
$new_end_string = array(
 'Autumn' => $sd->spring_start,
 'Spring' => $sd->spring_end,
 'Year'   => $sd->spring_end,
 'Resit'  => $sd->session . '-09-01'
);


$courses = $DB->get_records('course');

foreach ($courses as $c) {
 $c->start_datetime = new \DateTime();
 $c->start_datetime->setTimestamp($c->startdate);
 $c->start_string = $c->start_datetime->format('Y-m-d');

 $c->end_datetime = new \DateTime();
 $c->end_datetime->setTimestamp($c->enddate);
 $c->end_string = $c->end_datetime->format('Y-m-d');

 $y0 = (int) $c->start_datetime->format('Y');
 $y1 = (int) $c->end_datetime->format('Y');
 if (! ($y0 == $di->session - 1)) {
  echo "Skipping {$c->shortname}\n";
  continue;
 }
 
 $n0 = (int) $c->start_datetime->format('n');
 $n1 = (int) $c->end_datetime->format('n');

 $c->semester = null;
 if (1 <= $n0 && $n0 <= 2) {
  $c->semester = 'Spring';
 } else if (6 <= $n0 && $n0 <= 8) {
  $c->semester = 'Resit';
 } else if (9 <= $n0 && $n0 <= 10) {
  if ($n1 == 12 || $n1 == 1 || $n1 == 2) {
   $c->semester = 'Autumn';
  } else {
   $c->semester = 'Year';
  }
 }

 $c->new_start_string = '';
 $c->new_end_string = '';
 $c->new_start_date = '';
 $c->new_end_date = '';
 
 if ($c->semester) {
  $c->new_start_string = $new_start_string[$c->semester];
  $c->new_end_string = $new_end_string[$c->semester];
  $c->new_start_date = strtotime($c->new_start_string);
  $c->new_end_date = strtotime($c->new_end_string);
 }

 $c->startdate = $c->new_start_date;
 $c->enddate = $c->new_end_date;
 $DB->update_record('course',$c);
 $sc = new \stacker\course();
 $sc->raw_fill($c);
 $sc->read_dates_file();
 echo "Setting quiz dates for {$c->shortname}\n";
 $sc->set_quiz_dates($weeks);
 
 echo "{$c->shortname}: {$c->start_string}: {$c->end_string}: {$c->semester}: {$c->new_start_date}: {$c->new_end_date} \n";

}


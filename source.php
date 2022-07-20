<?php

/**
 * This file is a test
 *
 * @copyright  2022 Neil Strickland
 * @license   http://www.gnu.org/copyleft/gpl.html GNU GPL v3 or later
 */


require_once('../../config.php');
require_once($CFG->dirroot . '/mod/quiz/locallib.php');
require_once($CFG->dirroot . '/question/editlib.php');
require_once('./stacker.inc');

$cmid    = required_param('cmid', PARAM_INT);
$compile = optional_param('compile', 0, PARAM_BOOL);
$debug   = optional_param('debug', 0, PARAM_BOOL);

list($module, $cm) = get_module_from_cmid($cmid);

if (!$raw_course = $DB->get_record('course', array('id' => $cm->course))) {
 print_error('coursemisconf');
}

require_login($raw_course, false, $cm);
$context = context_module::instance($cm->id);
require_capability('mod/quiz:manage', $context);

$course = new \stacker\course();
$course->raw_fill($raw_course);
$moodle_quiz = quiz::create($cm->instance, $USER->id);
$quiz = new \stacker\quiz();
$quiz->fill($moodle_quiz);
$quiz->set_dirs($course->source_dir);

$source_url = new moodle_url('/local/stacker/source.php',
                              array('cmid' => $cm->id));

$compile_url = new moodle_url('/local/stacker/source.php',
                              array('cmid' => $cm->id,
                                    'compile' => 1));

$debug_url = new moodle_url('/local/stacker/source.php',
                            array('cmid' => $cm->id,
                                  'compile' => 1,
                                  'debug' => 1
                            ));

$f = $quiz->full_stack_file_name;
$errors = array();

if (file_exists($f)) {
 $t = '(Modified ' . date('Y-m-d H:i:s',filemtime($f)) . ')';
 $t = '(' . $quiz->get_status() . ')';
 $s = \stacker\compiler::esc(file_get_contents($f));
 if ($compile) {
  $errors = $quiz->compile_and_install($debug);
  if ($errors) {
   $compile = 0;
   $debug = 0;
  }
 }
} else {
 $t = '';
 $s = "No source file ($f)";
 $compile = 0;
 $debug = 0;
}

$PAGE->set_url('/local/stacker/source.php', array('cmid' => $cmid));
$PAGE->navbar->add('Stacker');
if ($compile) {
 $PAGE->set_title("Compilation: $quiz->name");
} else {
 $PAGE->set_title("Source: $quiz->name");
}
$PAGE->set_heading($quiz->name);
$PAGE->set_pagelayout('admin');
$PAGE->activityheader->disable();

echo $OUTPUT->header();
echo $OUTPUT->heading('Stacker source ' . $t);

if ($compile) {
  $msg = <<<HTML
The file compiled without errors and the quiz has been updated.
<br/>

HTML
      ;
 echo $msg;
 echo $OUTPUT->single_button($source_url,'Source','get');
 
} else {
 if ($errors) {
  $msg = <<<HTML
There were compilation errors as listed below, so the Moodle XML
file was not updated and the quiz was left unchanged.
<ul>
      
HTML
      ;
  echo $msg;

  foreach($errors as $e) {
   echo "<li>" . \stacker\compiler::esc($e) . "</li>" . PHP_EOL;
  }

  echo "</ul><br/>" . PHP_EOL;
 }

 echo $OUTPUT->single_button($compile_url,'Compile','get');
 echo $OUTPUT->single_button($debug_url,'Compile (debug mode)','get');

 echo html_writer::tag('pre',$s);
}

echo $OUTPUT->footer();



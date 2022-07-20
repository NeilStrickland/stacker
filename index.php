<?php

// This file is part of Moodle - http://moodle.org/
//
// Moodle is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Moodle is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Moodle.  If not, see <http://www.gnu.org/licenses/>.

/**
 * This file is a test
 *
 * @copyright  2022 Neil Strickland
 * @license   http://www.gnu.org/copyleft/gpl.html GNU GPL v3 or later
 */


require_once('../../config.php');
require_once($CFG->dirroot . '/mod/quiz/locallib.php');
require_once('./lib.php');
require_once('./stacker.inc');

$id = required_param('id', PARAM_INT); // Course.
$raw_course = $DB->get_record('course', array('id' => $id), '*', MUST_EXIST);
require_course_login($raw_course);

$course = new \stacker\course();
$course->raw_fill($raw_course);

$PAGE->set_url('/local/stacker/index.php', array('id' => $id));
$PAGE->navbar->add('Stacker');
$PAGE->set_title("$raw_course->shortname: Stacker");
$PAGE->set_heading($raw_course->fullname);
$PAGE->set_pagelayout('course');

echo $OUTPUT->header();
echo $OUTPUT->heading('Stacker');

$table = new html_table();
$table->head = array('Quiz','attempted','status');

foreach($course->quizzes as $quiz) {
 $q = $quiz->moodle_quiz;
 if (!isset($q->cmid)) {
  $cm = get_coursemodule_from_instance('quiz', $quiz->id, $course->id);
  $q->cmid = $cm->id;
 }

 $u = new \moodle_url('/local/stacker/source.php',array('cmid' => $q->cmid));
 $l = html_writer::link($u,$quiz->short_name);
 $quiz->has_attempts = quiz_has_attempts($quiz->id);
 $a = $quiz->has_attempts ? 'Y' : 'N';
 $table->data[] = array($l,$a,$quiz->get_status());
}

echo html_writer::table($table);
 
//echo html_writer::div('Hello World!!??');

echo $OUTPUT->footer();





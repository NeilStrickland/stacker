<?php

/* This file removes all student data from all courses.  It should
 * be run in July or maybe September to reset the system for the 
 * next academic year.  It is very slow.  If you start it running 
 * in the evening it should finish by the next morning.
 */

define('CLI_SCRIPT', true);
require(__DIR__.'/../../config.php');
cron_setup_user();

require_once($CFG->dirroot.'/lib/moodlelib.php');
require_once($CFG->dirroot.'/course/lib.php');

$courses = $DB->get_records('course');

foreach($courses as $course) {
 echo "Removing student data for {$course->shortname}\n";
 
 $d = new stdClass();
 $d->id = $course->id;
 
 $roles_to_remove =
                  get_archetype_roles('student') +
                  get_archetype_roles('guest') +
                  get_archetype_roles('user');
 
 $role_ids_to_remove = array();
 foreach($roles_to_remove as $r) {
  $role_ids_to_remove[] = $r->id;
 }
 
 $d->unenrol_users = $role_ids_to_remove;
 $d->reset_roles_local = true;
 $d->reset_roles_override = true;
 $d->reset_quiz_attempts = true;
 $d->reset_quiz_user_overrides = true;
 $d->reset_quiz_group_overrides = true;
 
 reset_course_userdata($d);
}

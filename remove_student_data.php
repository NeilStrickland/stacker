<?php

$source_dir = '/home/sa_pm1nps/Stack';
require_once('cli_tools.inc');

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

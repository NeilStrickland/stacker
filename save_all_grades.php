<?php

define('CLI_SCRIPT', true);

require(__DIR__.'/../../config.php');
require_once($CFG->dirroot.'/grade/export/lib.php');
require_once($CFG->libdir . '/csvlib.class.php');

cron_setup_user();


// This is a copy of code from grade/export/txt/grade_export_txt.php
// modified so as not to call exit() and to write output to a local file.

class grade_export_stacker extends grade_export {

    public $plugin = 'txt';
    public $separator = 'comma'; // default separator
    public $dir = '/var/moodle_archive/results';
 
    /**
     * Constructor should set up all the private variables ready to be pulled
     * @param object $course
     * @param int $groupid id of selected group, 0 means all
     * @param stdClass $formdata The validated data from the grade export form.
     */
    public function __construct($course, $dir) {
        $groupid = 0;
        $formdata = new stdClass();
        $formdata->separator = "comma";
        parent::__construct($course, $groupid, $formdata);
        $this->dir = $dir;

        // Overrides.
        $this->usercustomfields = true;
    }

    public function get_export_params() {
        $params = parent::get_export_params();
        $params['separator'] = $this->separator;
        $this->displaytype = array('real' => '1');
        return $params;
    }

    public function print_grades() {
        global $CFG;

        $export_tracking = $this->track_exports();

        $strgrades = get_string('grades');
        $profilefields = grade_helper::get_user_profile_fields($this->course->id, $this->usercustomfields);

        $shortname = format_string($this->course->shortname, true, array('context' => context_course::instance($this->course->id)));
        $downloadfilename = clean_filename("$shortname $strgrades");
        $csvexport = new csv_export_writer($this->separator);
        $csvexport->set_filename($downloadfilename);

        // Print names of all the fields
        $exporttitle = array();
        foreach ($profilefields as $field) {
            $exporttitle[] = $field->fullname;
        }

        if (!$this->onlyactive) {
            $exporttitle[] = get_string("suspended");
        }

        // Add grades and feedback columns.
        foreach ($this->columns as $grade_item) {
            foreach ($this->displaytype as $gradedisplayname => $gradedisplayconst) {
                $exporttitle[] = $this->format_column_name($grade_item, false, $gradedisplayname);
            }
            if ($this->export_feedback) {
                $exporttitle[] = $this->format_column_name($grade_item, true);
            }
        }
        // Last downloaded column header.
        $exporttitle[] = get_string('timeexported', 'gradeexport_txt');
        $csvexport->add_data($exporttitle);

        // Print all the lines of data.
        $geub = new grade_export_update_buffer();
        $gui = new graded_users_iterator($this->course, $this->columns, $this->groupid);
        $gui->require_active_enrolment($this->onlyactive);
        $gui->allow_user_custom_fields($this->usercustomfields);
        $gui->init();
        while ($userdata = $gui->next_user()) {

            $exportdata = array();
            $user = $userdata->user;

            foreach ($profilefields as $field) {
                $fieldvalue = grade_helper::get_user_field_value($user, $field);
                $exportdata[] = $fieldvalue;
            }
            if (!$this->onlyactive) {
                $issuspended = ($user->suspendedenrolment) ? get_string('yes') : '';
                $exportdata[] = $issuspended;
            }
            foreach ($userdata->grades as $itemid => $grade) {
                if ($export_tracking) {
                    $status = $geub->track($grade);
                }

                foreach ($this->displaytype as $gradedisplayconst) {
                    $exportdata[] = $this->format_grade($grade, $gradedisplayconst);
                }

                if ($this->export_feedback) {
                    $exportdata[] = $this->format_feedback($userdata->feedbacks[$itemid], $grade);
                }
            }
            // Time exported.
            $exportdata[] = time();
            $csvexport->add_data($exportdata);
        }
        $gui->close();
        $geub->close();
        $s = $csvexport->print_csv_data(true);
        file_put_contents($this->dir . '/' . $shortname . '.csv', $s);
    }
}

$courses = $DB->get_records('course');

foreach ($courses as $course) {
 echo "Saving grades for $course->shortname\n";
 $e = new grade_export_stacker($course,'/var/moodle_archive/results/2021-22');
 $e->get_export_params();
 $e->print_grades();
}

echo "Done\n";


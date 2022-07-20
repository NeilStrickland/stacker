<?php

/**
 *
 * @package    local_stacker
 * @copyright  2022 onwards Neil Strickland <N.P.Strickland@sheffield.ac.uk>
 * @license    http://www.gnu.org/copyleft/gpl.html GNU GPL v3 or later
 */

defined('MOODLE_INTERNAL') || die();

/**
 * This function extends the course navigation 
 *
 * @param navigation_node $navigation The navigation node to extend
 * @param stdClass        $course     The course 
 * @param context         $context    The context of the course
 */

function local_stacker_extend_navigation_course($navigation, $course, $context) {
 $url = new moodle_url('/local/stacker/index.php', array('id' => $course->id));
 $x = $navigation->add('Stacker', $url);
}

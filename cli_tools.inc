<?php

define('CLI_SCRIPT', true);
 
require(__DIR__.'/../../config.php');

require_once($CFG->libdir.'/clilib.php');
require_once($CFG->dirroot.'/mod/quiz/attemptlib.php');
require_once($CFG->dirroot.'/mod/quiz/accessmanager.php');
require_once($CFG->dirroot.'/question/engine/bank.php');
require_once($CFG->dirroot.'/question/format/xml/format.php');
require_once($CFG->dirroot.'/lib/questionlib.php');

cron_setup_user();

//////////////////////////////////////////////////////////////////////

class stacker_course {
 function load_by_name($name) {
  global $DB;

  $x = $DB->get_record('course',array('shortname' => $name));
  if ($x) { 
   $this->fill($x);
  } else {
   error("Course not found");
  }
 }

 function fill($x) {
  global $DB;

  $this->data = $x;
  $this->id = $this->data->id;
  $this->shortname = $this->data->shortname;
  $this->context = $DB->get_record('context',array('contextlevel' => 50,'instanceid' => $this->id));
  $this->contextid = $this->context->id;

  $quizdatas =
   $DB->get_records('course_modules',array('course' => $this->id, 'module' => 16));
  $this->quizzes = array();
  $this->quizzes_by_name = array();
  $this->quizzes_by_short_name = array();

  foreach($quizdatas as $quizdata) {
   // $moodle_quiz is an instance of quiz, defined in moodle/mod/quiz/attemptlib.php
   $moodle_quiz = quiz::create($quizdata->instance);
   $moodle_quiz->preload_questions();
   $moodle_quiz->load_questions();
   $moodle_quiz->get_structure();
   
   $nm = $moodle_quiz->get_quiz_name();
   $i = strpos($nm,':');
   if ($i === false) {
    $sn = $nm;
   } else {
    $sn = substr($nm,0,$i);
   }

   $moodle_quiz->short_name = $sn;
   
   $this->quizzes[] = $moodle_quiz;
   $this->quizzes_by_name[$nm] = $moodle_quiz;
   $this->quizzes_by_short_name[$sn] = $moodle_quiz;
  }
 }
}

function remove_questions_from_quiz($quizid) {
 global $DB;
 
 $quiz = quiz::create($quizid);
 $quiz->preload_questions();
 $quiz->load_questions();
 $quiz->get_structure();
 
 $structure = $quiz->get_structure();
 $slots = $structure->get_slots();

 while(($slot = $structure->get_last_slot()) !== FALSE) {
  // cli_writeln("Removing slot: ({$slot->id},{$slot->slot})");
  $structure->remove_slot($slot->slot);
  $structure = $quiz->get_structure();
 }

 $context = $quiz->get_context();
 
 $cat_name = 'Default for ' . $quiz->get_quiz_name();

 $category = get_default_category($quizid);

 if ($category) {
  $finder = question_finder::get_instance();
  
  $question_ids =
                $finder->get_questions_from_categories(array($category->id),"");

  foreach ($question_ids as $i) {
   question_delete_question($i);
  }
 } else {
  echo "Category '$cat_name' does not exist" . PHP_EOL;
 }
}

function remove_stack_questions_from_quiz($quizid) {
 global $DB;
 
 $quiz = quiz::create($quizid);
 $quiz->preload_questions();
 $quiz->load_questions();
 $quiz->get_structure();
 
 $structure = $quiz->get_structure();
 $slots = $structure->get_slots();

 while(($slot = $structure->get_last_slot()) !== FALSE) {
  // cli_writeln("Removing slot: ({$slot->id},{$slot->slot})");
  $structure->remove_slot($slot->slot);
  $structure = $quiz->get_structure();
 }

 $context = $quiz->get_context();
 
 $cat_name = 'Default for ' . $quiz->get_quiz_name();

 $category = get_default_category($quizid);

 if ($category) {
  $finder = question_finder::get_instance();
  
  $question_ids =
                $finder->get_questions_from_categories(array($category->id),"");

  foreach ($question_ids as $i) {
   question_delete_question($i);
  }
 } else {
  echo "Category '$cat_name' does not exist" . PHP_EOL;
 }
}

function import_xml($quizid,$dir,$file) {
 $quiz = quiz::create($quizid); 
 $context = $quiz->get_context();

 $qf = new qformat_xml();
 $qf->setContexts(array($context));
 $qf->setFilename($dir . '/' . $file);
 $qf->setRealFilename($file);
 $qf->setCatfromfile(true);
 $qf->setContextfromfile(true);
 $qf->set_display_progress(false);
 $qf->importprocess();
}

function get_default_category($quizid) {
 global $DB;
 
 $quiz = quiz::create($quizid); 
 $context = $quiz->get_context();
 $cat_name = 'Default for ' . $quiz->get_quiz_name();

 $cat = $DB->get_record('question_categories',
                        array('name' => $cat_name,
                              'contextid' => $context->id));

 if (! $cat) {
  echo "No category with name $cat_name in context {$context->id}" . PHP_EOL;
  
  $top_cat = $DB->get_record('question_categories',
                             array('name' => 'top',
                                   'contextid' => $context->id));
  if (! $top_cat) {
   echo "No category with name top in context {$context->id}" . PHP_EOL;
   $top_cat = new stdClass();
   $top_cat->name = 'top';
   $top_cat->info = '';
   $top_cat->sortorder = 0;
   $top_cat->contextid = $context->id;
   $top_cat->stamp = make_unique_id_code();
   $top_cat->id = $DB->insert_record('question_categories',$top_cat);
  }
  
  $cat = new stdClass();
  $cat->name = $cat_name;
  $cat->info = "The default category for questions shared in context '" .
             $quiz->get_quiz_name() . "'";
  $cat->contextid = $context->id;
  $cat->parent = $top_cat->id;
  $cat->stamp = make_unique_id_code();
  $cat->id = $DB->insert_record('question_categories',$cat);
  $cat = $DB->get_record('question_categories',
                         array('name' => $cat_name,
                               'contextid' => $context->id));
 }

 return($cat);
}

function use_default_category($quizid,$short_name = '') {
 global $DB;
 
 $quiz = quiz::create($quizid); 
 $quiz->preload_questions();
 $quiz->load_questions();
 $structure = $quiz->get_structure();
  
 $context = $quiz->get_context();

 if ($short_name) {
  $cat_name = 'Default for ' . $short_name;
 } else {
  $cat_name = 'Default for ' . $quiz->get_quiz_name();
 }
 
 $category = get_default_category($quizid);

 if ($category) {
  $finder = question_finder::get_instance();
  
  $question_ids =
                $finder->get_questions_from_categories(array($category->id),"");
  
  $q = $quiz->get_quiz();
  $q->cmid = $quiz->get_cmid();
  $q->instance = $q->id;
  
  foreach($question_ids as $i) {
   quiz_add_quiz_question($i,$q,1);
  }

  quiz_delete_previews($q);
  quiz_update_sumgrades($q);
  quiz_repaginate_questions($quizid, 4);
 } else {
  echo "Category '$cat_name' does not exist" . PHP_EOL;
 }
}

function set_quiz_options($quizid) {
 global $DB;
 $quiz = quiz::create($quizid); 

 $q = $quiz->get_quiz();
 
 $q->preferredbehaviour = 'adaptive';
 $q->reviewattempt = 0x11110;
 $q->reviewcorrectness = 0x11110;
 $q->reviewmarks = 0x11110;
 $q->reviewspecificfeedback = 0x11110;
 $q->reviewgeneralfeedback = 0x10;
 $q->reviewrightanswer = 0x10;
 $q->reviewoverallfeedback = 0x10;
 $q->attempts = 1;
 
 return($q);
}

function set_quiz_dev_options($quizid) {
 global $DB;
 $quiz = quiz::create($quizid); 

 $q = $quiz->get_quiz();
 
 $q->preferredbehaviour = 'interactive';
 $q->reviewattempt = 0x11110;
 $q->reviewcorrectness = 0x11110;
 $q->reviewmarks = 0x11110;
 $q->reviewspecificfeedback = 0x11110;
 $q->reviewgeneralfeedback = 0x1110;
 $q->reviewrightanswer = 0x1110;
 $q->reviewoverallfeedback = 0x1110;
 $q->timeclose = strtotime('2020-09-20');
 
 return($q);
}

function add_question_hints($qid,$num_hints) {
 global $DB;

 $hints = $DB->get_records('question_hints',
                           array('questionid' => $qid));

 if ($hints) {
  $n = count($hints);
 } else {
  $n = 0;
 }
 
 for ($i = $n; $i < $num_hints; $i++) {
  $hint = new stdClass();
  $hint->questionid = $qid;
  $hint->hint = '<p>-</p>';
  $hint->hintformat = 1;
  $hint->id = $DB->insert_record('question_hints', $hint);
 }
}

function add_quiz_hints($quizid,$num_hints) {
 $quiz = quiz::create($quizid); 
 $quiz->preload_questions();
 $quiz->load_questions();
 $qids = array_keys($quiz->get_questions());

 foreach ($qids as $qid) {
  add_question_hints($qid,$num_hints);
 }
}

function allow_late_access($quizid,$on = 1) {
 global $DB;

 $a = $DB->get_record('quizaccess_addreview',array('quizid' => $quizid));

 if ($a) {
  $a->addreview = $on ? 1 : 0;
  $DB->update_record('quizaccess_addreview',$a);
 } else {
  $a = new stdClass();
  $a->quizid = $quizid;
  $a->addreview = $on ? 1 : 0;
  $DB->insert_record('quizaccess_addreview',$a);
 }
}

function make_adaptive($quizid) {
 global $DB;
 $quiz = quiz::create($quizid); 

 $q = $quiz->get_quiz();
 
 $q->preferredbehaviour = 'adaptive';
 
 $DB->update_record('quiz',$q);

 $sql_a = <<<SQL
SELECT a.id,a.uniqueid
FROM mdl_quiz_attempts a
WHERE a.quiz=:quizid AND a.preview=0
ORDER BY a.id
SQL;

 $params = array('quizid' => $quizid);
 
 $aids = $DB->get_records_sql($sql_a,$params);

 $sql_b = <<<SQL
SELECT b.id,b.behaviour
FROM mdl_question_attempts b
WHERE b.questionusageid=:quid
ORDER BY b.id
SQL;

 foreach($aids as $a) {
  $u = new stdClass();
  $u->id = $a->uniqueid;
  $u->preferredbehaviour = 'adaptive';
  $DB->update_record('question_usages',$u);
  $bb = $DB->get_records_sql($sql_b,['quid' => $a->uniqueid]);
  foreach($bb as $b) {
   $b->behaviour = 'adaptivemultipart';
   $DB->update_record('question_attempts',$b);
  }
 }
 
 return($q);
}

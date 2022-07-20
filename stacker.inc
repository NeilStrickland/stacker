<?php

namespace stacker;

require_once('../../config.php');

require_once($CFG->dirroot.'/mod/quiz/attemptlib.php');
require_once($CFG->dirroot.'/mod/quiz/accessmanager.php');
require_once($CFG->dirroot.'/question/engine/bank.php');
require_once($CFG->dirroot.'/question/format/xml/format.php');
require_once($CFG->dirroot.'/lib/questionlib.php');

require_once('parse_time.inc');

$source_dir = '/home/sa_pm1nps/Stack';

/* This class is just a container for methods related to compilation 
 * of stacker markup files.
 */

class compiler {
 public $log_level = 1;

 function log($s,$n=1) {
  if ($this->log_level >= $n) {
   echo $s . PHP_EOL;
  }
 }

 static function esc($s) {
  return htmlspecialchars($s, ENT_XML1, 'UTF-8');
 }

 /* A utility method for converting input option values to 0 or 1 
  * Note that the empty string is converted to 1, so |n| is equivalent
  * to |n=1|, for example.
  */
 function to01($s) {
  if ($s == '' || $s == '1' || $s == 'true' || $s == 1 || $s == true) {
   return 1;
  } else {
   return 0;
  }
 }

 /* This method searches through $s for substrings delimited by {!...!}
  * or {?...?}.  It returns an object $x with fields as follows:
  *
  * - $x->text is the same as $s but with {!...!} converted to 
  *   [[validation:...]] and {?...?} converted to 
  *   [[input:...]], combined with [[validation:...]] if there is
  *   no corresponding {!...!} sequence.
  *
  * - $x->inputs is a list of objects of class stacker\input 
  *   (declared later in this file) corresponding to the {?...?}
  *   sequences.
  *
  * - $x->setup is an array of strings that should be appended to 
  *   the "Question variables" section to set up the inputs. 
  *
  * In certain circumstances a default input element will be created,
  * but that is done by the compile_question() method.
  */
 
 function parse_inputs($s) {
  $inputs = array();
  $has_validation = array();
  $setup = array();
  $s0 = $s;

  /* Search for validation elements */
  while (preg_match("/{!([A-Za-z0-9_]+)!}/",$s0,$m)) {
   $name = $m[1];
   $ms = "[[validation:" . $name . "]]";
   $s0 = str_replace($m[0],$ms,$s0);
   $has_validation[$m[1]] = 1;
   //  echo "Validation element: $name" . PHP_EOL . PHP_EOL . $s0 . PHP_EOL; exit;
  }

  /* Search for input elements */
  while (preg_match("/{[?]([^?]*)[?]}/",$s0,$m)) {
   $m0 = $m[0]; // The full string including {? and ?}
   $m1 = explode('|',$m[1]);
   $m2 = array();
   foreach ($m1 as $e) {
    $i = strpos($e,'=');
    if ($i === false) {
     $e1 = array($e,'');
    } else {
     $e1 = array(substr($e,0,$i),substr($e,$i+1));
    }
    $m2[] = $e1;
   }
   $name = $m2[0][0]; // The name of the input (typically 'ans')
   $ip = new input($name);
   // $ta will be inserted in the "Model answer" box.
   // For algebraic or numerical inputs it really is the model answer,
   // but for multiple choice/multiple response/builder questions it
   // is more complicated than that.
   $ta = $m2[0][1];

   // If no model answer has been supplied explicitly we assume that
   // it has been assigned to the variable ta
   if ($ta == '') { $ta = 'ta'; }
   
   if (preg_match("/^M(C|D)(L?)\\((.*)\\)$/",$ta,$m)) {
    // This clause handles several types of multiple choice inputs
    // in which the correct answer is encoded in the option list
    $ip->type = ($m[1] == 'C') ? 'radio' : 'dropdown';
    if ($m[2] == 'L') {
     $ip->tans = "{$name}_options";
     $setup[] = "{$name}_options: mcq_add_labels({$m[3]});\n";
     $setup[] = "{$name}_correct: mcq_correct({$name}_options)[1];\n";
    } else {
     $ip->tans = $m[3];
     $setup[] = "{$name}_correct: mcq_correct({$m[3]})[1];\n";
    }
    $ip->prt_tans = $name . '_correct';
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^M(C|D)(L?)\\((.*)\\):(.*)$/",$ta,$m)) {
    // This clause handles several types of multiple choice inputs
    // in which the correct answer is given in the input specifier
    $ip->type = ($m[1] == 'C') ? 'radio' : 'dropdown';
    if ($m[2] == 'L') {
     $ip->tans = "{$name}_options";
     $ip->prt_tans = "{$name}_correct";
     $setup[] = "{$name}_options: mcq_add_labels(mcq_add_bool({$m[3]},{$m[4]}));";
     $setup[] = "{$name}_correct: mcq_correct({$name}_options)[1];";
    } else {
     $ip->tans = "{$name}_options";
     $ip->prt_tans = $m[4];
     $setup[] = "{$name}_options: mcq_add_bool({$m[3]},{$m[4]});";
    }
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^MR(L?)\\((.*)\\)$/",$ta,$m)) {
    // This clause handles several types of multiple response inputs
    // in which the correct answer is encoded in the option list
    $ip->type = 'checkbox';
    if ($m[1] == 'L') {
     $ip->tans = "{$name}_options";
     $ip->prt_tans = "{$name}_correct";
     $setup[] = "{$name}_options: mcq_add_labels({$m[2]});";
     $setup[] = "{$name}_correct: mcq_correct({$name}_options);";
    } else {
     $ip->tans = $m[2];
     $ip->prt_tans = $name . '_correct';
     $setup[] = "{$name}_correct: mcq_correct({$m[2]});";
    }
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^MR(L?)\\((.*)\\):(.*)$/",$ta,$m)) {
    // This clause handles several types of multiple response inputs
    // in which the correct answer is given in the input specifier
    $ip->type = 'checkbox';
    if ($m[1] == 'L') {
     $ip->tans = "{$name}_options";
     $ip->prt_tans = "{$name}_correct";
     $setup[] = "{$name}_options: mcq_add_labels(mrq_add_bool({$m[2]},{$m[3]}));";
     $setup[] = "{$name}_correct: mcq_correct({$name}_options);";
    } else {
     $ip->tans = "{$name}_options";
     $ip->prt_tans = "{$name}_correct";
     $setup[] = "{$name}_options: mrq_add_bool({$m[2]},{$m[3]});";
     $setup[] = "{$name}_correct: mcq_correct({$name}_options);";
    }
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^MATRIX:(.*)$/",$ta,$m)) {
    // This clause handles inputs where the student is asked to
    // fill in a matrix of fixed size.
    $ip->type = 'matrix';
    $ip->tans = $m[1];
    $ip->prt_tans = $m[1];
   } else if (preg_match("/^TF:(.*)$/",$ta,$m)) {
    // This clause handles true/false inputs
    $ip->type = 'radio';
    $ip->tans = "{$name}_options";
    $ip->prt_tans = "{$name}_correct";
    $setup[] = "{$name}_correct: mcq_to_01({$m[1]});";
    $setup[] = "{$name}_options: mcq_add_bool([[1,\"True\"],[0,\"False\"]],{$name}_correct);";
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^YN:(.*)$/",$ta,$m)) {
    // This clause handles yes/no inputs
    $ip->type = 'radio';
    $ip->tans = "{$name}_options";
    $ip->prt_tans = "{$name}_correct";
    $setup[] = "{$name}_correct: mcq_to_01({$m[1]});";
    $setup[] = "{$name}_options: mcq_add_bool([[1,\"Yes\"],[0,\"No\"]],{$name}_correct);";
    $ip->mustverify = 0;
    $ip->showvalidation = 0;
    $ip->prt_answertest = 'CasEqual';
   } else if (preg_match("/^B\\((.*)\\)$/",$ta,$m)) {
    // This clause handles builder inputs, where students are asked to
    // construct an answer (typically a proof) by selecting from a
    // menu of phrases.
    $ip->type = 'builder';
    $ip->tans = "{$name}_options";
    $ip->prt_tans = "{$name}_correct";
    $setup[] = "{$name}_options: {$m[1]};";
    $setup[] = "{$name}_correct: builder_correct({$name}_options);";
   } else {
    // This clause handles any inputs that have not been handled already.
    $ip->tans = $ta;
    $ip->prt_tans = $ta;
   }

   // Here we loop through any options that were provided in the
   // input specifier, separated by | characters
   for ($i = 1; $i < count($m2); $i++) {
    $e = $m2[$i];
    $key = $e[0];
    $val = $e[1];
    // For an option that is an equation, $key will be the LHS and $val will be the RHS
    // For an option that is not an equation, $key will be the option and $val will be empty
    if ($key == 'type'                      || $key == 't') {
     $ip->type = $val;
    } else if ($key == 'numerical'          || $key == 'n') { 
     $ip->type = 'numerical';
    } else if ($key == 'string'             || $key == '~') { 
     $ip->type = 'string';
     $ip->showvalidation = 0;
     $ip->mustverify = 0;
     $ip->hidevalidation = 1;
     $ip->prt_answertest = 'CasEqual';
    } else if ($key == 'singlechar'         || $key == '1') { 
     $ip->type = 'singlechar';
     $ip->showvalidation = 0;
     $ip->mustverify = 0;
     $ip->hidevalidation = 1;
     $ip->prt_answertest = 'CasEqual';
     $ip->boxsize = 1;
    } else if ($key == 'boxsize'            || $key == 's') { 
     $ip->boxsize = (int) $val;
    } else if ($key == 'strictsyntax'       || $key == 'x') { 
     $ip->strictsyntax = $this->to01($val);
    } else if ($key == 'laxsyntax'          || $key == '-x') { 
     $ip->strictsyntax = 1 - $this->to01($val);
    } else if ($key == 'insertstars'        || $key == '*') { 
     $ip->insertstars = $this->to01($val);
    } else if ($key == 'syntaxhint'         || $key == 'h') { 
     $ip->syntaxhint = $val;
    } else if ($key == 'syntaxattribute'    || $key == 'y') { 
     $ip->syntaxattribute = $this->to01($val);
    } else if ($key == 'forbidwords'        || $key == 'f') { 
     $ip->forbidwords = $val;
    } else if ($key == 'allowwords'         || $key == 'a') { 
     $ip->allowwords = $val;
    } else if ($key == 'forbidfloat'        || $key == '-.') { 
     $ip->forbidfloat = $this->to01($val);
    } else if ($key == 'allowfloat'         || $key == '.') { 
     $ip->forbidfloat = 1 - $this->to01($val);
    } else if ($key == 'requirelowestterms' || $key == '/') { 
     $ip->requirelowestterms = $this->to01($val);
    } else if ($key == 'mustverify'         || $key == 'v') { 
     $ip->mustverify = $this->to01($val);
    } else if ($key == 'showvalidation'     || $key == 'w') { 
     $ip->showvalidation = $this->to01($val);
    } else if ($key == 'hidevalidation'     || $key == 'z') { 
     $ip->showvalidation = 0;
     $ip->mustverify = 0;
     $ip->hidevalidation = 1;
    } else if ($key == 'opts'               || $key == 'o') { 
     $ip->opts = $val;
    } else {
     throw new \Exception("Invalid key: $key");
    }
   }

   if (isset($has_validation[$name])) {
    $ms = "[[input:{$name}]]";
   } else {
    if ($ip->hidevalidation) {
     $ms = "[[input:{$name}]]" .
         "<span style=\"display:none\">[[validation:{$name}]]</span>";
    } else {
     $ms = "[[input:{$name}]][[validation:{$name}]]";
    }
   }
  
   $inputs[] = $ip;
   $s0 = str_replace($m0,$ms,$s0);
  }
 
  $x = new \stdClass();
  $x->text = $s0;
  $x->inputs = $inputs;
  $x->setup = $setup;
  return $x;
 }

 /* This method is used to interpret if> clauses in the potential 
  * response tree.  It returns an object $x with fields 
  * $x->test, $x->sans and $x->tans.  In the default (but rare)
  * case, we expect $s to be like "Foo(a,b)", then the three fields
  * in $x will be "Foo", "a" and "b".  More commonly $s will be 
  * like "a = b" or "a == b" or "True(p)", and there is special 
  * treatment for these cases.
  */
 function parse_answer_test($s) {
  $s0 = $s;
  $s0 = str_replace(' ','',$s0);
  $s0 = str_replace("\t",'',$s0);
  $x = new \stdClass();
 
  if (preg_match("/^([A-Za-z]+)\\(([A-Za-z0-9_.]+),([A-Za-z0-9_.]+)\\)$/",$s0,$m)) {
   $x->test = $m[1]; $x->sans = $m[2]; $x->tans = $m[3]; return $x;
  } else if (preg_match("/^([A-Za-z0-9_.]+)=([A-Za-z0-9_.]+)$/",$s0,$m)) {
   $x->test = 'AlgEquiv'; $x->sans = $m[1]; $x->tans = $m[2]; return $x;
  } else if (preg_match("/^([A-Za-z0-9_.]+)==([A-Za-z0-9_.]+)$/",$s0,$m)) {
   $x->test = 'CasEqual'; $x->sans = $m[1]; $x->tans = $m[2]; return $x;
  } else if (preg_match("/^True\\(([A-Za-z0-9_.]+)\\)$/",$s0,$m)) {
   $x->test = 'CasEqual'; $x->sans = $m[1]; $x->tans = 'true'; return $x;
  } else {
   throw new \Exception("Invalid answer test: $s");
  }
 }

 /* This method is used to interpret score> flags in the potential
  * response tree.  Almost always, these have the form "score> 0" or
  * "score> 1", and the score is set to the stated value.  However,
  * there are a few cases where we say "score> +0.5", indicating that
  * the score, as calculated by previous layers of the PRT, should
  * be increased by 0.5.
  */
 function parse_score($s) {
  $s0 = $s;
  $s0 = str_replace(' ','',$s0);
  $s0 = str_replace("\t",'',$s0);
 
  if ($s0 == '') {
   throw new \Exception('Empty score');
  }
 
  $x = new \stdClass();
  
  $x->mode = '=';
  $m = substr($s0,0,1);
  
  if ($m == '=' || $m == '+' || $m == '!') {
   $s0 = substr($s0,1);
   $x->mode = ($m == '!') ? '-' : $m;
  }
 
  $x->val = (float) $s0;
 
  return $x;
 }

 /* Here $s is expected to be a line from a markup file, and
  * $indent should be the number of spaces of indentation that
  * we expect if this line is a continuation of the previous line.
  * The return value is an object $x with fields 
  * $x->flag, $x->text and $x->indent.  If $s starts with a
  * flag, then $x->flag will be that flag (with any initial
  * underscores removed), and $x->text will be the rest of $s 
  * after the flag (with initial spaces removed) and $x->indent
  * will be the amount of indentation expected for subsequent 
  * continuation lines.  If $s does not start with a flag then
  * it will be treated as a continuation line, and $x->flag will
  * be null.
  */
 function find_flag($s,$indent) {
  $x = new \stdClass();
  if (preg_match('/^(_*)([a-z]+>)( ?)(.*)$/',$s,$m)) {
   $x->flag = $this->full_flag($m[2]);
   $x->text = $m[4];
   $x->indent = strlen($m[1] . $m[2] . $m[3]);
  } else {
   $text = $s;
   if (trim(substr($text,0,$indent)) == '') {
    $text = substr($text,$indent);
   }

   $x->flag = null;
   $x->text = $text;
   $x->indent = $indent;
  }

  return $x;
 }

 /* Every flag has both a short form and a long form.  This 
  * method converts everything to the long form.
  */
 function full_flag($s) {
  if ($s == 'a>' || $s == 'answer>') {
   return 'answer>';
  } else if ($s == 'av>' || $s == 'answervariable>') {
   return 'answervariable>';
  } else if ($s == 'c>'  || $s == 'inputtype>') {
   return 'inputtype>';
  } else if ($s == 'f>'  || $s == 'forbid>') {
   return 'forbid>';
  } else if ($s == 'h>'  || $s == 'hide>') {
   return 'hide>';
  } else if ($s == 'k>'  || $s == 'keywords>') {
   return 'keywords>';
  } else if ($s == 'n>'  || $s == 'note>') {
   return 'note>';
  } else if ($s == 'o>'  || $s == 'options>') {
   return 'options>';
  } else if ($s == 'sb>' || $s == 'beginsol>') {
   return 'beginsol>';
  } else if ($s == 'se>' || $s == 'endsol>') {
   return 'endsol>';
  } else if ($s == 'mb>' || $s == 'beginmark>') {
   return 'beginmark>';
  } else if ($s == 'me>' || $s == 'endmark>') {
   return 'endmark>';
  } else if ($s == 'sc>' || $s == 'score>') {
   return 'score>';
  } else if ($s == 'an>' || $s == 'answernote>') {
   return 'answernote>';
  } else if ($s == 'fb>' || $s == 'feedback>') {
   return 'feedback>';
  } else if ($s == 't>'  || $s == 'type>') {
   return 'type>';
  } else if ($s == 'v>'  || $s == 'value>') {
   return 'value>';
  } else if ($s == 'pn>' || $s == 'penalty>') {
   return 'penalty>';
  } else if ($s == 'name>') {
   return 'name>';
  } else if ($s == 'keywords>') {
   return 'keywords>';
  } else if ($s == 'end>') {
   return 'end>';
  } else if ($s == 'if>') {
   return 'if>';
  } else if ($s == 'else>') {
   return 'else>';
  } else if ($s == 'fi>') {
   return 'fi>';
  } else {
   throw new \Exception("Invalid flag: $s");
  }
 }

 /* Here $s is a string which will typically be the contents of a 
  * markup file.  This method splits $s into lines, but it combines
  * continuation lines that are attached to the same flag, and 
  * ignores empty lines and comment lines.  It returns a list of
  * objects $x with fields $x->flag and $x->text.
  */
 function split_lines($s) {
  $lines = explode("\n",str_replace("\r",'',$s));
  $retseq = array();
  $flag = '';
  $line = '';
  $indent = 0;
 
  foreach ($lines as $l) {
   if (trim($l) == '' || substr($l,0,1) == '#') {
    continue;
   }

   $f = $this->find_flag($l,$indent);
   $indent = $f->indent;
  
   if ($f->flag != null) {
    // We have a new flag. First deal with the previous flag, if there was one.
    if ($flag != '') {
     $x = new \stdClass();
     $x->flag = $flag;
     $x->text = $line;
     $retseq[] = $x;
    }
    $flag = $f->flag;
    $line = $f->text;
   } else if ($flag != '') {
    // $f is a continuation line, attached to a flag that we read earlier.
    $line = $line . "\n" . $f->text;
   } else {
    throw new \Exception('First nonempty line does not start with a flag');
   }
  }

  $x = new \stdClass();
  $x->flag = $flag;
  $x->text = $line;
  $retseq[] = $x;
  
  return $retseq;
 }

 /* This applies split_lines() to the contents of a file */
 function read_lines($file) {
  $this->log("Reading file: $file");
  return $this->split_lines(file_get_contents($file));
 }

 /* This expects $lines to be an array of the type returned by 
  * split_lines().  This may encode several different questions, 
  * separated by flags of the form "end>".  This method returns
  * an array of arrays, with one array for each question.
  */
 function split_questions($lines) {
  $questions = array();
  $question = array();
  
  foreach($lines as $line) {
   if ($line->flag == 'end>') {
    if ($question) {
     $questions[] = $question;
    }
    
    $question = array();
   } else {
    $question[] = $line;
   }
  }
  if ($question) {
   $questions[] = $question;
  }
  
  return $questions;
 }

 /* This expects $lines to be an array as returned by split_lines(),
  * but with all lines pertaining to a single question.  It returns
  * an object of class stacker\question encapsulating information
  * about the corresponding question.  (This will later be used to
  * create a Moodle XML file, which can be imported into Moodle,
  * and the import process will create entries in a number of 
  * different database tables which together represent the question.)
  * 
  * The argument $name will be used as a default name for the
  * question, but it will be overridden by the name> flag if one
  * is present.  If $debug is 1 then the question will be compiled 
  * in a form where the solution and other information is displayed
  * in the question body, which is useful for debugging.
  */
 function compile_question($lines,$name = '',$debug = 0) {
  // question fields
  $opts = array();
  $defaultgrade = 1;
  $penalty = 0.1;
  $questiontext = array();
  $generalfeedback = array();
  $inputs = array();
  $prts = array();
  $keywords = array();

  // options fields
  $questionnote = '';
  $questionvariables = array();
  $specificfeedback = '';
  $numhints = 4;
  
  // default input fields
  $tans = '';
  $ansvar = 'ans1';
  $iptype = 'algebraic';
  $prompt = '';
  $postprompt = '';
  $forbidwords = array();
  $typespec = null;
  
  // prt fields
  $prt = new prt();
  $feedbackvariables = array();
  
  // other
  $input_setup = array();
  $answertest = '';
  
  $question = new question();
  $opts = $question->opts;
  
  // The variable $state will change later to "mark" or "solution"
  // as we process flags related to the marking procedure or the
  // worked solution.
  $state = 'statement';
  
  $flagnum = 0;
  foreach ($lines as $line) {
   $flagnum++;
   $flag = $line->flag;
   $text = $line->text;
   
   if ($state == "statement") {
    if ($flag == "name>") {
     // Specify the question name
     $name = trim($text);
    } else if ($flag == "keywords>") {
     // We have not been using this
     $keywords = array_merge($keywords,explode(',',$text));
    } else if ($flag == "value>") {
     // Specify the maximum possible mark for the question
     // We have almost always accepted the default value of 1
     $defaultgrade = (float) $text;
    } else if ($flag == "penalty>") {
     // Specify the penalty for an incorrect attempt
     // We have usually accepted the default value of 0.1
     $penalty = '' . ((float) $text);
    } else if ($flag == "answer>") {
     // Specify the correct answer
     // This is deprecated in favour of input specifiers like {?ans=ta?}
     $tans = $text;
     $questiontext[] = "__DEFAULT_INPUT__";
    } else if ($flag == "answervariable>") {
     // Specify the name of the answer variable
     // This is deprecated in favour of input specifiers like {?ans=ta?}
     $ansvar = trim(text);
     if ($ansvar == '') { $ansvar = 'ans1'; }
    } else if ($flag == "note>") {
     // Specify the question note
     // This is important in the case of questions with randomisation,
     // and sometimes convenient for other questions that introduce
     // many unimportant intermediate variables
     $questionnote = $text;
    } else if ($flag == "options>") {
     // We have rarely used this
     $opts->parse($text);
    } else if ($flag == "inputtype>") {
     // This is deprecated in favour of input specifiers
     $typespec = trim($text);
    } else if ($flag == "forbid>") {
     // Specify words that are forbidden in the student's answer
     $forbidwords[] = $text;
    } else if ($flag == "hide>") {
     // Maxima code to be inserted in the question variables or feedback variables
     $questionvariables[] = $text;
    } else if ($flag == "type>") {
     // Question text or solution text
     $x = $this->parse_inputs($text);
     $inputs = array_merge($inputs,$x->inputs);
     $input_setup = array_merge($input_setup,$x->setup);
     $questiontext[] = $x->text;
    } else if ($flag == "numhints>") {
     // Number of hints
     // We need hints if we use the question behaviour
     // "Interactive with multiple tries".  However, our default is
     // now "Adaptive mode", and this makes the hints redundant.
     $numhints = 0 + $text;
    } else if ($flag == "beginsol>") {
     // Beginning of the solution section (which becomes "General feedback")
     $state = "solution";
    } else if ($flag == "endsol>") {
     // End of the solution section; this is an error as $state = "statement"
     $x = new \stdClass();
     $x->is_error = true;
     $x->question_name = $name;
     $x->flagnum = $flagnum;
     $x->lines = $lines;
     $x->msg = "se> or endsol> flag before corresponding sb> or beginsol>";
     return $x;
    } else if ($flag == "beginmark>") {
     // Beginning of a marking section (which becomes a potential response tree)
     $state = "mark";
     $node = null;
     $bool = null;
    } else if ($flag == "endmark>") {
     // End of a marking section; this is an error as $state = "statement"
     $x = new \stdClass();
     $x->is_error = true;
     $x->question_name = $name;
     $x->flagnum = $flagnum;
     $x->lines = $lines;
     $x->msg = "me> or endmark> flag before corresponding mb> or beginmark>";
     return $x;
    }
   } else if ($state == "solution") {
    if ($flag == "hide>") {
     $questionvariables[] = $text;
    } else if ($flag == "type>") {
     $generalfeedback[] = $text;
    } else if ($flag == "endsol>") {
     // End of the solution section
     $state = "statement";
    } else {
     $x = new \stdClass();
     $x->is_error = true;
     $x->question_name = $name;
     $x->flagnum = $flagnum;
     $x->lines = $lines;
     $x->msg = "Invalid flag in solution section: $flag";
     return $x;
    }
   } else if ($state == "mark") {
    if ($flag == "hide>") {
     $feedbackvariables[] = $text;
    } else if ($flag == "if>") {
     // An if> flag corresponds to a new PRT node
     $parent = $node;
     $node = $prt->new_node();
     $node->parent = $parent;
     if ($parent) {
      $parent->set_nextnode($bool,$node->nodename);
      $node->bool = $bool;
      $node->truescore = 0;
     }
     $x = $this->parse_answer_test($text);
     $node->answertest = $x->test;
     $node->sans = $x->sans;
     $node->tans = $x->tans;
     $bool = true;
    } else if ($flag == "else>") {
     $bool = false;
    } else if ($flag == "score>") {
     $x = $this->parse_score($text);
     $node->set_scoremode($bool,$x->mode);
     $node->set_score($bool,$x->val);
    } else if ($flag == "penalty>") {
     $node->set_penalty($bool,$text);
    } else if ($flag == "feedback>") {
     $node->set_feedback($bool,$text);
    } else if ($flag == "answernote>") {
     $node->set_answernote($bool,$text);
    } else if ($flag == "fi>") {
     $node = $node->parent;
    } else if ($flag == "endmark>") {
     // End of a marking section
     $prt->feedbackvariables = implode("\n",$feedbackvariables);
     $feedbackvariables = array();
     if (! $prt->nodes) {
      $ip = end($inputs);
      $node = $prt->new_node();
      $node->answertest = $ip->prt_answertest;
      $node->sans = $ip->name;
      $node->tans = $ip->prt_tans;
      $node->falsepenalty = $penalty;
      $prt->nodes[] = $node;
     }
     
     $prts[] = $prt;
     $n = count($prts) + 1;
     $prt = new prt();
     $prt->name = 'prt' . $n;
     $state = "statement";
    }
   }
  }
  
  $questiontext = implode("\n",$questiontext);
  $forbidwords = implode(',',$forbidwords);
  
  if (! $inputs) {
   // If there are no input elements, then we create one with default
   // options.  This is deprecated.
   $this->log("No explicit input elements",2);
   $this->log("typespec:" . PHP_EOL . $typespec . PHP_EOL,2);
   $ip = new input();
   $ip->tans = $tans;
   $ip->prt_tans = $tans;
   $ip->name = $ansvar;
   
   if ($typespec) {
    if (preg_match("/^MC\\((.*)\\)$/sm",$typespec,$m)) {
     $this->log("Input type MC()",2);
     $ip = new input();
     $ip->set_MC();
     $ip->name = $ansvar;
     $ip->tans = $ansvar . '_options';
     $ip->prt_tans = $ansvar . '_correct';
     
     $input_setup[] = "{$ansvar}_options: mcq_options([{$m[1]}],{$tans});";
     $input_setup[] = "{$ansvar}_correct: {$tans};";
    } elseif (preg_match("/^MCL\\((.*)\\)$/sm",$typespec,$m)) {
     $this->log("Input type MCL()",2);
     $ip = new input();
     $ip->set_MC();
     $ip->name = $ansvar;
     $ip->tans = $ansvar . '_options';
     $ip->prt_tans = $ansvar . '_correct';
     
     $input_setup[] = "{$ansvar}_options: mcq_add_labels(mcq_add_bool([{$m[1]}],{$tans}));";
     $input_setup[] = "{$ansvar}_correct: mcq_correct({$ansvar}_options)[1];";
    } else if (preg_match("/^MR\\((.*)\\)$/sm",$typespec,$m)) {
     $this->log("Input type MR()",2);
     $ip = new input();
     $ip->name = $ansvar;
     $ip->set_MR();
     $ip->tans = $ansvar . '_options';
     $ip->prt_tans = $ansvar . '_correct';
     
     $input_setup[] = "{$ansvar}_options: mrq_options({$m[1]},{$tans});";
     $input_setup[] = "{$ansvar}_correct: {$tans};";
    } else if (preg_match("/^MRL\\((.*)\\)$/sm",$typespec,$m)) {
     $this->log("Input type MRL()",2);
     $ip = new input();
     $ip->name = $ansvar;
     $ip->set_MR();
     $ip->tans = $ansvar . '_options';
     $ip->prt_tans = $ansvar . '_correct';
     
     $input_setup[] = "{$ansvar}_options: mcq_add_labels(mrq_add_bool([{$m[1]}],{$tans}));";
     $input_setup[] = "{$ansvar}_correct: mcq_correct({$ansvar}_options);";
    }
   }
   
   $inputs[] = $ip;
   
   $s = "[[input:{$ip->name}]][[validation:{$ip->name}]]";
   
   if ($ip->hidevalidation) {
    $s = "[[input:{$ip->name}]]" .
       "<span style=\"display:none\">[[validation:{$ip->name}]]</span>";
   }
   
   $questiontext = str_replace('__DEFAULT_INPUT__',$s,$questiontext);
  }
  
  if ($forbidwords) {
   foreach($inputs as $ip) {
    if (! $ip->forbidwords) {
     $ip->forbidwords = $forbidwords;
    }
   }
  }
  
  // If there are no PRTs, then we create one with default options.
  // This is mildly deprecated.
  if (! $prts) {
   $ip = end($inputs);
   $node = $prt->new_node();
   $node->answertest = $ip->prt_answertest;
   $node->sans = $ip->name;
   $node->tans = $ip->prt_tans;
   $node->falsepenalty = $penalty;
   $prt->nodes[] = $node;
   $prts[] = $prt;
  }

  // For the "Question variables" section, we use the explicitly
  // supplied setup code combined with some extra things needed to
  // set up multiple choice, multiple response and builder input
  // elements.
  $opts->questionvariables =
    implode("\n",$questionvariables) . "\n" .
    implode("\n",$input_setup);
  
  $opts->questionnote = $questionnote;
  
  $opts->specificfeedback = '';
  foreach ($prts as $prt) {
   $opts->specificfeedback .= "[[feedback:{$prt->name}]]<br/>";
  }
  
  $question->name = $name;
  $question->questiontext = $questiontext;
  $question->inputs = $inputs;
  $question->prts = $prts;
  
  $question->keywords = $keywords;
  $question->defaultgrade = $defaultgrade;
  $question->penalty = $penalty;
  
  $question->generalfeedback = implode("\n",$generalfeedback);
  $question->numhints = $numhints;
  
  // In debug mode, we add extra information to the question text
  if ($debug &&
      count($prts) == 1 &&
      preg_match('/m[rc]_feedback\(ans,opts/',$prts[0]->feedbackvariables)) {
   $question->questiontext .= "\n<br/><hr/>{@show_opts(opts)@}"; 
  }
  
  if ($debug && trim($question->generalfeedback)) {
   $question->questiontext .= "\n<br/><hr/>\n" . $question->generalfeedback; 
  }
  
  return $question;
 }

 /* This method expects $lines to be an array as returned by split_lines()
  * It returns an array of objects of class stacker\question, as well 
  * as an array of error messages.
  */
 function compile_lines($lines,$debug = 0) {
  $questions0 = $this->split_questions($lines);
  $questions = array();
  $errors = array();
  
  $i = 0;
  foreach($questions0 as $x) {
   $i++;
   $q = $this->compile_question($x,'',$debug);
   if (isset($q->is_error) && $q->is_error) {
    $errors[] = $q;
   } else {
    if ($q->name == '') {
     $ii = '' . $i;
     if (strlen($ii) < 2) { $ii = '0' . $ii; }
     $q->name = 'Q' . $ii;
    }
    $questions[] = $q;
   }
  }

  return(array($questions,$errors));
 }

 function compile_string($s,$debug = 0) {
  return $this->compile_lines($this->split_lines($s),$debug);
 }

 function compile_file($f,$debug=0) {
  $this->log("Compiling file: $f",1);
  if (! file_exists($f)) {
   $questions = array();
   $errors = array("File $f does not exist");
   return(array($questions,$errors));
  }
  $s = file_get_contents($f);
  return $this->compile_string($s,$debug);
 }
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class question {
 public $id = 0;
 public $idnumber = 0;
 public $hidden = 0;
 public $name = '';
 public $opts = null;
 public $defaultgrade = 1.0;
 public $penalty = 0.1;
 public $questiontext = '';
 public $questiontextformat = 'html';
 public $generalfeedback = '';
 public $generalfeedbackformat = 'html';
 public $numhints = 4;
 public $inputs = null;
 public $prts = null;

 function __construct($name = '') {
  $this->opts = new options();
  $this->name = $name;
 }

 function to_xml($doc_ = null) {
  $doc = $doc_;
  if (! $doc) { $doc = new \DOMDocument(); }
  
  $ox = $this->opts->xml_nodes($doc);
  $x = $doc->createElement('question');
  $x->setAttribute('type','stack');
  $x->appendChild($n = $doc->createElement('name'));
  $n->appendChild($t = $doc->createElement('text',\stacker\compiler::esc($this->name)));
  $x->appendChild($n = $doc->createElement('questiontext'));
  $n->setAttribute('format',$this->questiontextformat);
  $n->appendChild($t = $doc->createElement('text',\stacker\compiler::esc($this->questiontext)));
  $x->appendChild($n = $doc->createElement('generalfeedback'));
  $n->setAttribute('format',$this->generalfeedbackformat);
  $n->appendChild($t = $doc->createElement('text',\stacker\compiler::esc($this->generalfeedback)));
  $x->appendChild($n = $doc->createElement('defaultgrade',$this->defaultgrade));
  $x->appendChild($n = $doc->createElement('penalty',$this->penalty));
  $x->appendChild($n = $doc->createElement('hidden',$this->hidden ? 1 : 0));
  $x->appendChild($n = $doc->createElement('idnumber',$this->idnumber ? $this->idnumber : ''));

  $keys = array('stackversion','questionvariables','specificfeedback','questionnote',
                'questionsimplify','assumepositive','assumereal',
                'prtcorrect','prtpartiallycorrect','prtincorrect',
                'multiplicationsign','sqrtsign','complexno','inversetrig',
                'matrixparens','variantsselectionseed');

  foreach($keys as $key) { $x->appendChild($ox->$key); }

  foreach($this->inputs as $ip) {
   $x->appendChild($ip->to_xml($doc));
  }

  for ($i = 0; $i < $this->numhints; $i++) {
   $x->appendChild($h = $doc->createElement('hint'));
   $h->setAttribute('format','html');
   $h->appendChild($doc->createElement('text',\stacker\compiler::esc('<p>-</p>')));
  }

  foreach($this->prts as $prt) {
   $x->appendChild($prt->to_xml($doc));
  }
  
  return $x;
 }

 function to_xml_string($doc_ = null) {
  $doc = $doc_;
  if (! $doc) { $doc = new \DOMDocument(); }
  $x = $this->to_xml($doc);
  return $doc->saveXML($x);
 }
}

//////////////////////////////////////////////////////////////////////

class options {
 public $assumepositive = false;
 public $assumereal = false;
 public $complexno = "i";
 public $inversetrig = "arccos";
 public $matrixparens = "[";
 public $multiplicationsign = "dot";
 public $prtcorrect =
     "<span style=\"font-size: 1.5em; color:green;\"><i class=\"fa fa-check\"></i></span> Correct answer,well done";
 public $prtcorrectformat = "html";
 public $prtincorrect = "<span style=\"font-size: 1.5em; color:red;\"><i class=\"fa fa-times\"></i></span> Incorrect answer.";
 public $prtincorrectformat = "html";
 public $prtpartiallycorrect = "<span style=\"font-size: 1.5em; color:orange;\"><i class=\"fa fa-adjust\"></i></span> Your answer is partially correct.";
 public $prtpartiallycorrectformat = "html";
 public $questionid = "";
 public $questionnote = "";
 public $questionsimplify = true;
 public $questionvariables = "";
 public $specificfeedback = "[[feedback:prt1]]";
 public $specificfeedbackformat = "html";
 public $sqrtsign = "1";
 public $stackversion = "2019052700";
 public $variantsselectionseed = "";

 function parse($s) {
  $oo = json_decode($s,true);

  foreach(get_class_vars('stacker\options') as $v => $d) {
   if (array_key_exists($v,$oo)) {
    $this->$v = $oo[$v];
   }
  }
 }
 
 function xml_nodes($doc) {
  $x = new \stdClass();

  foreach(array('assumepositive','assumereal','questionsimplify') as $k) {
   $x->$k = $doc->createElement($k,$this->$k ? 1 : 0);
  }
  
  foreach(array('complexno','inversetrig','matrixparens','multiplicationsign',
                'sqrtsign','variantsselectionseed') as $k) {
   $x->$k = $doc->createElement($k,$this->$k);
  }

  foreach(array('prtcorrect','prtpartiallycorrect','prtincorrect','specificfeedback') as $k) {
   $kf = $k . 'format';
   $x->$k = $doc->createElement($k);
   $x->$k->setAttribute('format',$this->$kf);
   $x->$k->appendChild($doc->createElement('text',\stacker\compiler::esc($this->$k)));
  }

  foreach(array('stackversion','questionvariables','questionnote') as $k) {
   $kf = $k . 'format';
   $x->$k = $doc->createElement($k);
   $x->$k->appendChild($doc->createElement('text',\stacker\compiler::esc($this->$k)));
  }

  return $x;
 }
}

//////////////////////////////////////////////////////////////////////

class input {
 public $id = 0;
 public $questionid = 0;
 public $name = 'ans1';
 public $type = 'algebraic';
 public $tans = 'ta';
 public $prt_tans = 'ta';
 public $prt_answertest = 'AlgEquiv';
 public $boxsize = 15;
 public $strictsyntax = 1;
 public $insertstars = 0;
 public $syntaxhint = '';
 public $syntaxattribute = 0;
 public $forbidwords = '';
 public $allowwords = '';
 public $forbidfloat = 1;
 public $requirelowestterms = 0;
 public $checkanswertype = 0;
 public $mustverify = 1;
 public $showvalidation = 1;
 public $hidevalidation = 0;
 public $options = '';
 
 function __construct($name = '',$type = 'algebraic',$tans = 'ta') {
  $this->name = $name;
  $this->type = $type;
  $this->tans = $tans;
 }

 function set_MC() {
  $this->type = 'radio';
  $this->mustverify = 0;
  $this->showvalidation = 0;
  $this->hidevalidation = 1;
  $this->prt_answertest = 'CasEqual';
 }

 function set_MR() {
  $this->type = 'checkbox';
  $this->mustverify = 0;
  $this->showvalidation = 0;
  $this->hidevalidation = 1;
  $this->prt_answertest = 'CasEqual';
 }

 function to_xml($doc) {
  $x = $doc->createElement('input');

  foreach(array('name','type','tans','boxsize','strictsyntax','insertstars',
                'syntaxhint','syntaxattribute','forbidwords','allowwords',
                'forbidfloat','requirelowestterms','checkanswertype','mustverify',
                'showvalidation','options') as $k) {
   $x->appendChild($doc->createElement($k,\stacker\compiler::esc($this->$k)));
  }

  return $x;
 }
}

//////////////////////////////////////////////////////////////////////

class prt {
 public $id = 0;
 public $questionid = 0;
 public $name = 'prt1';
 public $value = 1.0;
 public $autosimplify = 0;
 public $feedbackstyle = 0;
 public $feedbackvariables = '';
 public $nodes = null;

 function __construct() {
  $this->nodes = array();
 }

 function new_node() {
  $n = count($this->nodes);
  $x = new prtnode();
  $x->nodename = '' . $n;
  $x->prtname = $this->name;
  $x->trueanswernote  = "{$this->name}-{$n}-T";
  $x->falseanswernote = "{$this->name}-{$n}-F";
  $this->nodes[] = $x;
  return $x;
 }

 function to_xml($doc) {
  $x = $doc->createElement('prt');
  $x->appendChild($doc->createElement('name',$this->name));
  $x->appendChild($doc->createElement('value',$this->value));
  $x->appendChild($doc->createElement('autosimplify',$this->autosimplify));
  $x->appendChild($y = $doc->createElement('feedbackvariables'));
  $y->appendChild($doc->createElement('text',\stacker\compiler::esc($this->feedbackvariables)));

  foreach($this->nodes as $n) {
   $x->appendChild($n->to_xml($doc));
  }

  return $x;
 }
}

//////////////////////////////////////////////////////////////////////

class prtnode {
 public $id = 0;
 public $questionid = 0;
 public $prtname = "";
 public $nodename = 0;
 public $answertest = "AlgEquiv";
 public $sans = "ans1";
 public $tans = "ta";
 public $testoptions = "";
 public $quiet = 0;
 public $truescoremode = "=";
 public $truescore = 1.;
 public $truepenalty = '';
 public $truenextnode = -1;
 public $trueanswernote = "prt1-1-T";
 public $truefeedback = "";
 public $truefeedbackformat = "html";
 public $falsescoremode = "=";
 public $falsescore = 0.;
 public $falsepenalty = '';
 public $falsenextnode = -1;
 public $falseanswernote = "prt1-1-F";
 public $falsefeedback = "";
 public $falsefeedbackformat = "html";
 public $parent = null;
 public $bool = true;

 function set_bool($b,$f,$x) {
  $bf = ($b ? 'true' : 'false') . $f;
  $x0 = $x;
  if ($f == 'score' || ($f == 'penalty' && $x0 != '')) {
   if ($x0 != '') {
    $x0 = '' . ((float) $x0);
   }
  }
  $this->$bf = $x0;
 }

 function set_scoremode ($b,$x) { $this->set_bool($b,'scoremode' ,$x); }
 function set_score     ($b,$x) { $this->set_bool($b,'score'     ,$x); }
 function set_penalty   ($b,$x) { $this->set_bool($b,'penalty'   ,$x); }
 function set_nextnode  ($b,$x) { $this->set_bool($b,'nextnode'  ,$x); }
 function set_answernote($b,$x) { $this->set_bool($b,'answernote',$x); }
 function set_feedback  ($b,$x) { $this->set_bool($b,'feedback'  ,$x); }

 function to_xml($doc) {

  $x = $doc->createElement('node');

  $x->appendChild($doc->createElement('name',$this->nodename));

  foreach(array('answertest','sans','tans','testoptions','quiet',
                'truescoremode','truescore','truepenalty','truenextnode','trueanswernote',
                'falsescoremode','falsescore','falsepenalty','falsenextnode','falseanswernote')
          as $k) {
   $x->appendChild($doc->createElement($k,\stacker\compiler::esc('' . $this->$k)));
  }

  $x->appendChild($n = $doc->createElement('truefeedback'));
  $n->setAttribute('format',$this->truefeedbackformat);
  $n->appendChild($doc->createElement('text',\stacker\compiler::esc($this->truefeedback)));
  
  $x->appendChild($n = $doc->createElement('falsefeedback'));
  $n->setAttribute('format',$this->falsefeedbackformat);
  $n->appendChild($doc->createElement('text',\stacker\compiler::esc($this->falsefeedback)));

  return $x;
 }
}

//////////////////////////////////////////////////////////////////////

class quiz {
 public $name = '';
 public $file_name = '';
 public $category_name = '';
 public $category_info = '';
 public $questions = [];
 public $source_dir = '';
 public $stack_dir = '';
 public $xml_dir = '';
 public $moodle_quiz = null;

 function fill($moodle_quiz) {
  $this->moodle_quiz = $moodle_quiz;
  $this->raw_quiz = $moodle_quiz->get_quiz();
  $this->name = $moodle_quiz->get_quiz_name();
  $this->id = $moodle_quiz->get_quizid();

  $i = strpos($this->name,':');
  if ($i === false) {
   $sn = $this->name;
  } else {
   $sn = substr($this->name,0,$i);
  }

  $this->short_name = $sn;
   
  $this->set_file_name($this->name);
 }

 function set_dirs($source_dir) {
  $this->source_dir = $source_dir;
  $this->stack_dir = $this->source_dir . '/stack';
  $this->xml_dir = $this->source_dir . '/xml';
  $this->stack_file_name = $this->name . '.stack';
  $this->xml_file_name = $this->name . '.xml';
  $this->full_stack_file_name = $this->stack_dir . '/' . $this->stack_file_name;
  $this->full_xml_file_name = $this->xml_dir . '/' . $this->xml_file_name;
 }

 function set_file_name($file_name) {
  $this->base_file_name = $file_name;
  $this->stack_file_name = $file_name . '.stack';
  $this->xml_file_name = $file_name . '.xml';
  $this->full_stack_file_name = $this->stack_dir . '/' . $this->stack_file_name;
  $this->full_xml_file_name = $this->xml_dir . '/' . $this->xml_file_name;
 }
 
 function get_category_name() {
  if ($this->category_name) {
   return $this->category_name;
  } else {
   return 'Default for ' . $this->name;
  }
 }

 function get_category_info() {
  if ($this->category_info) {
   return $this->category_info;
  } else {
   return "The default category for questions shared in context '{$this->name}'.";
  }
 }

 function get_default_category() {
  global $DB;
 
  $quiz = $this->moodle_quiz;
  $context = $quiz->get_context();
  $cat_name = 'Default for ' . $quiz->get_quiz_name();

  $cat = $DB->get_record('question_categories',
                        array('name' => $cat_name,
                              'contextid' => $context->id));

  if (! $cat) {
   // echo "No category with name $cat_name in context {$context->id}" . PHP_EOL;
  
   $top_cat = $DB->get_record('question_categories',
                              array('name' => 'top',
                                    'contextid' => $context->id));
   if (! $top_cat) {
    // echo "No category with name top in context {$context->id}" . PHP_EOL;
    $top_cat = new \stdClass();
    $top_cat->name = 'top';
    $top_cat->info = '';
    $top_cat->sortorder = 0;
    $top_cat->contextid = $context->id;
    $top_cat->stamp = make_unique_id_code();
    $top_cat->id = $DB->insert_record('question_categories',$top_cat);
   }
  
   $cat = new \stdClass();
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

 function use_default_category($short_name = '') {
  global $DB;

  if (\quiz_has_attempts($this->id)) {
    throw new \Exception("Quiz has attempts, cannot use default category");
  }
  
  $quiz = $this->moodle_quiz;
  $quiz->preload_questions();
  $quiz->load_questions();
  $structure = $quiz->get_structure();
  
  $context = $quiz->get_context();

  if ($short_name) {
   $cat_name = 'Default for ' . $short_name;
  } else {
   $cat_name = 'Default for ' . $quiz->get_quiz_name();
  }
 
  $category = $this->get_default_category();

  if ($category) {
   $finder = \question_finder::get_instance();
  
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
   quiz_repaginate_questions($q->id, 4);
  } else {
   throw new \Exception("Category '$cat_name' does not exist" . PHP_EOL);
  }
 }

 /* Set quiz options to the SoMaS defaults
  */
 function set_options($save = true) {
  global $DB;
  
  $raw_quiz = $this->moodle_quiz->get_quiz();
 
  $raw_quiz->preferredbehaviour = 'adaptive';
  $raw_quiz->reviewattempt = 0x11110;
  $raw_quiz->reviewcorrectness = 0x11110;
  $raw_quiz->reviewmarks = 0x11110;
  $raw_quiz->reviewspecificfeedback = 0x11110;
  $raw_quiz->reviewgeneralfeedback = 0x10;
  $raw_quiz->reviewrightanswer = 0x10;
  $raw_quiz->reviewoverallfeedback = 0x10;
  $raw_quiz->attempts = 1;

  if ($save) {
   $DB->update_record('quiz',$raw_quiz);
  }
  
  return($raw_quiz);
 }

 /* Set quiz options to the SoMaS defaults for quizzes under development
  */
 function set_dev_options($save = true) {
  global $DB;

  $raw_quiz = $this->moodle_quiz->get_quiz();

  $raw_quiz->preferredbehaviour = 'interactive';
  $raw_quiz->reviewattempt = 0x11110;
  $raw_quiz->reviewcorrectness = 0x11110;
  $raw_quiz->reviewmarks = 0x11110;
  $raw_quiz->reviewspecificfeedback = 0x11110;
  $raw_quiz->reviewgeneralfeedback = 0x1110;
  $raw_quiz->reviewrightanswer = 0x1110;
  $raw_quiz->reviewoverallfeedback = 0x1110;
  $raw_quiz->timeclose = strtotime(date('Y') . '-12-31');
  
  if ($save) {
   $DB->update_record('quiz',$raw_quiz);
  }
  
  return($raw_quiz);
 }

 /* If we want to use the question behaviour "Interactive with multiple tries",
  * each question has to have attached hints, with as many hints as the
  * number of tries that we want to permit.  This method adds hints with no
  * content, which is OK.  However, we now use "Adaptive mode" by default,
  * and this makes the hints redundant.
  */
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
   $hint = new \stdClass();
   $hint->questionid = $qid;
   $hint->hint = '<p>-</p>';
   $hint->hintformat = 1;
   $hint->id = $DB->insert_record('question_hints', $hint);
  }
 }

 /* Add hints to all questions in the quiz
  */
 function add_hints($num_hints) {
  $quiz = $this->moodle_quiz;
  $quiz->preload_questions();
  $quiz->load_questions();
  $qids = array_keys($quiz->get_questions());
  
  foreach ($qids as $qid) {
   $this->add_question_hints($qid,$num_hints);
  }
 }

 /* By default, if a student never accesses a quiz when it is open, then
  * they cannot access it after it closes to see the solutions.  The
  * purpose of this method is to override that rule.
  */
 function allow_late_access($on = 1) {
  global $DB;

  $id = $this->moodle_quiz->get_quizid();
  $a = $DB->get_record('quizaccess_addreview',array('quizid' => $id));

  if ($a) {
   $a->addreview = $on ? 1 : 0;
   $DB->update_record('quizaccess_addreview',$a);
  } else {
   $a = new \stdClass();
   $a->quizid = $id;
   $a->addreview = $on ? 1 : 0;
   $DB->insert_record('quizaccess_addreview',$a);
  }
 }

 /* This method switches all questions in the quiz to use "Adaptive mode"
  * question behaviour.  We now use that mode by default, so this method
  * should not be necessary.
  */
 function make_adaptive() {
  global $DB;

  $quiz = $this->moodle_quiz; 
  $id = $quiz->get_quizid();
  $raw_quiz = $quiz->get_quiz();
 
  $raw_quiz->preferredbehaviour = 'adaptive';
 
  $DB->update_record('quiz',$raw_quiz);

  $sql_a = <<<SQL
SELECT a.id,a.uniqueid
FROM mdl_quiz_attempts a
WHERE a.quiz=:quizid AND a.preview=0
ORDER BY a.id
SQL;

  $params = array('quizid' => $id);
 
  $aids = $DB->get_records_sql($sql_a,$params);

  $sql_b = <<<SQL
SELECT b.id,b.behaviour
FROM mdl_question_attempts b
WHERE b.questionusageid=:quid
ORDER BY b.id
SQL;

  foreach($aids as $a) {
   $u = new \stdClass();
   $u->id = $a->uniqueid;
   $u->preferredbehaviour = 'adaptive';
   $DB->update_record('question_usages',$u);
   $bb = $DB->get_records_sql($sql_b,['quid' => $a->uniqueid]);
   foreach($bb as $b) {
    $b->behaviour = 'adaptivemultipart';
    $DB->update_record('question_attempts',$b);
   }
  }
 
  return($raw_quiz);
 }

 /* Get the base name of the markup file where this question is stored.
  * This will not include the extension .stack or the directory name.
  */
 function get_file_name() {
  if ($this->file_name) {
   return $this->file_name;
  } else {
   return $this->name;
  }
 }

 /* The stacker markup file for a quiz gets converted to a Moodle XML
  * file which can be imported by Moodle.  That XML file has a
  * <quiz> tag which contains a number of <question> tags.  The first
  * <question> tag is not really a question, but a record of the
  * category in which the questions should be inserted.  This method
  * creates that fake <question> tag. 
  */
 function category_question_xml($doc) {
  $cn = $this->get_category_name();
  $ci = $this->get_category_info();
  $x = $doc->createElement('question');
  $x->setAttribute('type','category');
  $x->appendChild($c = $doc->createElement('category'));
  $c->appendChild($doc->createElement('text','$module$/top/' . $cn));
  $x->appendChild($i = $doc->createElement('info'));
  $i->setAttribute('format','moodle_auto_format');
  $i->appendChild($doc->createElement('text',$ci));

  return $x;
 }

 /* If there are previews of this quiz in the database, then this method
  * returns the first preview.  Note that if the quiz is updated using
  * the compilation system then all previews will be deleted.  Thus, if
  * a preview exists it implies that someone has looked at the current
  * version.
  */
 function get_preview() {
  global $DB;
  
  $this->preview = null;
  $this->preview_time = null;

  if (! $this->moodle_quiz) { return null; }
  $id = $this->moodle_quiz->get_quizid();

  $pp = $DB->get_records('quiz_attempts',
                         array('quiz' => $this->moodle_quiz->get_quizid(),
                               'preview' => 1));

  if ($pp) {
   $this->preview = reset($pp);
   $this->preview_time = $this->preview->timestart;
  }

  return $this->preview;
 }

 /* This returns the time at which the markup file for this question
  * was last modified.
  */
 function get_file_time() {
  $this->file_time = 0;
  $file = $this->full_stack_file_name;
  if (! file_exists($file) && $this->moodle_quiz) {
   $this->set_file_name($this->short_name);
   $file = $this->full_stack_file_name;
  }

  if (file_exists($file)) {
   $this->file_time = filemtime($file);
  }

  return $this->file_time;
 }

 /* This method returns a string that describes the modification status
  * of this quiz.  The algorithm is not completely robust.
  *
  *  - "uncompiled" suggests that the markup file has been modified 
  *    but that these changes have not been incorporated in the database.
  *  - "modified" suggests that one or more questions have been modified
  *    via the web interface, and that these changes have not been 
  *    incorporated in the markup file
  *  - "unchecked" suggests that there is at least one question whose
  *    current version has not been previewed by anyone.
  */
 function get_status() {
  $this->get_preview();
  $pt = $this->preview_time;
  $ft = $this->get_file_time();

  $ct = time();
  $mt = 0;

  if ($this->moodle_quiz) {
   foreach($this->moodle_quiz->get_questions() as $q) {
    $ct = min($ct,$q->timecreated);
    $mt = max($mt,$q->timemodified);
   }
  }

  $all_dates = 
    "pt : $pt : " . date('Y-m-d H:i:s', $pt) . PHP_EOL .
    "ft : $ft : " . date('Y-m-d H:i:s', $ft) . PHP_EOL .
    "ct : $ct : " . date('Y-m-d H:i:s', $ct) . PHP_EOL .
    "mt : $mt : " . date('Y-m-d H:i:s', $mt) . PHP_EOL;
  
  if ($ft >= $ct) {
   $status = 'uncompiled';
  } elseif ($ct < $mt) {
   if ($mt >= $pt) {
    $status = 'modified, unchecked';
   } else {
    $status = 'modified';
   }
  } elseif ($mt >= $pt) {
   $status = 'unchecked';
  } else {
   $status = 'ok';
  }

  $this->status = $status;
  return $status;
 }

 /* This method sets the opening and closing times for the quiz.
  * The argument $weeks is expected to contain information about
  * the academic calendar, as returned by the function read_weeks_file()
  * in parse_time.inc.  It is assumed that $this->open_spec and 
  * $this->close_spec have been set, as is done by the read_dates_file()
  * method of stacker\course.
  */
 function set_dates($weeks) {
  global $DB;
  if (isset($this->open_spec) && isset($this->close_spec)) {
   $open_time = parse_time($this->open_spec,$weeks);
   $close_time = parse_time($this->close_spec,$weeks);

   $quiz1 = $this->moodle_quiz->get_quiz();
   $quiz1->timeopen  = $open_time->timestamp;
   $quiz1->timeclose = $close_time->timestamp;
   $DB->update_record('quiz',$quiz1);
   quiz_update_events($quiz1);
   $quiz_name = $this->moodle_quiz->get_quiz_name();
   $this->new_dates_msg = 
    "$quiz_name : opens {$open_time->ymdhm}, closes {$close_time->ymdhm}";
  }
 }

 /* When we recompile a markup file, we remove all the questions from 
  * the quiz, and then we import an XML file that creates new questions
  * to replace the old ones.  It would be better to modify the old
  * questions rather than completely replacing them, but that would be 
  * much more complicated.  This method just does the removal.
  */
 function remove_questions() {
  global $DB;

  $quiz = $this->moodle_quiz;
 
  $structure = $quiz->get_structure();
  $slots = $structure->get_slots();

  $has_stack_slots = true;
  while ($has_stack_slots) {
   $has_stack_slots = false;
   foreach($structure->get_slots() as $slot) {
    $qtype = $structure->get_question_type_for_slot($slot->slot);
    if ($qtype == 'stack') {
     $structure->remove_slot($slot->slot);
     $structure = $quiz->get_structure();
     $has_stack_slots = true;
     break;
    }
   }
  }
  
  $category = $this->get_default_category();

  if ($category) {
   $finder = \question_finder::get_instance();
  
   $question_ids =
    $finder->get_questions_from_categories(array($category->id),"");

   foreach ($question_ids as $i) {
    $question = $DB->get_record('question', array('id' => $i));
    if ($question->qtype == "stack") {
     question_delete_question($i);
    }
   }
  }
 }
 
 /* Our compilation process converts our markup files into Moodle XML files.
  * This method then imports one of those XML files into the Moodle database.
  * This process is somewhat complex as there are extra entries in several
  * different database tables corresponding to PRTs, input elements and so on.
  */
 function import_xml() {
  $dir = $this->xml_dir;
  $file = $this->xml_file_name;
  $quiz = $this->moodle_quiz; 
  $context = $quiz->get_context();
  
  $qf = new \qformat_xml();
  $qf->setContexts(array($context));
  $qf->setFilename($dir . '/' . $file);
  $qf->setRealFilename($file);
  $qf->setCatfromfile(true);
  $qf->setContextfromfile(true);
  $qf->set_display_progress(false);
  $qf->importprocess();
 }

 function to_xml($doc_ = null) {
  $doc = $doc_; if (! $doc) { $doc = new \DOMDocument(); }

  $x = $doc->createElement('quiz');
  $x->appendChild($this->category_question_xml($doc));

  foreach($this->questions as $q) {
   $x->appendChild($q->to_xml($doc));
  }

  return $x;
 }

 function to_xml_string($doc_ = null) {
  $doc = $doc_; if (! $doc) { $doc = new \DOMDocument(); }
  
  $x = $this->to_xml($doc);
  return $doc->saveXML($x);
 }

 function save_xml($doc_ = null) {
  $doc = $doc_; if (! $doc) { $doc = new \DOMDocument(); }

  $s = $this->to_xml_string($doc);
  $f = $this->full_xml_file_name;
  file_put_contents($f,$s);
 }

 function compile($debug = 0) {
  $f = $this->full_stack_file_name;
  $C = new \stacker\compiler();
  $c = $C->compile_file($f,$debug);
  $this->questions = $c[0];
  $this->errors = $c[1];
 }

 function compile_and_install($debug = 0) {
  $this->compile($debug);
  $doc = new \DOMDocument();
  $doc->formatOutput = true;
  $this->save_xml($doc);
  $this->remove_questions();
  $this->import_xml();
  $this->use_default_category();
 }
}
 
//////////////////////////////////////////////////////////////////////

class course {
 function load_by_name($name) {
  global $DB;

  $raw_course = $DB->get_record('course',array('shortname' => $name));
  if ($raw_course) { 
   $this->raw_fill($raw_course);
  } else {
   error("Course not found");
  }
 }

 function raw_fill($raw_course) {
  global $DB, $source_dir;

  $this->raw_course = $raw_course;
  $this->id = $this->raw_course->id;
  $this->shortname = $this->raw_course->shortname;
  $this->context =
    $DB->get_record('context',array('contextlevel' => 50,'instanceid' => $this->id));
  $this->contextid = $this->context->id;

  $this->source_dir = $source_dir . '/questions/' . $this->shortname;
  $this->stack_dir = $this->source_dir . '/stack';
  $this->xml_dir = $this->source_dir . '/xml';
  
  $this->dates_file = $source_dir . '/dates/' . $this->shortname . "_dates.csv";
  
  $course_modules =
   $DB->get_records('course_modules',array('course' => $this->id, 'module' => 16));
  $this->quizzes = array();
  $this->quizzes_by_name = array();
  $this->quizzes_by_short_name = array();

  foreach($course_modules as $course_module) {
   $quiz = new \stacker\quiz();
   // $moodle_quiz is an instance of quiz, defined in moodle/mod/quiz/attemptlib.php
   $moodle_quiz = \quiz::create($course_module->instance);
   $moodle_quiz->preload_questions();
   $moodle_quiz->load_questions();
   $moodle_quiz->get_structure();
   $quiz->fill($moodle_quiz);
   
   $this->quizzes[] = $quiz;
   $this->quizzes_by_name[$quiz->name] = $quiz;
   $this->quizzes_by_short_name[$quiz->short_name] = $quiz;
  }
 }

 function get_quiz_by_name($quiz_name) {
  if (isset($this->quizzes_by_name[$quiz_name])) {
   return($this->quizzes_by_name[$quiz_name]);
  }
  if (isset($this->quizzes_by_short_name[$quiz_name])) {
   return($this->quizzes_by_short_name[$quiz_name]);
  }
  return null;
 }
 
 function read_dates_file() {
  $dates_file = realpath($this->dates_file);
  $errors = array();
  
  if (! $dates_file) {
   $errors[] = "{$this->shortname}: dates_file = ''";
   return $errors;
  }

  if (! file_exists($dates_file)) {
   $errors[] = "{$this->shortname}: dates_file = {$dates_file} does not exist.";
   return $errors;
  }
  
  $fh = fopen($dates_file,"r");
  if ($fh === false) {
   $errors[] = "Dates file {$dates_file} not found";
   return $errors;
  }

  $i = 1;
  while(($x = fgetcsv($fh)) !== false) {
   if (count($x) == 0) { continue; }
   if (count($x) != 3) {
    $errors[] = "Bad line (number $i) in dates file";
    continue;
   }
 
   $quiz_name = $x[0];
   $quiz = null;
   
   if (isset($this->quizzes_by_short_name[$quiz_name])) {
    $quiz = $this->quizzes_by_short_name[$quiz_name];
   } else if (isset($this->quizzes_by_name[$quiz_name])) {
    $quiz = $this->quizzes_by_name[$quiz_name];
   }

   if ($quiz) {
    $quiz->open_spec = $x[1];
    $quiz->close_spec = $x[2];
   } else {
    $errors[] = "Quiz $quiz_name on line $i not found";
    continue;
   }
  }

  fclose($fh);
  return $errors;
 }

 function set_quiz_dates($weeks) {
  global $DB;
  
  foreach ($this->quizzes as $quiz) {
   if (isset($quiz->open_spec) && isset($quiz->close_spec)) {
    $open_time = parse_time($quiz->open_spec,$weeks);
    $close_time = parse_time($quiz->close_spec,$weeks);

    $quiz1 = $quiz->raw_quiz;
    $quiz1->timeopen  = $open_time->timestamp;
    $quiz1->timeclose = $close_time->timestamp;
    $DB->update_record('quiz',$quiz1);
    quiz_update_events($quiz1);
    $quiz_name = $quiz->name;
    // echo "$quiz_name : opens {$open_time->ymdhm}, closes {$close_time->ymdhm}" . PHP_EOL;
   }
  }
 }
}

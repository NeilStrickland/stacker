<?php

namespace stacker;

$log_level = 1;

function log($s,$n=1) {
 global $log_level;

 if ($log_level >= $n) {
  echo $s . PHP_EOL;
 }
}

function esc($s) {
 return htmlspecialchars($s, ENT_XML1, 'UTF-8');
}

function to01($s) {
 if ($s == '' || $s == '1' || $s == 'true' || $s == 1 || $s == true) {
  return 1;
 } else {
  return 0;
 }
}
 
function parse_inputs($s) {
 $inputs = array();
 $has_validation = array();
 $setup = array();
 $s0 = $s;

 while (preg_match("/{!([A-Za-z0-9_]+)!}/",$s0,$m)) {
  $name = $m[1];
  $ms = "[[validation:" . $name . "]]";
  $s0 = str_replace($m[0],$ms,$s0);
  $has_validation[$m[1]] = 1;
  //  echo "Validation element: $name" . PHP_EOL . PHP_EOL . $s0 . PHP_EOL; exit;
 }

 // echo "No validation elements" . PHP_EOL; exit;
 
 while (preg_match("/{[?]([^?]*)[?]}/",$s0,$m)) {
  $m0 = $m[0];
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
  $name = $m2[0][0];
  $ip = new input($name);
  $ta = $m2[0][1];
  if ($ta == '') { $ta = 'ta'; }
  if (preg_match("/^M(C|D)(L?)\\((.*)\\)$/",$ta,$m)) {
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
   $ip->type = 'matrix';
   $ip->tans = $m[1];
   $ip->prt_tans = $m[1];
  } else if (preg_match("/^TF:(.*)$/",$ta,$m)) {
   $ip->type = 'radio';
   $ip->tans = "{$name}_options";
   $ip->prt_tans = "{$name}_correct";
   $setup[] = "{$name}_correct: mcq_to_01({$m[1]});";
   $setup[] = "{$name}_options: mcq_add_bool([[1,\"True\"],[0,\"False\"]],{$name}_correct);";
   $ip->mustverify = 0;
   $ip->showvalidation = 0;
   $ip->prt_answertest = 'CasEqual';
  } else if (preg_match("/^YN:(.*)$/",$ta,$m)) {
   $ip->type = 'radio';
   $ip->tans = "{$name}_options";
   $ip->prt_tans = "{$name}_correct";
   $setup[] = "{$name}_correct: mcq_to_01({$m[1]});";
   $setup[] = "{$name}_options: mcq_add_bool([[1,\"Yes\"],[0,\"No\"]],{$name}_correct);";
   $ip->mustverify = 0;
   $ip->showvalidation = 0;
   $ip->prt_answertest = 'CasEqual';
  } else if (preg_match("/^B\\((.*)\\)$/",$ta,$m)) {
   $ip->type = 'builder';
   $ip->tans = "{$name}_options";
   $ip->prt_tans = "{$name}_correct";
   $setup[] = "{$name}_options: {$m[1]};";
   $setup[] = "{$name}_correct: builder_correct({$name}_options);";
  } else {
   $ip->tans = $ta;
   $ip->prt_tans = $ta;
  }
  
  for ($i = 1; $i < count($m2); $i++) {
   $e = $m2[$i];
   $key = $e[0];
   $val = $e[1];
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
    $ip->strictsyntax = to01($val);
   } else if ($key == 'laxsyntax'          || $key == '-x') { 
    $ip->strictsyntax = 1 - to01($val);
   } else if ($key == 'insertstars'        || $key == '*') { 
    $ip->insertstars = to01($val);
   } else if ($key == 'syntaxhint'         || $key == 'h') { 
    $ip->syntaxhint = $val;
   } else if ($key == 'syntaxattribute'    || $key == 'y') { 
    $ip->syntaxattribute = to01($val);
   } else if ($key == 'forbidwords'        || $key == 'f') { 
    $ip->forbidwords = $val;
   } else if ($key == 'allowwords'         || $key == 'a') { 
    $ip->allowwords = $val;
   } else if ($key == 'forbidfloat'        || $key == '-.') { 
    $ip->forbidfloat = to01($val);
   } else if ($key == 'allowfloat'         || $key == '.') { 
    $ip->forbidfloat = 1 - to01($val);
   } else if ($key == 'requirelowestterms' || $key == '/') { 
    $ip->requirelowestterms = to01($val);
   } else if ($key == 'mustverify'         || $key == 'v') { 
    $ip->mustverify = to01($val);
   } else if ($key == 'showvalidation'     || $key == 'w') { 
    $ip->showvalidation = to01($val);
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

function find_flag($s,$indent) {
 $x = new \stdClass();
 if (preg_match('/^(_*)([a-z]+>)( ?)(.*)$/',$s,$m)) {
  $x->flag = full_flag($m[2]);
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

  $f = find_flag($l,$indent);
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

function read_lines($file) {
 log("Reading file: $file");
 return split_lines(file_get_contents($file));
}

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
  
 $state = 'statement';
  
 $flagnum = 0;
 foreach ($lines as $line) {
  $flagnum++;
  $flag = $line->flag;
  $text = $line->text;
         
  if ($state == "statement") {
   if ($flag == "name>") {
    $name = trim($text);
   } else if ($flag == "keywords>") {
    $keywords = array_merge($keywords,explode(',',$text));
   } else if ($flag == "value>") {
    $defaultgrade = (float) $text;
   } else if ($flag == "penalty>") {
    $penalty = '' . ((float) $text);
   } else if ($flag == "answer>") {
     $tans = $text;
     $questiontext[] = "__DEFAULT_INPUT__";
   } else if ($flag == "answervariable>") {
     $ansvar = trim(text);
     if ($ansvar == '') { $ansvar = 'ans1'; }
   } else if ($flag == "note>") {
    $questionnote = $text;
   } else if ($flag == "options>") {
    $opts->parse($text);
   } else if ($flag == "inputtype>") {
    $typespec = trim($text);
   } else if ($flag == "forbid>") {
    $forbidwords[] = $text;
   } else if ($flag == "hide>") {
    $questionvariables[] = $text;
   } else if ($flag == "type>") {
     $x = parse_inputs($text);
     $inputs = array_merge($inputs,$x->inputs);
     $input_setup = array_merge($input_setup,$x->setup);
     $questiontext[] = $x->text;
   } else if ($flag == "numhints>") {
    $numhints = 0 + $text;
   } else if ($flag == "beginsol>") {
     $state = "solution";
   } else if ($flag == "endsol>") {
    $x = new \stdClass();
    $x->is_error = true;
    $x->question_name = $name;
    $x->flagnum = $flagnum;
    $x->lines = $lines;
    $x->msg = "se> or endsol> flag before corresponding sb> or beginsol>";
    return $x;
   } else if ($flag == "beginmark>") {
     $state = "mark";
     $node = null;
     $bool = null;
   } else if ($flag == "endmark>") {
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
    $parent = $node;
    $node = $prt->new_node();
    $node->parent = $parent;
    if ($parent) {
     $parent->set_nextnode($bool,$node->nodename);
     $node->bool = $bool;
     $node->truescore = 0;
    }
    $x = parse_answer_test($text);
    $node->answertest = $x->test;
    $node->sans = $x->sans;
    $node->tans = $x->tans;
    $bool = true;
   } else if ($flag == "else>") {
     $bool = false;
   } else if ($flag == "score>") {
    $x = parse_score($text);
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
  log("No explicit input elements",2);
  log("typespec:" . PHP_EOL . $typespec . PHP_EOL,2);
  $ip = new input();
  $ip->tans = $tans;
  $ip->prt_tans = $tans;
  $ip->name = $ansvar;
  
  if ($typespec) {
   if (preg_match("/^MC\\((.*)\\)$/sm",$typespec,$m)) {
    log("Input type MC()",2);
    $ip = new input();
    $ip->set_MC();
    $ip->name = $ansvar;
    $ip->tans = $ansvar . '_options';
    $ip->prt_tans = $ansvar . '_correct';

    $input_setup[] = "{$ansvar}_options: mcq_options([{$m[1]}],{$tans});";
    $input_setup[] = "{$ansvar}_correct: {$tans};";
   } elseif (preg_match("/^MCL\\((.*)\\)$/sm",$typespec,$m)) {
    log("Input type MCL()",2);
    $ip = new input();
    $ip->set_MC();
    $ip->name = $ansvar;
    $ip->tans = $ansvar . '_options';
    $ip->prt_tans = $ansvar . '_correct';

    $input_setup[] = "{$ansvar}_options: mcq_add_labels(mcq_add_bool([{$m[1]}],{$tans}));";
    $input_setup[] = "{$ansvar}_correct: mcq_correct({$ansvar}_options)[1];";
   } else if (preg_match("/^MR\\((.*)\\)$/sm",$typespec,$m)) {
    log("Input type MR()",2);
    $ip = new input();
    $ip->name = $ansvar;
    $ip->set_MR();
    $ip->tans = $ansvar . '_options';
    $ip->prt_tans = $ansvar . '_correct';

    $input_setup[] = "{$ansvar}_options: mrq_options({$m[1]},{$tans});";
    $input_setup[] = "{$ansvar}_correct: {$tans};";
   } else if (preg_match("/^MRL\\((.*)\\)$/sm",$typespec,$m)) {
    log("Input type MRL()",2);
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

function compile_lines($lines,$debug = 0) {
 $questions0 = split_questions($lines);
 $questions = array();
 $errors = array();
 
 $i = 0;
 foreach($questions0 as $x) {
  $i++;
  $q = compile_question($x,'',$debug);
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
 return compile_lines(split_lines($s),$debug);
}

function compile_file($f,$debug=0) {
 log("Compiling file: $f",1);
 $s = file_get_contents($f);
 // echo $s;
 return compile_string($s,$debug);
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
  if (! $doc) { $doc = new DOMDocument(); }
  
  $ox = $this->opts->xml_nodes($doc);
  $x = $doc->createElement('question');
  $x->setAttribute('type','stack');
  $x->appendChild($n = $doc->createElement('name'));
  $n->appendChild($t = $doc->createElement('text',esc($this->name)));
  $x->appendChild($n = $doc->createElement('questiontext'));
  $n->setAttribute('format',$this->questiontextformat);
  $n->appendChild($t = $doc->createElement('text',esc($this->questiontext)));
  $x->appendChild($n = $doc->createElement('generalfeedback'));
  $n->setAttribute('format',$this->generalfeedbackformat);
  $n->appendChild($t = $doc->createElement('text',esc($this->generalfeedback)));
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
   $h->appendChild($doc->createElement('text',esc('<p>-</p>')));
  }

  foreach($this->prts as $prt) {
   $x->appendChild($prt->to_xml($doc));
  }
  
  return $x;
 }

 function to_xml_string($doc_ = null) {
  $doc = $doc_;
  if (! $doc) { $doc = new DOMDocument(); }
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
   $x->$k->appendChild($doc->createElement('text',esc($this->$k)));
  }

  foreach(array('stackversion','questionvariables','questionnote') as $k) {
   $kf = $k . 'format';
   $x->$k = $doc->createElement($k);
   $x->$k->appendChild($doc->createElement('text',esc($this->$k)));
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
   $x->appendChild($doc->createElement($k,esc($this->$k)));
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
  $y->appendChild($doc->createElement('text',esc($this->feedbackvariables)));

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
   $x->appendChild($doc->createElement($k,esc('' . $this->$k)));
  }

  $x->appendChild($n = $doc->createElement('truefeedback'));
  $n->setAttribute('format',$this->truefeedbackformat);
  $n->appendChild($doc->createElement('text',esc($this->truefeedback)));
  
  $x->appendChild($n = $doc->createElement('falsefeedback'));
  $n->setAttribute('format',$this->falsefeedbackformat);
  $n->appendChild($doc->createElement('text',esc($this->falsefeedback)));

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

 function munch_moodle_quiz($moodle_quiz) {
  $this->moodle_quiz = $moodle_quiz;
  $this->name = $moodle_quiz->get_quiz_name();
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

 function get_file_name() {
  if ($this->file_name) {
   return $this->file_name;
  } else {
   return $this->name;
  }
 }
 
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

 function get_file_time() {
  $this->file_time = 0;
  $file = $this->full_stack_file_name;
  if (! file_exists($file) && $this->moodle_quiz) {
   $this->set_file_name($this->moodle_quiz->short_name);
   $file = $this->full_stack_file_name;
  }

  if (file_exists($file)) {
   $this->file_time = filemtime($file);
  }

  return $this->file_time;
 }

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
 
 function compile($debug = 0) {
  $f = $this->full_stack_file_name;
  $c = compile_file($f,$debug);
  $this->questions = $c[0];
  $this->errors = $c[1];
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
}
 

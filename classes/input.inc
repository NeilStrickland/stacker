<?php

namespace stacker;

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

 static function parse($s) {
  if (! preg_match("/{[?]([^?]*)[?]}(.*)$/",$s,$m)) {
   return false;
  }

  $m0 = $m[0];
  $m1 = explode('|',$m[1]);
  $rest = $m[2];
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
  $ms = "[[input:{$name}]][[validation:{$name}]]";
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

  if ($ip->hidevalidation) {
   $ms = "[[input:{$name}]]" .
      "<span style=\"display:none\">[[validation:{$name}]]</span>";
  }

  return array($ip,$rest);
 }
}


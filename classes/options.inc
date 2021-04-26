<?php

namespace stacker;

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

<?php

namespace stacker;

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


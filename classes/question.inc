<?php

namespace stacker;
 
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


<?php

namespace stacker;

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


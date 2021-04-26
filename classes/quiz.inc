class quiz {
 public $name = '';
 public $file_name = '';
 public $category_name = '';
 public $category_info = '';
 public $questions = [];
 public $stack_dir = '';
 public $xml_dir = '';
 public $moodle_quiz = null;
  
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

 function set_dirs($d) {
  $this->stack_dir = $d . '/stack';
  $this->xml_dir = $d . '/xml';
 }

 function compile() {
  $f = $this->stack_dir . '/' . $this->get_file_name() . '.stack';
  $c = compile_file($f);
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
  $f = $this->xml_dir . '/' . $this->get_file_name() . '.xml';
  file_put_contents($f,$s);
 }
}
 

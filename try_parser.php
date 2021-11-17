<?php

$source_dir = '/home/sa_pm1nps/Stack';
chdir('/var/www/html/moodle/scripts/stacker');
require_once('cli_tools.inc');

require_once($CFG->dirroot . '/question/type/stack/stack/maximaparser/corrective_parser.php');

$errors = array();
$answernote = array();
$parseoptions = array();
$string = <<<TEXT
s : sconcat("a" "b")
TEXT;

$result = maxima_corrective_parser::parse($string,$errors,$answernote,$parseoptions);

echo "RESULT:" . PHP_EOL;
var_dump($result);

echo PHP_EOL . PHP_EOL . "ERRORS:" . PHP_EOL;
var_dump($errors);

<?php
// $Id: devel_generate.inc,v 1.41 2009/06/30 17:07:48 weitzman Exp $
// If not in 'safe mode', increase the maximum execution time:
if (!ini_get('safe_mode')) {
  set_time_limit(240);
}

/**
 * Generate some random users.
 *
 * @param $num
 *  Number of users to generate.
 * @param $kill
 *  Boolean that indicates if existing users should be removed first.
 * @param $age
 *  The max age of each randomly-generated user, in seconds.
 */
function devel_create_users($num, $kill, $age = 0) {
  $url = parse_url($GLOBALS['base_url']);
  if ($kill) {
    db_delete('users')->condition('uid', 1, '>')->execute();
    drupal_set_message(t('Users deleted.'));
  }
  // Determine if we should create user pictures.
  $pic_config = FALSE;
  module_load_include('inc', 'system', 'image.gd');
  if (variable_get('user_pictures', 0) && drupal_function_exists('image_gd_check_settings') && image_gd_check_settings()) {
    $pic_config['path'] = variable_get('user_picture_path', 'pictures');
    list($pic_config['width'], $pic_config['height']) = explode('x', variable_get('user_picture_dimensions', '85x85'));
  }

  for ($i = 0; $i < $num; $i++) {
    $name = devel_generate_word(rand(6, 12));
    $edit = array(
      'name' => $name,
      'pass' => user_password(),
      'mail' => $name .'@'. $url['host'],
      'status' => 1,
      'created' => REQUEST_TIME - rand(0, $age),
    );
    $account = user_save(NULL, $edit);

    if ($pic_config) {
     $im = imagecreatetruecolor($pic_config['width'], $pic_config['height']);

      // Randomize the foreground using the md5 of the user id, then invert it
      // for the background color so there's enough contrast to read the text.
      $parts = array_map('hexdec', str_split(md5($account->uid), 2));
      $fg = imagecolorallocate($im, $parts[1], $parts[3], $parts[5]);
      $bg = imagecolorallocate($im, 255 - $parts[0], 255 - $parts[1], 255 - $parts[2]);

      // Fill the background then print their user info.
      imagefill($im, 0, 0, $bg);
      imagestring($im, 5, 5, 5, "#" . $account->uid, $fg);
      imagestring($im, 5, 5, 25, $account->name, $fg);

      // Create an empty managed file then write the image to it.
      $file = file_save_data('', file_create_path($pic_config['path'] . '/picture-' . $account->uid . '.png'));
      imagepng($im, $file->filepath);
      imagedestroy($im);

      // Clear the cached filesize the set the owner and resave the file.
      clearstatcache();
      $file->uid = $account->uid;
      $file = file_save($file);

      // Save the user record with the new picture.
      $edit = (array) $account;
      $edit['picture'] = $file;
      user_save($account, $edit);
    }
  }
  drupal_set_message(t('!num_users created.', array('!num_users' => format_plural($num, '1 user', '@count users'))));
}


/**
 * The main API function for creating content.
 *
 * See devel_generate_content_form() for the supported keys in $form_state['values'].
 * Other modules may participate by form_alter() on that form and then handling their data during hook_nodeapi('pre_save') or in own submit handler for the form.
 *
 * @param string $form_state
 * @return void
 */
function devel_generate_content($form_state) {
  if (!empty($form_state['values']['kill_content'])) {
    devel_generate_content_kill($form_state['values']);
  }

  if (count($form_state['values']['node_types'])) {
    // Generate nodes.
    devel_generate_content_pre_node($form_state['values']);
    for ($i = 1; $i <= $form_state['values']['num_nodes']; $i ++) {
      devel_generate_content_add_node($form_state['values']);
    }
  }

  drupal_set_message(format_plural($form_state['values']['num_nodes'], '1 node created.', '@count nodes created'));
}

function devel_generate_add_comments($node, $users, $num_comments, $title_length = 8) {
  // Insert new data:
  for ($i = 1; $i <= $num_comments; $i++) {
    $comment->nid = $node->nid;
    $comment->cid = NULL;
    $comment->comment_format = FILTER_FORMAT_DEFAULT;
    $comment->name = 'devel generate';
    $comment->mail = 'devel_generate@example.com';
    $comment->timestamp = rand($node->created, REQUEST_TIME);

    switch ($i % 3) {
      case 1:
        $comment->pid = db_result(db_query_range("SELECT cid FROM {comment} WHERE pid = 0 AND nid = :nid ORDER BY RAND()", array(':nid' => $comment->nid), 0, 1));
        break;
      case 2:
        $comment->pid = db_result(db_query("SELECT cid FROM {comment} WHERE pid > 0 AND nid = :nid ORDER BY RAND()", array(':nid' => $comment->nid), 0, 1));
        break;
      default:
        $comment->pid = 0;
    }

    $comment->subject = devel_create_greeking(rand(1, $title_length), TRUE);
    $comment->comment = devel_create_content();
    $comment->uid = $users[array_rand($users)];

    // This is slow but gets the threading right.
    $comment = (array) $comment;
    comment_save($comment);
  }
}

function devel_generate_vocabs($records, $maxlength = 12, $types = array('story', 'blog', 'forum', 'page')) {
  $vocs = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
    $voc = new stdClass();
    $voc->name = devel_generate_word(rand(2, $maxlength));
    $voc->description = "description of ". $voc->name;
    // TODO: not working
    $voc->nodes = array_flip(array($types[array_rand($types)]));
    foreach ($voc->nodes as $key => $value) {
      $voc->nodes[$key] = $key;
    }

    $voc->multiple = 1;
    $voc->required = 0;
    $voc->relations = 1;
    $voc->hierarchy = 1;
    $voc->weight = rand(0,10);

    taxonomy_vocabulary_save($voc);
    $vocs[] = $voc->name;

    unset($voc);
  }
  return $vocs;
}

function devel_generate_terms($records, $vocs, $maxlength = 12) {
  $terms = array();

  // Insert new data:
  for ($i = 1; $i <= $records; $i++) {
  	$term = new stdClass();
    switch ($i % 2) {
      case 1:
        $term->vid = $vocs[array_rand($vocs)];
        // dont set a parent. handled by taxonomy_save_term()
        // $term->parent = 0;
        break;
      case 2:
      default:
        $parent = db_fetch_object(db_query_range("SELECT t.tid, v.vid FROM {taxonomy_term_data} t INNER JOIN {taxonomy_vocabulary} v ON t.vid = v.vid ORDER BY RAND()", 0, 1));
        $term->parent = array($parent->tid);
        $term->vid = $parent->vid;
        break;
    }

    $term->name = devel_generate_word(rand(2, $maxlength));
    $term->description = "description of ". $term->name;
    $term->weight = rand(0,10);
    $status = taxonomy_term_save($term);
    $output = NULL;

    if ($status) {
      $terms[] = $term->name;
    }

    unset($term);
  }
  return $terms;
}

function devel_generate_get_vocabs() {
  $vocs = array();
  $result = db_query("SELECT vid FROM {taxonomy_vocabulary}");
  while($voc = db_fetch_object($result)){
    $vocs[] = $voc->vid;
  }
  return $vocs;
}

function devel_generate_taxonomy_data($num_vocab, $num_terms, $title_length, $kill) {

  if ($kill) {
    db_query("DELETE FROM {taxonomy_term_data}");
    db_query("DELETE FROM {taxonomy_term_node}");
    db_query("DELETE FROM {taxonomy_term_hierarchy}");
    db_query("DELETE FROM {taxonomy_term_relation}");
    db_query("DELETE FROM {taxonomy_term_synonym}");
    db_query("DELETE FROM {taxonomy_vocabulary}");
    db_query("DELETE FROM {taxonomy_vocabulary_node_type}");
    drupal_set_message(t('Deleted existing vocabularies and terms.'));
  }

  $new_vocs = devel_generate_vocabs($num_vocab, $title_length);
  if (!empty($new_vocs)) {
    drupal_set_message(t('Created the following new vocabularies: !vocs', array('!vocs' => theme('item_list', $new_vocs))));
  }
  $vocs = devel_generate_get_vocabs();
  $new_terms = devel_generate_terms($num_terms, $vocs, $title_length);
  if (!empty($new_terms)) {
    drupal_set_message(t('Created the following new terms: !terms', array('!terms' => theme('item_list', $new_terms))));
  }
}

function devel_generate_word($length){
  srand((double)microtime()*1000000);

  $vowels = array("a", "e", "i", "o", "u");
  $cons = array("b", "c", "d", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "u", "v", "w", "tr",
  "cr", "br", "fr", "th", "dr", "ch", "ph", "wr", "st", "sp", "sw", "pr", "sl", "cl", "sh");

  $num_vowels = count($vowels);
  $num_cons = count($cons);
  $word = '';

  while(strlen($word) < $length){
    $word .= $cons[rand(0, $num_cons - 1)] . $vowels[rand(0, $num_vowels - 1)];
  }

  return substr($word, 0, $length);
}

function devel_create_content($type = NULL) {
  $nparas = rand(1,12);
  $type = empty($type) ? rand(0,3) : $type;

  $output = "";
  switch($type % 3) {
    case 1: // html
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60),1);
      }
      break;

    case 2: // brs only
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60),2);
      }
      break;

    default: // plain text
      for ($i = 1; $i <= $nparas; $i++) {
        $output .= devel_create_para(rand(10,60)) ."\n\n";
      }
  }

  return $output;
}

function devel_create_para($words, $type = 0) {
  $output = "";
  switch ($type) {
    case 1:
      $output .= "<p>";
      $output .= devel_create_greeking($words);
      $output = trim($output) ."</p>";
      break;

    case 2:
      $output .= devel_create_greeking($words);
      $output = trim($output) ."<br />";
      break;

    default:
      $output .= devel_create_greeking($words);
      $output = trim($output);
  }
  return $output;
}

function devel_create_greeking($words, $title = FALSE) {
  $dictionary = array("abbas", "abdo", "abico", "abigo", "abluo", "accumsan",
    "acsi", "ad", "adipiscing", "aliquam", "aliquip", "amet", "antehabeo",
    "appellatio", "aptent", "at", "augue", "autem", "bene", "blandit",
    "brevitas", "caecus", "camur", "capto", "causa", "cogo", "comis",
    "commodo", "commoveo", "consectetuer", "consequat", "conventio", "cui",
    "damnum", "decet", "defui", "diam", "dignissim", "distineo", "dolor",
    "dolore", "dolus", "duis", "ea", "eligo", "elit", "enim", "erat",
    "eros", "esca", "esse", "et", "eu", "euismod", "eum", "ex", "exerci",
    "exputo", "facilisi", "facilisis", "fere", "feugiat", "gemino",
    "genitus", "gilvus", "gravis", "haero", "hendrerit", "hos", "huic",
    "humo", "iaceo", "ibidem", "ideo", "ille", "illum", "immitto",
    "importunus", "imputo", "in", "incassum", "inhibeo", "interdico",
    "iriure", "iusto", "iustum", "jugis", "jumentum", "jus", "laoreet",
    "lenis", "letalis", "lobortis", "loquor", "lucidus", "luctus", "ludus",
    "luptatum", "macto", "magna", "mauris", "melior", "metuo", "meus",
    "minim", "modo", "molior", "mos", "natu", "neo", "neque", "nibh",
    "nimis", "nisl", "nobis", "nostrud", "nulla", "nunc", "nutus", "obruo",
    "occuro", "odio", "olim", "oppeto", "os", "pagus", "pala", "paratus",
    "patria", "paulatim", "pecus", "persto", "pertineo", "plaga", "pneum",
    "populus", "praemitto", "praesent", "premo", "probo", "proprius",
    "quadrum", "quae", "qui", "quia", "quibus", "quidem", "quidne", "quis",
    "ratis", "refero", "refoveo", "roto", "rusticus", "saepius",
    "sagaciter", "saluto", "scisco", "secundum", "sed", "si", "similis",
    "singularis", "sino", "sit", "sudo", "suscipere", "suscipit", "tamen",
    "tation", "te", "tego", "tincidunt", "torqueo", "tum", "turpis",
    "typicus", "ulciscor", "ullamcorper", "usitas", "ut", "utinam",
    "utrum", "uxor", "valde", "valetudo", "validus", "vel", "velit",
    "veniam", "venio", "vereor", "vero", "verto", "vicis", "vindico",
    "virtus", "voco", "volutpat", "vulpes", "vulputate", "wisi", "ymo",
    "zelus");

  $greeking = "";

  if (!$title) {
	  while ($words > 0) {
	    $sentence_length = rand(3,10);

	    $greeking .= ucfirst($dictionary[array_rand($dictionary)]);
	    for ($i = 1; $i < $sentence_length; $i++) {
	      $greeking .= " " . $dictionary[array_rand($dictionary)];
	    }

	    $greeking .= ". ";
	    $words -= $sentence_length;
	  }
  }
  else {
  	// use different method for titles
  	$title_length = $words;
  	$array = array();
  	for ($i = 0; $i < $words; $i++) {
  		$array[] = $dictionary[array_rand($dictionary)];
  	}
  	$greeking = ucwords(implode(' ', $array));
  }
  return $greeking;
}

function devel_generate_add_terms(&$node) {
  $vocabs = taxonomy_get_vocabularies($node->type);
  foreach ($vocabs as $vocab) {
    $sql = "SELECT tid FROM {taxonomy_term_data} WHERE vid = :vid ORDER BY RAND()";
    $result = db_query_range($sql, array(':vid' => $vocab->vid), 0, 5);
    while ($row = db_fetch_object($result)) {
      if ($vocab->tags) {
        $node->taxonomy['tags'][] = $row->tid;
      }
      else {
        $node->taxonomy[$row->tid] = $row->tid;
      }
      if (!$vocab->multiple) {
        break;
      }
    }
  }
}

function devel_get_users() {
  $users = array();
  $result = db_query_range("SELECT uid FROM {users}", 0, 50);
  while($user = db_fetch_object($result)){
    $users[] = $user->uid;
  }
  return $users;
}

/**
 * Generate statistics information for a node.
 *
 * @param $node
 *   A node object.
 */
function devel_generate_add_statistics($node) {
  $statistic = array(
    'nid' => $node->nid,
    'totalcount' => mt_rand(0, 500),
    'timestamp' => REQUEST_TIME - mt_rand(0, $node->created),
  );
  $statistic['daycount'] = mt_rand(0, $statistic['totalcount']);
  db_insert('node_counter')->fields($statistic)->execute();
}

function devel_generate_add_upload(&$node) {
  $source = DRUPAL_ROOT. '/misc/blog.png';

  // $after this call, $source contains the new path.
  $file = new stdClass();
  $file->filepath = file_unmanaged_copy($source);
  $file->filename = 'blog.png';
  $file->filemime = 'image/png';
  $file->list = variable_get('upload_list_default', TRUE);
  $file->description = 'b log.png was here';
  $file->weight = rand(0,10);
  $file->uid = $node->uid;
  $file->status = 0;
  $file->new = TRUE;
  $file = file_save($file);

  $_SESSION['upload_files'][$file->fid] = $file;
  $node->files[$file->fid] = $file;
}

/**
 * Handle the devel_generate_content_form request to kill all of the content.
 * This is used by both the batch and non-batch branches of the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_kill($values) {
  $results = db_select('node', 'n')
              ->fields('n', array('nid'))
              ->condition('type', $values['node_types'], 'IN')
              ->execute();
  foreach ($results as $result) {
    $nids[] = $result->nid;
  }
  
  if (!empty($nids)) {
    node_delete_multiple($nids);
    drupal_set_message(t('Deleted %count nodes.', array('%count' => count($nids))));
  } 
}

/**
 * Pre-process the devel_generate_content_form request.  This is needed so
 * batch api can get the list of users once.  This is used by both the batch
 * and non-batch branches of the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_pre_node(&$results) {
  // Get user id.
  $users = devel_get_users();
  $users = array_merge($users, array('0'));
  $results['users'] = $users;
}

/**
 * Create one node.  This is used by both the batch and non-batch branches of
 * the code.
 *
 * @param $num
 *   array of options obtained from devel_generate_content_form.
 */
function devel_generate_content_add_node(&$results) {
  // Insert new data:
  $node->type = array_rand($results['node_types']);
  module_load_include('inc', 'node', 'node.pages');
  node_object_prepare($node);
  $users = $results['users'];
  $node->uid = $users[array_rand($users)];

  $type = node_type_get_type($node);
  $node->title = $type->has_title ? devel_create_greeking(rand(1, $results['title_length']), TRUE) : '';
  $node->body[0]['value'] = $type->has_body ? "node ($node->type) - ". devel_create_content() : '';
  $node->filter = variable_get('filter_default_format', 1);
  $node->node_format = FILTER_FORMAT_DEFAULT;
  $node->language = '';
  $node->revision = rand(0,1);
  $node->promote = rand(0, 1);
  // Avoid NOTICE.
  if (!isset($results['time_range'])) {
    $results['time_range'] = 0;
  }
  $node->created = REQUEST_TIME - rand(0, $results['time_range']);

  // A flag to let hook_nodeapi() implementations know that this is a generated node.
  $node->devel_generate = $results;

  // See devel_generate_nodeapi() for actions that happen before and after this save.
  node_save($node);
}
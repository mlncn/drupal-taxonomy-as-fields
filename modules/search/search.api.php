<?php
// $Id: search.api.php,v 1.11 2009/06/22 09:10:06 dries Exp $

/**
 * @file
 * Hooks provided by the Search module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define a custom search routine.
 *
 * This hook allows a module to perform searches on content it defines
 * (custom node types, users, or comments, for example) when a site search
 * is performed.
 *
 * Note that you can use form API to extend the search. You will need to use
 * hook_form_alter() to add any additional required form elements. You can
 * process their values on submission using a custom validation function.
 * You will need to merge any custom search values into the search keys
 * using a key:value syntax. This allows all search queries to have a clean
 * and permanent URL. See node_form_search_form_alter() for an example.
 *
 * The example given here is for node.module, which uses the indexed search
 * capabilities. To do this, node module also implements hook_update_index()
 * which is used to create and maintain the index.
 *
 * We call do_search() with the keys, the module name, and extra SQL fragments
 * to use when searching. See hook_update_index() for more information.
 *
 * @param $op
 *   A string defining which operation to perform:
 *   - 'admin': The hook should return a form array containing any fieldsets the
 *     module wants to add to the Search settings page at admin/settings/search.
 *   - 'name': The hook should return a translated name defining the type of
 *     items that are searched for with this module ('content', 'users', ...).
 *   - 'reset': The search index is going to be rebuilt. Modules which use
 *     hook_update_index() should update their indexing bookkeeping so that it
 *     starts from scratch the next time hook_update_index() is called.
 *   - 'search': The hook should perform a search using the keywords in $keys.
 *   - 'status': If the module implements hook_update_index(), it should return
 *     an array containing the following keys:
 *     - remaining: The amount of items that still need to be indexed.
 *     - total: The total amount of items (both indexed and unindexed).
 * @param $keys
 *   The search keywords as entered by the user.
 * @return
 *   This varies depending on the operation.
 *   - 'admin': The form array for the Search settings page at
 *     admin/settings/search.
 *   - 'name': The translated string of 'Content'.
 *   - 'reset': None.
 *   - 'search': An array of search results. To use the default search result
 *     display, each item should have the following keys':
 *     - 'link': Required. The URL of the found item.
 *     - 'type': The type of item.
 *     - 'title': Required. The name of the item.
 *     - 'user': The author of the item.
 *     - 'date': A timestamp when the item was last modified.
 *     - 'extra': An array of optional extra information items.
 *     - 'snippet': An excerpt or preview to show with the result (can be
 *     generated with search_excerpt()).
 *   - 'status': An associative array with the key-value pairs:
 *     - 'remaining': The number of items left to index.
 *     - 'total': The total number of items to index.
 *
 * @ingroup search
 */
function hook_search($op = 'search', $keys = NULL) {
  switch ($op) {
    case 'name':
      return t('Content');

    case 'reset':
      db_query("UPDATE {search_dataset} SET reindex = %d WHERE type = 'node'", REQUEST_TIME);
      return;

    case 'status':
      $total = db_result(db_query('SELECT COUNT(*) FROM {node} WHERE status = 1'));
      $remaining = db_result(db_query("SELECT COUNT(*) FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE n.status = 1 AND d.sid IS NULL OR d.reindex <> 0"));
      return array('remaining' => $remaining, 'total' => $total);

    case 'admin':
      $form = array();
      // Output form for defining rank factor weights.
      $form['content_ranking'] = array(
        '#type' => 'fieldset',
        '#title' => t('Content ranking'),
      );
      $form['content_ranking']['#theme'] = 'node_search_admin';
      $form['content_ranking']['info'] = array(
        '#value' => '<em>' . t('The following numbers control which properties the content search should favor when ordering the results. Higher numbers mean more influence, zero means the property is ignored. Changing these numbers does not require the search index to be rebuilt. Changes take effect immediately.') . '</em>'
      );

      // Note: reversed to reflect that higher number = higher ranking.
      $options = drupal_map_assoc(range(0, 10));
      foreach (module_invoke_all('ranking') as $var => $values) {
        $form['content_ranking']['factors']['node_rank_' . $var] = array(
          '#title' => $values['title'],
          '#type' => 'select',
          '#options' => $options,
          '#default_value' => variable_get('node_rank_' . $var, 0),
        );
      }
      return $form;

    case 'search':
      // Build matching conditions
      list($join1, $where1) = _db_rewrite_sql();
      $arguments1 = array();
      $conditions1 = 'n.status = 1';

      if ($type = search_query_extract($keys, 'type')) {
        $types = array();
        foreach (explode(',', $type) as $t) {
          $types[] = "n.type = '%s'";
          $arguments1[] = $t;
        }
        $conditions1 .= ' AND (' . implode(' OR ', $types) . ')';
        $keys = search_query_insert($keys, 'type');
      }

      if ($category = search_query_extract($keys, 'category')) {
        $categories = array();
        foreach (explode(',', $category) as $c) {
          $categories[] = "tn.tid = %d";
          $arguments1[] = $c;
        }
        $conditions1 .= ' AND (' . implode(' OR ', $categories) . ')';
        $join1 .= ' INNER JOIN {taxonomy_term_node} tn ON n.vid = tn.vid';
        $keys = search_query_insert($keys, 'category');
      }

      if ($languages = search_query_extract($keys, 'language')) {
        $categories = array();
        foreach (explode(',', $languages) as $l) {
          $categories[] = "n.language = '%s'";
          $arguments1[] = $l;
        }
        $conditions1 .= ' AND (' . implode(' OR ', $categories) . ')';
        $keys = search_query_insert($keys, 'language');
      }

      // Get the ranking expressions.
      $rankings = _node_rankings();

      // When all search factors are disabled (ie they have a weight of zero),
      // The default score is based only on keyword relevance.
      if ($rankings['total'] == 0) {
        $total = 1;
        $arguments2 = array();
        $join2 = '';
        $select2 = 'i.relevance AS score';
      }
      else {
        $total = $rankings['total'];
        $arguments2 = $rankings['arguments'];
        $join2 = implode(' ', $rankings['join']);
        $select2 = '(' . implode(' + ', $rankings['score']) . ') AS score';
      }

      // Do search.
      $find = do_search($keys, 'node', 'INNER JOIN {node} n ON n.nid = i.sid ' . $join1, $conditions1 . (empty($where1) ? '' : ' AND ' . $where1), $arguments1, $select2, $join2, $arguments2);

      // Load results.
      $results = array();
      foreach ($find as $item) {
        // Build the node body.
        $node = node_load($item->sid);
        $node = node_build_content($node, 'search_result');
        $node->body = drupal_render($node->content);

        // Fetch comments for snippet.
        $node->body .= module_invoke('comment', 'node', $node, 'update_index');
        // Fetch terms for snippet.
        $node->body .= module_invoke('taxonomy', 'node', $node, 'update_index');

        $extra = module_invoke_all('node_search_result', $node);

        $results[] = array(
          'link' => url('node/' . $item->sid, array('absolute' => TRUE)),
          'type' => check_plain(node_type_get_name($node)),
          'title' => $node->title,
          'user' => theme('username', $node),
          'date' => $node->changed,
          'node' => $node,
          'extra' => $extra,
          'score' => $total ? ($item->score / $total) : 0,
          'snippet' => search_excerpt($keys, $node->body),
        );
      }
      return $results;
  }
}

/**
 * Preprocess text for the search index.
 *
 * This hook is called both for text added to the search index, as well as
 * the keywords users have submitted for searching.
 *
 * This is required for example to allow Japanese or Chinese text to be
 * searched. As these languages do not use spaces, it needs to be split into
 * separate words before it can be indexed. There are various external
 * libraries for this.
 *
 * @param $text
 *   The text to split. This is a single piece of plain-text that was
 *   extracted from between two HTML tags. Will not contain any HTML entities.
 * @return
 *   The text after processing.
 */
function hook_search_preprocess($text) {
  // Do processing on $text
  return $text;
}

/**
 * Update Drupal's full-text index for this module.
 *
 * Modules can implement this hook if they want to use the full-text indexing
 * mechanism in Drupal.
 *
 * This hook is called every cron run if search.module is enabled. A module
 * should check which of its items were modified or added since the last
 * run. It is advised that you implement a throttling mechanism which indexes
 * at most 'search_cron_limit' items per run (see example below).
 *
 * You should also be aware that indexing may take too long and be aborted if
 * there is a PHP time limit. That's why you should update your internal
 * bookkeeping multiple times per run, preferably after every item that
 * is indexed.
 *
 * Per item that needs to be indexed, you should call search_index() with
 * its content as a single HTML string. The search indexer will analyse the
 * HTML and use it to assign higher weights to important words (such as
 * titles). It will also check for links that point to nodes, and use them to
 * boost the ranking of the target nodes.
 *
 * @ingroup search
 */
function hook_update_index() {
  $limit = (int)variable_get('search_cron_limit', 100);

  $result = db_query_range("SELECT n.nid FROM {node} n LEFT JOIN {search_dataset} d ON d.type = 'node' AND d.sid = n.nid WHERE d.sid IS NULL OR d.reindex <> 0 ORDER BY d.reindex ASC, n.nid ASC", 0, $limit);

  foreach ($result as $node) {
    $node = node_load($node->nid);

    // Save the changed time of the most recent indexed node, for the search
    // results half-life calculation.
    variable_set('node_cron_last', $node->changed);

    // Render the node.
    $node = node_build_content($node, 'search_index');
    $node->rendered = drupal_render($node->content);

    $text = '<h1>' . check_plain($node->title) . '</h1>' . $node->rendered;

    // Fetch extra data normally not visible
    $extra = module_invoke_all('node_update_index', $node);
    foreach ($extra as $t) {
      $text .= $t;
    }

    // Update index
    search_index($node->nid, 'node', $text);
  }
}
/**
 * @} End of "addtogroup hooks".
 */

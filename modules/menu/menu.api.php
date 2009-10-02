<?php
// $Id: menu.api.php,v 1.12 2009/10/02 00:44:22 dries Exp $

/**
 * @file
 * Hooks provided by the Menu module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define menu items and page callbacks.
 *
 * This hook enables modules to register paths in order to define how URL
 * requests are handled. Paths may be registered for URL handling only, but can
 * also register a link to be placed in a menu (usually the Navigation menu). A
 * path and its associated information is commonly called a "menu router item".
 *
 * hook_menu() implementations return an associative array whose keys define
 * paths and whose values are an associative array defining properties for each
 * path. The definition for each path may refer to a callback function that is
 * invoked when the registered path is requested. In case there is no other
 * registered path that fits the requested path better, any further path
 * components are optionally passed to the callback function by default.
 * For example, when registering the path 'abc/def':
 * @code
 *   function mymodule_menu() {
 *     $items['abc/def'] = array(
 *       'page callback' => 'mymodule_abc_view',
 *     );
 *   }
 *
 *   function mymodule_abc_view($ghi = 0, $jkl = '') {
 *     // ...
 *   }
 * @endcode
 * When the path 'abc/def' was requested, then no further arguments would be
 * passed to the callback function, so $ghi and $jkl would take the default
 * values as defined in the function signature.
 * In case 'abc/def/123/foo' was requested, then $ghi would be '123' and $jkl
 * would be 'foo'.
 *
 * In addition to optional path arguments, the definition for each path may
 * specify a list of arguments for each callback function as an array. These
 * argument lists may contain fixed/hard-coded argument values, but may also
 * contain integers that correspond to path components. When integers are used
 * and the callback function is called, the corresponding path components will
 * be substituted. For example:
 * @code
 *   function mymodule_menu() {
 *     $items['abc/def'] = array(
 *       'page callback' => 'mymodule_abc_view',
 *       'page arguments' => array(1, 'foo'),
 *     );
 *   }
 * @endcode
 * When the path 'abc/def' was requested, the callback function would get 'def'
 * as first argument and (always) 'foo' as second argument.
 * The integer 1 in an argument list would be replaced with 'def' and integer 0
 * would be replaced with 'abc', i.e. path components are counted from zero.
 * This allows to re-use a callback function for several different paths.
 *
 * Arguments may also be used to replace wildcards within paths. For example,
 * when registering the path 'my-module/%/edit':
 * @code
 *   $items['my-module/%/edit'] = array(
 *     'page callback' => 'mymodule_abc_edit',
 *     'page arguments' => array(1),
 *   );
 * @endcode
 * When the path 'my-module/foo/edit' is requested, then integer 1 will be
 * replaced with 'foo' and passed to the callback function.
 *
 * Registered paths may also contain special "auto-loader" wildcard components
 * in the form of '%mymodule_abc', where the '%' part means that this path
 * component is a wildcard, and the 'mymodule_abc' part defines the prefix for a
 * menu argument loader function, which here would be mymodule_abc_load().
 * For example, when registering the path 'my-module/%mymodule_abc/edit':
 * @code
 *   $items['my-module/%mymodule_abc/edit'] = array(
 *     'page callback' => 'mymodule_abc_edit',
 *     'page arguments' => array(1),
 *   );
 * @endcode
 * When the path 'my-module/123/edit' is requested, then the argument loader
 * function mymodule_abc_load() will be invoked with the argument '123', and it
 * is supposed to take that value to load and return data for "abc" having the
 * internal id 123:
 * @code
 *   function mymodule_abc_load($abc_id) {
 *     return db_query("SELECT * FROM {mymodule_abc} WHERE abc_id = :abc_id", array(':abc_id' => $abc_id))->fetchObject();
 *   }
 * @endcode
 * The returned data of the argument loader will be passed in place of the
 * original path component to all callback functions referring to that (integer)
 * component in their argument list.
 * Menu argument loader functions may also be passed additional arguments; see
 * "load arguments" below.
 *
 * If a registered path defines an argument list, then those defined arguments
 * will always be passed first to the callback function. In case there are any
 * further components contained in the requested path, then those will always
 * come last.
 *
 * Special care should be taken for the page callback drupal_get_form(), because
 * the callback function will always receive $form and &$form_state as the very
 * first function arguments:
 * @code
 *   function mymodule_abc_form($form, &$form_state) {
 *     // ...
 *     return $form;
 *   }
 * @endcode
 * See @link form_api Form API documentation @endlink for details.
 *
 * This hook is rarely called (for example, when modules are enabled), and
 * its results are cached in the database.
 *
 * @return
 *   An array of menu items. Each menu item has a key corresponding to the
 *   Drupal path being registered. The corresponding array value is an
 *   associative array that may contain the following key-value pairs:
 *   - "title": Required. The untranslated title of the menu item.
 *   - "title callback": Function to generate the title; defaults to t().
 *     If you require only the raw string to be output, set this to FALSE.
 *   - "title arguments": Arguments to send to t() or your custom callback,
 *     with path component substitution as described above.
 *   - "description": The untranslated description of the menu item.
 *   - "page callback": The function to call to display a web page when the user
 *     visits the path. If omitted, the parent menu item's callback will be used
 *     instead.
 *   - "page arguments": An array of arguments to pass to the page callback
 *     function, with path component substitution as described above.
 *   - "access callback": A function returning a boolean value that determines
 *     whether the user has access rights to this menu item. Defaults to
 *     user_access() unless a value is inherited from a parent menu item.
 *   - "access arguments": An array of arguments to pass to the access callback
 *     function, with path component substitution as described above.
 *   - "theme callback": Optional. A function returning the machine-readable
 *     name of the theme that will be used to render the page. If the function
 *     returns nothing, the main site theme will be used. If no function is
 *     provided, the main site theme will also be used, unless a value is
 *     inherited from a parent menu item.
 *   - "theme arguments": An array of arguments to pass to the theme callback
 *     function, with path component substitution as described above.
 *   - "file": A file that will be included before the callbacks are accessed;
 *     this allows callback functions to be in separate files. The file should
 *     be relative to the implementing module's directory unless otherwise
 *     specified by the "file path" option. Note: This does not apply to the
 *     'access callback'.
 *   - "file path": The path to the folder containing the file specified in
 *     "file". This defaults to the path to the module implementing the hook.
 *   - "load arguments": An array of arguments to be passed to each of the
 *     wildcard object loaders in the path. For example, for the path
 *     node/%node/revisions/%/view, a "load arguments" value of array(1, 3) will
 *     call node_load() with the second and fourth path components passed in (as
 *     described above, integers are automatically replaced with path
 *     components). There are also two "magic" values: "%index" will correspond
 *     to the index of the wildcard path component, and "%map" will correspond
 *     to the full menu map, passed in by reference.
 *   - "weight": An integer that determines the relative position of items in
 *     the menu; higher-weighted items sink. Defaults to 0. When in doubt, leave
 *     this alone; the default alphabetical order is usually best.
 *   - "menu_name": Optional. Set this to a custom menu if you don't want your
 *     item to be placed in Navigation.
 *   - "tab_parent": For local task menu items, the path of the task's parent
 *     item; defaults to the same path without the last component (e.g., the
 *     default parent for 'admin/people/create' is 'admin/people').
 *   - "tab_root": For local task menu items, the path of the closest non-tab
 *     item; same default as "tab_parent".
 *   - "block callback": Name of a function used to render the block on the
 *     system administration page for this item (called with no arguments).
 *     If not provided, system_admin_menu_block() is used to generate it.
 *   - "position": Position of the block ('left' or 'right') on the system
 *     administration page for this item.
 *   - "type": A bitmask of flags describing properties of the menu item.
 *     Many shortcut bitmasks are provided as constants in menu.inc:
 *     - MENU_NORMAL_ITEM: Normal menu items show up in the menu tree and can be
 *       moved/hidden by the administrator.
 *     - MENU_CALLBACK: Callbacks simply register a path so that the correct
 *       function is fired when the path is accessed.
 *     - MENU_SUGGESTED_ITEM: Modules may "suggest" menu items that the
 *       administrator may enable.
 *     - MENU_LOCAL_TASK: Local tasks are rendered as tabs by default.
 *     - MENU_DEFAULT_LOCAL_TASK: Every set of local tasks should provide one
 *       "default" task, that links to the same path as its parent when clicked.
 *     If the "type" element is omitted, MENU_NORMAL_ITEM is assumed.
 *
 * For a detailed usage example, see page_example.module.
 * For comprehensive documentation on the menu system, see
 * http://drupal.org/node/102338.
 */
function hook_menu() {
  $items['blog'] = array(
    'title' => 'blogs',
    'page callback' => 'blog_page',
    'access arguments' => array('access content'),
    'type' => MENU_SUGGESTED_ITEM,
  );
  $items['blog/feed'] = array(
    'title' => 'RSS feed',
    'page callback' => 'blog_feed',
    'access arguments' => array('access content'),
    'type' => MENU_CALLBACK,
  );

  return $items;
}

/**
 * Alter the data being saved to the {menu_router} table after hook_menu is invoked.
 *
 * This hook is invoked by menu_router_build(). The menu definitions are passed
 * in by reference. Each element of the $items array is one item returned
 * by a module from hook_menu. Additional items may be added, or existing items
 * altered.
 *
 * @param $items
 *   Associative array of menu router definitions returned from hook_menu().
 */
function hook_menu_alter(&$items) {
  // Example - disable the page at node/add
  $items['node/add']['access callback'] = FALSE;
}

/**
 * Alter the data being saved to the {menu_links} table by menu_link_save().
 *
 * @param $item
 *   Associative array defining a menu link as passed into menu_link_save().
 */
function hook_menu_link_alter(&$item) {
  // Example 1 - make all new admin links hidden (a.k.a disabled).
  if (strpos($item['link_path'], 'admin') === 0 && empty($item['mlid'])) {
    $item['hidden'] = 1;
  }
  // Example 2  - flag a link to be altered by hook_translated_menu_link_alter()
  if ($item['link_path'] == 'devel/cache/clear') {
    $item['options']['alter'] = TRUE;
  }
}

/**
 * Alter a menu link after it's translated, but before it's rendered.
 *
 * This hook may be used, for example, to add a page-specific query string.
 * For performance reasons, only links that have $item['options']['alter'] == TRUE
 * will be passed into this hook. The $item['options']['alter'] flag should
 * generally be set using hook_menu_link_alter().
 *
 * @param $item
 *   Associative array defining a menu link after _menu_link_translate()
 * @param $map
 *   Associative array containing the menu $map (path parts and/or objects).
 */
function hook_translated_menu_link_alter(&$item, $map) {
  if ($item['href'] == 'devel/cache/clear') {
    $item['localized_options']['query'] = drupal_get_destination();
  }
}

 /**
 * Inform modules that a menu link has been created.
 *
 * This hook is used to notify module that menu items have been
 * created. Contributed modules may use the information to perform
 * actions based on the information entered into the menu system.
 *
 * @param $link
 *   The $link record saved into the {menu_links} table.
 * @return
 *   None.
 *
 * @see hook_menu_link_update()
 * @see hook_menu_link_delete()
 */
function hook_menu_link_insert($link) {
  // In our sample case, we track menu items as editing sections
  // of the site. These are stored in our table as 'disabled' items.
  $record['mlid'] = $link['mlid'];
  $record['menu_name'] = $link['menu_name'];
  $record['status'] = 0;
  drupal_write_record('menu_example', $record);
}

/**
 * Inform modules that a menu link has been updated.
 *
 * This hook is used to notify module that menu items have been
 * updated. Contributed modules may use the information to perform
 * actions based on the information entered into the menu system.
 *
 * @param $link
 *   The $link record saved into the {menu_links} table.
 * @return
 *   None.
 *
 * @see hook_menu_link_insert()
 * @see hook_menu_link_delete()
 */
function hook_menu_link_update($link) {
  // If the parent menu has changed, update our record.
  $menu_name = db_result(db_query("SELECT mlid, menu_name, status FROM {menu_example} WHERE mlid = :mlid", array(':mlid' => $link['mlid'])));
  if ($menu_name != $link['menu_name']) {
    db_update('menu_example')
      ->fields(array('menu_name' => $link['menu_name']))
      ->condition('mlid', $link['mlid'])
      ->execute();
  }
}

/**
 * Inform modules that a menu link has been deleted.
 *
 * This hook is used to notify module that menu items have been
 * deleted. Contributed modules may use the information to perform
 * actions based on the information entered into the menu system.
 *
 * @param $link
 *   The $link record saved into the {menu_links} table.
 * @return
 *   None.
 *
 * @see hook_menu_link_insert()
 * @see hook_menu_link_update()
 */
function hook_menu_link_delete($link) {
  // Delete the record from our table.
  db_delete('menu_example')
    ->condition('mlid', $link['mlid'])
    ->execute();
}

/**
 * @} End of "addtogroup hooks".
 */

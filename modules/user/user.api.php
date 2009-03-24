<?php
// $Id: user.api.php,v 1.4 2009/03/14 23:01:38 webchick Exp $

/**
 * @file
 * Hooks provided by the User module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Act on user account actions.
 *
 * This hook allows modules to react when operations are performed on user
 * accounts.
 *
 * @param $op
 *   What kind of action is being performed. Possible values (in alphabetical order):
 *   - "after_update": The user object has been updated and changed. Use this if
 *     (probably along with 'insert') if you want to reuse some information from
 *     the user object.
 *   - "categories": A set of user information categories is requested.
 *   - "form": The user account edit form is about to be displayed. The module
 *     should present the form elements it wishes to inject into the form.
 *   - "insert": The user account is being added. The module should save its
 *     custom additions to the user object into the database and set the saved
 *     fields to NULL in $edit.
 *   - "load": The user account is being loaded. The module may respond to this
 *   - "login": The user just logged in.
 *   - "logout": The user just logged out.
 *     and insert additional information into the user object.
 *   - "register": The user account registration form is about to be displayed.
 *     The module should present the form elements it wishes to inject into the
 *     form.
 *   - "submit": Modify the account before it gets saved.
 *   - "update": The user account is being changed. The module should save its
 *     custom additions to the user object into the database and set the saved
 *     fields to NULL in $edit.
 *   - "validate": The user account is about to be modified. The module should
 *     validate its custom additions to the user object, registering errors as
 *     necessary.
 *   - "view": The user's account information is being displayed. The module
 *     should format its custom additions for display and add them to the
 *     $account->content array.
 * @param &$edit
 *   The array of form values submitted by the user.
 * @param &$account
 *   The user object on which the operation is being performed.
 * @param $category
 *   The active category of user information being edited.
 * @return
 *   This varies depending on the operation.
 *   - "categories": A linear array of associative arrays. These arrays have
 *     keys:
 *     - "name": The internal name of the category.
 *     - "title": The human-readable, localized name of the category.
 *     - "weight": An integer specifying the category's sort ordering.
 *   - "delete": None.
 *   - "form", "register": A $form array containing the form elements to display.
 *   - "insert": None.
 *   - "load": None.
 *   - "login": None.
 *   - "logout": None.
 *   - "submit": None:
 *   - "update": None.
 *   - "validate": None.
 *   - "view": None. For an example see: user_user().
 */
function hook_user($op, &$edit, &$account, $category = NULL) {
  if ($op == 'form' && $category == 'account') {
    $form['comment_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Comment settings'),
      '#collapsible' => TRUE,
      '#weight' => 4);
    $form['comment_settings']['signature'] = array(
      '#type' => 'textarea',
      '#title' => t('Signature'),
      '#default_value' => $edit['signature'],
      '#description' => t('Your signature will be publicly displayed at the end of your comments.'));
    return $form;
  }
}
/**
 * Act on user objects when loaded from the database.
 *
 * Due to the static cache in user_load_multiple() you should not use this
 * hook to modify the user properties returned by the {users} table itself
 * since this may result in unreliable results when loading from cache.
 *
 * @param $users
 *   An array of user objects, indexed by uid.
 *
 * @see user_load_multiple()
 * @see profile_user_load()
 */
function hook_user_load($users) {
  $result = db_query('SELECT * FROM {my_table} WHERE uid IN (:uids)', array(':uids' => array_keys($users)));
  foreach ($result as $record) {
    $users[$record->uid]->foo = $result->foo;
  }
}

/**
 * Act on user account cancellations.
 *
 * The user account is being canceled. Depending on the account cancellation
 * method, the module should either do nothing, unpublish content, anonymize
 * content, or delete content and data belonging to the canceled user account.
 *
 * Expensive operations should be added to the global batch with batch_set().
 *
 * @param $edit
 *   The array of form values submitted by the user.
 * @param $account
 *   The user object on which the operation is being performed.
 * @param $method
 *   The account cancellation method.
 *
 * @see user_cancel_methods()
 * @see hook_user_cancel_methods_alter()
 * @see user_cancel()
 */
function hook_user_cancel($edit, $account, $method) {
  switch ($method) {
    case 'user_cancel_block_unpublish':
      // Unpublish nodes (current revisions).
      module_load_include('inc', 'node', 'node.admin');
      $nodes = db_select('node', 'n')->fields('n', array('nid'))->condition('uid', $account->uid)->execute()->fetchCol();
      node_mass_update($nodes, array('status' => 0));
      break;

    case 'user_cancel_reassign':
      // Anonymize nodes (current revisions).
      module_load_include('inc', 'node', 'node.admin');
      $nodes = db_select('node', 'n')->fields('n', array('nid'))->condition('uid', $account->uid)->execute()->fetchCol();
      node_mass_update($nodes, array('uid' => 0));
      // Anonymize old revisions.
      db_update('node_revision')->fields(array('uid' => 0))->condition('uid', $account->uid)->execute();
      // Clean history.
      db_delete('history')->condition('uid', $account->uid)->execute();
      break;

    case 'user_cancel_delete':
      // Delete nodes (current revisions).
      $nodes = db_select('node', 'n')->fields('n', array('nid'))->condition('uid', $account->uid)->execute()->fetchCol();
      foreach ($nodes as $nid) {
        node_delete($nid);
      }
      // Delete old revisions.
      db_delete('node_revision')->condition('uid', $account->uid)->execute();
      // Clean history.
      db_delete('history')->condition('uid', $account->uid)->execute();
      break;
  }
}

/**
 * Modify account cancellation methods.
 *
 * By implementing this hook, modules are able to add, customize, or remove
 * account cancellation methods. All defined methods are turned into radio
 * button form elements by user_cancel_methods() after this hook is invoked.
 * The following properties can be defined for each method:
 * - title: The radio button's title.
 * - description: (optional) A description to display on the confirmation form
 *   if the user is not allowed to select the account cancellation method. The
 *   description is NOT used for the radio button, but instead should provide
 *   additional explanation to the user seeking to cancel their account.
 * - access: (optional) A boolean value indicating whether the user can access
 *   a method. If #access is defined, the method cannot be configured as default
 *   method.
 *
 * @param &$methods
 *   An array containing user account cancellation methods, keyed by method id.
 *
 * @see user_cancel_methods()
 * @see user_cancel_confirm_form()
 */
function hook_user_cancel_methods_alter(&$methods) {
  // Limit access to disable account and unpublish content method.
  $methods['user_cancel_block_unpublish']['access'] = user_access('administer site configuration');

  // Remove the content re-assigning method.
  unset($methods['user_cancel_reassign']);

  // Add a custom zero-out method.
  $methods['mymodule_zero_out'] = array(
    'title' => t('Delete the account and remove all content.'),
    'description' => t('All your content will be replaced by empty strings.'),
    // access should be used for administrative methods only.
    'access' => user_access('access zero-out account cancellation method'),
  );
}

/**
 * Add mass user operations.
 *
 * This hook enables modules to inject custom operations into the mass operations
 * dropdown found at admin/user/user, by associating a callback function with
 * the operation, which is called when the form is submitted. The callback function
 * receives one initial argument, which is an array of the checked users.
 *
 * @return
 *   An array of operations. Each operation is an associative array that may
 *   contain the following key-value pairs:
 *   - "label": Required. The label for the operation, displayed in the dropdown menu.
 *   - "callback": Required. The function to call for the operation.
 *   - "callback arguments": Optional. An array of additional arguments to pass to
 *     the callback function.
 *
 */
function hook_user_operations() {
  $operations = array(
    'unblock' => array(
      'label' => t('Unblock the selected users'),
      'callback' => 'user_user_operations_unblock',
    ),
    'block' => array(
      'label' => t('Block the selected users'),
      'callback' => 'user_user_operations_block',
    ),
    'cancel' => array(
      'label' => t('Cancel the selected user accounts'),
    ),
  );
  return $operations;
}

/**
 * @} End of "addtogroup hooks".
 */

<?php
// $Id: update.php,v 1.307 2009/10/09 07:48:06 webchick Exp $

/**
 * Root directory of Drupal installation.
 */
define('DRUPAL_ROOT', getcwd());

/**
 * @file
 * Administrative page for handling updates from one Drupal version to another.
 *
 * Point your browser to "http://www.example.com/update.php" and follow the
 * instructions.
 *
 * If you are not logged in using either the site maintenance account or an
 * account with the "Administer software updates" permission, you will need to
 * modify the access check statement inside your settings.php file. After
 * finishing the upgrade, be sure to open settings.php again, and change it
 * back to its original state!
 */

/**
 * Global flag to identify update.php run, and so avoid various unwanted
 * operations, such as hook_init() and hook_exit() invokes, css/js preprocessing
 * and translation, and solve some theming issues. This flag is checked on several
 * places in Drupal code (not just update.php).
 */
define('MAINTENANCE_MODE', 'update');

function update_selection_page() {
  drupal_set_title('Drupal database update');
  $output = drupal_render(drupal_get_form('update_script_selection_form'));

  update_task_list('select');

  return $output;
}

function update_script_selection_form() {
  $form = array();
  $count = 0;
  $form['start'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );

  // Ensure system.module's updates appear first
  $form['start']['system'] = array();

  $updates = update_get_update_list();
  foreach ($updates as $module => $update) {
    if (!isset($update['start'])) {
      $form['start'][$module] = array(
        '#title' => $module,
        '#item'  => $update['warning'],
        '#prefix' => '<div class="warning">',
        '#suffix' => '</div>',
      );
      continue;
    }
    if (!empty($update['pending'])) {
      $form['start'][$module] = array(
        '#type' => 'hidden',
        '#value' => $update['start'],
      );
      $form['start'][$module . '_updates'] = array(
        '#markup' => theme('item_list', array('items' => $update['pending'], 'title' => $module . ' module')),
      );
    }
    if (isset($update['pending'])) {
      $count = $count + count($update['pending']);
    }
  }

  if (empty($count)) {
    drupal_set_message(t('No pending updates.'));
    unset($form);
    $form['links'] = array(
      '#markup' => theme('item_list', array('items' => update_helpful_links())),
    );
  }
  else {
    $form['help'] = array(
      '#markup' => '<p>The version of Drupal you are updating from has been automatically detected.</p>',
      '#weight' => -5,
    );
    $form['start']['#title'] = format_plural($count, '1 pending update', '@count pending updates');
    $form['has_js'] = array(
      '#type' => 'hidden',
      '#default_value' => FALSE,
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Apply pending updates',
    );
  }
  return $form;
}

function update_helpful_links() {
  // NOTE: we can't use l() here because the URL would point to 'update.php?q=admin'.
  $links[] = '<a href="' . base_path() . '">Front page</a>';
  $links[] = '<a href="' . base_path() . '?q=admin">Administration pages</a>';
  return $links;
}

function update_results_page() {
  drupal_set_title('Drupal database update');
  $links = update_helpful_links();

  update_task_list();
  // Report end result
  if (module_exists('dblog')) {
    $log_message = ' All errors have been <a href="' . base_path() . '?q=admin/reports/dblog">logged</a>.';
  }
  else {
    $log_message = ' All errors have been logged.';
  }

  if ($_SESSION['update_success']) {
    $output = '<p>Updates were attempted. If you see no failures below, you may proceed happily to the <a href="' . base_path() . '?q=admin">administration pages</a>. Otherwise, you may need to update your database manually.' . $log_message . '</p>';
  }
  else {
    list($module, $version) = array_pop(reset($_SESSION['updates_remaining']));
    $output = '<p class="error">The update process was aborted prematurely while running <strong>update #' . $version . ' in ' . $module . '.module</strong>.' . $log_message;
    if (module_exists('dblog')) {
      $output .= ' You may need to check the <code>watchdog</code> database table manually.';
    }
    $output .= '</p>';
  }

  if (!empty($GLOBALS['update_free_access'])) {
    $output .= "<p><strong>Reminder: don't forget to set the <code>\$update_free_access</code> value in your <code>settings.php</code> file back to <code>FALSE</code>.</strong></p>";
  }

  $output .= theme('item_list', array('items' => $links));

  // Output a list of queries executed
  if (!empty($_SESSION['update_results'])) {
    $output .= '<div id="update-results">';
    $output .= '<h2>The following updates returned messages</h2>';
    foreach ($_SESSION['update_results'] as $module => $updates) {
      $output .= '<h3>' . $module . ' module</h3>';
      foreach ($updates as $number => $queries) {
        if ($number != '#abort') {
          $messages = array();
          foreach ($queries as $query) {
            // If there is no message for this update, don't show anything.
            if (empty($query['query'])) {
              continue;
            }
            if ($query['success']) {
              $messages[] = '<li class="success">' . $query['query'] . '</li>';
            }
            else {
              $messages[] = '<li class="failure"><strong>Failed:</strong> ' . $query['query'] . '</li>';
            }
          }

          if ($messages) {
            $output .= '<h4>Update #' . $number . "</h4>\n";
            $output .= '<ul>' . implode("\n", $messages) . "</ul>\n";
          }
        }
        $output .= '</ul>';
      }
    }
    $output .= '</div>';
  }
  unset($_SESSION['update_results']);
  unset($_SESSION['update_success']);

  return $output;
}

function update_info_page() {
  // Change query-strings on css/js files to enforce reload for all users.
  _drupal_flush_css_js();
  // Flush the cache of all data for the update status module.
  if (db_table_exists('cache_update')) {
    cache_clear_all('*', 'cache_update', TRUE);
  }

  update_task_list('info');
  drupal_set_title('Drupal database update');
  $token = drupal_get_token('update');
  $output = '<p>Use this utility to update your database whenever a new release of Drupal or a module is installed.</p><p>For more detailed information, see the <a href="http://drupal.org/node/258">Installation and upgrading handbook</a>. If you are unsure what these terms mean you should probably contact your hosting provider.</p>';
  $output .= "<ol>\n";
  $output .= "<li><strong>Back up your database</strong>. This process will change your database values and in case of emergency you may need to revert to a backup.</li>\n";
  $output .= "<li><strong>Back up your code</strong>. Hint: when backing up module code, do not leave that backup in the 'modules' or 'sites/*/modules' directories as this may confuse Drupal's auto-discovery mechanism.</li>\n";
  $output .= '<li>Put your site into <a href="' . base_path() . '?q=admin/config/development/maintenance">maintenance mode</a>.</li>' . "\n";
  $output .= "<li>Install your new files in the appropriate location, as described in the handbook.</li>\n";
  $output .= "</ol>\n";
  $output .= "<p>When you have performed the steps above, you may proceed.</p>\n";
  $output .= '<form method="post" action="update.php?op=selection&amp;token=' . $token . '"><p><input type="submit" value="Continue" /></p></form>';
  $output .= "\n";
  return $output;
}

function update_access_denied_page() {
  drupal_add_http_header('403 Forbidden');
  watchdog('access denied', 'update.php', NULL, WATCHDOG_WARNING);
  drupal_set_title('Access denied');
  return '<p>Access denied. You are not authorized to access this page. Please log in using either an account with the <em>administer software updates</em> permission or the site maintenance account (the account you created during installation). If you cannot log in, you will have to edit <code>settings.php</code> to bypass this access check. To do this:</p>
<ol>
 <li>With a text editor find the settings.php file on your system. From the main Drupal directory that you installed all the files into, go to <code>sites/your_site_name</code> if such directory exists, or else to <code>sites/default</code> which applies otherwise.</li>
 <li>There is a line inside your settings.php file that says <code>$update_free_access = FALSE;</code>. Change it to <code>$update_free_access = TRUE;</code>.</li>
 <li>As soon as the update.php script is done, you must change the settings.php file back to its original form with <code>$update_free_access = FALSE;</code>.</li>
 <li>To avoid having this problem in the future, remember to log in to your website using either an account with the <em>administer software updates</em> permission or the site maintenance account (the account you created during installation) before you backup your database at the beginning of the update process.</li>
</ol>';
}

/**
 * Determines if the current user is allowed to run update.php.
 *
 * @return
 *   TRUE if the current user should be granted access, or FALSE otherwise.
 */
function update_access_allowed() {
  global $update_free_access, $user;

  // Allow the global variable in settings.php to override the access check.
  if (!empty($update_free_access)) {
    return TRUE;
  }
  // Calls to user_access() might fail during the Drupal 6 to 7 update process,
  // so we fall back on requiring that the user be logged in as user #1.
  try {
    require_once drupal_get_path('module', 'user') . '/user.module';
    return user_access('administer software updates');
  }
  catch (Exception $e) {
    return ($user->uid == 1);
  }
}

/**
 * Add the update task list to the current page.
 */
function update_task_list($active = NULL) {
  // Default list of tasks.
  $tasks = array(
    'requirements' => 'Verify requirements',
    'info' => 'Overview',
    'select' => 'Review updates',
    'run' => 'Run updates',
    'finished' => 'Review log',
  );

  drupal_add_region_content('sidebar_first', theme('task_list', array('items' => $tasks, 'active' => $active)));
}

/**
 * Returns (and optionally stores) extra requirements that only apply during
 * particular parts of the update.php process.
 */
function update_extra_requirements($requirements = NULL) {
  static $extra_requirements = array();
  if (isset($requirements)) {
    $extra_requirements += $requirements;
  }
  return $extra_requirements;
}

/**
 * Check update requirements and report any errors.
 */
function update_check_requirements() {
  // Check the system module and update.php requirements only.
  $requirements = module_invoke('system', 'requirements', 'update');
  $requirements += update_extra_requirements();
  $severity = drupal_requirements_severity($requirements);

  // If there are issues, report them.
  if ($severity == REQUIREMENT_ERROR) {
    update_task_list('requirements');
    drupal_set_title('Requirements problem');
    $status_report = theme('status_report', array('requirements' => $requirements));
    $status_report .= 'Please check the error messages and <a href="' . request_uri() . '">try again</a>.';
    print theme('update_page', array('content' => $status_report));
    exit();
  }
}

// Some unavoidable errors happen because the database is not yet up-to-date.
// Our custom error handler is not yet installed, so we just suppress them.
ini_set('display_errors', FALSE);

// We prepare a minimal bootstrap for the update requirements check to avoid
// reaching the PHP memory limit.
require_once DRUPAL_ROOT . '/includes/bootstrap.inc';
require_once DRUPAL_ROOT . '/includes/update.inc';
require_once DRUPAL_ROOT . '/includes/common.inc';
require_once DRUPAL_ROOT . '/includes/entity.inc';
update_prepare_d7_bootstrap();

// Determine if the current user has access to run update.php.
drupal_bootstrap(DRUPAL_BOOTSTRAP_SESSION);

// Only allow the requirements check to proceed if the current user has access
// to run updates (since it may expose sensitive information about the site's
// configuration).
$op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
if (empty($op) && update_access_allowed()) {
  require_once DRUPAL_ROOT . '/includes/install.inc';
  require_once DRUPAL_ROOT . '/includes/file.inc';
  require_once DRUPAL_ROOT . '/modules/system/system.install';

  // Load module basics.
  include_once DRUPAL_ROOT . '/includes/module.inc';
  $module_list['system']['filename'] = 'modules/system/system.module';
  $module_list['filter']['filename'] = 'modules/filter/filter.module';
  module_list(TRUE, FALSE, FALSE, $module_list);
  drupal_load('module', 'system');
  drupal_load('module', 'filter');

  // Reset the module_implements() cache so that any new hook implementations
  // in updated code are picked up.
  module_implements('', FALSE, TRUE);

  // Set up $language, since the installer components require it.
  drupal_language_initialize();

  // Set up theme system for the maintenance page.
  drupal_maintenance_theme();

  // Check the update requirements for Drupal.
  update_check_requirements();

  // Redirect to the update information page if all requirements were met.
  install_goto('update.php?op=info');
}

drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
drupal_maintenance_theme();

// Turn error reporting back on. From now on, only fatal errors (which are
// not passed through the error handler) will cause a message to be printed.
ini_set('display_errors', TRUE);

// Only proceed with updates if the user is allowed to run them.
if (update_access_allowed()) {

  include_once DRUPAL_ROOT . '/includes/install.inc';
  include_once DRUPAL_ROOT . '/includes/batch.inc';
  drupal_load_updates();

  update_fix_d7_requirements();
  update_fix_compatibility();

  $op = isset($_REQUEST['op']) ? $_REQUEST['op'] : '';
  switch ($op) {
    // update.php ops

    case 'selection':
      if (isset($_GET['token']) && $_GET['token'] == drupal_get_token('update')) {
        $output = update_selection_page();
        break;
      }

    case 'Apply pending updates':
      if (isset($_GET['token']) && $_GET['token'] == drupal_get_token('update')) {
        update_batch($_POST['start'], $base_url . '/update.php?op=results', $base_url . '/update.php');
        break;
      }

    case 'info':
      $output = update_info_page();
      break;

    case 'results':
      $output = update_results_page();
      break;

    // Regular batch ops : defer to batch processing API
    default:
      update_task_list('run');
      $output = _batch_page();
      break;
  }
}
else {
  $output = update_access_denied_page();
}
if (isset($output) && $output) {
  // We defer the display of messages until all updates are done.
  $progress_page = ($batch = batch_get()) && isset($batch['running']);
  print theme('update_page', array('content' => $output, 'show_messages' => !$progress_page));
}

<?php
// $Id: filter.api.php,v 1.12 2009/08/26 10:28:45 dries Exp $

/**
 * @file
 * Hooks provided by the Filter module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Define content filters.
 *
 * Content in Drupal is passed through a group of filters before it is
 * output. This lets a module modify content to the site administrator's
 * liking.
 *
 * This hook allows modules to declare input filters they provide.
 *
 * A module can contain as many filters as it wants.
 *
 * Filtering is a two-step process. First, the content is 'prepared' by calling
 * the 'prepare callback' function for every filter. The purpose of the 'prepare
 * callback' is to escape HTML-like structures. For example, imagine a filter
 * which allows the user to paste entire chunks of programming code without
 * requiring manual escaping of special HTML characters like @< or @&. If the
 * programming code were left untouched, then other filters could think it was
 * HTML and change it. For most filters however, the prepare-step is not
 * necessary, and they can just return the input without changes.
 *
 * Filters should not use the 'prepare callback' step for anything other than
 * escaping, because that would short-circuits the control the user has over the
 * order in which filters are applied.
 *
 * The second step is the actual processing step. The result from the prepare
 * step gets passed to all the filters again, this time with the 'process
 * callback' function. It's here that filters should perform actual changing of
 * the content: transforming URLs into hyperlinks, converting smileys into
 * images, etc.
 *
 * An important aspect of the filtering system is 'text formats'. Every text
 * format is an entire filter setup: which filters to enable, in what order
 * and with what settings. Filters that provide settings should usually store
 * these settings per format.
 *
 * If the filter's behavior depends on an extensive list and/or external data
 * (e.g. a list of smileys, a list of glossary terms) then filters are allowed
 * to provide a separate, global configuration page rather than provide settings
 * per format. In that case, there should be a link from the format-specific
 * settings to the separate settings page.
 *
 * For performance reasons content is only filtered once; the result is stored
 * in the cache table and retrieved the next time the piece of content is
 * displayed. If a filter's output is dynamic it can override the cache
 * mechanism, but obviously this feature should be used with caution: having one
 * filter that doesn't support caching in a particular text format disables
 * caching for the entire format, not just for one filter.
 *
 * Beware of the filter cache when developing your module: it is advised to set
 * your filter to 'cache' => FALSE while developing, but be sure to remove it
 * again if it's not needed. You can clear the cache by running the SQL query
 * 'DELETE * FROM cache_filter';
 *
 * @return
 *   An array of filter items. Each filter item has a unique name, prefixed
 *   with the name of the module that provides it. The item is an associative
 *   array that may contain the following key-value pairs:
 *   - 'title': Required. The title of the filter.
 *   - 'description': Short description of what this filter does.
 *   - 'prepare callback': The callback function to call in the 'prepare' step
 *     of the filtering.
 *   - 'process callback': Required. The callback function to call in the
 *     'process' step of the filtering.
 *   - 'settings callback': The callback function that provides form controls
 *     for the filter's settings. These settings are stored with variable_set()
 *     when the form is submitted. Remember to use the $format identifier in the
 *     variable and control names  to store settings per text format (e.g.
 *     'mymodule_setting_$format').
 *   - 'tips callback': The callback function that provide tips for using
 *     filters. A module's tips should be informative and to the point. Short
 *     tips are preferably one-liners.
 *   - 'cache': Specify if the filter result can be cached. TRUE by default.
 *
 * For a detailed usage example, see filter_example.module. For an example of
 * using multiple filters in one module, see filter_filter_info().
 */
function hook_filter_info() {
  $filters['filter_html'] = array(
    'title' => t('Limit allowed HTML tags'),
    'description' => t('Allows you to restrict the HTML tags the user can use. It will also remove harmful content such as JavaScript events, JavaScript URLs and CSS styles from those tags that are not removed.'),
    'process callback' => '_filter_html',
    'settings callback' => '_filter_html_settings',
    'tips callback' => '_filter_html_tips',
  );
  $filters['filter_autop'] = array(
    'title' => t('Convert line breaks'),
    'description' => t('Converts line breaks into HTML (i.e. &lt;br&gt; and &lt;p&gt;) tags.'),
    'process callback' => '_filter_autop',
    'tips callback' => '_filter_autop_tips',
  );
  return $filters;
}

/**
 * Perform alterations on filter definitions.
 *
 * @param $info
 *   Array of information on filters exposed by hook_filter_info()
 *   implementations.
 */
function hook_filter_info_alter(&$info) {
  // Replace the PHP evaluator process callback with an improved
  // PHP evaluator provided by a module.
  $info['php_code']['process callback'] = 'my_module_php_evaluator';
}

/**
 * Perform actions when a new text format has been created.
 *
 * @param $format
 *   The format object of the format being updated.
 *
 * @see hook_filter_format_update().
 * @see hook_filter_format_delete().
 */
function hook_filter_format_insert($format) {
  mymodule_cache_rebuild();
}

/**
 * Perform actions when a text format has been updated.
 *
 * This hook allows modules to act when a text format has been updated in any
 * way. For example, when filters have been reconfigured, disabled, or
 * re-arranged in the text format.
 *
 * @param $format
 *   The format object of the format being updated.
 *
 * @see hook_filter_format_insert().
 * @see hook_filter_format_delete().
 */
function hook_filter_format_update($format) {
  mymodule_cache_rebuild();
} 

/**
 * Perform actions when a text format has been deleted.
 *
 * It is recommended for modules to implement this hook, when they store
 * references to text formats to replace existing references to the deleted
 * text format with the default format.
 *
 * @param $format
 *   The format object of the format being deleted.
 * @param $default
 *   The format object of the site's default format.
 *
 * @see hook_filter_format_update().
 * @see hook_filter_format_delete().
 */
function hook_filter_format_delete($format, $default) {
  // Replace the deleted format with the default format.
  db_update('my_module_table')
    ->fields(array('format' => $default->format))
    ->condition('format', $format->format)
    ->execute();
}

/**
 * @} End of "addtogroup hooks".
 */

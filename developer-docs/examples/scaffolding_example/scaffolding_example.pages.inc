<?php
// $Id: scaffolding_example.pages.inc,v 1.5 2008/10/12 08:52:37 davereid Exp $

/**
 * @file
 * Builds the module's user-facing pages.
 *
 * Contains a simple page callback and themable function for rendering a single
 * record.
 */

/**
 * Build a simple listing page for records.
 */
function scaffolding_example_page() {
  $output = '';

  if ($records = scaffolding_example_record_load_all()) {
    foreach ($records as $record) {
      $output .= theme('scaffolding_example_record', $record);
    }
  }
  else {
    drupal_set_message(t('No records have been added.'));
  }

  return $output;
}

/**
 * Theme a single record.
 *
 * @ingroup themeable
 */
function theme_scaffolding_example_record($record) {
  $output = '<div class="scaffolding-example-record" id="scaffolding-example-record-' . check_plain($record['record_id']) . '">';
  $output .= '<h3 class="title">' . check_plain($record['title']) . '</h3>';
  $output .= '<p>' . check_markup($record['content']) . '</p>';
  $output .= '</div>';

  return $output;
}

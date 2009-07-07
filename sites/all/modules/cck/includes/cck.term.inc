<?php
// $Id$

/**
 * Implementation of hook_field_settings_form()
 * on behalf of core List module.
 */

function taxonomy_field_settings_form($field, $instance) {
  // Get the right values for allowed_values_function, which is a core setting.
  $options = array();
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $vocabulary) {
    $options[$vocabulary->vid] = $vocabulary->name;
  }
  $form['vid'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary'),
    '#default_value' => $field['settings']['vid'],
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('The vocabulary which supplies the options for this field.'),
  );
  $form['parent'] = array(
    '#type' => 'value',
    '#value' => $field['settings']['parent'],
  );

  return $form;
}

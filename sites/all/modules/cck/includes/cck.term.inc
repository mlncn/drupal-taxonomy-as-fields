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
  $form['allowed_values'] = array(
    '#tree' => TRUE,
  );
  foreach ($field['settings']['allowed_values'] as $delta => $tree) {
    $form['allowed_values'][$delta]['vid'] = array(
      '#type' => 'select',
      '#title' => t('Vocabulary'),
      '#default_value' => $tree['vid'],
      '#options' => $options,
      '#required' => TRUE,
      '#description' => t('The vocabulary which supplies the options for this field.'),
    );
    $form['allowed_values'][$delta]['parent'] = array(
      '#type' => 'value',
      '#value' => $tree['parent'],
    );
  }

  return $form;
}

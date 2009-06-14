<?php
// $Id$

/**
 * Implementation of hook_field_settings_form()
 * on behalf of core List module.
 */
function term_field_settings_form($field, $instance) {

  // Alter the description for allowed values slightly depending
  // on the type of widget.
  $widget_type = $instance['widget']['type'];
  $field_type = $field['type'];
  $label = $instance['label'];

  // Get the right values for allowed_values_function, which is a core setting.
  $defaults = field_info_field_settings($field['type']);
  $settings = array_merge($defaults, $field['settings']);
  $options = array();
  $vocabularies = taxonomy_get_vocabularies();
  foreach ($vocabularies as $vocabulary) {
    $options[$vocabulary->vid] = $vocabulary->name;
  }
  $form['vid'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary'),
    '#default_value' => $settings['vid'],
    '#options' => $options,
    '#required' => TRUE,
    '#description' => t('The vocabulary which supplies the options for this field.'),
  );

  return $form;
}

<?php
// $Id: cck.number.inc,v 1.6 2009/03/16 22:06:28 yched Exp $

/**
 * Implementation of hook_field_settings_form() on behalf of core Number module.
 */
function number_field_settings_form($field) {
  $form = array();
  if ($field['type'] == 'number_decimal') {
    $form['precision'] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(range(10, 32)),
      '#title' => t('Precision'),
      '#description' => t('The total number of digits to store in the database, including those to the right of the decimal.'),
      '#default_value' => $field['settings']['precision'],
    );
    $form['scale'] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(range(0, 10)),
      '#title' => t('Scale'),
      '#description' => t('The number of digits to the right of the decimal.'),
      '#default_value' => $field['settings']['scale'],
    );
    $form['decimal'] = array(
      '#type' => 'select',
      '#options' => array('.' => 'decimal point', ',' => 'comma', ' ' => 'space'),
      '#title' => t('Decimal marker'),
      '#description' => t('The character users will input to mark the decimal point in forms.'),
      '#default_value' => $field['settings']['decimal'],
    );
  }
  return $form;
}

function number_field_instance_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $field_type = field_info_field_types($field['type']);
  $defaults = field_info_instance_settings($field['type']);
  $settings = array_merge($defaults, $instance['settings']);

  $form['min'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum'),
    '#element_validate' => array('_element_validate_number'),
    '#default_value' => $instance['settings']['min'],
    '#description' => t('The minimum value that should be allowed in this field. Leave blank for no minimum.')
  );
  $form['max'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum'),
    '#element_validate' => array('_element_validate_number'),
    '#default_value' => $instance['settings']['max'],
    '#description' => t('The maximum value that should be allowed in this field. Leave blank for no maximum.')
  );
  $form['prefix'] = array(
    '#type' => 'textfield',
    '#title' => t('Prefix'),
    '#size' => 60,
    '#default_value' => $instance['settings']['prefix'],
    '#description' => t('Define a string that should be prefixed to the value, like $ or €. Leave blank for none. Separate singular and plural values with a pipe (pound|pounds).'),
  );
  $form['suffix'] = array(
    '#type' => 'textfield',
    '#title' => t('Suffix'),
    '#size' => 60,
    '#default_value' => $instance['settings']['suffix'],
    '#description' => t('Define a string that should suffixed to the value, like m², m/s², kb/s. Leave blank for none. Separate singular and plural values with a pipe (pound|pounds).'),
  );
  return $form;
}
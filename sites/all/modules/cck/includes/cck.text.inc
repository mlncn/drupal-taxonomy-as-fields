<?php
// $Id: cck.text.inc,v 1.7 2009/04/15 01:10:33 yched Exp $

/**
 * Implementation of hook_field_settings_form()
 * on behalf of core Text module.
 */
function text_field_settings_form($field) {
  $form = array();
  $defaults = field_info_field_settings($field['type']);
  $settings = array_merge($defaults, $field['settings']);
  if (isset($defaults['max_length'])) {
    $form['max_length'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum length'),
      '#default_value' => !empty($settings['max_length']) && is_numeric($settings['max_length']) ? $settings['max_length'] :  $defaults['max_length'],
      '#required' => FALSE,
      '#element_validate' => array('_element_validate_integer_positive'),
      '#description' => t('The maximum length of the field in characters. Leave blank for an unlimited size.'),
    );
  }
  return $form;
}

/**
 * Implementation of hook_widget_settings_form()
 * on behalf of core Text module.
 */
function text_widget_settings_form($instance) {
  $form = array();
  $widget = $instance['widget'];
  $defaults = field_info_widget_settings($widget['type']);
  $settings = array_merge($defaults, $widget['settings']);
  if ($widget['type'] == 'text_textfield') {
    $form['size'] = array(
      '#type' => 'textfield',
      '#title' => t('Size of textfield'),
      '#default_value' => $settings['size'],
      '#element_validate' => array('_element_validate_integer_positive'),
      '#required' => TRUE,
    );
  }
  else {
    $form['rows'] = array(
      '#type' => 'textfield',
      '#title' => t('Rows'),
      '#default_value' => $settings['rows'],
      '#element_validate' => array('_element_validate_integer_positive'),
      '#required' => TRUE,
    );
  }
  return $form;
}

/**
 * Implementation of hook_field_instance_settings_form()
 * on behalf of core Text module.
 */
function text_field_instance_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $field_type = field_info_field_types($field['type']);
  $defaults = field_info_instance_settings($field['type']);
  $settings = array_merge($defaults, $instance['settings']);
  $options = array(0 => t('Plain text'), 1 => t('Filtered text (user selects input format)'));
  $form['text_processing'] = array(
    '#type' => 'radios',
    '#title' => t('Text processing'),
    '#default_value' => !empty($settings['text_processing']) && is_numeric($settings['text_processing']) ? $settings['text_processing'] : $defaults['text_processing'],
    '#options' => $options,
  );
  if ($field['type'] == 'text_with_summary') {
    $form['display_summary'] = array(
      '#type' => 'select',
      '#options' => array(0 => t('No'), 1 => t('Yes')),
      '#title' => t('Display summary'),
      '#description' => t('Display the summary to allow the user to input a summary value. Hide the summary to automatically fill it with a trimmed portion from the main post. '),
      '#default_value' => !empty($settings['display_summary']) ? $settings['display_summary'] :  0,
    );
  }
  return $form;
}
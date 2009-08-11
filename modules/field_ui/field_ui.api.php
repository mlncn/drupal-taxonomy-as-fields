<?php
// $Id$

/**
 * @file
 * Hooks provided by the Field UI module.
 */

/**
 * @defgroup field_ui_fieldable_type Fieldable Types API
 * @{
 */

/**
 * Expose 'psuedo-field' components on fieldable objects.
 *
 * Field UI's 'Manage fields' page lets users reorder fields, but also
 * non-field components. For nodes, that would be title, menu settings, or
 * other hook_node()-added elements by contributed modules...
 *
 * Contributed modules that want to have their components supported should
 * expose them using this hook, and use field_ui_extra_field_weight() to
 * retrieve the user-defined weight when inserting the component.
 *
 * @param $bundle
 *   The name of the bundle being considered.
 * @return
 *   An array of 'pseudo-field' components.
 *   The keys are the name of the element as it appears in the form structure.
 *   The values are arrays with the following key/value pairs:
 *   - label: the human readable name of the component.
 *   - description: a short description of the component contents.
 *   - weight: the default weight of the element.
 *   - view: (optional) the name of the element as it appears in the render
 *     structure, if different from the name in the form.
 */
function hook_field_ui_extra_fields($bundle) {
  $extra = array();
  if ($type = node_type_get_type($bundle)) {
    if ($type->has_title) {
      $extra['title'] = array(
        'label' => $type->title_label,
        'description' => t('Node module element.'),
        'weight' => -5
      );
    }
    $extra['revision_information'] = array(
      'label' => t('Revision information'),
      'description' => t('Node module form.'),
      'weight' => 20
    );
    $extra['author'] = array(
      'label' => t('Authoring information'),
      'description' => t('Node module form.'),
      'weight' => 20
    );
    $extra['options'] = array(
      'label' => t('Publishing options'),
      'description' => t('Node module form.'),
      'weight' => 25
    );
    if (module_exists('comment') && variable_get("comment_$bundle", 2) != 0) {
      $extra['comment_settings'] = array(
        'label' => t('Comment settings'),
        'description' => t('Comment module form.'),
        'weight' => 30
      );
    }
    if (module_exists('locale') && variable_get("language_content_type_$bundle", 0)) {
      $extra['language'] = array(
        'label' => t('Language'),
        'description' => t('Locale module element.'),
        'weight' => 0
      );
    }
    if (module_exists('menu')) {
      $extra['menu'] = array(
        'label' => t('Menu settings'),
        'description' => t('Menu module element.'),
        'weight' => -2
      );
    }
    if (module_exists('taxonomy') && taxonomy_get_vocabularies($bundle)) {
      $extra['taxonomy'] = array(
        'label' => t('Taxonomy'),
        'description' => t('Taxonomy module element.'),
        'weight' => -3
      );
    }
    if (module_exists('book')) {
      $extra['book'] = array(
        'label' => t('Book'),
        'description' => t('Book module element.'),
        'weight' => 10
      );
    }
    if ($bundle == 'poll' && module_exists('poll')) {
      $extra['title'] = array(
        'label' => t('Poll title'),
        'description' => t('Poll module title.'),
        'weight' => -5
      );
      $extra['choice_wrapper'] = array(
        'label' => t('Poll choices'),
        'description' => t('Poll module choices.'),
        'weight' => -4
      );
      $extra['settings'] = array(
        'label' => t('Poll settings'),
        'description' => t('Poll module settings.'),
        'weight' => -3
      );
    }
    if (module_exists('upload') && variable_get("upload_$bundle", TRUE)) {
      $extra['attachments'] = array(
        'label' => t('File attachments'),
        'description' => t('Upload module element.'),
        'weight' => 30,
        'view' => 'files'
      );
    }
  }

  return $extra;
}

/**
 * @} End of "ingroup field_ui_fieldable_type"
 */

/**
 * @ingroup field_ui_field_type
 * @{
 */

/**
 * Field settings form.
 *
 * @param $field
 *   The field structure being configured.
 * @param $instance
 *   The instance structure being configured.
 * @return
 *   The form definition for the field settings.
 */
function hook_field_settings_form($field, $instance) {
  $settings = $field['settings'];
  $form['max_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum length'),
    '#default_value' => $settings['max_length'],
    '#required' => FALSE,
    '#element_validate' => array('_element_validate_integer_positive'),
    '#description' => t('The maximum length of the field in characters. Leave blank for an unlimited size.'),
  );
  return $form;
}

/**
 * Instance settings form.
 *
 * @param $field
 *   The field structure being configured.
 * @param $instance
 *   The instance structure being configured.
 * @return
 *   The form definition for the field instance settings.
 */
function hook_field_instance_settings_form($field, $instance) {
  $settings = $instance['settings'];
  $options = array(0 => t('Plain text'), 1 => t('Filtered text (user selects input format)'));
  $form['text_processing'] = array(
    '#type' => 'radios',
    '#title' => t('Text processing'),
    '#default_value' => $settings['text_processing'],
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

/**
 * Widget settings form.
 *
 * @param $field
 *   The field structure being configured.
 * @param $instance
 *   The instance structure being configured.
 * @return
 *   The form definition for the widget settings.
 */
function hook_field_widget_settings_form($field, $instance) {
  $widget = $instance['widget'];
  $settings = $widget['settings'];
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
 * Formatter settings form.
 *
 * @todo Not implemented yet. The signature below is only prospective, but
 * providing $instance is not enough, since one $instance holds several display
 * settings.
 *
 * @param $formatter
 *   The type of the formatter being configured.
 * @param $settings
 *   The current values of the formatter settings.
 * @param $field
 *   The field structure being configured.
 * @param $instance
 *   The instance structure being configured.
 * @return
 *   The form definition for the formatter settings.
 */
function hook_field_formatter_settings_form($formatter, $settings, $field, $instance) {
}

/**
 * @} End of "ingroup field_ui_field_type"
 */

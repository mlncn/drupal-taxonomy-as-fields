<?php
// $Id: cck.list.inc,v 1.7 2009/03/07 18:34:21 karens Exp $

/**
 * A custom function to return allowed values from PHP code
 * or by exploding a text list.
 */
function cck_list_allowed_values_php($field) {
  $allowed_values = array();
  $php = cck_field_get_setting('allowed_values_php', 'field', $field);
  if (!empty($php)) {
    ob_start();
    $result = eval($php);
    if (is_array($result)) {
      $allowed_values = $result;
    }
    ob_end_clean();
  }
  else {
    $allowed_values = cck_allowed_values($field);
  }
  return $allowed_values;  
}

/**
 * Implementation of hook_field_settings_form()
 * on behalf of core List module.
 */
function list_field_settings_form($field, $instance) {
      
  // Alter the description for allowed values slightly depending 
  // on the type of widget.
  $widget_type = $instance['widget']['type'];
  $field_type = $field['type'];
  $label = $instance['label'];
  $description = '<p>' . t('Create a list of options as a list or a function name. These values will be the same for %field in all field types.', array('%field' => $label)) . '</p>';
  if ($widget_type == 'options_onoff') {
    $description .= '<p>' . t("For a 'single on/off checkbox' widget, define the 'off' value first, then the 'on' value in the <strong>Allowed values</strong> section. Note that the checkbox will be labeled with the label of the 'on' value.") . '</p>';
  }
  elseif ($widget_type == 'options_buttons') {
    $description .= '<p>' . t("The 'checkboxes/radio buttons' widget will display checkboxes if the cardinality option is selected for this field, otherwise radios will be displayed.") . '</p>';
  }

  $form = array(
    '#element_validate' => array('list_field_settings_form_validate'),
    '#prefix' => '<div class="form-item"><label>' . t('Allowed values') . ':</label><div class="description">' . $description . '</div></div>',
    '#field' => $field,
    );
  
  // Retrieve allowed_values and allowed_values_php, which are non-core
  // settings handled by CCK.
  $allowed_values = cck_field_get_setting('allowed_values', 'field', $field);
  $allowed_values_php = cck_field_get_setting('allowed_values_php', 'field', $field);

  // Get the right values for allowed_values_function, which is a core setting.
  $defaults = field_info_field_settings($field['type']);
  $settings = array_merge($defaults, $field['settings']);
  $allowed_values_function = !empty($settings['allowed_values_function']) && $settings['allowed_values_function'] != 'cck_list_allowed_values_php' ? $settings['allowed_values_function'] : '';
  $form['allowed_values'] = array(
    '#type' => 'textarea',
    '#title' => t('Allowed values list'),
    '#default_value' => $allowed_values,
    '#required' => FALSE,
    '#rows' => 10,
    '#description' => t('The possible values this field can contain. Enter one value per line, in the format key|label. The key is the value that will be stored in the database, and must be a %type value. The label is optional, and the key will be used as the label if no label is specified.<br />Allowed HTML tags in labels: @tags', array('%type' => $field['type'] == 'list_text' ? 'text' : 'numeric', '@tags' => _field_filter_xss_display_allowed_tags())),
  );
  $form['allowed_values_function'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed values function'),
    '#default_value' => $allowed_values_function,
    '#description' => t('The name of a function that will return the allowed values list. If this field is filled out, the array returned by this code will override the allowed values list above.'),
  );
  $form['advanced_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('PHP code for allowed values'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($allowed_values_php),
  );
  if (user_access('Use PHP input for field settings (dangerous - grant with care)')) {
    $form['advanced_options']['allowed_values_php'] = array(
      '#type' => 'textarea',
      '#title' => t('Code'),
      '#default_value' => $allowed_values_php,
      '#rows' => 6,
      '#description' => t("Advanced usage only: PHP code that returns a keyed array of allowed values. Should not include &lt;?php ?&gt; delimiters. If this field is filled out, the array returned by this code will override the allowed values list, and the allowed values function will be set to 'cck_list_allowed_values_php' to execute this custom code. <strong>It is strongly recommended that you move this code to a custom function in a custom module and simply identify the custom function in the box above!</strong>"),
    );
  }
  else {
    $form['advanced_options']['allowed_values_php'] = array(
      '#type' => 'value',
      '#value' => $allowed_values_php,
    );
    $form['advanced_options']['markup_allowed_values_php'] = array(
      '#type' => 'item',
      '#title' => t('Code'),
      '#value' => !empty($allowed_values_php) ? '<code>'. check_plain($allowed_values_php) .'</code>' : t('&lt;none&gt;'),
      '#description' => empty($allowed_values_php) ? t("You're not allowed to input PHP code.") : t('This PHP code was set by an administrator and will override the allowed values list and allowed values functions shown above.'),
    );
  }

  return $form;
}

/**
 * Handle Allowed values PHP code.
 */
function list_field_settings_form_validate($form, &$form_state) {
  $form_values = $form_state['values'];
  $field = $form['#field'];
  
  // Store allowed_values and allowed_values_php in the CCK table, 
  // they are not core settings. CCK stores them and will return the 
  // right value in response to the hook.
  $option = $form_values['field']['settings']['advanced_options']['allowed_values_php'];
  cck_field_set_setting('allowed_values_php', 'field', $option, $field);
  $option = $form_values['field']['settings']['allowed_values'];
  cck_field_set_setting('allowed_values', 'field', $option, $field);
  
  // Set allowed_values_function, which is a core setting.
  $new_values['allowed_values_function'] = $form_values['field']['settings']['allowed_values_function'];
  if (empty($new_values['allowed_values_function'])) {
    $new_values['allowed_values_function'] = 'cck_list_allowed_values_php';
  }
  form_set_value($form, $new_values, $form_state);   
}

/**
 *  Create an array of the allowed values for this field.
 *
 *  Explode a string with keys and labels separated 
 *  with '|' and with each new value on its own line.
 */
function cck_allowed_values($field) {
  $allowed_values[$field['field_name']] = array();

  $list = cck_field_get_setting('allowed_values', 'field', $field);
  $list = explode("\n", $list);
  $list = array_map('trim', $list);
  $list = array_filter($list, 'strlen');
  foreach ($list as $opt) {
    // Sanitize the user input with a permissive filter.
    $opt = field_filter_xss($opt);
    if (strpos($opt, '|') !== FALSE) {
      list($key, $value) = explode('|', $opt);
      $allowed_values[$field['field_name']][$key] = (isset($value) && $value !=='') ? $value : $key;
    }
    else {
      $allowed_values[$field['field_name']][$opt] = $opt;
    }
  }
  return $allowed_values[$field['field_name']];
}

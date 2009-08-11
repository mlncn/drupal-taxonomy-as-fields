<?php
// $Id$

/**
 * @file
 * Administrative interface for custom field type creation.
 */

/**
 * Menu callback; lists all defined fields for quick reference.
 */
function field_ui_fields_list() {
  $instances = field_info_instances();
  $field_types = field_info_field_types();
  $bundles = field_info_bundles();
  $header = array(t('Field name'), t('Field type'), t('Used in'));
  $rows = array();
  foreach ($instances as $bundle => $info) {
    foreach ($info as $field_name => $instance) {
      $field = field_info_field($field_name);
      $admin_path = _field_ui_bundle_admin_path($bundle);
      $rows[$field_name]['data'][0] = $field['locked'] ? t('@field_name (Locked)', array('@field_name' => $field_name)) : $field_name;
      $rows[$field_name]['data'][1] = t($field_types[$field['type']]['label']);
      $rows[$field_name]['data'][2][] = l($bundles[$bundle]['label'], $admin_path . '/fields');
      $rows[$field_name]['class'] = $field['locked'] ? 'menu-disabled' : '';
    }
  }
  foreach ($rows as $field_name => $cell) {
    $rows[$field_name]['data'][2] = implode(', ', $cell['data'][2]);
  }
  if (empty($rows)) {
    $output = t('No fields have been defined for any content type yet.');
  }
  else {
    // Sort rows by field name.
    ksort($rows);
    $output = theme('table', $header, $rows);
  }
  return $output;
}

/**
 * Helper function to display a message about inactive fields.
 */
function field_ui_inactive_message($bundle) {
  $inactive_instances = field_ui_inactive_instances($bundle);
  if (!empty($inactive_instances)) {
    $field_types = field_info_field_types();
    $widget_types = field_info_widget_types();

    foreach ($inactive_instances as $field_name => $instance) {
      $list[] = t('%field (@field_name) field requires the %widget_type widget provided by %widget_module module', array(
      '%field' => $instance['label'],
      '@field_name' => $instance['field_name'],
      '%widget_type' => array_key_exists($instance['widget']['type'], $widget_types) ? $widget_types[$instance['widget']['type']]['label'] : $instance['widget']['type'],
      '%widget_module' => $instance['widget']['module'],
      ));
    }
    drupal_set_message(t('Inactive fields are not shown unless their providing modules are enabled. The following fields are not enabled: !list', array('!list' => theme('item_list', $list))), 'error');
  }
}

/**
 * Menu callback; listing of fields for a content type.
 *
 * Allows fields to be reordered and nested in fieldgroups using
 * JS drag-n-drop. Non-field form elements can also be moved around.
 */
function field_ui_field_overview_form(&$form_state, $obj_type, $bundle) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  field_ui_inactive_message($bundle);
  $admin_path = _field_ui_bundle_admin_path($bundle);

  // When displaying the form, make sure the list of fields
  // is up-to-date.
  if (empty($form_state['post'])) {
    field_cache_clear();
  }

  // Gather bundle information.
  $instances = field_info_instances($bundle);
  $field_types = field_info_field_types();
  $widget_types = field_info_widget_types();

  $extra = field_ui_extra_field_values($bundle);

  $groups = $group_options = array();
  if (module_exists('fieldgroup')) {
    $groups = fieldgroup_groups($bundle);
    $group_types = fieldgroup_types();
    $group_options = _fieldgroup_groups_label($bundle);
    // Add the ability to group under the newly created row.
    $group_options['_add_new_group'] = '_add_new_group';
  }

  // Store the default weights as we meet them, to be able to put the
  //'add new' rows after them.
  $weights = array();

  $form = array(
    '#tree' => TRUE,
    '#bundle' => $bundle,
    '#fields' => array_keys($instances),
    '#groups' => array_keys($groups),
    '#extra' => array_keys($extra),
    '#field_rows' => array(),
    '#group_rows' => array(),
  );

  // Fields.
  foreach ($instances as $name => $instance) {
    $field = field_info_field($instance['field_name']);
    $admin_field_path = $admin_path . '/fields/' . $instance['field_name'];
    $weight = $instance['weight'];
    $form[$name] = array(
      'label' => array(
        '#markup' => check_plain($instance['label'])
      ),
      'field_name' => array(
        '#markup' => $instance['field_name']
      ),
      'type' => array(
        '#markup' => l(t($field_types[$field['type']]['label']), $admin_field_path . '/field-settings', array('attributes' => array('title' => t('Edit field settings.'))))
      ),
      'widget_type' => array(
        '#markup' => l(t($widget_types[$instance['widget']['type']]['label']), $admin_field_path . '/widget-type', array('attributes' => array('title' => t('Change widget type.'))))
      ),
      'configure' => array(
        '#markup' => l(t('Configure'), $admin_field_path, array('attributes' => array('title' => t('Edit instance settings.'))))
      ),
      'remove' => array(
        '#markup' => l(t('Remove'), $admin_field_path . '/remove', array('attributes' => array('title' => t('Remove instance.'))))
       ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
       ),
      'parent' => array(
        '#type' => 'select',
        '#options' => $group_options,
        '#default_value' => ''
       ),
      'prev_parent' => array(
        '#type' => 'hidden',
        '#value' => ''
       ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $instance['field_name']
       ),
      '#leaf' => TRUE,
      '#row_type' => 'field',
       // TODO: is this really needed ?
       'field' => array(
         '#type' => 'value',
         '#value' => $field
       ),
    );

    if (!empty($instance['locked'])) {
      $form[$name]['configure'] = array('#value' => t('Locked'));
      $form[$name]['remove'] = array();
      $form[$name]['#disabled_row'] = TRUE;
    }
    $form['#field_rows'][] = $name;
    $weights[] = $weight;
  }

  // Groups.
  foreach ($groups as $name => $group) {
    $weight = $group['weight'];
    $form[$name] = array(
      'label' => array(
        '#markup' => check_plain($group['label'])
      ),
      'group_name' => array(
        '#markup' => $group['group_name']
      ),
      'group_type' => array(
        '#markup' => t($group_types[$group['group_type']])
      ),
      'configure' => array(
        '#markup' => l(t('Configure'), $admin_path . '/groups/' . $group['group_name'])
      ),
      'remove' => array(
        '#markup' => l(t('Remove'), $admin_path . '/groups/' . $group['group_name'] . '/remove')
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
      ),
      'parent' => array(
        '#type' => 'hidden',
        '#default_value' => ''
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $group['group_name']
      ),
      '#root' => TRUE,
      '#row_type' => 'group',
      'group' => array(
        '#type' => 'value',
        '#value' => $group
      ),
    );
    // Adjust child fields rows.
    foreach ($group['fields'] as $field_name => $field) {
      $form[$field_name]['parent']['#default_value'] = $name;
      $form[$field_name]['prev_parent']['#value'] = $name;
    }
    $form['#group_rows'][] = $name;
    $weights[] = $weight;
  }

  // Non-field elements.
  foreach ($extra as $name => $label) {
    $weight = $extra[$name]['weight'];
    $form[$name] = array(
      'label' => array(
        '#markup' => t($extra[$name]['label'])
      ),
      'description' => array(
        '#markup' => isset($extra[$name]['description']) ? $extra[$name]['description'] : ''
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
      ),
      'parent' => array(
        '#type' => 'hidden',
        '#default_value' => ''
      ),
      'configure' => array(
        '#markup' => isset($extra[$name]['configure']) ? $extra[$name]['configure'] : ''
      ),
      'remove' => array(
        '#markup' => isset($extra[$name]['remove']) ? $extra[$name]['remove'] : ''
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $name
      ),
      '#leaf' => TRUE,
      '#root' => TRUE,
      '#disabled_row' => TRUE,
      '#row_type' => 'extra',
    );
    $form['#field_rows'][] = $name;
    $weights[] = $weight;
  }

  // Additional row : add new field.
  $weight = !empty($weights) ? max($weights) + 1 : 0;
  $field_type_options = field_ui_field_type_options();
  $widget_type_options = field_ui_widget_type_options(NULL, TRUE);
  if ($field_type_options && $widget_type_options) {
    array_unshift($field_type_options, t('- Select a field type -'));
    array_unshift($widget_type_options, t('- Select a widget -'));
    $name = '_add_new_field';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'field_name' => array(
        '#type' => 'textfield',
        // This field should stay LTR even for RTL languages.
        '#field_prefix' => '<span dir="ltr">field_',
        '#field_suffix' => '</span>&lrm;',
        '#attributes' => array('dir'=>'ltr'),
        '#size' => 15,
        '#description' => t('Field name (a-z, 0-9, _)'),
      ),
      'type' => array(
        '#type' => 'select',
        '#options' => $field_type_options,
        '#description' => t('Type of data to store.'),
      ),
      'widget_type' => array(
        '#type' => 'select',
        '#options' => $widget_type_options,
        '#description' => t('Form element to edit the data.'),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
      ),
      'parent' => array(
        '#type' => 'select',
        '#options' => $group_options,
        '#default_value' => ''
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $name
      ),
      '#leaf' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_new_field',
    );
    $form['#field_rows'][] = $name;
  }

  // Additional row : add existing field.
  $existing_field_options = field_ui_existing_field_options($bundle);
  if ($existing_field_options && $widget_type_options) {
    $weight++;
    array_unshift($existing_field_options, t('- Select an existing field -'));
    $name = '_add_existing_field';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'field_name' => array(
        '#type' => 'select',
        '#options' => $existing_field_options,
        '#description' => t('Field to share'),
      ),
      'widget_type' => array(
        '#type' => 'select',
        '#options' => $widget_type_options,
        '#description' => t('Form element to edit the data.'),
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
      ),
      'parent' => array(
        '#type' => 'select',
        '#options' => $group_options,
        '#default_value' => ''
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $name
      ),
      '#leaf' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_existing_field',
    );
    $form['#field_rows'][] = $name;
  }

  // Additional row : add new group.
  if (module_exists('fieldgroup')) {
    $weight++;
    $name = '_add_new_group';
    $form[$name] = array(
      'label' => array(
        '#type' => 'textfield',
        '#size' => 15,
        '#description' => t('Label'),
      ),
      'group_name' => array(
        '#type' => 'textfield',
        // This field should stay LTR even for RTL languages.
        '#field_prefix' => '<span dir="ltr">group_',
        '#field_suffix' => '</span>&lrm;',
        '#attributes' => array('dir'=>'ltr'),
        '#size' => 15,
        '#description' => t('Group name (a-z, 0-9, _)'),
      ),
      'group_option' => array(
        '#type' => 'hidden',
        '#value' => '',
      ),
      'group_type' => array(
        '#type' => 'hidden',
        '#value' => 'standard',
      ),
      'weight' => array(
        '#type' => 'textfield',
        '#default_value' => $weight,
        '#size' => 3
      ),
      'parent' => array(
        '#type' => 'hidden',
        '#default_value' => ''
      ),
      'hidden_name' => array(
        '#type' => 'hidden',
        '#default_value' => $name
      ),
      '#root' => TRUE,
      '#add_new' => TRUE,
      '#row_type' => 'add_new_group',
    );
    $form['#group_rows'][] = $name;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  return $form;
}

/**
 * Theme preprocess function for field_ui-field-overview-form.tpl.php.
 */
function template_preprocess_field_ui_field_overview_form(&$vars) {
  $form = &$vars['form'];

  drupal_add_css(drupal_get_path('module', 'field_ui') . '/field_ui.css');
  drupal_add_tabledrag('field-overview', 'match', 'parent', 'group-parent', 'group-parent', 'field-name', TRUE, 1);
  drupal_add_tabledrag('field-overview', 'order', 'sibling', 'field-weight');
  drupal_add_js(drupal_get_path('module', 'field_ui') . '/field_ui.js');
  // Add settings for the update selects behavior.
  $js_fields = array();
  foreach (field_ui_existing_field_options($form['#bundle']) as $field_name => $fields) {
    $field = field_info_field($field_name);
    $instance = field_info_instance($field_name, $form['#bundle']);
    $js_fields[$field_name] = array('label' => $instance['label'], 'type' => $field['type'], 'widget' => $instance['widget']['type']);
  }
  drupal_add_js(array('fieldWidgetTypes' => field_ui_widget_type_options(), 'fields' => $js_fields), 'setting');

  // TODO: Abstract texts over bundles / entity types.
  switch ($form['#bundle']) {
    case 'user':
      $vars['help'] = t('Add fields to the user, and arrange them on user display and input forms.');
      break;
    default:
      $vars['help'] = t('Add fields to the Content type, and arrange them on content display and input forms.');
  }
  if (module_exists('fieldgroup')) {
    $vars['help'] .= '<br>' . t('You can add a field to a group by dragging it below and to the right of the group.');
  }

  $order = _field_ui_overview_order($form, $form['#field_rows'], $form['#group_rows']);
  $rows = array();

  // Identify the 'new item' keys in the form, they look like
  // _add_new_field, add_new_group.
  $keys = array_keys($form);
  $add_rows = array();
  foreach ($keys as $key) {
    if (substr($key, 0, 4) == '_add') {
      $add_rows[] = $key;
    }
  }
  while ($order) {
    $key = reset($order);
    $element = &$form[$key];

    // Only display the 'Add' separator if the 'add' rows are still
    // at the end of the table.
    if (!isset($added_separator)) {
      $remaining_rows = array_diff($order, $add_rows);
      if (empty($remaining_rows) && empty($element['#depth'])) {
        $row = new stdClass();
        $row->row_type = 'separator';
        $row->class = 'tabledrag-leaf region';
        $rows[] = $row;
        $added_separator = TRUE;
      }
    }

    $row = new stdClass();

    // Add target classes for the tabledrag behavior.
    $element['weight']['#attributes']['class'] = 'field-weight';
    $element['parent']['#attributes']['class'] = 'group-parent';
    $element['hidden_name']['#attributes']['class'] = 'field-name';
    // Add target classes for the update selects behavior.
    switch ($element['#row_type']) {
      case 'add_new_field':
        $element['type']['#attributes']['class'] = 'field-type-select';
        $element['widget_type']['#attributes']['class'] = 'widget-type-select';
        break;
      case 'add_existing_field':
        $element['field_name']['#attributes']['class'] = 'field-select';
        $element['widget_type']['#attributes']['class'] = 'widget-type-select';
        $element['label']['#attributes']['class'] = 'label-textfield';
        break;
    }
    foreach (element_children($element) as $child) {
      $row->{$child} = drupal_render($element[$child]);
    }
    $row->label_class = 'label-' . strtr($element['#row_type'], '_', '-');
    $row->row_type = $element['#row_type'];
    $row->indentation = theme('indentation', isset($element['#depth']) ? $element['#depth'] : 0);
    $row->class = 'draggable';
    $row->class .= isset($element['#disabled_row']) ? ' menu-disabled' : '';
    $row->class .= isset($element['#add_new']) ? ' add-new' : '';
    $row->class .= isset($element['#leaf']) ? ' tabledrag-leaf' : '';
    $row->class .= isset($element['#root']) ? ' tabledrag-root' : '';

    $rows[] = $row;
    array_shift($order);
  }
  $vars['rows'] = $rows;
  $vars['submit'] = drupal_render_children($form);
}

/**
 * Validate handler for the field overview form.
 */
function field_ui_field_overview_form_validate($form, &$form_state) {
  _field_ui_field_overview_form_validate_add_new($form, $form_state);
  _field_ui_field_overview_form_validate_add_existing($form, $form_state);
}

/**
 * Helper function for field_ui_field_overview_form_validate.
 *
 * Validate the 'add new field' row.
 */
function _field_ui_field_overview_form_validate_add_new($form, &$form_state) {
  $field = $form_state['values']['_add_new_field'];

  // Validate if any information was provided in the 'add new field' row.
  if (array_filter(array($field['label'], $field['field_name'], $field['type'], $field['widget_type']))) {
    // No label.
    if (!$field['label']) {
      form_set_error('_add_new_field][label', t('Add new field: you need to provide a label.'));
    }

    // No field name.
    if (!$field['field_name']) {
      form_set_error('_add_new_field][field_name', t('Add new field: you need to provide a field name.'));
    }
    // Field name validation.
    else {
      $field_name = $field['field_name'];

      // Add the 'field_' prefix.
      if (substr($field_name, 0, 6) != 'field_') {
        $field_name = 'field_' . $field_name;
        form_set_value($form['_add_new_field']['field_name'], $field_name, $form_state);
      }

      // Invalid field name.
      if (!preg_match('!^field_[a-z0-9_]+$!', $field_name)) {
        form_set_error('_add_new_field][field_name', t('Add new field: the field name %field_name is invalid. The name must include only lowercase unaccentuated letters, numbers, and underscores.', array('%field_name' => $field_name)));
      }
      if (strlen($field_name) > 32) {
        form_set_error('_add_new_field][field_name', t("Add new field: the field name %field_name is too long. The name is limited to 32 characters, including the 'field_' prefix.", array('%field_name' => $field_name)));
      }

      // Field name already exists. We need to check inactive fields as well, so
      // we can't use field_info_fields().
      $fields = field_read_fields(array('field_name' => $field_name), array('include_inactive' => TRUE));
      if ($fields) {
        form_set_error('_add_new_field][field_name', t('Add new field: the field name %field_name already exists.', array('%field_name' => $field_name)));
      }
    }

    // No field type.
    if (!$field['type']) {
      form_set_error('_add_new_field][type', t('Add new field: you need to select a field type.'));
    }

    // No widget type.
    if (!$field['widget_type']) {
      form_set_error('_add_new_field][widget_type', t('Add new field: you need to select a widget.'));
    }
    // Wrong widget type.
    elseif ($field['type']) {
      $widget_types = field_ui_widget_type_options($field['type']);
      if (!isset($widget_types[$field['widget_type']])) {
        form_set_error('_add_new_field][widget_type', t('Add new field: invalid widget.'));
      }
    }
  }
}

/**
 * Helper function for field_ui_field_overview_form_validate.
 *
 * Validate the 'add existing field' row.
 */
function _field_ui_field_overview_form_validate_add_existing($form, &$form_state) {
  // The form element might be absent if no existing fields can be added to
  // this content type
  if (isset($form_state['values']['_add_existing_field'])) {
    $field = $form_state['values']['_add_existing_field'];

    // Validate if any information was provided in the 'add existing field' row.
    if (array_filter(array($field['label'], $field['field_name'], $field['widget_type']))) {
      // No label.
      if (!$field['label']) {
        form_set_error('_add_existing_field][label', t('Add existing field: you need to provide a label.'));
      }

      // No existing field.
      if (!$field['field_name']) {
        form_set_error('_add_existing_field][field_name', t('Add existing field: you need to select a field.'));
      }

      // No widget type.
      if (!$field['widget_type']) {
        form_set_error('_add_existing_field][widget_type', t('Add existing field: you need to select a widget.'));
      }
      // Wrong widget type.
      elseif ($field['field_name'] && ($existing_field = field_info_field($field['field_name']))) {
        $widget_types = field_ui_widget_type_options($existing_field['type']);
        if (!isset($widget_types[$field['widget_type']])) {
          form_set_error('_add_existing_field][widget_type', t('Add existing field: invalid widget.'));
        }
      }
    }
  }
}

/**
 * Submit handler for the field overview form.
 */
function field_ui_field_overview_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $bundle = $form['#bundle'];
  $admin_path = _field_ui_bundle_admin_path($bundle);

  // Update field weights.
  $extra = array();
  foreach ($form_values as $key => $values) {
    if (in_array($key, $form['#fields'])) {
      db_update('field_config_instance')
        ->fields(array('weight' => $values['weight']))
        ->condition('bundle', $bundle)
        ->condition('field_name', $key)
        ->execute();
    }
    elseif (in_array($key, $form['#extra'])) {
      $extra[$key] = $values['weight'];
    }
  }

  if ($extra) {
    variable_set("field_extra_weights_$bundle", $extra);
  }
  else {
    variable_del("field_extra_weights_$bundle");
  }

  $destinations = array();

  // Create new field.
  $field = array();
  if (!empty($form_values['_add_new_field']['field_name'])) {
    $values = $form_values['_add_new_field'];

    $field = array(
      'field_name' => $values['field_name'],
      'type' => $values['type'],
    );
    $instance = array(
      'field_name' => $field['field_name'],
      'bundle' => $bundle,
      'label' => $values['label'],
      'weight' => $values['weight'],
      'widget' => array(
        'type' => $values['widget_type'],
      ),
    );

    // Create the field and instance.
    try {
      field_create_field($field);
      field_create_instance($instance);

      // Rebuild the menu so we can navigate to the field settings screen.
      //field_clear_type_cache(TRUE);
      //menu_rebuild();
      $destinations[] = $admin_path . '/fields/refresh';
      $destinations[] = $admin_path . '/fields/' . $field['field_name'] . '/field-settings';
      $destinations[] = $admin_path . '/fields/' . $field['field_name'] . '/edit';

      // Store new field information for fieldgroup submit handler.
      $form_state['fields_added']['_add_new_field'] = $field['field_name'];
    }
    catch (Exception $e) {
      drupal_set_message(t('There was a problem creating field %label: @message.', array('%label' => $instance['label'], '@message' => $e->getMessage())));
    }
  }

  // Add existing field.
  if (!empty($form_values['_add_existing_field']['field_name'])) {
    $values = $form_values['_add_existing_field'];
    $field = field_info_field($values['field_name']);
    if (!empty($field['locked'])) {
      drupal_set_message(t('The field %label cannot be added because it is locked.', array('%label' => $values['label'])));
    }
    else {
      $instance = array(
        'field_name' => $field['field_name'],
        'bundle' => $bundle,
        'label' => $values['label'],
        'weight' => $values['weight'],
        'widget' => array(
          'type' => $values['widget_type'],
        ),
      );

      try {
        field_create_instance($instance);
        $destinations[] = $admin_path . '/fields/refresh';
        $destinations[] = $admin_path . '/fields/' . $instance['field_name'] . '/edit';
        // Store new field information for fieldgroup submit handler.
        $form_state['fields_added']['_add_existing_field'] = $instance['field_name'];
      }
      catch (Exception $e) {
        drupal_set_message(t('There was a problem creating field instance %label: @message.', array('%label' => $instance['label'], '@message' => $e->getMessage())));
      }
    }
  }

  if ($destinations) {
    $destinations[] = urldecode(substr(drupal_get_destination(), 12));
    unset($_REQUEST['destination']);
    $form_state['redirect'] = field_ui_get_destinations($destinations);
  }

  field_cache_clear();
}

/**
 * Menu callback; presents a listing of fields display settings for a content type.
 *
 * Form includes form widgets to select which fields appear for teaser, full node
 * and how the field labels should be rendered.
 */
function field_ui_display_overview_form(&$form_state, $obj_type, $bundle, $build_modes_selector = 'basic') {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  field_ui_inactive_message($bundle);
  $admin_path = _field_ui_bundle_admin_path($bundle);

  // Gather type information.
  $entity = field_info_bundle_entity($bundle);
  $instances = field_info_instances($bundle);
  $field_types = field_info_field_types();

  $groups = $group_options = array();
  if (module_exists('fieldgroup')) {
    $groups = fieldgroup_groups($bundle);
    $group_options = _fieldgroup_groups_label($bundle);
  }

  $build_modes = field_ui_build_modes_tabs($entity, $build_modes_selector);

  $form = array(
    '#tree' => TRUE,
    '#bundle' => $bundle,
    '#fields' => array_keys($instances),
    '#groups' => array_keys($groups),
    '#contexts' => $build_modes_selector,
  );

  if (empty($instances)) {
    drupal_set_message(t('There are no fields configured yet. You can add new fields on the <a href="@link">Manage fields</a> page.', array('@link' => url($admin_path . '/fields'))), 'warning');
    return $form;
  }

  // Fields.
  $label_options = array(
    'above' => t('Above'),
    'inline' => t('Inline'),
    'hidden' => t('<Hidden>'),
  );
  foreach ($instances as $name => $instance) {
    $field = field_info_field($instance['field_name']);
    $weight = $instance['weight'];

    $form[$name] = array(
      'human_name' => array(
        '#markup' => check_plain($instance['label'])
      ),
      'weight' => array(
        '#type' => 'value',
        '#value' => $weight
      ),
      'parent' => array(
        '#type' => 'value',
        '#value' => ''
      ),
    );
    $defaults = $instance['display'];

    $formatter_options = field_ui_formatter_options($field['type']);
    $formatter_options['hidden'] = t('<Hidden>');
    foreach ($build_modes as $build_mode => $label) {
      $display = isset($instance['display'][$build_mode]) ? $instance['display'][$build_mode] : $instance['display']['full'];
      $form[$name][$build_mode]['label'] = array(
        '#type' => 'select',
        '#options' => $label_options,
        '#default_value' => $display['label'],
      );
      $form[$name][$build_mode]['type'] = array(
        '#type' => 'select',
        '#options' => $formatter_options,
        '#default_value' => $display['type'],
      );
    }
  }

  // Groups.
  $label_options = array(
    'above' => t('Above'),
    'hidden' => t('<Hidden>'),
  );
  $options = array(
    'no_style' => t('no styling'),
    'simple' => t('simple'),
    'fieldset' => t('fieldset'),
    'fieldset_collapsible' => t('fieldset - collapsible'),
    'fieldset_collapsed' => t('fieldset - collapsed'),
    'hidden' => t('<Hidden>'),
  );
  foreach ($groups as $name => $group) {
    $defaults = $group['settings']['display'];
    $weight = $group['weight'];

    $form[$name] = array(
      'human_name' => array('#markup' => check_plain($group['label'])),
      'weight' => array('#type' => 'value', '#value' => $weight),
    );
    foreach ($build_modes as $build_mode => $label) {
      $form[$name][$build_mode]['label'] = array(
        '#type' => 'select',
        '#options' => $label_options,
        '#default_value' => isset($defaults[$build_mode]['label']) ? $defaults[$build_mode]['label'] : 'above',
      );
      $form[$name][$build_mode]['format'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => isset($defaults[$build_mode]['format']) ? $defaults[$build_mode]['format'] : 'fieldset',
      );
    }
    foreach ($group['fields'] as $field_name => $field) {
      $form[$field_name]['parent']['#value'] = $name;
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}


/**
 * Theme preprocess function for field_ui-display-overview-form.tpl.php.
 */
function template_preprocess_field_ui_display_overview_form(&$vars) {
  $form = &$vars['form'];

  $contexts_selector = $form['#contexts'];
  $vars['basic'] = $contexts_selector == 'basic';

  $vars['contexts'] = field_ui_build_modes_tabs(field_info_bundle_entity($form['#bundle']), $contexts_selector);

  // TODO: Abstract texts over bundles / entity types.
  switch ($form['#bundle']) {
    case 'user':
      $help = t('Configure how user fields and field labels should be displayed.');
      break;

    default:
      if ($contexts_selector == 'basic') {
        $help = t("Configure how this content type's fields and field labels should be displayed when it's viewed in teaser and full-page mode.");
      }
      else {
        $help = t("Configure how this content type's fields should be displayed when it's rendered in the following contexts.");
      }
  }

  $vars['help'] = $help;

  $order = _field_ui_overview_order($form, $form['#fields'], $form['#groups']);
  if (empty($order)) {
    $vars['rows'] = array();
    $vars['submit'] = '';
    return;
  }
  $rows = array();
  foreach ($order as $key) {
    $element = &$form[$key];
    $row = new stdClass();
    foreach (element_children($element) as $child) {
      if (array_key_exists('label', $element[$child])) {
        $row->{$child}->label = drupal_render($element[$child]['label']);
        $row->{$child}->type = drupal_render($element[$child]['type']);
      }
      else {
        $row->{$child} = drupal_render($element[$child]);
      }
    }
    $row->label_class = in_array($key, $form['#groups']) ? 'label-group' : 'label-field';
    $row->indentation = theme('indentation', isset($element['#depth']) ? $element['#depth'] : 0);
    $rows[] = $row;
  }

  $vars['rows'] = $rows;
  $vars['submit'] = drupal_render_children($form);
}

/**
 * Submit handler for the display overview form.
 */
function field_ui_display_overview_form_submit($form, &$form_state) {
  module_load_include('inc', 'field', 'includes/field.crud');
  $form_values = $form_state['values'];
  foreach ($form_values as $key => $values) {
    // Groups are handled in fieldgroup_display_overview_form_submit().
    if (in_array($key, $form['#fields'])) {
      $instance = field_info_instance($key, $form['#bundle']);
      unset($values['weight'], $values['parent']);
      $instance['display'] = array_merge($instance['display'], $values);
      field_update_instance($instance);
    }
  }
  drupal_set_message(t('Your settings have been saved.'));
}

/**
 * Return an array of field_type options.
 */
function field_ui_field_type_options() {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $options = array();
    $field_types = field_info_field_types();
    $field_type_options = array();
    foreach ($field_types as $name => $field_type) {
      // Skip field types which have no widget types.
      if (field_ui_widget_type_options($name)) {
        $options[$name] = $field_type['label'];
      }
    }
    asort($options);
  }
  return $options;
}

/**
 * Return an array of widget type options for a field type.
 *
 * If no field type is provided, returns a nested array of all widget types,
 * keyed by field type human name.
 */
function field_ui_widget_type_options($field_type = NULL, $by_label = FALSE) {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $options = array();
    $field_types = field_info_field_types();
    foreach (field_info_widget_types() as $name => $widget_type) {
      foreach ($widget_type['field types'] as $widget_field_type) {
        // Check that the field type exists.
        if (isset($field_types[$widget_field_type])) {
          $options[$widget_field_type][$name] = $widget_type['label'];
        }
      }
    }
  }

  if ($field_type) {
    return !empty($options[$field_type]) ? $options[$field_type] : array();
  }
  elseif ($by_label) {
    $field_types = field_info_field_types();
    $options_by_label = array();
    foreach ($options as $field_type => $widgets) {
      $options_by_label[$field_types[$field_type]['label']] = $widgets;
    }
    return $options_by_label;
  }
  else {
    return $options;
  }
}

/**
 * Return an array of formatter options for a field type.
 *
 * If no field type is provided, returns a nested array of all formatters, keyed
 * by field type.
 */
function field_ui_formatter_options($field_type = NULL) {
  $options = &drupal_static(__FUNCTION__);

  if (!isset($options)) {
    $field_types = field_info_field_types();
    $options = array();
    foreach (field_info_formatter_types() as $name => $formatter) {
      foreach ($formatter['field types'] as $formatter_field_type) {
        // Check that the field type exists.
        if (isset($field_types[$formatter_field_type])) {
          $options[$formatter_field_type][$name] = $formatter['label'];
        }
      }
    }
  }

  if ($field_type) {
    return !empty($options[$field_type]) ? $options[$field_type] : array();
  }
  else {
    return $options;
  }
}

/**
 * Return an array of existing field to be added to a bundle.
 */
function field_ui_existing_field_options($bundle) {
  $options = array();
  $field_types = field_info_field_types();
  foreach (field_info_instances() as $bundle_name => $instances) {
    // No need to look in the current bundle.
    if ($bundle_name != $bundle) {
      foreach ($instances as $instance) {
        $field = field_info_field($instance['field_name']);
        // Don't show locked fields or fields already in the current bundle.
        if (empty($field['locked']) && !field_info_instance($field['field_name'], $bundle)) {
          $text = t('@type: @field (@label)', array('@type' => $field_types[$field['type']]['label'], '@label' => t($instance['label']), '@field' => $instance['field_name']));
          $options[$instance['field_name']] = (drupal_strlen($text) > 80) ? truncate_utf8($text, 77) . '...' : $text;
        }
      }
    }
  }
  // Sort the list by type, then by field name, then by label.
  asort($options);
  return $options;
}

/**
 * Helper function to determine if a field has data in the database.
 */
function field_ui_field_has_data($field) {
  $results = field_attach_query($field['field_name'], array(), 1);
  return !empty($results);
}

/**
 * Menu callback; presents the field settings edit page.
 */
function field_ui_field_settings_form(&$form_state, $obj_type, $bundle, $field_name) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  $instance = field_info_field($field_name, $bundle);
  $field = field_info_field($field_name);

  // When a field is first created, we have to get data from the db.
  if (!isset($instance['label'])) {
    $instance = field_read_instance($field_name, $bundle);
    $field = field_read_field($field_name);
  }

  $field_type = field_info_field_types($field['type']);

  $info_function = $field['module'] . '_field_info';
  $info = $info_function();
  $description = '<p><strong>' . $info[$field['type']]['label'] . ':</strong> ';
  $description .= $info[$field['type']]['description'] . '</p>';
  $form['#prefix'] = '<div class="description">' . $description . '</div>';

  $description = '<p>' . t('These settings apply to the %field field everywhere it is used. These settings impact the way that data is stored in the database and cannot be changed once data has been created.', array('%field' => $instance['label'])) . '</p>';

  // Create a form structure for the field values.
  $form['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('%field field settings', array('%field' => $instance['label'])),
    '#description' => $description,
    '#tree' => TRUE,
  );

  // See if data already exists for this field.
  // If so, prevent changes to the field settings.
  $has_data = field_ui_field_has_data($field);
  if ($has_data) {
    $form['field']['#description'] = '<div class=error>' . t('There is data for this field in the database. The field settings can no longer be changed.' . '</div>') . $form['field']['#description'];
  }

  // Build the non-configurable field values.
  $form['field']['field_name'] = array('#type' => 'value', '#value' => $field_name);
  $form['field']['type'] = array('#type' => 'value', '#value' => $field['type']);
  $form['field']['module'] = array('#type' => 'value', '#value' => $field['module']);
  $form['field']['active'] = array('#type' => 'value', '#value' => $field['active']);

  // Add settings provided by the field module.
  $form['field']['settings'] = array();
  $additions = module_invoke($field_type['module'], 'field_settings_form', $field, $instance);
  if (is_array($additions)) {
    $form['field']['settings'] = $additions;
    // TODO: Filter this so only the settings that cannot be changed are shown
    // in this form. For now, treating all settings as changeable, which means
    // they show up here and in edit form.
  }
  if (empty($form['field']['settings'])) {
    $form['field']['settings'] = array(
      '#markup' => t('%field has no field settings.', array('%field' => $instance['label'])),
      );
  }
  else {
    foreach ($form['field']['settings'] as $key => $setting) {
      if (substr($key, 0, 1) != '#') {
        $form['field']['settings'][$key]['#disabled'] = $has_data;
      }
    }
  }

  $form['#bundle'] = $bundle;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save field settings'),
  );

  return $form;
}

/**
 * Save a field's settings after editing.
 */
function field_ui_field_settings_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $field_values = $form_values['field'];

  // Merge incoming form values into the existing field.
  $field = field_info_field($field_values['field_name']);

  // Don't allow changes to fields with data.
  if (field_ui_field_has_data($field)) {
    return;
  }

  // Remove the 'bundles' element added by field_info_field.
  // TODO: This is ugly, there should be a better way.
  unset($field['bundles']);
  $bundle = $form['#bundle'];
  $instance = field_info_instance($field['field_name'], $bundle);

  // Update the field.
  $field = array_merge($field, $field_values);
  field_ui_update_field($field);

  drupal_set_message(t('Updated field %label field settings.', array('%label' => $instance['label'])));
  $form_state['redirect'] = field_ui_next_destination($bundle);
}

/**
 * Menu callback; select a widget for the field.
 */
function field_ui_widget_type_form(&$form_state, $obj_type, $bundle, $field_name) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  $instance = field_read_instance($field_name, $bundle);
  $field = field_read_field($field_name);

  $field_type = field_info_field_types($field['type']);
  $widget_type = field_info_widget_types($instance['widget']['type']);
  $bundles = field_info_bundles();
  $bundle_label = $bundles[$bundle]['label'];

  $form = array();
  $form['basic'] = array(
    '#type' => 'fieldset',
    '#title' => t('Change widget'),
  );
  $form['basic']['widget_type'] = array(
    '#type' => 'select',
    '#title' => t('Widget type'),
    '#required' => TRUE,
    '#options' => field_ui_widget_type_options($field['type']),
    '#default_value' => $instance['widget']['type'],
    '#description' => t('The type of form element you would like to present to the user when creating this field in the %type type.', array('%type' => $bundle_label)),
  );

  $form['#instance'] = $instance;
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
  );

  $form['#validate'] = array();
  $form['#submit'] = array('field_ui_widget_type_form_submit');

  return $form;
}

/**
 * Submit the change in widget type.
 */
function field_ui_widget_type_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $instance = $form['#instance'];
  $bundle = $instance['bundle'];

  // Set the right module information.
  $widget_type = field_info_widget_types($form_values['widget_type']);
  $widget_module = $widget_type['module'];

  if (drupal_function_exists('field_update_instance')) {
    $instance['widget']['type'] = $form_values['widget_type'];
    $instance['widget']['module'] = $widget_module;
    try {
      field_update_instance($instance);
      drupal_set_message(t('Changed the widget for field %label.', array('%label' => $instance['label'])));
    }
    catch (FieldException $e) {
      drupal_set_message(t('There was a problem changing the widget for field %label.', array('%label' => $instance['label'])));
    }
  }
  else {
    drupal_set_message(t('There was a problem changing the widget for field %label.', array('%label' => $instance['label'])));
  }

  $form_state['redirect'] = field_ui_next_destination($bundle);
}

/**
 * Menu callback; present a form for removing a field from a content type.
 */
function field_ui_field_remove_form(&$form_state, $obj_type, $bundle, $field_name) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  $field = field_info_field($field_name);
  $instance = field_info_instance($field_name, $bundle);
  $admin_path = _field_ui_bundle_admin_path($bundle);

  $form = array();
  $form['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle,
  );
  $form['field_name'] = array(
    '#type' => 'value',
    '#value' => $field_name,
  );

  $output = confirm_form($form,
    t('Are you sure you want to remove the field %field?', array('%field' => $instance['label'])),
    $admin_path . '/fields',
    t('If you have any content left in this field, it will be lost. This action cannot be undone.'),
    t('Remove'), t('Cancel'),
    'confirm'
  );

  if ($field['locked']) {
    unset($output['actions']['submit']);
    $output['description']['#markup'] = t('This field is <strong>locked</strong> and cannot be removed.');
  }

  return $output;
}

/**
 * Remove a field from a content type.
 */
function field_ui_field_remove_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $field = field_info_field($form_values['field_name']);
  $instance = field_info_instance($form_values['field_name'], $form_values['bundle']);
  $bundles = field_info_bundles();
  $bundle = $form_values['bundle'];
  $bundle_label = $bundles[$bundle]['label'];

  if (!empty($bundle) && $field && !$field['locked'] && $form_values['confirm']) {
    field_delete_instance($field['field_name'], $bundle);
    // Delete the field if that was the last instance.
    if (count($field['bundles'] == 1)) {
      field_delete_field($field['field_name']);
    }
    drupal_set_message(t('The field %field has been removed from %type.', array('%field' => $instance['label'], '%type' => $bundle_label)));
  }
  else {
    drupal_set_message(t('There was a problem removing the %field from %type.', array('%field' => $instance['label'], '%type' => $bundle_label)));
  }
  $form_state['redirect'] = field_ui_next_destination($bundle);
}

/**
 * Menu callback; presents the field instance edit page.
 */
function field_ui_field_edit_form(&$form_state, $obj_type, $bundle, $field_name) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  $output = '';
  $instance = field_info_instance($field_name, $bundle);
  $field = field_info_field($field_name);
  $form = array();
  $form['#field'] = $field;

  if (!empty($field['locked'])) {
    $output = array();
    $output['locked'] = array(
      '#markup' => t('The field %field is locked and cannot be edited.', array('%field' => $instance['label'])),
    );
    return $output;
  }

  $field_type = field_info_field_types($field['type']);
  $widget_type = field_info_widget_types($instance['widget']['type']);
  $bundles = field_info_bundles();

  $title = isset($instance['label']) ? $instance['label'] : $instance['field_name'];
  drupal_set_title(check_plain($title));

  // Create a form structure for the instance values.
  $form['instance'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('%type settings', array('%type' => $bundles[$bundle]['label'])),
    '#description' => t('These settings apply only to the %field field when used in the %type type.', array('%field' => $instance['label'], '%type' => $bundles[$bundle]['label'])),
  );

  // Build the non-configurable instance values.
  $form['instance']['field_name'] = array('#type' => 'value', '#value' => $field_name);
  $form['instance']['bundle'] = array('#type' => 'value', '#value' => $bundle);
  $form['instance']['weight'] = array('#type' => 'value', '#value' => !empty($instance['weight']) ? $instance['weight'] : 0);

  // Build the configurable instance values.
  $form['instance']['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => !empty($instance['label']) ? $instance['label'] : $field_name,
    '#required' => TRUE,
    '#description' => t('The human-readable label for this field.'),
  );
  $form['instance']['required'] = array(
    '#type' => 'checkbox',
    '#title' => t('Required'),
    '#default_value' => !empty($instance['required']),
    '#description' => t('Check if a value must be provided.'),
  );

  $form['instance']['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Help text'),
    '#default_value' => !empty($instance['description']) ? $instance['description'] : '',
    '#rows' => 5,
    '#description' => t('Instructions to present to the user below this field on the editing form.<br />Allowed HTML tags: @tags', array('@tags' => _field_filter_xss_display_allowed_tags())),
    '#required' => FALSE,
  );

  // Build the widget component of the instance.
  $form['instance']['widget'] = array(
    'type' =>   array('#type' => 'value', '#value' => $instance['widget']['type']),
    'module' => array('#type' => 'value', '#value' => $widget_type['module']),
    'active' => array('#type' => 'value', '#value' => !empty($field['instance']['widget']['active']) ? 1 : 0),
  );

  // Add additional field instance settings from the field module.
  $additions = module_invoke($field['module'], 'field_instance_settings_form', $field, $instance);
  if (is_array($additions)) {
    $form['instance']['settings'] = $additions;
  }

  // Add additional widget settings from the widget module.
  $additions = module_invoke($widget_type['module'], 'field_widget_settings_form', $field, $instance);
  if (is_array($additions)) {
    $form['instance']['widget']['settings'] = $additions;
    $form['instance']['widget']['active']['#value'] = 1;
  }

  // Add handling for default value if not provided by any other module.
  if (field_behaviors_widget('default value', $instance) == FIELD_BEHAVIOR_DEFAULT && empty($instance['default_value_function'])) {
    // Store the original default value for use in programmed forms. The
    // '#default_value' property is used instead of '#value' so programmed
    // values can override whatever we set here.
    $form['instance']['settings']['default_value'] = array(
      '#type' => 'value',
      '#default_value' => $instance['settings']['default_value'],
    );

    // We can't tell at the time we build the form if this is a programmed form
    // or not, so we always end up adding the default value widget even if we
    // won't use it.
    field_ui_default_value_widget($field, $instance, $form, $form_state);
  }

  $info = field_info_field_types($field['type']);
  $description = '<p><strong>' . $info['label'] . ':</strong> ';
  $description .= $info['description'] . '</p>';
  $form['#prefix'] = '<div class="description">' . $description . '</div>';

  $description = '<p>' . t('These settings apply to the %field field everywhere it is used.', array('%field' => $instance['label'])) . '</p>';

  // Create a form structure for the field values.
  $form['field'] = array(
    '#type' => 'fieldset',
    '#title' => t('%field field settings', array('%field' => $instance['label'])),
    '#description' => $description,
    '#tree' => TRUE,
  );

  // Build the configurable field values.
  $description = t('Maximum number of values users can enter for this field.');
  if (field_behaviors_widget('multiple values', $instance) == FIELD_BEHAVIOR_DEFAULT) {
    $description .= '<br/>' . t("'Unlimited' will provide an 'Add more' button so the users can add as many values as they like.");
  }
  $form['field']['cardinality'] = array(
    '#type' => 'select',
    '#title' => t('Number of values'),
    '#options' => array(FIELD_CARDINALITY_UNLIMITED => t('Unlimited')) + drupal_map_assoc(range(1, 10)),
    '#default_value' => $field['cardinality'],
    '#description' => $description,
  );

  // Add additional field settings from the field module.
  $additions = module_invoke($field['module'], 'field_settings_form', $field, $instance);
  if (is_array($additions)) {
    $form['field']['settings'] = $additions;
    // TODO: Filter in only settings that can be changed.
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
  );
  return $form;
}

/**
 * Build default value fieldset.
 */
function field_ui_default_value_widget($field, $instance, &$form, &$form_state) {
  $form['instance']['settings']['default_value_widget'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default value'),
    '#collapsible' => FALSE,
    '#tree' => TRUE,
    '#description' => t('The default value for this field, used when creating new content.')
  );

  // Make sure the default value is not a required field.
  $instance['required'] = FALSE;
  $instance['label'] = t('Default value');
  $instance['description'] = '';
  $items = $instance['settings']['default_value'];
  // Set up form info that the default value widget will need.
  $form['#fields'] = array(
    $field['field_name'] => array(
      'field' => $field,
      'instance' => $instance,
    ),
  );
  drupal_function_exists('field_default_form');
  // TODO: Allow multiple values (requires more work on 'add more' JS handler).
  $widget_form = field_default_form(NULL, NULL, $field, $instance, $items, $form, $form_state, 0);
  $form['instance']['settings']['default_value_widget'] += $widget_form;
  $form['#fields'][$field['field_name']]['form_path'] = array('instance', 'default_value_widget', $field['field_name']);
}

/**
 * Validate a field's settings.
 */
function field_ui_field_edit_form_validate($form, &$form_state) {
  $form_values = $form_state['values'];
  $instance = $form_values['instance'];
  $field = field_info_field($instance['field_name']);
  $field_type = field_info_field_types($field['type']);
  $widget_type = field_info_widget_types($instance['widget']['type']);

  // Do no validation here. Assume field and widget modules are
  // handling their own validation of form settings.

  // If field.module is handling the default value,
  // validate the result using the field validation.
  if (field_behaviors_widget('default value', $instance) == FIELD_BEHAVIOR_DEFAULT) {

    // If this is a programmed form, get rid of the default value widget,
    // we have the default values already.
    if (!empty($form_state['programmed'])) {
      form_set_value(array('#parents' => array('instance', 'widget', 'settings', 'default_value_widget')), NULL, $form_state);
      return;
    }

    if (!empty($form_values['instance']['widget']['default_value_widget'])) {
      // Fields that handle their own multiple values may use an expected
      // value as the top-level key, so just pop off the top element.
      $key = array_shift(array_keys($form_values['instance']['widget']['settings']['default_value_widget']));
      $default_value = $form_values['instance']['widget']['settings']['default_value_widget'][$key];
      $is_code = FALSE;
      form_set_value(array('#parents' => array('instance', 'widget', 'settings', 'default_value')), $default_value, $form_state);
    }
    if (isset($default_value)) {
      // TODO: Update this code for D7.
      $node = array();
      $node[$field['field_name']] = $default_value;
      $field['required'] = FALSE;
      $field_function = $field_type['module'] . '_field';
      $errors_before = form_get_errors();

      // Widget now does its own validation, should be no need
      // to add anything for widget validation here.
      if (drupal_function_exists($field_function)) {
        $field_function('validate', $node, $field, $default_value, $form, NULL);
      }
      // The field validation routine won't set an error on the right field,
      // so set it here.
      $errors_after = form_get_errors();
      if (count($errors_after) > count($errors_before)) {
        form_set_error('default_value', t('The default value is invalid.'));
      }
    }
  }
}

/**
 * Save instance settings after editing.
 */
function field_ui_field_edit_form_submit($form, &$form_state) {
  $form_values = $form_state['values'];
  $instance = $form_values['instance'];

  // Update any field settings that have changed.
  $field = field_info_field($instance['field_name']);
  // Remove the 'bundles' element added by field_info_field.
  // TODO: This is ugly, there should be a better way.
  unset($field['bundles']);
  $field = array_merge($field, $form_state['values']['field']);
  field_ui_update_field($field);

  // Move the default value from the sample widget to the default value field.
  if (isset($instance['settings']['default_value_widget'])) {
    $instance['settings']['default_value'] = $instance['settings']['default_value_widget'][$instance['field_name']];
    unset($instance['settings']['default_value_widget']);
  }

  // TODO: Move this to text.module and number.module?
  // Make sure the allowed values fieldset does not get stored.
  if (isset($instance['settings']['allowed_values_fieldset'])) {
    $instance['settings'] += $instance['settings']['allowed_values_fieldset'];
    unset($instance['settings']['allowed_values_fieldset']);
  }

  // Update the instance settings.
  module_load_include('inc', 'field', 'includes/field.crud');
  field_update_instance($instance);

  drupal_set_message(t('Saved %label configuration.', array('%label' => $instance['label'])));

  $form_state['redirect'] = field_ui_next_destination($instance['bundle']);
}

/**
 * Helper functions to handle multipage redirects.
 */
function field_ui_get_destinations($destinations) {
  $query = array();
  $path = array_shift($destinations);
  if ($destinations) {
    $query['destinations'] = $destinations;
  }
  return array($path, $query);
}

/**
 * Return the next redirect path in a multipage sequence.
 */
function field_ui_next_destination($bundle) {
  $destinations = !empty($_REQUEST['destinations']) ? $_REQUEST['destinations'] : array();
  if (!empty($destinations)) {
    unset($_REQUEST['destinations']);
    return field_ui_get_destinations($destinations);
  }
  else {
    $admin_path = _field_ui_bundle_admin_path($bundle);
    return $admin_path . '/fields';
  }
}

/**
 * Menu callback; rebuild the menu after adding new fields.
 *
 * Dummy function to force a page refresh so menu_rebuild() will work right
 * when creating a new field that creates a new menu item.
 */
function field_ui_field_menu_refresh($obj_type, $bundle) {
  $bundle = field_attach_extract_bundle($obj_type, $bundle);

  menu_rebuild();
  $destinations = field_ui_next_destination($bundle);
  if (is_array($destinations)) {
    $path = array_shift($destinations);
    drupal_goto($path, $destinations);
  }
  else {
    drupal_goto($destinations);
  }
}

/**
 * Helper function to order fields and groups when theming overview forms.
 *
 * The $form is passed by reference because we assign depths as parenting
 * relationships are sorted out.
 */
function _field_ui_overview_order(&$form, $field_rows, $group_rows) {
  // Put weight and parenting values into a $dummy render structure
  // and let drupal_render figure out the corresponding row order.
  $dummy = array();
  // Group rows: account for weight.
  if (module_exists('fieldgroup')) {
    foreach ($group_rows as $name) {
      $dummy[$name] = array('#markup' => $name . ' ', '#weight' => $form[$name]['weight']['#value']);
    }
  }
  // Field rows : account for weight and parenting.
  foreach ($field_rows as $name) {
    $dummy[$name] = array('#markup' => $name . ' ', '#type' => 'markup', '#weight' => $form[$name]['weight']['#value']);
    if (module_exists('fieldgroup')) {
      if ($parent = $form[$name]['parent']['#value']) {
        $form[$name]['#depth'] = 1;
        $dummy[$parent][$name] = $dummy[$name];
        unset($dummy[$name]);
      }
    }
  }
  return $dummy ? explode(' ', trim(drupal_render($dummy))) : array();
}

/**
 * Helper form element validator: integer.
 */
function _element_validate_integer($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value)) {
    form_error($element, t('%name must be an integer.', array('%name' => $element['#title'])));
  }
}

/**
 * Helper form element validator: integer > 0.
 */
function _element_validate_integer_positive($element, &$form_state) {
  $value = $element['#value'];
  if ($value !== '' && (!is_numeric($value) || intval($value) != $value || $value <= 0)) {
    form_error($element, t('%name must be a positive integer.', array('%name' => $element['#title'])));
  }
}

/**
 * Helper form element validator: number.
 */
function _element_validate_number($element, &$form_state) {
  $value = $element['#value'];
  if ($value != '' && !is_numeric($value)) {
    form_error($element, t('%name must be a number.', array('%name' => $element['#title'])));
  }
}

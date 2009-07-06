<?php
// $Id: theme.inc,v 1.8 2009/06/25 00:30:37 yched Exp $

/**
 * @file
 * Theme preprocess function for cck-admin-field-overview-form.tpl.php.
 */
function template_preprocess_cck_field_overview_form(&$vars) {
  $form = &$vars['form'];

  // TODO D7: abstract texts over bundles / entity types
  switch ($form['#bundle']) {
    case 'user':
      $vars['help'] = theme('advanced_help_topic', 'cck', 'manage-user-fields') . t('Add fields to the user, and arrange them on user display and input forms.');
      break;
    default:
      $vars['help'] = theme('advanced_help_topic', 'cck', 'manage-fields') . t('Add fields to the Content type, and arrange them on content display and input forms.');
  }
  if (module_exists('fieldgroup')) {
    $vars['help'] .= '<br>'. t('You can add a field to a group by dragging it below and to the right of the group.');
  }
  if (!module_exists('advanced_help')) {
    // TODO D7
    //$vars['help'] .= '<br/>' . t('Note: Installing the <a href="!adv_help">Advanced help</a> module will let you access more and better help.', array('!adv_help' => 'http://drupal.org/project/advanced_help'));
  }

  $order = _cck_overview_order($form, $form['#field_rows'], $form['#group_rows']);
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
        $element['type']['#attributes']['class'] = 'cck-field-type-select';
        $element['widget_type']['#attributes']['class'] = 'cck-widget-type-select';
        break;
      case 'add_existing_field':
        $element['field_name']['#attributes']['class'] = 'cck-field-select';
        $element['widget_type']['#attributes']['class'] = 'cck-widget-type-select';
        $element['label']['#attributes']['class'] = 'cck-label-textfield';
        break;
    }
    foreach (element_children($element) as $child) {
      $row->{$child} = drupal_render($element[$child]);
    }
    $row->label_class = 'label-'. strtr($element['#row_type'], '_', '-');
    $row->row_type = $element['#row_type'];
    $row->indentation = theme('indentation', isset($element['#depth']) ? $element['#depth'] : 0);
    $row->class = 'draggable';
    $row->class .= isset($element['#disabled_row']) ? ' menu-disabled' : '';
    $row->class .= isset($element['#add_new']) ? ' cck-add-new' : '';
    $row->class .= isset($element['#leaf']) ? ' tabledrag-leaf' : '';
    $row->class .= isset($element['#root']) ? ' tabledrag-root' : '';

    $rows[] = $row;
    array_shift($order);
  }
  $vars['rows'] = $rows;
  $vars['submit'] = drupal_render_children($form);

  // Add tabledrag behavior.
  drupal_add_tabledrag('cck-field-overview', 'match', 'parent', 'group-parent', 'group-parent', 'field-name', TRUE, 1);
  drupal_add_tabledrag('cck-field-overview', 'order', 'sibling', 'field-weight');

  // Add settings for the update selects behavior.
  // TODO D7 : adapt...
  $js_fields = array();
  foreach (cck_existing_field_options($form['#bundle']) as $field_name => $fields) {
    $field = field_info_field($field_name);
    $instance = field_info_instance($field_name, $form['#bundle']);
    $js_fields[$field_name] = array('label' => $instance['label'], 'type' => $field['type'], 'widget' => $instance['widget']['type']);
  }
  drupal_add_js(array('cckWidgetTypes' => cck_widget_type_options(), 'cckFields' => $js_fields), 'setting');
  drupal_add_js(drupal_get_path('module', 'cck') .'/cck.js');
}

/**
 * Theme preprocess function for cck-admin-display-overview-form.tpl.php.
 */
function template_preprocess_cck_display_overview_form(&$vars) {
  $form = &$vars['form'];

  $contexts_selector = $form['#contexts'];
  $vars['basic'] = $contexts_selector == 'basic';

  $vars['contexts'] = cck_build_modes(field_info_bundle_entity($form['#bundle']), $contexts_selector);

  // TODO D7: abstract texts over bundles / entity types
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

  $order = _cck_overview_order($form, $form['#fields'], $form['#groups']);
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
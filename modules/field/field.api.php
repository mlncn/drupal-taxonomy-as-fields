<?php
// $Id: field.api.php,v 1.2 2009/02/10 03:16:14 webchick Exp $

/**
 * @ingroup field_fieldable_type
 * @{
 */

/**
 * Inform the Field API about one or more fieldable types (object
 * types to which fields can be attached).
 *
 * @return
 *   An array whose keys are fieldable object type names and
 *   whose values identify properties of those types that the Field
 *   system needs to know about:
 *
 *   name: The human-readable name of the type.
 *   id key: The object property that contains the primary id for the
 *     object. Every object passed to the Field API must
 *     have this property and its value must be numeric.
 *   revision key: The object property that contains the revision id
 *     for the object, or NULL if the object type is not
 *     versioned. The Field API assumes that all revision ids are
 *     unique across all instances of a type; this means, for example,
 *     that every object's revision ids cannot be 0, 1, 2, ...
 *   bundle key: The object property that contains the bundle name for
 *     the object (bundle name is what nodes call "content type").
 *     The bundle name defines which fields are connected to the object.
 *   cacheable: A boolean indicating whether Field API should cache
 *     loaded fields for each object, reducing the cost of
 *     field_attach_load().
 *   bundles: An array of all existing bundle names for this object
 *     type. TODO: Define format. TODO: I'm unclear why we need
 *     this.
 */
function hook_fieldable_info() {
  $return = array(
    'node' => array(
      'name' => t('Node'),
      'id key' => 'nid',
      'revision key' => 'vid',
      'bundle key' => 'type',
      // Node.module handles its own caching.
      'cacheable' => FALSE,
      // Bundles must provide human readable name so
      // we can create help and error messages about them.
      'bundles' => node_get_types('names'),
    ),
  );
  return $return;
}

/**
 * @} End of "ingroup field_fieldable_type"
 */

/**
 * @defgroup field_types Field Types API
 * @{
 * Define field types, widget types, and display formatter types.
 *
 * The bulk of the Field Types API are related to field types. A
 * field type represents a particular data storage type (integer,
 * string, date, etc.) that can be attached to a fieldable object.
 * hook_field_info() defines the basic properties of a field type, and
 * a variety of other field hooks are called by the Field Attach API
 * to perform field-type-specific actions.
 *
 * The Field Types API also defines widget types via
 * hook_field_widget_info(). Widgets are Form API elements with
 * additional processing capabilities. A field module can define
 * widgets that work with its own field types or with any other
 * module's field types. Widget hooks are typically called by the
 * Field Attach API when creating the field form elements during
 * field_attach_form().
 *
 * TODO Display formatters.
 */

/**
 * Define Field API field types.
 *
 * @return
 *   An array whose keys are field type names and whose values are:
 *
 *   label: TODO
 *   description: TODO
 *   settings: TODO
 *   instance_settings: TODO
 *   default_widget: TODO
 *   default_formatter: TODO
 *   behaviors: TODO
 */
function hook_field_info() {
  return array(
    'text' => array(
      'label' => t('Text'),
      'description' => t('This field stores varchar text in the database.'),
      'settings' => array('max_length' => 255),
      'instance_settings' => array('text_processing' => 0),
      'default_widget' => 'text_textfield',
      'default_formatter' => 'text_default',
    ),
    'textarea' => array(
      'label' => t('Textarea'),
      'description' => t('This field stores long text in the database.'),
      'instance_settings' => array('text_processing' => 0),
      'default_widget' => 'text_textarea',
      'default_formatter' => 'text_default',
    ),
  );
}

/**
 * Define the Field API schema for a field structure.
 *
 * @param $field
 *   A field structure.
 * @return
 *   A Field API schema is an array of Schema API column
 *   specifications, keyed by field-independent column name. For
 *   example, a field may declare a column named 'value'. The SQL
 *   storage engine may create a table with a column named
 *   <fieldname>_value_0, but the Field API schema column name is
 *   still 'value'.
 */
function hook_field_columns($field) {
  if ($field['type'] == 'textarea') {
    $columns = array(
      'value' => array(
        'type' => 'text',
        'size' => 'big',
        'not null' => FALSE,
      ),
    );
  }
  else {
    $columns = array(
      'value' => array(
        'type' => 'varchar',
        'length' => $field['settings']['max_length'],
        'not null' => FALSE,
      ),
    );
  }
  $columns += array(
    'format' => array(
      'type' => 'int',
      'unsigned' => TRUE,
      'not null' => FALSE,
    ),
  );
  return $columns;
}

/**
 * Define Field API widget types.
 *
 * @return
 *   An array whose keys are field type names and whose values are:
 *
 *   label: TODO
 *   description: TODO
 *   field types: TODO
 *   settings: TODO
 *   behaviors: TODO
 */
function hook_field_widget_info() {
}

/*
 * Define Field API formatter types.
 *
 * @return
 *   An array whose keys are field type names and whose values are:
 *
 *   label: TODO
 *   description: TODO
 *   field types: TODO
 *   behaviors: TODO
 */
function hook_field_formatter_info() {
}

/**
 * Define custom load behavior for this module's field types.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_load($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom validate behavior for this module's field types.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 * @param $form
 *   The form structure being validated. NOTE: This parameter will
 *   become obsolete (see field_attach_validate()).
 */
function hook_field_validate($obj_type, $object, $field, $instance, $items, $form) {
}

/**
 * Define custom presave behavior for this module's field types.
 * TODO: The behavior of this hook is going to change (see
 * field_attach_presave()).
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_presave($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom insert behavior for this module's field types.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_insert($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom update behavior for this module's field types.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_update($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom delete behavior for this module's field types. This
 * hook is invoked just before the data is deleted from field storage.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_delete($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom delete_revision behavior for this module's field
 * types. This hook is invoked just before the data is deleted from
 * field storage, and will only be called for fieldable types that are
 * versioned.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_delete_revision($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom sanitize behavior for this module's field types.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_sanitize($obj_type, $object, $field, $instance, $items) {
}

/**
 * Define custom prepare_translation behavior for this module's field
 * types. TODO: This hook may or may not survive in Field API.
 *
 * @param $obj_type
 *   The type of $object.
 * @param $object
 *   The object for the operation.
 * @param $field
 *   The field structure for the operation.
 * @param $instance
 *   The instance structure for $field on $object's bundle.
 * @param $items
 *   $object->{$field['field_name']}, or an empty array if unset.
 */
function hook_field_prepare_translation($obj_type, $object, $field, $instance, $items) {
}

/**
 * Return a single form element for a form.
 *
 * It will be built out and validated in the callback(s) listed in
 * hook_elements. We build it out in the callbacks rather than in
 * hook_field_widget so it can be plugged into any module that can
 * provide it with valid $field information.
 *
 * Field API will set the weight, field name and delta values for each
 * form element. If there are multiple values for this field, the
 * Field API will call this function as many times as needed.
 *
 * @param $form
 *   The entire form array, $form['#node'] holds node information.
 *   TODO: Not #node any more.
 * @param $form_state
 *   The form_state, $form_state['values'][$field['field_name']]
 *   holds the field's form values.
 * @param $field
 *   The field structure.
 * @param $instance
 *   The field instance.
 * @param $items
 *   Array of default values for this field.
 * @param $delta
 *   The order of this item in the array of subelements (0, 1, 2, etc).
 * @return
 *   The form item for a single element for this field.
 */
function hook_field_widget(&$form, &$form_state, $field, $instance, $items, $delta = 0) {
  $element = array(
    '#type' => $instance['widget']['type'],
    '#default_value' => isset($items[$delta]) ? $items[$delta] : '',
  );
  return $element;
}

/**
 * @} End of "ingroup field_type"
 */

/**
 * @ingroup field_attach
 * @{
 */

/**
 * Act on field_attach_form. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_form() for details and arguments.
 */
function hook_field_attach_form($obj_type, $object, &$form, &$form_state) {
}

/**
 * Act on field_attach_load. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_load() for details and arguments. TODO:
 * Currently, this hook only accepts a single object a time.
 */
function hook_field_attach_load($obj_type, $object) {
}

/**
 * Act on field_attach_validate. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_validate() for details and arguments.
 */
function hook_field_attach_validate($obj_type, $object, &$form) {
}

/**
 * Act on field_attach_submit. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_submit() for details and arguments.
 */
function hook_field_attach_submit($obj_type, $object, $form, &$form_state) {
}

/**
 * Act on field_attach_presave. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_presave() for details and arguments.
 */
function hook_field_attach_presave($obj_type, $object) {
}

/**
 * Act on field_attach_insert. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_insert() for details and arguments.
 */
function hook_field_attach_insert($obj_type, $object) {
}

/**
 * Act on field_attach_update. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_update() for details and arguments.
 */
function hook_field_attach_update($obj_type, $object) {
}

/**
 * Act on field_attach_delete. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_delete() for details and arguments.
 */
function hook_field_attach_delete($obj_type, $object) {
}

/**
 * Act on field_attach_delete_revision. This hook is invoked after
 * the field module has performed the operation.
 *
 * See field_attach_delete_revision() for details and arguments.
 */
function hook_field_attach_delete_revision($obj_type, $object) {
}

/**
 * Act on field_attach_view. This hook is invoked after the field module
 * has performed the operation.
 *
 * @param $output
 *  The structured content array tree for all of $object's fields.
 * @param $obj_type
 *   The type of $object; e.g. 'node' or 'user'.
 * @param $object
 *   The object with fields to render.
 * @param $teaser
 *   Whether to display the teaser only, as on the main page.
 */
function hook_field_attach_view($output, $obj_type, $object, $teaser) {
}

/**
 * Act on field_attach_create_bundle. This hook is invoked after the
 * field module has performed the operation.
 *
 * See field_attach_create_bundle() for details and arguments.
 */
function hook_field_attach_create_bundle($bundle) {
}

/**
 * Act on field_attach_rename_bundle. This hook is invoked after the
 * field module has performed the operation.
 *
 * See field_attach_rename_bundle() for details and arguments.
 */
function hook_field_rename_bundle($bundle_old, $bundle_new) {
}

/**
 * Act on field_attach_delete_bundle. This hook is invoked after the field module
 * has performed the operation.
 *
 * See field_attach_delete_bundle() for details and arguments.
 */
function hook_field_attach_delete_bundle($bundle) {
}

/**
 * @} End of "ingroup field_attach"
 */

/**********************************************************************
 * Field Storage API
 **********************************************************************/

/**
 * @ingroup field_storage
 * @{
 */

/**
 * Load field data for a set of objects.
 *
 * @param $obj_type
 *   The entity type of objects being loaded, such as 'node' or
 *   'user'.
 * @param $objects
 *   The array of objects for which to load data.
 * @param $age
 *   FIELD_LOAD_CURRENT to load the most recent revision for all
 *   fields, or FIELD_LOAD_REVISION to load the version indicated by
 *   each object.
 * @return
 *   An array of field data for the objects, keyed by entity id, field
 *   name, and item delta number.
 */
function hook_field_storage_load($obj_type, $queried_objs, $age) {
}

/**
 * Write field data for an object.
 *
 * @param $obj_type
 *   The entity type of object, such as 'node' or 'user'.
 * @param $object
 *   The object on which to operate.
 * @param $update
 *   TRUE if this is an update to an existing object, FALSE if it is
 *   an insert of a new object.
 */
function hook_field_storage_write($obj_type, $object, $update = TRUE) {
}

/**
 * Delete all field data for an object.
 *
 * @param $obj_type
 *   The entity type of object, such as 'node' or 'user'.
 * @param $object
 *   The object on which to operate.
 */
function hook_field_storage_delete($obj_type, $object) {
}

/**
 * Delete a single revision of field data for an object.
 *
 * @param $obj_type
 *   The entity type of object, such as 'node' or 'user'.
 * @param $object
 *   The object on which to operate. The revision to delete is
 *   indicated by the object's revision id property, as identified by
 *   hook_fieldable_info() for $obj_type.
 */
function hook_field_storage_delete_revision($obj_type, $object) {
}

/**
 * Act on creation of a new bundle.
 *
 * @param $bundle
 *   The name of the bundle being created.
 */
function hook_field_storage_create_bundle($bundle) {
}

/**
 * Act on a bundle being renamed.
 *
 * @param $bundle_old
 *   The old name of the bundle.
 * @param $bundle_new
 *   The new name of the bundle.
 */
function hook_field_storage_rename_bundle($bundle_old, $bundle_new) {
}

/**
 * Act on creation of a new field.
 *
 * @param $field
 *   The field structure being created.
 */
function hook_field_storage_create_field($field) {
}

/**
 * Act on deletion of a field.
 *
 * @param $field_name
 *   The name of the field being deleted.
 */
function hook_field_storage_delete_field($field_name) {
}

/**
 * Act on deletion of a field instance.
 *
 * @param $field_name
 *   The name of the field in the new instance.
 * @param $bundle
 *   The name of the bundle in the new instance.
 */
function hook_field_storage_delete_instance($field_name, $bundle) {
}

/**
 * @} End of "ingroup field_storage"
 */

/**********************************************************************
 * Field CRUD API
 **********************************************************************/

/**
 * @ingroup field_crud
 * @{
 */

/**
 * Act on a field being created. This hook is invoked after the field
 * is created and so it cannot modify the field itself.
 *
 * TODO: Not implemented.
 *
 * @param $field
 *   The field just created.
 */
function hook_field_create_field($field) {
}

/**
 * Act on a field instance being created. This hook is invoked after
 * the instance record is saved and so it cannot modify the instance
 * itself.
 *
 * @param $instance
 *   The instance just created.
 */
function hook_field_create_instance($instance) {
}

/**
 * Act on a field being deleted. This hook is invoked just before the
 * field is deleted.
 *
 * TODO: Not implemented.
 *
 * @param $field
 *   The field being deleted.
 */
function hook_field_delete_field($field) {
}


/**
 * Act on a field instance being updated. This hook is invoked after
 * the instance record is saved and so it cannot modify the instance
 * itself.
 *
 * TODO: Not implemented.
 *
 * @param $instance
 *   The instance just updated.
 */
function hook_field_update_instance($instance) {
}

/**
 * Act on a field instance being deleted. This hook is invoked just
 * before the instance is deleted.
 *
 * TODO: Not implemented.
 *
 * @param $instance
 *   The instance just updated.
 */
function hook_field_delete_instance($instance) {
}

/**
 * Act on field records being read from the database.
 *
 * @param $field
 *   The field record just read from the database.
 */
function hook_field_read_field($field) {
}

/**
 * Act on a field record being read from the database.
 *
 * @param $instance
 *   The instance record just read from the database.
 */
function hook_field_read_instance($instance) {
}

/**
 * @} End of "ingroup field_crud"
 */

/**********************************************************************
 * TODO: I'm not sure where these belong yet.
 **********************************************************************/

/**
 * TODO
 *
 * Note : Right now this belongs to the "Fieldable Type API".
 * Whether 'build modes' is actually a 'fields' concept is to be debated
 * in a separate overhaul patch for core.
 */
function hook_field_build_modes($obj_type) {
}

/**
 * Determine whether the user has access to a given field.
 *
 * @param $op
 *   The operation to be performed. Possible values:
 *   - "edit"
 *   - "view"
 * @param $field
 *   The field on which the operation is to be performed.
 * @param $account
 *   (optional) The account to check, if not given use currently logged in user.
 * @return
 *   TRUE if the operation is allowed;
 *   FALSE if the operation is denied.
 */
function hook_field_access($op, $field, $account) {
}

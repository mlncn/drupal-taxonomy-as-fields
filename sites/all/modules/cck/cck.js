// $Id: cck.js,v 1.6 2009/03/04 23:13:16 karens Exp $
(function($) {

Drupal.behaviors.cckManageFields = {
  attach: function(context) {
    attachUpdateSelects(context);
  }
};

function attachUpdateSelects(context) {
  var widgetTypes = Drupal.settings.cckWidgetTypes;
  var fields = Drupal.settings.cckFields;

  // Store the default text of widget selects.
  $('#cck-field-overview .cck-widget-type-select', context).each(function() {
    this.initialValue = this.options[0].text;
  });

  // 'Field type' select updates its 'Widget' select.
  $('#cck-field-overview .cck-field-type-select', context).each(function() {
    this.targetSelect = $('.cck-widget-type-select', $(this).parents('tr').eq(0));

    $(this).change(function() {
      var selectedFieldType = this.options[this.selectedIndex].value;
      var options = (selectedFieldType in widgetTypes) ? widgetTypes[selectedFieldType] : [ ];
      this.targetSelect.cckPopulateOptions(options);
    });

    // Trigger change on initial pageload to get the right widget options
    // when field type comes pre-selected (on failed validation).
    $(this).trigger('change');
  });

  // 'Existing field' select updates its 'Widget' select and 'Label' textfield.
  $('#cck-field-overview .cck-field-select', context).each(function() {
    this.targetSelect = $('.cck-widget-type-select', $(this).parents('tr').eq(0));
    this.targetTextfield = $('.cck-label-textfield', $(this).parents('tr').eq(0));

    $(this).change(function(e, updateText) {
      var updateText = (typeof(updateText) == 'undefined') ? true : updateText;
      var selectedField = this.options[this.selectedIndex].value;
      var selectedFieldType = (selectedField in fields) ? fields[selectedField].type : null;
      var selectedFieldWidget = (selectedField in fields) ? fields[selectedField].widget : null
      var options = (selectedFieldType && (selectedFieldType in widgetTypes)) ? widgetTypes[selectedFieldType] : [ ];
      this.targetSelect.cckPopulateOptions(options, selectedFieldWidget);

      if (updateText) {
        $(this.targetTextfield).attr('value', (selectedField in fields) ? fields[selectedField].label : '');
      }
    });

    // Trigger change on initial pageload to get the right widget options
    // and label when field type comes pre-selected (on failed validation).
    $(this).trigger('change', false);
  });
}

jQuery.fn.cckPopulateOptions = function(options, selected) {
  return this.each(function() {
    var disabled = false;
    if (options.length == 0) {
      options = [this.initialValue];
      disabled = true;
    }

    // If possible, keep the same widget selected when changing field type.
    // This is based on textual value, since the internal value might be
    // different (options_buttons vs. nodereference_buttons).
    var previousSelectedText = this.options[this.selectedIndex].text;

    var html = '';
    jQuery.each(options, function(value, text) {
      // Figure out which value should be selected. The 'selected' param
      // takes precedence.
      var is_selected = ((typeof(selected) !== 'undefined' && value == selected) || (typeof(selected) == 'undefined' && text == previousSelectedText));
      html += '<option value="' + value + '"' + (is_selected ? ' selected="selected"' : '') +'>' + text + '</option>';
    });

    $(this)
      .html(html)
      .attr('disabled', disabled ? 'disabled' : '');
  });
}
})(jQuery);

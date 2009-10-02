// $Id: system.js,v 1.35 2009/09/09 21:53:15 dries Exp $
(function ($) {

/**
 * Show/hide the 'Email site administrator when updates are available' checkbox
 * on the install page.
 */
Drupal.hideEmailAdministratorCheckbox = function () {
  // Make sure the secondary box is shown / hidden as necessary on page load.
  if ($('#edit-update-status-module-1').is(':checked')) {
    $('.form-item-update-status-module-2').show();
  }
  else {
    $('.form-item-update-status-module-2').hide();
  }

  // Toggle the display as necessary when the checkbox is clicked.
  $('#edit-update-status-module-1').change( function () {
    $('.form-item-update-status-module-2').toggle();
  })
};

/**
 * Internal function to check using Ajax if clean URLs can be enabled on the
 * settings page.
 *
 * This function is not used to verify whether or not clean URLs
 * are currently enabled.
 */
Drupal.behaviors.cleanURLsSettingsCheck = {
  attach: function (context, settings) {
    // This behavior attaches by ID, so is only valid once on a page.
    // Also skip if we are on an install page, as Drupal.cleanURLsInstallCheck will handle
    // the processing.
    if (!($('#edit-clean-url').length) || $('#edit-clean-url.install').once('clean-url').length) {
      return;
    }
    var url = settings.basePath + 'admin/config/search/clean-urls/check';
    $.ajax({
      url: location.protocol + '//' + location.host + url,
      dataType: 'json',
      success: function () {
        // Check was successful. Redirect using a "clean URL". This will force the form that allows enabling clean URLs.
        location = settings.basePath +"admin/config/search/clean-urls";
      }
    });
  }
};

/**
 * Internal function to check using Ajax if clean URLs can be enabled on the
 * install page.
 *
 * This function is not used to verify whether or not clean URLs
 * are currently enabled.
 */
Drupal.cleanURLsInstallCheck = function () {
  var url = location.protocol + '//' + location.host + Drupal.settings.basePath + 'admin/config/search/clean-urls/check';
  // Submit a synchronous request to avoid database errors associated with
  // concurrent requests during install.
  $.ajax({
    async: false,
    url: url,
    dataType: 'json',
    success: function () {
      // Check was successful.
      $('#edit-clean-url').attr('value', 1);
    }
  });
};

/**
 * When a field is filled out, apply its value to other fields that will likely
 * use the same value. In the installer this is used to populate the
 * administrator e-mail address with the same value as the site e-mail address.
 */
Drupal.behaviors.copyFieldValue = {
  attach: function (context, settings) {
    for (var sourceId in settings.copyFieldValue) {
      $('#' + sourceId, context).once('copy-field-values').bind('blur', function () {
        // Get the list of target fields.
        var targetIds = settings.copyFieldValue[sourceId];
        // Add the behavior to update target fields on blur of the primary field.
        for (var delta in targetIds) {
          var targetField = $('#' + targetIds[delta]);
          if (targetField.val() == '') {
            targetField.val(this.value);
          }
        }
      });
    }
  }
};

/**
 * Show/hide custom format sections on the regional settings page.
 */
Drupal.behaviors.dateTime = {
  attach: function (context, settings) {
    // Show/hide custom format depending on the select's value.
    $('select.date-format', context).once('date-time').change(function () {
      $(this).parents('div.date-container').children('div.custom-container')[$(this).val() == 'custom' ? 'show' : 'hide']();
    });

    // Attach keyup handler to custom format inputs.
    $('input.custom-format', context).once('date-time').keyup(function () {
      var input = $(this);
      var url = settings.dateTime.lookup + (settings.dateTime.lookup.match(/\?q=/) ? '&format=' : '?format=') + encodeURIComponent(input.val());
      $.getJSON(url, function (data) {
        $('div.description span', input.parent()).html(data);
      });
    });

    // Trigger the event handler to show the form input if necessary.
    $('select.date-format', context).trigger('change');
  }
};

/**
 * Show/hide settings for user configurable time zones depending on whether
 * users are able to set their own time zones or not.
 */
Drupal.behaviors.userTimeZones = {
  attach: function (context, settings) {
    $('#empty-timezone-message-wrapper .description').hide();
    $('#edit-configurable-timezones', context).change(function () {
      $('#empty-timezone-message-wrapper').toggle();
    });
  }
};

/**
 * Show the powered by Drupal image preview
 */
Drupal.behaviors.poweredByPreview = {
  attach: function (context, settings) {
    $('#edit-color, #edit-size').change(function () {
      var path = settings.basePath + 'misc/' + $('#edit-color').val() + '-' + $('#edit-size').val() + '.png';
      $('img.powered-by-preview').attr('src', path);
    });
  }
};


 /**
 * Show/hide settings for page caching depending on whether page caching is
 * enabled or not.
 */
Drupal.behaviors.pageCache = {
  attach: function (context, settings) {
    $('#edit-cache-0', context).change(function () {
      $('#page-compression-wrapper').hide();
      $('#cache-error').hide();
    });
    $('#edit-cache-1', context).change(function () {
      $('#page-compression-wrapper').show();
      $('#cache-error').hide();
    });
    $('#edit-cache-2', context).change(function () {
      $('#page-compression-wrapper').show();
      $('#cache-error').show();
    });
  }
};

/**
 * Attach the auto machine readable name behavior.
 *
 * Settings are expected to be an object of elements to process, where the key
 * defines the source element in the form and the value is an object defining
 * the following properties:
 * - text: The label to display before the auto-generated value.
 * - target: The target form element name.
 * - searchPattern: A regular expression (without modifiers) matching disallowed
 *   characters in the machine readable name, f.e. '[^a-z0-9]+'.
 * - replaceToken: A replacement string to replace disallowed characters, f.e.
 *   '-' or '_'.
 *
 * @see menu_edit_menu()
 */
Drupal.behaviors.machineReadableValue = {
  attach: function () {
    for (var value in Drupal.settings.machineReadableValue) {
      var settings = Drupal.settings.machineReadableValue[value];

      var searchPattern = new RegExp(settings.searchPattern, 'g');
      // Build selector for the source name entered by a user.
      var source = '#edit-' + value;
      var suffix = source + '-suffix';
      // Build selector for the machine readable name.
      var target = '#edit-' + settings.target;
      // Build selector for the wrapper element around the target field.
      var wrapper = '.form-item-' + settings.target;

      // Do not process the element if we got an error or the given name and the
      // machine readable name are identical or the machine readable name is
      // empty.
      if (!$(target).hasClass('error') && ($(target).val() == $(source).val().toLowerCase().replace(searchPattern, settings.replaceToken) || $(target).val() == '')) {
        // Hide wrapper element.
        $(wrapper).hide();
        // Bind keyup event to source element.
        $(source).keyup(function () {
          var machine = $(this).val().toLowerCase().replace(searchPattern, settings.replaceToken);
          if (machine != '_' && machine != '') {
            // Set machine readable name to the user entered value.
            $(target).val(machine);
            // Append the machine readable name and a link to edit it to the source field.
            $(suffix).empty().append(' ' + settings.text + ': ' + machine + ' [').append($('<a href="#">' + Drupal.t('Edit') + '</a>').click(function () {
              $(wrapper).show();
              $(target).focus();
              $(suffix).hide();
              $(source).unbind('keyup');
              return false;
            })).append(']');
          }
          else {
            $(target).val(machine);
            $(suffix).text('');
          }
        });
        // Call keyup event on source element.
        $(source).keyup();
      }
    }
  }
};

})(jQuery);


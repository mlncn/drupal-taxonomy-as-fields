// $Id: openid.js,v 1.11 2009/07/04 14:57:23 dries Exp $
(function ($) {

Drupal.behaviors.openid = {
  attach: function (context) {
    var loginElements = $('.form-item-name-wrapper, .form-item-pass-wrapper, li.openid-link');
    var openidElements = $('.form-item-openid-identifier-wrapper, li.user-link');

    // This behavior attaches by ID, so is only valid once on a page.
    if (!$('#edit-openid-identifier.openid-processed').size() && $('#edit-openid-identifier').val()) {
      $('#edit-openid-identifier').addClass('openid-processed');
      loginElements.hide();
      // Use .css('display', 'block') instead of .show() to be Konqueror friendly.
      openidElements.css('display', 'block');
    }
    $('li.openid-link:not(.openid-processed)', context)
      .addClass('openid-processed')
      .click(function () {
         loginElements.hide();
         openidElements.css('display', 'block');
        // Remove possible error message.
        $('#edit-name, #edit-pass').removeClass('error');
        $('div.messages.error').hide();
        // Set focus on OpenID Identifier field.
        $('#edit-openid-identifier')[0].focus();
        return false;
      });
    $('li.user-link:not(.openid-processed)', context)
      .addClass('openid-processed')
      .click(function () {
         openidElements.hide();
         loginElements.css('display', 'block');
        // Clear OpenID Identifier field and remove possible error message.
        $('#edit-openid-identifier').val('').removeClass('error');
        $('div.messages.error').css('display', 'block');
        // Set focus on username field.
        $('#edit-name')[0].focus();
        return false;
      });
  }
};

})(jQuery);

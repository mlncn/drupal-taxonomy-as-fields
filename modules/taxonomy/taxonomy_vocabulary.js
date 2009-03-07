// $Id$
(function($) {

Drupal.behaviors.contentTypes = {
  attach: function() {
    if ($('#edit-field-name').val() == $('#edit-name').val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_') || $('#edit-field-name').val() == '') {
      $('#edit-field-name-wrapper').hide();
      $('#edit-name').keyup(function() {
        var machine = $(this).val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_');
        if (machine != '_' && machine != '') {
          $('#edit-field-name').val(machine);
          $('#vocabulary-field-name-suffix').empty().append(' Machine name: ' + machine + ' [').append($('<a href="#">'+ Drupal.t('Edit') +'</a>').click(function() {
            $('#edit-field-name-wrapper').show();
            $('#vocabulary-field-name-suffix').hide();
            $('#edit-name').unbind('keyup');
            return false;
          })).append(']');
        }
        else {
          $('#edit-field-name').val(machine);
          $('#vocabulary-field-name-suffix').text('');
        }
      });
      $('#edit-name').keyup();
    }
  }
};

})(jQuery);

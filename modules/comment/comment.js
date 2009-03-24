// $Id: comment.js,v 1.9 2009/03/13 23:15:09 webchick Exp $
(function($) {

Drupal.behaviors.comment = {
  attach: function(context, settings) {
    var parts = new Array("name", "homepage", "mail");
    var cookie = '';
    for (i=0;i<3;i++) {
      cookie = Drupal.comment.getCookie('comment_info_' + parts[i]);
      if (cookie != '') {
        $("#comment-form input[name=" + parts[i] + "]:not(.comment-processed)", context)
          .val(cookie)
          .addClass('comment-processed');
      }
    }
  }
};

Drupal.comment = {};

Drupal.comment.getCookie = function(name) {
  var search = name + '=';
  var returnValue = '';

  if (document.cookie.length > 0) {
    offset = document.cookie.indexOf(search);
    if (offset != -1) {
      offset += search.length;
      var end = document.cookie.indexOf(';', offset);
      if (end == -1) {
        end = document.cookie.length;
      }
      returnValue = decodeURIComponent(document.cookie.substring(offset, end).replace(/\+/g, '%20'));
    }
  }

  return returnValue;
};

})(jQuery);

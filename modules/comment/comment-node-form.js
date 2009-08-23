// $Id: comment-node-form.js,v 1.3 2009/08/21 00:21:48 webchick Exp $

(function ($) {

Drupal.behaviors.commentFieldsetSummaries = {
  attach: function (context) {
    $('fieldset#edit-comment-settings', context).setSummary(function (context) {
      return Drupal.checkPlain($('input:checked', context).parent().text());
    });
    // Provide the summary for the node type form.
    $('fieldset#edit-comment', context).setSummary(function(context) {
      var vals = [];

      // Default comment setting.
      vals.push($("select[name='comment'] option:selected", context).text());

      // Threading.
      var threading = $("input[name='comment_default_mode']:checked", context).parent().text();
      if (threading) {
        vals.push(threading);
      }

      // Comments per page.
      var number = $("select[name='comment_default_per_page'] option:selected", context).val();
      vals.push(Drupal.t('@number comments per page', {'@number': number}));

      return Drupal.checkPlain(vals.join(', '));
    });
  }
};

})(jQuery);

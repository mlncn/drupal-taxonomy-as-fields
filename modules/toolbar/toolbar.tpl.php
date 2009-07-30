<?php
// $Id: toolbar.tpl.php,v 1.2 2009/07/29 12:28:41 dries Exp $

/**
 * @file
 * Default template for admin toolbar.
 *
 * Available variables:
 * - $toolbar['toolbar_user']: User account / logout links.
 * - $toolbar['toolbar_menu']: Top level management menu links.
 * - $toolbar['toolbar_shortcuts']: Convenience shortcuts.
 *
 * @see template_preprocess()
 * @see template_preprocess_admin_toolbar()
 */
?>
<div id="toolbar" class="clearfix">
  <div class="toolbar-menu clearfix">
    <span class="toggle toggle-active"><?php print t('Show shortcuts'); ?></span>
    <?php print render($toolbar['toolbar_menu']); ?>
    <?php print render($toolbar['toolbar_user']); ?>
  </div>

  <div class="toolbar-shortcuts clearfix">
    <?php print render($toolbar['toolbar_shortcuts']); ?>
  </div>

  <div class="shadow"></div>
</div>

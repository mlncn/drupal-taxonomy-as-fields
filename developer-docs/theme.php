<?php
// $Id: theme.php,v 1.2 2009/04/28 16:41:35 darrenoh Exp $

/**
 * @file
 * Shadow theme functions for theme templates.
 *
 * This file provides function definitions for output that is generated by theme
 * templates by default. Themers who wish to override output with theme
 * functions instead of templates can find documentation for those functions
 * here.
 *
 * @addtogroup themeable
 * @{
 */

/**
 * Theme a user page.
 *
 * @param $elements
 *   A content form array. $elements['#account'] contains the user object.
 * @return
 *   A formatted HTML string.
 *
 * @see template_preprocess_user_profile()
 * @see user-profile.tpl.php
 */
function theme_user_profile($elements) {
  return drupal_render($elements);
}

/**
 * @} End of "addtogroup themeable".
 */

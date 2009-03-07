<?php
// $Id: forum-submitted.tpl.php,v 1.4 2008/10/13 12:31:42 dries Exp $

/**
 * @file
 * Default theme implementation to format a simple string indicated when and
 * by whom a topic was submitted.
 *
 * Available variables:
 *
 * - $author: The author of the post.
 * - $time: How long ago the post was created.
 * - $topic: An object with the raw data of the post. Unsafe, be sure
 *   to clean this data before printing.
 *
 * @see template_preprocess_forum_submitted()
 * @see theme_forum_submitted()
 */
?>
<?php if ($time): ?>
  <?php print t(
  '@time ago<br />by !author', array(
    '@time' => $time,
    '!author' => $author,
    )); ?>
<?php else: ?>
  <?php print t('n/a'); ?>
<?php endif; ?>

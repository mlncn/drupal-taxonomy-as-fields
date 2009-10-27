<?php
// $Id: node.tpl.php,v 1.19 2009/10/19 01:30:07 dries Exp $
?>
<div id="node-<?php print $node->nid; ?>" class="<?php print $classes; ?>"<?php print $attributes; ?>>

  <?php if (!$page && $contextual_links): ?>
    <?php print render($contextual_links); ?>
  <?php endif; ?>

  <?php print $user_picture; ?>

  <?php if (!$page): ?>
    <h2<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>"><?php print $node_title; ?></a></h2>
  <?php endif; ?>

  <?php if ($display_submitted): ?>
    <span class="submitted"><?php print $date; ?> — <?php print $name; ?></span>
  <?php endif; ?>

  <div class="content clearfix"<?php print $content_attributes; ?>>
    <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['comments']);
      hide($content['links']);
      print render($content);
    ?>
  </div>

  <div class="clearfix">
    <?php if (!empty($content['links']['terms'])): ?>
      <div class="meta">
        <div class="terms"><?php print render($content['links']['terms']); ?></div>
      </div>
    <?php endif; ?>

    <?php if (!empty($content['links'])): ?>
      <div class="links"><?php print render($content['links']); ?></div>
    <?php endif; ?>

    <?php print render($content['comments']); ?>
  </div>

</div>

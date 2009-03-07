<?php
// $Id: node.tpl.php,v 1.7 2009/01/26 14:08:44 dries Exp $
?>
<div class="node<?php if ($sticky) { print " sticky"; } ?><?php if (!$status) { print " node-unpublished"; } ?>">
  <?php print $picture ?>
  <?php if (!$page): ?>
    <h1 class="title"><a href="<?php print $node_url ?>"><?php print $title ?></a></h1>
  <?php endif; ?>
    <span class="submitted"><?php print $submitted ?></span>
    <div class="taxonomy"><?php print $terms ?></div>
    <div class="content"><?php print $content ?></div>
    <?php if ($links): ?>
      <div class="links">&raquo; <?php print $links ?></div>
    <?php endif; ?>

    <?php print $comments; ?>

</div>

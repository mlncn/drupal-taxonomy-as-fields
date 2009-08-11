<?php
// $Id$

/**
 * @file
 * Default theme implementation to configure field display settings.
 *
 * Available variables:
 * @todo
 */
?>
<div>
  <?php print $help; ?>
</div>
<?php if ($rows): ?>
  <table id="field-display-overview" class="sticky-enabled">
    <thead>
      <tr>
        <th>&nbsp;</th>
        <?php foreach ($contexts as $key => $value): ?>
          <th colspan="2"><?php print $value; ?>
        <?php endforeach; ?>
      </tr>
      <tr>
        <th><?php print t('Field'); ?></th>
        <?php foreach ($contexts as $key => $value): ?>
          <th><?php print t('Label'); ?></th>
          <th><?php print t('Format'); ?></th>
        <?php endforeach; ?>
      </tr>
    </thead>
    <tbody>
      <?php
      $count = 0;
      foreach ($rows as $row): ?>
        <tr class="<?php print $count % 2 == 0 ? 'odd' : 'even'; ?>">
          <td><?php print $row->indentation; ?><span class="<?php print $row->label_class; ?>"><?php print $row->human_name; ?></span></td>
          <?php foreach ($contexts as $context => $title): ?>
            <td><?php print $row->{$context}->label; ?></td>
            <td><?php print $row->{$context}->type; ?></td>
          <?php endforeach; ?>
        </tr>
        <?php $count++;
      endforeach; ?>
    </tbody>
  </table>
  <?php print $submit; ?>
<?php endif; ?>

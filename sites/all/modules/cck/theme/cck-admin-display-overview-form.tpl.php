<?php
// $Id: cck-admin-display-overview-form.tpl.php,v 1.5 2009/02/03 19:53:48 karens Exp $
?>
<div>
  <?php print $help; ?>
</div>
<?php if ($rows): ?>
  <table id="cck-display-overview" class="sticky-enabled">
    <thead>
      <tr>
        <th>&nbsp;</th>
        <?php foreach ($contexts as $key => $value): ?>
          <th colspan="3"><?php print $value; ?>
        <?php endforeach; ?>
      </tr>
      <tr>
        <th><?php print t('Field'); ?></th>
        <?php foreach ($contexts as $key => $value): ?>
          <th><?php print t('Label'); ?></th>
          <th><?php print t('Format'); ?></th>
          <th><?php print t('Exclude'); ?></th>
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
            <td><?php print $row->{$context}->exclude; ?></td>
          <?php endforeach; ?>
        </tr>
        <?php $count++;
      endforeach; ?>
    </tbody>
  </table>
  <?php print $submit; ?>
<?php endif; ?>

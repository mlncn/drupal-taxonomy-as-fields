<?php
// $Id$

/**
 * @file
 * Default theme implementation to configure field settings.
 *
 * Available variables:
 * @todo
 */
?>
<div>
  <?php print $help; ?>
</div>
<table id="field-overview" class="sticky-enabled">
  <thead>
    <tr>
      <th><?php print t('Label'); ?></th>
      <th><?php print t('Weight'); ?></th>
      <th><?php print t('Name'); ?></th>
      <th><?php print t('Field'); ?></th>
      <th><?php print t('Widget'); ?></th>
      <th><?php print t('Operations'); ?></th>
    </tr>
  </thead>
  <tbody>
    <?php
    $count = 0;
    foreach ($rows as $row): ?>
      <tr class="<?php print $count % 2 == 0 ? 'odd' : 'even'; ?> <?php print $row->class ?>">
      <?php
      switch ($row->row_type):
        case 'field': ?>
          <td>
            <?php print $row->indentation; ?>
            <span class="<?php print $row->label_class; ?>"><?php print $row->label; ?></span>
          </td>
          <td><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td><?php print $row->field_name; ?></td>
          <td><?php print $row->type; ?></td>
          <td><?php print $row->widget_type; ?></td>
          <td><?php print $row->configure; ?>&nbsp;&nbsp;<?php print $row->remove; ?></td>
          <?php break;
        case 'group': ?>
          <td>
            <?php print $row->indentation; ?>
            <span class="<?php print $row->label_class; ?>"><?php print $row->label; ?></span>
          </td>
          <td><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td><?php print $row->group_name; ?></td>
          <td><?php print $row->group_type; ?></td>
          <td><?php print $row->configure; ?>&nbsp;&nbsp;<?php print $row->remove; ?></td>
          <?php break;
        case 'extra': ?>
          <td>
            <?php print $row->indentation; ?>
            <span class="<?php print $row->label_class; ?>"><?php print $row->label; ?></span>
          </td>
          <td><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td colspan="3"><?php print $row->description; ?></td>
          <td><?php print $row->configure; ?>&nbsp;&nbsp;<?php print $row->remove; ?></td>
          <?php break;
        case 'separator': ?>
          <td colspan="6" class="region"><?php print t('Add'); ?></td>
          <?php break;
        case 'add_new_field': ?>
          <td>
            <?php print $row->indentation; ?>
            <div class="<?php print $row->label_class; ?>">
              <div class="new"><?php t('New field'); ?></div>
              <?php print $row->label; ?>
            </div>
          </td>
          <td><div class="new">&nbsp;</div><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td colspan="2"><div class="new">&nbsp;</div><?php print $row->field_name; ?></td>
          <td><div class="new">&nbsp;</div><?php print $row->type; ?></td>
          <td><div class="new">&nbsp;</div><?php print $row->widget_type; ?></td>
          <?php break;
        case 'add_existing_field': ?>
          <td>
            <?php print $row->indentation; ?>
            <div class="<?php print $row->label_class; ?>">
              <div class="new"><?php print t('Existing field'); ?></div>
              <?php print $row->label; ?>
            </div>
          </td>
          <td><div class="new">&nbsp;</div><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td colspan="3"><div class="new">&nbsp;</div><?php print $row->field_name; ?></td>
          <td><div class="new">&nbsp;</div><?php print $row->widget_type; ?></td>
          <?php break;
       case 'add_new_group': ?>
          <td>
            <?php print $row->indentation; ?>
            <div class="<?php print $row->label_class; ?>">
              <div class="new"><?php t('New group'); ?></div>
              <?php print $row->label; ?>
            </div>
          </td>
          <td><div class="new">&nbsp;</div><?php print $row->weight . $row->parent . $row->hidden_name; ?></td>
          <td colspan="2"><div class="new">&nbsp;</div><?php print $row->group_name; ?></td>
          <td><div class="new">&nbsp;</div><?php print $row->group_type; ?></td>
          <td><div class="new">&nbsp;</div><?php print $row->group_option; ?></td>
        <?php break;
      endswitch; ?>
      </tr>
      <?php $count++;
    endforeach; ?>
  </tbody>
</table>

<?php print $submit; ?>

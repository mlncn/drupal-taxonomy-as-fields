<?php
// $Id: comment.api.php,v 1.12 2009/10/10 13:37:09 dries Exp $

/**
 * @file
 * Hooks provided by the Comment module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * The comment passed validation and is about to be saved.
 *
 * Modules may make changes to the comment before it is saved to the database.
 *
 * @param $comment
 *   The comment object.
 */
function hook_comment_presave($comment) {
  // Remove leading & trailing spaces from the comment subject.
  $comment->subject = trim($comment->subject);
}

/**
 * The comment is being inserted.
 *
 * @param $comment
 *   The comment object.
 */
function hook_comment_insert($comment) {
  // Reindex the node when comments are added.
  search_touch_node($comment->nid);
}

/**
 * The comment is being updated.
 *
 * @param $comment
 *   The comment object.
 */
function hook_comment_update($comment) {
  // Reindex the node when comments are updated.
  search_touch_node($comment->nid);
}

/**
 * Comments are being loaded from the database.
 *
 * @param $comments
 *  An array of comment objects indexed by cid.
 */
function hook_comment_load($comments) {
  $result = db_query('SELECT cid, foo FROM {mytable} WHERE cid IN (:cids)', array(':cids' => array_keys($comments)));
  foreach ($result as $record) {
    $comments[$record->cid]->foo = $record->foo;
  }
}

/**
 * The comment is being viewed. This hook can be used to add additional data to the comment before theming.
 *
 * @param $comment
 *   Passes in the comment the action is being performed on.
 * @return
 *   Nothing.
 */
function hook_comment_view($comment) {
  // how old is the comment
  $comment->time_ago = time() - $comment->changed;
}

/**
 * The comment is being published by the moderator.
 *
 * @param $comment
 *   Passes in the comment the action is being performed on.
 * @return
 *   Nothing.
 */
function hook_comment_publish($comment) {
  drupal_set_message(t('Comment: @subject has been published', array('@subject' => $comment->subject)));
}

/**
 * The comment is being unpublished by the moderator.
 *
 * @param $comment
 *   Passes in the comment the action is being performed on.
 * @return
 *   Nothing.
 */
function hook_comment_unpublish($comment) {
  drupal_set_message(t('Comment: @subject has been unpublished', array('@subject' => $comment->subject)));
}

/**
 * The comment is being deleted by the moderator.
 *
 * @param $comment
 *   Passes in the comment the action is being performed on.
 * @return
 *   Nothing.
 */
function hook_comment_delete($comment) {
  drupal_set_message(t('Comment: @subject has been deleted', array('@subject' => $comment->subject)));
}

/**
 * @} End of "addtogroup hooks".
 */

<?php
// $Id: simpletest.api.php,v 1.2 2009/05/24 17:39:34 dries Exp $

/**
 * @file
 * Hooks provided by the SimpleTest module.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * A test group has started.
 *
 * This hook is called just once at the beginning of a test group.
 *
 * @return
 *   None.
 *
 */
function hook_test_group_started() {
}

/**
 * A test group has finished.
 *
 * This hook is called just once at the end of a test group.
 *
 * @return
 *   None.
 *
 */
function hook_test_group_finished() {
}

/**
 * An individual test has finished.
 *
 * This hook is called when an individual test has finished.
 *
 * @param
 *   $results The results of the test as gathered by DrupalWebTestCase.
 *
 * @return
 *   None.
 *
 * @see DrupalWebTestCase->results
 */
function hook_test_finished($results) {
}


/**
 * @} End of "addtogroup hooks".
 */

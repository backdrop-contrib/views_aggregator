<?php

/**
 * @file
 * views_aggregator_functions.inc
 *
 * The complete set of views aggregation functions that come with the module.
 * Note that other modules can add their own functions by implementing
 * hook_views_aggregation_functions_info()
 */

/**
 * Implements hook_views_aggregation_functions_info().
 *
 * @return array
 *   array of available aggregation function names with descriptions to display
 *   in the Views UI
 */
function views_aggregator_views_aggregation_functions_info() {
  $functions = array(
    'views_aggregator_group_and_compress' => t('Group and compress into single rows'),
    'views_aggregator_count' => t('Count no. members in group'),
    'views_aggregator_sum' => t('Sum group'),
    'views_aggregator_average' => t('Average group'),
    'views_aggregator_maximum' => t('Maximum value in group'),
    'views_aggregator_minimum' => t('Minimum value in group'),
    'views_aggregator_enumerate' => t('Enumerate group members'),
    'views_aggregator_tally' => t('Enumerate group members (with counts)'),
    'views_aggregator_first' => t('Display only the first member (default)'),
    'views_aggregator_replace' => t('Replace group by text (supply below)'),
  );
  return $functions;
}

/**
 * Aggregates the supplied view results into grouped rows.
 *
 * This function must be selected for one column (field) in the results table.
 *
 * @param object $view
 *   the view object with query results
 * @param object $field_handler
 *   the handler for the view column to group rows on
 */
function views_aggregator_group_and_compress($view, $field_handler) {
  foreach ($view->result as $num => $row) {
    $group_value = views_aggregator_get_cell($field_handler, $row);
    $view->group[$group_value][$num] = $row;
  }
}

/**
 * Aggregates a group of result rows as the minimum amongst its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find the minimum in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_average($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $sum = 0.0;
    foreach ($rows as $row) {
      $sum += views_aggregator_get_cell($field_handler, $row);
    }
    $values[$group] = $sum / count($rows);
  }
  return $values;
}

/**
 * Aggregates a group of result rows as a count of the number of group members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to count groups members in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_count($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $values[$group] = count($rows);
  }
  return $values;
}

/**
 * Aggregates a group of result rows as the enumeration of its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find members of the group
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_enumerate($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $cell_values = array();
    foreach ($rows as $num => $row) {
      $cell = views_aggregator_get_cell($field_handler, $row);
      if (!in_array($cell, $cell_values)) {
        $cell_values[] = $cell;
      }
    }
    if (count($cell_values) > 1) {
      // After grouping the fields in the group no longer belong to one
      // entity. Cannot easily support hyper-linking, so switch it off.
      unset($field_handler->options['link_to_node']);
      @sort($cell_values, SORT_NATURAL | SORT_FLAG_CASE);
      $values[$group] = implode(', ', $cell_values);
    }
    else {
      // Leave the original cell value as is.
      $values[$group] = NULL;
    }
  }
  return $values;
}

/**
 * Aggregates a group of result rows as the first member of the group.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find the first group member in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_first($view, $field_handler) {
  // This is the default operation anyway, so nothing to do.
  return array();
}

/**
 * Aggregates a group of result rows as the maximum across its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find the maximum groups member in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_maximum($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $maximum = views_aggregator_get_cell($field_handler, reset($rows));
    while ($row = next($rows)) {
      $value = views_aggregator_get_cell($field_handler, $row);
      if ($value > $maximum) {
        $maximum = $value;
      }
    };
    $values[$group] = $maximum;
  }
  return $values;
}

/**
 * Aggregates a group of result rows as the minimum across its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find the minimum groups member in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_minimum($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $minimum = views_aggregator_get_cell($field_handler, reset($rows));
    while ($row = next($rows)) {
      $value = views_aggregator_get_cell($field_handler, $row);
      if ($value < $minimum) {
        $minimum = $value;
      }
    };
    $values[$group] = $minimum;
  }
  return $values;
}

/**
 * Aggregates a group of result rows as a word, phrase or number.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column
 * @param string $parameter
 *   an optional parameter, specifying the replacement text to use
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_replace($view, $field_handler, $parameter = NULL) {
  if (!isset($parameter)) {
    $parameter = '-' . t('many') . '-';
  }
  $values = array();
  foreach ($view->group as $group => $rows) {
    if (count($rows) > 1) {
      $values[$group] = $parameter;
      // With more than one field in a group the fields no longer belong to one
      // particular entity. Cannot support hyper-linking, so switch it off.
      // Unfortunately this applies to the entire column.
      unset($field_handler->options['link_to_node']);
    }
    else {
      $values[$group] =  NULL;
    }
  }
  return $values;
}

/**
 * Aggregates a group of result rows as a sum.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to sum groups in
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_sum($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $sum = 0.0;
    foreach ($rows as $row) {
      $sum += views_aggregator_get_cell($field_handler, $row);
    };
    $values[$group] = $sum;
  }
  return $values;
}

/**
 * Aggregates a group of result rows as the tally of its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to find members of the group
 *
 * @return array
 *   an array of column values, one for each group.
 */
function views_aggregator_tally($view, $field_handler) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    $tally = array();
    foreach ($rows as $num => $row) {
      $cell = views_aggregator_get_cell($field_handler, $row);
      if (isset($tally[$cell])) {
        $tally[$cell]++;
      }
      else {
        $tally[$cell] = 1;
      }
    }
    if (count($tally) > 1) {
      // With more than one field in a group the fields no longer belong to one
      // particular entity. Cannot support hyper-linking, so switch it off.
      // Unfortunately this applies to the entire column.
      unset($field_handler->options['link_to_node']);
    }
    @ksort($tally, SORT_NATURAL | SORT_FLAG_CASE);
    $rendered_tally = array();
    foreach ($tally as $cell => $count) {
      $rendered_tally[] = "$cell ($count)";
    }
    $values[$group] = implode(', ', $rendered_tally);
  }
  return $values;
}
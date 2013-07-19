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
    'views_aggregator_group_and_compress' => array(
      'group' => t('Group and compress'),
      'column' => NULL,
    ),
    'views_aggregator_count' => array(
      'group' => t('Count (having value)'),
      'column' => t('Count (having value)')
    ),
    'views_aggregator_sum' => array(
      'group' => t('Sum'),
      'column' => t('Sum'),
    ),
    'views_aggregator_average' => array(
      'group' => t('Average'),
      'column' => t('Average'),
    ),
    'views_aggregator_maximum' => array(
      'group' => t('Maximum'),
      'column' => t('Maximum'),
    ),
    'views_aggregator_minimum' => array(
      'group' => t('Minimum'),
      'column' => t('Minimum '),
    ),
    'views_aggregator_enumerate' => array(
      'group' => t('Enumerate members'),
      'column' => NULL,
    ),
    'views_aggregator_tally' => array(
      'group' => t('Tally members'),
      'column' => NULL,
    ),
    'views_aggregator_first' => array(
      'group' => t('Display first member'),
      'column' => NULL,
    ),
    'views_aggregator_replace' => array(
      'group' => t('Label (enter below)'),
      'column' => t('Label (enter below)'),
    ),
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
 *   an array of column values, one for each group and one for the column.
 */
function views_aggregator_average($view, $field_handler) {
  $values = array();
  $sum_column = 0.0;
  $count_all_rows = 0;
  foreach ($view->group as $group => $rows) {
    $sum = 0.0;
    foreach ($rows as $row) {
      $sum += views_aggregator_get_cell($field_handler, $row);
    }
    $values[$group] = $sum / count($rows);
    $sum_column += $sum;
    $count_all_rows += count($rows);
  }
  $values['column'] = $sum_column / $count_all_rows;
  return $values;
}

/**
 * Aggregates a group of result rows as a count of the number of group members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column to count groups members in
 * @parm string $value
 *   an optional value to count, if omitted all group values count
 * 
 * @return array
 *   an array of column values, one for each group and one for the column
 */
function views_aggregator_count($view, $field_handler, $group_value = NULL, $column_value = NULL) {
  $values = array();
  $count_column = 0;
  $value = isset($group_value) ? $group_value : $column_value;
  foreach ($view->group as $group => $rows) {
    if (isset($value)) {
      $count = 0;
      foreach ($rows as $row) {
        $cell = views_aggregator_get_cell($field_handler, $row);
        if ($cell == $value) {
          $count++;
        }
      }
    }
    else {
      $count = count($rows);
    }
    $values[$group] = $count;
    $count_column += $count;
  }
  $values['column'] = $count_column;
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
    foreach ($rows as $row) {
      $cell = views_aggregator_get_cell($field_handler, $row);
      if (!empty($cell) && !in_array($cell, $cell_values)) {
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
 *   an array of column values, one for each group and one for the column
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
    if (!isset($maximum_column) || $maximum > $maximum_column) {
      $maximum_column = $maximum;
    }
  }
  $values['column'] = $maximum_column;
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
 *   an array of column values, one for each group and one for the column.
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
    if (!isset($minimum_column) || $minimum < $minimum_column) {
      $minimum_column = $minimum;
    }
  }
  $values['column'] = $minimum_column;
  return $values;
}

/**
 * Aggregates a group of result rows as a word, phrase or number.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 * @param object $field_handler
 *   the handler for the view column
 * @param string $replace_text
 *   an optional parameter, specifying the replacement text to use
 * @param string $column_label
 *   an optional parameter, specifying a label placed the bottom of the column
 *
 * @return array
 *   an array of column values, one for each group and one for the column
 */
function views_aggregator_replace($view, $field_handler, $replace_text = NULL, $column_label = NULL) {
  $values = array();
  foreach ($view->group as $group => $rows) {
    if (isset($replace_text)) {
      $values[$group] = $replace_text;
    }
    if (count($rows) > 1 && isset($replace_text)) {
      // With more than one field in a group the fields no longer belong to one
      // particular entity. Cannot support hyper-linking, so switch it off.
      // Unfortunately this applies to the entire column.
      unset($field_handler->options['link_to_node']);
    }
  }
  if (isset($column_label)) {
    $values['column'] = $column_label;
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
  $sum_column = 0.0;
  foreach ($view->group as $group => $rows) {
    $sum = 0.0;
    foreach ($rows as $row) {
      $sum += views_aggregator_get_cell($field_handler, $row);
    };
    $values[$group] = $sum;
    $sum_column += $sum;
  }
  $values['column'] = $sum_column;
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
      if (empty($cell)) {
        // No tallying empty values.
        $values[$group] = NULL;
        break;
      }
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
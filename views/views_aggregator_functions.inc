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
 *   array of function names with descriptions to display in the Views UI
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
    'views_aggregator_replace' => t('Replace group by fixed text or number'),
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
 *
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
 *
 * @param object $field_handler
 *   the handler for the view column to find the minimum in
 */
function views_aggregator_average($view, $field_handler) {
  foreach ($view->group as $rows) {
    $sum = 0.0;
    foreach ($rows as $row) {
      $sum += views_aggregator_get_cell($field_handler, $row);
    }
    $average = $sum / count($rows);
    foreach ($rows as $num => &$row) {
      views_aggregator_set_cell($field_handler, $row, $num, $average);
      // Only have to set on the first of the group
      break;
    }
  }
}

/**
 * Aggregates a group of result rows as a count of the number of group members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to count groups members in
 */
function views_aggregator_count($view, $field_handler) {
  foreach ($view->group as &$rows) {
    $count = count($rows);
    foreach ($rows as $num => $row) {
      views_aggregator_set_cell($field_handler, $row, $num, $count);
      // Only have to set on the first of the group
      break;
    }
  }
}

/**
 * Aggregates a group of result rows as the enumeration of its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to find members of the group
 */
function views_aggregator_enumerate($view, $field_handler) {
  foreach ($view->group as $rows) {
    if (count($rows) <= 1) {
      continue;
    }
    $cell_values = array();
    foreach ($rows as $num => $row) {
      $cell = views_aggregator_get_cell($field_handler, $row);
      if (!in_array($cell, $cell_values)) {
        $cell_values[] = $cell;
      }
    }
    @sort($cell_values, SORT_NATURAL | SORT_FLAG_CASE);
    foreach ($rows as $num => &$row) {
      views_aggregator_set_cell($field_handler, $row, $num, implode(', ', $cell_values));
      // After grouping the row no longer belongs to one particular entity.
      // Cannot easily support hyper-linking, so switch it off for now.
      // Unfortunately this affects the entire column, as this setting applies
      // to the field, not just the group.
      unset($field_handler->options['link_to_node']);
      break;
    }
  }
}

/**
 * Aggregates a group of result rows as the first member of the group.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to find the first group member in
 */
function views_aggregator_first($view, $field_handler) {
  // This is the default operation anyway, so nothing to do.
  return;
}

/**
 * Aggregates a group of result rows as the maximum across its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to find the maximum groups member in
 */
function views_aggregator_maximum($view, $field_handler) {
  foreach ($view->group as $rows) {
    $maximum = views_aggregator_get_cell($field_handler, reset($rows));
    while ($row = next($rows)) {
      $value = views_aggregator_get_cell($field_handler, $row);
      if ($value > $maximum) {
        $maximum = $value;
      }
    };
    foreach ($rows as $num => &$row) {
      views_aggregator_set_cell($field_handler, $row, $num, $maximum);
      // Only have to set on the first of the group
      break;
    }
  }
}

/**
 * Aggregates a group of result rows as the minimum across its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to find the minimum groups member in
 */
function views_aggregator_minimum($view, $field_handler) {
  foreach ($view->group as $rows) {
    $minimum = views_aggregator_get_cell($field_handler, reset($rows));
    while ($row = next($rows)) {
      $value = views_aggregator_get_cell($field_handler, $row);
      if ($value < $minimum) {
        $minimum = $value;
      }
    };
    foreach ($rows as $num => &$row) {
      views_aggregator_set_cell($field_handler, $row, $num, $minimum);
      // Only have to set on the first of the group
      break;
    }
  }
}

/**
 * Aggregates a group of result rows as a word, phrase or number.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column
 */
function views_aggregator_replace($view, $field_handler, $phrase = NULL) {
  if (!isset($phrase)) {
    $phrase = '-' . t('many') . '-';
  }
  foreach ($view->group as &$rows) {
    if (count($rows) > 1) {
      foreach ($rows as $num => $row) {
        views_aggregator_set_cell($field_handler, $row, $num, $phrase);
        // After grouping the row no longer belongs to one particular entity.
        // Cannot easily support hyper-linking, so switch it off for now.
        unset($field_handler->options['link_to_node']);
        // Only have to set on the first of the group
        break;
      }
    }
  }
}

/**
 * Aggregates a group of result rows as a sum.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to sum groups in
 */
function views_aggregator_sum($view, $field_handler) {
  foreach ($view->group as $rows) {
    $sum = 0.0;
    if (count($rows) > 1) {
      foreach ($rows as $row) {
        $sum += views_aggregator_get_cell($field_handler, $row);
      };
      foreach ($rows as $num => &$row) {
        views_aggregator_set_cell($field_handler, $row, $num, $sum);
        // Only have to set on the first of the group
        break;
      }
    }
  }
}

/**
 * Aggregates a group of result rows as the tally of its members.
 *
 * @param object $view
 *   the view object with grouped query result rows in $view->group
 *
 * @param object $field_handler
 *   the handler for the view column to find members of the group
 */
function views_aggregator_tally($view, $field_handler) {
  foreach ($view->group as $rows) {
    if (count($rows) <= 1) {
      continue;
    }
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
    @ksort($tally, SORT_NATURAL | SORT_FLAG_CASE);
    $rendered_tally = array();
    foreach ($tally as $cell => $count) {
      $rendered_tally[] = "$cell ($count)";
    }
    foreach ($rows as $num => &$row) {
      views_aggregator_set_cell($field_handler, $row, $num, implode(', ', $rendered_tally));
      // After grouping the row no longer belongs to one particular entity.
      // Cannot easily support hyper-linking, so switch it off for now.
      unset($field_handler->options['link_to_node']);
      break;
    }
  }
}
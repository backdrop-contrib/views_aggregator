<?php
/**
 * @file
 * views_aggregator_plugin_style_table.inc
 *
 * Based on the default Views table style plugin and a similar class in the
 * Views Calc module.
 */

/**
 * Style plugin to render each item as a row in a table.
 *
 * @ingroup views_style_plugins
 */
class views_aggregator_plugin_style_table extends views_plugin_style_table {

  /**
   * Option definition.
   */
  public function option_definition() {
    $options = parent::option_definition();
    // Add defaults here, if any
    return $options;
  }

  /**
   * Create the tabular form with the aggregation options.
   */
  public function options_form(&$form, &$form_state) {
    // Note: bulk of form is provided by superclass views_plugin_style_table.
    parent::options_form($form, $form_state);

    // See function views_aggregator_theme().
    $form['#theme'] = 'views_aggregator_plugin_style_table';

    // Views style of grouping (splitting table into many) interferes, so
    // get rid of the form.
    unset($form['grouping']);

    $form['description_markup'] = array(
      '#markup' => '<div class="description form-item">' . t('Apply <em>"Group and compress"</em> to <strong>one</strong> field only. Select any of the other aggregation functions for some or all of the fields. Some functions take an optional parameter, but most do not. You may combine multiple fields into the same column. If you do, the separator specified will be used to separate the fields. You may control column order and field labels in the Fields section of the main configuration pain.') . '</div>',
    );

    $columns = $this->sanitize_columns($this->options['columns']);
    foreach ($columns as $field => $column) {
      $safe = str_replace(array('][', '_', ' '), '-', $field);

      $form['info'][$field]['has_aggr'] = array(
        '#type' => 'checkbox',
        '#title' => t('Apply function'),
        '#default_value' => isset($this->options['info'][$field]['has_aggr']) ? $this->options['info'][$field]['has_aggr'] : FALSE,
      );

      $group_options = array();
      $column_options = array();
      foreach (views_aggregator_get_aggregation_functions_info() as $function => $display_names) {
        if (!empty($display_names['group'])) {
          $group_options[$function] = $display_names['group'];
        }
        if (!empty($display_names['column'])) {
          $column_options[$function] = $display_names['column'];
        }
      }
      $form['info'][$field]['aggr'] = array(
        '#type' => 'select',
        '#options' => $group_options,
        '#multiple' => TRUE,
        '#default_value' => empty($this->options['info'][$field]['aggr']) ? array('views_aggregator_first') : $this->options['info'][$field]['aggr'],
        '#states' => array(
          'visible' => array(
            'input[name="style_options[info][' . $field . '][has_aggr]"]' => array(
              'checked' => TRUE,
            ),
          ),
        ),
      );
      // Optional parameter for the selected aggregation function.
      $form['info'][$field]['aggr_par'] = array(
        '#type' => 'textfield',
        '#size' => 23,
        '#title' => t('Parameter'),
        '#default_value' => isset($this->options['info'][$field]['aggr_par']) ? $this->options['info'][$field]['aggr_par'] : '',
        '#states' => array(
          'visible' => array(
            'input[name="style_options[info][' . $field . '][has_aggr]"]' => array(
              'checked' => TRUE,
            ),
            'select[name="style_options[info][' . $field . '][aggr]"' => array(
              // 'value' => array('views_aggregator_replace') Can't get this to work
            )
          ),
        ),
      );

      $form['info'][$field]['has_aggr_column'] = array(
        '#type' => 'checkbox',
        '#title' => t('Apply function'),
        '#default_value' => isset($this->options['info'][$field]['has_aggr_column']) ? $this->options['info'][$field]['has_aggr_column'] : FALSE,
      );
      $form['info'][$field]['aggr_column'] = array(
        '#type' => 'select',
        '#options' => $column_options,
        '#multiple' => FALSE,
        '#default_value' => empty($this->options['info'][$field]['aggr_column']) ? 'views_aggregator_sum' : $this->options['info'][$field]['aggr_column'],
        '#states' => array(
          'visible' => array(
            'input[name="style_options[info][' . $field . '][has_aggr_column]"]' => array(
              'checked' => TRUE,
            ),
          ),
        ),
      );
      // Optional parameter for the selected column aggregation function.
      $form['info'][$field]['aggr_par_column'] = array(
        '#type' => 'textfield',
        '#size' => 24,
        '#title' => t('Parameter'),
        '#default_value' => isset($this->options['info'][$field]['aggr_par_column']) ? $this->options['info'][$field]['aggr_par_column'] : '',
        '#states' => array(
          'visible' => array(
            'input[name="style_options[info][' . $field . '][has_aggr_column]"]' => array(
              'checked' => TRUE,
            ),
            'select[name="style_options[info][' . $field . '][aggr_column]"' => array(
              // 'value' => array('views_aggregator_replace') Can't get this to work
            )
          ),
        ),
      );
    }
  }

  /**
   * Overrides options_validate().
   */
  public function options_validate(&$form, &$form_state) {
    parent::options_validate($form, $form_state);

    // Count the number of occurrences of the grouping and other aggregation
    // functions.
    $num_grouped = 0;
    $num_aggregation_functions = 0;
    foreach ($form_state['values']['style_options']['info'] as $field_name => $options) {
      if (!empty($options['has_aggr'])) {
        $num_aggregation_functions += count($options['aggr']);
        if (in_array('views_aggregator_group_and_compress', $options['aggr'])) {
          $num_grouped++;
          $num_aggregation_functions--;
        }
      }
    }
    // When we have no aggregation functions, we must have 0 or 1 grouping
    // function. When we have aggregation functions, there must be 1 grouping.
    $ok = ($num_aggregation_functions == 0) ? $num_grouped <= 1 : $num_grouped == 1;
    if (!$ok) {
      $msg = t('When applying group aggregation functions, you must also select <em>"Group and compress"</em> for exactly one field.');
      foreach ($form_state['values']['style_options']['info'] as $field_name => $options) {
        form_error($form['info'][$field_name]['aggr'], $msg);
        $msg = '';
      }
    }
  }

  /*
   * Overrides pre_render().
   *
   * Note that this class being a views_plugin, rather than a views_handler,
   * it does not have a post_execute() function
   */
  public function pre_render($results) {
    parent::pre_render($results);
    if (empty($this->view->result)) {
      return;
    }
    // Aggregation functions rely on there being $view->group, as fleshed out by
    // views_aggregator_group_and_compress(), which must be called first and for
    // one column/field only.
    $this->view->group = array();

    // Find the column to group by and execute the grouping.
    foreach ($this->options['info'] as $field_name => $options) {
      if (!empty($options['has_aggr']) && in_array('views_aggregator_group_and_compress', $options['aggr'])) {
        if (isset($this->view->field[$field_name])) {
          $field_handler = $this->view->field[$field_name];
          views_aggregator_group_and_compress($this->view, $field_handler);
          break;
        }
      }
    }
    // Now call all the aggregation functions selected for each of the fields.
    $values = array();
    foreach ($this->options['info'] as $field_name => $options) {
      if (!isset($this->view->field[$field_name])) {
        continue;
      }
      // Make a list of the group and column functions to call for this field.
      $functions = empty($options['has_aggr_column']) ? array() : array($options['aggr_column']);
      if (!empty($options['has_aggr'])) {
        foreach ($options['aggr'] as $function) {
          if ($function != 'views_aggregator_group_and_compress' && !in_array($function, $functions)) {
            $functions[] = $function;
          }
        }
      }
      $field_handler = $this->view->field[$field_name];
      $values[$field_name] = FALSE;
      // Now execute the selected aggregation functions on this field.
      foreach ($functions as $function) {
        $group_par = (!isset($options['aggr_par']) || $options['aggr_par'] == '') ? NULL : $options['aggr_par'];
        $column_par = (!isset($options['aggr_par_column']) || $options['aggr_par_column'] == '') ? NULL : $options['aggr_par_column'];
        $return = $function($this->view, $field_handler, $group_par, $column_par);
        if (!empty($return)) {
          $values[$field_name] = $return;
        }
      }
    }
    // With the aggregation functions now executed, spare the first row of each
    // group as it holds the aggregation result. Destroy remainder of rows.
    foreach ($this->view->group as &$rows) {
      $is_first = NULL;
      foreach ($rows as $row_num => $row) {
        if (!isset($is_first)) {
          // Preserve only the first row.
          $is_first = $row_num;
        }
        else {
          // Remove row from group and View result set and destroy.
          unset($rows[$row_num]);
          unset($this->view->result[$row_num]);
        }
      }
    }
    // Write the raw results back into the View result and add footer values.
    $this->views_aggregator_set_aggregated_values($values);

    if (isset($this->active) && !empty($this->options['info'][$this->active]['has_aggr'])) {
      // Sort the table based on the selected sort column, i.e. $this->active.
      usort($this->view->group, array($this, '_views_aggregator_compare_rows'));
    }
  }

  /**
   * Write the aggregated results back into the View results and footer.
   *
   * @param array $values
   *   an array of column values, indexed by field name
   */
  private function views_aggregator_set_aggregated_values($values) {
    if (!empty($values)) {
      // Set the "total" cell at the bottom of the column.
      foreach ($this->options['info'] as $field_name => $options) {
        $field_handler = $this->view->field[$field_name];
        if (empty($field_handler->options['exclude'])) {
          if (isset($values[$field_name])) {
            views_aggregator_set_column($this->view, $field_handler, $values[$field_name]);
          }
          $this->view->totals[$field_name] = !empty($options['has_aggr_column']) && isset($values[$field_name]['column']) ? $values[$field_name]['column'] : '';
        }
      }
    }
  }

  /**
   * Compare function for aggregated groups, for use in sorting functions.
   *
   * @param array $group1
   *   The first aggregated group of result rows.
   * @param array $group2
   *   The second aggregated group of result rows.
   *
   * @return int, -1, 0, 1
   *   The compare code indicating whether $group1 is smaller than, equal to or
   *   greater than $group2.
   */
  private function _views_aggregator_compare_rows($group1, $group2) {
    $field_handler = $this->view->field[$this->active];
    $cell1 = views_aggregator_get_cell($field_handler, reset($group1));
    $cell2 = views_aggregator_get_cell($field_handler, reset($group2));
    $compare = ($cell1 == $cell2) ? 0 : ($cell1 < $cell2 ? -1 : 1);
    return ($this->order == 'asc') ? $compare : -$compare;
  }

  /**
   * Records the "active" field, i.e. the column clicked to be sorted.
   *
   * Also records the sort order ('asc' or 'desc').
   * This is identical to views_plugin_style_table::build_sort_post(), except
   * for the last statement, which has a condition added.
   */
  public function build_sort_post() {
    if (!isset($_GET['order'])) {
      // Check for a 'default' clicksort. If there isn't one, exit gracefully.
      if (empty($this->options['default'])) {
        return;
      }
      $sort = $this->options['default'];
      if (!empty($this->options['info'][$sort]['default_sort_order'])) {
        $this->order = $this->options['info'][$sort]['default_sort_order'];
      }
      else {
        $this->order = !empty($this->options['order']) ? $this->options['order'] : 'asc';
      }
    }
    else {
      $sort = $_GET['order'];
      // Store the $order for later use.
      $this->order = !empty($_GET['sort']) ? strtolower($_GET['sort']) : 'asc';
    }
    // If a sort we don't know about gets through, exit gracefully.
    if (empty($this->view->field[$sort])) {
      return;
    }
    // Ensure $this->order is valid.
    if ($this->order != 'asc' && $this->order != 'desc') {
      $this->order = 'asc';
    }
    // Store the $sort for later use.
    $this->active = $sort;

    // Tell the field to click sort, but only if it is not aggregated, in
    // which case sorting will be dealt with in $this->pre_render().
    // This is here predominantly to avoid notices from ViewsPHP, but also
    // makes normal column sorting more efficient by not adding any unnecessary
    // WHERE-clauses.
    if (empty($this->options['info'][$sort]['has_aggr'])) {
      $this->view->field[$sort]->click_sort($this->order);
    }
  }

}

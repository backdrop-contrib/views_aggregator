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
    $options['totals_per_page'] = array('default' => TRUE);
    $options['totals_row_position'] = array('default' => array(1 => 0, 2 => 2));
    return $options;
  }

  /**
   * Create the tabular form with the aggregation options.
   */
  public function options_form(&$form, &$form_state) {

    $handlers = $this->display->handler->get_handlers('field');
    $columns = $this->sanitize_columns($this->options['columns']);

    foreach ($columns as $field => $column) {
      if ($field == $column) {
        // Make all columns potentially sortable, including Math Expressions.
        // Do this before parent::options_form($form, $form_state);
        $handlers[$field]->definition['click sortable'] = TRUE;
      }
    }
    // Note: bulk of form is provided by superclass views_plugin_style_table.
    parent::options_form($form, $form_state);

    // See function views_aggregator_theme().
    $form['#theme'] = 'views_aggregator_plugin_style_table';

    // Views style of grouping (splitting table into many) interferes, so
    // get rid of the form.
    unset($form['grouping']);

    $form['description_markup'] = array(
      '#markup' => '<div class="description form-item">' . t('Column aggregation functions may be enabled independently of group aggregation functions. Every group aggregation function, except <em>Filter rows (by regexp)</em>, requires exactly <strong>one</strong> field to be assigned the <em>Group and compress</em> function. With that done, select any of the other aggregation functions for some or all of the fields. Functions marked with an asterisk take an optional parameter. <br/>You may combine multiple fields into the same column. If you do, the separator specified will be used to separate the fields. You may control column order and field labels in the Fields section of the main configuration page. For column aggregation <em>Math expressions</em> you may use the tokens from that same page as documented in the "Replacement patterns" in the section "Rewrite the output of this field".') . '</div>',
    );

    foreach ($columns as $field => $column) {

      $form['info'][$field]['has_aggr'] = array(
        '#type' => 'checkbox',
        '#title' => t('Apply group function'),
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
            //'select[name="style_options[info][' . $field . '][aggr][]"]' => array(
            // 'value' => array('views_aggregator_sum') DOES NOT WORK
            //),
          ),
        ),
      );

      $form['info'][$field]['has_aggr_column'] = array(
        '#type' => 'checkbox',
        '#title' => t('Apply column function'),
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
          ),
        ),
      );
    }
    $form['totals_per_page'] = array(
      '#title' => t('Column aggregation row applies to:'),
      '#type' => 'radios',
      '#options' => array(
        1 => t('the page shown, if a pager is enabled'),
        0 => t('the entire result set, regardless of the page shown'),
      ),
      '#description' => t('If your View does not have a pager, then the two options are equivalent.'),
      '#default_value' => $this->options['totals_per_page'],
      '#weight' => -1,
    );
    $form['totals_row_position'] = array(
      '#title' => t('Column aggregation row position'),
      '#type' => 'checkboxes',
      '#options' => array(
        1 => t('in the table header'),
        2 => t('in the table footer'),
      ),
      '#default_value' => $this->options['totals_row_position'],
      '#weight' => -1,
    );
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
        if (in_array('views_aggregator_group_and_compress', $options['aggr'])) {
          $num_grouped++;
        }
        elseif (!in_array('views_aggregator_row_filter', $options['aggr'])) {
          $num_aggregation_functions += count($options['aggr']);
        }
      }
    }
    // When we have no aggregation functions, we must have 0 or 1 grouping
    // function. When we have aggregation functions, there must be 1 grouping.
    $ok = ($num_aggregation_functions == 0) ? $num_grouped <= 1 : $num_grouped == 1;
    if (!$ok) {
      $msg = t('When applying group aggregation functions, you must also select <em>"Group and compress"</em> on exactly one field.');
      foreach ($form_state['values']['style_options']['info'] as $field_name => $options) {
        form_error($form['info'][$field_name]['aggr'], $msg);
        $msg = '';
      }
    }
  }

  /*
   * Overrides pre_render().
   *
   * @param array $results
   *   the results returned from the database query
   *
   * Note that this class being a views_plugin, rather than a views_handler,
   * it does not have a post_execute() function.
   *
   * This function applies to the currently visible page only. If paging is
   * enabled for this display view->result may only contain part of the entire
   * result set.
   */
  public function pre_render($results) {
    if (!empty($this->view->is_temp_views_aggregator)) {
      return;
    }
    parent::pre_render($results);

    if (empty($this->view->result)) {
      return;
    }
    $functions = $this->collect_aggregation_functions();

    $show_global_totals_with_pager = empty($this->options['totals_per_page']) && !empty($this->view->total_rows);

    if ($show_global_totals_with_pager) {
      $view_without_pager = $this->execute_view_without_pager($this->view, $this->view->current_display);

      // First apply the row filters (if any), then aggregate the columns.
      $view_without_pager->style_plugin->apply_row_filters();
      // Only interested in column aggregation, so only 'column' group needed.
      $column_group = array('column' => array());
      foreach ($view_without_pager->result as $num => $row) {
        $column_group['column'][$num] = $row;
      }
      $totals = $view_without_pager->style_plugin->execute_aggregation_functions($column_group, $functions);
      $this->view->totals = $this->set_totals_row($totals);
    }
    // Because we are going to need the View results AFTER token replacement,
    // we render the result set here. This is NOT duplication of CPU time,
    // because render_fields(), if called for a second time, will do nothing
    // when $this->rendered_fields has been populated already.
    // render_fields() will puts currency signs in front of moneys, embeds node
    // and taxonomy term references in hyperlinks etc.
    $this->render_fields($results);

    // Apply the row filters first, then aggregate the groups.
    $this->apply_row_filters();
    $groups = $this->aggregate_groups();
    $values = $this->execute_aggregation_functions($groups, $functions);

    unset($groups['column']);

    // Write group aggregation results into the View results.
    $this->set_aggregated_group_values($groups, $values);
    if (empty($this->view->totals)) {
      // If not already set above, write the column aggregation result row on
      // the View object. This row will be rendered via
      // template_preprocess_views_aggregator_results_table().
      $this->view->totals = $this->set_totals_row($values);
    }

    // With the aggregation functions now complete, destroy rows not part of the
    // aggregation.
    $this->compress_grouped_results($groups);

    // Sort the table based on the selected sort column, i.e. $this->active.
    if (isset($this->active)) {
      // To aid in sorting, add the row's index to each row object.
      foreach ($this->view->result as $num => $row) {
        $this->view->result[$num]->num = $num;
      }
      uasort($this->view->result, array($this, 'compare_result_rows'));
    }
  }

  /**
   * Filters out rows from the table based on a field cell matching a regexp.
   */
  protected function apply_row_filters() {
    $field_handlers = $this->view->field;
    foreach ($this->options['info'] as $field_name => $options) {
      if (!empty($options['has_aggr']) && in_array('views_aggregator_row_filter', $options['aggr'])) {
        views_aggregator_row_filter($this, $field_handlers[$field_name], $options['aggr_par']);
      }
    }
  }

  /**
   * Aggregate and compress the View's rows into groups.
   *
   * @return array
   *   an array of aggregated groups
   */
  protected function aggregate_groups() {
    $field_handlers = $this->view->field;
    // Find the one column to group by and execute the grouping.
    foreach ($this->options['info'] as $field_name => $options) {
      if (!empty($options['has_aggr']) && in_array('views_aggregator_group_and_compress', $options['aggr'])) {
        $groups = views_aggregator_group_and_compress($this->view->result, $field_handlers[$field_name]);
        break;
      }
    }
    if (empty($groups)) {
      // If there are no regular groups, create a special group for column
      // aggregation. This group holds all View result rows.
      foreach ($this->view->result as $num => $row) {
        $groups['column'][$num] = $row;
      }
    }
    return $groups;
  }

  /**
   * Collect the aggregation functions from the Views UI.
   *
   * @return array functions
   */
  protected function collect_aggregation_functions() {
    $functions = array();
    foreach ($this->options['info'] as $field_name => $options) {
      // Make a list of the group and column functions to call for this field.
      if (!empty($options['has_aggr'])) {
        foreach ($options['aggr'] as $function) {
          if ($function != 'views_aggregator_row_filter' && $function != 'views_aggregator_group_and_compress') {
            if (empty($functions[$field_name]) || !in_array($function, $functions[$field_name])) {
              $functions[$field_name][] = $function;
            }
          }
        }
      }
      // Column aggregation function, if requested, is last.
      if (!empty($options['has_aggr_column'])) {
        $function = $options['aggr_column'];
        if (empty($functions[$field_name]) || !in_array($function, $functions[$field_name])) {
          $functions[$field_name][] = $function;
        }
      }
    }
    return $functions;
  }

  /**
   * Executes the supplied aggregation functions with the groups as arguments.
   *
   * @param array $groups
   * @param array $functions
   *
   * @return array of function return values
   */
  protected function execute_aggregation_functions($groups, $functions) {
    $field_handlers = $this->view->field;
    $values = array();
    foreach ($functions as $field_name => $field_functions) {
      if (empty($field_handlers[$field_name])) {
        continue;
      }
      $options = $this->options['info'][$field_name];
      foreach ($field_functions as $function) {
        $group_par = (!isset($options['aggr_par']) || $options['aggr_par'] == '') ? NULL : $options['aggr_par'];
        $column_par = (!isset($options['aggr_par_column']) || $options['aggr_par_column'] == '') ? NULL : $options['aggr_par_column'];
        $aggr_values = $function($groups, $field_handlers[$field_name], $group_par, $column_par);
        // $aggr_values is indexed by group value and/or 'column'.
        // 'column' is the last evaluated value for the field.
        if (isset($aggr_values['column'])) {
          $field_handlers[$field_name]->last_render = $aggr_values['column'];
        }
        foreach ($aggr_values as $group => $value) {
          // 'column' function is last so may override earlier value.
          if (!isset($values[$field_name][$group]) || $group == 'column') {
            $values[$field_name][$group] = $value;
          }
        }
      }
    }
    return $values;
  }

  /**
   * Removes no longer needed View result rows from the set.
   *
   * @param type $groups
   */
  protected function compress_grouped_results($groups) {
    foreach ($groups as $rows) {
      $is_first = TRUE;
      foreach ($rows as $num => $row) {
        // The aggregated row is the first of each group. Destroy the others.
        if (!$is_first) {
          unset($this->rendered_fields[$num]);
          unset($this->view->result[$num]);
        }
        $is_first = FALSE;
      }
    }
  }

  /**
   * Returns the raw or rendered result at the intersection of column and row.
   *
   * @param object $field_handler
   *   The handler associated with the result column being requested.
   * @param int $row_num
   *   The result row number.
   * @param bool $render
   *   Whether the rendered or raw value should be returned.
   *
   * @return string
   *  Returns empty string if there are no results for the requested row_num.
   */
  public function get_cell($field_handler, $row_num, $render) {
    $field_name = $field_handler->options['id'];
    if (isset($this->rendered_fields[$row_num][$field_name])) {
      // Bit of a hack for "Webform submission data: Value(...)" and 
      // "Global: Math expression" fields... Always pick up the rendered fields,
      // as it seems that's all we can get!
      if ($render || is_a($field_handler, 'webform_handler_field_submission_data')) {
        return $this->rendered_fields[$row_num][$field_name];
      }
      if (is_a($field_handler, 'views_handler_field_math')) {
        // Ignore non-numeric leading characters like currency signs.
        return vap_num($this->rendered_fields[$row_num][$field_name]);
      }
    }
    if (!isset($field_handler->view->result[$row_num])) {
      return '';
    }
    $field_handler->view->row_index = $row_num;
    return $this->get_cell_raw($field_handler, $field_handler->view->result[$row_num], TRUE);
  }

  /**
   * Returns the raw, unrendered result at the intersection of column and row.
   *
   * Should normally not be called, especially not for Math Expr. fields.
   *
   * @param object $field_handler
   *   The handler associated with the result column being requested.
   * @param object $result_row
   *   The result row.
   * @param bool $compressed
   *   If the result is a (nested) array, return the first primitive value.
   *
   * @return string
   *   the raw contents of the cell
   */
  private function get_cell_raw($field_handler, $result_row, $compressed = TRUE) {
    // If the field_handler is views_handler_field_field, get_value() may start
    // with cloning the entity pointed to by _field_data, most commonly:
    // $result_row->_field_data['nid']['entity']
    $value = $field_handler->get_value($result_row);
    if (!isset($value)) {
      // This may happen with a $field_handler like views_php_handler_field that
      // doesn't have its value available until after rendering. Render it here.
      $value = $field_handler->render($result_row);
    }
    // get_value($result_row) may produce values like $value[0]['tid'] == 37
    // or may have multiple subvalues like AddressFields:
    // $value[0]['country'] == 'AU'
    // $value[0]['postal_code'] = '3040' etc.
    //
    if ($compressed && is_array($value)) {
      $value = reset($value);
      if (is_array($value)) {
        $value = reset($value);
      }
    }
    return $value;
  }

  /**
   * Sets a value on the cell identified by the supplied field and row.
   *
   * @param object $field_handler
   *   The handler associated with the table column being requested.
   * @param int $row_num
   *   The result row number.
   * @param mixed $new_value
   *   The value to set. This should be the raw value, otherwise sorting may not
   *   work properly.
   *
   * @return mixed
   *   FALSE if the value is for a field that could not be set on the result
   *   set. Note that even in that case it will still show up correctly,
   *   however sorting, may not work.
   *   Node properties (e.g., title) and PHP expression values can always be
   *   set successfully.
   */
  protected function set_cell($field_handler, $row_num, $new_value) {
    $field_alias = $field_handler->field_alias;
    $result_row = $field_handler->view->result[$row_num];
    $is_set = TRUE;

    // Deal here with "No results" behaviour, in particular for Math Expr. ?

    if (is_a($field_handler, 'views_php_handler_field')) {
      if (isset($result_row->{$field_alias})) {
        $result_row->{$field_alias} = $new_value;
        // This prevents Views PHP from re-rendering the code snippet and makes
        // it pick up the value from $result_row.
        $field_handler->options['php_output'] = FALSE;
      }
    }
    elseif (empty($field_handler->field_alias) || $field_handler->field_alias == 'unknown' ||
      is_a($field_handler, 'webform_handler_field_submission_data')) {
      // Cannot use render_from_raw() due to missing or wrong field_handler->
      //   field_alias (i.e. Webform sid).
      // Set last_render as render_text() below relies on it.
      $field_handler->last_render = $this->format_numeric($field_handler, $new_value);
      $alter = array('phase' => VIEWS_HANDLER_RENDER_TEXT_PHASE_COMPLETELY) + $field_handler->options['alter'];
      // Try to avoid double rewriting in Tallies and Enumerations. This is
      // a poor heuristic approach. @todo
      if (strpos($new_value, '<br/>')) {
        unset($alter['alter_text']);
      }
      $field_handler->view->row_index = $row_num;
      $new_value = $field_handler->render_text($alter);
    }
    elseif (is_a($field_handler, 'views_handler_field_field')) {
      if ($rendered_value = $this->render_from_raw($field_handler, $row_num, $new_value)) {
        $new_value = $rendered_value;
      }
      else {
        $is_set = FALSE;
      }
    }
    elseif (isset($result_row->{$field_alias})) {
      // Not a field, but node property or Webform sid. Example: 'node_title'.
      $result_row->{$field_alias} = $new_value;
    }
    $field_name = $field_handler->options['id'];
    $this->rendered_fields[$row_num][$field_name] = $new_value;
    return $is_set ? $new_value : FALSE;
  }

  /**
   * Format a raw numeric value according to the supplied handler settings.
   *
   * @param object $field_handler
   * @param double $raw_value
   *
   * @return string, number formatted according to Views handler settings
   *
   * Note: this was taken in part from views_handler_field_math::render($values)
   */
  private function format_numeric($field_handler, $raw_value) {
    if (!empty($field_handler->options['set_precision'])) {
      $value = number_format($raw_value, $field_handler->options['precision'], $field_handler->options['decimal'], $field_handler->options['separator']);
    }
    elseif (isset($field_handler->options['separator'])) {
      $remainder = abs($raw_value) - intval(abs($raw_value));
      $value = $raw_value > 0 ? floor($raw_value) : ceil($raw_value);
      $value = number_format($value, 0, '', $field_handler->options['separator']);
      if ($remainder && isset($field_handler->options['decimal'])) {
        // The substr may not be locale safe.
        $value .= $field_handler->options['decimal'] . substr($remainder, 2);
      }
    }
    else {
      $value = $raw_value;
    }
    // Check to see if hiding should happen.
    if ($field_handler->options['hide_empty'] && empty($value) && ($value !== 0 || $field_handler->options['empty_zero'])) {
      return '';
    }
    // Should we format as a plural.
    if (!empty($field_handler->options['format_plural']) && ($value != 0 || !$field_handler->options['empty_zero'])) {
      $value = format_plural($value, $field_handler->options['format_plural_singular'], $field_handler->options['format_plural_plural']);
    }
    if (empty($value)) {
      return '';
    }
    $prefix = isset($field_handler->options['prefix']) ? $field_handler->options['prefix'] : '';
    $suffix = isset($field_handler->options['suffix']) ? $field_handler->options['suffix'] : '';
    return $field_handler->sanitize_value($prefix . $value . $suffix, 'xss_admin');
  }

  /**
   * Render a field from a raw value, with or without affecting the View result.
   *
   * The field will be rendered with appropriate CSS classes, without label.
   *
   * NB: This is messy code. The lengths we have to go through for this are just
   * ridiculous. Patches welcome!
   *
   * @param object $field_handler
   *   The views_handler_field_field object belonging to the View result field
   * @param int $row_num
   *   The view result row number to change. Pass NULL to simply render $raw_value
   *   outside the context of a View, without affecting any rows.
   * @param mixed $raw_value
   *   Compound or simple value. If NULL the field is re-rendered using its
   *   current (raw) value.
   *
   * @return string
   *   The rendered value or FALSE, if the type of field is not supported.
   */
  private function render_from_raw($field_handler, $row_num = NULL, $raw_value = NULL) {
    $field_name = $field_handler->options['id'];
    $field_alias = $field_handler->field_alias;
    $row = isset($row_num) ? $field_handler->view->result[$row_num] : reset($field_handler->view->result);
    $affect_view = isset($row_num);
    if (!$row || empty($row->_field_data[$field_alias])) {
      // This happens for ViewsPHP fields for instance, and for Math Expressions.
      return !$row || $affect_view ? FALSE : (isset($raw_value) ? $raw_value : '?');
    }
    $_field_data = $row->_field_data[$field_alias];
    if (isset($_field_data['entity']->{$field_name}) && is_scalar($_field_data['entity']->{$field_name})) {
      // E.g. when the $field_handler refers to a node property (rather than
      // a field) that does not have a renderer.
      return $affect_view ? ($_field_data['entity']->{$field_name} = $raw_value) : $raw_value;
    }
    // Clone entity if we don't want to affect the current View results or if we
    // have multiple displays.
    $entity = $affect_view ? $_field_data['entity'] : clone $_field_data['entity'];
    $entity_type = $_field_data['entity_type'];

    $lang = is_a($field_handler, 'views_handler_field_field') ? $field_handler->field_language($entity_type, $entity) : $entity->language;
    if (isset($raw_value)) {
      // Only supporting values of 1 item, at index 0.
      if (is_array($raw_value)) {
        $entity->{$field_name}[$lang][0] = $raw_value;
      }
      elseif (isset($entity->{$field_name})) {
        if (empty($entity->{$field_name})) {
          // This occurs when the field is empty. May have the "No results text".
          //$prev_value = isset($field_handler->options['empty']) ? $field_handler->options['empty'] : NULL;
          // Cannot deal with string when expecting number.
          $current_value = NULL;
          $key = 'value';
        }
        else {
          $current_value = reset($entity->{$field_name}[$lang][0]);
          $key = key($entity->{$field_name}[$lang][0]);
          // Cannot override 'tid' with non-numeric value. But like 'value',
          // 'tid' may be set in case of min, max, most frequent etc.
          if ($key != 'value' && !($key == 'tid' && is_numeric($raw_value))) {
            return $affect_view ? FALSE : (isset($raw_value) ? $raw_value : $value);
          }
        }
        $entity->{$field_name}[$lang][0][$key] = $raw_value;
      }
      if ($affect_view) {
        // Next employ set_items() to re-render the $entity updated above,
        // placing both the 'raw' and 'rendered' versions in field_...[].
        $row->{'field_' . $field_name} = $field_handler->set_items($row, $row_num);
        // Now set the current value back in case we have multiple displays, all
        // drawing from the same entity.
        if (isset($key)) {
          $entity->{$field_name}[$lang][0][$key] = $current_value;
        }

        // The final step is to theme the rendered values.
        // An alternative for the line below is:
        //return $field_handler->advanced_render($row);
        return $field_handler->theme($row);
      }
    }
    $display = array(
      'type' => $field_handler->options['type'],
      'settings' => $field_handler->options['settings'],
      'label' => 'hidden',
    );
    $render_array = field_view_field($entity_type, $entity, $field_name, $display, $lang);
    return drupal_render($render_array);
  }

  /**
   * Write the aggregated results back into the View's rendered results.
   *
   * @param array $groups
   *   an array of groups, indexed by group name
   * @param array $values
   *   an array of value arrays, indexed by field name first and group second
   */
  protected function set_aggregated_group_values($groups, $values) {
    $field_handlers = $this->view->field;
    foreach ($this->options['info'] as $field_name => $options) {
      foreach ($groups as $group => $rows) {
        if ($group != 'column' && isset($values[$field_name][$group])) {
          foreach ($rows as $num => $row) {
            $this->set_cell($field_handlers[$field_name], $num, $values[$field_name][$group]);
            // Only need to set on the first member of the group.
            break;
          }
        }
      }
    }
  }

  /**
   * Write the aggregated results back into the View results totals (footer).
   *
   * @param array $values
   *   an array of field value arrays, indexed by field name and 'column'
   */
  protected function set_totals_row($values) {
    $totals = array();
    $dont_render_functions = array('views_aggregator_count', 'views_aggregator_range', 'views_aggregator_replace');
    foreach ($values as $field_name => $group) {
      if (!empty($this->options['info'][$field_name]['has_aggr_column']) && isset($group['column'])) {
        if (in_array($this->options['info'][$field_name]['aggr_column'], $dont_render_functions)) {
          $totals[$field_name] = $group['column'];
        }
        else {
          $field_handler = $this->view->field[$field_name];
          if (empty($field_handler->field_alias) || $field_handler->field_alias == 'unknown' ||
            is_a($field_handler, 'webform_handler_field_submission_data')) {
            // Cannot use render_from_raw() due to missing or wrong (i.e.
            // Webform sid) field_handler->field_alias.
            // Set last_render as render_text() below relies on it.
            $field_handler->last_render = $this->format_numeric($field_handler, $group['column']);
          }
          else {
            $field_handler->last_render= $this->render_from_raw($field_handler, NULL, $group['column']);
          }
          $alter = array('phase' => VIEWS_HANDLER_RENDER_TEXT_PHASE_COMPLETELY) + $field_handler->options['alter'];
          $totals[$field_name] = $field_handler->render_text($alter);
        }
      }
    }
    return $totals;
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
    // Store the $sort and sortable flag for later use.
    $this->active = $sort;
    //$this->sortable = $this->options['info'][$sort]['sortable'];

    // Tell the field to click-sort, but only if it is not a Math Expression or
    // a field not aggregated, in which cases sorting will be dealt with in
    // $this->pre_render().
    // This is here predominantly to avoid notices from ViewsPHP, but also
    // makes normal column sorting more efficient by not adding any unnecessary
    // WHERE-clauses.
    if (!is_a($this->view->field[$sort], 'views_handler_field_math') &&
      empty($this->options['info'][$sort]['has_aggr'])) {
      $this->view->field[$sort]->click_sort($this->order);
    }
  }

  /**
   * Compare function for aggregated groups, for use in sorting functions.
   *
   * @param array $row1
   *   The first aggregated group of result rows.
   * @param array $row2
   *   The second aggregated group of result rows.
   *
   * @return int
   *   The compare code indicating whether $row2 is smaller than (-1), equal
   *   to (0) or greater than (1) $row2.
   */
  protected function compare_result_rows($row1, $row2) {
    // The sorting data may be raw or rendered, while the sorting style may be
    // alphabetical or numeric.
    //
    // Columns that need to be sorted using raw values:
    // o numbers and moneys, so that "$1,000" comes AFTER "$9.99" (ascending)
    // o dates and date ranges (@todo)
    //
    // Columns that need to be sorted using rendered, post-aggregated values:
    // o Views PHP expressions, addresses, taxonomy terms

    $field_handler = $this->view->field[$this->active];
    $field_type = isset($field_handler->field_info['type']) ? $field_handler->field_info['type'] : '';
/*
    $options = $this->options['info'][$this->active];
    if (!empty($options['has_aggr'])) {
      $aggr_functions = $options['aggr'];
    }
*/
    // AddressFields, taxonomy terms and Views PHP expressions are compared in
    // rendered format.
    $compare_rendered =
       ($field_type == 'addressfield') ||
       ($field_type == 'taxonomy_term_reference') ||
       is_a($field_handler, 'views_php_handler_field');

    // Note that raw data may contain HTML tags too, so always strip.
    $cell1 = strip_tags($this->get_cell($field_handler, $row1->num, $compare_rendered));
    $cell2 = strip_tags($this->get_cell($field_handler, $row2->num, $compare_rendered));

    if ((double)$cell1 == (double)$cell2) {
      // If both cells cast to zero, then compare alphabetically.
      $compare = ($cell1 == $cell2) ? 0 : ($cell1 < $cell2 ? -1 : 1);
    }
    else {
      // Compare numerically, i.e. "20 km" comes after "9.5 km".
      // The double cast causes a readu up to the first non-number related char.
      $compare = (double)$cell1 < (double)$cell2 ? -1 : 1;
    }
    return ($this->order == 'asc') ? $compare : -$compare;
  }

  /**
   * Strips the pager off an existing View, then executes and renders it.
   *
   * The View is rebuilt from scratch, with the extra pager query. The View
   * passed in as an argument is in no way affected.
   *
   * @param object $view_with_pager
   *   a View object
   *
   * @param object $display_id
   *   the display to execute, for example 'master', 'page', 'block'
   *
   * @return object
   *   the pageless View, including the complete rendered results set on
   *   $view->style_plugin->rendered_fields
   */
  private function execute_view_without_pager($view_with_pager, $display_id = NULL) {
    // This call exports the Views configuration, without any results attached.
    // We're doing it this way, because none of the below work after clone_view().
    // $view_without_pager->display['default']->handler->options['pager']['type'] = 'none';
    // $view_without_pager->display[$display_id]->handler->options['pager']['type'] = 'none';
    // $view_without_pager->display_handler->default_display->options['pager']['type'] = 'none';
    // The code below is based on view::copy()
    $code = $view_with_pager->export();
    $code_without_pagers = str_replace(
      "display_options['pager']['type'] = ",
      "display_options['pager']['type'] = 'none'; //", $code, $num_pagers);

    // This statement creates a View object by the name of $view.
    eval($code_without_pagers);

    // [#2213417], to avoid 2nd notice.
    $view->dom_id = $view_with_pager->dom_id;
    
    // Avoid recursion in views_aggregator_plugin_style_table::pre_render().
    $view->is_temp_views_aggregator = TRUE;

    // As this is a copy, let's keep caching behaviour the same as the original.
    // Execute the display and render it.
    $view->render($display_id);

    return $view;
  }

}

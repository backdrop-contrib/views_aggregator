<?php
/**
 * @file
 * theme_views_aggregator_plugin_style_table.inc
 *
 * Themes the View UI created in the class views_aggregator_plugin_style_table.
 * Based on similar functions in both Views and Views Calc modules.
 */

/**
 * Theme the form for the table style plugin.
 */
function theme_views_aggregator_plugin_style_table($variables) {
  $form = $variables['form'];

  $output = backdrop_render($form['description_markup']);

  // Note these are currently in the Views Calc rather than Views table order.
  $header = array(
    t('Field'),
    t('Align'),
    t('Group aggregation'),
    t('Column aggregation'),
    t('Render column'),
    t('Separator'),
    array(
      'data' => t('Sortable'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default sort'),
      'align' => 'center',
    ),
    array(
      'data' => t('Default order'),
      'align' => 'center',
    ),
    array(
      'data' => t('Hide empty column'),
      'align' => 'center',
    ),
  );

  $rows = array();
  foreach (element_children($form['columns']) as $id) {
    $row = array();
    $row[] = backdrop_render($form['info'][$id]['name']);
    $row[] = backdrop_render($form['info'][$id]['align']);
    $row[]
      = backdrop_render($form['info'][$id]['has_aggr'])
      . backdrop_render($form['info'][$id]['aggr'])
      . backdrop_render($form['info'][$id]['aggr_par']);
    $row[]
      = backdrop_render($form['info'][$id]['has_aggr_column'])
      . backdrop_render($form['info'][$id]['aggr_column'])
      . backdrop_render($form['info'][$id]['aggr_par_column']);
    $row[] = backdrop_render($form['columns'][$id]);
    $row[] = backdrop_render($form['info'][$id]['separator']);
    if (!empty($form['info'][$id]['sortable'])) {
      $row[] = array(
        'data' => backdrop_render($form['info'][$id]['sortable']),
        'align' => 'center',
      );
      $row[] = array(
        'data' => backdrop_render($form['default'][$id]),
        'align' => 'center',
      );
      $row[] = array(
        'data' => backdrop_render($form['info'][$id]['default_sort_order']),
        'align' => 'center',
      );
    }
    else {
      $row[] = '';
      $row[] = '';
      $row[] = '';
    }
    $row[] = array(
      'data' => backdrop_render($form['info'][$id]['empty_column']),
      'align' => 'center',
    );
    $rows[] = $row;
  }
  // Add the special 'None' row.
  $rows[] = array(
    t('None'),
    '',
    '',
    '',
    '',
    '',
    '',
    array(
      'align' => 'center',
      'data' => backdrop_render($form['default'][-1]),
    ),
    '',
    '',
  );
  $output .= theme('table', array('header' => $header, 'rows' => $rows));
  $output .= backdrop_render_children($form);
  return $output;
}

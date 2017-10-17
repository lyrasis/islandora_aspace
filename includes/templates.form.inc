<?php

/**
 * @file
 * This file contains the template settings forms.
 */

/**
 * Template list table.
 */
function islandora_aspace_templates() {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');

  $table = array(
    'header' => array(
      array(
        'data' => t('Title'),
      ),
      array(
        'data' => t('Machine Name'),
      ),
      array(
        'data' => t('Type'),
      ),
      array(
        'data' => t('Operations'),
        'colspan' => 3,
      ),
    ),
    'rows' => array(),
  );

  $hook_templates = islandora_aspace_hook_template_list();

  foreach ($hook_templates as $machine_name => $value) {
    $table['rows'][] = array(
      $value,
      $machine_name,
      t('Built-in'),
      l(t('Copy'), "admin/islandora/tools/aspace/templates/copy/$machine_name"),
      '',
      '',
    );
  }

  $variable_templates = IslandoraAspaceVariableTwigLoader::getAll();

  foreach ($variable_templates as $machine_name => $info) {
    $table['rows'][] = array(
      $info['name'],
      $machine_name,
      t('Custom'),
      l(t('Copy'), "admin/islandora/tools/aspace/templates/copy/$machine_name"),
      l(t('Edit'), "admin/islandora/tools/aspace/templates/edit/$machine_name"),
      l(t('Delete'), "admin/islandora/tools/aspace/templates/delete/$machine_name"),
    );
  }

  return theme('table', $table);
}

/**
 * Form for creating and editing templates.
 */
function islandora_aspace_template_create_form(array $form, array &$form_state, $machine_name = NULL) {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');
  $info = IslandoraAspaceVariableTwigLoader::get($machine_name);

  $form['new'] = array(
    '#type' => 'hidden',
    '#value' => $info === FALSE,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $info !== FALSE ? $info['name'] : NULL,
    '#disabled' => $info !== FALSE,
  );

  $form['source'] = array(
    '#type' => 'textarea',
    '#title' => t('Template'),
    '#default_value' => $info !== FALSE && !isset($form_state['values']) ? $info['source'] : NULL,
  );

  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Test Template'),
    '#description' => t('Test the template against an existing object.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['test']['pid'] = array(
    '#type' => 'textfield',
    '#title' => t('PID'),
  );

  $form['test']['test'] = array(
    '#type' => 'button',
    '#value' => t('Test'),
    '#validate' => array(
      'islandora_aspace_template_create_form_validate',
      'islandora_aspace_template_create_form_test',
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Validation function for template creation form.
 */
function islandora_aspace_template_create_form_validate(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');

  $machine_name = islandora_aspace_template_machine_name($form_state['values']['name']);
  $loader = new IslandoraAspaceVariableTwigLoader();
  $new = $form_state['values']['new'];

  if ($new && ($loader->exists($machine_name) || $form_state['values']['name'] == '')) {
    form_set_error('name', "Template name already exists.");
  }

  $twig = islandora_aspace_twig();
  try {
    $twig->parse($twig->tokenize(new Twig_Source($form_state['values']['source'], $machine_name)));
  }
  catch (Twig_Error_Syntax $e) {
    form_set_error('source', "Template syntax error. <br>" . $e->getMessage());
  }
}

/**
 * Submission function for template creation form.
 */
function islandora_aspace_template_create_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');
  $display_name = $form_state['values']['name'];
  $machine_name = islandora_aspace_template_machine_name($display_name);
  $source = $form_state['values']['source'];
  IslandoraAspaceVariableTwigLoader::set($machine_name, $display_name, $source);
  $form_state['redirect'] = 'admin/islandora/tools/aspace/templates';
}

/**
 * Form to deleting templates.
 */
function islandora_aspace_template_delete_form(array $form, array &$form_state, $machine_name) {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');
  $loader = new IslandoraAspaceVariableTwigLoader();
  if (!$loader->exists($machine_name)) {
    drupal_set_message(t('Template "%name" does not exist.', array('%name' => $machine_name)), 'error');
    drupal_not_found();
    exit();
  }

  $info = $loader::get($machine_name);

  $form['machine_name'] = array(
    '#type' => 'hidden',
    '#value' => $machine_name,
  );

  $form['description'] = array(
    '#type' => 'markup',
    '#prefix' => '<div>',
    '#markup' => t(
      'Are you sure you want to delete template <strong>%name</strong>? This action is irreversible.',
        array('%name' => $info['name'])
    ),
    '#suffix' => '</div>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

/**
 * Handler that prints information about templates for testing.
 */
function islandora_aspace_template_create_form_test(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  $pid = $form_state['values']['pid'];
  $source = $form_state['values']['source'];
  $object = islandora_object_load($pid);
  if (empty($object)) {
    drupal_set_message(t('PID "%pid" does not exist.', array('%pid' => $pid)), 'error');
    return;
  }
  if (!in_array(ISLANDORA_ASPACE_OBJECT_CMODEL, $object->models)) {
    drupal_set_message(t('PID "%pid" is not an ArchivesSpace object.', array('%pid' => $pid)), 'error');
    return;
  }

  $json = $object[ISLANDORA_ASPACE_DSID]->content;
  $aspace_data = json_decode($json, TRUE);
  $twig = islandora_aspace_twig();

  try {
    $template = $twig->createTemplate($source);
    drupal_set_message(t('<pre>@data</pre>', array('data' => $template->render($aspace_data))), 'status');
  }
  catch (Twig_Error $exception) {
    drupal_set_message(t('@error' array('@error' => $exception->getMessage())), 'error');
  }

}

/**
 * Submit function to delete templates.
 */
function islandora_aspace_template_delete_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/twig_variable_loader');
  $machine_name = $form_state['values']['machine_name'];
  IslandoraAspaceVariableTwigLoader::remove($machine_name);
  $form_state['redirect'] = 'admin/islandora/tools/aspace/templates';
}

/**
 * Form for copying templates.
 */
function islandora_aspace_template_copy_form(array $form, array &$form_state, $machine_name) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  $twig = islandora_aspace_twig();
  $loader = $twig->getLoader();
  if (!$loader->exists($machine_name)) {
    drupal_set_message(t('Template "%name" does not exist.', array('%name' => $machine_name)), 'error');
    drupal_not_found();
    exit();
  }

  $form['machine_name'] = array(
    '#type' => 'hidden',
    '#value' => $machine_name,
  );

  $form['new'] = array(
    '#type' => 'hidden',
    '#value' => TRUE,
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => 'Name for new template',
  );

  $form['source'] = array(
    '#type' => 'hidden',
    '#value' => $loader->getSourceContext($machine_name)->getCode(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  $form['#validate'] = array('islandora_aspace_template_create_form_validate');
  $form['#submit'] = array('islandora_aspace_template_create_form_submit');

  return $form;
}

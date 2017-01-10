<?php

/**
 * @file
 * This file contains forms and callbacks used in the ingest hooks.
 */


/**
 * Form to select if the integration should be used for a particular object.
 */
function islandora_archivespace_step_enable_form(array $form, array &$form_state) {
  $form = array(
    'islandora_archivespace_enable' => array(
      '#type' => 'radios',
      '#title' => t('Will this object be described in Archivespace?'),
      '#description' => t('If this option is selected a digital object will be creted in archivespace for this object and metadata will be updated from there.'),
      '#options' => array(
        TRUE => t('Yes'),
        FALSE => t('No'),
      ),
      '#required' => TRUE,
    ),
  );

  if(isset($form_state['islandora']['step_storage']['islandora_archivespace_step_enable_form']['islandora_archivespace_enable'])) {
    $form['islandora_archivespace_enable']['#default_value'] = $form_state['islandora']['step_storage']['islandora_archivespace_step_enable_form']['islandora_archivespace_enable'];
  }

  return $form;
}

/**
 * Enable form submit handler.
 */
function islandora_archivespace_step_enable_form_submit(array $form, array &$form_state) {
  $form_state['islandora']['step_storage']['islandora_archivespace_step_enable_form']['islandora_archivespace_enable'] = $form_state['values']['islandora_archivespace_enable'];
}

/**
 * Simple metadata form for archivespace.
 */
function islandora_archivespace_step_metadata_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_archivespace', 'includes/utilities');
  $repositories = islandora_archivespace_get_repositories();
  $options = array();
  foreach($repositories as $repository) {
    $options[$repository->uri] = $repository->name;
  }

  $form = array();

  $form['archivespace_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object Description'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['archivespace_metadata']['islandora_archivespace_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
  );

  $form['archivespace_metadata']['islandora_archivespace_identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#required' => TRUE,
  );

  $form['islandora_archivespace_repository'] = array(
    '#type' => 'select',
    '#title' => t('Select Repository'),
    '#description' => t('Choose which Archivespace repository to add this object to.'),
    '#options' => $options,
  );

  if(isset($form_state['islandora']['step_storage']['islandora_archivespace_step_metadata_form'])) {
    $storage = $form_state['islandora']['step_storage']['islandora_archivespace_step_metadata_form'];

    if(isset($storage['islandora_archivespace_title'])) {
      $form['archivespace_metadata']['islandora_archivespace_title']['#default_value'] = $storage['islandora_archivespace_title'];
    }

    if(isset($storage['islandora_archivespace_identifier'])) {
      $form['archivespace_metadata']['islandora_archivespace_identifier']['#default_value'] = $storage['islandora_archivespace_identifier'];
    }

    if(isset($storage['islandora_archivespace_repository'])) {
      $form['islandora_archivespace_repository']['#default_value'] = $storage['islandora_archivespace_repository'];
    }
  }

  return $form;
}

/**
 * Simple metadata form submit handler
 */
function islandora_archivespace_step_metadata_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_archivespace', 'includes/utilities');

  $storage = &$form_state['islandora']['step_storage']['islandora_archivespace_step_metadata_form'];
  $storage['islandora_archivespace_title'] = $form_state['values']['islandora_archivespace_title'];
  $storage['islandora_archivespace_identifier'] = $form_state['values']['islandora_archivespace_identifier'];
  $storage['islandora_archivespace_repository'] = $form_state['values']['islandora_archivespace_repository'];

  // maybe should happen after ingest...
  islandora_archivespace_create_digital_object($storage['islandora_archivespace_title'], $storage['islandora_archivespace_identifier'], $storage['islandora_archivespace_repository'], $form_state['islandora']['objects'][0]->id);

  $form_state['islandora']['objects'][0]->label = $storage['islandora_archivespace_title'];
}

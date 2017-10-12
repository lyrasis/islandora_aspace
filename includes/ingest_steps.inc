<?php

/**
 * @file
 * This file contains forms and callbacks used in the ingest hooks.
 */

/**
 * Form to select if the integration should be used for a particular object.
 */
function islandora_aspace_step_enable_form(array $form, array &$form_state) {
  $form = array(
    'islandora_aspace_enable' => array(
      '#type' => 'radios',
      '#title' => t('Will this object be described in ArchivesSpace?'),
      '#description' => t('If this option is selected a digital object will be creted in archivesspace for this object and metadata will be updated from there.'),
      '#options' => array(
        TRUE => t('Yes'),
        FALSE => t('No'),
      ),
      '#required' => TRUE,
    ),
  );

  if (isset($form_state['islandora']['step_storage']['islandora_aspace_step_enable_form']['islandora_aspace_enable'])) {
    $form['islandora_aspace_enable']['#default_value'] = $form_state['islandora']['step_storage']['islandora_aspace_step_enable_form']['islandora_aspace_enable'];
  }

  return $form;
}

/**
 * ArchivesSpace enable form submit handler.
 */
function islandora_aspace_step_enable_form_submit(array $form, array &$form_state) {
  $form_state['islandora']['step_storage']['islandora_aspace_step_enable_form']['islandora_aspace_enable'] = $form_state['values']['islandora_aspace_enable'];
}

/**
 * Simple metadata form for archivesspace.
 */
function islandora_aspace_step_metadata_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');
  $repositories = islandora_aspace_get_repositories();
  $options = array();
  if ($repositories !== FALSE) {
    foreach ($repositories as $repository) {
      $options[$repository->uri] = $repository->name;
    }
  }
  else {
    drupal_set_message(t('Error contacting ArchivesSpace server. Please try again later.'), 'error');
  }

  $form = array();

  $form['archivesspace_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object Description'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['archivesspace_metadata']['islandora_aspace_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
  );

  $form['archivesspace_metadata']['islandora_aspace_identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#required' => TRUE,
  );

  $form['archivesspace_metadata']['islandora_aspace_published'] = array(
    '#type' => 'checkbox',
    '#title' => t('Publish'),
    '#description' => 'Should the item be published on ingest?',
  );

  $form['islandora_aspace_repository'] = array(
    '#type' => 'select',
    '#title' => t('Select Repository'),
    '#description' => t('Choose which ArchivesSpace repository to add this object to.'),
    '#options' => $options,
    '#required' => TRUE,
  );

  $storage = &islandora_ingest_form_get_step_storage($form_state);
  $metadata = &$form['archivesspace_metadata'];

  if (isset($storage['islandora_aspace_title'])) {
    $metadata['islandora_aspace_title']['#default_value'] = $storage['islandora_aspace_title'];
  }

  if (isset($storage['islandora_aspace_identifier'])) {
    $metadata['islandora_aspace_identifier']['#default_value'] = $storage['islandora_aspace_identifier'];
  }

  if (isset($storage['islandora_aspace_published'])) {
    $metadata['islandora_aspace_published']['#default_value'] = $storage['islandora_aspace_published'];
  }

  if (isset($storage['islandora_aspace_repository'])) {
    $form['islandora_aspace_repository']['#default_value'] = $storage['islandora_aspace_repository'];
  }

  return $form;
}

/**
 * Metadata form validate handler.
 */
function islandora_aspace_step_metadata_form_validate(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');

  $title = $form_state['values']['islandora_aspace_title'];
  $identifier = $form_state['values']['islandora_aspace_identifier'];
  $repository = $form_state['values']['islandora_aspace_repository'];

  $id = islandora_aspace_check_identifier($repository, $identifier);

  if ($id === TRUE || is_object($id)) {
    if ($id === TRUE) {
      $error_message = "Identifier already exists in ArchivesSpace.";
      $error_location = "islandora_aspace_identifier";
    }
    elseif (is_object($id)) {
      $error_message = "ArchivesSpace Error. Code: {$id->code}. Message: {$id->error}.";
      $error_location = "archivesspace_metadata";
    }
    form_set_error($error_location, filter_xss($error_message));
  }
}

/**
 * Metadata form submit handler.
 */
function islandora_aspace_step_metadata_form_submit(array $form, array &$form_state) {
  $storage = &islandora_ingest_form_get_step_storage($form_state);
  $storage['islandora_aspace_title'] = $form_state['values']['islandora_aspace_title'];
  $storage['islandora_aspace_identifier'] = $form_state['values']['islandora_aspace_identifier'];
  $storage['islandora_aspace_repository'] = $form_state['values']['islandora_aspace_repository'];
  $storage['islandora_aspace_published'] = $form_state['values']['islandora_aspace_published'];
}

/**
 * Callback to create archivesspace object.
 */
function islandora_aspace_step_object_create_callback(array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  module_load_include('inc', 'islandora_aspace', 'includes/archivesspace');

  $object = islandora_ingest_form_get_object($form_state);
  $storage = &islandora_ingest_form_get_step_storage($form_state, 'islandora_aspace_step_metadata_form');

  $title = $storage['islandora_aspace_title'];
  $identifier = $storage['islandora_aspace_identifier'];
  $repository = $storage['islandora_aspace_repository'];
  $published = $storage['islandora_aspace_published'];

  $return = islandora_aspace_create_deposit($repository, $title, $identifier, $published, $object->id);
  $body = json_decode($return->data, TRUE);
  if ($return->code != 200) {
    $error_message = '';

    if (isset($body['error'])) {
      if (is_array($body['error'])) {
        foreach ($body['error'] as $key => $value) {
          $error_message .= "$key: {$value[0]} ";
        }
      }
      else {
        $error_message = "ArchivesSpace: {$body['error']}";
      }
    }
    else {
      $error_message = "ArchivesSpace: {$return->code}: {$return->error}";
    }

    return islandora_aspace_step_fom_set_error($form_state, $error_message);
  }

  $object->label = $title;
  $object->state = $published ? 'A' : 'I';
  $models = $object->models;
  $models[] = ISLANDORA_ASPACE_OBJECT_CMODEL;
  $object->models = $models;
  islandora_aspace_create_datastreams($object, $body);
}

/**
 * Set a form error when ASpace fails.
 */
function islandora_aspace_step_fom_set_error(array &$form_state, $message) {
  $form_state['submit_handlers'] = NULL;
  $form_state['validate_handlers'] = NULL;
  $form_state['islandora']['objects'] = array();
  $form_state['islandora']['shared_storage']['objects'] = array();
  drupal_static_reset('islandora_ingest_form_get_steps');
  drupal_set_message(
    t(
      "Ingest Failed.\n @message.\n Please contact your system administrator.",
      array('@message' => $message)
    ),
    'error'
  );
}

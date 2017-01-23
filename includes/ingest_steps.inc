<?php

/**
 * @file
 * This file contains forms and callbacks used in the ingest hooks.
 */


/**
 * Form to select if the integration should be used for a particular object.
 */
function islandora_archivesspace_step_enable_form(array $form, array &$form_state) {
  $form = array(
    'islandora_archivesspace_enable' => array(
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

  if(isset($form_state['islandora']['step_storage']['islandora_archivesspace_step_enable_form']['islandora_archivesspace_enable'])) {
    $form['islandora_archivesspace_enable']['#default_value'] = $form_state['islandora']['step_storage']['islandora_archivesspace_step_enable_form']['islandora_archivesspace_enable'];
  }

  return $form;
}

/**
 * ArchivesSpace enable form submit handler.
 */
function islandora_archivesspace_step_enable_form_submit(array $form, array &$form_state) {
  $form_state['islandora']['step_storage']['islandora_archivesspace_step_enable_form']['islandora_archivesspace_enable'] = $form_state['values']['islandora_archivesspace_enable'];
}

/**
 * Simple metadata form for archivesspace.
 */
function islandora_archivesspace_step_metadata_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_archivesspace', 'includes/archivesspace');
  $repositories = islandora_archivesspace_get_repositories();
  $options = array();
  if($repositories !== FALSE) {
    foreach($repositories as $repository) {
      $options[$repository->uri] = $repository->name;
    }
  }
  else {
    drupal_set_message('Error contacting ArchivesSpace server. Please try again later.', 'error');
  }

  $form = array();

  $form['archivesspace_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Object Description'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['archivesspace_metadata']['islandora_archivesspace_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
  );

  $form['archivesspace_metadata']['islandora_archivesspace_identifier'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifier'),
    '#required' => TRUE,
  );

  $form['islandora_archivesspace_repository'] = array(
    '#type' => 'select',
    '#title' => t('Select Repository'),
    '#description' => t('Choose which ArchivesSpace repository to add this object to.'),
    '#options' => $options,
    '#required' => TRUE,
  );

  if(isset($form_state['islandora']['step_storage']['islandora_archivesspace_step_metadata_form'])) {
    $storage = $form_state['islandora']['step_storage']['islandora_archivesspace_step_metadata_form'];

    if(isset($storage['islandora_archivesspace_title'])) {
      $form['archivesspace_metadata']['islandora_archivesspace_title']['#default_value'] = $storage['islandora_archivesspace_title'];
    }

    if(isset($storage['islandora_archivesspace_identifier'])) {
      $form['archivesspace_metadata']['islandora_archivesspace_identifier']['#default_value'] = $storage['islandora_archivesspace_identifier'];
    }

    if(isset($storage['islandora_archivesspace_repository'])) {
      $form['islandora_archivesspace_repository']['#default_value'] = $storage['islandora_archivesspace_repository'];
    }
  }

  return $form;
}

/**
 * metadata form validate handler
 */
function islandora_archivesspace_step_metadata_form_validate(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_archivesspace', 'includes/utilities');

  $title = $form_state['values']['islandora_archivesspace_title'];
  $identifier = $form_state['values']['islandora_archivesspace_identifier'];
  $repository = $form_state['values']['islandora_archivesspace_repository'];

  $id = islandora_archivesspace_check_identifier($repository, $identifier);

  if($id === TRUE || is_object($id)) {
    if($id === TRUE) {
      $error_message = "Identifier already exists in ArchivesSpace.";
      $error_location = "islandora_archivesspace_identifier";
    }
    elseif(is_object($id)) {
      $error_message = "ArchivesSpace Error. Code: {$id->code}. Message: {$id->error}.";
      $error_location = "archivesspace_metadata";
    }
    form_set_error($error_location, $error_message);
  }
}

/**
 * metadata form submit handler
 */
function islandora_archivesspace_step_metadata_form_submit(array $form, array &$form_state) {
  $storage = &islandora_ingest_form_get_step_storage($form_state);
  $storage['islandora_archivesspace_title'] = $form_state['values']['islandora_archivesspace_title'];
  $storage['islandora_archivesspace_identifier'] = $form_state['values']['islandora_archivesspace_identifier'];
  $storage['islandora_archivesspace_repository'] = $form_state['values']['islandora_archivesspace_repository'];
}

/**
 * Callback to create archivesspace object.
 */
function islandora_archivesspace_step_object_create_callback(array &$form_state) {
  module_load_include('inc', 'islandora_archivesspace', 'includes/utilities');
  module_load_include('inc', 'islandora_archivesspace', 'includes/archivesspace');

  $object = islandora_ingest_form_get_object($form_state);
  $storage = &islandora_ingest_form_get_step_storage($form_state, 'islandora_archivesspace_step_metadata_form');

  $title = $storage['islandora_archivesspace_title'];
  $identifier = $storage['islandora_archivesspace_identifier'];
  $repository = $storage['islandora_archivesspace_repository'];

  $return = islandora_archivesspace_create_deposit($repository, $title, $identifier, $object->id);
  if ($return->code != 200) {
    $body = json_decode($return->data);
    $error_message = '';

    if(isset($body->error)) {
      if(is_object($body->error)) {
        foreach($body->error as $key => $value) {
          $error_message .= "$key: {$value[0]} ";
        }
      }
      else {
        $error_message = "ArchivesSpace: {$body->error}";
      }
    }
    else {
      $error_message = "ArchivesSpace: {$return->code}: {$return->error}";
    }

    return islandora_archivesspace_step_fom_set_error($form_state, $error_message);
  }

  $body = json_decode($return->data);
  $aspace_id = $body->id;
  $object->label = $title;
  $models = $object->models;
  $models[] = ISLANDORA_ARCHIVESSPACE_OBJECT_CMODEL;
  $object->models = $models;

  $return = islandora_archivesspace_get_digital_object($repository, $aspace_id);
  if ($return->code != 200) {
    return islandora_archivesspace_step_fom_set_error($form_state, 'Unable to retrieve freshly created ArchivesSpace object. Code: {$return->code}');
  }

  $data = json_decode($return->data, TRUE);
  islandora_archivesspace_create_datastreams($object, $data);
}

function islandora_archivesspace_step_fom_set_error(array &$form_state, $message) {
  $form_state['submit_handlers'] = NULL;
  $form_state['validate_handlers'] = NULL;
  $form_state['islandora']['objects'] = array();
  $form_state['islandora']['shared_storage']['objects'] = array();
  drupal_static_reset('islandora_ingest_form_get_steps');
  drupal_set_message("Ingest Failed.\n $message.\n Please contact your system administrator.", 'error');
}

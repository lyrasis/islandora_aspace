<?php

/**
 * @file
 * This file contains the admin form for Islandora ArchivesSpace.
 */

/**
 * Admin Form for the module.
 */
function islandora_aspace_admin_form(array $form, array &$form_state) {
  $form = array();

  $form['settings'] = array(
    '#type' => 'vertical_tabs',
  );

  $form['settings']['archivesspace_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('General Settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['settings']['archivesspace_settings']['islandora_aspace_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => variable_get('islandora_aspace_username', 'admin'),
  );

  $form['settings']['archivesspace_settings']['islandora_aspace_password'] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#default_value' => variable_get('islandora_aspace_password', 'admin'),
  );

  $form['settings']['archivesspace_settings']['islandora_aspace_baseurl'] = array(
    '#type' => 'textfield',
    '#title' => t('REST URL'),
    '#default_value' => variable_get('islandora_aspace_baseurl', 'http://localhost:8089'),
  );

  $form['settings']['archivesspace_settings']['islandora_aspace_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('API Key'),
    '#disabled' => TRUE,
    '#default_value' => variable_get('islandora_aspace_api_key'),
  );

  $form['settings']['archivesspace_settings']['islandora_aspace_regen_api'] = array(
    '#type' => 'submit',
    '#value' => t('Regenerate API Key'),
    '#submit' => array('islandora_aspace_admin_regen_key'),
  );

  $form['settings']['archivesspace_metadata_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Metadata Settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  module_load_include('inc', 'xml_form_builder', 'includes/associations');

  $form['settings']['archivesspace_metadata_settings']['islandora_aspace_transform'] = array(
    '#type' => 'select',
    '#title' => t('DC Crosswalk'),
    '#options' => array_flip(xml_form_builder_get_transforms()),
    '#default_value' => variable_get('islandora_aspace_transform', 'No Transform'),
    '#description' => t("An XSL transform for setting the Fedora object's Dublin Core metadata datastream."),
  );

  $form['settings']['archivesspace_metadata_settings']['islandora_aspace_self_transform'] = array(
    '#type' => 'select',
    '#title' => t('Self XSL Transform'),
    '#options' => array_flip(xml_form_builder_get_self_transforms()),
    '#default_value' => variable_get('islandora_aspace_self_transform', 'No Transform'),
    '#description' => t('An optional transformation applied to mods data from archivesspace prior to ingestion.'),
  );

  $form['settings']['archivesspace_metadata_settings']['islandora_aspace_mods_template'] = array(
    '#type' => 'textarea',
    '#title' => t('MODs Template'),
    '#default_value' => variable_get('islandora_aspace_mods_template', ISLANDORA_ASPACE_MODS),
    '#element_validate' => array('token_element_validate'),
    '#token_types' => array('aspace'),
  );

  $form['settings']['archivesspace_metadata_settings']['token_tree'] = array(
    '#type' => 'fieldset',
    '#title' => t('Replacement patterns'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => theme('token_tree', array(
      'token_types' => array('aspace'),
      'global_types' => FALSE,
    )),
    '#weight' => 10,
  );

  $form['settings']['archivesspace_content_models'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content Models'),
    '#description' => t('Select the content models for which to enable ArchivesSpace integration.'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  module_load_include('inc', 'islandora', 'includes/utilities');

  $form['settings']['archivesspace_content_models']['islandora_aspace_enabled_models'] = islandora_content_model_select_table_form_element('islandora_aspace_enabled_models');

  $form = system_settings_form($form);

  $form['#submit'][] = 'islandora_aspace_admin_form_submit';

  return $form;
}

/**
 * Function to regenerate the API key.
 */
function islandora_aspace_admin_regen_key(array $form, array &$form_state) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  $token = islandora_aspace_generate_token();
  variable_set('islandora_aspace_api_key', $token);
  drupal_set_message("API Key Successfully Regenerated");
}

/**
 * Validate the admin form before submitting.
 */
function islandora_aspace_admin_form_validate(array $form, array &$form_state) {
  $old_state = libxml_use_internal_errors(TRUE);
  libxml_clear_errors();
  $xml = new DOMDocument();
  $success = @$xml->loadXML($form_state['input']['islandora_aspace_mods_template']);
  if (!$success) {
    $errors = libxml_get_errors();
    form_set_error('islandora_aspace_mods_template', "XML Parsing Error: {$errors[0]->message}");
  }
  libxml_use_internal_errors($old_state);
}

/**
 * Admin form submit handler.
 */
function islandora_aspace_admin_form_submit(array $form, array &$form_state) {
  $enabled_content_models = array();
  foreach ($form_state['values']['islandora_aspace_enabled_models'] as $key => $value) {
    if ($value) {
      $enabled_content_models[] = $key;
    }
  }

  variable_set('islandora_aspace_enabled_models', $enabled_content_models);

  $url = $form_state['values']['islandora_aspace_baseurl'];
  $url = rtrim(trim($url), '/');
  variable_set('islandora_aspace_baseurl', $url);
}

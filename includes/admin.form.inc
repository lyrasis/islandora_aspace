<?php

/**
 * @file
 * This file contains the admin form for Islandora Archivespace.
 */

/**
 * Admin Form for the module.
 */
function islandora_archivespace_admin_form(array $form, array &$form_state) {
    $form = array();

    $form['archivespace_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Archivespace REST Settings'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    $form['archivespace_settings']['islandora_archivespace_username'] = array(
      '#type' => 'textfield',
      '#title' => t('Username'),
      '#default_value' => variable_get('islandora_archivespace_username'),
    );

    $form['archivespace_settings']['islandora_archivespace_password'] = array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => variable_get('islandora_archivespace_password'),
    );

    $form['archivespace_settings']['islandora_archivespace_baseurl'] = array(
      '#type' => 'textfield',
      '#title' => t('REST URL'),
      '#default_value' => variable_get('islandora_archivespace_baseurl', 'http://foo'),
    );

    $form['archivespace_metadata_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Metadata Settings'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    module_load_include('inc', 'xml_form_builder', 'includes/associations');

    $form['archivespace_metadata_settings']['islandora_archivespace_transform'] = array(
      '#type' => 'select',
      '#title' => t('DC Crosswalk'),
      '#options' => xml_form_builder_get_transforms(),
      '#default_value' => variable_get('islandora_archivespace_transform', 'No Transform'),
      '#description' => t("An XSL transform for setting the Fedora object's Dublin Core metadata datastream."),
    );

    $form['archivespace_metadata_settings']['islandora_archivespace_self_transform'] = array(
      '#type' => 'select',
      '#title' => t('Self XSL Transform'),
      '#options' => xml_form_builder_get_self_transforms(),
      '#default_value' => variable_get('islandora_archivespace_self_transform', 'No Transform'),
      '#description' => t('An optional transformation applied to mods data from archivespace prior to ingestion.'),
    );

    $form['archivespace_content_models'] = array(
      '#type' => 'fieldset',
      '#title' => t('Content Models'),
      '#description' => t('Select the content models for which to enable Archivematica integration.'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );

    module_load_include('inc', 'islandora', 'includes/utilities');

    $form['archivespace_content_models']['islandora_archivespace_enabled_models'] = islandora_content_model_select_table_form_element('islandora_archivespace_enabled_models');

    $form = system_settings_form($form);

    $form['#submit'][] = 'islandora_archivespace_admin_form_submit';

    return $form;
}

/**
 * Admin form submit handler.
 */
function islandora_archivespace_admin_form_submit(array $form, array &$form_state) {
    $enabled_content_models = array();
    foreach($form_state['values']['islandora_archivespace_enabled_models'] as $key => $value) {
        if($value) {
            $enabled_content_models[] = $key;
        }
    }

    variable_set('islandora_archivespace_enabled_models', $enabled_content_models);
}

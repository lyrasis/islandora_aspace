<?php

/**
 * @file
 * Derivative creation functions for the integration module.
 */

/**
 * This creates the Digital Object in ASpace using the data in the ASpace DS.
 */
function islandora_aspace_create_aspace_object($object, $force = FALSE) {
  module_load_include('inc', 'islandora_aspace', 'includes/aspace_api');
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');

  $return = array('success' => TRUE, 'messages' => array());

  // If its already been uploaded, or the DS isn't present, then just return.
  if (islandora_aspace_get_aspace_flag($object) || !isset($object[ISLANDORA_ASPACE_DSID])) {
    return $return;
  }

  $json = $object[ISLANDORA_ASPACE_DSID]->content;
  $data = json_decode($json, TRUE);
  $aspace_repo = $data['repository']['ref'];

  $aspace = islandora_aspace_create_deposit($aspace_repo, $object, $data);
  $aspace_body = json_decode($aspace->data, TRUE);

  if ($aspace->code != 200) {
    $return['success'] = FALSE;
    if (isset($aspace_body['error'])) {
      if (is_array($aspace_body['error'])) {
        foreach ($aspace_body['error'] as $key => $value) {
          $return['messages'][] = array(
            'message' => t('Error from ArchivesSpace: @error'),
            'message_sub' => array('@error' => "$key: {$value[0]} "),
            'type' => 'watchdog',
            'severity' => WATCHDOG_ERROR,
          );
        }
      }
      else {
        $return['messages'][] = array(
          'message' => t('Error from ArchivesSpace: @error'),
          'message_sub' => array('@error' => $aspace_body['error']),
          'type' => 'watchdog',
          'severity' => WATCHDOG_ERROR,
        );
      }
    }
    else {
      $return['messages'][] = array(
        'message' => t('Error from ArchivesSpace: @code @error'),
        'message_sub' => array('@code' => $aspace->code, '@error' => $aspace->error),
        'type' => 'watchdog',
        'severity' => WATCHDOG_ERROR,
      );
    }
  }
  else {
    islandora_aspace_update_aspace_ds($object, $aspace_body);
    islandora_aspace_set_aspace_flag($object, TRUE);
  }

  return $return;
}

/**
 * Updates properties on the object based on the ASpace DS.
 */
function islandora_aspace_update_object($object, $force = FALSE) {
  $json = $object[ISLANDORA_ASPACE_DSID]->content;
  $data = json_decode($json, TRUE);
  $object->label = $data['title'];
  $object->state = $data['publish'] ? 'A' : 'I';

  return array('success' => TRUE, 'messages' => array());
}

/**
 * Create MODs based on the ASpace DS and the selected template.
 */
function islandora_aspace_create_mods($object, $force = FALSE) {
  module_load_include('inc', 'islandora_aspace', 'includes/templates');
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');

  $return = array('success' => TRUE, 'messages' => array());

  if (isset($object['MODS']) && !$force) {
    return $return;
  }

  $json = $object[ISLANDORA_ASPACE_DSID]->content;
  $data = json_decode($json, TRUE);
  $mods_template = variable_get('islandora_aspace_template', 'Default');
  $twig = islandora_aspace_twig();

  try {
    $mods_xml = $twig->render($mods_template, $data);
  }
  catch (Twig_Error $e) {
    $return['success'] = FALSE;
    $return['messages'][] = array(
      'message' => t('Twig Template Error: @error'),
      'message_sub' => array('@error' => $e->getMessage()),
      'type' => 'watchdog',
      'severity' => WATCHDOG_ERROR,
    );
    return $return;
  }

  $lint = islandora_aspace_lint_xml($mods_xml);
  if ($lint !== TRUE) {
    $return['success'] = FALSE;
    $return['messages'][] = array(
      'message' => t('XML Parsing Error: @error'),
      'message_sub' => array('@error' => $lint),
      'type' => 'watchdog',
      'severity' => WATCHDOG_ERROR,
    );
    return $return;
  }

  $mods_xml = islandora_aspace_pretty_xml($mods_xml);

  if (!isset($object['MODS'])) {
    $mods_ds = $object->constructDatastream('MODS', 'M');
    $mods_ds->label = 'MODS Record';
    $mods_ds->mimetype = 'application/xml';
    $mods_ds->setContentFromString($mods_xml);
    $object->ingestDatastream($mods_ds);
  }
  else {
    $object['MODS']->setContentFromString($mods_xml);
  }

  return $return;
}

/**
 * Create DC datastream based on the MODs.
 */
function islandora_aspace_create_dc($object, $force = FALSE) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');
  $return = array('success' => TRUE, 'messages' => array());

  if (isset($object['DC']) && !$force) {
    return $return;
  }

  $mods_xml = $object['MODS']->content;
  $dc_transform = variable_get('islandora_aspace_transform', 'No Transform');
  if ($dc_transform != 'No Transform') {
    $dc_xml = islandora_aspace_transform_document($mods_xml, $dc_transform);
    if (!isset($object['DC'])) {
      $dc_ds = $object->constructDatastream('DC', 'X');
      $dc_ds->label = 'DC Record';
      $dc_ds->mimetype = 'application/xml';
      $dc_ds->setContentFromString($dc_xml);
      $object->ingestDatastream($dc_ds);
    }
    else {
      $object['DC']->setContentFromString($dc_xml);
    }
  }

  return $return;
}

<?php

/**
 * @file
 * This file contains utility functions.
 */

/**
 * Runs an XSLT document on a XML string.
 */
function islandora_aspace_transform_document($xml_string, $transform_path, $format = TRUE) {
  if ($transform_path == 'No Transform') {
    return $xml_string;
  }

  $dom = new DOMDocument();
  $dom->loadXML($xml_string);

  $xsl = new DOMDocument();
  $xsl->load($transform_path);

  $xslt = new XSLTProcessor();
  $xslt->importStyleSheet($xsl);

  $dom = $xslt->transformToDoc($dom);
  $dom->formatOutput = $format;

  return $dom->saveXML();
}

/**
 * Mangles the data we get from ASpace to make it a little nicer for templates.
 */
function islandora_aspace_preprocess_aspace_data($data) {
  module_load_include('inc', 'islandora_aspace', 'includes/aspace_api');

  $resolveref = function (&$array) use (&$resolveref) {
    if (isset($array['_resolved'])) {
      $item = $array['_resolved'];
      unset($array['_resolved']);
      $array = array_merge($array, $item);
    }
    foreach ($array as $key => $item) {
      if (is_array($item)) {
        $resolveref($array[$key]);
      }
    }
  };
  $filter = function (&$array, $keys) use (&$filter) {
    $removal_list = array();
    foreach ($array as $key => $item) {
      if (in_array($key, $keys, TRUE)) {
        $removal_list[] = $key;
      }
      elseif (is_array($item)) {
        $filter($array[$key], $keys);
      }
    }
    foreach ($removal_list as $key) {
      unset($array[$key]);
    }
  };
  $keys = array(
    'lock_version',
    'system_mtime',
    'user_mtime',
    'jsonmodel_type',
  );
  $resolveref($data);
  $filter($data, $keys);
  return $data;
}

/**
 * Updates ASpace datastreams (including MODS, DC).
 */
function islandora_aspace_create_datastreams($object, $data) {
  module_load_include('inc', 'islandora_aspace', 'includes/templates');
  $data = islandora_aspace_preprocess_aspace_data($data);
  $json_data = json_encode($data, JSON_PRETTY_PRINT);

  if (!isset($object[ISLANDORA_ASPACE_DSID])) {
    $aspace_ds = $object->constructDatastream(ISLANDORA_ASPACE_DSID, 'M');
    $aspace_ds->label = 'ArchivesSpace JSON Data';
    $aspace_ds->mimetype = 'application/json';
    $aspace_ds->setContentFromString($json_data);
    $object->ingestDatastream($aspace_ds);
  }
  else {
    $object[ISLANDORA_ASPACE_DSID]->setContentFromString($json_data);
  }

  $mods_template = variable_get('islandora_aspace_template', 'Default');
  $twig = islandora_aspace_twig();
  $mods_xml = $twig->render($mods_template, $data);

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
}

/**
 * Reasonably securely generate a token.
 */
function islandora_aspace_generate_token() {
  $length = 40;
  $token = base64_encode(openssl_random_pseudo_bytes($length));
  $token = rtrim($token, '=');
  $token = str_replace('+', 'A', $token);
  $token = str_replace('/', 'z', $token);
  return $token;
}

/**
 * Check if an ASpace identifier exists.
 */
function islandora_aspace_check_identifier($repository_uri, $identifier) {
  module_load_include('inc', 'islandora_aspace', 'includes/aspace_api');
  $return = islandora_aspace_search_repository($repository_uri, "digital_object_id:{$identifier}");
  if ($return->code != 200) {
    return $return;
  }

  $data = json_decode($return->data, TRUE);
  if (isset($data['total_hits']) && $data['total_hits'] == 0) {
    return FALSE;
  }
  else {
    return TRUE;
  }
}

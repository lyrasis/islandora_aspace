<?php

/**
 * @file
 * This file contains utility functions.
 */

// Relations.
define('ISLANDORA_ASPACE_IN_ASPACE', 'inAspace');

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
 * Do initial ASpace object setup.
 */
function islandora_aspace_setup_object($object, $repository, $title, $published) {
  // Add the ASpace CModel.
  $models = $object->models;
  $models[] = ISLANDORA_ASPACE_OBJECT_CMODEL;
  $object->models = $models;

  // Set the object label.
  $object->label = $title;

  // Create intial DS for Aspace.
  $identifier = $object->id;
  $json = array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['publish'] = $published ? TRUE : FALSE;
  $json['file_versions'] = array(
    array(
      'file_uri' => url(
        "/islandora/object/$identifier",
        array('absolute' => TRUE)
      ),
    ),
  );
  $json['repository'] = array('ref' => $repository);
  islandora_aspace_update_aspace_ds($object, $json);

  // Set flag in RELS.
  islandora_aspace_set_aspace_flag($object, FALSE);
}

/**
 * Updates or created object based on aspace data.
 */
function islandora_aspace_update_aspace_ds($object, $data) {
  $processed_data = islandora_aspace_preprocess_aspace_data($data);
  $json_data = json_encode($processed_data, JSON_PRETTY_PRINT);
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

/**
 * Set the aspace flag on an object.
 */
function islandora_aspace_set_aspace_flag(AbstractObject $object, $value) {
  $current_flag = islandora_aspace_get_aspace_flag($object);

  if ($current_flag === $value) {
    return;
  }

  if ($current_flag !== NULL) {
    $object->relationships->autoCommit = FALSE;
    islandora_aspace_remove_aspace_flag($object);
  }

  $object->relationships->add(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_ASPACE_IN_ASPACE,
    $value ? 'true' : 'false',
    RELS_TYPE_PLAIN_LITERAL
  );
  $object->relationships->commitRelationships();
}

/**
 * Get the aspace flag on an object.
 */
function islandora_aspace_get_aspace_flag(AbstractObject $object) {
  $results = $object->relationships->get(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_ASPACE_IN_ASPACE,
    NULL,
    RELS_TYPE_PLAIN_LITERAL
  );

  if (count($results) > 1) {
    throw new Exception('Too many relationships!');
  }
  elseif (count($results) == 0) {
    return NULL;
  }
  else {
    if ($results[0]['object']['value'] === 'true') {
      return TRUE;
    }
    elseif ($results[0]['object']['value'] === 'false') {
      return FALSE;
    }
    else {
      throw new Exception('Incorrect literal!');
    }
  }
}

/**
 * Remove the aspace flag on an object.
 */
function islandora_aspace_remove_aspace_flag(AbstractObject $object) {
  $object->relationships->remove(
    ISLANDORA_RELS_EXT_URI,
    ISLANDORA_ASPACE_IN_ASPACE,
    NULL,
    RELS_TYPE_PLAIN_LITERAL
  );
}

/**
 * Lint an xml file to make sure its proper XML.
 */
function islandora_aspace_lint_xml($xml) {
  $return = TRUE;
  $old_state = libxml_use_internal_errors(TRUE);
  libxml_clear_errors();
  $document = new DOMDocument();
  $success = @$document->loadXML($xml);
  if (!$success) {
    $errors = libxml_get_errors();
    $return = $errors[0]->message;
  }
  libxml_use_internal_errors($old_state);
  return $return;
}

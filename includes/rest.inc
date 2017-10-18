<?php

/**
 * @file
 * Sets up REST endpoints for ASpace to call into islandora.
 */

/**
 * Get PID from ASpace data.
 */
function islandora_aspace_get_uri($data) {
  $uri = isset($data['file_versions'][0]['file_uri']) ? $data['file_versions'][0]['file_uri'] : '';
  $uri = explode('/', $uri);
  return urldecode($uri[count($uri) - 1]);
}

/**
 * Function called for POST and PUT requests from ASpace.
 */
function islandora_aspace_rest_create_update($pid, $data, $type) {
  module_load_include('inc', 'islandora_aspace', 'includes/utilities');

  if (islandora_aspace_get_uri($data) != $pid) {
    return services_error("PID and URI mismatch.", 412);
  }

  $object = islandora_object_load($pid);
  if (!$object) {
    return services_error("Unable to load object $pid.", 404);
  }

  if (!isset($data['title'])) {
    return services_error('Missing title.', 406);
  }

  if (!isset($data['publish'])) {
    return services_error('Missing publish.', 406);
  }

  if (!in_array(ISLANDORA_ASPACE_OBJECT_CMODEL, $object->models)) {
    if ($type == 'update') {
      return services_error('Object is not linked to ArchivesSpace.', 406);
    }
    else {
      $models = $object->models;
      $models[] = ISLANDORA_ASPACE_OBJECT_CMODEL;
      $object->models = $models;
    }
  }

  islandora_aspace_update_aspace_ds($object, $data);

  $islandora_uri = url("/islandora/object/$pid", array('absolute' => TRUE));
  return array(
    'uri' => $islandora_uri,
    'pid' => $pid,
  );
}

/**
 * Function called for PUT requests to integration.
 */
function islandora_aspace_rest_update($pid, $data) {
  return islandora_aspace_rest_create_update($pid, $data, 'update');
}

/**
 * Function called for POST requests to integration.
 */
function islandora_aspace_rest_create($pid, $data) {
  return islandora_aspace_rest_create_update($pid, $data, 'create');
}

/**
 * Function called for DELETE requests to integration.
 */
function islandora_aspace_rest_delete($pid) {
  $object = islandora_object_load($pid);
  if (!$object) {
    return services_error("Unable to load object $pid.", 404);
  }

  if (!in_array(ISLANDORA_ASPACE_OBJECT_CMODEL, $object->models)) {
    return services_error('Object is not linked to ArchivesSpace.', 406);
  }

  // Remove ASpace content model.
  $models = $object->models;
  $key = array_search(ISLANDORA_ASPACE_OBJECT_CMODEL, $models);
  unset($models[$key]);
  $object->models = $models;

  $islandora_uri = url("/islandora/object/$pid", array('absolute' => TRUE));

  return array(
    'uri' => $islandora_uri,
    'pid' => $pid,
  );
}

/**
 * Function called for GET requests to integration.
 */
function islandora_aspace_rest_retrieve($pid) {
  $object = islandora_object_load($pid);
  if (!$object) {
    return services_error("Unable to load object $pid.", 404);
  }

  $aspace = in_array(ISLANDORA_ASPACE_OBJECT_CMODEL, $object->models);

  $islandora_uri = url("/islandora/object/$pid", array('absolute' => TRUE));

  return array(
    'uri' => $islandora_uri,
    'pid' => $pid,
    'title' => $object->label,
    'linked' => (bool) $aspace,
    'data' => $aspace ? json_decode($object[ISLANDORA_ASPACE_DSID]->content, TRUE) : NULL,
  );
}

/**
 * Function checks access for requests from ASpace using API key.
 */
function islandora_aspace_rest_access() {
  if (!isset($_SERVER['HTTP_X_ISLANDORA_ASPACE_APIKEY'])) {
    return FALSE;
  }

  $key = variable_get('islandora_aspace_api_key');
  if (empty($key)) {
    return FALSE;
  }

  if ($_SERVER['HTTP_X_ISLANDORA_ASPACE_APIKEY'] === $key) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Implements hook_default_services_endpoint().
 */
function islandora_aspace_default_services_endpoint() {
  $endpoints = array();

  $endpoint = new stdClass();
  $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
  $endpoint->api_version = 3;
  $endpoint->name = 'islandora_aspace';
  $endpoint->server = 'rest_server';
  $endpoint->path = 'islandora/aspace';
  $endpoint->authentication = array();
  $endpoint->server_settings = array(
    'formatters' => array(
      'json' => TRUE,
      'xml' => TRUE,
      'bencode' => FALSE,
      'jsonp' => FALSE,
      'php' => FALSE,
    ),
    'parsers' => array(
      'application/json' => TRUE,
      'text/xml' => TRUE,
      'application/vnd.php.serialized' => FALSE,
      'application/x-www-form-urlencoded' => FALSE,
      'application/xml' => FALSE,
      'multipart/form-data' => FALSE,
    ),
  );
  $endpoint->resources = array(
    'islandora_aspace' => array(
      'alias' => 'object',
      'operations' => array(
        'create' => array(
          'enabled' => '1',
          'settings' => array(),
        ),
        'retrieve' => array(
          'enabled' => '1',
          'settings' => array(),
        ),
        'update' => array(
          'enabled' => '1',
          'settings' => array(),
        ),
        'delete' => array(
          'enabled' => '1',
          'settings' => array(),
        ),
      ),
    ),
  );
  $endpoint->debug = 0;

  $endpoints[] = $endpoint;
  return $endpoints;
}

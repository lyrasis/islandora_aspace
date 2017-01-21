<?php

function islandora_archivesspace_rest_update($pid, $data) {
  module_load_include('inc', 'islandora_archivesspace', 'includes/utilities');

  $uri = $data['file_versions'][0]['file_uri'];
  $uri = explode('/',$uri);
  $uri_pid = $uri[count($uri)-1];

  if($uri_pid != $pid) {
    return services_error("PID and URI mismatch.", 412);
  }

  $object = islandora_object_load($pid);
  if(!$object) {
    return services_error("Unable to load object $pid.", 404);
  }

  if(!isset($data['title'])) {
    return services_error('Missing title.', 406);
  }

  if(!isset($data['publish'])) {
    return services_error('Missing publish.', 406);
  }

  $title = $data['title'];
  $published = $data['publish'];

  $object->label = $title;
  $object->state = $published ? 'A' : 'I';

  islandora_archivesspace_create_datastreams($object, $data);

  if(!in_array(ISLANDORA_ARCHIVESSPACE_OBJECT_CMODEL, $object->models)){
    $models = $object->models;
    $models[] = ISLANDORA_ARCHIVESSPACE_OBJECT_CMODEL;
    $object->models = $models;
  }

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  return array(
    'uri' => $islandora_uri,
    'pid' => $pid,
  );
}

function islandora_archivesspace_rest_delete($pid) {
  $object = islandora_object_load($pid);
  if(!$object) {
    return services_error("Unable to load object $pid.", 404);
  }

  if(!in_array(ISLANDORA_ARCHIVESSPACE_OBJECT_CMODEL, $object->models)){
    return services_error('Object is not linked to ArchivesSpace.', 406);
  }

  // Remove ASpace content model
  $models = $object->models;
  $key = array_search(ISLANDORA_ARCHIVESSPACE_OBJECT_CMODEL, $models);
  unset($models[$key]);
  $object->models = $models;

  // Remove the ASpace JSON
  unset($object[ISLANDORA_ARCHIVESSPACE_DSID]);

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  return array(
    'uri' => $islandora_uri,
    'pid' => $pid,
  );
}

function islandora_archivesspace_rest_access() {
  if(!isset($_SERVER['HTTP_X_ISLANDORA_ASPACE_APIKEY'])) {
    return FALSE;
  }

  $key = variable_get('islandora_archivesspace_api_key');
  if(empty($key)) {
    return FALSE;
  }

  if($_SERVER['HTTP_X_ISLANDORA_ASPACE_APIKEY'] === $key) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}


/**
 * Implements hook_default_services_endpoint().
 */
function islandora_archivesspace_default_services_endpoint() {
  $endpoints = array();

  $endpoint = new stdClass();
  $endpoint->disabled = FALSE; /* Edit this to true to make a default endpoint disabled initially */
  $endpoint->api_version = 3;
  $endpoint->name = 'islandora_archivesspace';
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
    'islandora_archivesspace' => array(
      'alias' => 'object',
      'operations' => array(
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

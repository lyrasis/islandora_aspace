<?php

/**
 * @file
 * This file contains functions for calling aspace api.
 */

function islandora_archivesspace_get_repositories() {
  $data = drupal_http_request(variable_get('islandora_archivesspace_baseurl') . '/repositories');
  if($data->code != 200) {
    return FALSE;
  }

  return json_decode($data->data);
}

function islandora_archivesspace_authenticate($force = FALSE) {
  STATIC $session = array(FALSE, NULL);

  if(!$session[0] || $force) {
    $username = variable_get('islandora_archivesspace_username');
    $password = variable_get('islandora_archivesspace_password');
    $url = variable_get('islandora_archivesspace_baseurl') . '/users/' . urlencode($username) . '/login?password=' . urlencode($password);
    $data = drupal_http_request($url, array('method' => 'POST'));

    if($data->code == 200) {
      $json = json_decode($data->data);
      $session = array(TRUE, $json->session);
    }
    else {
      $session = array(FALSE, $data);
    }
  }

  return $session;
}

function islandora_archivesspace_create_digital_object($repository_uri, $title, $identifier, $pid) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  // Request to create new object.
  $json = Array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['file_versions'] = array(
    array(
      'file_uri' => $islandora_uri,
    ),
  );

  $url = variable_get('islandora_archivesspace_baseurl') . $repository_uri . '/digital_objects';
  return drupal_http_request($url,
    array(
      'method' => 'POST',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
      'data' => json_encode($json),
    )
  );
}

function islandora_archivesspace_delete_digital_object($repository_uri, $id) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_archivesspace_baseurl') . $repository_uri . '/digital_objects/' . $id;
  return drupal_http_request($url,
    array(
      'method' => 'DELETE',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

function islandora_archivesspace_get_digital_object($repository_uri, $id) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_archivesspace_baseurl') . $repository_uri . '/digital_objects/' . $id;
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

function islandora_archivesspace_get_ref($uri) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $success;
  }

  $url = variable_get('islandora_archivesspace_baseurl') . $uri;
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

function islandora_archivesspace_create_deposit($repository_uri, $title, $identifier, $pid) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  // Request to create new object.
  $json = Array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['file_versions'] = array(
    array(
      'file_uri' => $islandora_uri,
    ),
  );

  $url = variable_get('islandora_archivesspace_baseurl');
  $url .= '/plugins/aspace_islandora' . $repository_uri . '/islandora_deposits?pid=' . urlencode($pid);

  return drupal_http_request($url,
    array(
      'method' => 'POST',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
      'data' => json_encode($json),
    )
  );
}

function islandora_archivesspace_delete_deposit($object_uri) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $parts = explode('/', $object_uri);
  $url = variable_get('islandora_archivesspace_baseurl');
  $url .= "/plugins/aspace_islandora/repositories/{$parts[2]}/islandora_deposits/{$parts[4]}";

  return drupal_http_request($url,
    array(
      'method' => 'DELETE',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

function islandora_archivesspace_search_repository($repository_uri, $q, $page = 1) {
  list($success, $session_key) = islandora_archivesspace_authenticate();
  if(!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_archivesspace_baseurl') . "{$repository_uri}/search?page={$page}&q=" . urlencode($q);
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

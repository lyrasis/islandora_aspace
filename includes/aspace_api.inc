<?php

/**
 * @file
 * This file contains functions for calling aspace api.
 */

/**
 * Get list of repositories from ASpace.
 */
function islandora_aspace_get_repositories() {
  $data = drupal_http_request(variable_get('islandora_aspace_baseurl') . '/repositories');
  if ($data->code != 200) {
    return FALSE;
  }

  return json_decode($data->data);
}

/**
 * Authenticate against the ASpace API.
 */
function islandora_aspace_authenticate($force = FALSE) {
  static $session = array(FALSE, NULL);

  if (!$session[0] || $force) {
    $username = variable_get('islandora_aspace_username');
    $password = variable_get('islandora_aspace_password');
    $url = variable_get('islandora_aspace_baseurl') . '/users/' . urlencode($username) . '/login?password=' . urlencode($password);
    $data = drupal_http_request($url, array('method' => 'POST'));

    if ($data->code == 200) {
      $json = json_decode($data->data);
      $session = array(TRUE, $json->session);
    }
    else {
      $session = array(FALSE, $data);
    }
  }

  return $session;
}

/**
 * Create a new digital object in ASpace.
 */
function islandora_aspace_create_digital_object($repository_uri, $title, $identifier, $pid) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  // Request to create new object.
  $json = array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['file_versions'] = array(
    array(
      'file_uri' => $islandora_uri,
    ),
  );

  $url = variable_get('islandora_aspace_baseurl') . $repository_uri . '/digital_objects';
  return drupal_http_request($url,
    array(
      'method' => 'POST',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
      'data' => json_encode($json),
    )
  );
}

/**
 * Delete a digital object in ASpace.
 */
function islandora_aspace_delete_digital_object($repository_uri, $id) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_aspace_baseurl') . $repository_uri . '/digital_objects/' . $id;
  return drupal_http_request($url,
    array(
      'method' => 'DELETE',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

/**
 * Get ASpace digital object.
 */
function islandora_aspace_get_digital_object($repository_uri, $id) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_aspace_baseurl') . $repository_uri . '/digital_objects/' . $id;
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

/**
 * Resolve a reference in ASpace.
 */
function islandora_aspace_get_ref($uri) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $success;
  }

  $url = variable_get('islandora_aspace_baseurl') . $uri;
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

/**
 * Create a deposit against the ASpace Islandora plugin.
 */
function islandora_aspace_create_deposit($repository_uri, $title, $identifier, $published, $pid) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $islandora_uri = $GLOBALS['base_url'] . '/islandora/object/' . $pid;

  // Request to create new object.
  $json = array();
  $json['title'] = $title;
  $json['digital_object_id'] = $identifier;
  $json['publish'] = $published ? TRUE : FALSE;
  $json['file_versions'] = array(
    array(
      'file_uri' => $islandora_uri,
    ),
  );

  $url = variable_get('islandora_aspace_baseurl');
  $url .= '/plugins/aspace_islandora' . $repository_uri . '/islandora_deposits?pid=' . urlencode($pid);

  return drupal_http_request($url,
    array(
      'method' => 'POST',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
      'data' => json_encode($json),
    )
  );
}

/**
 * Delete the deposit.
 */
function islandora_aspace_delete_deposit($object_uri) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $parts = explode('/', $object_uri);
  $url = variable_get('islandora_aspace_baseurl');
  $url .= "/plugins/aspace_islandora/repositories/{$parts[2]}/islandora_deposits/{$parts[4]}";

  return drupal_http_request($url,
    array(
      'method' => 'DELETE',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}

/**
 * Do a search in ASpace using its API.
 */
function islandora_aspace_search_repository($repository_uri, $q, $page = 1) {
  list($success, $session_key) = islandora_aspace_authenticate();
  if (!$success) {
    return $session_key;
  }

  $url = variable_get('islandora_aspace_baseurl') . "{$repository_uri}/search?page={$page}&q=" . urlencode($q);
  return drupal_http_request($url,
    array(
      'method' => 'GET',
      'headers' => array('X-ArchivesSpace-Session' => $session_key),
    )
  );
}
